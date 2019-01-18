#include "DXVector.h"



DXMath::DXVector::DXVector() 
	: IManagedObject(new DirectX::XMFLOAT4(), true)
{

}

DXMath::DXVector::DXVector(float x, float y, float z, float w) 
	: IManagedObject(new DirectX::XMFLOAT4(x, y, z, w), true)
{
}

DXMath::DXVector::DXVector(const DirectX::XMFLOAT4 & xmfloat4)
	: IManagedObject(new DirectX::XMFLOAT4(
			xmfloat4.x, 
			xmfloat4.y, 
			xmfloat4.z, 
			xmfloat4.w), 
		true)

{
}
