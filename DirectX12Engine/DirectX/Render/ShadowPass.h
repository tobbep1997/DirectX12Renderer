#pragma once
#include "Template/IRender.h"

class X12DepthStencil;
class X12ConstantBuffer;

class ShadowPass :
	public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 2;

	struct ObjectBuffer
	{
		DirectX::XMFLOAT4X4A	WorldMatrix;

		DirectX::XMFLOAT4A		Padding[45];
	};
	struct LightBuffer
	{
		DirectX::XMFLOAT4X4A LightViewProjection[256];
	};
public:
	ShadowPass(RenderingManager * renderingManager, const Window & window);
	~ShadowPass();

	X12DepthStencil * m_depthStencil;
	X12ConstantBuffer * m_lightConstantBuffer;

	HRESULT Init() override;
	void Update(const Camera& camera) override;
	void Draw() override;
	void Clear() override;
	void Release() override;

private:
	HRESULT _preInit();
	HRESULT _signalGPU() const;

	HRESULT _createRenderTarget();
	

	HRESULT _initRootSignature();
	HRESULT _initShaders();
	HRESULT _initPipelineState();
	HRESULT _createConstantBuffer();
	void _createViewport();

	UINT m_width = 1024U;
	UINT m_height = 1024U;

	UINT m_rtvDescriptorSize = 0;
	ID3D12DescriptorHeap *	m_rtvDescriptorHeap = nullptr;
	ID3D12Resource *		m_renderTargets[FRAME_BUFFER_COUNT]{nullptr};

	ID3D12RootSignature *	m_rootSignature = nullptr;
	D3D12_ROOT_PARAMETER	m_rootParameter[ROOT_PARAMETERS]{};

	D3D12_SHADER_BYTECODE	m_vertexShader{};

	ID3D12PipelineState *	m_pipelineState = nullptr;
	D3D12_VIEWPORT	m_viewport{};
	D3D12_RECT		m_rect{};

	ID3D12Resource *		m_constantBuffer[FRAME_BUFFER_COUNT]{ nullptr };

	int m_constantBufferPerObjectAlignedSize = (sizeof(ObjectBuffer) + 255) & ~255;
	UINT8* m_constantBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };

	ObjectBuffer	m_objectValues{};
	LightBuffer		m_lightValues{};
};

