#pragma once
#include "DirectX12EnginePCH.h"
#include "DirectX12EnginePCH.h"


namespace ShaderCreator
{	
	inline HRESULT CreateShader(const std::wstring & path, D3D12_SHADER_BYTECODE * byteCode, const std::string & target, const std::string & entryPoint = "main")
	{
		HRESULT hr;
		ID3DBlob * blob			= nullptr;
		ID3DBlob * errorBlob	= nullptr;

		UINT flag = 0;

#ifdef _DEBUG
		flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		if (SUCCEEDED(hr = D3DCompileFromFile(
			path.c_str(),
			nullptr,
			nullptr,
			entryPoint.c_str(),
			target.c_str(),
			flag,
			0,
			&blob,
			&errorBlob
		)))
		{
			byteCode->BytecodeLength = blob->GetBufferSize();
			byteCode->pShaderBytecode = blob->GetBufferPointer();
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
}
