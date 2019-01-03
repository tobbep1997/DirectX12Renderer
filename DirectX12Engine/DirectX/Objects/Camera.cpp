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

	if (m_usePerspective)
	{
		XMStoreFloat4x4A(&this->m_projection,
			XMMatrixTranspose(
				XMMatrixPerspectiveFovLH(
					m_fov,
					m_aspectRatio,
					m_nearPlane,
					m_farPlane)));	
	}
	else
	{
		XMStoreFloat4x4A(&this->m_projection,
			XMMatrixTranspose(
				XMMatrixOrthographicLH(
					16, 
					16,
					m_nearPlane, 
					m_farPlane)));	
	}
}

void Camera::_calcViewProjection()
{
	using namespace DirectX;

	XMStoreFloat4x4A(&this->m_viewProjection,
		XMLoadFloat4x4A(&this->m_projection) * XMLoadFloat4x4A(&this->m_view));
}

Camera::Camera(const float& fov, const float& aspectRatio, const float& nearPlane, const float& farPlane, const BOOL & perspective)
{
	this->m_fov			= fov;
	this->m_aspectRatio = aspectRatio;
	this->m_nearPlane	= nearPlane;
	this->m_farPlane	= farPlane;
	this->m_usePerspective = perspective;
	Camera::Init();
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
	using namespace DirectX;
	XMVECTOR vDir = XMLoadFloat4(&direction);
	XMVector3Normalize(vDir);
	XMStoreFloat4(&this->m_direction, vDir);

	this->m_direction.w = direction.w;
}

void Camera::SetDirection(const float& x, const float& y, const float& z, const float& w)
{
	this->SetDirection(DirectX::XMFLOAT4(x, y, z, w));
}

void Camera::Rotate(const DirectX::XMFLOAT4& rotation)
{
	DirectX::XMVECTOR vDir = DirectX::XMLoadFloat4(&this->m_direction);
	const DirectX::XMVECTOR vLastDir = vDir;

	DirectX::XMFLOAT4A lUp(0, 1, 0, 0);

	DirectX::XMVECTOR vUp = DirectX::XMLoadFloat4A(&lUp);
	DirectX::XMVECTOR vRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vUp, vDir));

	vRight = DirectX::XMVectorScale(vRight, rotation.x);
	vUp = DirectX::XMVectorScale(vUp, rotation.y);
	vDir = DirectX::XMVectorScale(vDir, rotation.z);

	DirectX::XMVECTOR vRot = DirectX::XMVectorAdd(vRight, vUp);
	vRot = DirectX::XMVectorAdd(vRot, vDir);

	const DirectX::XMMATRIX mRot = DirectX::XMMatrixRotationRollPitchYawFromVector(vRot);
	
	const DirectX::XMVECTOR vNewDir = DirectX::XMVector3Normalize(DirectX::XMVector3Transform(vLastDir, mRot));
	vUp = DirectX::XMLoadFloat4(&m_up);

	const DirectX::XMVECTOR vDot = DirectX::XMVector3Dot(vNewDir, vUp);
	const float dot = DirectX::XMVectorGetX(vDot);
	if (fabs(dot) < 0.90)
	{
		DirectX::XMStoreFloat4(&this->m_direction, DirectX::XMVector3Normalize(vNewDir));
		m_direction.w = 0.0f;
	}
}

void Camera::Rotate(const float& x, const float& y, const float& z, const float& w)
{
	this->Rotate(DirectX::XMFLOAT4(x, y, z, w));
}

void Camera::Translate(const DirectX::XMFLOAT4& position)
{
	const DirectX::XMVECTOR vDir = DirectX::XMLoadFloat4(&this->m_direction);

	const DirectX::XMVECTOR vUp = DirectX::XMLoadFloat4(&this->m_up);
	const DirectX::XMVECTOR vRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vUp, vDir));

	DirectX::XMFLOAT3 right;
	DirectX::XMStoreFloat3(&right, vRight);

	DirectX::XMFLOAT4 newPos = GetPosition();

	newPos.x += position.x * right.x;
	newPos.y += position.x * right.y;
	newPos.z += position.x * right.z;

	newPos.x += position.y * m_up.x;
	newPos.y += position.y * m_up.y;
	newPos.z += position.y * m_up.z;

	newPos.x += position.z * m_direction.x;
	newPos.y += position.z * m_direction.y;
	newPos.z += position.z * m_direction.z;
	
	newPos.w = 1.0f;

	SetPosition(newPos);
}

void Camera::Translate(const float& x, const float& y, const float& z, const float& w)
{
	this->Translate(DirectX::XMFLOAT4(x, y, z, w));
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
