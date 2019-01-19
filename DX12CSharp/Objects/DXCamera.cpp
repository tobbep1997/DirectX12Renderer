#include "DXCamera.h"

namespace ID3D12 
{

	DXCamera::DXCamera() 
		: DXTransform(new Camera())
	{
	}

	DXCamera::DXCamera(float fov)
		: DXTransform(new Camera(fov))
	{
	}

	DXCamera::DXCamera(float fov, float aspectRatio)
		: DXTransform(new Camera(fov, aspectRatio))
	{
	}

	DXCamera::DXCamera(float fov, float aspectRatio, float nearPlane)
		: DXTransform(new Camera(fov, aspectRatio, nearPlane))
	{
	}

	DXCamera::DXCamera(float fov, float aspectRatio, float nearPlane, float farPlane)
		: DXTransform(new Camera(fov, aspectRatio, nearPlane, farPlane))
	{
	}

	DXCamera::DXCamera(float fov, float aspectRatio, float nearPlane, float farPlane, bool perspective)
		: DXTransform(new Camera(fov, aspectRatio, nearPlane, farPlane, perspective))
	{
	}

	void DXCamera::SetDirection(DXMath::DXVector^ direction)
	{
		GetInstance<Camera>()->SetDirection(*direction->GetInstance());
	}

	void DXCamera::SetFocusPoint(DXMath::DXVector^ point)
	{
		GetInstance<Camera>()->SetFocusPoint(*point->GetInstance());
	}

	DXMath::DXVector^ DXCamera::GetDirection()
	{
		return gcnew DXMath::DXVector(GetInstance<Camera>()->GetDirection());
	}

	DXMath::DXVector^ DXCamera::GetUp()
	{
		return gcnew DXMath::DXVector(GetInstance<Camera>()->GetUp());
	}

	void DXCamera::Rotate(DXMath::DXVector^ rotation)
	{
		GetInstance<Camera>()->Rotate(*rotation->GetInstance());
	}

	void DXCamera::Translate(DXMath::DXVector^ translation)
	{
		GetInstance<Camera>()->Translate(*translation->GetInstance());
	}

	void DXCamera::SetUp(DXMath::DXVector^ up)
	{
		GetInstance<Camera>()->SetUp(*up->GetInstance());
	}
}
