#pragma once
#include "../IManagedObject.h"
#include <DirectXMath.h>

namespace DXMath
{
	
	public ref class DXVector : ID3D12::IManagedObject < DirectX::XMFLOAT4 >
	{
	public:
		DXVector();
		DXVector(float x, float y, float z, float w);
		DXVector(const DirectX::XMFLOAT4 & xmfloat4);

		property float x
		{
		public:
			float get()
			{
				return p_instance->x;
			}
			void set(float value)
			{
				p_instance->x = value;
			}
		}

		property float y
		{
		public:
			float get()
			{
				return p_instance->y;
			}
			void set(float value)
			{
				p_instance->y = value;
			}
		}
		property float z
		{
		public:
			float get()
			{
				return p_instance->z;
			}
			void set(float value)
			{
				p_instance->z = value;
			}
		}
		property float w
		{
		public:
			float get()
			{
				return p_instance->w;
			}
			void set(float value)
			{
				p_instance->w = value;
			}
		}

	};

}
