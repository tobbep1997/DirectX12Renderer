#pragma once
#include "Template/IX12Object.h"
class X12StructuredBuffer : public IX12Object
{
public:
	X12StructuredBuffer(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr);
	~X12StructuredBuffer();

	HRESULT Create(const std::wstring& name, const UINT& size);
	void Copy(void * data, const UINT & size, const UINT & offset = 0);
	void Map(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList = nullptr);
	

	void Release() override;

private:
	UINT8 * m_resourceAddress[FRAME_BUFFER_COUNT] = {nullptr};
	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };
	SIZE_T m_descriptorHeapOffset = 0;
};

