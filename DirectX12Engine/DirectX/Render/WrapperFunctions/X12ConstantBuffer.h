#pragma once
#include "Template/IX12Object.h"

class X12ConstantBuffer :
	public IX12Object
{
public:
	X12ConstantBuffer(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr);
	~X12ConstantBuffer();

	HRESULT CreateBuffer(const std::wstring & name, void const* data, const UINT & sizeOf, const UINT & preAllocData = 0);

	void SetComputeRootConstantBufferView(const UINT & rootParameterIndex, const UINT & offset = 0, ID3D12GraphicsCommandList * commandList = nullptr);
	void SetGraphicsRootConstantBufferView(const UINT & rootParameterIndex, const UINT & offset = 0, ID3D12GraphicsCommandList * commandList = nullptr);
	void SetGraphicsRootShaderResourceView(const UINT & rootParameterIndex, const UINT & offset = 0, ID3D12GraphicsCommandList * commandList = nullptr);
	void Copy(void const* data, const UINT & sizeOf, const UINT & offset = 0);
	void Release() override;

	ID3D12Resource*const* GetResource() const;

private:
	ID3D12Resource			* m_constantBuffer[FRAME_BUFFER_COUNT] = { nullptr };

	UINT8* m_constantBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };
	SIZE_T m_descriptorHeapOffset = 0;
};

