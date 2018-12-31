#pragma once
class RenderingHelpClass
{
public:
	static void CreateRootDescriptorTable(
		D3D12_DESCRIPTOR_RANGE & descriptorRangeTable,
		D3D12_ROOT_DESCRIPTOR_TABLE & descriptorTable,
		const UINT & baseShaderRegister = 0,
		const UINT & registerSpace = 0,
		const UINT & numDescriptors = 1,
		const D3D12_DESCRIPTOR_RANGE_TYPE & descriptorRange = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		const UINT & offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
};
