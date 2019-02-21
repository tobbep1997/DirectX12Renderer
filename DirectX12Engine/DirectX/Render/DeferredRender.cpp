#include "DirectX12EnginePCH.h"
#include "DeferredRender.h"
#include "WrapperFunctions/X12RenderTargetView.h"
#include "WrapperFunctions/RenderingHelpClass.h"
#include "WrapperFunctions/X12ConstantBuffer.h"
#include "WrapperFunctions/X12ShaderResourceView.h"

#define SHADOW_SPACE	1
#define LIGHT_SPACE		2

#define WORLD_POS	0
#define ALBEDO		1
#define NORMAL		2
#define METALLIC	3

#define SHADOW_BUFFER	4
#define SHADOW_TEXTURE	5

#define SSAO_TEXTURE	6

#define REFLECTION_TEXTURE 7

#define LIGHT_BUFFER 8
#define LIGHT_TABLE 9

DeferredRender::DeferredRender(RenderingManager* renderingManager, const Window& window)
	: IRender(renderingManager, window)
{
	m_vertexList[0] = Vertex(DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0, 1, 0, 0));
	m_vertexList[1] = Vertex(DirectX::XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0, 0, 0, 0));
	m_vertexList[2] = Vertex(DirectX::XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1, 1, 0, 0));
	m_vertexList[3] = Vertex(DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1, 0, 0, 0));

	SAFE_NEW(m_lightBuffer, new X12ConstantBuffer(p_renderingManager, *p_window));
	SAFE_NEW(m_lightTable, new X12ConstantBuffer(p_renderingManager, *p_window));
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

void DeferredRender::Update(const Camera& camera, const float& deltaTime)
{

	ID3D12GraphicsCommandList* commandList = p_renderingManager->GetCommandList();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		p_renderingManager->GetRTVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		*p_renderingManager->GetFrameIndex(),
		*p_renderingManager->GetRTVDescriptorSize());

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	commandList->SetPipelineState(m_pipelineState);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_rect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	

	_copyLightData(camera);
	m_lightBuffer->SetGraphicsRootConstantBufferView(LIGHT_BUFFER, 0, commandList);
	m_lightTable->SetGraphicsRootShaderResourceView(LIGHT_TABLE, 0, commandList);

	for (UINT i = 0; i < this->m_renderTargetSize; i++)
	{
		m_geometryRenderTargetView[i]->SetGraphicsRootDescriptorTable(i, commandList);
	}

	UINT index = 0;
	m_shaderResourceView->BeginCopy(commandList);
	for (UINT i = 0; i < m_shadowMaps->size() && i < MAX_SHADOWS && index < MAX_SHADOWS; i++)
	{
		const D3D12_RESOURCE_DESC desc = m_shadowMaps->at(i)->Resource->GetDesc();
		UINT currentMatrix = 0;
		for (UINT pos = index; pos < desc.DepthOrArraySize + index; pos++)
		{
			m_shadowValues.ViewProjection[pos] = m_shadowMaps->at(i)->ViewProjection[currentMatrix++];
		}
		m_shaderResourceView->CopySubresource(index, m_shadowMaps->at(i)->Resource, commandList);
		index += m_shadowMaps->at(i)->Resource->GetDesc().DepthOrArraySize;
	}
	m_shaderResourceView->EndCopy(commandList);


	m_shadowValues.Values.x = index;

	m_shadowBuffer->Copy(&m_shadowValues, sizeof(m_shadowValues));
	m_shadowBuffer->SetGraphicsRootConstantBufferView(SHADOW_BUFFER, 0, commandList);

	m_shaderResourceView->SetGraphicsRootDescriptorTable(SHADOW_TEXTURE, commandList);

	if (m_ssao)
		m_ssao->SetGraphicsRootDescriptorTable(SSAO_TEXTURE, commandList);
	
	if (m_reflection)
		m_reflection->SetGraphicsRootDescriptorTable(REFLECTION_TEXTURE, commandList);
}

void DeferredRender::Draw()
{
	ID3D12GraphicsCommandList* commandList = p_renderingManager->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	commandList->DrawInstanced(4, 1, 0, 0);
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

	m_lightTable->Release();
	SAFE_DELETE(m_lightTable);

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

void DeferredRender::SetReflection(X12RenderTargetView* renderTarget)
{
	this->m_reflection = renderTarget;
}

void DeferredRender::AddShadowMap(
	ID3D12Resource* resource,
	DirectX::XMFLOAT4X4A const* ViewProjection) const
{
	ShadowMap* sm = nullptr;
	SAFE_NEW(sm, new ShadowMap());
	sm->Resource = resource;
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
	if (FAILED(hr = p_renderingManager->OpenCommandList()))
	{
		return hr;
	}
	if (FAILED(hr = _initID3D12RootSignature()))
	{
		return hr;
	}
	if (FAILED(hr = _initShaders()))
	{
		return hr;
	}
	if (FAILED(hr = _initID3D12PipelineState()))
	{
		return hr;
	}
	if (FAILED(hr = _createViewport()))
	{
		return hr;
	}
	if (FAILED(hr = _createQuadBuffer()))
	{
		return hr;
	}
	if (FAILED(hr = m_lightBuffer->CreateBuffer(
		L"LightBuffer", 
		nullptr, 
		0,
		256)))
	{
		return hr;
	}
	if (FAILED(hr = m_lightTable->CreateBuffer(
		L"LightTable",
		nullptr,
		0,
		1024 * 64)))
	{		
		return hr;
	}
	if (FAILED(hr = m_shadowBuffer->CreateBuffer(
		L"Deferred Shadow matrix", 
		&m_shadowValues, 
		sizeof(ShadowLightBuffer))))
	{
		return hr;
	}
	if (FAILED(hr = m_shaderResourceView->CreateShaderResourceView(
		SHADOW_MAP_SIZE,
		SHADOW_MAP_SIZE,
		MAX_SHADOWS,
		DXGI_FORMAT_R32_FLOAT)))
	{
		return hr;
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

	D3D12_DESCRIPTOR_RANGE reflectionRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE reflectionTable;
	RenderingHelpClass::CreateRootDescriptorTable(reflectionRangeTable, reflectionTable, 6);

	D3D12_DESCRIPTOR_RANGE shadowRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE shadowTable;
	RenderingHelpClass::CreateRootDescriptorTable(shadowRangeTable, shadowTable, 0, SHADOW_SPACE);

	D3D12_DESCRIPTOR_RANGE ssaoRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE ssaoTable;
	RenderingHelpClass::CreateRootDescriptorTable(ssaoRangeTable, ssaoTable, 4, 0);
	   
	D3D12_ROOT_DESCRIPTOR LightBuffer;
	LightBuffer.RegisterSpace = LIGHT_SPACE;
	LightBuffer.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR LightTable;
	LightTable.RegisterSpace = LIGHT_SPACE;
	LightTable.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR shadowRootDescriptor;
	shadowRootDescriptor.RegisterSpace = SHADOW_SPACE;
	shadowRootDescriptor.ShaderRegister = 0;

	m_rootParameters[WORLD_POS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[WORLD_POS].DescriptorTable = worldPosTable;
	m_rootParameters[WORLD_POS].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[ALBEDO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[ALBEDO].DescriptorTable = albedoTable1;
	m_rootParameters[ALBEDO].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[NORMAL].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[NORMAL].DescriptorTable = normalTable;
	m_rootParameters[NORMAL].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[METALLIC].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[METALLIC].DescriptorTable = metallicTable;
	m_rootParameters[METALLIC].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	   
	m_rootParameters[SHADOW_BUFFER].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[SHADOW_BUFFER].Descriptor = shadowRootDescriptor;
	m_rootParameters[SHADOW_BUFFER].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[SHADOW_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[SHADOW_TEXTURE].DescriptorTable = shadowTable;
	m_rootParameters[SHADOW_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[SSAO_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[SSAO_TEXTURE].DescriptorTable = ssaoTable;
	m_rootParameters[SSAO_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[REFLECTION_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[REFLECTION_TEXTURE].DescriptorTable = reflectionTable;
	m_rootParameters[REFLECTION_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[LIGHT_BUFFER].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[LIGHT_BUFFER].Descriptor = LightBuffer;
	m_rootParameters[LIGHT_BUFFER].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[LIGHT_TABLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	m_rootParameters[LIGHT_TABLE].Descriptor = LightTable;
	m_rootParameters[LIGHT_TABLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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

	D3D12_STATIC_SAMPLER_DESC samplers[] = {sampler, shadowSampler};

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

	ID3DBlob* signature = nullptr;
	if (		SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		nullptr)))
	{
		if (			FAILED(hr = p_renderingManager->GetDevice()->CreateRootSignature(
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
	ID3DBlob* blob = nullptr;

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
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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

			UpdateSubresources(p_renderingManager->GetCommandList(), m_vertexBuffer, m_vertexHeapBuffer, 0, 0, 1,
			                   &vertexData);

			p_renderingManager->GetCommandList()->ResourceBarrier(
				1, &CD3DX12_RESOURCE_BARRIER::Transition(
					m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		}
	}

	return hr;
}

void DeferredRender::_copyLightData(const Camera& camera) const
{
	LightStructuredBuffer values;

	PointLight * pl;
	DirectionalLight * dl;

	struct L_BUFFER
	{
		DirectX::XMFLOAT4 CameraPos;
		DirectX::XMUINT4 NumLights;
	} lBuffer;

	lBuffer.CameraPos = camera.GetPosition();
	lBuffer.NumLights.x = static_cast<UINT>(p_lightQueue->size());

	m_lightBuffer->Copy(&lBuffer, sizeof(lBuffer));

	for (UINT i = 0; i < p_lightQueue->size(); i++)
	{
		values.Position = p_lightQueue->at(i)->GetPosition();
		values.Color = p_lightQueue->at(i)->GetColor();

		values.Type.x = p_lightQueue->at(i)->GetType();
		values.Type.y = p_lightQueue->at(i)->GetType();
		values.Type.z = p_lightQueue->at(i)->GetType();
		values.Type.w = p_lightQueue->at(i)->GetType();

		if ((pl = dynamic_cast<PointLight*>(p_lightQueue->at(i))))
		{
			values.PointLight = DirectX::XMFLOAT4(pl->GetIntensity(), pl->GetDropOff(), pl->GetPow(), pl->GetRadius());
		}
		if ((dl = dynamic_cast<DirectionalLight*>(p_lightQueue->at(i))))
		{
			values.Direction = DirectX::XMFLOAT4A(
				dl->GetCamera()->GetDirection().x,
				dl->GetCamera()->GetDirection().y,
				dl->GetCamera()->GetDirection().z,
				p_lightQueue->at(i)->GetIntensity());
		
		}
		m_lightTable->Copy(&values, sizeof(LightStructuredBuffer), i * sizeof(LightStructuredBuffer));
	}
}
