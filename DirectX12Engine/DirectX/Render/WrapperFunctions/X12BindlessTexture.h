#pragma once
#include "Template/IX12Object.h"

class X12BindlessTexture : public IX12Object
{
public:
	X12BindlessTexture();
	~X12BindlessTexture();

	void ResetDescriptorHandle();
	void PushBackTexture(const Texture & texture); //pushes back a texture to the visible descriptor heap
	void PushBackCpuHandle(const D3D12_CPU_DESCRIPTOR_HANDLE & cpuHandle, const UINT & arraySize = 1);
	void Release() override;

	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex) const;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle {0};
	UINT m_numberOfTexture = 0;
};

