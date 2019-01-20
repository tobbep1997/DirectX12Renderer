#include "DXDirectionalLight.h"
#include "DirectX/Objects/Light/DirectionalLight.h"


namespace ID3D12
{
	DXDirectionalLight::DXDirectionalLight(DXRenderingManager^ renderingManager, DXWindow^ window) 
		: DXILight(new DirectionalLight(renderingManager->GetInstance(), *window->GetInstance()))
	{
	}

	bool DXDirectionalLight::Init()
	{
		return GetInstance<DirectionalLight>()->Init();
	}

	void DXDirectionalLight::Update()
	{
		GetInstance<DirectionalLight>()->Update();
	}

	void DXDirectionalLight::Release()
	{
		GetInstance<DirectionalLight>()->Release();
	}

	void DXDirectionalLight::SetDirection(DXMath::DXVector^ direction)
	{
		GetInstance<DirectionalLight>()->SetDirection(*direction->GetInstance());
	}

	void DXDirectionalLight::SetPosition(DXMath::DXVector^ position)
	{
		GetInstance<DirectionalLight>()->SetPosition(*position->GetInstance());
	}

}
