#pragma once
#include <Windows.h>
#include <cstdint>

class Window {
public:
	// コンストラクタ
	Window(uint32_t width, uint32_t height, const wchar_t* title);
	// デストラクタ
	~Window();

	// メッセージループの処理
	bool ProcessMessage();

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
};