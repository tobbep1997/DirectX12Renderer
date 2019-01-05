#pragma once
#include "../../Transform.h"
#include <DirectXMath.h>

class X12DepthStencil;
class X12RenderTargetView;

class ILight : public Transform
{
public:
	virtual ~ILight();

	void Queue();

	void SetIntensity(const float & intensity);
	const float & GetIntensity() const;
			
	void SetColor(const DirectX::XMFLOAT4 & color);
	void SetColor(const float & x, const float & y, const float & z, const float & w = 1.0f);
	const DirectX::XMFLOAT4 & GetColor() const;

	virtual const UINT & GetType() const = 0;

	virtual const UINT & GetNumRenderTargets() const = 0;

	X12DepthStencil * GetDepthStencil() const;
	X12RenderTargetView * GetRenderTargetView() const;

protected:
	ILight(RenderingManager * renderingManager, const Window & window);
	RenderingManager * p_renderingManager;
	const Window * p_window;

	UINT p_lightType = 0;
	X12DepthStencil * p_depthStencil = nullptr;
	X12RenderTargetView * p_renderTarget = nullptr;

	UINT p_renderTargets = 1;
private:
	DirectX::XMFLOAT4 m_color = DirectX::XMFLOAT4(1, 1, 1, 1);
	float m_intensity = 1;
	
};

