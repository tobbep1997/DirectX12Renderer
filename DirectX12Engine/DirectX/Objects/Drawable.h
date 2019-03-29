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

	BOOL Init() override;
	void Update() override;
	void Release() override;
	
	void SetMesh(StaticMesh & mesh);

	const StaticMesh * GetMesh() const;

	void Draw();

	void SetIsVisible(const BOOL & visible);
	const BOOL & GetIsVisible() const;

	void SetCastShadows(const BOOL & castShadows);
	const BOOL & GetCastShadows() const;

	void SetTexture(Texture * texture);
	void SetNormalMap(Texture * normal);
	void SetMetallicMap(Texture * metallic);
	void SetDisplacementMap(Texture * displacement);

	const Texture * GetTexture() const;
	const Texture * GetNormal() const;
	const Texture * GetMetallic() const;
	const Texture * GetDisplacement() const;

	bool Instance(const Drawable & other) const;

private:	
	StaticMesh * m_mesh;
	Texture * m_texture;
	Texture * m_normal;
	Texture * m_metallic;
	Texture * m_displacement;

	BOOL m_isVisible = TRUE;
	BOOL m_castShadows = TRUE;

	RenderingManager * m_renderingManager = nullptr;
};

