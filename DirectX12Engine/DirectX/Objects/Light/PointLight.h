#pragma once
#include "Template/ILight.h"
class PointLight :
	public ILight
{

public:
	PointLight(RenderingManager * renderingManager, const Window & window);
	~PointLight();

	void SetDropOff(const float & dropOff);
	void SetPow(const float & pow);

	const float & GetDropOff() const;
	const float & GetPow() const;

	const UINT & GetType() const override;
	const UINT & GetNumRenderTargets() const override;
private:
	float m_dropOff;
	float m_pow;
};

