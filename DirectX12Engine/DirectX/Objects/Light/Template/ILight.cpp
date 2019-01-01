#include "DirectX12EnginePCH.h"
#include "ILight.h"
#include "DirectX/Render/Template/IRender.h"


ILight::ILight()
{
	this->m_intensity = 1;
}
ILight::~ILight()
= default;

void ILight::Queue(RenderingManager* renderingManager)
{
	if (m_intensity > 0)
	{
		reinterpret_cast<IRender*>(renderingManager->GetGeometryPass())->QueueLight(this);
		reinterpret_cast<IRender*>(renderingManager->GetShadowPass())->QueueLight(this);
	}
}

void ILight::SetIntensity(const float& intensity)
{
	this->m_intensity = intensity;
}

const float& ILight::GetIntensity() const
{
	return this->m_intensity;
}

void ILight::SetColor(const DirectX::XMFLOAT4& color)
{
	this->m_color = color;
}

void ILight::SetColor(const float& x, const float& y, const float& z, const float& w)
{
	this->SetColor(DirectX::XMFLOAT4(x, y, z, w));
}

const DirectX::XMFLOAT4 & ILight::GetColor() const
{
	return this->m_color;
}

