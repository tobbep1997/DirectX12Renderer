#include "DirectX12EnginePCH.h"
#include "ParticlePass.h"
#include "WrapperFunctions/X12ConstantBuffer.h"
#include "../Objects/ParticleEmitter.h"
#include "GeometryPass.h"

ParticlePass::ParticlePass(RenderingManager* renderingManager, const Window& window)
	: IRender(renderingManager, window)
{
	SAFE_NEW(m_emitters, new std::vector<ParticleEmitter*>());
	this->m_geometryPass = renderingManager->GetGeometryPass();
}

ParticlePass::~ParticlePass()
{
	SAFE_DELETE(m_emitters);
}

HRESULT ParticlePass::Init()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = p_createCommandList(L"Particle")))
	{
		if (SUCCEEDED(hr = OpenCommandList()))
		{
			if (SUCCEEDED(hr = _initID3D12RootSignature()))
			{
				if (SUCCEEDED(hr = _initShaders()))
				{
					if (SUCCEEDED(hr = _initPipelineState()))
					{
						if (SUCCEEDED(hr = _createUAVOutput()))
						{
							
						}
					}
				}
			}
		}
	}
	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(hr = p_renderingManager->SignalGPU(p_commandList)))
		{
			
		}		
	}

	return hr;
}

void ParticlePass::Update(const Camera& camera, const float & deltaTime)
{	
	m_particleValues.CameraPosition = DirectX::XMFLOAT4A(
		camera.GetPosition().x,
		camera.GetPosition().y,
		camera.GetPosition().z,
		camera.GetPosition().w);

	
	
	for (size_t i = 0; i < m_emitters->size(); i++)
	{
		m_emitters->at(i)->UpdateEmitter(deltaTime);


		m_particleValues.WorldMatrix = m_emitters->at(i)->GetWorldMatrix();
		for (size_t j = 0; j < m_emitters->at(i)->GetPositions().size(); j++)
		{
			m_particleValues.ParticleInfo[j].x = deltaTime;
			m_particleValues.ParticleInfo[j].y = m_emitters->at(i)->GetPositions()[j].TimeAlive;
			m_particleValues.ParticleInfo[j].z = m_emitters->at(i)->GetPositions()[j].TimeToLive;
			
			m_particleValues.ParticlePosition[j] = DirectX::XMFLOAT4A(
				m_emitters->at(i)->GetPositions()[j].Position.x,
				m_emitters->at(i)->GetPositions()[j].Position.y,
				m_emitters->at(i)->GetPositions()[j].Position.z,
				m_emitters->at(i)->GetPositions()[j].Position.w
			);
			m_particleValues.ParticleSpeed[j] = DirectX::XMFLOAT4A(
				m_emitters->at(i)->GetSettings().Direction.x,
				m_emitters->at(i)->GetSettings().Direction.y,
				m_emitters->at(i)->GetSettings().Direction.z,
				m_emitters->at(i)->GetSettings().Speed
			);
			m_particleValues.ParticleSize[j] = DirectX::XMFLOAT4A(
				m_emitters->at(i)->GetSettings().Size.x,
				m_emitters->at(i)->GetSettings().Size.y,
				m_emitters->at(i)->GetSettings().Size.z,
				m_emitters->at(i)->GetSettings().Size.w
				);
			memcpy(m_constantParticleBufferGPUAddress[*p_renderingManager->GetFrameIndex()] + i * m_constantParticleBufferPerObjectAlignedSize, &m_particleValues, sizeof(m_particleValues));
		}
	}

	ParticleEmitter * emitter = nullptr;
	for (size_t i = 0; i < m_emitters->size(); i++)
	{
		emitter = m_emitters->at(i);

		ID3D12GraphicsCommandList * commandList = emitter->GetCommandList();
		emitter->OpenCommandList();

		emitter->SwitchToUAVState(commandList);

		commandList->SetPipelineState(m_computePipelineState);
		commandList->SetComputeRootSignature(m_rootSignature);
		
		commandList->SetComputeRootConstantBufferView(0, m_constantParticleBuffer[*p_renderingManager->GetFrameIndex()]->GetGPUVirtualAddress() + i * m_constantParticleBufferPerObjectAlignedSize);
		commandList->SetComputeRootUnorderedAccessView(1, emitter->GetVertexResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(2, emitter->GetCalcResource()->GetGPUVirtualAddress());
		
		commandList->Dispatch(static_cast<UINT>(m_emitters->at(i)->GetPositions().size()), 1, 1);

		emitter->SwitchToVertexState(commandList);

		emitter->ExecuteCommandList();
		emitter->UpdateData();

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

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_constantParticleBuffer[i]);
	}
}

void ParticlePass::AddEmitter(ParticleEmitter* particleEmitter) const
{
	m_emitters->push_back(particleEmitter);
}

HRESULT ParticlePass::_initID3D12RootSignature()
{
	HRESULT hr = 0;

	D3D12_ROOT_DESCRIPTOR rootDescriptor;
	rootDescriptor.RegisterSpace = 0;
	rootDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR uavVertexDescriptor;
	uavVertexDescriptor.RegisterSpace = 0;
	uavVertexDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR uavCalculationsDescriptor;
	uavCalculationsDescriptor.RegisterSpace = 0;
	uavCalculationsDescriptor.ShaderRegister = 1;

	m_rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[0].Descriptor = rootDescriptor;
	m_rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	m_rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	m_rootParameters[1].Descriptor = uavVertexDescriptor;
	m_rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	m_rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	m_rootParameters[2].Descriptor = uavCalculationsDescriptor;
	m_rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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

	ID3DBlob * signature = nullptr;
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		nullptr)))
	{
		if (FAILED(hr = p_renderingManager->GetDevice()->CreateRootSignature(
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

	if (FAILED(hr = p_renderingManager->GetDevice()->CreateComputePipelineState(
		&computePipelineStateDesc,
		IID_PPV_ARGS(&m_computePipelineState))))
	{
		
	}

	return hr;
}

HRESULT ParticlePass::_createUAVOutput()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantParticleBuffer[i]))))
		{
			SET_NAME(m_constantParticleBuffer[i], L"Particle ConstantLightBuffer Upload Resource Heap");
		}
		else
			return hr;

		CD3DX12_RANGE readRange(0, 0);
		if (SUCCEEDED(hr = m_constantParticleBuffer[i]->Map(
			0, &readRange,
			reinterpret_cast<void**>(&m_constantParticleBufferGPUAddress[i]))))
		{
			memcpy(m_constantParticleBufferGPUAddress[i], &m_particleValues, sizeof(ParticleBuffer));
		}
	}
	return hr;
}
