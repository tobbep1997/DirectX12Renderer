#pragma once
#include "DirectX/Objects/Camera.h"
#include "../IManagedObject.h"
#include "DXTransform.h"

namespace ID3D12
{
	public ref class DXCamera : public DXTransform
	{
	public:
		DXCamera();
		DXCamera(float fov);
		DXCamera(float fov, float aspectRatio);
		DXCamera(float fov, float aspectRatio, float nearPlane);
		DXCamera(float fov, float aspectRatio, float nearPlane, float farPlane);
		DXCamera(float fov, float aspectRatio, float nearPlane, float farPlane, bool perspective);
	
		void SetDirection(DXMath::DXVector^ direction);
		void SetFocusPoint(DXMath::DXVector^ point);

		DXMath::DXVector^ GetDirection();
		DXMath::DXVector^ GetUp();

		void Rotate(DXMath::DXVector^ rotation);
		void Translate(DXMath::DXVector^ translation);
		void SetUp(DXMath::DXVector^ up);

		property float Fov
		{
		public:
			float get()
			{
				return GetInstance<Camera>()->GetFov();
			}
			void set(float value)
			{
				GetInstance<Camera>()->SetFov(value);
			}
		}

		property float AspectRatio
		{
		public:
			float get()
			{
				return GetInstance<Camera>()->GetAspectRatio();
			}
			void set(float value)
			{
				GetInstance<Camera>()->SetAspectRatio(value);
			}
		}

		property float NearPlane
		{
		public:
			float get()
			{
				return GetInstance<Camera>()->GetNearPlane();
			}
			void set(float value)
			{
				GetInstance<Camera>()->SetNearPlane(value);
			}
		}

		property float FarPlane
		{
		public:
			float get()
			{
				return GetInstance<Camera>()->GetFarPlane();
			}
			void set(float value)
			{
				GetInstance<Camera>()->SetFarPlane(value);
			}
		}
	};
}
