#pragma once
#include "../Template/IObject.h"


struct aiScene;

class StaticMesh : //NOLINT
	public IObject
{
public:
	void Init() override;
	void Update() override;
	void Release() override;
	StaticMesh();
	~StaticMesh();

	void SetMesh(const std::vector<StaticVertex *> & mesh);
	void LoadStaticMesh(const std::string & path);

	BOOL CreateBuffer(RenderingManager * renderingManager);
	
	const std::vector<StaticVertex *> & GetStaticMesh() const;
private:

	
	HRESULT _createBuffer(RenderingManager * renderingManager);

	void _clearMesh();
	void _createMesh(const aiScene * scene);

	UINT m_vertexBufferSize = 0;
	D3D12_VERTEX_BUFFER_VIEW		m_vertexBufferView;
	ID3D12Resource *				m_vertexBuffer		= nullptr;
	ID3D12Resource *				m_vertexHeapBuffer	= nullptr;
	std::vector<StaticVertex *>	m_staticMesh;
};

