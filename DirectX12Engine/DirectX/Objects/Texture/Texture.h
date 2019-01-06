#pragma once
#include "../Template/IObject.h"
#include <wincodec.h>

class Texture :
	public IObject
{
public:
	Texture();
	~Texture();

	void Init() override;
	void Update() override;
	void Release() override;

	void SetRenderingManager(RenderingManager * renderingManager);

	BOOL LoadTexture(const std::string & path, RenderingManager * renderingManager = nullptr);

	ID3D12DescriptorHeap * GetId3D12DescriptorHeap() const;

	void MapTexture(RenderingManager * renderingManager, const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList = nullptr) const;

private:
	RenderingManager * m_renderingManager;

	BYTE* m_imageData;
	ID3D12Resource * m_textureBuffer = nullptr;
	ID3D12Resource * m_textureUploadHeap = nullptr;
	ID3D12DescriptorHeap * m_textureDescriptorHeap = nullptr;

	HRESULT _uploadTexture();

	static int _loadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDesc, LPCWSTR filename, int & bytesPerRow);
	
	static DXGI_FORMAT _getDxgiFormatFromWicFormat(WICPixelFormatGUID& wicFormatGUID);
	static WICPixelFormatGUID _getConvertToWicFormat(WICPixelFormatGUID& wicFormatGUID);
	static int _getDxgiFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
};



