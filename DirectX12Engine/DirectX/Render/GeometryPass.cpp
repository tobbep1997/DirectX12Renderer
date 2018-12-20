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
{
}

HRESULT GeometryPass::Init()
{
	HRESULT hr = 0;
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
	if (FAILED(hr = _createTriagnle()))
	{
		return hr;
	}
	if (FAILED(hr = _createViewport()))
	{
		return hr;
	}
	
	return hr;
}

HRESULT GeometryPass::Update()
{
	HRESULT hr = 0;

	return hr;
}

HRESULT GeometryPass::Draw()
{
	p_renderingManager->GetCommandList()->ClearDepthStencilView(
		m_depthStencilDescritorHeap->GetCPUDescriptorHandleForHeapStart(), 
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0,
		nullptr);

	HRESULT hr = 0;
	p_renderingManager->GetCommandList()->SetPipelineState(m_pipelineState);
	p_renderingManager->GetCommandList()->SetGraphicsRootSignature(m_rootSignature);
	p_renderingManager->GetCommandList()->RSSetViewports(1, &m_viewport);
	p_renderingManager->GetCommandList()->RSSetScissorRects(1, &m_rect);
	p_renderingManager->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	p_renderingManager->GetCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	p_renderingManager->GetCommandList()->IASetIndexBuffer(&m_indexBufferView);
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencilDescritorHeap->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		p_renderingManager->GetRTVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		*p_renderingManager->GetFrameIndex(),
		*p_renderingManager->GetRTVDescriptorSize());

	p_renderingManager->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	p_renderingManager->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
	p_renderingManager->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 4, 0);
	return hr;
}

HRESULT GeometryPass::Release()
{
	HRESULT hr = 0;

	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_vertexHeapBuffer);
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_indexHeapBuffer);
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_depthStencilDescritorHeap);
	return hr;
}

HRESULT GeometryPass::_initID3D12RootSignature()
{
	HRESULT hr = 0;
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob * signature;
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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

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
	ID3DBlob * blob;
	if (SUCCEEDED(hr = D3DCompileFromFile(
		L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryVertex.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_1",
		0,
		0,
		&blob,
		nullptr
	)))
	{
		m_vertexShader.BytecodeLength = blob->GetBufferSize();
		m_vertexShader.pShaderBytecode = blob->GetBufferPointer();
	}

	if (SUCCEEDED(hr = D3DCompileFromFile(
		L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryPixel.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_1",
		0,
		0,
		&blob,
		nullptr
	)))
	{
		m_pixelShader.BytecodeLength = blob->GetBufferSize();
		m_pixelShader.pShaderBytecode = blob->GetBufferPointer();
	}
	return hr;
}

HRESULT GeometryPass::_createTriagnle()
{
	HRESULT hr = 0;

	Vertex vList[] = {
		{ { -0.5f,  0.5f, 0.5f, 1.0f },	{1.0f, 0.0f, 0.0f, 1.0f} },
		{ {  0.5f, -0.5f, 0.5f, 1.0f },	{0.0f, 1.0f, 0.0f, 1.0f} },
		{ { -0.5f, -0.5f, 0.5f, 1.0f }, {0.0f, 0.0f, 1.0f, 1.0f} },
		{ {  0.5f,  0.5f, 0.5f, 1.0f }, {1.0f, 1.0f, 1.0f, 1.0f} },

		// second quad (further from camera, green)
		{ { -0.75f,  0.75f, 0.75f, 1.0f },	{1.0f, 0.0f, 0.0f, 1.0f} },
		{ {  0.0f ,  0.0f , 0.75f, 1.0f },	{0.0f, 1.0f, 0.0f, 1.0f} },
		{ { -0.75f,  0.0f , 0.75f, 1.0f },  {0.0f, 0.0f, 1.0f, 1.0f} },
		{ {  0.0f ,  0.75f, 0.75f, 1.0f },  {1.0f, 1.0f, 1.0f, 1.0f} },
	};

	m_vertexBufferSize = sizeof(vList);

	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
		D3D12_HEAP_FLAG_NONE, 
		&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST, 
		nullptr, 
		IID_PPV_ARGS(&m_vertexBuffer))))
	{
		

		m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

		
		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexHeapBuffer))))
		{
			m_vertexHeapBuffer->SetName(L"Vertex Buffer Upload Resource Heap");
			

			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = reinterpret_cast<void*>(vList); 
			vertexData.RowPitch = m_vertexBufferSize;
			vertexData.SlicePitch = m_vertexBufferSize;

			UpdateSubresources(p_renderingManager->GetCommandList(), m_vertexBuffer, m_vertexHeapBuffer, 0, 0, 1, &vertexData);

			p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

			//--
			DWORD iList[] =
			{
				0, 1, 2,
				0, 3, 1
			};

			m_indexBufferSize = sizeof(iList);

			if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_indexBuffer))))

			{
				SET_NAME(m_indexBuffer, L"Index Buffer");

				if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&m_indexHeapBuffer))))
				{
					D3D12_SUBRESOURCE_DATA indexData = {};
					indexData.pData = reinterpret_cast<void*>(iList);
					indexData.RowPitch = m_indexBufferSize;
					indexData.SlicePitch = m_indexBufferSize;

					UpdateSubresources(p_renderingManager->GetCommandList(),
						m_indexBuffer,
						m_indexHeapBuffer,
						0, 0, 1,
						&indexData);

					p_renderingManager->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

					if (FAILED(hr = _createDepthStencil()))
					{
						return hr;
					}

					p_renderingManager->GetCommandList()->Close();
					ID3D12CommandList* ppCommandLists[] = { p_renderingManager->GetCommandList() };
					p_renderingManager->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

					p_renderingManager->GetFenceValues()[*p_renderingManager->GetFrameIndex()]++;
					if (SUCCEEDED(hr = p_renderingManager->GetCommandQueue()->Signal(
						&p_renderingManager->GetFence()[*p_renderingManager->GetFrameIndex()],
						p_renderingManager->GetFenceValues()[*p_renderingManager->GetFrameIndex()])))
					{
						m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
						m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
						m_indexBufferView.SizeInBytes = m_indexBufferSize;

						m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
						m_vertexBufferView.StrideInBytes = sizeof(Vertex);
						m_vertexBufferView.SizeInBytes = m_vertexBufferSize;



					}
				}
			}
		}
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
		&dsvHeapDesc, IID_PPV_ARGS(&m_depthStencilDescritorHeap))))
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
				m_depthStencilDescritorHeap->GetCPUDescriptorHandleForHeapStart());
			
				
			
		}
	}

	return hr;
}
