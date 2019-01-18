#pragma once
#include "../IManagedObject.h"
#include <d3d12.h>
#include <dxgi1_4.h>

namespace ID3D12
{
	namespace DXWrap
	{
		public ref struct DX12Device : IManagedObject<ID3D12Device>
		{
			DX12Device(ID3D12Device * object) : IManagedObject(object, false){}
		};

		public ref struct DXSwapChain3 : IManagedObject<IDXGISwapChain3>
		{
			DXSwapChain3(IDXGISwapChain3 * object) : IManagedObject(object, false) {}
		};

		public ref struct DXGraphicsCommandList : IManagedObject<ID3D12GraphicsCommandList>
		{
			DXGraphicsCommandList(ID3D12GraphicsCommandList * object) : IManagedObject(object, false) {}
		};

		public ref struct DXUnknown : IManagedObject<IUnknown>
		{
			DXUnknown(IUnknown * object) : IManagedObject(object, false){}
		};
	}
}