#pragma once
#include "Template/IRender.h"

class ParticleEmitter;
class X12ConstantBuffer;

class ParticlePass :
	public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 3;

	struct ParticleBuffer
	{
		DirectX::XMFLOAT4A CameraPosition;
		DirectX::XMFLOAT4X4A WorldMatrix;

		DirectX::XMFLOAT4A ParticleInfo[256];
		DirectX::XMFLOAT4A ParticlePosition[256];
		DirectX::XMFLOAT4A ParticleSpeed[256];
		DirectX::XMFLOAT4A ParticleSize[256];
	};


public:
	ParticlePass(RenderingManager * renderingManager, const Window & window);
	~ParticlePass();

	HRESULT Init() override;
	void Update(const Camera& camera, const float & deltaTime) override;
	void Draw() override;
	void Clear() override;
	void Release() override;

	void AddEmitter(ParticleEmitter * particleEmitter) const;

private:
	HRESULT _initID3D12RootSignature();
	HRESULT _initShaders();
	HRESULT _initPipelineState();
	HRESULT _createUAVOutput();

	D3D12_ROOT_PARAMETER m_rootParameters[ROOT_PARAMETERS] {};
	ID3D12RootSignature * m_rootSignature = nullptr;
	D3D12_SHADER_BYTECODE m_computeShader {};
	ID3D12PipelineState * m_computePipelineState = nullptr;

	ParticleBuffer m_particleValues {};
		
	std::vector<ParticleEmitter*>* m_emitters = nullptr;

	ID3D12Resource *		m_constantParticleBuffer[FRAME_BUFFER_COUNT]{ nullptr };
	int m_constantParticleBufferPerObjectAlignedSize = (sizeof(ParticleBuffer) + 255) & ~255;
	UINT8* m_constantParticleBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };

	GeometryPass * m_geometryPass;

};