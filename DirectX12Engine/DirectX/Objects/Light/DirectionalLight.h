#pragma once

class DirectionalLight : 
	public ILight
{
public:
	DirectionalLight(RenderingManager * renderingManager, const Window & window);
	~DirectionalLight();

	void Init() override;
	void Update() override;
	void Release() override;

	Camera * GetCamera() const;

	void SetDirection(const DirectX::XMFLOAT4 & direction) const;
	void SetDirection(const float & x, const float & y, const float & z, const float & w = 0.0f) const;

	void SetPosition(const DirectX::XMFLOAT4& position) override;
	void SetPosition(const float& x, const float& y, const float& z, const float& w = 1.0f) override;

private:
	Camera * m_camera;
};
