#pragma once
#include <Windows.h>
#include <string>


class Window
{
public:
	static HRESULT CreateError(const std::wstring & errormsg);
	static Window * GetInstance();

	HRESULT Create(HINSTANCE hInstance, 
				const std::string & windowName, 
				const UINT & width, 
				const UINT & height,
				const BOOL & fullscreen = FALSE);
	const BOOL & IsOpen() const;
	BOOL Updating();

private:
	HWND m_hwnd = nullptr;

	LPCTSTR m_windowTitle = "";
	LPCTSTR m_windowName = "WNDCLASS";

	UINT m_width = 0;
	UINT m_height = 0;

	BOOL m_windowOpen = TRUE;

	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void _closeWindow();
};
