#include "DirectX12EnginePCH.h"
#include "Drawable.h"
#include "DirectX/Render/Template/IRender.h"


Drawable::Drawable()
{
	this->Init();
}


Drawable::~Drawable()
{
}

void Drawable::Init()
{
}

void Drawable::Update()
{
}

void Drawable::Release()
{
}

void Drawable::SetMesh(StaticMesh& mesh)
{
	this->m_mesh = &mesh;
}

void Drawable::Draw(IRender* iRender)
{
	iRender->Queue(this);
}

