#pragma once
#include "DirectX/Objects/Drawable.h"
#include "DXTransform.h"
#include "DXStaticMesh.h"
#include "../DXTexture.h"

namespace ID3D12
{

	public ref class DXDrawable : DXTransform
	{
	public:
		DXDrawable();
		void SetStaticMesh(DXStaticMesh^ staticMesh);

		void Draw(DXRenderingManager^ renderingManager);

		void SetTexture(DXTexture^ texture);
		void SetNormalMap(DXTexture^ normal);
		void SetMetallicMap(DXTexture^ metallic);
		void SetDisplacementMap(DXTexture^ displacement);
		

		property bool IsVisible
		{
		public:
			bool get()
			{
				return GetInstance<Drawable>()->GetIsVisible();
			}
			void set(bool value)
			{
				return GetInstance<Drawable>()->SetIsVisible(value);
			}
		}

		property bool CastShadows
		{
		public:
			bool get()
			{
				return GetInstance<Drawable>()->GetCastShadows();
			}
			void set(bool value)
			{
				return GetInstance<Drawable>()->SetCastShadows(value);
			}
		}
	
	};

}
