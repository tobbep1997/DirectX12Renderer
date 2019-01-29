#include "DirectX12EnginePCH.h"
#include "GeometryPass.h"
#include "WrapperFunctions/RenderingHelpClass.h"
#include "WrapperFunctions/X12DepthStencil.h"
#include "WrapperFunctions/X12ConstantBuffer.h"
#include "WrapperFunctions/X12RenderTargetView.h"
#include "DeferredRender.h"
#include "WrapperFunctions/X12ShaderResourceView.h"
#include "SSAOPass.h"

GeometryPass::GeometryPass(RenderingManager * renderingManager, 
	const Window & window) :
	IRender(renderingManager, window)
{
	if (!renderingManager)
		Window::CreateError("GeometryPass : Missing RenderingManager");
	m_inputLayoutDesc = {};
	SAFE_NEW(m_emitters, new std::vector<ParticleEmitter*>());
}


GeometryPass::~GeometryPass()
{
	SAFE_DELETE(m_emitters);
}

HRESULT GeometryPass::Init()
{
	HRESULT hr;
	if (SUCCEEDED(hr = _preInit()))
	{
		if (FAILED(hr = _signalGPU()))
		{
			return hr;
		}
	}
	return hr;
}

void GeometryPass::Update(const Camera & camera, const float & deltaTime)
{	
	OpenCommandList();
	m_cameraValues.CameraPosition = DirectX::XMFLOAT4A(camera.GetPosition().x,
		camera.GetPosition().y,
		camera.GetPosition().z,
		camera.GetPosition().w);
	m_cameraValues.ViewProjection = camera.GetViewProjectionMatrix();

	m_depthStencil->SwitchToDSV(p_commandList[*p_renderingManager->GetFrameIndex()]);
	m_depthStencil->ClearDepthStencil(p_commandList[*p_renderingManager->GetFrameIndex()]);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencil->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());

	

	D3D12_CPU_DESCRIPTOR_HANDLE d12CpuDescriptorHandle[RENDER_TARGETS];
	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		m_renderTarget[i]->SwitchToRTV(p_commandList[*p_renderingManager->GetFrameIndex()]);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			m_renderTarget[i]->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
			*p_renderingManager->GetFrameIndex(),
			m_renderTarget[i]->GetDescriptorSize());

		m_renderTarget[i]->Clear(rtvHandle, p_commandList[*p_renderingManager->GetFrameIndex()]);
		d12CpuDescriptorHandle[i] = rtvHandle;
	}

	p_commandList[*p_renderingManager->GetFrameIndex()]->OMSetRenderTargets(4, d12CpuDescriptorHandle, FALSE, &dsvHandle);

	p_commandList[*p_renderingManager->GetFrameIndex()]->SetPipelineState(m_pipelineState);
	p_commandList[*p_renderingManager->GetFrameIndex()]->SetGraphicsRootSignature(m_rootSignature);
	p_commandList[*p_renderingManager->GetFrameIndex()]->RSSetViewports(1, &m_viewport);
	p_commandList[*p_renderingManager->GetFrameIndex()]->RSSetScissorRects(1, &m_rect);
	p_commandList[*p_renderingManager->GetFrameIndex()]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	m_cameraBuffer->Copy(&m_cameraValues, sizeof(m_cameraValues));
	m_cameraBuffer->SetGraphicsRootConstantBufferView(0, p_commandList[*p_renderingManager->GetFrameIndex()]);
	m_cameraBuffer->SetGraphicsRootConstantBufferView(1, p_commandList[*p_renderingManager->GetFrameIndex()]);
}

void GeometryPass::Draw()
{	
	p_drawInstance(2, TRUE);
	const size_t emitterSize = m_emitters->size();

	if (emitterSize)
	{
		p_commandList[*p_renderingManager->GetFrameIndex()]->SetPipelineState(m_particlePipelineState);
		p_commandList[*p_renderingManager->GetFrameIndex()]->SetGraphicsRootSignature(m_particleRootSignature);
		p_commandList[*p_renderingManager->GetFrameIndex()]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_cameraBuffer->SetGraphicsRootConstantBufferView(0, p_commandList[*p_renderingManager->GetFrameIndex()]);
		
		ParticleEmitter * emitter = nullptr;
		for (size_t i = 0; i < emitterSize; i++)
		{
			emitter = m_emitters->at(i);
			emitter->GetShaderResourceView()->SetGraphicsRootDescriptorTable(1, p_commandList[*p_renderingManager->GetFrameIndex()]);
			p_commandList[*p_renderingManager->GetFrameIndex()]->IASetVertexBuffers(0, 1, &emitter->GetVertexBufferView());

			p_commandList[*p_renderingManager->GetFrameIndex()]->DrawInstanced(
				emitter->GetVertexSize(),
				1, 0, 0);
		}

	}

	m_depthStencil->SwitchToSRV(p_commandList[*p_renderingManager->GetFrameIndex()]);

	p_renderingManager->GetDeferredRender()->SetRenderTarget(m_renderTarget, RENDER_TARGETS);
	p_renderingManager->GetSSAOPass()->SetWorldPos(m_renderTarget[0]);
	p_renderingManager->GetSSAOPass()->SetDepthStencil(m_depthStencil);
	
	ExecuteCommandList();
}

void GeometryPass::Clear()
{
	this->p_drawQueue->clear();
	this->p_lightQueue->clear();
	this->m_emitters->clear();
	Instancing::ClearInstanceGroup(p_instanceGroups);
}

void GeometryPass::Release()
{	
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);

	SAFE_RELEASE(m_particlePipelineState);
	SAFE_RELEASE(m_particleRootSignature);

	m_depthStencil->Release();
	SAFE_DELETE(m_depthStencil);

	p_releaseInstanceBuffer();
	p_releaseCommandList();


	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		m_renderTarget[i]->Release();
		SAFE_DELETE(m_renderTarget[i]);
	}

	m_cameraBuffer->Release();
	SAFE_DELETE(m_cameraBuffer);
}

void GeometryPass::AddEmitter(ParticleEmitter* emitter) const
{
	m_emitters->push_back(emitter);
}

HRESULT GeometryPass::_preInit()
{
	HRESULT hr;

	if (SUCCEEDED(hr = p_createCommandList(L"Geometry")))
	{
		if (SUCCEEDED(hr = OpenCommandList()))
		{
			if (SUCCEEDED(hr = _initID3D12RootSignature()))
			{
				if (SUCCEEDED(hr = _initShaders()))
				{
					if (SUCCEEDED(hr = _initID3D12PipelineState()))
					{
						if (SUCCEEDED(hr = _createViewport()))
						{
							SAFE_NEW(m_depthStencil, new X12DepthStencil(p_renderingManager, *p_window, nullptr));
							if (SUCCEEDED(hr = m_depthStencil->CreateDepthStencil(L"Geometry",
								0, 0,
								1, TRUE)))
							{								
								for (UINT i = 0; i < RENDER_TARGETS; i++)									
									SAFE_NEW(m_renderTarget[i], new X12RenderTargetView(p_renderingManager, *p_window, nullptr));
								for (UINT i = 0; i < RENDER_TARGETS; i++)
								{
									if (FAILED(hr = m_renderTarget[i]->CreateRenderTarget(
										0, 0,
										1,
										TRUE,
										RENDER_TARGET_FORMAT)))
									{
										return hr;
									}
								}
								if (SUCCEEDED(hr = p_createInstanceBuffer(L"Geometry")))
								{
									SAFE_NEW(m_cameraBuffer, new X12ConstantBuffer(p_renderingManager, *p_window, nullptr));

									if (SUCCEEDED(hr = m_cameraBuffer->CreateBuffer(
										L"Geometry camera",
										&m_cameraValues,
										sizeof(CameraBuffer))))
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
	if (FAILED(hr))
		this->Release();
	return hr;
}

HRESULT GeometryPass::_signalGPU() const
{
	HRESULT hr;

	if (SUCCEEDED(hr = p_renderingManager->SignalGPU(p_commandList[*p_renderingManager->GetFrameIndex()])))
	{	}

	return hr;
}

HRESULT GeometryPass::_initID3D12RootSignature()
{
	HRESULT hr;
	
	D3D12_DESCRIPTOR_RANGE albedoRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE albedoTable;
	RenderingHelpClass::CreateRootDescriptorTable(albedoRangeTable, albedoTable, 0);

	D3D12_DESCRIPTOR_RANGE normalRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE normalTable;
	RenderingHelpClass::CreateRootDescriptorTable(normalRangeTable, normalTable, 1);
	
	D3D12_DESCRIPTOR_RANGE metallicRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE metallicTable;
	RenderingHelpClass::CreateRootDescriptorTable(metallicRangeTable, metallicTable, 2);	

	D3D12_DESCRIPTOR_RANGE displacementRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE displacementTable;
	RenderingHelpClass::CreateRootDescriptorTable(displacementRangeTable, displacementTable, 0);

	D3D12_DESCRIPTOR_RANGE displacementNormalRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE displacementNormalTable;
	RenderingHelpClass::CreateRootDescriptorTable(displacementNormalRangeTable, displacementNormalTable, 1);

	D3D12_ROOT_DESCRIPTOR rootDescriptor;
	rootDescriptor.RegisterSpace = 0;
	rootDescriptor.ShaderRegister = 0;

	m_rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[0].Descriptor = rootDescriptor;
	m_rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	m_rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[1].Descriptor = rootDescriptor;
	m_rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;

	m_rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[2].DescriptorTable = albedoTable;
	m_rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[3].DescriptorTable = normalTable;
	m_rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	
	m_rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[4].DescriptorTable = metallicTable;
	m_rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[5].DescriptorTable = displacementTable;
	m_rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;

	m_rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[6].DescriptorTable = displacementNormalTable;
	m_rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
		   
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

	D3D12_STATIC_SAMPLER_DESC domainSampler{};
	RenderingHelpClass::CreateSampler(domainSampler, 0, 0, D3D12_SHADER_VISIBILITY_DOMAIN);
	   
	D3D12_STATIC_SAMPLER_DESC samplers[] = { sampler, shadowSampler, domainSampler };

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameters),
		m_rootParameters, 
		3, 
		samplers,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |		
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

	D3D12_DESCRIPTOR_RANGE particleRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE particleTable;
	RenderingHelpClass::CreateRootDescriptorTable(particleRangeTable, particleTable, 0, 0);

	D3D12_ROOT_DESCRIPTOR particleRootDescriptor;
	particleRootDescriptor.RegisterSpace = 0;
	particleRootDescriptor.ShaderRegister = 0;

	m_particleRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_particleRootParameters[0].Descriptor = particleRootDescriptor;
	m_particleRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	m_particleRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_particleRootParameters[1].DescriptorTable = particleTable;
	m_particleRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC particleRootSignatureDesc;
	particleRootSignatureDesc.Init(_countof(m_particleRootParameters),
		m_particleRootParameters,
		1,
		&sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
	
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&particleRootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		nullptr)))
	{
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_particleRootSignature))))
		{
			SAFE_RELEASE(m_particleRootSignature);
		}
	}
	SAFE_RELEASE(signature);

	return hr;
}

HRESULT GeometryPass::_initID3D12PipelineState()
{
	HRESULT hr;

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

	m_inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	m_inputLayoutDesc.pInputElementDescs = inputLayout;
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = m_inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
	graphicsPipelineStateDesc.HS = m_hullShader;
	graphicsPipelineStateDesc.DS = m_domainShader;
	graphicsPipelineStateDesc.PS = m_pixelShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	graphicsPipelineStateDesc.NumRenderTargets = RENDER_TARGETS;

	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		graphicsPipelineStateDesc.RTVFormats[i] = RENDER_TARGET_FORMAT;
	}
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE, FALSE, 0, 0.0f, 0.0f, TRUE, FALSE, FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
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

	D3D12_INPUT_ELEMENT_DESC particleInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	m_inputLayoutDesc.NumElements = sizeof(particleInputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	m_inputLayoutDesc.pInputElementDescs = particleInputLayout;
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC particleGraphicsPipelineStateDesc = {};
	particleGraphicsPipelineStateDesc.InputLayout = m_inputLayoutDesc;
	particleGraphicsPipelineStateDesc.pRootSignature = m_particleRootSignature;
	particleGraphicsPipelineStateDesc.VS = m_particleVertexShader;
	particleGraphicsPipelineStateDesc.PS = m_particlePixelShader;
	particleGraphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	particleGraphicsPipelineStateDesc.NumRenderTargets = RENDER_TARGETS;
	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		particleGraphicsPipelineStateDesc.RTVFormats[i] = RENDER_TARGET_FORMAT;

	}
	particleGraphicsPipelineStateDesc.SampleMask = 0xffffffff;
	particleGraphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE, FALSE, 0, 0.0f, 0.0f, TRUE, FALSE, FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
	particleGraphicsPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	particleGraphicsPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	particleGraphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	particleGraphicsPipelineStateDesc.SampleDesc = desc.SampleDesc;

	if (FAILED(hr = p_renderingManager->GetDevice()->CreateGraphicsPipelineState(
		&particleGraphicsPipelineStateDesc,
		IID_PPV_ARGS(&m_particlePipelineState))))
	{
		SAFE_RELEASE(m_particlePipelineState);
	}

	return hr;
}

HRESULT GeometryPass::_initShaders()
{
	HRESULT hr;
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

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryHull.hlsl", blob, "hs_5_1")))
	{
		return hr;
	}
	else
	{
		m_hullShader.BytecodeLength = blob->GetBufferSize();
		m_hullShader.pShaderBytecode = blob->GetBufferPointer();
	}

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryDomain.hlsl", blob, "ds_5_1")))
	{
		return hr;
	}
	else
	{
		m_domainShader.BytecodeLength = blob->GetBufferSize();
		m_domainShader.pShaderBytecode = blob->GetBufferPointer();
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

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryParticleVertex.hlsl", blob, "vs_5_1")))
	{
		return hr;
	}
	else
	{
		m_particleVertexShader.BytecodeLength = blob->GetBufferSize();
		m_particleVertexShader.pShaderBytecode = blob->GetBufferPointer();
	}

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryParticlePixel.hlsl", blob, "ps_5_1")))
	{
		return hr;
	}
	else
	{
		m_particlePixelShader.BytecodeLength = blob->GetBufferSize();
		m_particlePixelShader.pShaderBytecode = blob->GetBufferPointer();
	}

	return hr;
}

HRESULT GeometryPass::_createViewport()
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