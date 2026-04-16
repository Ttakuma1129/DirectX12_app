#include <string>

#include "Engine/Core/Window.h"
#include "Engine/Graphics/GfxDevice.h"
#include "App/Scene.h"
#include "Engine/Graphics/Renderer.h"

// ImGui
#include "Engine/ThirdParty/imgui/imgui.h"
#include "Engine/ThirdParty/imgui/backends/imgui_impl_dx12.h"
#include "Engine/ThirdParty/imgui/backends/imgui_impl_win32.h"

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

	// メインループ
	while (window.ProcessMessage()){
		scene.Update();

		const auto& mouse = window.GetMouseInput();

		// ImGuiがマウスを使ってないときだけカメラ操作
		if (!ImGui::GetIO().WantCaptureMouse) {
			if (mouse.leftDown) {
				scene.GetCamera().Rotate(
					static_cast<float>(mouse.deltaX),
					static_cast<float>(mouse.deltaY));
			}
			if (mouse.middleDown) {
				scene.GetCamera().Pan(
					static_cast<float>(mouse.deltaX),
					static_cast<float>(mouse.deltaY));
			}
			if (mouse.wheelDelta != 0) {
				scene.GetCamera().Zoom(
					static_cast<float>(mouse.wheelDelta) / 120.0f);
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
		ImGui::SliderFloat("FOV", &scene.GetCamera().GetFov(), 10.0f, 120.0f);

		ImGui::Separator();
		ImGui::Text("Lighting");
		ImGui::SliderFloat3("Light Direction", scene.GetLightDir(), -1.0f, 1.0f);
		ImGui::SliderFloat("Specular", &scene.GetSapcIntensity(), 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &scene.GetSpecShiciness(), 1.0f, 256.0f);
		ImGui::ColorEdit3("Light Color", scene.GetLightColor());
		ImGui::ColorEdit3("Ambient", scene.GetAmbientColor());
		ImGui::SliderFloat("Shadow Bias", &scene.GetShadowBias(), 0.0f, 0.01f, "%.5f");


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