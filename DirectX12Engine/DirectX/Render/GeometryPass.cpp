#include "DirectX12EnginePCH.h"
#include "GeometryPass.h"


GeometryPass::GeometryPass(ID3D12Device * device, 
	IDXGISwapChain * swapChain, 
	ID3D12GraphicsCommandList * commandList) : IRender(device)
{
	if (!device)
		Window::CreateError(L"Geometry pass: Missing ID3D12Device");
	if (!swapChain)
		Window::CreateError(L"Geometry pass: Missing IDXGISwapChain");
	if (!commandList)
		Window::CreateError(L"Geometry pass: Missing ID3D12GraphicsCommandList");

	this->m_swapChain = swapChain;
	this->m_commandList = commandList;
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

	return hr;
}

HRESULT GeometryPass::Update()
{
	HRESULT hr = 0;

	return hr;
}

HRESULT GeometryPass::Draw()
{
	HRESULT hr = 0;
	return hr;
}

HRESULT GeometryPass::Release()
{
	HRESULT hr = 0;

	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_vertexBuffer);

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
		if (FAILED(hr = p_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature))))
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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	graphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		FALSE, 
		0, 0, 0, 
		FALSE, 
		FALSE, 
		FALSE, 
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
	graphicsPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.NumRenderTargets = 1;

	DXGI_SWAP_CHAIN_DESC desc;
	if (SUCCEEDED(hr = m_swapChain->GetDesc(&desc)))
	{
		graphicsPipelineStateDesc.SampleDesc = desc.SampleDesc;
	}
	else
		return hr;

	if (FAILED(hr = p_device->CreateGraphicsPipelineState(
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
	//if (FAILED(hr = ShaderCreator::CreateShader(
	//	L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryVertex.hlsl",
	//	m_vertexShader,
	//	"vs_5_1")))
	//{
	//	return hr;
	//}

	//if (FAILED(hr = ShaderCreator::CreateShader(
	//	L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryPixel.hlsl",
	//	m_pixelShader,
	//	"ps_5_1")))
	//{
	//	return hr;
	//}
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

	return hr;
}
