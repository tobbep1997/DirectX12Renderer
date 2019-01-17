#include "DirectX12EnginePCH.h"
#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(RenderingManager* renderingManager, const Window& window)
	: ILight(renderingManager, window, ILight::LightType::Directional)
{
	SAFE_NEW(m_camera, new Camera(DirectX::XM_PI * 0.5, 1.0f, 1, 100.0f, FALSE));
}

DirectionalLight::~DirectionalLight()
{
	SAFE_DELETE(m_camera);
}

BOOL DirectionalLight::Init()
{
	if (!ILight::Init())
		return FALSE;
	m_camera->Init();
	if (FAILED(p_createDirectXContext()))
	{
		return FALSE;
	}
	return TRUE;
}

void DirectionalLight::Update()
{
	m_camera->Update();
}

void DirectionalLight::Release()
{
	m_camera->Release();
	ILight::Release();
}

Camera* DirectionalLight::GetCamera() const
{
	return this->m_camera;
}

void DirectionalLight::SetDirection(const DirectX::XMFLOAT4& direction) const
{
	m_camera->SetDirection(direction);
}

void DirectionalLight::SetDirection(const float& x, const float& y, const float& z, const float& w) const
{
	this->SetDirection(DirectX::XMFLOAT4(x, y, z, w));
}

void DirectionalLight::SetPosition(const DirectX::XMFLOAT4& position)
{
	Transform::SetPosition(position);
	this->m_camera->SetPosition(position);
}

void DirectionalLight::SetPosition(const float& x, const float& y, const float& z, const float& w)
{
	SetPosition(DirectX::XMFLOAT4(x, y, z, w));
}




