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
	const Texture * GetTexture() const;

private:	
	StaticMesh * m_mesh;
	Texture * m_texture;
	BOOL m_isVisible = TRUE;

};

