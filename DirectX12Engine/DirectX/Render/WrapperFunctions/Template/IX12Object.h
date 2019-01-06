#pragma once

class IX12Object
{
protected:
	RenderingManager * p_renderingManager;
	const Window * p_window;
	ID3D12GraphicsCommandList * p_commandList;

	IX12Object(RenderingManager * renderingManager, const Window & window, ID3D12GraphicsCommandList * commandList = nullptr)
	{
		this->p_renderingManager = renderingManager;
		this->p_window = &window;

		if (commandList)
			this->p_commandList = commandList;
		else
			this->p_commandList = this->p_renderingManager->GetCommandList();
	}
public:
	virtual~IX12Object() = default;

	virtual void Release() = 0;
};
