#include "DirectX12EnginePCH.h"
#include "Camera.h"


void Camera::Init()
{
	this->m_direction	= DirectX::XMFLOAT4(0, 0, 1, 0);
	this->m_up			= DirectX::XMFLOAT4(0, 1, 0, 0);
}

void Camera::Update()
{
	_calcView();
	_calcProjection();
	_calcViewProjection();
}

void Camera::Release()
{
}

void Camera::_calcView()
{
	using namespace DirectX;
	XMStoreFloat4x4A(&this->m_view, XMMatrixTranspose(
		XMMatrixLookToLH(
			XMLoadFloat4(&GetPosition()), 
			XMLoadFloat4(&m_direction), 
			XMLoadFloat4(&m_up))));
}

void Camera::_calcProjection()
{
	using namespace DirectX;
	XMStoreFloat4x4A(&this->m_projection, 
		XMMatrixTranspose(
			XMMatrixPerspectiveFovLH(
				m_fov, 
				m_aspectRatio, 
				m_nearPlane, 
				m_farPlane)));
}

void Camera::_calcViewProjection()
{
	using namespace DirectX;

	XMStoreFloat4x4A(&this->m_viewProjection,
		XMLoadFloat4x4A(&this->m_projection) * XMLoadFloat4x4A(&this->m_view));
}

Camera::Camera(const float& fov, const float& aspectRatio, const float& nearPlane, const float& farPlane)
{
	this->m_fov			= fov;
	this->m_aspectRatio = aspectRatio;
	this->m_nearPlane	= nearPlane;
	this->m_farPlane	= farPlane;
	Init();
}

Camera::~Camera()
{
}	

const DirectX::XMFLOAT4X4A& Camera::GetViewMatrix() const
{
	return this->m_view;
}

const DirectX::XMFLOAT4X4A& Camera::GetProjectionMatrix() const
{
	return this->m_projection;
}

const DirectX::XMFLOAT4X4A& Camera::GetViewProjectionMatrix() const
{
	return this->m_viewProjection;
}

void Camera::SetDirection(const DirectX::XMFLOAT4& direction)
{
	this->m_direction = direction;
}

void Camera::SetDirection(const float& x, const float& y, const float& z, const float& w)
{
	this->SetDirection(DirectX::XMFLOAT4(x, y, z, w));
}

void Camera::SetUp(const DirectX::XMFLOAT4& up)
{
	this->m_up = up;
}

void Camera::SetUp(const float& x, const float& y, const float& z, const float& w)
{
	this->SetUp(DirectX::XMFLOAT4(x, y, z, w));
}

void Camera::SetFov(const float& fov)
{
	this->m_fov = fov;
}

void Camera::SetAspectRatio(const float& aspectRatio)
{
	this->m_aspectRatio = aspectRatio;
}

void Camera::SetNearPlane(const float& nearPlane)
{
	this->m_nearPlane = nearPlane;
}

void Camera::SetFarPlane(const float& farPlane)
{
	this->m_farPlane = farPlane;
}

const float& Camera::GetFov() const
{
	return this->m_fov;
}

const float& Camera::GetAspectRatio() const
{
	return this->m_aspectRatio;
}

const float& Camera::GetNearPlane() const
{
	return this->m_nearPlane;
}

const float& Camera::GetFarPlane() const
{
	return this->m_farPlane;
}

const DirectX::XMFLOAT4& Camera::GetDirection() const
{
	return this->m_direction;
}

const DirectX::XMFLOAT4& Camera::GetUp() const
{
	return this->m_up;
}
