#pragma once
#include "Template/IX12Object.h"

class X12ConstantBuffer :
	public IX12Object
{

public:
	X12ConstantBuffer() = default;
	~X12ConstantBuffer() = default;

	HRESULT CreateBuffer(const std::wstring & name, void const* data, const UINT & sizeOf, const UINT & preAllocData = 0);
	HRESULT CreateSharedBuffer(const std::wstring & name, const UINT & sizeOf, const UINT & preAllocData = 0);

	void SetComputeRootConstantBufferView(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex, const UINT & offset = 0);
	void SetComputeRootShaderResourceView(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex, const UINT & offset = 0);

	void SetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex, const UINT & offset = 0);
	void SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList * commandList, const UINT & rootParameterIndex, const UINT & offset = 0);

	void Copy(void const* data, const UINT & sizeOf, const UINT & offset = 0);
	void Release() override;

	ID3D12Resource*const* GetResource() const;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_handle[FRAME_BUFFER_COUNT] {0};
	ID3D12Resource			* m_constantBuffer[FRAME_BUFFER_COUNT] = { nullptr };

	UINT8* m_constantBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };
	
};

