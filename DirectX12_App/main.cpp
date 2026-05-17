#include <string>

#include "Engine/Core/Window.h"
#include "Engine/Graphics/GfxDevice.h"
#include "Engine/Graphics/Renderer.h"
#include "App/Scene.h"
#include "App/World/Chunk.h"
#include "App/World/World.h"

// ImGui
#include "Engine/ThirdParty/imgui/imgui.h"
#include "Engine/ThirdParty/imgui/backends/imgui_impl_dx12.h"
#include "Engine/ThirdParty/imgui/backends/imgui_impl_win32.h"

namespace {
	// 更新したチャンクの隣接するチャンクを更新
	void UpdateChunkNeighbors(int worldX, int worldY, int worldZ, World& world, Renderer& renderer, GfxDevice& gfxDevice, const std::unordered_map<ChunkCoord, uint32_t, ChunkCoordHash>& chunkMeshMap) {
		// 書き換えたブロックが属するチャンク座標とローカル座標を計算
		int chunkX = WorldCoord::FloorDiv(worldX, Chunk::CHUNK_SIZE);
		int chunkZ = WorldCoord::FloorDiv(worldZ, Chunk::CHUNK_SIZE);
		int localX = WorldCoord::Mod(worldX, Chunk::CHUNK_SIZE);
		int localZ = WorldCoord::Mod(worldZ, Chunk::CHUNK_SIZE);

		// 更新が必要なチャンクのリストを作成
		std::vector<ChunkCoord> toUpdate;
		toUpdate.push_back({ chunkX,chunkZ });

		// チャンクの境界に隣接しているブロックの場合、隣のチャンクをリストに追加
		if (localX == 0) {
			toUpdate.push_back({ chunkX - 1, chunkZ });
		}
		if (localZ == 0) {
			toUpdate.push_back({ chunkX, chunkZ - 1 });
		}
		if (localX == Chunk::CHUNK_SIZE - 1) {
			toUpdate.push_back({ chunkX + 1, chunkZ });
		}
		if (localZ == Chunk::CHUNK_SIZE -1) {
			toUpdate.push_back({ chunkX, chunkZ + 1 });
		}

		// GPU処理待ち
		gfxDevice.WaitForGPU();

		//各チャンクを更新
		for (const auto& coord : toUpdate) {
			auto chunkIt = world.GetChunks().find(coord);
			auto meshIt = chunkMeshMap.find(coord);

			// チャンクが存在しない or meshIndex未登録の場合描画をスキップ
			if (chunkIt == world.GetChunks().end()) {
				continue;
			}
			if (meshIt == chunkMeshMap.end()) {
				continue;
			}

			renderer.UpdateChunkMesh(gfxDevice.GetDevice(), gfxDevice.GetCommandQueue(), *chunkIt->second, world, coord.x, coord.z, meshIt->second);
		}
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// ウィンドウ作成
	Window window(1280, 720, L"DirectX12 APP");

	// GfxDevice初期化処理
	GfxDevice gfxDevice;
	if (!gfxDevice.Initialize(window.GetHwnd(), window.GetWidth(), window.GetHeight())) {
		return -1;
	}
	if (!gfxDevice.InitializeFrameResources()) {
		return -1;
	}

	Scene scene;
	scene.Initialize(static_cast<float>(window.GetWidth()) / window.GetHeight());

	Renderer renderer;
	if (!renderer.Initialize(gfxDevice.GetDevice(), gfxDevice.GetCommandQueue(), gfxDevice.GetCurrentFrame().GetAllocator())) {
		return -1;
	}
	if (!renderer.InitializeShadow(gfxDevice.GetDevice())) {
		return -1;
	}

	// 全オブジェクトを登録
	auto& objects = scene.GetObjects();
	for (auto& obj : objects) {
		renderer.RegisterObject(gfxDevice.GetDevice(), gfxDevice.GetCommandQueue(), obj);
	}

	// チャンクを生成
	World world;
	for (int cx = 0; cx < 4; ++cx) {
		for (int cz = 0; cz < 4; ++cz) {
			world.GenerateChunk(cx, cz);
		}
	}

	std::unordered_map<ChunkCoord, uint32_t, ChunkCoordHash> chunkMeshMap;

	// 各チャンクをRendererに登録
	for (const auto& pair : world.GetChunks()) {
		const ChunkCoord& coord = pair.first;
		const Chunk& chunk = *pair.second;

		SceneObject chunkObj;
		sprintf_s(chunkObj.name, "Chunk %d_%d", coord.x, coord.z);
		chunkObj.texturePath = "App/Textures/texture_atlas.png";
		chunkObj.position[0] = static_cast<float>(coord.x * Chunk::CHUNK_SIZE);
		chunkObj.position[1] = 0.0f;
		chunkObj.position[2] = static_cast<float>(coord.z * Chunk::CHUNK_SIZE);
		chunkObj.scale = 1.0f;

		renderer.RegisterChunk(gfxDevice.GetDevice(), gfxDevice.GetCommandQueue(), chunk, world, coord.x, coord.z, chunkObj.texturePath, chunkObj);

		// meshIndexを保存
		chunkMeshMap[coord] = chunkObj.meshIndex;

		scene.GetObjects().push_back(chunkObj);
	}

	// スカイボックス初期化
	std::string skyFaces[6] = {
		"App/Textures/skybox/sh_right.png",
		"App/Textures/skybox/sh_left.png",
		"App/Textures/skybox/sh_top.png",
		"App/Textures/skybox/sh_bottom.png",
		"App/Textures/skybox/sh_back.png",
		"App/Textures/skybox/sh_front.png",
	};
	renderer.InitializeSkybox(gfxDevice.GetDevice(), gfxDevice.GetCommandQueue(), gfxDevice.GetCurrentFrame().GetAllocator(), skyFaces);

	// ImGui初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window.GetHwnd());

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = gfxDevice.GetDevice();
	initInfo.CommandQueue = gfxDevice.GetCommandQueue();
	initInfo.NumFramesInFlight = 2;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	initInfo.SrvDescriptorHeap = renderer.GetImGuiSrvHeap().GetHeap();
	initInfo.LegacySingleSrvCpuDescriptor = renderer.GetImGuiSrvHeap().GetCPUHandle(0);
	initInfo.LegacySingleSrvGpuDescriptor = renderer.GetImGuiSrvHeap().GetGPUHandle(0);

	ImGui_ImplDX12_Init(&initInfo);

	// マウスを中央に固定
	window.SetMouseCaptured(true);

	auto lastTime = std::chrono::high_resolution_clock::now();

	// メインループ
	while (window.ProcessMessage()){
		scene.Update();

		auto now = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;

		// 入力
		const auto& mouse = window.GetMouseInput();
		const auto& keyboard = window.GetKeyboardInput();
		auto& fpsCam = scene.GetFPSCamera();
		auto& player = scene.GetPlayer();

		// プレイヤー更新
		float yaw = fpsCam.GetYaw();
		player.Update(deltaTime, world,
					  keyboard.w, keyboard.s, keyboard.a, keyboard.d,
					  keyboard.space, yaw);

		// カメラ位置をプレイヤーの目の位置に同期
		fpsCam.SetPosition(player.GetEyePosition());

		DirectX::XMFLOAT3 camPos = scene.GetFPSCamera().GetPosition();
		DirectX::XMFLOAT3 camDir = scene.GetFPSCamera().GetForward();

		RaycastResult ray = world.Raycast(camPos, camDir, 10.0f);


		// ImGuiがマウスを使ってないときだけカメラ操作
		if (!ImGui::GetIO().WantCaptureMouse) {
			fpsCam.Rotate(static_cast<float>(mouse.deltaX), static_cast<float>(mouse.deltaY));

			if (mouse.leftClicked) {
				if (ray.hit) {
					world.SetBlockAt(ray.blockX, ray.blockY, ray.blockZ, BlockType::Air);
					// チャンクメッシュ更新
					UpdateChunkNeighbors(ray.blockX, ray.blockY, ray.blockZ, world, renderer, gfxDevice, chunkMeshMap);
				}
			}
			if (mouse.rightClicked) {
				// 設置：レイが当たったブロックからfaceの方向に1ずれた位置
				switch (ray.face) {
				case 0:
					ray.blockX += 1;
					break;
				case 1:
					ray.blockX -= 1;
					break;
				case 2:
					ray.blockY += 1;
					break;
				case 3:
					ray.blockY -= 1;
					break;
				case 4:
					ray.blockZ += 1;
					break;
				case 5:
					ray.blockZ -= 1;
					break;
				}
				world.SetBlockAt(ray.blockX, ray.blockY, ray.blockZ, BlockType::Stone);
				// チャンクメッシュ更新
				UpdateChunkNeighbors(ray.blockX, ray.blockY, ray.blockZ, world, renderer, gfxDevice, chunkMeshMap);
			}
		}

		window.ResetMouseDelta();

		gfxDevice.BeginFrame();

		renderer.Render(
			gfxDevice.GetCommandList(),
			scene,
			gfxDevice.GetFrameIndex(),
			gfxDevice.GetCurrentFrame(),
			gfxDevice.GetCurrentRTV(),
			gfxDevice.GetDSV(),
			gfxDevice.GetWidth(),
			gfxDevice.GetHeight());

		// ImGui描画
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Debug");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

		ImGui::Separator();
		ImGui::SliderFloat("Rotation Speed", &scene.GetRotationSpeed(), 0.0f, 3.0f);
		ImGui::SliderFloat("FOV", &scene.GetFPSCamera().GetFov(), 10.0f, 120.0f);

		ImGui::Separator();
		ImGui::Text("Raycast");
		if (ray.hit) {
			ImGui::Text("Hit: (%d, %d, %d)", ray.blockX, ray.blockY, ray.blockZ);
			ImGui::Text("Face: %d", ray.face);
		}
		else {
			ImGui::Text("No hit");
		}

		ImGui::Separator();
		ImGui::Text("Lighting");
		ImGui::SliderFloat3("Light Direction", scene.GetLightDir(), -1.0f, 1.0f);
		ImGui::SliderFloat("Specular", &scene.GetSapcIntensity(), 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &scene.GetSpecShiciness(), 1.0f, 256.0f);
		ImGui::ColorEdit3("Light Color", scene.GetLightColor());
		ImGui::ColorEdit3("Ambient", scene.GetAmbientColor());
		ImGui::SliderFloat("Shadow Bias", &scene.GetShadowBias(), 0.0f, 0.01f, "%.5f");

		ImGui::Separator();
		ImGui::Text("Culling");
		ImGui::Checkbox("Frustum Culling", &renderer.GetCullingEnabled());
		ImGui::Text("Visible Chunks; %u / %u", renderer.GetVisibleChunkCount(), renderer.GetTotalChunkCount());

		ImGui::Separator();
		ImGui::Text("Objects (%d)", scene.GetObjectCount());

		auto& objects = scene.GetObjects();
		for (uint32_t i = 0; i < objects.size(); ++i) {
			ImGui::PushID(i);
			if (ImGui::TreeNode(objects[i].name)) {
				ImGui::InputText("Name", objects[i].name, sizeof(objects[i].name));
				ImGui::SliderFloat3("Position", objects[i].position, -10.0f, 10.0f);
				ImGui::SliderFloat3("Rotaion", objects[i].rotation, -180.0f, 180.0f);
				ImGui::SliderFloat("Scale", &objects[i].scale, 0.01f, 5.0f);

				// モデル切り替え
				const auto& modelPaths = renderer.GetModelPaths();
				int currentModel = static_cast<int>(objects[i].meshIndex);
				if (ImGui::BeginCombo("Model", modelPaths[currentModel].c_str())) {
					for (int m = 0; m < modelPaths.size(); ++m) {
						if (ImGui::Selectable(modelPaths[m].c_str(), m == currentModel)) {
							objects[i].meshIndex = m;
						}
					}
					ImGui::EndCombo();
				}

				// テクスチャ切り替え
				const auto& texPaths = renderer.GetTexturePaths();
				int currentTex = static_cast<int>(objects[i].textureIndex);
				if (ImGui::BeginCombo("Texture", texPaths[currentTex].c_str())) {
					for (int t = 0; t < texPaths.size(); ++t) {
						if (ImGui::Selectable(texPaths[t].c_str(), t == currentTex)) {
							objects[i].textureIndex = t;
						}
					}
					ImGui::EndCombo();
				}

				if (ImGui::Button("Delete") && objects.size() > 1) {
					objects.erase(objects.begin() + i);
					ImGui::TreePop();
					ImGui::PopID();
					break;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		// パス入力用のバッファ
		// モデル
		static char modelPathBuffer[256] = "App/Models/sword.obj";
		ImGui::InputText("Model Path", modelPathBuffer, sizeof(modelPathBuffer));
		// テクスチャ
		static char texturePathBuffer[256] = "App/Textures/sample.png";
		ImGui::InputText("Texture Path", texturePathBuffer, sizeof(texturePathBuffer));

		if (objects.size() < 16 && ImGui::Button("+ Add Object")) {
			SceneObject newObj;
			sprintf_s(newObj.name, "Object %d", (int)objects.size());
			newObj.modelPath = modelPathBuffer;
			newObj.texturePath = texturePathBuffer;
			newObj.scale = 0.1f;

			renderer.RegisterObject(gfxDevice.GetDevice(), gfxDevice.GetCommandQueue(), newObj);
			objects.push_back(newObj);
		}

		ImGui::End();
		ImGui::Render();

		auto* cmdList = gfxDevice.GetCommandList();
		ID3D12DescriptorHeap* imguiHeaps[] = { renderer.GetImGuiSrvHeap().GetHeap() };
		cmdList->SetDescriptorHeaps(1, imguiHeaps);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);

		gfxDevice.EndFrame();
	}

	// ImGuiシャットダウン
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}