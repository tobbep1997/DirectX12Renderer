#pragma once
#include "Template/ILight.h"
class PointLight :
	public ILight
{
public:
	PointLight();
	~PointLight();

	void SetDropOff(const float & dropOff);
	void SetPow(const float & pow);

	const float & GetDropOff() const;
	const float & GetPow() const;

private:
	float m_dropOff;
	float m_pow;
};

