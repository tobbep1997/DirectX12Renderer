#pragma once
#include "Template/ILight.h"

class X12DepthStencil;
class X12RenderTargetView;

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

	const Camera *const* GetCameras() const;

	void Init() override;
	void Update() override;
	void Release() override;

	X12DepthStencil *const* GetDepthStencil() const;
	X12RenderTargetView *const* GetRenderTargetView() const;

private:
	float m_dropOff;
	float m_pow;

	Camera * m_cameras[6];

	X12DepthStencil * m_depthStencils[6];
	X12RenderTargetView * m_renderTargetViews[6];
};

