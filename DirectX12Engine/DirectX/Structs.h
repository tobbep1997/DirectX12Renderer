#pragma once
#include <DirectXMath.h>
struct StaticVertex
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Normal;
	DirectX::XMFLOAT4 Tangent;
	DirectX::XMFLOAT4 TexCord;
};