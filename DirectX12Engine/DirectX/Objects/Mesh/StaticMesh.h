#pragma once
#include "../Template/IObject.h"


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

	BOOL CreateBuffer(RenderingManager * renderingManager);
	
	const std::vector<StaticVertex> & GetStaticMesh() const;

	const D3D12_VERTEX_BUFFER_VIEW & GetVertexBufferView() const;

private:
	HRESULT _createBuffer(RenderingManager * renderingManager);
	void _clearMesh();
	BOOL _createMesh(const aiScene * scene);

	BOOL m_meshLoaded = FALSE;
	UINT m_vertexBufferSize = 0;
	D3D12_VERTEX_BUFFER_VIEW		m_vertexBufferView{};
	ID3D12Resource *				m_vertexBuffer		= nullptr;
	ID3D12Resource *				m_vertexHeapBuffer	= nullptr;
	std::vector<StaticVertex>	m_staticMesh;

	std::string m_name;
};

