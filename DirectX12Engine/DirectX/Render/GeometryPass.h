#pragma once
#include "Template/IRender.h"


class GeometryPass :
	public IRender
{
private:

	static const UINT ROOT_PARAMETERS = 7;
	static const UINT NUM_BUFFERS = 2;

	struct ObjectBuffer
	{
		DirectX::XMFLOAT4A		CameraPosition;
		DirectX::XMFLOAT4X4A	WorldMatrix;
		DirectX::XMFLOAT4X4A	ViewProjection;
		
		DirectX::XMFLOAT4A		Padding[40];
	};

	struct LightBuffer
	{
		DirectX::XMFLOAT4A	CameraPosition;
		DirectX::XMUINT4	Type[256];
		DirectX::XMFLOAT4A	Position[256];
		DirectX::XMFLOAT4A	Color[256];
		DirectX::XMFLOAT4A	Vector[256];
	};
public:
	GeometryPass(RenderingManager * renderingManager, const Window & window);
	~GeometryPass();
	
	
	HRESULT Init() override;
	HRESULT Update(const Camera & camera) override;
	HRESULT Draw() override;
	HRESULT Clear() override;
	HRESULT Release() override;

private:
	HRESULT _preInit();
	HRESULT _signalGPU();

	HRESULT _initID3D12RootSignature();
	HRESULT _initID3D12PipelineState();
	HRESULT _initShaders();
	HRESULT _createViewport();
	HRESULT _createDepthStencil();
	HRESULT _createConstantBuffer();
	

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC  m_inputLayoutDesc;

	ID3D12Resource		* m_depthStencilBuffer  = nullptr;
	ID3D12DescriptorHeap* m_depthStencilDescriptorHeap = nullptr;

	D3D12_VIEWPORT	m_viewport;
	D3D12_RECT		m_rect;

	D3D12_SHADER_BYTECODE m_vertexShader;
	D3D12_SHADER_BYTECODE m_hullShader;
	D3D12_SHADER_BYTECODE m_domainShader;
	D3D12_SHADER_BYTECODE m_pixelShader;

	D3D12_ROOT_PARAMETER  m_rootParameters[ROOT_PARAMETERS] {};


	//0 Camera
	//1 Lights
	ID3D12DescriptorHeap	* m_constantBufferDescriptorHeap[NUM_BUFFERS][FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12Resource			* m_constantBuffer[NUM_BUFFERS][FRAME_BUFFER_COUNT] = { nullptr };

	ObjectBuffer m_objectValues {};
	int m_constantBufferPerObjectAlignedSize = (sizeof(ObjectBuffer) + 255) & ~255;

	LightBuffer m_lightValues{};


	UINT8* m_cameraBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };
	UINT8* m_lightBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };
	   	
	struct Vertex
	{
		Vertex(const DirectX::XMFLOAT4 & position = DirectX::XMFLOAT4(0,0,0,0), 
			const DirectX::XMFLOAT4 & color = DirectX::XMFLOAT4(0,0,0,0))
		{
			this->Position = DirectX::XMFLOAT4(position);
			this->Color = DirectX::XMFLOAT4(color);
		}
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Color;
	};
};

