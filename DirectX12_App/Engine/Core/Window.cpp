#include <windowsx.h>

#include "Window.h"
#include "../ThirdParty/imgui/imgui.h"
#include "../ThirdParty/imgui/backends/imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window* Window::s_instance = nullptr;

// コンストラクタ
Window::Window(uint32_t width, uint32_t height, const wchar_t* title) : m_width(width), m_height(height) {
	s_instance = this;

	m_wndClass.cbSize = sizeof(WNDCLASSEX);
	m_wndClass.style = CS_HREDRAW | CS_VREDRAW;
	m_wndClass.lpfnWndProc = WindowProc;
	m_wndClass.hInstance = GetModuleHandle(nullptr);
	m_wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	m_wndClass.lpszClassName = L"DX12GameWindowClass";

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

void Window::ResetMouseDelta() {
	m_mouse.deltaX = 0;
	m_mouse.deltaY = 0;
	m_mouse.wheelDelta = 0;
	m_mouse.leftClicked = false;
	m_mouse.rightClicked = false;

	// キャプチャー中はカーソルを中央に戻す
	if (m_mouseCaptured) {
		RECT rect;
		GetClientRect(m_hwnd, &rect);

		int cx = (rect.right - rect.left) / 2;
		int cy = (rect.bottom - rect.top) / 2;
		m_lastMouseX = cx;
		m_lastMouseY = cy;

		POINT center = { cx, cy };
		ClientToScreen(m_hwnd, &center);
		SetCursorPos(center.x, center.y);
	}

}

void Window::SetMouseCaptured(bool capture) {
	if (m_mouseCaptured == capture) {
		return;
	}
	m_mouseCaptured = capture;

	if (capture) {
		ShowCursor(FALSE);

		// ウィンドウ中央にカーソルを移動
		RECT rect;
		GetClientRect(m_hwnd, &rect);
		int cx = (rect.right - rect.left) / 2;
		int cy = (rect.bottom - rect.top) / 2;
		m_lastMouseX = cx;
		m_lastMouseY = cy;

		POINT center = { cx, cy };
		ClientToScreen(m_hwnd, &center);
		SetCursorPos(center.x, center.y);
	}
	else {
		ShowCursor(TRUE);
	}
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// Imguiのメッセージを転送
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	switch (msg) {
		case WM_DESTROY: // ウィンドウが閉じられたとき
			PostQuitMessage(0);
			return 0;
	
		case WM_LBUTTONDOWN:
			s_instance->m_mouse.leftDown = true;
			s_instance->m_mouse.leftClicked = true;
			s_instance->m_lastMouseX = GET_X_LPARAM(lparam);
			s_instance->m_lastMouseY = GET_Y_LPARAM(lparam);
			SetCapture(hwnd);
			return 0;

		case WM_LBUTTONUP:
			s_instance->m_mouse.leftDown = false;
			ReleaseCapture();
			return 0;

		case WM_RBUTTONDOWN:
			s_instance->m_mouse.rightDown = true;
			s_instance->m_mouse.rightClicked = true;
			return 0;

		case WM_RBUTTONUP:
			s_instance->m_mouse.rightDown = false;
			return 0;
	
		case WM_MBUTTONDOWN:
			s_instance->m_mouse.middleDown = true;
			s_instance->m_lastMouseX = GET_X_LPARAM(lparam);
			s_instance->m_lastMouseY = GET_Y_LPARAM(lparam);
			SetCapture(hwnd);
			return 0;
	
		case WM_MBUTTONUP:
			s_instance->m_mouse.middleDown = false;
			ReleaseCapture();
			return 0;
	
		case WM_MOUSEMOVE: {
			int x = GET_X_LPARAM(lparam);
			int y = GET_Y_LPARAM(lparam);
			s_instance->m_mouse.deltaX += x - s_instance->m_lastMouseX;
			s_instance->m_mouse.deltaY += y - s_instance->m_lastMouseY;
			s_instance->m_lastMouseX = x;
			s_instance->m_lastMouseY = y;
			return 0;
		}
	
		case WM_MOUSEWHEEL:
			s_instance->m_mouse.wheelDelta += GET_WHEEL_DELTA_WPARAM(wparam);
			return 0;
	
		case WM_KEYDOWN: {
			switch (wparam) {
			case 'W':
				s_instance->m_keyboard.w = true;
				break;
			case 'A':
				s_instance->m_keyboard.a = true;
				break;
			case 'S':
				s_instance->m_keyboard.s = true;
				break;
			case 'D':
				s_instance->m_keyboard.d = true;
				break;
			case VK_SPACE: 
				s_instance->m_keyboard.space = true;
				break;
			case VK_SHIFT:
				s_instance->m_keyboard.shift = true;
			}
			return 0;
		}
		case WM_KEYUP: {
			switch (wparam){
			case 'W':
				s_instance->m_keyboard.w = false;
				break;
			case 'A':
				s_instance->m_keyboard.a = false;
				break;
			case 'S':
				s_instance->m_keyboard.s = false;
				break;
			case 'D':
				s_instance->m_keyboard.d = false;
				break;
			case VK_SPACE:
				s_instance->m_keyboard.space = false;
				break;
			case VK_SHIFT:
				s_instance->m_keyboard.shift = false;
			}
			return 0;
		}

	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}