#pragma once
#include "Template/IRender.h"
#include "WrapperFunctions/X12StructuredBuffer.h"

class ReflectionPass :
	public IRender
{
private:
	struct ScreenOffset
	{
		
	};

public:
	ReflectionPass(RenderingManager * renderingManager, const Window & window);
	~ReflectionPass();


	HRESULT Init() override;
	void Update(const Camera& camera, const float& deltaTime) override;
	void Draw() override;
	void Clear() override;
	void Release() override;
private:
	HRESULT _preInit();
	
	X12StructuredBuffer * m_screenOffset;

	UINT m_width = 0, m_widthSlice = 0;
	UINT m_height = 0, m_heightSlice = 0;

	

};

