#include "DirectX12EnginePCH.h"
#include "X12BindlessTexture.h"


X12BindlessTexture::X12BindlessTexture() : IX12Object(RenderingManager::GetInstance(), *Window::GetInstance())
{
}


X12BindlessTexture::~X12BindlessTexture()
{
}

void X12BindlessTexture::ResetDescriptorHandle()
{
	m_numberOfTexture = 0;
}

void X12BindlessTexture::PushBackTexture(const Texture& texture)
{
	if (m_numberOfTexture == 0)
		m_GpuHandle = p_renderingManager->CopyToGpuDescriptorHeap(texture.GetCpuHandle(), texture.GetResource()->GetDesc().DepthOrArraySize);
	else
		p_renderingManager->CopyToGpuDescriptorHeap(texture.GetCpuHandle(), texture.GetResource()->GetDesc().DepthOrArraySize);
	m_numberOfTexture++;
}

void X12BindlessTexture::PushBackCpuHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, const UINT & arraySize)
{
	if (m_numberOfTexture == 0)
		m_GpuHandle = p_renderingManager->CopyToGpuDescriptorHeap(cpuHandle, arraySize);
	else
		p_renderingManager->CopyToGpuDescriptorHeap(cpuHandle, arraySize);
	m_numberOfTexture++;
}

void X12BindlessTexture::Release()
{
	
}

void X12BindlessTexture::SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* commandList, const UINT& rootParameterIndex) const
{
	if (m_numberOfTexture != 0)
		commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_GpuHandle);		
}
