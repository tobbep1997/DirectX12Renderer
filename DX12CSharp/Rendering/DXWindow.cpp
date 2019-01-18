#include "DXWindow.h"

namespace ID3D12
{
#include "../Converter.h"
	   
	DXWindow::DXWindow()
		: IManagedObject(Window::GetInstance(), false)
	{

	}

	void DXWindow::Create(void* hInstance, String ^ windowName, unsigned width, unsigned height, bool fullScreen)
	{
		p_instance->Create(hInstance, Converter::SystemStringToCharArr(windowName), width, height, fullScreen);
	}

	bool DXWindow::Updating()
	{
		return p_instance->Updating();
	}

}
