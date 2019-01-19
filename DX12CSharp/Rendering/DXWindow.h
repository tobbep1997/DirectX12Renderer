#pragma once
#include "../IManagedObject.h"
#include <Window/Window.h>

namespace ID3D12
{
	
	using namespace System;
	public ref class DXWindow :
		public ID3D12::IManagedObject<Window>
	{
	private:



	public:
		DXWindow();

		void Create(void* hInstance,
			String ^ windowName,
			unsigned int width,
			unsigned int height,
			bool fullScreen);
	
		bool Updating();

		void Release();

		property unsigned int Width
		{
		public:
			unsigned int get()
			{
				return p_instance->GetWidth();
			}
		}
		
		property unsigned int Height
		{
		public:
			unsigned int get()
			{
				return p_instance->GetHeight();
			}
		}

		property bool Fullscreen
		{
		public:
			bool get()
			{
				return p_instance->GetFullscreen();
			}
		}

		property bool IsOpen
		{
		public:
			bool get()
			{
				return p_instance->IsOpen();
			}
		}

	};
	
}
