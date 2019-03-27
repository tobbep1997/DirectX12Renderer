#pragma once
#include "Template/IX12Object.h"
class X12StructuredBuffer : public IX12Object
{
public:
	X12StructuredBuffer() = default;
	~X12StructuredBuffer() = default;

	HRESULT Create(const std::wstring& name, const UINT& size);
	void Copy(void * data, const UINT & size, const UINT & offset = 0);
	

	void SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex);
	void Release() override;

private:
	UINT8 * m_resourceAddress[FRAME_BUFFER_COUNT] = {nullptr};
	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle[FRAME_BUFFER_COUNT]{ 0 };
};

