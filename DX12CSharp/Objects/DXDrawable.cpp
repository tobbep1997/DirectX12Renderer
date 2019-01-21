#include "DXDrawable.h"


namespace ID3D12
{
	DXDrawable::DXDrawable()
		: DXTransform(new Drawable())
	{
	}


	void DXDrawable::SetStaticMesh(DXStaticMesh^ staticMesh)
	{
		GetInstance<Drawable>()->SetMesh(*staticMesh->GetInstance());
	}

	void DXDrawable::Draw(DXRenderingManager^ renderingManager)
	{
		GetInstance<Drawable>()->Draw(renderingManager->GetInstance());
	}

	void DXDrawable::SetTexture(DXTexture^ texture)
	{
		GetInstance<Drawable>()->SetTexture(texture->GetInstance());
	}

	void DXDrawable::SetNormalMap(DXTexture^ normal)
	{
		GetInstance<Drawable>()->SetNormalMap(normal->GetInstance());
	}

	void DXDrawable::SetMetallicMap(DXTexture^ metallic)
	{
		GetInstance<Drawable>()->SetMetallicMap(metallic->GetInstance());
	}

	void DXDrawable::SetDisplacementMap(DXTexture^ displacement)
	{
		GetInstance<Drawable>()->SetDisplacementMap(displacement->GetInstance());
	}
}
