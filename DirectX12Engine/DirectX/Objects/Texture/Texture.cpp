#include "DirectX12EnginePCH.h"
#include "Texture.h"

Texture::Texture()
{
	this->m_renderingManager = nullptr;
	this->m_imageData = nullptr;
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
	if (m_imageData)
		delete[] m_imageData;
	SAFE_RELEASE(m_textureBuffer);
	SAFE_RELEASE(m_textureUploadHeap);
	SAFE_RELEASE(m_textureDescriptorHeap);
}

void Texture::SetRenderingManager(RenderingManager* renderingManager)
{
	this->m_renderingManager = renderingManager;
}

BOOL Texture::LoadTexture(const std::string& path, RenderingManager * renderingManager)
{
	if (!m_renderingManager && !renderingManager)
		return FALSE;
	if (!m_renderingManager && renderingManager)
		m_renderingManager = renderingManager;

	HRESULT hr = 0;

	// Load the image from file
	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	const int imageSize = _loadImageDataFromFile(&m_imageData, textureDesc, DEBUG::StringToWstring(path).c_str(), imageBytesPerRow);

	if (imageSize <= 0)
	{
		return FALSE;
	}
	if (SUCCEEDED(hr = m_renderingManager->OpenCommandList()))
	{

		if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&textureDesc, // the description of our texture
			D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
			nullptr, // used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&m_textureBuffer))))
		{
			SET_NAME(m_textureBuffer, DEBUG::StringToWstring(path) + L" Texture Buffer");
			UINT64 textureUploadBufferSize;
			m_renderingManager->GetDevice()->GetCopyableFootprints(&textureDesc,
				0, 1, 0,
				nullptr,
				nullptr,
				nullptr,
				&textureUploadBufferSize);

			if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_textureUploadHeap))))
			{
				SET_NAME(m_textureBuffer, DEBUG::StringToWstring(path) + L" Texture Upload Heap");

				D3D12_SUBRESOURCE_DATA textureData = {};
				textureData.pData = &m_imageData[0]; // pointer to our image data
				textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
				textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data

				// Now we copy the upload buffer contents to the default heap
				UpdateSubresources(
					m_renderingManager->GetCommandList(),
					m_textureBuffer,
					m_textureUploadHeap,
					0, 0, 1,
					&textureData);

				// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
				m_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.NumDescriptors = 1;
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

				if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateDescriptorHeap(
					&heapDesc,
					IID_PPV_ARGS(&m_textureDescriptorHeap))))
				{
					SET_NAME(m_textureBuffer, DEBUG::StringToWstring(path) + L" Texture DescriptorHeap");

					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.Format = textureDesc.Format;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MipLevels = 1;
					m_renderingManager->GetDevice()->CreateShaderResourceView(
						m_textureBuffer, &srvDesc, m_textureDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

					if (SUCCEEDED(hr = _uploadTexture()))
					{

					}

				}
			}
		}
	}


	if (SUCCEEDED(hr))
		return TRUE;
	return FALSE;
}
ID3D12DescriptorHeap* Texture::GetId3D12DescriptorHeap() const
{
	return m_textureDescriptorHeap;
}

HRESULT Texture::_uploadTexture()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = m_renderingManager->SignalGPU()))
	{
		delete[] m_imageData;
		m_imageData = nullptr;
	}
	return hr;
}

int Texture::_loadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDesc, LPCWSTR filename,
	int& bytesPerRow)
{
	HRESULT hr = 0;

	static IWICImagingFactory * wicFactory = nullptr;

	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;

	BOOL imageConverted = FALSE;

	if (wicFactory == nullptr)
	{
		CoInitialize(NULL);

		if (FAILED(hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory))))
		{
			return 0;
		}
	}

	if (SUCCEEDED(hr = wicFactory->CreateDecoderFromFilename(
		filename,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&wicDecoder)))
	{
		if (SUCCEEDED(hr = wicDecoder->GetFrame(0, &wicFrame)))
		{
			WICPixelFormatGUID pixelFormat;
			if (SUCCEEDED(hr = wicFrame->GetPixelFormat(&pixelFormat)))
			{
				UINT textureWidth, textureHeight;
				if (SUCCEEDED(hr = wicFrame->GetSize(&textureWidth, &textureHeight)))
				{
					DXGI_FORMAT dxgiFormat = _getDxgiFormatFromWicFormat(pixelFormat);


					if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
					{
						WICPixelFormatGUID convertToPixelFormat = _getConvertToWicFormat(pixelFormat);

						if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

						dxgiFormat = _getDxgiFormatFromWicFormat(convertToPixelFormat);

						hr = wicFactory->CreateFormatConverter(&wicConverter);
						if (FAILED(hr)) return 0;

						BOOL canConvert = FALSE;
						hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
						if (FAILED(hr) || !canConvert) return 0;

						hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
						if (FAILED(hr)) return 0;

						imageConverted = true;
					}

					int bitsPerPixel = _getDxgiFormatBitsPerPixel(dxgiFormat);

					bytesPerRow = (textureWidth * bitsPerPixel) / 8;
					int imageSize = bytesPerRow * textureHeight;

					*imageData = (BYTE*)malloc(imageSize);

					if (imageConverted)
					{
						if (FAILED(hr = wicConverter->CopyPixels(0,
							bytesPerRow,
							imageSize,
							*imageData)))
						{
							return 0;
						}
					}
					else
					{
						if (FAILED(hr = wicFrame->CopyPixels(0,
							bytesPerRow,
							imageSize,
							*imageData)))
						{
							return 0;
						}
					}

					resourceDesc = {};
					resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
					resourceDesc.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
					resourceDesc.Width = textureWidth; // width of the texture
					resourceDesc.Height = textureHeight; // height of the texture
					resourceDesc.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
					resourceDesc.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
					resourceDesc.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
					resourceDesc.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
					resourceDesc.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
					resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
					resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags
					return imageSize;
				}
			}
		}
	}
	return 0;
}

DXGI_FORMAT Texture::_getDxgiFormatFromWicFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
	if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
	if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
	if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
	if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

	return DXGI_FORMAT_UNKNOWN;
}

WICPixelFormatGUID Texture::_getConvertToWicFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
	if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
	if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
	if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
	if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
	if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
	if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
	if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
	if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
	if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

	return GUID_WICPixelFormatDontCare;
}

int Texture::_getDxgiFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
	if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;

	return 0;
}
