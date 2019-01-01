#pragma once

class DirectionalLight : 
	public ILight
{
public:
	DirectionalLight();
	~DirectionalLight();

	void Init() override;
	void Update() override;
	void Release() override;

	Camera * GetCamera();

	void SetDirection(const DirectX::XMFLOAT4 & direction);
	void SetDirection(const float & x, const float & y, const float & z, const float & w = 0.0f);

	const UINT& GetType() const override;
private:
	Camera * m_camera;
};
