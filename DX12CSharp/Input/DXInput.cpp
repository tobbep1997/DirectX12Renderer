#include "DXInput.h"
#include "Window/Input.h"


bool ID3D12::DXInput::IsKeyPressed(System::Char^ key)
{
	const wchar_t k = static_cast<wchar_t>(key);
	if (k < 0 || k >= 256)
		return false;
	return Input::IsKeyPressed(static_cast<char>(k));
}

bool ID3D12::DXInput::IsKeyPressed(unsigned key)
{
	if (key < 0 || key >= 256)
		return false;
	return Input::IsKeyPressed(key);
}
