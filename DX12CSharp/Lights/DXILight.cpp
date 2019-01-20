#include "DXILight.h"

namespace ID3D12
{
	DXILight::DXILight(void* light)
		: IManagedObject(light, true)
	{
		m_light = static_cast<ILight*>(light);
	}

	void DXILight::SetColor(DXMath::DXVector^ color)
	{
		m_light->SetColor(*color->GetInstance());
	}

	DXMath::DXVector^ DXILight::GetColor()
	{
		return gcnew DXMath::DXVector(m_light->GetColor());
	}

	void DXILight::Queue()
	{
		m_light->Queue();
	}
}
