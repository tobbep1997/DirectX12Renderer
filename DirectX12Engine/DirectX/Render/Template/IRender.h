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

	ID3D12DescriptorHeap * m_gpuDescriptorHeap;
	SIZE_T m_copyOffset;
	SIZE_T m_resourceIncrementalSize;

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

	ID3D12CommandAllocator * p_commandAllocator[FRAME_BUFFER_COUNT] { nullptr };
	ID3D12GraphicsCommandList * p_commandList[FRAME_BUFFER_COUNT] = { nullptr };

	HRESULT p_createCommandList(const std::wstring & name, const D3D12_COMMAND_LIST_TYPE & type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	void p_releaseCommandList();

	void p_resetDescriptorHeap();
	void p_setResourceDescriptorHeap(ID3D12GraphicsCommandList * commandList) const;
	HRESULT p_createDescriptorHeap();
	void p_releaseDescriptorHeap();
	D3D12_GPU_DESCRIPTOR_HANDLE p_copyToDescriptorHeap(const D3D12_CPU_DESCRIPTOR_HANDLE & descriptorHandle, const UINT & numDescriptors = 1);
	

	HRESULT p_createInstanceBuffer(const std::wstring & name, const UINT & bufferSize = 1024u * 64u);
	UINT64 p_updateInstanceBuffer(D3D12_VERTEX_BUFFER_VIEW & vertexBufferView) const;

	void p_drawInstance(const UINT & textureStartIndex = 0, const BOOL & mapTextures = FALSE);
	void p_releaseInstanceBuffer();
	
public:

	HRESULT OpenCommandList(ID3D12PipelineState * pipelineState = nullptr);
	HRESULT ExecuteCommandList(ID3D12CommandQueue * commandQueue = nullptr) const;	

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