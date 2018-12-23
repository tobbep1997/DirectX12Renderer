#pragma once
class Input
{
private:
	static BOOL m_keys[256];
public:

	enum Keys
	{
		UpArrow = 38,
		DownArrow = 40,
		LeftArrow = 37,
		RightArrow = 39
	};

	Input();
	~Input();

	static void PressKey(const UINT & key);
	static void UnPressKey(const UINT & key);
	
	static BOOL IsKeyPressed(const UINT & key);
};

