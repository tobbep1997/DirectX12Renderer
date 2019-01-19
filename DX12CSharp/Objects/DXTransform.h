#pragma once
#include "DirectX/Objects/Transform.h"
#include "../Math/DXVector.h"

namespace ID3D12
{
	
	public ref class DXTransform : public IManagedObject<void>
	{
	private:
		Transform * m_transform;
	public:
		DXTransform(void * transform);

		void SetPosition(DXMath::DXVector^ position);
		void Translate(DXMath::DXVector^ position);

		void SetRotation(DXMath::DXVector^ rotation);
		void SetScale(DXMath::DXVector^ scale);

		DXMath::DXVector^ GetPosition();
		DXMath::DXVector^ GetRotation();
		DXMath::DXVector^ GetScale();

		template <typename T>
		T* GetInstance()
		{
			return reinterpret_cast<T*>(p_instance);
		}

	};

}