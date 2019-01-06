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

	const Camera *const* GetCameras() const;

	void Init() override;
	void Update() override;
	void Release() override;
private:
	float m_dropOff;
	float m_pow;

	Camera * m_cameras[6];
};

