#include "DirectX12EnginePCH.h"
#include "Window.h"

BOOL Window::m_windowOpen = FALSE;

Window * Window::thisWindow = nullptr;

HRESULT Window::CreateError(const HRESULT& hr)
{
	const _com_error err(hr);
	return CreateError(err.ErrorMessage());
}

HRESULT Window::CreateError(const std::string& errorMsg)
{
	return CreateError(std::wstring(errorMsg.begin(), errorMsg.end()));
}

HRESULT Window::CreateError(const std::wstring& errorMsg)
{
	return CreateError(LPCWSTR(errorMsg.c_str()));
}

HRESULT Window::CreateError(const LPCWSTR& errorMsg)
{
	MessageBoxW(nullptr, errorMsg,
		L"Error", MB_OK | MB_ICONERROR);
	return E_FAIL;
}


Window* Window::GetInstance()
{
	m_windowOpen = TRUE;
	static Window window;
	return &window;
}

Window* Window::GetPointerInstance()
{
	m_windowOpen = TRUE;
	if (!thisWindow)
		thisWindow = new Window();
	return thisWindow;
}

void Window::DeletePointerInstance()
{
	delete thisWindow;
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
		const HMONITOR hmonitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);  // NOLINT
		MONITORINFO monitorInfo = { sizeof(monitorInfo) };// NOLINT
		GetMonitorInfo(hmonitor, &monitorInfo);

		this->m_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
		this->m_height = monitorInfo.rcMonitor.bottom- monitorInfo.rcMonitor.top;
	}
	else
	{
		this->m_width = width;
		this->m_height = height;
	}

	this->m_windowTitle = LPCTSTR(windowName.c_str());
	this->m_fullscreen = fullscreen;
	WNDCLASSEX wc;

	m_windowName = "WNDCLASS";

	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = StaticWndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 2);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = this->m_windowName;
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

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

HRESULT Window::Create(void* hInstance, const std::string& windowName, const UINT& width, const UINT& height,
	const BOOL& fullscreen)
{
	return this->Create((HINSTANCE)hInstance, windowName, width, height, fullscreen);
}

const BOOL& Window::IsOpen()
{
	return m_windowOpen;
}

BOOL Window::Updating()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
		pParent = static_cast<Window*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pParent));
	}
	else
	{
		pParent = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!pParent) return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	pParent->m_hwnd = hWnd;
	return pParent->_wndProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::_wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) const
{
	switch (msg)
	{

	case WM_KEYDOWN:
		Input::PressKey(static_cast<UINT>(wParam));		
		return 0;
	case WM_KEYUP:
		Input::UnPressKey(static_cast<UINT>(wParam));
		return 0;
		
	case WM_DESTROY:
		CloseWindow();
		return 0;

	default: 
		break;
	}
	return DefWindowProc(this->m_hwnd,
		msg,
		wParam,
		lParam);
}
