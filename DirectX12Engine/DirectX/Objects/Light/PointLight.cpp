#include "DirectX12EnginePCH.h"
#include "PointLight.h"
#include "DirectX/Render/GeometryPass.h"
#include "DirectX/Render/WrapperFunctions/X12DepthStencil.h"
#include "DirectX/Render/WrapperFunctions/X12RenderTargetView.h"

PointLight::PointLight(RenderingManager* renderingManager, const Window& window)
	: ILight(renderingManager, window)
{
	p_lightType = 0U;

	m_dropOff = 1.0f;
	m_pow = 2.0f;

	this->p_renderTargets = 6u;

	for (UINT i = 0; i < 6; i++)
	{
		m_cameras[i] = new Camera(DirectX::XM_PI * 0.5f, 1.0f, 0.1f, 50.0f);
	}
	//UP
	m_cameras[0]->SetDirection(0, 1, 0);
	m_cameras[0]->SetUp(1, 0, 0);
	//DOWN
	m_cameras[1]->SetDirection(0, -1, 0);
	m_cameras[1]->SetUp(1, 0, 0);
	//RIGHT
	m_cameras[2]->SetDirection(1, 0, 0);
	//LEFT
	m_cameras[3]->SetDirection(-1, 0, 0);
	//FORWARD
	m_cameras[4]->SetDirection(0, 0, 1);
	//BACKWARDS
	m_cameras[5]->SetDirection(0, 0, -1);

	for (UINT i = 0; i < 6; i++)
	{
		m_cameras[i]->Update();
	}


	p_renderTarget = new X12RenderTargetView(renderingManager, window);
	p_depthStencil = new X12DepthStencil(renderingManager, window);
	
}

PointLight::~PointLight()
{
	for (UINT i = 0; i < 6u; i++)
	{
		m_cameras[i]->Release();
		delete m_cameras[i];
	}


	delete p_renderTarget;
	delete p_depthStencil;
	
}

void PointLight::SetDropOff(const float& dropOff)
{
	this->m_dropOff = dropOff;
}

void PointLight::SetPow(const float& pow)
{
	this->m_pow = pow;
}

const float& PointLight::GetDropOff() const
{
	return this->m_dropOff;
}

const float& PointLight::GetPow() const
{
	return this->m_pow;
}

const UINT & PointLight::GetType() const
{
	return p_lightType;
}

const UINT& PointLight::GetNumRenderTargets() const
{
	return this->p_renderTargets;
}

const Camera* const* PointLight::GetCameras() const
{
	return this->m_cameras;
}

void PointLight::Init()
{
	HRESULT hr = 0;

	if (SUCCEEDED(hr = p_renderingManager->OpenCommandList()))
	{		

		if (SUCCEEDED(hr = p_depthStencil->CreateDepthStencil(L"PointLight",
			SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
			p_renderTargets)))
		{
			if (SUCCEEDED(hr = p_renderTarget->CreateRenderTarget(
				SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
				p_renderTargets)))
			{		
				if (SUCCEEDED(hr = p_renderingManager->SignalGPU()))
				{
				}
			}			
		}
	}


	if (FAILED(hr))
	{
		Window::CreateError("FAILED to create pointLight");
	}

}

void PointLight::Update()
{
	Transform::Update();
	for (UINT i = 0; i < 6; i++)
	{
		m_cameras[i]->SetPosition(GetPosition());
		m_cameras[i]->Update();
	}
}

void PointLight::Release()
{
	p_renderTarget->Release();
	p_depthStencil->Release();	
}
