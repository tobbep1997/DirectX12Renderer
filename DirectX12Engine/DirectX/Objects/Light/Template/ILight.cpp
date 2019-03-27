#include "DirectX12EnginePCH.h"
#include "ILight.h"
#include "DirectX/Render/Template/IRender.h"
#include "DirectX/Render/WrapperFunctions/X12RenderTargetView.h"
#include "DirectX/Render/WrapperFunctions/X12DepthStencil.h"

#pragma warning (disable : 4172)

ILight::ILight(RenderingManager* renderingManager, const Window& window, const LightType& lightType)
{
	p_renderingManager = renderingManager;
	p_window = &window;
	   
	this->p_lightType = lightType;

	this->m_lightTypeInteger = lightType;

	this->m_intensity = 1;
	this->m_color = DirectX::XMFLOAT4(1, 1, 1, 1);
}

BOOL ILight::p_createDirectXContext(const UINT& renderTargets, const BOOL& createTexture)
{
	HRESULT hr = 0;
	if (p_renderTarget || p_depthStencil)
		throw "Shit";

	p_renderTargets = renderTargets;
	SAFE_NEW(p_depthStencil, new X12DepthStencil());
	SAFE_NEW(p_renderTarget, new X12RenderTargetView());

	if (SUCCEEDED(hr = p_renderingManager->OpenCommandList()))
	{
		if (SUCCEEDED(hr = p_renderTarget->CreateRenderTarget(
			SHADOW_MAP_SIZE,
			SHADOW_MAP_SIZE,
			renderTargets,
			createTexture)))
		{
			if (SUCCEEDED(hr = p_depthStencil->CreateDepthStencil(
				L"Directional Light",
				SHADOW_MAP_SIZE,
				SHADOW_MAP_SIZE,
				renderTargets,
				createTexture)))
			{
				if (SUCCEEDED(hr = p_renderingManager->SignalGPU()))
				{

				}
			}
		}
	}

	return FALSE;
}

BOOL ILight::Init()
{
	return Transform::Init();
}

void ILight::Update()
{
	Transform::Update();
}

void ILight::Release()
{
	Transform::Release();
	p_renderTarget->Release();
	p_depthStencil->Release();
}


ILight::~ILight()
{
	SAFE_DELETE(p_renderTarget);
	SAFE_DELETE(p_depthStencil);
}

void ILight::Queue()
{
	if (m_intensity > 0)
	{
		IRender * deferredPass = reinterpret_cast<IRender*>(p_renderingManager->GetDeferredRender());
		IRender * shadowPass = reinterpret_cast<IRender*>(p_renderingManager->GetShadowPass());

		if (deferredPass)
			deferredPass->QueueLight(this);
		if (shadowPass)
			shadowPass->QueueLight(this);
	}
}

void ILight::SetIntensity(const float& intensity)
{
	this->m_intensity = intensity;
}

const float& ILight::GetIntensity() const
{
	return this->m_intensity;
}

void ILight::SetColor(const DirectX::XMFLOAT4& color)
{
	this->m_color = color;
}

void ILight::SetColor(const float& x, const float& y, const float& z, const float& w)
{
	this->SetColor(DirectX::XMFLOAT4(x, y, z, w));
}

const DirectX::XMFLOAT4 & ILight::GetColor() const
{
	return this->m_color;
}

void ILight::SetCastShadows(const BOOL& castShadows)
{
	this->m_castShadows = castShadows;
}

const BOOL& ILight::GetCastShadows() const
{
	return m_castShadows;
}

const UINT& ILight::GetType() const
{
	return m_lightTypeInteger;
}

const UINT& ILight::GetNumRenderTargets() const
{
	return p_renderTargets;
}

X12DepthStencil* ILight::GetDepthStencil() const
{
	return p_depthStencil;
}

X12RenderTargetView* ILight::GetRenderTargetView() const
{
	return p_renderTarget;
}





