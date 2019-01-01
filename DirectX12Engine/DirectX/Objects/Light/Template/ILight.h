#pragma once
#include "../../Transform.h"
#include <DirectXMath.h>

class ILight : public Transform
{
public:
	virtual ~ILight();

	void Queue(RenderingManager * renderingManager);

	void SetIntensity(const float & intensity);
	const float & GetIntensity() const;
			
	void SetColor(const DirectX::XMFLOAT4 & color);
	void SetColor(const float & x, const float & y, const float & z, const float & w = 1.0f);
	const DirectX::XMFLOAT4 & GetColor() const;

	virtual const UINT & GetType() const = 0;

protected:
	ILight();
	UINT p_lightType = 0;
private:
	DirectX::XMFLOAT4 m_color = DirectX::XMFLOAT4(1, 1, 1, 1);
	float m_intensity = 1;
	
};

