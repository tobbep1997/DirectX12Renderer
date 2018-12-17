#include "DirectX12EnginePCH.h"
#include "GeometryPass.h"


GeometryPass::GeometryPass(ID3D12Device * device, IDXGISwapChain * swapChain) : IRender(device)
{
	if (!device)
		Window::CreateError(L"Geometry pass: Missing ID3D12Device");
	if (!swapChain)
		Window::CreateError(L"Geometry pass: Missing IDXGISwapChain");

	this->m_swapChain = swapChain;

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

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	if (FAILED(hr = _initInputLayout(&inputLayoutDesc)))
	{
		return hr;
	}
	if (FAILED(hr = _initID3D12PipelineState(inputLayoutDesc)))
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

HRESULT GeometryPass::_initInputLayout(D3D12_INPUT_LAYOUT_DESC * inputLayoutDesc)
{
	const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	

	inputLayoutDesc->NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc->pInputElementDescs = inputLayout;
	   
	return S_OK;
}

HRESULT GeometryPass::_initID3D12PipelineState(const D3D12_INPUT_LAYOUT_DESC & inputLayoutDesc)
{
	HRESULT hr = 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
	graphicsPipelineStateDesc.PS = m_pixelShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
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
	if (FAILED(hr = ShaderCreator::CreateShader(
		L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryVertex.hlsl",
		&this->m_vertexShader,
		"vs_5_1")))
	{
		return hr;
	}

	if (FAILED(hr = ShaderCreator::CreateShader(
		L"../DirectX12Engine/DirectX/Shaders/GeometryPass/DefaultGeometryPixel.hlsl",
		&this->m_pixelShader,
		"ps_5_1")))
	{
		return hr;
	}
	return hr;
}
