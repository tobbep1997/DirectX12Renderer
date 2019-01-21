#include "DXTexture.h"
#include "Converter.h"


namespace ID3D12
{
	DXTexture::DXTexture() : IManagedObject(new Texture(), true)
	{
	}

	void DXTexture::Release()
	{
		p_instance->Release();
	}

	void DXTexture::SetRenderingManager(DXRenderingManager renderingManager)
	{
		p_instance->SetRenderingManager(renderingManager.GetInstance());
	}

	bool DXTexture::LoadTexture(System::String^ path)
	{
		return p_instance->LoadTexture(std::string(::Converter::SystemStringToCharArr(path)));
	}

	bool DXTexture::LoadTexture(System::String^ path, bool generateMip)
	{
		return p_instance->LoadTexture(std::string(::Converter::SystemStringToCharArr(path)), generateMip);
	}

	bool DXTexture::LoadTexture(System::String^ path, bool generateMip, DXRenderingManager^ renderingManager)
	{
		return p_instance->LoadTexture(std::string(::Converter::SystemStringToCharArr(path)), generateMip, renderingManager->GetInstance());
	}

	bool DXTexture::LoadDDSTexture(System::String^ path)
	{
		return p_instance->LoadDDSTexture(std::string(::Converter::SystemStringToCharArr(path)));
	}

	bool DXTexture::LoadDDSTexture(System::String^ path, bool generateMip)
	{
		return p_instance->LoadDDSTexture(std::string(::Converter::SystemStringToCharArr(path)), generateMip);
	}

	bool DXTexture::LoadDDSTexture(System::String^ path, bool generateMip, DXRenderingManager^ renderingManager)
	{
		return p_instance->LoadDDSTexture(std::string(::Converter::SystemStringToCharArr(path)), generateMip, renderingManager->GetInstance());
	}
}
