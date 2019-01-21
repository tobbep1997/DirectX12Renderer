#pragma once
#include "../IManagedObject.h"
#include "DirectX/Objects/Light/Template/ILight.h"
#include "../Math/DXVector.h"

namespace ID3D12
{

	public ref class DXILight : public IManagedObject<void>
	{
	private:
		ILight * m_light;
	public:
		DXILight(void * light);

		void SetColor(DXMath::DXVector^ color);
		DXMath::DXVector^ GetColor();

		void Queue();

		property bool CastShadows
		{
		public:
			bool get()
			{
				return m_light->GetCastShadows();
			}
			void set(bool value)
			{
				m_light->SetCastShadows(value);
			}
		}

		property float Intensity
		{
		public:
			float get()
			{
				return m_light->GetIntensity();
			}
			void set(float value)
			{
				m_light->SetIntensity(value);
			}
		}
		template <typename T>
		T* GetInstance()
		{
			return reinterpret_cast<T*>(p_instance);
		}
	};

}
