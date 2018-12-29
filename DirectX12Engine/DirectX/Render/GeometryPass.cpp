#include "DirectX12EnginePCH.h"
#include "GeometryPass.h"


GeometryPass::GeometryPass(RenderingManager * renderingManager, 
	const Window & window) :
	IRender(renderingManager, window)
{
	if (!renderingManager)
		Window::CreateError("GeometryPass : Missing RenderingManager");
	m_inputLayoutDesc = {};
}


GeometryPass::~GeometryPass()
= default;

HRESULT GeometryPass::Init()
{
	HRESULT hr = 0;

	if (SUCCEEDED(hr = _preInit()))
	{
		if (FAILED(hr = _signalGPU()))
		{
			return hr;
		}
	}
	return hr;
}

HRESULT GeometryPass::Update(const Camera & camera)
{
	HRESULT hr = 0;
	p_renderingManager->GetCommandList()->ClearDepthStencilView(
		m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0,
		nullptr);

	m_objectBuffer.CameraPosition = DirectX::XMFLOAT4A(camera.GetPosition().x,
		camera.GetPosition().y,
		camera.GetPosition().z,
		camera.GetPosition().w);
	m_objectBuffer.ViewProjection = camera.GetViewProjectionMatrix();
	const UINT drawQueueSize = static_cast<UINT>(p_drawQueue->size());
	for (UINT i = 0; i < drawQueueSize; i++)
	{
		m_objectBuffer.WorldMatrix = p_drawQueue->at(i)->GetWorldMatrix();

		memcpy(m_cameraBufferGPUAddress[*p_renderingManager->GetFrameIndex()] + i * m_constantBufferPerObjectAlignedSize, &m_objectBuffer, sizeof(m_objectBuffer));
	}


	//memcpy(m_cameraBufferGPUAddress[*p_renderingManager->GetFrameIndex()], &m_objectBuffer, sizeof(m_objectBuffer));


	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		p_renderingManager->GetRTVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		*p_renderingManager->GetFrameIndex(),
		*p_renderingManager->GetRTVDescriptorSize());

	p_renderingManager->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	p_renderingManager->GetCommandList()->SetPipelineState(m_pipelineState);
	p_renderingManager->GetCommandList()->SetGraphicsRootSignature(m_rootSignature);
	p_renderingManager->GetCommandList()->RSSetViewports(1, &m_viewport);
	p_renderingManager->GetCommandList()->RSSetScissorRects(1, &m_rect);
	p_renderingManager->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	ID3D12DescriptorHeap* descriptorHeaps[] = { m_textureDescriptorHeap };
	p_renderingManager->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	p_renderingManager->GetCommandList()->SetGraphicsRootDescriptorTable(1, m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());


	return hr;
}

HRESULT GeometryPass::Draw()
{
	HRESULT hr = 0;
	const UINT drawQueueSize = static_cast<UINT>(p_drawQueue->size());
	for (UINT i = 0; i < drawQueueSize; i++)
	{		
		p_renderingManager->GetCommandList()->IASetVertexBuffers(0, 1, &p_drawQueue->at(i)->GetMesh().GetVertexBufferView());		

		p_renderingManager->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_constantBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + i * m_constantBufferPerObjectAlignedSize);
		
		p_renderingManager->GetCommandList()->DrawInstanced(static_cast<UINT>(p_drawQueue->at(i)->GetMesh().GetStaticMesh().size()), 1, 0, 0);
	}

	return hr;
}

HRESULT GeometryPass::Clear()
{
	this->p_drawQueue->clear();
	return S_OK;
}

HRESULT GeometryPass::Release()
{
	HRESULT hr = 0;
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_depthStencilDescriptorHeap);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_constantBufferDescriptorHeap[i]);
		SAFE_RELEASE(m_constantBuffer[i]);
	}

	SAFE_RELEASE(m_textureBuffer);
	SAFE_RELEASE(m_textureUploadHeap);
	SAFE_RELEASE(m_textureDescriptorHeap);
	if (imageData)
		delete[] imageData;
	return hr;
}

HRESULT GeometryPass::_preInit()
{
	HRESULT hr = 0;
	
	if (SUCCEEDED(hr = p_renderingManager->OpenCommandList()))
	{
		if (SUCCEEDED(hr = _initID3D12RootSignature()))
		{
			if (SUCCEEDED(hr = _initShaders()))
			{
				if (SUCCEEDED(hr = _initID3D12PipelineState()))
				{
					if (SUCCEEDED(hr = _createViewport()))
					{
						if (SUCCEEDED(hr = _createDepthStencil()))
						{
							if (SUCCEEDED(hr = _createConstantBuffer()))
							{
								if (SUCCEEDED(hr = tempLoadTexture()))
								{
									
								}
							}
						}
					}
				}
			}
		}
	}
	if (FAILED(hr))
		this->Release();
	return hr;
}

HRESULT GeometryPass::_signalGPU()
{
	HRESULT hr = 0;

	if (SUCCEEDED(hr = p_renderingManager->SignalGPU()))
	{
		delete[] imageData;
		imageData = nullptr;
	}

	return hr;
}

HRESULT GeometryPass::_initID3D12RootSignature()
{
	HRESULT hr = 0;

	D3D12_DESCRIPTOR_RANGE descriptorRangeTable[1];
	descriptorRangeTable[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeTable[0].NumDescriptors = 1;
	descriptorRangeTable[0].BaseShaderRegister = 0;
	descriptorRangeTable[0].RegisterSpace = 0;
	descriptorRangeTable[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorRangeTable);
	descriptorTable.pDescriptorRanges = &descriptorRangeTable[0];

	D3D12_ROOT_DESCRIPTOR rootDescriptor;
	rootDescriptor.RegisterSpace = 0;
	rootDescriptor.ShaderRegister = 0;

	m_rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[0].Descriptor = rootDescriptor;
	m_rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	m_rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[1].DescriptorTable = descriptorTable;
	m_rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameters),
		m_rootParameters, 
		1, 
		&sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob * signature = nullptr;
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr)))
	{
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature))))
		{
			SAFE_RELEASE(m_rootSignature);
		}
	}
	SAFE_RELEASE(signature);
	return hr;
}

HRESULT GeometryPass::_initID3D12PipelineState()
{
	HRESULT hr = 0;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	m_inputLayoutDesc.pInputElementDescs = inputLayout;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = m_inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
	graphicsPipelineStateDesc.PS = m_pixelShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	DXGI_SWAP_CHAIN_DESC desc;
	if (SUCCEEDED(hr = p_renderingManager->GetSwapChain()->GetDesc(&desc)))
	{
		graphicsPipelineStateDesc.SampleDesc = desc.SampleDesc;
	}
	else
		return hr;

	if (FAILED(hr = p_renderingManager->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&m_pipelineState))))
	{
		SAFE_RELEASE(m_pipelineState);
	}

	return hr;
}

HRESULT GeometryPass::_initShaders()
{
	HRESULT hr = 0;
	ID3DBlob * blob = nullptr;

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryVertex.hlsl", blob, "vs_5_1")))
	{
		return hr;
	}
	else
	{
		m_vertexShader.BytecodeLength = blob->GetBufferSize();
		m_vertexShader.pShaderBytecode = blob->GetBufferPointer();
	}

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryPixel.hlsl", blob, "ps_5_1")))
	{
		return hr;
	}
	else
	{
		m_pixelShader.BytecodeLength = blob->GetBufferSize();
		m_pixelShader.pShaderBytecode = blob->GetBufferPointer();
	}

	return hr;
}

HRESULT GeometryPass::_createViewport()
{
	HRESULT hr = 0;
	// Fill out the Viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<FLOAT>(p_window->GetWidth());
	m_viewport.Height = static_cast<FLOAT>(p_window->GetHeight());
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	m_rect.left = 0;
	m_rect.top = 0;
	m_rect.right = p_window->GetWidth();
	m_rect.bottom = p_window->GetHeight();
	return hr;
}

HRESULT GeometryPass::_createDepthStencil()
{
	HRESULT hr = 0;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};

	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(&m_depthStencilDescriptorHeap))))
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Tex2D(
					DXGI_FORMAT_D32_FLOAT,
					p_window->GetWidth(), p_window->GetHeight(),
					1, 0, 1, 0, 
					D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencilBuffer))))
		{
			p_renderingManager->GetDevice()->CreateDepthStencilView(
				m_depthStencilBuffer,
				&depthStencilDesc,
				m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			
				
			
		}
	}

	return hr;
}

HRESULT GeometryPass::_createConstantBuffer()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		//D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		//heapDesc.NumDescriptors = BUFFER_SIZE;
		//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//if (FAILED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
		//	&heapDesc,
		//	IID_PPV_ARGS(&m_constantBufferDescriptorHeap[i]))))
		//{
		//	return hr;
		//}

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer[i]))))
		{
			SET_NAME(m_constantBuffer[i], L"ConstantBuffer Upload Resource Heap");
		}
		else
			return hr;

		//D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		//cbvDesc.BufferLocation = m_constantBuffer[i]->GetGPUVirtualAddress();
		//cbvDesc.SizeInBytes = (sizeof(m_constantBuffer) + 255) & ~255;

		/*p_renderingManager->GetDevice()->CreateConstantBufferView(
			&cbvDesc,
			m_constantBufferDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());*/

		CD3DX12_RANGE readRange(0, 0);
		if (SUCCEEDED(hr = m_constantBuffer[i]->Map(
			0, &readRange,
			reinterpret_cast<void **>(&m_cameraBufferGPUAddress[i]))))
		{
			memcpy(m_cameraBufferGPUAddress[i], &m_objectBuffer, sizeof(m_objectBuffer));
		}
	}
	return hr;
}

HRESULT GeometryPass::tempLoadTexture()
{
	HRESULT hr = 0;
	
	// Load the image from file
	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"../car.bmp", imageBytesPerRow);

	if (imageSize <= 0)
	{
		Window::CreateError("NO TEXTURE");
	}

	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&textureDesc, // the description of our texture
		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
		nullptr, // used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&m_textureBuffer))))
	{
		UINT64 textureUploadBufferSize;
		p_renderingManager->GetDevice()->GetCopyableFootprints(&textureDesc,
			0, 1, 0,
			nullptr,
			nullptr,
			nullptr,
			&textureUploadBufferSize);

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_textureUploadHeap))))
		{
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = &imageData[0]; // pointer to our image data
			textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
			textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data

			// Now we copy the upload buffer contents to the default heap
			UpdateSubresources(
				p_renderingManager->GetCommandList(),
				m_textureBuffer, 
				m_textureUploadHeap, 
				0, 0, 1, 
				&textureData);

			// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
			p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = 1;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

			if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
				&heapDesc, 
				IID_PPV_ARGS(&m_textureDescriptorHeap))))
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = textureDesc.Format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;
				p_renderingManager->GetDevice()->CreateShaderResourceView(
					m_textureBuffer, &srvDesc, m_textureDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			}
		}
	}
	return hr;
}

int GeometryPass::LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDesc, LPCWSTR filename,
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
					DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);


					if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
					{
						WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

						if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

						dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

						hr = wicFactory->CreateFormatConverter(&wicConverter);
						if (FAILED(hr)) return 0;

						BOOL canConvert = FALSE;
						hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
						if (FAILED(hr) || !canConvert) return 0;

						hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
						if (FAILED(hr)) return 0;

						imageConverted = true;
					}

					int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat);

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

DXGI_FORMAT GeometryPass::GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
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

WICPixelFormatGUID GeometryPass::GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
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

int GeometryPass::GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
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
}
