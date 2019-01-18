#pragma once
#include <Windows.h>
#include <string>

class Window
{
public:
	static HRESULT CreateError(const HRESULT & hr);
	static HRESULT CreateError(const std::string & errorMsg);
	static HRESULT CreateError(const std::wstring & errorMsg);
	static HRESULT CreateError(const LPCWSTR & errorMsg);
	static Window * GetInstance();
	static void CloseWindow();

	HRESULT Create(HINSTANCE hInstance, 
				const std::string & windowName, 
				const UINT & width, 
				const UINT & height,
				const BOOL & fullscreen = FALSE);

	HRESULT Create(void* hInstance,
		const std::string & windowName,
		const UINT & width,
		const UINT & height,
		const BOOL & fullscreen = FALSE);

	static const BOOL & IsOpen();
	static BOOL Updating();

	const UINT & GetWidth() const;
	const UINT & GetHeight() const;
	const BOOL & GetFullscreen() const;
	const HWND & GetHWND() const;

private:
	HWND m_hwnd = nullptr;

	LPCTSTR m_windowTitle;
	LPCTSTR m_windowName;

	UINT m_width = 0;
	UINT m_height = 0;
	
	BOOL m_fullscreen = FALSE;
	static BOOL m_windowOpen;

	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT _wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) const;

};
