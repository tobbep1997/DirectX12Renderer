#pragma once

class IX12Object
{
protected:
	RenderingManager * p_renderingManager;
	const Window * p_window;

	IX12Object()
	{
		this->p_renderingManager = RenderingManager::GetInstance();
		this->p_window = Window::GetInstance();
	}
public:
	virtual~IX12Object() = default;

	virtual void Release() = 0;
};
