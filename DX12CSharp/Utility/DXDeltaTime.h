#pragma once
#include "../IManagedObject.h"
#include "Utility/DeltaTime.h"

namespace ID3D12
{
	public ref class DXDeltaTime : public IManagedObject<DeltaTime>
	{
	public:
		DXDeltaTime();

		void Init();
		float GetDeltaTime();
	};

}
