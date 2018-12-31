#pragma once
#include "Template/IRender.h"

class X12DepthStencil;

class ShadowPass :
	public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 1;
public:
	ShadowPass(RenderingManager * renderingManager, const Window & window);
	~ShadowPass();

	X12DepthStencil * m_depthStencil;

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
	void _createViewport();

	ID3D12RootSignature *	m_rootSignature = nullptr;
	D3D12_ROOT_PARAMETER	m_rootParameter[ROOT_PARAMETERS];

	D3D12_SHADER_BYTECODE	m_vertexShader;

	ID3D12PipelineState *	m_pipelineState = nullptr;
	D3D12_VIEWPORT	m_viewport;
	D3D12_RECT		m_rect;
};

