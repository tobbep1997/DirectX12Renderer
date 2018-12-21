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
	HRESULT _preInit();
	HRESULT _signalGPU();

	HRESULT _initID3D12RootSignature();
	HRESULT _initID3D12PipelineState();
	HRESULT _initShaders();
	HRESULT _createViewport();
	HRESULT _createDepthStencil();

	HRESULT _createVertexBuffer();
	HRESULT _createIndexBuffer();

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC  m_inputLayoutDesc;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW  m_indexBufferView;

	UINT				  m_vertexBufferSize = 0;
	ID3D12Resource		* m_vertexBuffer		= nullptr;
	ID3D12Resource		* m_vertexHeapBuffer	= nullptr;

	UINT				  m_indexBufferSize = 0;
	ID3D12Resource		* m_indexBuffer			= nullptr;
	ID3D12Resource		* m_indexHeapBuffer		= nullptr;

	ID3D12Resource		* m_depthStencilBuffer  = nullptr;
	ID3D12DescriptorHeap* m_depthStencilDescritorHeap = nullptr;

	D3D12_VIEWPORT	m_viewport;
	D3D12_RECT		m_rect;

	D3D12_SHADER_BYTECODE m_vertexShader;
	D3D12_SHADER_BYTECODE m_pixelShader;

	struct Vertex
	{
		Vertex(const DirectX::XMFLOAT4 & position, const DirectX::XMFLOAT4 & color)
		{
			this->position = DirectX::XMFLOAT4(position);
			this->color = DirectX::XMFLOAT4(color);
		}
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
	};
};

