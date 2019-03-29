#pragma once
#include "Template/IRender.h"

class X12ConstantBuffer;

class SSAOPass : //NOLINT
	public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 3;
	static const UINT BLUR_ROOT_PARAMETERS = 1;

	const DXGI_FORMAT RENDER_TARGET_FORMAT = DXGI_FORMAT_R32G32B32A32_FLOAT;

	struct CameraBuffer
	{
		DirectX::XMFLOAT4X4A ViewProjection;
	};

public:
	SSAOPass(RenderingManager * renderingManager, const Window & window);
	~SSAOPass();


	HRESULT Init() override;
	void Update(const Camera& camera, const float& deltaTime) override;
	void Draw() override;
	void Clear() override;
	void Release() override;

	void SetWorldPos(X12RenderTargetView * renderTargetView);
	void SetDepthStencil(X12DepthStencil * depthStencil);

private:

	HRESULT _preInit();
	HRESULT _initID3D12RootSignature();
	HRESULT _initShaders();
	HRESULT _initID3D12PipelineState();
	HRESULT _initBlurPass();

	HRESULT _createLocalCommandList();
	HRESULT _openCommandList();
	HRESULT _executeCommandList() const;

	void _createViewport();

	HRESULT _createQuadBuffer();

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
	ID3D12Resource * m_vertexBuffer = nullptr;
	ID3D12Resource * m_vertexHeapBuffer = nullptr;

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_rect{};

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;
	D3D12_ROOT_PARAMETER  m_rootParameters[ROOT_PARAMETERS]{};

	D3D12_SHADER_BYTECODE m_vertexShader{};
	D3D12_SHADER_BYTECODE m_pixelShader{};
	D3D12_INPUT_LAYOUT_DESC  m_inputLayoutDesc{};

	ID3D12CommandAllocator * m_blurCommandAllocator[FRAME_BUFFER_COUNT] {nullptr};
	ID3D12GraphicsCommandList * m_blurCommandList[FRAME_BUFFER_COUNT] = { nullptr };

	ID3D12RootSignature * m_blurRootSignature = nullptr;
	ID3D12PipelineState * m_blurPipelineState = nullptr;

	D3D12_INPUT_LAYOUT_DESC  m_blurInputLayoutDesc{};

	D3D12_SHADER_BYTECODE m_blurVertex{};
	D3D12_SHADER_BYTECODE m_blurPixel {};
	D3D12_ROOT_PARAMETER m_blurRootParameter[BLUR_ROOT_PARAMETERS]{};

	X12RenderTargetView * m_blurRenderTarget = nullptr;

	CameraBuffer m_cameraValues{};
	X12ConstantBuffer * m_cameraBuffer = nullptr;
	X12RenderTargetView * m_renderTarget = nullptr;

	X12DepthStencil * m_depthStencils = nullptr;
	X12RenderTargetView * m_worldPos = nullptr;

	X12Fence * m_fence = nullptr;

	struct Vertex
	{
		Vertex()
		{
			this->Position = { 0,0,0,0 };
			this->Uv = { 0,0,0,0 };
		}
		Vertex(const DirectX::XMFLOAT4 & position, const DirectX::XMFLOAT4 & uv)
		{
			this->Position = position;
			this->Uv = uv;
		}
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Uv;
	};
	UINT m_vertexBufferSize = 0;

	Vertex m_vertexList[4];

};

