#include "DirectX12EnginePCH.h"
#include "Window.h"

BOOL Window::m_windowOpen = FALSE;


HRESULT Window::CreateError(const HRESULT& hr)
{
	const _com_error err(hr);
	return CreateError(err.ErrorMessage());
}

HRESULT Window::CreateError(const std::string& errormsg)
{
	return CreateError(std::wstring(errormsg.begin(), errormsg.end()));
}

HRESULT Window::CreateError(const std::wstring& errormsg)
{
	return CreateError(LPCWSTR(errormsg.c_str()));
}

HRESULT Window::CreateError(const LPCWSTR& errormsg)
{
	MessageBoxW(NULL, errormsg,
		L"Error", MB_OK | MB_ICONERROR);
	return E_FAIL;
}


Window* Window::GetInstance()
{
	m_windowOpen = TRUE;
	static Window window;
	return &window;
}

void Window::CloseWindow()
{
	m_windowOpen = FALSE;
	PostQuitMessage(0);
}

HRESULT Window::Create(HINSTANCE hInstance, const std::string& windowName, const UINT& width, const UINT& height,
	const BOOL& fullscreen)
{
	if (fullscreen)
	{
		HMONITOR hmonitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorinfo = { sizeof(monitorinfo) };
		GetMonitorInfo(hmonitor, &monitorinfo);

		this->m_width = monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left;
		this->m_height = monitorinfo.rcMonitor.bottom- monitorinfo.rcMonitor.top;
	}
	else
	{
		this->m_width = width;
		this->m_height = height;
	}

	this->m_windowTitle = LPCTSTR(windowName.c_str());
	this->m_fullscreen = fullscreen;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = StaticWndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = this->m_windowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (RegisterClassEx(&wc))
	{
		this->m_hwnd = CreateWindow(
			wc.lpszClassName,
			this->m_windowTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			this->m_width,
			this->m_height,
			nullptr,
			nullptr,
			hInstance,
			this);

		if (this->m_hwnd)
		{
			if (fullscreen)
			{
				SetWindowLong(this->m_hwnd, GWL_STYLE, 0);
			}

			ShowWindow(this->m_hwnd, 10);
			UpdateWindow(this->m_hwnd);
		}
		else
		{
			return CreateError(std::wstring(L"Error creating Window"));
		}
	}
	else
	{
		return CreateError(std::wstring(L"Error registering class"));
	}
	return S_OK;
}

const BOOL& Window::IsOpen() const
{
	return this->m_windowOpen;
}

BOOL Window::Updating()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		return TRUE;
	}
	return FALSE;
}

const UINT& Window::GetWidth() const
{
	return this->m_width;
}

const UINT& Window::GetHeight() const
{
	return this->m_height;
}

const BOOL& Window::GetFullscreen() const
{
	return this->m_fullscreen;
}

const HWND& Window::GetHWND() const
{
	return this->m_hwnd;
}

LRESULT Window::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* pParent;
	if (msg == WM_CREATE)
	{
		pParent = (Window*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pParent);
	}
	else
	{
		pParent = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (!pParent) return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	pParent->m_hwnd = hWnd;
	return pParent->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			DestroyWindow(this->m_hwnd);
		}
		Input::PressKey(wParam);		
		return 0;
	case WM_KEYUP:
		Input::UnPressKey(wParam);
		return 0;
		
	case WM_DESTROY:
		CloseWindow();
		return 0;
	}
	return DefWindowProc(this->m_hwnd,
		msg,
		wParam,
		lParam);
}
