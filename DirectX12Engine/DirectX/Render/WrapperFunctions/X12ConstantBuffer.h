#pragma once
#include "Template/IX12Object.h"

class X12ConstantBuffer :
	public IX12Object
{
public:
	X12ConstantBuffer(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr);
	~X12ConstantBuffer();

	HRESULT CreateBuffer(const std::wstring & name, void const* data, const UINT & sizeOf);

	void SetComputeRootConstantBufferView(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList = nullptr);
	void SetGraphicsRootConstantBufferView(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList = nullptr);
	void Copy(void const* data, const UINT & sizeOf);
	void Release() override;

	ID3D12Resource*const* GetResource() const;
	ID3D12DescriptorHeap*const* GetDescriptorHeap() const;

private:
	ID3D12Resource			* m_constantBuffer[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12DescriptorHeap	* m_constantBufferDescriptorHeap[FRAME_BUFFER_COUNT] = { nullptr };

	UINT8* m_constantBufferGPUAddress[FRAME_BUFFER_COUNT] = { nullptr };
};

