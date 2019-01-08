#include "DirectX12EnginePCH.h"
#include "ShadowPass.h"
#include "WrapperFunctions/X12DepthStencil.h"
#include "WrapperFunctions/X12RenderTargetView.h"
#include "GeometryPass.h"
#include "DeferredRender.h"


ShadowPass::ShadowPass(RenderingManager* renderingManager, const Window& window)
	: IRender(renderingManager, window)
{
}

ShadowPass::~ShadowPass()
{
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
	OpenCommandList();
	const UINT lightQueueSize = static_cast<UINT>(p_lightQueue->size());
	UINT counter = 0;
	for (UINT i = 0; i < lightQueueSize; i++)
	{
		if (dynamic_cast<DirectionalLight*>(p_lightQueue->at(i)))
		{
			DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(p_lightQueue->at(i));
			m_lightValues.LightType.x = directionalLight->GetType();
			m_lightValues.LightViewProjection[0] = directionalLight->GetCamera()->GetViewProjectionMatrix();

			memcpy(m_constantLightBufferGPUAddress[*p_renderingManager->GetFrameIndex()] + counter++ * m_constantLightBufferPerObjectAlignedSize, &m_lightValues, sizeof(m_lightValues));
		}
		else if (dynamic_cast<PointLight*>(p_lightQueue->at(i)))
		{
			PointLight* directionalLight = dynamic_cast<PointLight*>(p_lightQueue->at(i));
			m_lightValues.LightType.x = directionalLight->GetType();
			for (UINT j = 0; j < 6; j++)
			{
				m_lightValues.LightViewProjection[j] = directionalLight->GetCameras()[j]->GetViewProjectionMatrix();
			}
			memcpy(m_constantLightBufferGPUAddress[*p_renderingManager->GetFrameIndex()] + counter++ * m_constantLightBufferPerObjectAlignedSize, &m_lightValues, sizeof(m_lightValues));
		}
	}
	   
	p_commandList->SetPipelineState(m_pipelineState);
	p_commandList->SetGraphicsRootSignature(m_rootSignature);
	p_commandList->RSSetViewports(1, &m_viewport);
	p_commandList->RSSetScissorRects(1, &m_rect);
	p_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
}

void ShadowPass::Draw()
{
	const UINT lightQueueSize = static_cast<UINT>(p_lightQueue->size());

	UINT counter = 0;
	for (UINT i = 0; i < lightQueueSize; i++)
	{
		p_lightQueue->at(i)->GetDepthStencil()->SwitchToDSV(p_commandList);
		p_lightQueue->at(i)->GetDepthStencil()->ClearDepthStencil(p_commandList);

		const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(p_lightQueue->at(i)->GetDepthStencil()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			p_lightQueue->at(i)->GetRenderTargetView()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
			*p_renderingManager->GetFrameIndex(),
			p_lightQueue->at(i)->GetRenderTargetView()->GetDescriptorSize());
		p_lightQueue->at(i)->GetRenderTargetView()->Clear(rtvHandle, p_commandList);
		
		p_commandList->OMSetRenderTargets(p_lightQueue->at(i)->GetNumRenderTargets(), &rtvHandle, TRUE, &dsvHandle);
		
		p_commandList->SetGraphicsRootConstantBufferView(0, m_constantLightBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + counter * m_constantLightBufferPerObjectAlignedSize);
		
		p_drawInstance();

		counter++;

		p_lightQueue->at(i)->GetDepthStencil()->SwitchToSRV(p_commandList);

		if (dynamic_cast<DirectionalLight*>(p_lightQueue->at(i)))
		{
			DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(p_lightQueue->at(i));			

			DirectX::XMFLOAT4X4A arr[1] = { directionalLight->GetCamera()->GetViewProjectionMatrix() };

			p_renderingManager->GetDeferredRender()->AddShadowMap(
				directionalLight->GetDepthStencil()->GetResource(),
				directionalLight->GetDepthStencil()->GetTextureDescriptorHeap(),
				arr);
		}
		else if (dynamic_cast<PointLight*>(p_lightQueue->at(i)))
		{
			PointLight* pointLight = dynamic_cast<PointLight*>(p_lightQueue->at(i));

			DirectX::XMFLOAT4X4A arr[6];
			for (UINT k = 0; k < 6; k++)
			{
				arr[k] = pointLight->GetCameras()[k]->GetViewProjectionMatrix();
			}

			p_renderingManager->GetDeferredRender()->AddShadowMap(
				pointLight->GetDepthStencil()->GetResource(),
				pointLight->GetDepthStencil()->GetTextureDescriptorHeap(),
				arr);			
		}
	}
	ExecuteCommandList();
}

void ShadowPass::Clear()
{
	p_drawQueue->clear();
	p_lightQueue->clear();
	Instancing::ClearInstanceGroup(p_instanceGroups);
}

void ShadowPass::Release()
{
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);
	   
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_constantLightBuffer[i]);
	}
	p_releaseInstanceBuffer();
	p_releaseCommandList();
}

HRESULT ShadowPass::_preInit()
{
	HRESULT hr = 0;

	_createViewport();
	if (SUCCEEDED(hr = p_createCommandList()))
	{
		if (SUCCEEDED(hr = OpenCommandList()))
		{
			if (SUCCEEDED(hr = _initRootSignature()))
			{
				if (SUCCEEDED(hr = _initShaders()))
				{
					if (SUCCEEDED(hr = _initPipelineState()))
					{
						if (SUCCEEDED(hr = _createConstantBuffer()))
						{
							if (SUCCEEDED(hr = p_createInstanceBuffer()))
							{
								
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

	if (SUCCEEDED(hr = p_renderingManager->SignalGPU(p_commandList)))
	{	}

	return hr;
}

HRESULT ShadowPass::_initRootSignature()
{
	HRESULT hr = 0;
	   
	D3D12_ROOT_DESCRIPTOR lightDescriptor;
	lightDescriptor.RegisterSpace = 0;
	lightDescriptor.ShaderRegister = 0;

	m_rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameter[0].Descriptor = lightDescriptor;
	m_rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameter),
		m_rootParameter,
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT	|				
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS			|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS		|				
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

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/ShadowPass/DefaultShadowGeometry.hlsl", blob, "gs_5_1")))
	{
		return hr;
	}
	else
	{
		m_geometryShader.BytecodeLength = blob->GetBufferSize();
		m_geometryShader.pShaderBytecode = blob->GetBufferPointer();
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
		{ "TEXCORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
	graphicsPipelineStateDesc.GS = m_geometryShader; 
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.NumRenderTargets = 6;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.RTVFormats[4] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.RTVFormats[5] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.RasterizerState = 
	CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE, 0, 0.0f, 0.0f, TRUE, FALSE, FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
	graphicsPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
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
			IID_PPV_ARGS(&m_constantLightBuffer[i]))))
		{
			SET_NAME(m_constantLightBuffer[i], L"Shadow ConstantLightBuffer Upload Resource Heap");
		}
		else
			return hr;

		CD3DX12_RANGE readRange(0, 0);
		if (SUCCEEDED(hr = m_constantLightBuffer[i]->Map(
			0, &readRange,
			reinterpret_cast<void**>(&m_constantLightBufferGPUAddress[i]))))
		{
			memcpy(m_constantLightBufferGPUAddress[i], &m_lightValues, sizeof(LightBuffer));
		}
	}
	return hr;
}

void ShadowPass::_createViewport()
{
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width	= static_cast<FLOAT>(SHADOW_MAP_SIZE);
	m_viewport.Height	= static_cast<FLOAT>(SHADOW_MAP_SIZE);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_rect.left = 0;
	m_rect.top = 0;
	m_rect.right = SHADOW_MAP_SIZE;
	m_rect.bottom = SHADOW_MAP_SIZE;
}
