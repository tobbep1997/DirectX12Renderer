#pragma once
#include "Template/IObject.h"

class Transform : //NOLINT
	public IObject
{
public:
	Transform();
	~Transform();

	BOOL Init() override;
	void Update() override;
	void Release() override;

	virtual void SetPosition(const DirectX::XMFLOAT4 & position);
	virtual void SetPosition(const float & x, const float & y, const float & z, const float & w = 1.0f);

	virtual void Translate(const DirectX::XMFLOAT4 & position);
	virtual void Translate(const float & x, const float & y, const float & z, const float & w = 1.0f);
	
	virtual void SetRotation(const DirectX::XMFLOAT4 & rotation);
	virtual void SetRotation(const float & x, const float & y, const float & z, const float & w = 1.0f);

	virtual void SetScale(const DirectX::XMFLOAT4 & scale);
	virtual void SetScale(const float & x, const float & y, const float & z, const float & w = 1.0f);

	virtual const DirectX::XMFLOAT4 & GetPosition() const;
	virtual const DirectX::XMFLOAT4 & GetRotation() const;
	virtual const DirectX::XMFLOAT4 & GetScale() const;

	virtual const DirectX::XMFLOAT4X4A & GetWorldMatrix() const;

private:
	DirectX::XMFLOAT4 m_position	= DirectX::XMFLOAT4(0, 0, 0, 1);
	DirectX::XMFLOAT4 m_scale		= DirectX::XMFLOAT4(1, 1, 1, 1);
	DirectX::XMFLOAT4 m_rotation	= DirectX::XMFLOAT4(0, 0, 0, 1);

	DirectX::XMFLOAT4X4A m_worldMatrix;
protected:

	void _calcWorldMatrix();
	
};

