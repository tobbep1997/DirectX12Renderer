#pragma once
#include "Transform.h"

class X12ShaderResourceView;
class X12ConstantBuffer;

#define MAX_PARTICLES 4096

class ParticleEmitter :
	public Transform
{
private:
	struct Particle
	{
		Particle(const DirectX::XMFLOAT4 & startPosition, const float & timeToLive)
		{
			Position = startPosition;
			SpawnPosition = startPosition;
			TimeAlive = 0;
			TimeToLive = timeToLive;
		}
		DirectX::XMFLOAT4 SpawnPosition;
		DirectX::XMFLOAT4 Position;
		float TimeAlive;
		float TimeToLive;
	};

	struct EmitterSettings
	{
		float Speed = 1.0f;
		float Spread = 0.0f;
		float SpawnSpread = 0.2f;
		DirectX::XMFLOAT4 Direction = DirectX::XMFLOAT4(0,1,0,0);
		DirectX::XMFLOAT4 Size = DirectX::XMFLOAT4(.05f, .05f, 0, 0);

		float SpawnRate = 0.0001f;

		float ParticleMaxLife = 3.75f;
		float ParticleMinLife = 2.55f;
		
		UINT MaxParticles = MAX_PARTICLES;


	};

public:
	ParticleEmitter(		
		const Window & window, 
		const UINT & textureWidth, 
		const UINT & textureHeight,
		const UINT & arraySize = 1,
		const DXGI_FORMAT& textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM);
	~ParticleEmitter();

	BOOL Init() override;
	void Release() override;
	void Update() override;
	void Draw();
	void UpdateEmitter(const float & deltaTime);
	void UpdateData(const UINT & frameIndex);

	ID3D12Resource * GetVertexResource() const;
	ID3D12Resource * GetCalcResource() const;

	X12ConstantBuffer * GetParticleBuffer() const;

	ID3D12GraphicsCommandList * GetCommandList() const;
	const std::vector<Particle> & GetParticles() const;

	HRESULT OpenCommandList();
	HRESULT ExecuteCommandList() const;

	const D3D12_VERTEX_BUFFER_VIEW & GetVertexBufferView() const;

	void SwitchToVertexState(ID3D12GraphicsCommandList * commandList);
	void SwitchToUAVState(ID3D12GraphicsCommandList * commandList);

	void SetTextures(Texture *const* textures);

	UINT GetVertexSize() const;

	const EmitterSettings & GetSettings() const;

	X12ShaderResourceView * GetShaderResourceView() const;

	const Texture *const* GetTextures() const;

	D3D12_CPU_DESCRIPTOR_HANDLE GetVertexCpuDescriptorHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCalcCpuDescriptorHandle() const;

	X12Fence *const* GetFence() const;

private:
	HRESULT _createCommandList();
	HRESULT _createBuffer();
	void _updateParticles(const float & deltaTime);

	EmitterSettings m_emitterSettings {};
	float m_spawnTimer = 0;
	std::vector<Particle> * m_particles = nullptr;

	D3D12_RESOURCE_STATES m_currentState[FRAME_BUFFER_COUNT] {};
	ID3D12Resource * m_vertexOutputResource[FRAME_BUFFER_COUNT]{ nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE m_vertexOutputHandle[FRAME_BUFFER_COUNT]{{0}};

	ID3D12Resource * m_calculationsOutputResource[FRAME_BUFFER_COUNT]{ nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE m_calculationsOutputHandle[FRAME_BUFFER_COUNT]{{0}};

	ID3D12Resource * m_vertexResource[FRAME_BUFFER_COUNT]{ nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE m_vertexHandle[FRAME_BUFFER_COUNT]{{0}};

	ID3D12GraphicsCommandList * m_commandList[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12CommandAllocator * m_commandAllocator[FRAME_BUFFER_COUNT] { nullptr };

	X12Fence * m_fences[FRAME_BUFFER_COUNT]{ nullptr };

	RenderingManager * m_renderingManager = nullptr;
	const Window * m_window = nullptr;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView {};
	
	X12ShaderResourceView * m_shaderResourceView = nullptr;
	UINT m_width;
	UINT m_height;
	UINT m_arraySize;
	DXGI_FORMAT m_format;

	X12ConstantBuffer * m_particleBuffer = nullptr;

	Texture *const* m_textures = nullptr;
};

