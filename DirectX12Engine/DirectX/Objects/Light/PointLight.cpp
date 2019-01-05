#include "DirectX12EnginePCH.h"
#include "PointLight.h"

PointLight::PointLight(RenderingManager* renderingManager, const Window& window)
	: ILight(renderingManager, window)
{
	p_lightType = 0U;

	m_dropOff = 1.0f;
	m_pow = 2.0f;

	this->p_renderTargets = 6u;
}

PointLight::~PointLight()
{
}

void PointLight::SetDropOff(const float& dropOff)
{
	this->m_dropOff = dropOff;
}

void PointLight::SetPow(const float& pow)
{
	this->m_pow = pow;
}

const float& PointLight::GetDropOff() const
{
	return this->m_dropOff;
}

const float& PointLight::GetPow() const
{
	return this->m_pow;
}

const UINT & PointLight::GetType() const
{
	return p_lightType;
}

const UINT& PointLight::GetNumRenderTargets() const
{
	return this->p_renderTargets;
}
