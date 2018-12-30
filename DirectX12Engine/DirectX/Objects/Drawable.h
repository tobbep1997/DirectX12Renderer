#pragma once
#include "Transform.h"
#include "Mesh/StaticMesh.h"

class Texture;
class IRender;

class Drawable :
	public Transform
{	
public:
	Drawable();
	~Drawable();

	void Init() override;
	void Update() override;
	void Release() override;
	
	void SetMesh(StaticMesh & mesh);

	const StaticMesh & GetMesh() const;

	void Draw(RenderingManager * renderingManager);

	void SetIsVisible(const BOOL & visible);
	const BOOL & GetIsVisible() const;

	void SetTexture(Texture * texture);
	void SetNormalMap(Texture * normal);
	void SetMetallicMap(Texture * metallic);
	
	const Texture * GetTexture() const;
	const Texture * GetNormal() const;
	const Texture * GetMetallic() const;

private:	
	StaticMesh * m_mesh;
	Texture * m_texture;
	Texture * m_normal;
	Texture * m_metallic;
	BOOL m_isVisible = TRUE;

};

