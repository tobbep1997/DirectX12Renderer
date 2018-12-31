#include "DirectX12EnginePCH.h"
#include "RenderingHelpClass.h"


void RenderingHelpClass::CreateRootDescriptorTable(
	D3D12_DESCRIPTOR_RANGE & descriptorRangeTable,
	D3D12_ROOT_DESCRIPTOR_TABLE & descriptorTable,
	const UINT& baseShaderRegister, const UINT& registerSpace,
	const UINT& numDescriptors, const D3D12_DESCRIPTOR_RANGE_TYPE& descriptorRange,
	const UINT& offsetInDescriptorsFromTableStart)
{	
	descriptorRangeTable.BaseShaderRegister = baseShaderRegister;
	descriptorRangeTable.RegisterSpace = registerSpace;
	descriptorRangeTable.NumDescriptors = numDescriptors;
	descriptorRangeTable.RangeType = descriptorRange;
	descriptorRangeTable.OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
	   
	descriptorTable.NumDescriptorRanges = 1;
	descriptorTable.pDescriptorRanges = &descriptorRangeTable;
}
