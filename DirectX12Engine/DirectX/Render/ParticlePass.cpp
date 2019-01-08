#include "DirectX12EnginePCH.h"
#include "ParticlePass.h"
#include "WrapperFunctions/X12ConstantBuffer.h"


ParticlePass::ParticlePass(RenderingManager* renderingManager, const Window& window)
	: IRender(renderingManager, window)
{
	m_particleValues.ParticleInfo.x = 5;
	for (UINT i = 0; i < 5; i++)
	{
		m_particleValues.ParticlePosition[i] = DirectX::XMFLOAT4A(1 * i, 5, 0, 1);
	}
}

ParticlePass::~ParticlePass()
= default;

HRESULT ParticlePass::Init()
{
	HRESULT hr = 0;

	if (SUCCEEDED(hr = p_createCommandList()))
	{
		if (SUCCEEDED(hr = OpenCommandList()))
		{
			if (SUCCEEDED(hr = _initID3D12RootSignature()))
			{
				if (SUCCEEDED(hr = _initShaders()))
				{
					if (SUCCEEDED(hr = _initPipelineState()))
					{
						SAFE_NEW(m_particleBuffer, new X12ConstantBuffer(p_renderingManager, *p_window, p_commandList));
						if (SUCCEEDED(hr = m_particleBuffer->CreateBuffer(L"Particle constant", &m_particleValues, sizeof(ParticleBuffer))))
						{
							if (SUCCEEDED(hr = _createUAVOutput()))
							{
								
							}
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

void ParticlePass::Update(const Camera& camera)
{
	m_particleValues.CameraPosition = DirectX::XMFLOAT4A(
		camera.GetPosition().x,
		camera.GetPosition().y,
		camera.GetPosition().z,
		camera.GetPosition().w);
	m_particleBuffer->Copy(&m_particleValues, sizeof(m_particleValues));

	OpenCommandList();
	
	p_commandList->SetPipelineState(m_computePipelineState);
	p_commandList->SetComputeRootSignature(m_rootSignature);

	m_particleBuffer->SetComputeRootConstantBufferView(0);
	p_commandList->SetComputeRootUnorderedAccessView(1, m_particleUAVOutput->GetGPUVirtualAddress());
	
	p_commandList->Dispatch(m_particleValues.ParticleInfo.x, 1, 1);

	ExecuteCommandList();


}

void ParticlePass::Draw()
{
}

void ParticlePass::Clear()
{
	this->p_drawQueue->clear();
	this->p_lightQueue->clear();
	Instancing::ClearInstanceGroup(this->p_instanceGroups);
}

void ParticlePass::Release()
{
	p_releaseCommandList();
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_computePipelineState);

	SAFE_RELEASE(m_particleUAVOutput);
	SAFE_RELEASE(m_particleUAVOutputDescriptorHeap);

	m_particleBuffer->Release();
	SAFE_DELETE(m_particleBuffer);
}

HRESULT ParticlePass::_initID3D12RootSignature()
{
	HRESULT hr = 0;

	D3D12_ROOT_DESCRIPTOR rootDescriptor;
	rootDescriptor.RegisterSpace = 0;
	rootDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR uavDescriptor;
	uavDescriptor.RegisterSpace = 0;
	uavDescriptor.ShaderRegister = 0;

	m_rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	m_rootParameters[0].Descriptor = rootDescriptor;
	m_rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	m_rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	m_rootParameters[1].Descriptor = uavDescriptor;
	m_rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


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

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateDescriptorHeap(
		&descriptorHeapDesc,
		IID_PPV_ARGS(&m_particleUAVOutputDescriptorHeap))))
	{
		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64);
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;			   		 	  
		
		if (SUCCEEDED(hr = p_renderingManager->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&m_particleUAVOutput))))
		{
			SET_NAME(m_particleUAVOutput, L"Particle UAV output");

			D3D12_BUFFER_UAV uav{};
			uav.NumElements = 256;
			uav.FirstElement = 0;
			uav.StructureByteStride = sizeof(DirectX::XMFLOAT3X3);
			uav.CounterOffsetInBytes = 0;
			uav.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		
			D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
			unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
			unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			unorderedAccessViewDesc.Buffer = uav;

			p_renderingManager->GetDevice()->CreateUnorderedAccessView(
				m_particleUAVOutput,
				nullptr,
				&unorderedAccessViewDesc,
				m_particleUAVOutputDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		}



	}

	return hr;
}
