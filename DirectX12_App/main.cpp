#include <string>
#include <random>

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
	const int RENDER_DISTANCE = 4; //メッシュを表示する範囲
	const int MAX_LOADS_PER_FRAME = 2; // 1フレームに作るメッシュ数の上限

	const int HOTBAR_SIZE = 5;

	BlockType g_hotbar[HOTBAR_SIZE] = {
		BlockType::Grass,
		BlockType::Dirt,
		BlockType::Stone,
	};
	int g_selectedSlot = 0;

	struct LoadedChunk { uint32_t meshIndex; };

	// プレイヤーの位置に合わせてチャンクをロード・アンロードする
	void UpdateStreaming(World& world, Renderer& renderer, GfxDevice& gfx, Scene& scene,
						 std::unordered_map<ChunkCoord, LoadedChunk, ChunkCoordHash>& loaded) {
		// プレイヤーが「どのチャンクにいるか」を計算
		DirectX::XMFLOAT3 playerPos = scene.GetPlayer().GetPosition();
		int playerChunkX = WorldCoord::FloorDiv((int)floorf(playerPos.x), Chunk::CHUNK_SIZE);
		int playerChunkZ = WorldCoord::FloorDiv((int)floorf(playerPos.z), Chunk::CHUNK_SIZE);

		// RENDER_DISTANCE+2より遠いメッシュとデータを破棄
		std::vector<ChunkCoord> toUnload;
		for (auto& kv : loaded) {
			if ((std::max)(abs(kv.first.x - playerChunkX), abs(kv.first.z - playerChunkZ)) > RENDER_DISTANCE + 2) {
				toUnload.push_back(kv.first);
			}
		}
		if (!toUnload.empty()) {
			for (auto& chunkCoord : toUnload) {
				renderer.ReleaseChunkMesh(loaded[chunkCoord].meshIndex, gfx);
				world.RemoveChunk(chunkCoord);
				loaded.erase(chunkCoord);
				// 対応するSceneObjectを名前で探して外す
				auto& objects = scene.GetObjects();
				char name[32];
				sprintf_s(name, "Chunk %d_%d", chunkCoord.x, chunkCoord.z);
				for (size_t i = 0;i < objects.size(); ++i) {
					if (strcmp(objects[i].name, name) == 0) {
						objects.erase(objects.begin() + i);
						break;
					}
				}
			}
		}

		//  RENDER_DISTANCE+1の距離にあるチャンクのデータを確保
		for (int dataZ = -(RENDER_DISTANCE + 1); dataZ <= RENDER_DISTANCE + 1; ++dataZ) {
			for (int dataX = -(RENDER_DISTANCE + 1); dataX <= RENDER_DISTANCE + 1; ++dataX) {
				ChunkCoord coord{ playerChunkX + dataX, playerChunkZ + dataZ };
				if (!world.HasChunk(coord)) {
					world.GenerateChunk(coord.x, coord.z);
				}
			}
		}

		// RENDER_DISTANCE内のチャンクのメッシュを近い順にMAX_LOADS_PER_FRAMEだけ生成
		int made = 0;
		for (int distance = 0;distance <= RENDER_DISTANCE && made < MAX_LOADS_PER_FRAME; ++distance) {
			for (int dataZ = -distance; dataZ <= distance && made < MAX_LOADS_PER_FRAME; ++dataZ) {
				for (int dataX = -distance; dataX <= distance && made < MAX_LOADS_PER_FRAME; ++dataX) {
					if ((std::max)(abs(dataX), abs(dataZ)) != distance) {
						continue;
					}
					ChunkCoord coord{ playerChunkX + dataX, playerChunkZ + dataZ };
					if (loaded.find(coord) != loaded.end()) {
						continue;
					}

					Chunk* chunk = world.GetChunkPtr(coord);
					if (!chunk) {
						continue;
					}
					int meshIndex = renderer.CreateChunkMesh(gfx.GetDevice(), *chunk, world, coord.x, coord.z);
					if (meshIndex < 0) {
						continue;
					}

					SceneObject object;
					sprintf_s(object.name, "Chunk %d_%d", coord.x, coord.z);
					object.texturePath = "App/Textures/texture_atlas.png";
					object.position[0] = (float)(coord.x * Chunk::CHUNK_SIZE);
					object.position[1] = 0.0f;
					object.position[2] = (float)(coord.z * Chunk::CHUNK_SIZE);
					object.meshIndex = (uint32_t)meshIndex;
					object.textureIndex = renderer.LoadTexture(gfx.GetDevice(), gfx.GetCommandQueue(), object.texturePath);
					
					scene.GetObjects().push_back(object);

					loaded[coord] = { (uint32_t)meshIndex };
					++made;
				}
			}
		}
	}

	// 更新したチャンクの隣接するチャンクを更新
	void UpdateChunkNeighbors(int worldX, int worldY, int worldZ, World& world, Renderer& renderer, GfxDevice& gfxDevice, const std::unordered_map<ChunkCoord, LoadedChunk, ChunkCoordHash>& loaded) {
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

		//各チャンクを更新
		for (const auto& coord : toUpdate) {
			auto chunkIt = world.GetChunks().find(coord);
			auto meshIt = loaded.find(coord);

			// チャンクが存在しない or meshIndex未登録の場合描画をスキップ
			if (chunkIt == world.GetChunks().end()) {
				continue;
			}
			if (meshIt == loaded.end()) {
				continue;
			}

			renderer.UpdateChunkMeshDeferred(gfxDevice.GetDevice(), *chunkIt->second, world, coord.x, coord.z, meshIt->second.meshIndex, gfxDevice);
		}
	}

	ImU32 BlockTypeToColor(BlockType t) {
		switch (t) {
		case BlockType::Grass:
			return IM_COL32(110, 175, 70, 255);
		case BlockType::Dirt:
			return IM_COL32(140, 95, 55, 255);
		case BlockType::Stone:
			return IM_COL32(150, 150, 150, 255);
		default:
			return IM_COL32(0, 0, 0, 0);
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
	world.SetSeed(std::random_device{}());

	std::unordered_map<ChunkCoord, LoadedChunk, ChunkCoordHash> loaded;

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
	float accumulator = 0.0f;
	const float FIXED_DELTATIME = 1.0f / 120.0f;

	// メインループ
	while (window.ProcessMessage()){
		scene.Update();

		auto now = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;

		deltaTime = (std::min)(deltaTime, 0.25f);
		accumulator += deltaTime;

		// 入力
		const auto& mouse = window.GetMouseInput();
		const auto& keyboard = window.GetKeyboardInput();
		auto& fpsCam = scene.GetFPSCamera();
		auto& player = scene.GetPlayer();
		float yaw = fpsCam.GetYaw();

		// プレイヤー更新
		while (accumulator >= FIXED_DELTATIME) {
			player.Update(FIXED_DELTATIME, world,
				keyboard.w, keyboard.s, keyboard.a, keyboard.d,
				keyboard.space, yaw);
			accumulator -= FIXED_DELTATIME;
		}

		// カメラ位置をプレイヤーの目の位置に同期
		fpsCam.SetPosition(player.GetEyePosition());

		DirectX::XMFLOAT3 camPos = scene.GetFPSCamera().GetPosition();
		DirectX::XMFLOAT3 camDir = scene.GetFPSCamera().GetForward();

		RaycastResult ray = world.Raycast(camPos, camDir, 10.0f);

		// エスケープを押したときカーソル固定を解除
		if (keyboard.escapePressed) {
			window.SetMouseCaptured(!window.IsMouseCaptured());
		}

		// 数字キーでホットバー切り替え
		for (int i = 0;i < HOTBAR_SIZE;++i) {
			if (GetAsyncKeyState('1' + i) & 0x8000) {
				g_selectedSlot = i;
			}
		}

		// キャプチャ中のときのみ視点操作
		if (window.IsMouseCaptured()) {
			fpsCam.Rotate(static_cast<float>(mouse.deltaX), static_cast<float>(mouse.deltaY));

			if (mouse.leftClicked) {
				if (ray.hit) {
					world.SetBlockAt(ray.blockX, ray.blockY, ray.blockZ, BlockType::Air);
					// チャンクメッシュ更新
					UpdateChunkNeighbors(ray.blockX, ray.blockY, ray.blockZ, world, renderer, gfxDevice, loaded);
				}
			}
			if (mouse.rightClicked) {
				if (ray.hit) {
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

					// プレイヤーと重なる位置には設置しない
					if (!player.IntersectsBlock(ray.blockX, ray.blockY, ray.blockZ)) {
						world.SetBlockAt(ray.blockX, ray.blockY, ray.blockZ, g_hotbar[g_selectedSlot]);
						// チャンクメッシュ更新
						UpdateChunkNeighbors(ray.blockX, ray.blockY, ray.blockZ, world, renderer, gfxDevice, loaded);
					}
				}
			}
		}

		window.ResetMouseDelta();

		UpdateStreaming(world, renderer, gfxDevice, scene, loaded);

		gfxDevice.BeginFrame();

		renderer.FlushPendingUploads(gfxDevice.GetCommandList(), gfxDevice);

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

		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		ImVec2 screen = ImGui::GetIO().DisplaySize;

		// クロスヘア
		if (window.IsMouseCaptured()) {
			float captureX = screen.x * 0.5f;
			float captureY = screen.y * 0.5f;
			float arm = 8.0f;
			ImU32 cross = IM_COL32(255, 255, 255, 220);
			drawList->AddLine(ImVec2(captureX - arm, captureY), ImVec2(captureX + arm, captureY), cross, 2.0f);
			drawList->AddLine(ImVec2(captureX, captureY - arm), ImVec2(captureX, captureY + arm), cross, 2.0f);
		}

		// ホットバー
		const float SLOT = 56.0f;
		const float GAP = 4.0f;
		float totalWidth = HOTBAR_SIZE * SLOT + (HOTBAR_SIZE - 1) * GAP;
		float startX = (screen.x - totalWidth) * 0.5;
		float y = screen.y - SLOT - 24.0f;
		
		for (int i = 0; i < HOTBAR_SIZE; ++i) {
			float x = startX + i * (SLOT + GAP);
			ImVec2 a(x, y), b(x + SLOT, y + SLOT);

			// 背景
			drawList->AddRectFilled(a, b, IM_COL32(0, 0, 0, 170), 4.0f);

			// アイコン代わりのブロックの色
			ImU32 col = BlockTypeToColor(g_hotbar[i]);
			drawList->AddRectFilled(ImVec2(a.x + 8, a.y + 8), ImVec2(b.x - 8, b.y - 8), col, 3.0f);

			// 枠 選択中は太くなる
			bool select = (i == g_selectedSlot);
			drawList->AddRect(a, b, select ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, 255, 80), 4.0f, 0, select ? 3.0f : 1.0f);
			
			// 番号
			char num[4];
			sprintf_s(num, "%d", i + 1);
			drawList->AddText(ImVec2(a.x + 6, a.y + 2), IM_COL32(255, 255, 255, 200), num);
		}

		// デバッグウィンドウ
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
		ImGui::Text("Fog");
		ImGui::ColorEdit3("Fog Color", scene.GetFogColor());
		ImGui::SliderFloat("Fog Start", &scene.GetFogStart(), 0.0f, 200.0f);
		ImGui::SliderFloat("Fog End", &scene.GetFogEnd(), 0.0f, 200.0f);

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

		if (objects.size() < 64 && ImGui::Button("+ Add Object")) {
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
	gfxDevice.WaitForGPU();
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}