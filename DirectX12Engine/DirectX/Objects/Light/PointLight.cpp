#include "DirectX12EnginePCH.h"
#include "PointLight.h"

PointLight::PointLight(RenderingManager* renderingManager, const Window& window)
	: ILight(renderingManager, window, ILight::LightType::Point)
{
	m_dropOff = 1.0f;
	m_pow = 2.0f;

	this->p_renderTargets = 6u;

	for (UINT i = 0; i < 6; i++)
	{
		SAFE_NEW(m_cameras[i], new Camera(DirectX::XM_PI * 0.5f, 1.0f, 0.1f, 50.0f));
	}

	m_cameras[0]->SetDirection(0, 1, 0);
	m_cameras[0]->SetUp(1, 0, 0);
	m_cameras[1]->SetDirection(0, -1, 0);
	m_cameras[1]->SetUp(1, 0, 0);
	m_cameras[2]->SetDirection(1, 0, 0);
	m_cameras[3]->SetDirection(-1, 0, 0);
	m_cameras[4]->SetDirection(0, 0, 1);
	m_cameras[5]->SetDirection(0, 0, -1);

	for (UINT i = 0; i < 6; i++)
	{
		m_cameras[i]->Update();
	}	
}

PointLight::~PointLight()
{
	for (UINT i = 0; i < 6u; i++)
	{
		m_cameras[i]->Release();
		SAFE_DELETE(m_cameras[i]);
	}	
}

void PointLight::SetDropOff(const float& dropOff)
{
	this->m_dropOff = dropOff;
}

void PointLight::SetPow(const float& pow)
{
	this->m_pow = pow;
}

void PointLight::SetRadius(const float& radius)
{
	this->m_radius = radius;
}

const float& PointLight::GetRadius() const
{
	return this->m_radius;
}

const float& PointLight::GetDropOff() const
{
	return this->m_dropOff;
}

const float& PointLight::GetPow() const
{
	return this->m_pow;
}

const Camera* const* PointLight::GetCameras() const
{
	return this->m_cameras;
}

BOOL PointLight::Init()
{
	if (!ILight::Init())
		return FALSE;
	HRESULT hr = 0;

	if (FAILED(hr = p_createDirectXContext(6u)))
	{
		return FALSE;
	}
	return TRUE;
}

void PointLight::Update()
{
	ILight::Update();
	for (UINT i = 0; i < 6; i++)
	{
		m_cameras[i]->SetPosition(GetPosition());
		m_cameras[i]->Update();
	}
}

void PointLight::Release()
{
	for (UINT i = 0; i < 6; i++)
	{
		m_cameras[i]->Release();
	}
	ILight::Release();
}
