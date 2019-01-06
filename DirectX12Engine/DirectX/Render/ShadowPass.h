#pragma once
#include "Template/IRender.h"

class X12DepthStencil;
class X12ConstantBuffer;
class X12RenderTargetView;

class ShadowPass :
	public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 2;

	struct ObjectBuffer
	{
		DirectX::XMFLOAT4X4A	WorldMatrix;

		DirectX::XMFLOAT4A		Padding[44];
	};
	struct LightBuffer
	{
		DirectX::XMUINT4 LightType;
		DirectX::XMFLOAT4X4A LightViewProjection[6];

		DirectX::XMFLOAT4A		Padding[43];
	};
public:
	ShadowPass(RenderingManager * renderingManager, const Window & window);
	~ShadowPass();

	HRESULT Init() override;
	void Update(const Camera& camera) override;
	void Draw() override;
	void Clear() override;
	void Release() override;

private:
	HRESULT _preInit();
	HRESULT _signalGPU() const;

	HRESULT _initRootSignature();
	HRESULT _initShaders();
	HRESULT _initPipelineState();
	HRESULT _createConstantBuffer();
	void _createViewport();
	
	X12ConstantBuffer * m_lightConstantBuffer = nullptr;

	ID3D12RootSignature *	m_rootSignature = nullptr;
	D3D12_ROOT_PARAMETER	m_rootParameter[ROOT_PARAMETERS]{};

	D3D12_SHADER_BYTECODE	m_vertexShader{};
	D3D12_SHADER_BYTECODE	m_geometryShader{};

	ID3D12PipelineState *	m_pipelineState = nullptr;
	D3D12_VIEWPORT	m_viewport{};
	D3D12_RECT		m_rect{};

	ID3D12Resource *		m_constantBuffer[FRAME_BUFFER_COUNT]{ nullptr };
	int m_constantBufferPerObjectAlignedSize = (sizeof(ObjectBuffer) + 255) & ~255;
	UINT8* m_constantBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };

	ID3D12Resource *		m_constantLightBuffer[FRAME_BUFFER_COUNT]{ nullptr };
	int m_constantLightBufferPerObjectAlignedSize = (sizeof(LightBuffer) + 255) & ~255;
	UINT8* m_constantLightBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };

	ObjectBuffer	m_objectValues{};
	LightBuffer		m_lightValues{};
};

