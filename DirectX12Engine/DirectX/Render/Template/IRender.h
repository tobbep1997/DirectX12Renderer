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
		p_lightQueue = new std::vector<ILight*>();
	}

	std::vector<Drawable*> * p_drawQueue;
	std::vector<ILight*> * p_lightQueue;
public:
	virtual~IRender()
	{
		delete p_drawQueue;
		delete p_lightQueue;
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

	void QueueLight(ILight * light) const
	{
		p_lightQueue->push_back(light);
	}
	
};