#include "Engine/Core/Window.h"
#include "Engine/Graphics/GfxDevice.h"
#include "App/Scene.h"

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

	// メインループ
	while (window.ProcessMessage()){
		scene.Update();
		gfxDevice.BeginFrame();
		gfxDevice.EndFrame();
	}
	return 0;
}