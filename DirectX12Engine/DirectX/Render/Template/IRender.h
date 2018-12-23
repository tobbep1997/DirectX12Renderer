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
		p_drawQueue = new std::vector<Drawable*>();
	}

	std::vector<Drawable*> * p_drawQueue;

public:
	virtual~IRender()
	{
		delete p_drawQueue;
	}


	virtual HRESULT Init()		= 0;
	virtual HRESULT Update(const Camera & camera)	= 0;
	virtual HRESULT Draw()		= 0;
	virtual HRESULT Clear()		= 0;
	virtual HRESULT Release()	= 0;

	void Queue(Drawable * drawable) const
	{
		p_drawQueue->push_back(drawable);
	}
	
};