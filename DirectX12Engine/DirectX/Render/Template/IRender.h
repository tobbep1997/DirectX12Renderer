#pragma once
#include "DirectX12EnginePCH.h"

class IRender
{
protected:
	ID3D12Device * p_device;

	IRender(ID3D12Device * device) { this->p_device = device; }
public:
	virtual~IRender() = default;

	virtual HRESULT Init()		= 0;
	virtual HRESULT Update()	= 0;
	virtual HRESULT Draw()		= 0;
	virtual HRESULT Release()	= 0;

	
};