#pragma once
#include "Transform.h"
#include "Mesh/StaticMesh.h"

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

	void Draw(IRender * iRender);



private:	
	StaticMesh * m_mesh;

};

