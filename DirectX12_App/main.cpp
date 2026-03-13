#include "Engine/Core/Window.h"
#include "Engine/Graphics/GfxDevice.h"

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

	// メインループ
	while (window.ProcessMessage()){
		gfxDevice.BeginFrame();
		gfxDevice.EndFrame();
	}
	return 0;
}