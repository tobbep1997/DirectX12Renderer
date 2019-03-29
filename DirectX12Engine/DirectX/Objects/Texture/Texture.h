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


	BOOL LoadTexture(const std::string & path, const BOOL & generateMips = TRUE);
	BOOL LoadDDSTexture(const std::string & path, const BOOL & generateMips = TRUE);

	ID3D12Resource * GetResource() const;

	void CopyDescriptorHeap() const;
	void MapTexture(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList = nullptr) const;

	const D3D12_CPU_DESCRIPTOR_HANDLE & GetCpuHandle() const;

private:
	RenderingManager * m_renderingManager = nullptr;

	BYTE* m_imageData = nullptr;
	ID3D12Resource * m_textureBuffer = nullptr;

	HRESULT _uploadTexture();

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ 0 };
	mutable D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{ 0 };
};



