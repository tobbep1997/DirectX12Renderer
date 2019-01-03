#include "DirectX12EnginePCH.h"
#include "DirectionalLight.h"

DirectionalLight::DirectionalLight() : ILight()
{
	p_lightType = 1;
	m_camera = new Camera(DirectX::XM_PI * 0.5, 1.0f, 1, 7.5f, FALSE);
}

DirectionalLight::~DirectionalLight()
{
	delete m_camera;
	m_camera = nullptr;
}

void DirectionalLight::Init()
{
	m_camera->Init();
}

void DirectionalLight::Update()
{
	m_camera->Update();
}

void DirectionalLight::Release()
{
	m_camera->Release();
}

Camera* DirectionalLight::GetCamera()
{
	return this->m_camera;
}

void DirectionalLight::SetDirection(const DirectX::XMFLOAT4& direction)
{
	m_camera->SetDirection(direction);
}

void DirectionalLight::SetDirection(const float& x, const float& y, const float& z, const float& w)
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

const UINT& DirectionalLight::GetType() const
{
	return p_lightType;
}
