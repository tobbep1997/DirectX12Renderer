#include "DirectX12EnginePCH.h"
#include "GeometryPass.h"
#include "WrapperFunctions/RenderingHelpClass.h"

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

	m_objectValues.CameraPosition = DirectX::XMFLOAT4A(camera.GetPosition().x,
		camera.GetPosition().y,
		camera.GetPosition().z,
		camera.GetPosition().w);
	m_objectValues.ViewProjection = camera.GetViewProjectionMatrix();
	const UINT drawQueueSize = static_cast<UINT>(p_drawQueue->size());
	for (UINT i = 0; i < drawQueueSize; i++)
	{
		m_objectValues.WorldMatrix = p_drawQueue->at(i)->GetWorldMatrix();

		memcpy(m_cameraBufferGPUAddress[*p_renderingManager->GetFrameIndex()] + i * m_constantBufferPerObjectAlignedSize, &m_objectValues, sizeof(m_objectValues));
	}
	const UINT lightQueueSize = static_cast<const UINT>(p_lightQueue->size());
	for (UINT i = 0; i < lightQueueSize && i < 256; i++)
	{
		m_lightValues.CameraPosition = DirectX::XMFLOAT4A(camera.GetPosition().x,
			camera.GetPosition().y,
			camera.GetPosition().z,
			camera.GetPosition().w);
		m_lightValues.Type[i] = DirectX::XMUINT4(lightQueueSize, 0, 0, 0);
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

			m_lightValues.Type[i].y = 1;
			m_lightValues.Vector[i] = DirectX::XMFLOAT4A(pl->GetIntensity(),
				pl->GetDropOff(),
				pl->GetPow(),
				0);
		}
	}
	memcpy(m_lightBufferGPUAddress[*p_renderingManager->GetFrameIndex()], &m_lightValues, sizeof(m_lightValues));

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
	//p_renderingManager->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	p_renderingManager->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);


	p_renderingManager->GetCommandList()->SetGraphicsRootConstantBufferView(2, m_constantBuffer[1][*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress());
	return hr;
}

HRESULT GeometryPass::Draw()
{
	HRESULT hr = 0;
	const UINT drawQueueSize = static_cast<UINT>(p_drawQueue->size());
	for (UINT i = 0; i < drawQueueSize; i++)
	{	
		if (p_drawQueue->at(i)->GetTexture())
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { p_drawQueue->at(i)->GetTexture()->GetId3D12DescriptorHeap() };
			p_renderingManager->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			p_renderingManager->GetCommandList()->SetGraphicsRootDescriptorTable(3, p_drawQueue->at(i)->GetTexture()->GetId3D12DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		}
		if(p_drawQueue->at(i)->GetNormal())
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { p_drawQueue->at(i)->GetNormal()->GetId3D12DescriptorHeap() };
			p_renderingManager->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			p_renderingManager->GetCommandList()->SetGraphicsRootDescriptorTable(4, p_drawQueue->at(i)->GetNormal()->GetId3D12DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		}
		if (p_drawQueue->at(i)->GetMetallic())
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { p_drawQueue->at(i)->GetMetallic()->GetId3D12DescriptorHeap() };
			p_renderingManager->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			p_renderingManager->GetCommandList()->SetGraphicsRootDescriptorTable(5, p_drawQueue->at(i)->GetMetallic()->GetId3D12DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		}

		if (p_drawQueue->at(i)->GetDisplacement())
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { p_drawQueue->at(i)->GetDisplacement()->GetId3D12DescriptorHeap() };
			p_renderingManager->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			p_renderingManager->GetCommandList()->SetGraphicsRootDescriptorTable(6, p_drawQueue->at(i)->GetDisplacement()->GetId3D12DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		}

		p_renderingManager->GetCommandList()->IASetVertexBuffers(0, 1, &p_drawQueue->at(i)->GetMesh().GetVertexBufferView());		

		p_renderingManager->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_constantBuffer[0][*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + i * m_constantBufferPerObjectAlignedSize);
		p_renderingManager->GetCommandList()->SetGraphicsRootConstantBufferView(1, m_constantBuffer[0][*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + i * m_constantBufferPerObjectAlignedSize);
		
		p_renderingManager->GetCommandList()->DrawInstanced(static_cast<UINT>(p_drawQueue->at(i)->GetMesh().GetStaticMesh().size()), 1, 0, 0);
	}

	return hr;
}

HRESULT GeometryPass::Clear()
{
	this->p_drawQueue->clear();
	this->p_lightQueue->clear();
	return S_OK;
}

HRESULT GeometryPass::Release()
{
	HRESULT hr = 0;
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_depthStencilDescriptorHeap);
	for (UINT i = 0; i < NUM_BUFFERS; i++)
	{
		for (UINT j = 0; j < FRAME_BUFFER_COUNT; j++)
		{
		
			SAFE_RELEASE(m_constantBufferDescriptorHeap[i][j]);
			SAFE_RELEASE(m_constantBuffer[i][j]);
		}
	}

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
	{	}

	return hr;
}

HRESULT GeometryPass::_initID3D12RootSignature()
{
	HRESULT hr = 0;
	
	D3D12_DESCRIPTOR_RANGE descriptorRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	RenderingHelpClass::CreateRootDescriptorTable(descriptorRangeTable, descriptorTable, 0);

	D3D12_DESCRIPTOR_RANGE textureRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE textureTable;
	RenderingHelpClass::CreateRootDescriptorTable(textureRangeTable, textureTable, 1);
	
	D3D12_DESCRIPTOR_RANGE metallicRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE metallicTable;
	RenderingHelpClass::CreateRootDescriptorTable(metallicRangeTable, metallicTable, 2);

	D3D12_DESCRIPTOR_RANGE displacementRangeTable;
	D3D12_ROOT_DESCRIPTOR_TABLE displacementTable;
	RenderingHelpClass::CreateRootDescriptorTable(displacementRangeTable, displacementTable, 0);


	D3D12_ROOT_DESCRIPTOR rootDescriptor;
	rootDescriptor.RegisterSpace = 0;
	rootDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR lightRootDescriptor;
	lightRootDescriptor.RegisterSpace = 0;
	lightRootDescriptor.ShaderRegister = 0;

	m_rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[0].Descriptor = rootDescriptor;
	m_rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	m_rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[1].Descriptor = rootDescriptor;
	m_rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;

	m_rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[2].Descriptor = lightRootDescriptor;
	m_rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[3].DescriptorTable = descriptorTable;
	m_rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[4].DescriptorTable = textureTable;
	m_rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[5].DescriptorTable = metallicTable;
	m_rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	    
	m_rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameters[6].DescriptorTable = displacementTable;
	m_rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
	

	D3D12_STATIC_SAMPLER_DESC sampler{};
	RenderingHelpClass::CreateSampler(sampler, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	
	D3D12_STATIC_SAMPLER_DESC domainSampler{};
	RenderingHelpClass::CreateSampler(domainSampler, 0, 0, D3D12_SHADER_VISIBILITY_DOMAIN);
	   
	D3D12_STATIC_SAMPLER_DESC samplers[] = { sampler, domainSampler };

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameters),
		m_rootParameters, 
		2, 
		samplers,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |		
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
	graphicsPipelineStateDesc.HS = m_hullShader;
	graphicsPipelineStateDesc.DS = m_domainShader;
	graphicsPipelineStateDesc.PS = m_pixelShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_WIREFRAME,
			D3D12_CULL_MODE_NONE,
			FALSE,
			0,
			0.0f,
			0.0f,
			TRUE,
			FALSE,
			FALSE,
			0,
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
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
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_constantBufferDescriptorHeap[1][i]))))
		{
			return hr;
		}


		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer[1][i]))))
		{
			SET_NAME(m_constantBuffer[1][i], L"Light Upload Resource Heap");
		}
		else
			return hr;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBuffer[1][i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (sizeof(m_constantBuffer) + 255) & ~255;

		p_renderingManager->GetDevice()->CreateConstantBufferView(
			&cbvDesc,
			m_constantBufferDescriptorHeap[1][i]->GetCPUDescriptorHandleForHeapStart());

		CD3DX12_RANGE readRange(0, 0);
		if (SUCCEEDED(hr = m_constantBuffer[1][i]->Map(
			0, &readRange,
			reinterpret_cast<void **>(&m_lightBufferGPUAddress[i]))))
		{
			memcpy(m_lightBufferGPUAddress[i], &m_lightValues, sizeof(m_lightValues));
		}


		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer[0][i]))))
		{
			SET_NAME(m_constantBuffer[0][i], L"ConstantBuffer Upload Resource Heap");
		}
		else
			return hr;

		if (SUCCEEDED(hr = m_constantBuffer[0][i]->Map(
			0, &readRange,
			reinterpret_cast<void **>(&m_cameraBufferGPUAddress[i]))))
		{
			memcpy(m_cameraBufferGPUAddress[i], &m_objectValues, sizeof(m_objectValues));
		}
	}
	return hr;
}