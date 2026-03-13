#include "Window.h"

// コンストラクタ
Window::Window(uint32_t width, uint32_t height, const wchar_t* title) : m_width(width), m_height(height) {

	m_wndClass.cbSize = sizeof(WNDCLASSEX);
	m_wndClass.style = CS_HREDRAW | CS_VREDRAW;
	m_wndClass.lpfnWndProc = WindowProc;
	m_wndClass.hInstance = GetModuleHandle(nullptr);
	m_wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	m_wndClass.lpszClassName = L"DX12GameWindwClass";

	RegisterClassEx(&m_wndClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	m_hwnd = CreateWindow(
		m_wndClass.lpszClassName,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		m_wndClass.hInstance,
		nullptr
	);

	ShowWindow(m_hwnd, SW_SHOW);
}

// デストラクタ
Window::~Window() {
	UnregisterClass(m_wndClass.lpszClassName, m_wndClass.hInstance);
}

// OSからのメッセージを処理する
bool Window::ProcessMessage() {
	MSG msg = {};

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY: // ウィンドウが閉じられたとき
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}