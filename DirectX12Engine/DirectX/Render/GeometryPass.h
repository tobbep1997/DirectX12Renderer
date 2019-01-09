#pragma once
#include "Template/IRender.h"

class X12RenderTargetView;
class X12ConstantBuffer;
class X12DepthStencil;

class GeometryPass :
	public IRender
{
private:

	static const UINT ROOT_PARAMETERS = 10;
	static const UINT PARTICLE_ROOT_PARAMETERS = 1;
	static const UINT NUM_BUFFERS = 2;

	static const UINT RENDER_TARGETS = 4;
	static const DXGI_FORMAT RENDER_TARGET_FORMAT = DXGI_FORMAT_R32G32B32A32_FLOAT;

	struct CameraBuffer
	{
		DirectX::XMFLOAT4A		CameraPosition;
		DirectX::XMFLOAT4X4A	ViewProjection;
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
	void Update(const Camera & camera) override;
	void Draw() override;
	void Clear() override;
	void Release() override;

	void AddEmitter(ParticleEmitter * emitter);

private:
	HRESULT _preInit();
	HRESULT _signalGPU() const;

	HRESULT _initID3D12RootSignature();
	HRESULT _initID3D12PipelineState();
	HRESULT _initShaders();
	HRESULT _createViewport();	

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;
	D3D12_ROOT_PARAMETER  m_rootParameters[ROOT_PARAMETERS] {};

	ID3D12PipelineState * m_particlePipelineState = nullptr;
	ID3D12RootSignature * m_particleRootSignature = nullptr;
	D3D12_ROOT_PARAMETER m_particleRootParameters[PARTICLE_ROOT_PARAMETERS] {};

	D3D12_INPUT_LAYOUT_DESC  m_inputLayoutDesc;

	X12DepthStencil * m_depthStencil = nullptr;
	X12RenderTargetView * m_renderTarget[RENDER_TARGETS] = { nullptr };

	D3D12_VIEWPORT	m_viewport{};
	D3D12_RECT		m_rect{};

	D3D12_SHADER_BYTECODE m_vertexShader{};
	D3D12_SHADER_BYTECODE m_hullShader{};
	D3D12_SHADER_BYTECODE m_domainShader{};
	D3D12_SHADER_BYTECODE m_pixelShader{};


	CameraBuffer m_cameraValues {};
	X12ConstantBuffer * m_cameraBuffer = nullptr;
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

	std::vector<ParticleEmitter*>* m_emitters = nullptr;
};

