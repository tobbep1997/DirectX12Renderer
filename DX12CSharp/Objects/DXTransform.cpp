#include "DXTransform.h"


namespace ID3D12
{	
	DXTransform::DXTransform(void * transform) : IManagedObject(transform, true)
	{
		m_transform = reinterpret_cast<Transform*>(transform);
	}

	void DXTransform::SetPosition(DXMath::DXVector^ position)
	{
		m_transform->SetPosition(*position->GetInstance());
	}

	void DXTransform::Translate(DXMath::DXVector^ position)
	{
		m_transform->Translate(*position->GetInstance());
	}

	void DXTransform::SetRotation(DXMath::DXVector^ rotation)
	{
		m_transform->SetRotation(*rotation->GetInstance());
	}

	void DXTransform::SetScale(DXMath::DXVector^ scale)
	{
		m_transform->SetScale(*scale->GetInstance());
	}

	DXMath::DXVector^ DXTransform::GetPosition()
	{
		return gcnew DXMath::DXVector(m_transform->GetPosition());
	}

	DXMath::DXVector^ DXTransform::GetRotation()
	{
		return gcnew DXMath::DXVector(m_transform->GetRotation());
	}

	DXMath::DXVector^ DXTransform::GetScale()
	{
		return gcnew DXMath::DXVector(m_transform->GetScale());
	}

	void DXTransform::Update()
	{
		m_transform->Update();
	}
}
