#pragma once
#include "DirectX12EnginePCH.h"


class ShaderCreator
{	
public:
	static HRESULT CreateShader(const std::wstring & path, ID3DBlob *& blob, const std::string & target, const std::string & entryPoint = "main")
	{
		HRESULT hr;
		std::wstring newPath;
#ifdef DXPATH
		newPath = L"../" + path;
#else
		newPath = path;
#endif
		
		ID3DBlob * errorBlob = nullptr;
		if (FAILED(hr = D3DCompileFromFile(
			newPath.c_str(),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			target.c_str(),
			D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES,
			0,
			&blob,
			&errorBlob
		)))
		{
			if (!errorBlob)
				return hr;
			OutputDebugStringW(std::wstring(L"\n" + path + L"\n{\n\n ").c_str());
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
			OutputDebugStringW(std::wstring(L"\n\n}\n").c_str());
		}

		if (errorBlob) errorBlob->Release(); errorBlob = nullptr;
		return hr;
	}
};
