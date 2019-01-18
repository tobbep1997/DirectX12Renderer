#pragma once
#include "Transform.h"
class Camera :
	public Transform
{
public:
	BOOL Init() override;
	void Update() override;
	void Release() override;
private:
	DirectX::XMFLOAT4 m_direction, m_focusPoint;
	DirectX::XMFLOAT4 m_up;

	DirectX::XMFLOAT4X4A m_view;
	DirectX::XMFLOAT4X4A m_projection;
	DirectX::XMFLOAT4X4A m_viewProjection;

	float m_fov;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;

	BOOL m_usePerspective;
	BOOL m_useFocusPoint = FALSE;

	void _calcView();
	void _calcProjection();
	void _calcViewProjection();

public:
	explicit Camera(
		const DirectX::XMFLOAT4 & position);
	explicit Camera(
		const float & fov = DirectX::XM_PI * 0.5f,
		const float & aspectRatio = 16.0f / 9.0f, 
		const float & nearPlane = 1.0f,
		const float & farPlane = 100.0f, 
		const BOOL & perspective = TRUE);
	~Camera();

	void SetDirection(const DirectX::XMFLOAT4 & direction);
	void SetDirection(const float & x, const float & y, const float & z, const float & w = 0.0f);

	void SetFocusPoint(const DirectX::XMFLOAT4 & focusPoint);
	void SetFocusPoint(const float & x, const float & y, const float & z, const float & w = 1.0f);


	void Rotate(const DirectX::XMFLOAT4 & rotation);
	void Rotate(const float & x, const float & y, const float & z, const float & w = 0.0f);

	void Translate(const DirectX::XMFLOAT4 & position) override;
	void Translate(const float & x, const float & y, const float & z, const float & w = 1.0f) override;

	void SetUp(const DirectX::XMFLOAT4 & up);
	void SetUp(const float & x = 0.0f, const float & y = 1.0f, const float & z = 0.0f, const float & w = 0.0f);

	void SetFov(const float & fov);
	void SetAspectRatio(const float & aspectRatio);
	void SetNearPlane(const float & nearPlane);
	void SetFarPlane(const float & farPlane);

	void SetPerspective(const BOOL & perspective);
	void SetFocusPoint(const BOOL & focusPoint);

	const float & GetFov() const;
	const float & GetAspectRatio() const;
	const float & GetNearPlane() const;
	const float & GetFarPlane() const;

	const DirectX::XMFLOAT4 & GetDirection();
	const DirectX::XMFLOAT4 & GetUp() const;

	const DirectX::XMFLOAT4X4A & GetViewMatrix() const;
	const DirectX::XMFLOAT4X4A & GetProjectionMatrix() const;
	const DirectX::XMFLOAT4X4A & GetViewProjectionMatrix() const;
};

