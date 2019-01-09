#pragma once
#include "Transform.h"

class ParticleEmitter :
	public Transform
{
public:
	ParticleEmitter(RenderingManager * renderingManager);
	~ParticleEmitter();

	void Init() override;
	void Release() override;
	void Draw();

	ID3D12Resource * GetResource() const;
	ID3D12DescriptorHeap * GetDescriptorHeap() const;

	ID3D12GraphicsCommandList * GetCommandList() const;

	const std::vector<DirectX::XMFLOAT4> & GetPositions() const;

	HRESULT OpenCommandList();
	HRESULT ExecuteCommandList() const;

private:
	HRESULT _createCommandList();
	HRESULT _createBuffer();

	std::vector<DirectX::XMFLOAT4> m_positions;

	ID3D12Resource * m_resource = nullptr;
	ID3D12DescriptorHeap * m_descriptorHeap = nullptr;

	ID3D12GraphicsCommandList * m_commandList = nullptr;
	ID3D12CommandAllocator * m_commandAllocator[FRAME_BUFFER_COUNT] {nullptr};

	RenderingManager * m_renderingManager = nullptr;
};

