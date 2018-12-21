#include "DirectX12EnginePCH.h"
#include "DeltaTime.h"

DeltaTime::DeltaTime()
{
}


DeltaTime::~DeltaTime()
{
}

void DeltaTime::Init()
{
	using namespace std::chrono;
	preTime = steady_clock::now();
	currentTime = steady_clock::now();
}

double DeltaTime::GetDeltaTimeInSeconds()
{
	using namespace std::chrono;

	currentTime = steady_clock::now();
	const double dt = duration_cast<nanoseconds>(currentTime - preTime).count() / 1000000000.0f;
	preTime = currentTime;

	return dt;
}
