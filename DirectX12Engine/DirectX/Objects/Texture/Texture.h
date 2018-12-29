#pragma once
#include "../Template/IObject.h"
#include "Submodule/DirectXTK/Inc/WICTextureLoader.h"
class Texture :
	public IObject
{
public:
	Texture(RenderingManager * renderingManager);
	~Texture();

	void Init() override;
	void Update() override;
	void Release() override;
	   
	BOOL LoadTexture(const std::string & path);

private:
	ID3D12Resource * m_texture;
	RenderingManager * m_renderingManager;
};



