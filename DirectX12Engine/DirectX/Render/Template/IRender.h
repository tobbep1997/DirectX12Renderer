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

	std::vector<Drawable*> p_drawQueue;

public:
	virtual~IRender() = default;


	virtual HRESULT Init()		= 0;
	virtual HRESULT Update(const Camera & camera)	= 0;
	virtual HRESULT Draw()		= 0;
	virtual HRESULT Clear()		= 0;
	virtual HRESULT Release()	= 0;

	void Queue(Drawable * drawable)
	{
		p_drawQueue.push_back(drawable);
	}
	
};