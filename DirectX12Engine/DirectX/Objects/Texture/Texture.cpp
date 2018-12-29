#include "DirectX12EnginePCH.h"
#include "Texture.h"

Texture::Texture(RenderingManager* renderingManager)
{
	this->m_renderingManager = renderingManager;
}

Texture::~Texture()
{
}

void Texture::Init()
{
}

void Texture::Update()
{
}

void Texture::Release()
{
}

BOOL Texture::LoadTexture(const std::string& path)
{
	if (!m_renderingManager)
		return FALSE;


	
	return FALSE;
}
