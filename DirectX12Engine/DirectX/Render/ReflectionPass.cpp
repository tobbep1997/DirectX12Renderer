#include "DirectX12EnginePCH.h"
#include "ReflectionPass.h"
#include "WrapperFunctions/RenderingHelpClass.h"
#include "WrapperFunctions/X12RenderTargetView.h"


ReflectionPass::ReflectionPass(RenderingManager * renderingManager, const Window & window) : IRender(renderingManager, window)
{
	m_vertexList[0] = Vertex(DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0, 1, 0, 0));
	m_vertexList[1] = Vertex(DirectX::XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0, 0, 0, 0));
	m_vertexList[2] = Vertex(DirectX::XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1, 1, 0, 0));
	m_vertexList[3] = Vertex(DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1, 0, 0, 0));
}


ReflectionPass::~ReflectionPass()
{
}

HRESULT ReflectionPass::Init()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = _preInit()))
	{
		if (SUCCEEDED(hr = p_renderingManager->SignalGPU(p_commandList[*p_renderingManager->GetFrameIndex()])))
		{
			
		}
	}
	if (FAILED(hr))
	{
		this->Release();
	}
	return hr;
}

void ReflectionPass::Update(const Camera& camera, const float& deltaTime)
{
	OpenCommandList(m_pipelineState);
	m_cameraBuffer->Copy(&camera.GetPosition(), sizeof(DirectX::XMFLOAT4));

	ID3D12GraphicsCommandList * commandList = p_commandList[*p_renderingManager->GetFrameIndex()];

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_renderTargetView->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		*p_renderingManager->GetFrameIndex(),
		m_renderTargetView->GetDescriptorSize());

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_rect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

}

void ReflectionPass::Draw()
{
	ID3D12GraphicsCommandList * commandList = p_commandList[*p_renderingManager->GetFrameIndex()];

	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	commandList->DrawInstanced(4, 1, 0, 0);

	ExecuteCommandList();
}

void ReflectionPass::Clear()
{
}

void ReflectionPass::Release()
{
	m_cameraBuffer->Release();
	SAFE_DELETE(m_cameraBuffer);

	m_renderTargetView->Release();
	SAFE_DELETE(m_renderTargetView);

	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_vertexHeapBuffer);

	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);


	p_releaseCommandList();
}

HRESULT ReflectionPass::_preInit()
{
	HRESULT hr = 0;
	if (FAILED(hr = p_createCommandList(L"Reflection")))
	{
		return hr;
	}
	if (FAILED(hr = OpenCommandList()))
	{
		return hr;
	}
	if (FAILED(hr = _initRootSignature()))
	{
		return hr;
	}
	if (FAILED(hr = _initShaders()))
	{
		return hr;
	}
	if (FAILED(hr = _initPipelineState()))
	{
		return hr;
	}
	if (FAILED(hr = _createQuadBuffer()))
	{
		return hr;
	}

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = m_vertexBufferSize;

	_createViewPort();
	SAFE_NEW(m_renderTargetView, new X12RenderTargetView(p_renderingManager, *p_window));
	if (FAILED(hr = m_renderTargetView->CreateRenderTarget(0, 0, 1, FALSE, DXGI_FORMAT_R32G32B32A32_FLOAT)))
	{
		return hr;
	}

	SAFE_NEW(m_cameraBuffer, new X12ConstantBuffer(p_renderingManager, *p_window));
	if (FAILED(hr = m_cameraBuffer->CreateBuffer(
		L"Reflection Camera", 
		nullptr, 
		sizeof(DirectX::XMFLOAT4), 
		1024 * 64)))
	{
		return hr;
	}

	return hr;
}

HRESULT ReflectionPass::_initRootSignature()
{
	D3D12_ROOT_DESCRIPTOR camera;
	camera.RegisterSpace = 0;
	camera.ShaderRegister = 0;

	m_rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[0].Descriptor = camera;
	m_rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC sampler{};
	RenderingHelpClass::CreateSampler(sampler, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	HRESULT hr = 0;
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameters),
		m_rootParameters,
		1,
		&sampler,
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

HRESULT ReflectionPass::_initShaders()
{
	HRESULT hr;
	ID3DBlob * blob = nullptr;

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/ReflectionPass/DefaultReflectionVertex.hlsl", blob, "vs_5_1")))
	{
		return hr;
	}
	else
	{
		m_vertexShader.BytecodeLength = blob->GetBufferSize();
		m_vertexShader.pShaderBytecode = blob->GetBufferPointer();
	}

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/ReflectionPass/DefaultReflectionPixel.hlsl", blob, "ps_5_1")))
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

HRESULT ReflectionPass::_initPipelineState()
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
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

void ReflectionPass::_createViewPort()
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
}

HRESULT ReflectionPass::_createQuadBuffer()
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

			UpdateSubresources(p_commandList[*p_renderingManager->GetFrameIndex()], m_vertexBuffer, m_vertexHeapBuffer, 0, 0, 1, &vertexData);

			p_commandList[*p_renderingManager->GetFrameIndex()]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		}
	}

	return hr;
}
