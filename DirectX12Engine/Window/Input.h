#pragma once
class Input
{
private:
	static BOOL m_keys[256];
public:
	Input();
	~Input();

	static void PressKey(const UINT & key);
	static void UnPressKey(const UINT & key);
	
	static BOOL IsKeyPressed(const UINT & key);
};

