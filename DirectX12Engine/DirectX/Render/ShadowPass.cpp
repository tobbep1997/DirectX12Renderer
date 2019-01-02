#include "DirectX12EnginePCH.h"
#include "ShadowPass.h"
#include "WrapperFunctions/X12DepthStencil.h"
#include "WrapperFunctions/X12ConstantBuffer.h"
#include "GeometryPass.h"


ShadowPass::ShadowPass(RenderingManager* renderingManager, const Window& window)
	: IRender(renderingManager, window)
{
	m_depthStencil = new X12DepthStencil(renderingManager, window);
	m_lightConstantBuffer = new X12ConstantBuffer(renderingManager, window);
}

ShadowPass::~ShadowPass()
{
	delete m_depthStencil;
	m_depthStencil = nullptr;

	delete m_lightConstantBuffer;
	m_lightConstantBuffer = nullptr;
}

HRESULT ShadowPass::Init()
{
	HRESULT hr = 0;

	if (SUCCEEDED(hr = _preInit()))
	{
		if (SUCCEEDED(hr = _signalGPU()))
		{
			
		}
	}

	return hr;	
}

void ShadowPass::Update(const Camera& camera)
{

	const UINT drawQueueSize = static_cast<UINT>(p_drawQueue->size());
	for (UINT i = 0; i < drawQueueSize; i++)
	{
		m_objectValues.WorldMatrix = p_drawQueue->at(i)->GetWorldMatrix();
		memcpy(m_constantBufferGPUAddress[*p_renderingManager->GetFrameIndex()] + i * m_constantBufferPerObjectAlignedSize, &m_objectValues, sizeof(m_objectValues));
	}

	const UINT lightQueueSize = static_cast<UINT>(p_lightQueue->size());
	for (UINT i = 0; i < lightQueueSize; i++)
	{
		if (dynamic_cast<DirectionalLight*>(p_lightQueue->at(i)))
		{
			DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(p_lightQueue->at(i));

			m_lightValues.LightViewProjection[i] = directionalLight->GetCamera()->GetViewProjectionMatrix();
		}
	}
	

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencil->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		*p_renderingManager->GetFrameIndex(),
		m_rtvDescriptorSize);

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	p_renderingManager->GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_depthStencil->ClearDepthStencil();


	p_renderingManager->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	p_renderingManager->GetCommandList()->SetPipelineState(m_pipelineState);
	p_renderingManager->GetCommandList()->SetGraphicsRootSignature(m_rootSignature);
	p_renderingManager->GetCommandList()->RSSetViewports(1, &m_viewport);
	p_renderingManager->GetCommandList()->RSSetScissorRects(1, &m_rect);
	p_renderingManager->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_lightConstantBuffer->Copy(&m_lightValues, sizeof(m_lightValues));
	m_lightConstantBuffer->SetGraphicsRootConstantBufferView(1);
}

void ShadowPass::Draw()
{
	const UINT drawQueueSize = static_cast<UINT>(p_drawQueue->size());
	for (UINT i = 0; i < drawQueueSize; i++)
	{
		p_renderingManager->GetCommandList()->IASetVertexBuffers(0, 1, &p_drawQueue->at(i)->GetMesh().GetVertexBufferView());

		p_renderingManager->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_constantBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + i * m_constantBufferPerObjectAlignedSize);

		p_renderingManager->GetCommandList()->DrawInstanced(static_cast<UINT>(p_drawQueue->at(i)->GetMesh().GetStaticMesh().size()), 1, 0, 0);
	}
	p_renderingManager->GetGeometryPass()->AddShadowMap(m_depthStencil->GetTextureResource(), m_depthStencil->GetTextureDescriptorHeap(), dynamic_cast<DirectionalLight*>(p_lightQueue->at(0))->GetCamera()->GetViewProjectionMatrix());
}

void ShadowPass::Clear()
{
	p_drawQueue->clear();
	p_lightQueue->clear();
}

void ShadowPass::Release()
{
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);

	SAFE_RELEASE(m_rtvDescriptorHeap);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_renderTargets[i]);
		SAFE_RELEASE(m_constantBuffer[i]);
	}

	m_depthStencil->Release();
	m_lightConstantBuffer->Release();
}

HRESULT ShadowPass::_preInit()
{
	HRESULT hr = 0;

	_createViewport();
	if (SUCCEEDED(hr = p_renderingManager->OpenCommandList()))
	{
		if (SUCCEEDED(hr = _createRenderTarget()))
		{
			if (SUCCEEDED(hr = _initRootSignature()))
			{
				if (SUCCEEDED(hr = _initShaders()))
				{
					if (SUCCEEDED(hr = _initPipelineState()))
					{
						if (SUCCEEDED(hr = _createConstantBuffer()))
						{						
							if (SUCCEEDED(hr = m_depthStencil->CreateDepthStencil(L"Shadow", m_width, m_height, TRUE)))
							{
								if (SUCCEEDED(hr = m_lightConstantBuffer->CreateBuffer(L"Shadow", &m_lightValues, sizeof(LightBuffer))))
								{
									
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

HRESULT ShadowPass::_signalGPU() const
{
	HRESULT hr;

	if (SUCCEEDED(hr = p_renderingManager->SignalGPU()))
	{	}

	return hr;
}

HRESULT ShadowPass::_createRenderTarget()
{
	HRESULT hr = 0;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Alignment = 0;
	resourceDesc.DepthOrArraySize = 3;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Width = m_width;
	resourceDesc.Height = m_height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = p_renderingManager->GetDevice()->GetResourceAllocationInfo(0, 1, &resourceDesc);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12Heap * heap = nullptr;
	
	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_rtvDescriptorHeap))))
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_HEAP_DESC heapDesc{};
		heapDesc.Alignment = allocationInfo.Alignment;
		heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		heapDesc.Properties = heapProperties;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		depthOptimizedClearValue.Color[0] = 0.0f;
		depthOptimizedClearValue.Color[1] = 0.0f;
		depthOptimizedClearValue.Color[2] = 0.0f;
		depthOptimizedClearValue.Color[3] = 0.0f;

		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateHeap(
			&heapDesc,
			IID_PPV_ARGS(&heap))))
		{
			m_rtvDescriptorSize = p_renderingManager->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

			for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
			{
				if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreatePlacedResource(
					heap,
					0,
					&CD3DX12_RESOURCE_DESC::Tex2D(
						DXGI_FORMAT_R8G8B8A8_UNORM,
						m_width,
						m_height,
						1,0,1,0,
						D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					&depthOptimizedClearValue,
					IID_PPV_ARGS(&m_renderTargets[i]))))
				{
					p_renderingManager->GetDevice()->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
					rtvHandle.Offset(1, m_rtvDescriptorSize);
				}
			}
		}
	}
	SAFE_RELEASE(heap);
	return hr;
}



HRESULT ShadowPass::_initRootSignature()
{
	HRESULT hr = 0;

	D3D12_ROOT_DESCRIPTOR objectDescriptor;
	objectDescriptor.RegisterSpace = 0;
	objectDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR lightDescriptor;
	lightDescriptor.RegisterSpace = 0;
	lightDescriptor.ShaderRegister = 1;

	m_rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameter[0].Descriptor = objectDescriptor;
	m_rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	m_rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameter[1].Descriptor = lightDescriptor;
	m_rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameter),
		m_rootParameter,
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT	|				
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS			|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS		|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS		|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

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

HRESULT ShadowPass::_initShaders()
{
	HRESULT hr = 0;
	ID3DBlob * blob = nullptr;

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/ShadowPass/DefaultShadowVertex.hlsl", blob, "vs_5_1")))
	{
		return hr;
	}
	else
	{
		m_vertexShader.BytecodeLength = blob->GetBufferSize();
		m_vertexShader.pShaderBytecode = blob->GetBufferPointer();
	}
	return hr;
}

HRESULT ShadowPass::_initPipelineState()
{
	HRESULT hr = 0;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
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

HRESULT ShadowPass::_createConstantBuffer()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer[i]))))
		{
			SET_NAME(m_constantBuffer[i], L"Shadow ConstantBuffer Upload Resource Heap");
		}
		else
			return hr;

		CD3DX12_RANGE readRange(0, 0);
		if (SUCCEEDED(hr = m_constantBuffer[i]->Map(
			0, & readRange,
			reinterpret_cast<void**>(&m_constantBufferGPUAddress[i]))))
		{
			memcpy(m_constantBufferGPUAddress[i], &m_objectValues, sizeof(m_objectValues));
		}
	}
	return hr;
}

void ShadowPass::_createViewport()
{
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width	= static_cast<FLOAT>(m_width);
	m_viewport.Height	= static_cast<FLOAT>(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_rect.left = 0;
	m_rect.top = 0;
	m_rect.right = m_width;
	m_rect.bottom = m_height;
}
