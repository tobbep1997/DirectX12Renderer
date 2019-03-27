#include "DirectX12EnginePCH.h"
#include "ParticlePass.h"
#include "WrapperFunctions/X12ConstantBuffer.h"
#include "../Objects/ParticleEmitter.h"
#include "GeometryPass.h"
#include <stdlib.h>

#define MAX_EMITTERS 256

#define PARTICLE_INFO	0
#define PARTICLE_BUFFER 1
#define VERTEX_OUTPUT	2
#define CALC_OUTPUT		3

ParticlePass::ParticlePass(RenderingManager* renderingManager, const Window& window)
	: IRender(renderingManager, window)
{
	SAFE_NEW(m_emitters, new std::vector<ParticleEmitter*>());
	SAFE_NEW(m_particleBuffer, new X12ConstantBuffer());
	SAFE_NEW(m_particleInfoBuffer, new X12ConstantBuffer());
	SAFE_NEW(m_fence, new X12Fence());

	this->m_geometryPass = renderingManager->GetGeometryPass();
}

ParticlePass::~ParticlePass()
{
	SAFE_DELETE(m_emitters);
}

HRESULT ParticlePass::Init()
{
	HRESULT hr = 0;

	if (FAILED(hr = p_createCommandList(L"tmp")))
	{		
		return hr;
	}

	const D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ID3D12Device * device = p_renderingManager->GetSecondAdapter() ? p_renderingManager->GetSecondAdapter()->GetDevice() : p_renderingManager->GetMainAdapter()->GetDevice();

	if (FAILED(hr = _initCommandQueue(device, type, 0)))
	{		
		return hr;
	}
	if (FAILED(hr = _initCommandList(device, type, 0)))
	{		
		return hr;
	}

	if (FAILED(hr = _initID3D12RootSignature()))
	{
		return hr;
	}
	if (FAILED(hr = _initShaders()))
	{
		return hr;
	}
	if (FAILED(hr = _initPipelineState()))
	{
		return hr;
	}


	if (FAILED(hr = m_particleBuffer->CreateSharedBuffer(L"Particle buffer", sizeof(ParticleBuffer), 4096 * 4096)))
	{
		if (FAILED(hr = m_particleBuffer->CreateBuffer(L"Particle buffer", nullptr, 0, 4096 * 4096)))
		{		
			return hr;
		}		
	}
	if (FAILED(hr = m_particleInfoBuffer->CreateSharedBuffer(L"Particle info buffer", 0, 256)))
	{
		if (FAILED(hr = m_particleInfoBuffer->CreateBuffer(L"Particle info buffer", nullptr, 0, 256)))
		{
			return hr;
		}	
	}


	if (FAILED(hr = m_fence->CreateFence(L"Particle fence", device)))
	{		
		return hr;
	}



	return hr;
}

void ParticlePass::Update(const Camera& camera, const float & deltaTime)
{
	struct ParticleInfoBuffer
	{
		DirectX::XMFLOAT4	CameraPosition;
		DirectX::XMFLOAT4X4 WorldMatrix;
	}particleInfoBuffer;


	particleInfoBuffer.CameraPosition = DirectX::XMFLOAT4A(
		camera.GetPosition().x,
		camera.GetPosition().y,
		camera.GetPosition().z,
		camera.GetPosition().w);

	UINT offset = 0;	
	for (size_t i = 0; i < m_emitters->size() && i < MAX_EMITTERS; i++)
	{
		m_emitters->at(i)->UpdateEmitter(deltaTime);

		particleInfoBuffer.WorldMatrix = m_emitters->at(i)->GetWorldMatrix();
		for (size_t j = 0; j < m_emitters->at(i)->GetPositions().size(); j++)
		{
			m_particleValues.ParticleInfo.x = deltaTime;
			m_particleValues.ParticleInfo.y = m_emitters->at(i)->GetPositions()[j].TimeAlive;
			m_particleValues.ParticleInfo.z = m_emitters->at(i)->GetPositions()[j].TimeToLive;
			
			m_particleValues.ParticlePosition = DirectX::XMFLOAT4A(
				m_emitters->at(i)->GetPositions()[j].Position.x,
				m_emitters->at(i)->GetPositions()[j].Position.y,
				m_emitters->at(i)->GetPositions()[j].Position.z,
				m_emitters->at(i)->GetPositions()[j].Position.w
			);
			m_particleValues.ParticleSpeed = DirectX::XMFLOAT4A(
				m_emitters->at(i)->GetSettings().Direction.x,
				m_emitters->at(i)->GetSettings().Direction.y,
				m_emitters->at(i)->GetSettings().Direction.z,
				m_emitters->at(i)->GetSettings().Speed
			);
			m_particleValues.ParticleSize = DirectX::XMFLOAT4A(
				m_emitters->at(i)->GetSettings().Size.x,
				m_emitters->at(i)->GetSettings().Size.y,
				m_emitters->at(i)->GetSettings().Size.z,
				m_emitters->at(i)->GetSettings().Size.w
				);
			m_particleBuffer->Copy(&m_particleValues, sizeof(ParticleBuffer), offset);
			offset += sizeof(ParticleBuffer);
		}
		m_particleInfoBuffer->Copy(&particleInfoBuffer, sizeof(ParticleInfoBuffer), static_cast<UINT>(i) * sizeof(ParticleInfoBuffer));
	}

	const UINT frameIndex = p_renderingManager->GetFrameIndex();
	if (FAILED(m_commandAllocator[frameIndex]->Reset()))
	{
		return;
	}
	if (FAILED(m_commandList[frameIndex]->Reset(m_commandAllocator[frameIndex], m_computePipelineState)))
	{
		return;
	}

	ParticleEmitter * emitter = nullptr;
	for (size_t i = 0; i < m_emitters->size(); i++)
	{
		emitter = m_emitters->at(i);

		ID3D12GraphicsCommandList * commandList = m_commandList[frameIndex];

		emitter->SwitchToUAVState(commandList);

		commandList->SetComputeRootSignature(m_rootSignature);
		
		m_particleInfoBuffer->SetComputeRootConstantBufferView(commandList, PARTICLE_INFO, 0);
		m_particleBuffer->SetComputeRootShaderResourceView(commandList, PARTICLE_BUFFER, 0);

		commandList->SetComputeRootUnorderedAccessView(VERTEX_OUTPUT, emitter->GetVertexResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(CALC_OUTPUT, emitter->GetCalcResource()->GetGPUVirtualAddress());
		
		commandList->Dispatch(static_cast<UINT>(m_emitters->at(i)->GetPositions().size()), 1, 1);

		emitter->SwitchToVertexState(commandList);


		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(emitter->GetCalcResource()));
		
	
	}

	if (FAILED(m_commandList[frameIndex]->Close()))
	{
		return;
	}

	ID3D12CommandList * ppCommandLists[] = { m_commandList[frameIndex] };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	   
	if (SUCCEEDED(m_fence->Signal(m_commandQueue)))
	{
		if (SUCCEEDED(m_fence->WaitCpu()))
		{
			for (size_t i = 0; i < m_emitters->size(); i++)
			{
				emitter = m_emitters->at(i);
				emitter->UpdateData();
			}
		}
	}
	for (size_t i = 0; i < m_emitters->size(); i++)
	{
		emitter = m_emitters->at(i);
		if (!emitter->GetPositions().empty())
			m_geometryPass->AddEmitter(emitter);
	}

}

void ParticlePass::Draw()
{

}

void ParticlePass::Clear()
{
	this->p_drawQueue->clear();
	this->p_lightQueue->clear();
	this->m_emitters->clear();
	Instancing::ClearInstanceGroup(this->p_instanceGroups);
}

void ParticlePass::Release()
{
	p_releaseCommandList();
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_computePipelineState);

	if (m_particleBuffer)
		m_particleBuffer->Release();
	SAFE_DELETE(m_particleBuffer);

	if (m_particleInfoBuffer)
		m_particleInfoBuffer->Release();
	SAFE_DELETE(m_particleInfoBuffer);

	if (m_fence)
		m_fence->Release();
	SAFE_DELETE(m_fence);

	SAFE_RELEASE(m_commandQueue);
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_commandAllocator[i]);
		SAFE_RELEASE(m_commandList[i]);
	}

}

void ParticlePass::AddEmitter(ParticleEmitter* particleEmitter) const
{
	m_emitters->push_back(particleEmitter);
}

HRESULT ParticlePass::_initCommandQueue(ID3D12Device * device, const D3D12_COMMAND_LIST_TYPE& type, const UINT & nodeMask)
{
	HRESULT hr = 0;

	D3D12_COMMAND_QUEUE_DESC desc{ type, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE, nodeMask };

	if (FAILED(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue))))
	{
		return hr;
	}
	SET_NAME(m_commandQueue, L"Particle CommandQueue");
	return hr;
}

HRESULT ParticlePass::_initCommandList(ID3D12Device * device, const D3D12_COMMAND_LIST_TYPE& type, const UINT& nodeMask)
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = device->CreateCommandAllocator(
			type, 
			IID_PPV_ARGS(&m_commandAllocator[i]))))
		{
			return hr;
		}
		if (FAILED(hr = device->CreateCommandList(
			nodeMask, 
			type, 
			m_commandAllocator[i], 
			nullptr, 
			IID_PPV_ARGS(&m_commandList[i]))))
		{			
			return hr;
		}
		m_commandList[i]->Close();
		SET_NAME(m_commandAllocator[i], L"Particle CommandAllocator" + std::to_wstring(i));
		SET_NAME(m_commandList[i], L"Particle CommandList" + std::to_wstring(i));
	}
	return hr;
}


HRESULT ParticlePass::_initID3D12RootSignature()
{
	HRESULT hr = 0;

	D3D12_ROOT_DESCRIPTOR rootDescriptor;
	rootDescriptor.RegisterSpace = 0;
	rootDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR particleBuffer;
	particleBuffer.RegisterSpace = 0;
	particleBuffer.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR uavVertexDescriptor;
	uavVertexDescriptor.RegisterSpace = 0;
	uavVertexDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR uavCalculationsDescriptor;
	uavCalculationsDescriptor.RegisterSpace = 0;
	uavCalculationsDescriptor.ShaderRegister = 1;

	m_rootParameters[PARTICLE_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[PARTICLE_INFO].Descriptor = rootDescriptor;
	m_rootParameters[PARTICLE_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	m_rootParameters[PARTICLE_BUFFER].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	m_rootParameters[PARTICLE_BUFFER].Descriptor = particleBuffer;
	m_rootParameters[PARTICLE_BUFFER].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	m_rootParameters[VERTEX_OUTPUT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	m_rootParameters[VERTEX_OUTPUT].Descriptor = uavVertexDescriptor;
	m_rootParameters[VERTEX_OUTPUT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	m_rootParameters[CALC_OUTPUT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	m_rootParameters[CALC_OUTPUT].Descriptor = uavCalculationsDescriptor;
	m_rootParameters[CALC_OUTPUT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(m_rootParameters),
		m_rootParameters,
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS	|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS		|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS	|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS	|
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

	ID3D12Device * device = p_renderingManager->GetSecondAdapter() ? p_renderingManager->GetSecondAdapter()->GetDevice() : p_renderingManager->GetMainAdapter()->GetDevice();


	ID3DBlob * signature = nullptr;
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		nullptr)))
	{
		if (FAILED(hr = device->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature))))
		{
			SAFE_RELEASE(m_rootSignature);
		}
	}
	SAFE_RELEASE(signature);
	return hr;
}

HRESULT ParticlePass::_initShaders()
{
	HRESULT hr = 0;
	ID3DBlob * blob = nullptr;

	if (FAILED(hr = ShaderCreator::CreateShader(L"../DirectX12Engine/DirectX/Shaders/ParticlePass/DefaultParticleCompute.hlsl", blob, "cs_5_1")))
	{
		return hr;
	}
	else
	{
		m_computeShader.BytecodeLength = blob->GetBufferSize();
		m_computeShader.pShaderBytecode = blob->GetBufferPointer();
	}
	return hr;
}

HRESULT ParticlePass::_initPipelineState()
{
	HRESULT hr = 0;

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc = {};
	computePipelineStateDesc.pRootSignature = m_rootSignature;
	computePipelineStateDesc.CS = m_computeShader;


	ID3D12Device * device = p_renderingManager->GetSecondAdapter() ? p_renderingManager->GetSecondAdapter()->GetDevice() : p_renderingManager->GetMainAdapter()->GetDevice();
	if (FAILED(hr = device->CreateComputePipelineState(
		&computePipelineStateDesc,
		IID_PPV_ARGS(&m_computePipelineState))))
	{
		
	}

	return hr;
}
