#pragma once
#include "DirectX12EnginePCH.h"

class Camera;

class IRender
{
protected:
	RenderingManager * p_renderingManager;
	const Window * p_window;
	IRender(RenderingManager * renderingManager, 
		const Window & window)
	{
		this->p_renderingManager = renderingManager;
		this->p_window = &window;
	}
public:
	virtual~IRender() = default;

	virtual HRESULT Init()		= 0;
	virtual HRESULT Update(const Camera & camera)	= 0;
	virtual HRESULT Draw()		= 0;
	virtual HRESULT Release()	= 0;

	
};