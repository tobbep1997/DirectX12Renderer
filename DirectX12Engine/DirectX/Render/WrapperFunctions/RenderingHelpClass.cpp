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

void RenderingHelpClass::CreateSampler(D3D12_STATIC_SAMPLER_DESC& sampler, const UINT& shaderRegister,
	const UINT& registerSpace, const D3D12_SHADER_VISIBILITY& shaderVisibility, const D3D12_FILTER& filter,
	const D3D12_TEXTURE_ADDRESS_MODE& addressU, const D3D12_TEXTURE_ADDRESS_MODE& addressV,
	const D3D12_TEXTURE_ADDRESS_MODE& addressW, const FLOAT& mipLODBias, const UINT& maxAnisotropy,
	const D3D12_COMPARISON_FUNC& comparisonFunc, const D3D12_STATIC_BORDER_COLOR& borderColor, const FLOAT& minLOD,
	const FLOAT& maxLOD)
{
	sampler.Filter = filter;
	sampler.AddressU = addressU;
	sampler.AddressV = addressV;
	sampler.AddressW = addressW;
	sampler.MipLODBias = mipLODBias;
	sampler.MaxAnisotropy = maxAnisotropy;
	sampler.ComparisonFunc = comparisonFunc;
	sampler.BorderColor = borderColor;
	sampler.MinLOD = minLOD;
	sampler.MaxLOD = maxLOD;
	sampler.ShaderRegister = shaderRegister;
	sampler.RegisterSpace = registerSpace;
	sampler.ShaderVisibility = shaderVisibility;
}
