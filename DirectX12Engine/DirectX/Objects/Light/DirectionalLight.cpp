#include "DirectX12EnginePCH.h"
#include "DirectionalLight.h"
#include "DirectX/Render/WrapperFunctions/X12RenderTargetView.h"
#include "DirectX/Render/WrapperFunctions/X12DepthStencil.h"


DirectionalLight::DirectionalLight(RenderingManager* renderingManager, const Window& window)
	: ILight(renderingManager, window)
{
	p_lightType = 1;
	m_camera = new Camera(DirectX::XM_PI * 0.5, 1.0f, 1, 100.0f, FALSE);

	p_renderTarget = new X12RenderTargetView(renderingManager, window);
	p_depthStencil = new X12DepthStencil(renderingManager, window);
}

DirectionalLight::~DirectionalLight()
{
	delete m_camera;
	m_camera = nullptr;

	delete p_renderTarget;
	delete p_depthStencil;
}

void DirectionalLight::Init()
{
	m_camera->Init();
	if (SUCCEEDED(_createDirectXContent()))
	{
		PRINT("Successfully created Directional Light") NEW_LINE;
	}	
}

void DirectionalLight::Update()
{
	m_camera->Update();
}

void DirectionalLight::Release()
{
	m_camera->Release();
	p_renderTarget->Release();
	p_depthStencil->Release();
}

const UINT& DirectionalLight::GetNumRenderTargets() const
{
	return this->p_renderTargets;
}

Camera* DirectionalLight::GetCamera()
{
	return this->m_camera;
}

void DirectionalLight::SetDirection(const DirectX::XMFLOAT4& direction)
{
	m_camera->SetDirection(direction);
}

void DirectionalLight::SetDirection(const float& x, const float& y, const float& z, const float& w)
{
	this->SetDirection(DirectX::XMFLOAT4(x, y, z, w));
}

void DirectionalLight::SetPosition(const DirectX::XMFLOAT4& position)
{
	Transform::SetPosition(position);
	this->m_camera->SetPosition(position);
}

void DirectionalLight::SetPosition(const float& x, const float& y, const float& z, const float& w)
{
	SetPosition(DirectX::XMFLOAT4(x, y, z, w));
}

const UINT& DirectionalLight::GetType() const
{
	return p_lightType;
}

HRESULT DirectionalLight::_createDirectXContent()
{
	HRESULT hr = 0;
	if (SUCCEEDED(hr = p_renderingManager->OpenCommandList()))
	{
		if (SUCCEEDED(hr = p_renderTarget->CreateRenderTarget(SHADOW_MAP_SIZE,
			SHADOW_MAP_SIZE,
			1)))
		{
			if (SUCCEEDED(hr = p_depthStencil->CreateDepthStencil(
				L"Directional Light",
				SHADOW_MAP_SIZE,
				SHADOW_MAP_SIZE,
				1)))
			{
				if (SUCCEEDED(hr = p_renderingManager->SignalGPU()))
				{
					
				}
			}
		}
	}
	return hr;
}


