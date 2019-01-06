#pragma once
#include "DirectX12EnginePCH.h"

class Camera;

class IRender
{
protected:
	RenderingManager * p_renderingManager;
	const Window * p_window;

	IRender(RenderingManager * renderingManager,
		const Window & window);


	std::vector<Drawable*> * p_drawQueue;
	std::vector<ILight*> * p_lightQueue;

	ID3D12GraphicsCommandList * p_commandList = nullptr;
	ID3D12CommandAllocator * p_commandAllocator[FRAME_BUFFER_COUNT] { nullptr };

	HRESULT p_createCommandList();
	void p_releaseCommandList();

public:

	HRESULT OpenCommandList();
	HRESULT ExecuteCommandList() const;	

	virtual~IRender();

	virtual HRESULT Init()	= 0;
	virtual void Update(const Camera & camera)	= 0;
	virtual void Draw()		= 0;
	virtual void Clear()	= 0;
	virtual void Release()	= 0;

	void Queue(Drawable * drawable) const;
	void QueueLight(ILight * light) const;	
};