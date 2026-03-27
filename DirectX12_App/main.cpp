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
	ImGui_ImplDX12_Init(
		gfxDevice.GetDevice(),
		2,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		renderer.GetImGuiSrvHeap().GetHeap(),
		renderer.GetImGuiSrvHeap().GetCPUHandle(0),
		renderer.GetImGuiSrvHeap().GetGPUHandle(0));

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

		gfxDevice.EndFrame();
	}

	// ImGuiシャットダウン
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}