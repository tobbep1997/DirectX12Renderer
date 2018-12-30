#include "DirectX12EnginePCH.h"
#include "Drawable.h"
#include "DirectX/Render/Template/IRender.h"


Drawable::Drawable()
{
	m_mesh = nullptr;
	m_texture = nullptr;
	m_normal = nullptr;
}


Drawable::~Drawable()
{
}

void Drawable::Init()
{
}

void Drawable::Update()
{
	Transform::Update();
}

void Drawable::Release()
{
}

void Drawable::SetMesh(StaticMesh& mesh)
{
	this->m_mesh = &mesh;
}

const StaticMesh& Drawable::GetMesh() const
{
	return *this->m_mesh;
}

void Drawable::Draw(RenderingManager * renderingManager)
{
	if (this->m_isVisible)
		reinterpret_cast<IRender*>(renderingManager->GetGeometryPass())->Queue(this);
}

void Drawable::SetIsVisible(const BOOL& visible)
{
	this->m_isVisible = visible;
}

const BOOL& Drawable::GetIsVisible() const
{
	return this->m_isVisible;
}

void Drawable::SetTexture(Texture* texture)
{
	this->m_texture = texture;
}

void Drawable::SetNormalMap(Texture* normal)
{
	this->m_normal = normal;
}

void Drawable::SetMetallicMap(Texture* metallic)
{
	this->m_metallic = metallic;
}

void Drawable::SetDisplacementMap(Texture* displacement)
{
	this->m_displacement = displacement;
}

const Texture* Drawable::GetDisplacement() const
{
	return this->m_displacement;
}

const Texture* Drawable::GetMetallic() const
{
	return this->m_metallic;
}

const Texture* Drawable::GetTexture() const
{
	return this->m_texture;
}

const Texture* Drawable::GetNormal() const
{
	return this->m_normal;
}

