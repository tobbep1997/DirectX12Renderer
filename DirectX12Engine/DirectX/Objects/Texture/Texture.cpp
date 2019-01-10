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

void Texture::Init()
{
}

void Texture::Update()
{
}

void Texture::Release()
{	
	SAFE_DELETE_ARRAY(m_imageData);
	SAFE_RELEASE(m_textureBuffer);
	SAFE_RELEASE(m_textureUploadHeap);
	SAFE_RELEASE(m_textureDescriptorHeap);	
}

void Texture::SetRenderingManager(RenderingManager* renderingManager)
{
	this->m_renderingManager = renderingManager;
}

BOOL Texture::LoadTexture(const std::string& path, const BOOL & generateMips, RenderingManager * renderingManager)
{
	if (!m_renderingManager && !renderingManager)
		return FALSE;
	if (!m_renderingManager && renderingManager)
		m_renderingManager = renderingManager;

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
	DirectX::ResourceUploadBatch resourceUpload(m_renderingManager->GetDevice());

	resourceUpload.Begin();

	;
	if (SUCCEEDED(hr = DirectX::CreateWICTextureFromFile(
		m_renderingManager->GetDevice(),
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

	if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&m_textureDescriptorHeap))))
	{
		SET_NAME(m_textureBuffer, DEBUG::StringToWstring(path) + L" Texture DescriptorHeap");

		const D3D12_RESOURCE_DESC resourceDesc = m_textureBuffer->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = resourceDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

		m_renderingManager->GetDevice()->CreateShaderResourceView(
			m_textureBuffer,
			&srvDesc,
			m_textureDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	return TRUE;	
}

BOOL Texture::LoadDDSTexture(const std::string& path, const BOOL & generateMips, RenderingManager* renderingManager)
{
	if (!m_renderingManager && !renderingManager)
		return FALSE;
	if (!m_renderingManager && renderingManager)
		m_renderingManager = renderingManager;

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
	DirectX::ResourceUploadBatch resourceUpload(m_renderingManager->GetDevice());

	resourceUpload.Begin();
		
	if (SUCCEEDED(hr = DirectX::CreateDDSTextureFromFile(
		m_renderingManager->GetDevice(),
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

	if (SUCCEEDED(hr = m_renderingManager->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&m_textureDescriptorHeap))))
	{
		SET_NAME(m_textureBuffer, DEBUG::StringToWstring(path) + L" Texture DescriptorHeap");

		const D3D12_RESOURCE_DESC resourceDesc = m_textureBuffer->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = resourceDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

		m_renderingManager->GetDevice()->CreateShaderResourceView(
			m_textureBuffer,
			&srvDesc,
			m_textureDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	return TRUE;
}

ID3D12Resource* Texture::GetResource() const
{
	return this->m_textureBuffer;
}

ID3D12DescriptorHeap* Texture::GetId3D12DescriptorHeap() const
{
	return m_textureDescriptorHeap;
}

void Texture::MapTexture(RenderingManager* renderingManager, const UINT& rootParameterIndex, ID3D12GraphicsCommandList * commandList) const
{
	ID3D12GraphicsCommandList * gcl = commandList ? commandList : renderingManager->GetCommandList();

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_textureDescriptorHeap };
	gcl->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);	
	gcl->SetGraphicsRootDescriptorTable(rootParameterIndex, m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
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