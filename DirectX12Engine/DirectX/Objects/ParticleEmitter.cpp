#include "DirectX12EnginePCH.h"
#include "ParticleEmitter.h"
#include "DirectX/Render/ParticlePass.h"
#include "DirectX/Structs.h"
#include "DirectX/Render/WrapperFunctions/X12ShaderResourceView.h"

ParticleEmitter::ParticleEmitter(
	RenderingManager* renderingManager, 
	const Window & window,
	const UINT & textureWidth, 
	const UINT & textureHeight,
	const UINT & arraySize,
	const DXGI_FORMAT& textureFormat)
{
	this->m_renderingManager = renderingManager;
	this->m_window = &window;

	this->m_width = textureWidth;
	this->m_height = textureHeight;
	this->m_arraySize = arraySize;
	this->m_format = textureFormat;
}

ParticleEmitter::~ParticleEmitter()
{
}

BOOL ParticleEmitter::Init()
{
	if (!Transform::Init())
		return FALSE;

	HRESULT hr = 0;	
	if (SUCCEEDED(hr = _createCommandList()))
	{
		if (SUCCEEDED(hr = _createBuffer()))
		{			
			SAFE_NEW(m_shaderResourceView, new X12ShaderResourceView(m_renderingManager, *m_window));
			if (SUCCEEDED(hr = m_shaderResourceView->CreateShaderResourceView(
				m_width,
				m_height,
				m_arraySize,
				this->m_format)))
			{
				if (SUCCEEDED(hr = OpenCommandList()))
				{					
					for (UINT i = 0; i < m_arraySize; i++)
					{
						m_shaderResourceView->BeginCopy(m_commandList[*m_renderingManager->GetFrameIndex()]);

						m_shaderResourceView->CopySubresource(i,
							m_textures[i]->GetResource(), m_commandList[*m_renderingManager->GetFrameIndex()]);

						m_shaderResourceView->EndCopy(m_commandList[*m_renderingManager->GetFrameIndex()]);
					}
					if (SUCCEEDED(hr = m_renderingManager->SignalGPU(m_commandList[*m_renderingManager->GetFrameIndex()])))
					{
						
					}
				}
			}
		}
	}
	if (FAILED(hr))
	{
		this->Release();
		return FALSE;
	}

	SAFE_NEW(m_particles, new std::vector<Particle>());
	return TRUE;
}

void ParticleEmitter::Release()
{
	SAFE_RELEASE(m_vertexOutputResource);



	m_particles->clear();
	SAFE_DELETE(m_particles);

	m_shaderResourceView->Release();
	SAFE_DELETE(m_shaderResourceView);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_commandList[i]);
		SAFE_RELEASE(m_commandAllocator[i]);
		SAFE_RELEASE(m_calculationsOutputResource[i]);
	}
	Transform::Release();
}

void ParticleEmitter::Update()
{
	Transform::Update();
}

ID3D12Resource* ParticleEmitter::GetVertexResource() const
{
	return this->m_vertexOutputResource;
}

ID3D12Resource* ParticleEmitter::GetCalcResource() const
{
	return this->m_calculationsOutputResource[*m_renderingManager->GetFrameIndex()];
}

ID3D12GraphicsCommandList* ParticleEmitter::GetCommandList() const
{
	return this->m_commandList[*m_renderingManager->GetFrameIndex()];
}

const std::vector<ParticleEmitter::Particle>& ParticleEmitter::GetPositions() const
{
	return *m_particles;
}

HRESULT ParticleEmitter::OpenCommandList()
{
	HRESULT hr = 0;
	const UINT frameIndex = *m_renderingManager->GetFrameIndex();
	if (SUCCEEDED(hr = this->m_commandAllocator[frameIndex]->Reset()))
	{
		if (SUCCEEDED(hr = this->m_commandList[*m_renderingManager->GetFrameIndex()]->Reset(this->m_commandAllocator[frameIndex], nullptr)))
		{

		}
	}
	return hr;
}

HRESULT ParticleEmitter::ExecuteCommandList() const
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = m_commandList[*m_renderingManager->GetFrameIndex()]->Close()))
	{
		ID3D12CommandList* ppCommandLists[] = { m_commandList[*m_renderingManager->GetFrameIndex()] };
		m_renderingManager->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	return hr;
}

const D3D12_VERTEX_BUFFER_VIEW& ParticleEmitter::GetVertexBufferView() const
{
	return this->m_vertexBufferView;
}

void ParticleEmitter::SwitchToVertexState(ID3D12GraphicsCommandList * commandList)
{
	if (D3D12_RESOURCE_STATE_UNORDERED_ACCESS == m_currentState)
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexOutputResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	m_currentState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

}

void ParticleEmitter::SwitchToUAVState(ID3D12GraphicsCommandList * commandList)
{
	if (D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER == m_currentState)
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexOutputResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	m_currentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
}

void ParticleEmitter::SetTextures(Texture* const* textures)
{
	m_textures = textures;
}

UINT ParticleEmitter::GetVertexSize() const
{
	return static_cast<UINT>(this->m_particles->size()) * 6u;
}

const ParticleEmitter::EmitterSettings& ParticleEmitter::GetSettings() const
{
	return this->m_emitterSettings;
}

X12ShaderResourceView* ParticleEmitter::GetShaderResourceView() const
{
	return this->m_shaderResourceView;
}

D3D12_CPU_DESCRIPTOR_HANDLE ParticleEmitter::GetVertexCpuDescriptorHandle() const
{
	return { m_renderingManager->GetCpuDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_vertexOutputOffset };
}

D3D12_CPU_DESCRIPTOR_HANDLE ParticleEmitter::GetCalcCpuDescriptorHandle() const
{
	return { m_renderingManager->GetCpuDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_calculationsOutputOffset };

}

HRESULT ParticleEmitter::_createCommandList()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_renderingManager->GetDevice()->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocator[i]))))
		{
			return hr;
		}
		SET_NAME(m_commandAllocator[i], L"Particle CommandAllocator");

		if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_commandAllocator[0],
			nullptr,
			IID_PPV_ARGS(&m_commandList[i]))))
		{
			SET_NAME(m_commandList[i], L"Particle CommandList");
			m_commandList[i]->Close();
		}
	}
	return hr;
}

HRESULT ParticleEmitter::_createBuffer()
{
	HRESULT hr = 0;

	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(4096 * 4096);
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&m_vertexOutputResource))))
	{
		m_currentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		SET_NAME(m_vertexOutputResource, L"Particle UAV output");

		D3D12_BUFFER_UAV uav{};
		uav.NumElements = 256;
		uav.FirstElement = 0;
		uav.StructureByteStride = sizeof(DirectX::XMFLOAT4) * 2;
		uav.CounterOffsetInBytes = 0;
		uav.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
		unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		unorderedAccessViewDesc.Buffer = uav;

		m_vertexOutputOffset = m_renderingManager->GetResourceCurrentIndex() * m_renderingManager->GetResourceIncrementalSize();
		const D3D12_CPU_DESCRIPTOR_HANDLE handle =
		{ m_renderingManager->GetCpuDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_vertexOutputOffset };

		m_renderingManager->GetDevice()->CreateUnorderedAccessView(
			m_vertexOutputResource,
			nullptr,
			&unorderedAccessViewDesc,
			handle);
		m_renderingManager->IterateCbvSrvUavDescriptorHeapIndex();
	}
	
	if (FAILED(hr))
		return hr;
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&m_calculationsOutputResource[i]))))
		{
			m_currentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			SET_NAME(m_calculationsOutputResource[i], L"Particle UAV output");

			D3D12_BUFFER_UAV uav{};
			uav.NumElements = 256;
			uav.FirstElement = 0;
			uav.StructureByteStride = sizeof(DirectX::XMFLOAT4) * 2;
			uav.CounterOffsetInBytes = 0;
			uav.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
			unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
			unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			unorderedAccessViewDesc.Buffer = uav;

			m_calculationsOutputOffset = m_renderingManager->GetResourceCurrentIndex() * m_renderingManager->GetResourceIncrementalSize();
			const D3D12_CPU_DESCRIPTOR_HANDLE handle =
			{ m_renderingManager->GetCpuDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_calculationsOutputOffset };

			m_renderingManager->GetDevice()->CreateUnorderedAccessView(
				m_calculationsOutputResource[i],
				nullptr,
				&unorderedAccessViewDesc,
				handle);
			m_renderingManager->IterateCbvSrvUavDescriptorHeapIndex();
		}

	}
	
	return hr;
}

void ParticleEmitter::_updateParticles(const float & deltaTime)
{
	using namespace DirectX;
	for (size_t i = 0; i < m_particles->size(); i++)
	{
		if (m_particles->at(i).TimeAlive >= m_particles->at(i).TimeToLive)
		{
			const XMVECTOR baseSpawn = XMVectorAdd(
				XMLoadFloat4(&GetPosition()),
				XMLoadFloat4(&XMFLOAT4(
					sinf(static_cast<float>(rand())) * m_emitterSettings.SpawnSpread,
					0,
					cosf(static_cast<float>(rand()))* m_emitterSettings.SpawnSpread,
					0)));

			XMFLOAT4 position; XMStoreFloat4(&position, baseSpawn);
			position.w = 1;

			float timeToLive = (static_cast<float>(rand() % 1000) / 1000.f);
			timeToLive *= m_emitterSettings.ParticleMaxLife;
			if (timeToLive <= m_emitterSettings.ParticleMinLife)
				timeToLive = m_emitterSettings.ParticleMinLife;
			m_particles->at(i) = Particle(position, timeToLive);
		}
	}
	m_spawnTimer += deltaTime;
	while (m_particles->size() < m_emitterSettings.MaxParticles &&  m_spawnTimer >= m_emitterSettings.SpawnRate)
	{
		const XMVECTOR baseSpawn = XMVectorAdd(
			XMLoadFloat4(&GetPosition()),
			XMLoadFloat4(&XMFLOAT4(
				sinf(static_cast<float>(rand())) * m_emitterSettings.SpawnSpread,
				0,
				cosf(static_cast<float>(rand()))* m_emitterSettings.SpawnSpread,
				0)));

		XMFLOAT4 position; XMStoreFloat4(&position, baseSpawn);
		position.w = 1;

		float timeToLive = (static_cast<float>(rand() % 1000) / 1000.f);
		timeToLive *= m_emitterSettings.ParticleMaxLife;
		if (timeToLive <= m_emitterSettings.ParticleMinLife)
			timeToLive = m_emitterSettings.ParticleMinLife;
		m_particles->push_back(Particle(position, timeToLive));
		m_spawnTimer = 0;
	}
}

void ParticleEmitter::Draw()
{
	ParticlePass * pass = m_renderingManager->GetParticlePass();
	if (pass)
		pass->AddEmitter(this);
}

void ParticleEmitter::UpdateEmitter(const float & deltaTime)
{
	_updateParticles(deltaTime);

}

void ParticleEmitter::UpdateData()
{
	using namespace DirectX;

	m_vertexBufferView.BufferLocation = m_vertexOutputResource->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(ParticleVertex);
	m_vertexBufferView.SizeInBytes = m_vertexBufferView.StrideInBytes * GetVertexSize();

	struct OutputCalculations
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 ParticleInfo;
	};

	const UINT frameIndex = *m_renderingManager->GetFrameIndex();
	if (frameIndex == UINT_MAX)
		return;

	OutputCalculations * outputArray = nullptr;
	CD3DX12_RANGE readRange(0, sizeof(OutputCalculations) * m_particles->size());
	if (SUCCEEDED(m_calculationsOutputResource[frameIndex]->Map(0, &readRange, reinterpret_cast<void**>(&outputArray))))
	{
		for (size_t i = 0; i < m_particles->size(); i++)
		{
			m_particles->at(i).Position = outputArray[i].Position;
			m_particles->at(i).TimeAlive = outputArray[i].ParticleInfo.y;	
		}
		m_calculationsOutputResource[frameIndex]->Unmap(0, &readRange);
	}
}
