#pragma once

namespace ID3D12
{
	enum Keys
	{
		UpArrow = 38,
		DownArrow = 40,
		LeftArrow = 37,
		RightArrow = 39
	};
	

public ref class DXInput abstract sealed
{
public:
	static bool IsKeyPressed(System::Char ^ key);
	static bool IsKeyPressed(unsigned int key);
};

}
