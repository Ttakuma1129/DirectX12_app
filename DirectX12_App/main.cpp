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