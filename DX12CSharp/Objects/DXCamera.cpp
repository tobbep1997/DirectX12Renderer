#include "DXCamera.h"

namespace ID3D12 
{

	DXCamera::DXCamera() : IManagedObject(new Camera(), true)
	{
	}
}