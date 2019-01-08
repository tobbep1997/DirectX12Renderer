#pragma once
#include "Template/IRender.h"

class X12ConstantBuffer;

class ParticlePass :
	public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 2;

	struct ParticleBuffer
	{
		DirectX::XMFLOAT4A CameraPosition;
		DirectX::XMUINT4 ParticleInfo;
		DirectX::XMFLOAT4A ParticlePosition[256];
	};
public:
	ParticlePass(RenderingManager * renderingManager, const Window & window);
	~ParticlePass();

	HRESULT Init() override;
	void Update(const Camera& camera) override;
	void Draw() override;
	void Clear() override;
	void Release() override;

private:
	HRESULT _initID3D12RootSignature();
	HRESULT _initShaders();
	HRESULT _initPipelineState();
	HRESULT _createUAVOutput();

	D3D12_ROOT_PARAMETER m_rootParameters[ROOT_PARAMETERS] {};
	ID3D12RootSignature * m_rootSignature = nullptr;
	D3D12_SHADER_BYTECODE m_computeShader {};
	ID3D12PipelineState * m_computePipelineState = nullptr;

	X12ConstantBuffer * m_particleBuffer = nullptr;
	ParticleBuffer m_particleValues {};

	ID3D12Resource * m_particleUAVOutput;
	ID3D12DescriptorHeap * m_particleUAVOutputDescriptorHeap;
};

