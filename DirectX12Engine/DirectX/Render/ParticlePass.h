#pragma once
#include "Template/IRender.h"
#include "WrapperFunctions/X12ConstantBuffer.h"
#include "WrapperFunctions/X12Fence.h"

class ParticleEmitter;
class X12ConstantBuffer;

class ParticlePass :
	public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 4;

	struct ParticleBuffer
	{
		DirectX::XMFLOAT4A ParticleInfo;
		DirectX::XMFLOAT4A ParticlePosition;
		DirectX::XMFLOAT4A ParticleSpeed;
		DirectX::XMFLOAT4A ParticleSize;
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
	HRESULT _initCommandQueue(const D3D12_COMMAND_LIST_TYPE& type, const UINT& nodeMask);
	HRESULT _initCommandList(const D3D12_COMMAND_LIST_TYPE & type, const UINT& nodeMask);
	HRESULT _initID3D12RootSignature();
	HRESULT _initShaders();
	HRESULT _initPipelineState();

	D3D12_ROOT_PARAMETER m_rootParameters[ROOT_PARAMETERS] {};
	ID3D12RootSignature * m_rootSignature = nullptr;
	D3D12_SHADER_BYTECODE m_computeShader {};
	ID3D12PipelineState * m_computePipelineState = nullptr;

	ID3D12CommandQueue * m_commandQueue = nullptr;
	ID3D12CommandAllocator * m_commandAllocator[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12GraphicsCommandList * m_commandList[FRAME_BUFFER_COUNT] {nullptr};

	ParticleBuffer m_particleValues {};
		
	std::vector<ParticleEmitter*>* m_emitters = nullptr;

	X12ConstantBuffer * m_particleInfoBuffer = nullptr;
	X12ConstantBuffer * m_particleBuffer = nullptr;

	GeometryPass * m_geometryPass;

	X12Fence * m_fence;
};