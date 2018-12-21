#pragma once
#include "Transform.h"
class Camera :
	public Transform
{
public:
	void Init() override;
	void Release() override;
private:
	DirectX::XMFLOAT4 m_direction;
	DirectX::XMFLOAT4 m_up;
	DirectX::XMFLOAT4 m_right;

	DirectX::XMFLOAT4X4A m_view;
	DirectX::XMFLOAT4X4A m_projection;
	DirectX::XMFLOAT4X4A m_viewProjection;

	float m_fov;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;

	void _calcView();
	void _calcProjection();
	void _calcViewProjection();

public:
	explicit Camera(const float & fov = DirectX::XM_PI * 0.5f, const float & aspectRatio = 16.0f / 9.0f, const float & nearPlane = 1.0f, const float & farPlane = 20.0f);
	~Camera();

	void SetDirection(const DirectX::XMFLOAT4 & direction);
	void SetDirection(const float & x, const float & y, const float & z, const float & w = 0.0f);

	void SetUp(const DirectX::XMFLOAT4 & up);
	void SetUp(const float & x = 0.0f, const float & y = 1.0f, const float & z = 0.0f, const float & w = 0.0f);

	void SetFov(const float & fov);
	void SetAspectRatio(const float & aspectRatio);
	void SetNearPlane(const float & nearPlane);
	void SetFarPlane(const float & farPlane);

	const float & GetFov() const;
	const float & GetAspectRatio() const;
	const float & GetNearPlane() const;
	const float & GetFarPlane() const;

	const DirectX::XMFLOAT4 & GetDirection() const;
	const DirectX::XMFLOAT4 & GetUp() const;

	const DirectX::XMFLOAT4X4A & GetViewMatrix();
	const DirectX::XMFLOAT4X4A & GetProjectionMatrix();
	const DirectX::XMFLOAT4X4A & GetViewProjectionMatrix();
};

