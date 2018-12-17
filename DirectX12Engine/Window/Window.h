#pragma once
#include "DirectX12EnginePCH.h"
#include <string>

class Window
{
public:
	static HRESULT CreateError(const HRESULT & hr);
	static HRESULT CreateError(const std::string & errormsg);
	static HRESULT CreateError(const std::wstring & errormsg);
	static HRESULT CreateError(const LPCWSTR & errormsg);
	static Window * GetInstance();
	void CloseWindow();

	HRESULT Create(HINSTANCE hInstance, 
				const std::string & windowName, 
				const UINT & width, 
				const UINT & height,
				const BOOL & fullscreen = FALSE);
	const BOOL & IsOpen() const;
	BOOL Updating();

	const UINT & GetWidth() const;
	const UINT & GetHeight() const;
	const BOOL & GetFullscreen() const;
	const HWND & GetHWND() const;

private:
	HWND m_hwnd = nullptr;

	LPCTSTR m_windowTitle = "";
	LPCTSTR m_windowName = "WNDCLASS";

	UINT m_width = 0;
	UINT m_height = 0;
	
	BOOL m_fullscreen;
	static BOOL m_windowOpen;

	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

};
