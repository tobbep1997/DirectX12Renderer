#include "DXDeltaTime.h"


namespace ID3D12
{
	DXDeltaTime::DXDeltaTime()
		: IManagedObject(new DeltaTime(), true)
	{
	}

	void DXDeltaTime::Init()
	{
		p_instance->Init();
	}

	float DXDeltaTime::GetDeltaTime()
	{
		return static_cast<float>(p_instance->GetDeltaTimeInSeconds());
	}
}
