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

	void CopyDescriptorHeap() const;
	void MapTexture(RenderingManager * renderingManager, const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList = nullptr) const;

private:
	RenderingManager * m_renderingManager;

	BYTE* m_imageData;
	ID3D12Resource * m_textureBuffer = nullptr;

	HRESULT _uploadTexture();

	//SIZE_T m_descriptorHeapOffset = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ 0 };
	mutable D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{ 0 };
};



