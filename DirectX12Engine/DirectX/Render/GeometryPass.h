#pragma once
#include "Template/IRender.h"

class GeometryPass :
	public IRender
{
public:
	GeometryPass(RenderingManager * renderingManager, const Window & window);
	~GeometryPass();
	
	
	HRESULT Init() override;
	HRESULT Update() override;
	HRESULT Draw() override;
	HRESULT Release() override;

private:
	HRESULT _initID3D12RootSignature();
	HRESULT _initID3D12PipelineState();
	HRESULT _initShaders();
	HRESULT _createTriagnle();
	HRESULT _createViewport();

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;
	ID3D12Resource		* m_vertexBuffer  = nullptr;
	ID3D12Resource		* m_heapBuffer	  = nullptr;

	D3D12_INPUT_LAYOUT_DESC m_inputLayoutDesc;
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

