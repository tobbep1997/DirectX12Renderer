#pragma once
#include "Template/IObject.h"

class Transform : //NOLINT
	public IObject
{
public:
	Transform();
	~Transform();

	void Init() override;
	void Update() override;
	void Release() override;

	void SetPosition(const DirectX::XMFLOAT4 & position);
	void SetPosition(const float & x, const float & y, const float & z, const float & w = 1.0f);

	void Translate(const DirectX::XMFLOAT4 & position);
	void Translate(const float & x, const float & y, const float & z, const float & w = 1.0f);


	void SetRotation(const DirectX::XMFLOAT4 & rotation);
	void SetRotation(const float & x, const float & y, const float & z, const float & w = 1.0f);

	void SetScale(const DirectX::XMFLOAT4 & scale);
	void SetScale(const float & x, const float & y, const float & z, const float & w = 1.0f);

	const DirectX::XMFLOAT4 & GetPosition() const;
	const DirectX::XMFLOAT4 & GetRotation() const;
	const DirectX::XMFLOAT4 & GetScale() const;

	const DirectX::XMFLOAT4X4A & GetWorldMatrix() const;

private:
	DirectX::XMFLOAT4 m_position	= DirectX::XMFLOAT4(0, 0, 0, 1);
	DirectX::XMFLOAT4 m_scale		= DirectX::XMFLOAT4(0, 0, 0, 1);
	DirectX::XMFLOAT4 m_rotation	= DirectX::XMFLOAT4(0, 0, 0, 1);

	DirectX::XMFLOAT4X4A m_worldMatrix;
protected:

	void _calcWorldMatrix();
	
};

