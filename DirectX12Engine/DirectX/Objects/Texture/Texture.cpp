#include "DirectX12EnginePCH.h"
#include "Texture.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <ResourceUploadBatch.h>

Texture::Texture()
{
	this->m_renderingManager = nullptr;
	this->m_imageData = nullptr;
}

Texture::~Texture()
{
}

BOOL Texture::Init()
{
	return TRUE;
}

void Texture::Update()
{
}

void Texture::Release()
{	
	SAFE_DELETE_ARRAY(m_imageData);
	SAFE_RELEASE(m_textureBuffer);
}



BOOL Texture::LoadTexture(const std::string& path, const BOOL & generateMips)
{
	if (!m_renderingManager)
		m_renderingManager = RenderingManager::GetInstance();


	HRESULT hr = 0;
	static BOOL init = FALSE;
	if (!init)
	{
		if (FAILED(hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
		{
			return FALSE;
		}
		init = TRUE;
	}
	DirectX::ResourceUploadBatch resourceUpload(m_renderingManager->GetMainAdapter()->GetDevice());

	resourceUpload.Begin();
	
	if (SUCCEEDED(hr = DirectX::CreateWICTextureFromFile(
		m_renderingManager->GetMainAdapter()->GetDevice(),
		resourceUpload, 
		DEBUG::StringToWstring(path).c_str(), 
		&m_textureBuffer, 
		generateMips)))
	{
		auto uploadResourceFinish = resourceUpload.End(m_renderingManager->GetCommandQueue());
		uploadResourceFinish.wait();
	}
	else
		Window::CreateError(hr);		
	   
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;


	SET_NAME(m_textureBuffer, DEBUG::StringToWstring(path) + L" Texture DescriptorHeap");

	const D3D12_RESOURCE_DESC resourceDesc = m_textureBuffer->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = resourceDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

	m_cpuHandle = m_renderingManager->GetMainAdapter()->GetNextHandle().DescriptorHandle;
	m_renderingManager->GetMainAdapter()->GetDevice()->CreateShaderResourceView(
		m_textureBuffer,
		&srvDesc,
		m_cpuHandle);

	
	return TRUE;	
}

BOOL Texture::LoadDDSTexture(const std::string& path, const BOOL & generateMips)
{
	if (!m_renderingManager)
		m_renderingManager = RenderingManager::GetInstance();

	HRESULT hr = 0;

	static BOOL init = FALSE;
	if (!init)
	{
		if (FAILED(hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
		{
			return FALSE;
		}
		init = TRUE;
	}
	DirectX::ResourceUploadBatch resourceUpload(m_renderingManager->GetMainAdapter()->GetDevice());

	resourceUpload.Begin();
		
	if (SUCCEEDED(hr = DirectX::CreateDDSTextureFromFile(
		m_renderingManager->GetMainAdapter()->GetDevice(),
		resourceUpload,
		DEBUG::StringToWstring(path).c_str(),
		&m_textureBuffer,
		generateMips)))
	{
		auto uploadResourceFinish = resourceUpload.End(m_renderingManager->GetCommandQueue());
		uploadResourceFinish.wait();
	}
	else
	{
		Window::CreateError(hr);
		return FALSE;
	}
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	

	
	SET_NAME(m_textureBuffer, DEBUG::StringToWstring(path) + L" Texture DescriptorHeap");

	const D3D12_RESOURCE_DESC resourceDesc = m_textureBuffer->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = resourceDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

	m_cpuHandle = m_renderingManager->GetMainAdapter()->GetNextHandle().DescriptorHandle;
	m_renderingManager->GetMainAdapter()->GetDevice()->CreateShaderResourceView(
		m_textureBuffer,
		&srvDesc,
		m_cpuHandle);

	
	return TRUE;
}

ID3D12Resource* Texture::GetResource() const
{
	return this->m_textureBuffer;
}

void Texture::CopyDescriptorHeap() const
{
	m_gpuHandle = m_renderingManager->CopyToGpuDescriptorHeap(m_cpuHandle, m_textureBuffer->GetDesc().DepthOrArraySize);
}

void Texture::MapTexture(const UINT& rootParameterIndex, ID3D12GraphicsCommandList * commandList) const
{
	if (m_gpuHandle.ptr == 0)
		throw "GPU handle null";

	ID3D12GraphicsCommandList * gcl = commandList ? commandList : m_renderingManager->GetCommandList();
	
	gcl->SetGraphicsRootDescriptorTable(rootParameterIndex, m_gpuHandle);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& Texture::GetCpuHandle() const
{
	return this->m_cpuHandle;
}

HRESULT Texture::_uploadTexture()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = m_renderingManager->SignalGPU()))
	{
		SAFE_DELETE_ARRAY(m_imageData);
		m_imageData = nullptr;
	}
	return hr;
}
