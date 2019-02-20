#pragma once
#include "Transform.h"

class X12ShaderResourceView;

class ParticleEmitter :
	public Transform
{
private:
	struct Particle
	{
		Particle(const DirectX::XMFLOAT4 & startPosition, const float & timeToLive)
		{
			Position = startPosition;
			TimeAlive = 0;
			TimeToLive = timeToLive;
		}
		DirectX::XMFLOAT4 Position;
		float TimeAlive;
		float TimeToLive;
	};

	struct EmitterSettings
	{
		float Speed = 5.0f;
		float Spread = 0.0f;
		float SpawnSpread = 0.2f;
		DirectX::XMFLOAT4 Direction = DirectX::XMFLOAT4(0,1,0,0);
		DirectX::XMFLOAT4 Size = DirectX::XMFLOAT4(.15f, .15f, 0, 0);

		float SpawnRate = 0.02f;

		float ParticleMaxLife = 0.35f;
		float ParticleMinLife = 0.15f;
		
		UINT MaxParticles = 256;


	};

public:
	ParticleEmitter(
		RenderingManager * renderingManager, 
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
	void UpdateData();

	ID3D12Resource * GetVertexResource() const;
	ID3D12Resource * GetCalcResource() const;

	ID3D12GraphicsCommandList * GetCommandList() const;
	const std::vector<Particle> & GetPositions() const;

	HRESULT OpenCommandList();
	HRESULT ExecuteCommandList() const;

	const D3D12_VERTEX_BUFFER_VIEW & GetVertexBufferView() const;

	void SwitchToVertexState(ID3D12GraphicsCommandList * commandList);
	void SwitchToUAVState(ID3D12GraphicsCommandList * commandList);

	void SetTextures(Texture *const* textures);

	UINT GetVertexSize() const;

	const EmitterSettings & GetSettings() const;

	X12ShaderResourceView * GetShaderResourceView() const;

private:
	HRESULT _createCommandList();
	HRESULT _createBuffer();
	void _updateParticles(const float & deltaTime);

	EmitterSettings m_emitterSettings {};
	float m_spawnTimer = 0;
	std::vector<Particle> * m_particles = nullptr;

	D3D12_RESOURCE_STATES m_currentState {};
	ID3D12Resource * m_vertexOutputResource = nullptr;
	SIZE_T m_vertexOutputOffset = 0;

	ID3D12Resource * m_calculationsOutputResource = nullptr;
	SIZE_T m_calculationsOutputOffset = 0;

	ID3D12GraphicsCommandList * m_commandList[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12CommandAllocator * m_commandAllocator[FRAME_BUFFER_COUNT] { nullptr };

	RenderingManager * m_renderingManager = nullptr;
	const Window * m_window = nullptr;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView {};
	
	X12ShaderResourceView * m_shaderResourceView = nullptr;
	UINT m_width;
	UINT m_height;
	UINT m_arraySize;
	DXGI_FORMAT m_format;

	Texture *const* m_textures = nullptr;
};

