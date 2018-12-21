#include "DirectX12EnginePCH.h"
#include "Input.h"

BOOL Input::m_keys[256];

Input::Input()
{
}


Input::~Input()
{
}

void Input::PressKey(const UINT& key)
{
	if (key < 256)
		m_keys[key] = TRUE;
	else
		throw std::exception("Index out of range");
}

void Input::UnPressKey(const UINT& key)
{
	if (key < 256)
		m_keys[key] = FALSE;
	else
		throw std::exception("Index out of range");
}

BOOL Input::IsKeyPressed(const UINT& key)
{
	if (key < 256)
		return m_keys[key];
	else
		throw std::exception("Index out of range");
}
