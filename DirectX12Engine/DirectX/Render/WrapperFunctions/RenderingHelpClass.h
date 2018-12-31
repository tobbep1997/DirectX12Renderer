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

	static void CreateSampler(
		D3D12_STATIC_SAMPLER_DESC & sampler,
		const UINT & shaderRegister,
		const UINT & registerSpace,
		const D3D12_SHADER_VISIBILITY & shaderVisibility,
		const D3D12_FILTER & filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		const D3D12_TEXTURE_ADDRESS_MODE & addressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		const D3D12_TEXTURE_ADDRESS_MODE & addressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		const D3D12_TEXTURE_ADDRESS_MODE & addressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		const FLOAT & mipLODBias = 0.0f,
		const UINT & maxAnisotropy = 0,
		const D3D12_COMPARISON_FUNC & comparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		const D3D12_STATIC_BORDER_COLOR & borderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
		const FLOAT & minLOD = 0.0f,
		const FLOAT & maxLOD = D3D12_FLOAT32_MAX
	);
};
