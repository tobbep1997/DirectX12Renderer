#pragma once
#include "../../Transform.h"
#include <DirectXMath.h>

class X12DepthStencil;
class X12RenderTargetView;

class ILight : public Transform
{
protected:
	enum LightType
	{
		Point = 0,
		Directional = 1
	};
public:
	virtual ~ILight();
	void Queue();

	void SetIntensity(const float & intensity);
	const float & GetIntensity() const;
			
	void SetColor(const DirectX::XMFLOAT4 & color);
	void SetColor(const float & x, const float & y, const float & z, const float & w = 1.0f);
	const DirectX::XMFLOAT4 & GetColor() const;

	void SetCastShadows(const BOOL & castShadows);
	const BOOL & GetCastShadows() const;

	virtual const UINT & GetType() const;
	virtual const UINT & GetNumRenderTargets() const;

	X12DepthStencil * GetDepthStencil() const;
	X12RenderTargetView * GetRenderTargetView() const;

protected:
	ILight(RenderingManager * renderingManager, const Window & window, const LightType & lightType);
	RenderingManager * p_renderingManager;
	const Window * p_window;

	UINT p_renderTargets = 1;
	LightType p_lightType = LightType::Point;
	X12DepthStencil * p_depthStencil = nullptr;
	X12RenderTargetView * p_renderTarget = nullptr;

	BOOL p_createDirectXContext(const UINT & renderTargets = 1, const BOOL & createTexture = FALSE);
	
	BOOL Init() override;
	void Update() override;
	void Release() override;
	
private:
	DirectX::XMFLOAT4 m_color = DirectX::XMFLOAT4(1, 1, 1, 1);
	
	UINT m_lightTypeInteger;
	float m_intensity = 1;
	BOOL m_castShadows = TRUE;
};

