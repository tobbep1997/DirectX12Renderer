#pragma once

class IX12Object
{
protected:
	RenderingManager * p_renderingManager;
	const Window * p_window;
	IX12Object(RenderingManager * renderingManager, const Window & window)
	{
		this->p_renderingManager = renderingManager;
		this->p_window = &window;
	}
public:
	virtual~IX12Object() = default;

	virtual void Release() = 0;
};
