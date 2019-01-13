#pragma once
#include <thread>
#include "DirectX12EnginePCH.h"
#include "../WrapperFunctions/Functions/Instancing.h"

class Camera;

class IRender
{
private:
	std::thread m_thread;
	BOOL m_threadRunning;
	BOOL m_threadDone;

	Camera m_camera;
	float m_deltaTime;

	void _updateWithThreads();

protected:
	RenderingManager * p_renderingManager;
	const Window * p_window;

	IRender(RenderingManager * renderingManager,
		const Window & window);


	std::vector<Drawable*> * p_drawQueue = nullptr;
	std::vector<ILight*> * p_lightQueue = nullptr;

	std::vector<Instancing::InstanceGroup> * p_instanceGroups = nullptr;
	ID3D12Resource * p_instanceBuffer = nullptr;
	ID3D12Resource * p_intermediateInstanceBuffer = nullptr;

	ID3D12GraphicsCommandList * p_commandList = nullptr;
	ID3D12CommandAllocator * p_commandAllocator[FRAME_BUFFER_COUNT] { nullptr };

	HRESULT p_createCommandList();
	void p_releaseCommandList();

	HRESULT p_createInstanceBuffer(const UINT & bufferSize = 1024u * 64u);
	UINT64 p_updateInstanceBuffer(D3D12_VERTEX_BUFFER_VIEW & vertexBufferView) const;

	void p_drawInstance(const UINT & textureStartIndex = 0, const BOOL & mapTextures = FALSE) const;
	void p_releaseInstanceBuffer();
	
public:

	HRESULT OpenCommandList();
	HRESULT ExecuteCommandList() const;	

	virtual~IRender();

	virtual HRESULT Init()	= 0;
	virtual void Update(const Camera & camera, const float & deltaTime)	= 0;
	virtual void Draw()		= 0;
	virtual void Clear()	= 0;
	virtual void Release()	= 0;

	void ThreadUpdate(const Camera & camera, const float & deltaTime);
	void ThreadJoin() const;
	void KillThread();

	void Queue(Drawable * drawable) const;
	void QueueLight(ILight * light) const;	
};