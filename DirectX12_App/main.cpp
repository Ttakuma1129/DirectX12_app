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

	// スカイボックス初期化
	std::string skyFaces[6] = {
		"App/Texture/skybox/sh_back.png"
		"App/Texture/skybox/sh_bottom.png"
		"App/Texture/skybox/sh_front.png"
		"App/Texture/skybox/sh_left.png"
		"App/Texture/skybox/sh_right.png"
		"App/Texture/skybox/sh_top.png"
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
		ImGui::SliderFloat("FOV", &scene.GetFov(), 10.0f, 120.0f);

		ImGui::Separator();
		ImGui::Text("Camera Position");
		ImGui::SliderFloat("X", &scene.GetCameraPos()[0], -10.0f, 10.0f);
		ImGui::SliderFloat("Y", &scene.GetCameraPos()[1], -10.0f, 10.0f);
		ImGui::SliderFloat("Z", &scene.GetCameraPos()[2], -10.0f, -0.5f);

		ImGui::Separator();
		ImGui::Text("Object Scale");
		ImGui::SliderFloat("Model Scale", &scene.GetModelScale(), 0.1f, 20.0f);

		ImGui::Separator();
		ImGui::Text("Lighting");
		ImGui::SliderFloat3("Light Direction", scene.GetLightDir(), -1.0f, 1.0f);
		ImGui::SliderFloat("Specular", &scene.GetSapcIntensity(), 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &scene.GetSpecShiciness(), 1.0f, 256.0f);
		ImGui::ColorEdit3("Light Color", scene.GetLightColor());
		ImGui::ColorEdit3("Ambient", scene.GetAmbientColor());

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

				if (ImGui::Button("Delete") && objects.size() > 1) {
					objects.erase(objects.begin() + 1);
					ImGui::TreePop();
					ImGui::PopID();
					break;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		if (objects.size() < 16 && ImGui::Button("+ Add Object")) {
			SceneObject newObj;
			sprintf_s(newObj.name, "Object %d", (int)objects.size());
			newObj.meshIndex = 0;
			newObj.scale = 0.1f;
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