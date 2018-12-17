#pragma once
#include "Template/IRender.h"

class GeometryPass :
	public IRender
{
private:
	IDXGISwapChain * m_swapChain;

public:
	GeometryPass(ID3D12Device * device, IDXGISwapChain * swapChain);
	~GeometryPass();
	
	
	HRESULT Init() override;
	HRESULT Update() override;
	HRESULT Draw() override;
	HRESULT Release() override;

private:
	HRESULT _initID3D12RootSignature();
	HRESULT _initInputLayout(D3D12_INPUT_LAYOUT_DESC * inputLayoutDesc);
	HRESULT _initID3D12PipelineState(const D3D12_INPUT_LAYOUT_DESC & inputLayoutDesc);
	HRESULT _initShaders();

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;
	ID3D12Resource		* m_vertexBuffer  = nullptr;


	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_VIEWPORT	m_viewport;
	D3D12_RECT		m_rect;

	D3D12_SHADER_BYTECODE m_vertexShader;
	D3D12_SHADER_BYTECODE m_pixelShader;

	struct Vertex
	{
		DirectX::XMFLOAT4 position;
	};
};

