#include "DirectX12EnginePCH.h"
#include "Transform.h"



Transform::Transform() : IObject()
{
	this->Init();
}

Transform::~Transform()
{
}

void Transform::Init()
{
	m_position	= DirectX::XMFLOAT4(0, 0, 0, 1);
	m_scale		= DirectX::XMFLOAT4(0, 0, 0, 1);
	m_rotation	= DirectX::XMFLOAT4(0, 0, 0, 1);

	_calcWorldMatrix();
}

void Transform::Update()
{
	_calcWorldMatrix();
}

void Transform::Release()
{
}

void Transform::SetPosition(const DirectX::XMFLOAT4& position)
{
	this->m_position = position;
}

void Transform::SetPosition(const float& x, const float& y, const float& z, const float& w)
{
	this->SetPosition(DirectX::XMFLOAT4(x, y, z, w));
}

void Transform::AddPosition(const DirectX::XMFLOAT4& position)
{
	this->SetPosition(m_position.x + position.x,
		m_position.y + position.y,
		m_position.z + position.z,
		position.w);
}

void Transform::AddPosition(const float& x, const float& y, const float& z, const float& w)
{
	this->SetPosition(m_position.x + x,
		m_position.y + y,
		m_position.z + z,
		w);
}

void Transform::SetRotation(const DirectX::XMFLOAT4& rotation)
{
	this->m_rotation = rotation;
}

void Transform::SetRotation(const float& x, const float& y, const float& z, const float& w)
{
	this->SetRotation(DirectX::XMFLOAT4(x, y, z, w));
}

void Transform::SetScale(const DirectX::XMFLOAT4& scale)
{
	this->m_scale = scale;
}

void Transform::SetScale(const float& x, const float& y, const float& z, const float& w)
{
	this->SetScale(DirectX::XMFLOAT4(x, y, z, w));
}

const DirectX::XMFLOAT4& Transform::GetPosition() const
{
	return this->m_position;
}

const DirectX::XMFLOAT4& Transform::GetRotation() const
{
	return this->m_rotation;
}

const DirectX::XMFLOAT4& Transform::GetScale() const
{
	return this->m_scale;
}

const DirectX::XMFLOAT4X4A& Transform::GetWorldMatrix() const
{
	return this->m_worldMatrix;
}

void Transform::_calcWorldMatrix()
{
	using namespace DirectX;
	
	const XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat4(&this->m_scale));
	const XMMATRIX rotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4(&this->m_rotation));
	const XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat4(&this->m_position));
	
	XMStoreFloat4x4A(&this->m_worldMatrix, XMMatrixTranspose(scale * rotation * translation));
}
