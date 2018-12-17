#pragma once
#include "DirectX12EnginePCH.h"


class ShaderCreator
{	
public:
	static HRESULT CreateShader(const std::wstring & path, D3D12_SHADER_BYTECODE & byteCode, const std::string & target, const std::string & entryPoint = "main")
	{
		HRESULT hr;
		ID3DBlob * blob			= nullptr;
		ID3DBlob * errorBlob	= nullptr;


		if (SUCCEEDED(hr = D3DCompileFromFile(
			path.c_str(),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			target.c_str(),
			0,
			0,
			&blob,
			&errorBlob
		)))
		{
			OutputDebugStringW(std::wstring(std::to_wstring(blob->GetBufferSize())).c_str());
			(byteCode).BytecodeLength = blob->GetBufferSize();
			(byteCode).pShaderBytecode = blob->GetBufferPointer();
		}

		if (FAILED(hr))
		{
			OutputDebugStringW(std::wstring(L"\n"+path + L"\n{\n\n ").c_str());
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
			OutputDebugStringW(std::wstring(L"\n\n}\n").c_str());

		}
		if (blob) blob->Release(); blob = nullptr;
		if (errorBlob) errorBlob->Release(); errorBlob = nullptr;
		return hr;
	}
};
