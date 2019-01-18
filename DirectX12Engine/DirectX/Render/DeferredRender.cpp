#include "DirectX12EnginePCH.h"
#include "DeferredRender.h"
#include "WrapperFunctions/X12RenderTargetView.h"
#include "WrapperFunctions/RenderingHelpClass.h"
#include "WrapperFunctions/X12ConstantBuffer.h"
#include "WrapperFunctions/X12ShaderResourceView.h"


DeferredRender::DeferredRender(RenderingManager* renderingManager, const Window& window)
	: IRender(renderingManager, window)
{	
	m_vertexList[0] = Vertex(DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0, 1, 0, 0));
	m_vertexList[1] = Vertex(DirectX::XMFLOAT4(-1.0f,  1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0, 0, 0, 0));
	m_vertexList[2] = Vertex(DirectX::XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1, 1, 0, 0));
	m_vertexList[3] = Vertex(DirectX::XMFLOAT4( 1.0f,  1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1, 0, 0, 0));

	SAFE_NEW(m_lightBuffer, new X12ConstantBuffer(p_renderingManager, *p_window));
	SAFE_NEW(m_shadowBuffer, new X12ConstantBuffer(p_renderingManager, *p_window));
	SAFE_NEW(m_shaderResourceView, new X12ShaderResourceView(p_renderingManager, *p_window));

	SAFE_NEW(m_shadowMaps, new std::vector<ShadowMap*>());
}

DeferredRender::~DeferredRender()
{
}

HRESULT DeferredRender::Init()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = _preInit()))
	{
		if (SUCCEEDED(hr = p_renderingManager->SignalGPU()))
		{
			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);
			m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
		}
	}
	return hr;
}

void DeferredRender::Update(const Camera& camera, const float & deltaTime)
{
	m_lightValues.CameraPosition = DirectX::XMFLOAT4A(camera.GetPosition().x,
		camera.GetPosition().y,
		camera.GetPosition().z,
		camera.GetPosition().w);

	const UINT lightQueueSize = static_cast<const UINT>(p_lightQueue->size());
	for (UINT i = 0; i < lightQueueSize && i < 256; i++)
	{
		m_lightValues.Type[i] = DirectX::XMUINT4(lightQueueSize,
			p_lightQueue->at(i)->GetType(),
			0, 0);
		m_lightValues.Position[i] = DirectX::XMFLOAT4A(p_lightQueue->at(i)->GetPosition().x,
			p_lightQueue->at(i)->GetPosition().y,
			p_lightQueue->at(i)->GetPosition().z,
			p_lightQueue->at(i)->GetPosition().w);
		m_lightValues.Color[i] = DirectX::XMFLOAT4A(p_lightQueue->at(i)->GetColor().x,
			p_lightQueue->at(i)->GetColor().y,
			p_lightQueue->at(i)->GetColor().z,
			p_lightQueue->at(i)->GetColor().w);

		if (dynamic_cast<PointLight*>(p_lightQueue->at(i)))
		{
			PointLight* pl = dynamic_cast<PointLight*>(p_lightQueue->at(i));

			m_lightValues.Vector[i] = DirectX::XMFLOAT4A(pl->GetIntensity(),
				pl->GetDropOff(),
				pl->GetPow(),
				pl->GetRadius());
		}
		if (dynamic_cast<DirectionalLight*>(p_lightQueue->at(i)))
		{
			DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(p_lightQueue->at(i));

			m_lightValues.Vector[i] = DirectX::XMFLOAT4A(
				directionalLight->GetCamera()->GetDirection().x,
				directionalLight->GetCamera()->GetDirection().y,
				directionalLight->GetCamera()->GetDirection().z,
				p_lightQueue->at(i)->GetIntensity());
		}
	}


	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		p_renderingManager->GetRTVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		*p_renderingManager->GetFrameIndex(),
		*p_renderingManager->GetRTVDescriptorSize());

	p_renderingManager->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	p_renderingManager->GetCommandList()->SetPipelineState(m_pipelineState);
	p_renderingManager->GetCommandList()->SetGraphicsRootSignature(m_rootSignature);
	p_renderingManager->GetCommandList()->RSSetViewports(1, &m_viewport);
	p_renderingManager->GetCommandList()->RSSetScissorRects(1, &m_rect);
	p_renderingManager->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_lightBuffer->Copy(&m_lightValues, sizeof(m_lightValues));
	m_lightBuffer->SetGraphicsRootConstantBufferView(4);

	for (UINT i = 0; i < this->m_renderTargetSize; i++)
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { m_geometryRenderTargetView[i]->GetTextureDescriptorHeap() };
		p_renderingManager->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		p_renderingManager->GetCommandList()->SetGraphicsRootDescriptorTable(i, m_geometryRenderTargetView[i]->GetTextureDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	}

	UINT index = 0;
	m_shaderResourceView->BeginCopy();
	for (UINT i = 0; i < m_shadowMaps->size() && i < MAX_SHADOWS && index < MAX_SHADOWS; i++)
	{
		const D3D12_RESOURCE_DESC desc = m_shadowMaps->at(i)->Resource->GetDesc();
		UINT currentMatrix = 0;
		for (UINT pos = index; pos < desc.DepthOrArraySize + index; pos++)
		{
			m_shadowValues.ViewProjection[pos] = m_shadowMaps->at(i)->ViewProjection[currentMatrix++];
		}
		m_shaderResourceView->CopySubresource(index, m_shadowMaps->at(i)->Resource);
		index += m_shadowMaps->at(i)->Resource->GetDesc().DepthOrArraySize;
	}
	m_shaderResourceView->EndCopy();


	m_shadowValues.values.x = index;
	
	m_shadowBuffer->Copy(&m_shadowValues, sizeof(m_shadowValues));
	m_shadowBuffer->SetGraphicsRootConstantBufferView(5);
	
	m_shaderResourceView->SetGraphicsRootDescriptorTable(6);

	ID3D12DescriptorHeap* descriptorHeaps[] = { this->m_ssao->GetTextureDescriptorHeap() };
	p_renderingManager->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	p_renderingManager->GetCommandList()->SetGraphicsRootDescriptorTable(7, this->m_ssao->GetTextureDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
}

void DeferredRender::Draw()
{
	p_renderingManager->GetCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	p_renderingManager->GetCommandList()->DrawInstanced(4, 1, 0, 0);
}

void DeferredRender::Clear()
{
	this->p_drawQueue->clear();
	this->p_lightQueue->clear();

	for (UINT i = 0; i < this->m_shadowMaps->size(); i++)
	{
		SAFE_DELETE(this->m_shadowMaps->at(i));
	}
	this->m_shadowMaps->clear();
}

void DeferredRender::Release()
{
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);

	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_vertexHeapBuffer);

	m_lightBuffer->Release();
	SAFE_DELETE(m_lightBuffer);
	SAFE_DELETE(m_shadowMaps);

	m_shaderResourceView->Release();
	SAFE_DELETE(m_shaderResourceView);

	m_shadowBuffer->Release();
	SAFE_DELETE(m_shadowBuffer);

}

void DeferredRender::SetRenderTarget(X12RenderTargetView** renderTarget, const UINT& size)
{
	this->m_geometryRenderTargetView = renderTarget;
	this->m_renderTargetSize = size;
}

void DeferredRender::AddShadowMap(
	ID3D12Resource* resource,
	ID3D12DescriptorHeap* map, 
	DirectX::XMFLOAT4X4A const* ViewProjection) const
{
	ShadowMap * sm = nullptr;
	SAFE_NEW(sm, new ShadowMap());
	sm->Resource = resource;
	sm->Map = map;
	for (UINT i = 0; i < resource->GetDesc().DepthOrArraySize; i++)
	{
		sm->ViewProjection[i] = ViewProjection[i];
	}

	m_shadowMaps->push_back(sm);
}

void DeferredRender::SetSSAO(X12RenderTargetView* renderTarget)
{
	this->m_ssao = renderTarget;
}


HRESULT DeferredRender::_preInit()
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
						if (SUCCEEDED(hr = _createQuadBuffer()))
						{
							if (SUCCEEDED(hr = m_lightBuffer->CreateBuffer(L"LightBuffer", &m_lightValues, sizeof(LightBuffer))))
							{
								if (SUCCEEDED(hr = m_shadowBuffer->CreateBuffer(L"Deferred Shadow matrix", &m_shadowValues, sizeof(ShadowLightBuffer))))
								{
									if (SUCCEEDED(hr = m_shaderResourceView->CreateShaderResourceView(
										SHADOW_MAP_SIZE,
										SHADOW_MAP_SIZE,
										MAX_SHADOWS,
										DXGI_FORMAT_R32_FLOAT)))
									{
										
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return hr;
}

HRESULT DeferredRender::_initID3D12RootSignature()
{
	D3D12_DESCRIPTOR_RANGE worldPosRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE worldPosTable;
	RenderingHelpClass::CreateRootDescriptorTable(worldPosRangeTable, worldPosTable, 0);

	D3D12_DESCRIPTOR_RANGE albedoRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE albedoTable1;
	RenderingHelpClass::CreateRootDescriptorTable(albedoRangeTable, albedoTable1, 1);

	D3D12_DESCRIPTOR_RANGE normalRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE normalTable;
	RenderingHelpClass::CreateRootDescriptorTable(normalRangeTable, normalTable, 2);

	D3D12_DESCRIPTOR_RANGE metallicRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE metallicTable;
	RenderingHelpClass::CreateRootDescriptorTable(metallicRangeTable, metallicTable, 3);

	D3D12_DESCRIPTOR_RANGE shadowRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE shadowTable;
	RenderingHelpClass::CreateRootDescriptorTable(shadowRangeTable, shadowTable, 0, 1);

	D3D12_DESCRIPTOR_RANGE ssaoRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE ssaoTable;
	RenderingHelpClass::CreateRootDescriptorTable(ssaoRangeTable, ssaoTable, 4, 0);

	D3D12_ROOT_DESCRIPTOR lightRootDescriptor;
	lightRootDescriptor.RegisterSpace = 0;
	lightRootDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR shadowRootDescriptor;
	shadowRootDescriptor.RegisterSpace = 1;
	shadowRootDescriptor.ShaderRegister = 0;

	m_rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[0].DescriptorTable = worldPosTable;
	m_rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[1].DescriptorTable = albedoTable1;
	m_rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[2].DescriptorTable = normalTable;
	m_rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[3].DescriptorTable = metallicTable;
	m_rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[4].Descriptor = lightRootDescriptor;
	m_rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[5].Descriptor = shadowRootDescriptor;
	m_rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[6].DescriptorTable = shadowTable;
	m_rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[7].DescriptorTable = ssaoTable;
	m_rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC sampler{};
	RenderingHelpClass::CreateSampler(sampler, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC shadowSampler{};
	RenderingHelpClass::CreateSampler(shadowSampler, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0, 0,
		D3D12_COMPARISON_FUNC_LESS_EQUAL);

	D3D12_STATIC_SAMPLER_DESC samplers[] = { sampler, shadowSampler };

	HRESULT hr = 0;
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameters),
		m_rootParameters,
		_countof(samplers),
		samplers,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob * signature = nullptr;
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		nullptr)))
	{
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature))))
		{
			SAFE_RELEASE(m_rootSignature);
		}
	}
	SAFE_RELEASE(signature);
	return hr;
}

HRESULT DeferredRender::_initShaders()
{
	HRESULT hr;
	ID3DBlob * blob = nullptr;

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/DeferredPass/DefaultDeferredVertex.hlsl", blob, "vs_5_1")))
	{
		return hr;
	}
	else
	{
		m_vertexShader.BytecodeLength = blob->GetBufferSize();
		m_vertexShader.pShaderBytecode = blob->GetBufferPointer();
	}

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/DeferredPass/DefaultDeferredPixel.hlsl", blob, "ps_5_1")))
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

HRESULT DeferredRender::_initID3D12PipelineState()
{
	HRESULT hr;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	m_inputLayoutDesc.pInputElementDescs = inputLayout;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = m_inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
	graphicsPipelineStateDesc.PS = m_pixelShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

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

HRESULT DeferredRender::_createViewport()
{
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<FLOAT>(p_window->GetWidth());
	m_viewport.Height = static_cast<FLOAT>(p_window->GetHeight());
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_rect.left = 0;
	m_rect.top = 0;
	m_rect.right = p_window->GetWidth();
	m_rect.bottom = p_window->GetHeight();
	return S_OK;
}

HRESULT DeferredRender::_createQuadBuffer()
{
	m_vertexBufferSize = sizeof(m_vertexList);

	HRESULT hr = 0;
	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer))))
	{	

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexHeapBuffer))))
		{

			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = reinterpret_cast<void*>(m_vertexList);
			vertexData.RowPitch = m_vertexBufferSize;
			vertexData.SlicePitch = m_vertexBufferSize;

			UpdateSubresources(p_renderingManager->GetCommandList(), m_vertexBuffer, m_vertexHeapBuffer, 0, 0, 1, &vertexData);

			p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		}
	}

	return hr;
}
