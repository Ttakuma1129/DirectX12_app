#pragma once
#include <Windows.h>
#include <cstdint>

struct MouseInput {
	int deltaX = 0; // フレーム内の移動量
	int deltaY = 0;
	int wheelDelta = 0;
	bool leftDown = false;
	bool middleDown = false;
};

class Window {
public:
	// コンストラクタ
	Window(uint32_t width, uint32_t height, const wchar_t* title);
	// デストラクタ
	~Window();

	// メッセージループの処理
	bool ProcessMessage();

	// マウス入力を返す
	const MouseInput& GetMouseInput() const {
		return m_mouse;
	}

	void ResetMouseDelta();

	// DirectXの初期化に必要なハンドル
	HWND GetHwnd() const {
		return m_hwnd;
	}
	uint32_t GetWidth() const {
		return m_width;
	}
	uint32_t GetHeight() const {
		return m_height;
	}

private:
	// OSからのメッセージの処理(ウィンドウプロシージャ)
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	HWND m_hwnd = nullptr; // ウィンドウハンドル
	WNDCLASSEX m_wndClass = {}; // ウィンドウクラス情報
	uint32_t m_width = 0;
	uint32_t m_height = 0;

	MouseInput m_mouse;
	int m_lastMouseX = 0;
	int m_lastMouseY = 0;

	static Window* s_instance;
};