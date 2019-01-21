#pragma once
#include "../Template/IObject.h"
#include <wincodec.h>
#include <string>
#include <d3d12.h>

class RenderingManager;

class Texture :
	public IObject
{
public:
	Texture();
	~Texture();

	BOOL Init() override;
	void Update() override;
	void Release() override;

	void SetRenderingManager(RenderingManager * renderingManager);

	BOOL LoadTexture(const std::string & path, const BOOL & generateMips = TRUE, RenderingManager * renderingManager = nullptr);
	BOOL LoadDDSTexture(const std::string & path, const BOOL & generateMips = TRUE, RenderingManager * renderingManager = nullptr);

	ID3D12Resource * GetResource() const;
	ID3D12DescriptorHeap * GetId3D12DescriptorHeap() const;

	void MapTexture(RenderingManager * renderingManager, const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList = nullptr) const;

private:
	RenderingManager * m_renderingManager;

	BYTE* m_imageData;
	ID3D12Resource * m_textureBuffer = nullptr;
	ID3D12Resource * m_textureUploadHeap = nullptr;
	ID3D12DescriptorHeap * m_textureDescriptorHeap = nullptr;

	HRESULT _uploadTexture();
};



