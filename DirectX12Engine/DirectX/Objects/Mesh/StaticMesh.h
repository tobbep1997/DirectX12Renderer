#pragma once
#include "DirectX/Structs.h"
#include "../Template/IObject.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <d3d12.h>
#include "DirectX/Render/WrapperFunctions/X12Fence.h"

class RenderingManager;
struct aiScene;

class StaticMesh : //NOLINT
	public IObject
{
public:
	BOOL Init() override;
	void Update() override;
	void Release() override;
	StaticMesh();
	~StaticMesh();

	const BOOL & LoadStaticMesh(const std::string & path);

	BOOL CreateBuffer();
	
	const std::vector<StaticVertex> & GetStaticMesh() const;

	const D3D12_VERTEX_BUFFER_VIEW & GetVertexBufferView() const;

private:
	HRESULT _createBuffer();
	void _clearMesh();
	BOOL _createMesh(const aiScene * scene);

	BOOL m_meshLoaded = FALSE;
	UINT m_vertexBufferSize = 0;
	D3D12_VERTEX_BUFFER_VIEW		m_vertexBufferView{};
	ID3D12Resource *				m_vertexBuffer		= nullptr;
	ID3D12Resource *				m_vertexHeapBuffer	= nullptr;
	std::vector<StaticVertex>	m_staticMesh;

	RenderingManager * m_renderingManager = nullptr;

	X12Fence * m_fence = nullptr;

	std::string m_name;
};

