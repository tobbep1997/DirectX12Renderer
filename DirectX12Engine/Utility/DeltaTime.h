#pragma once
class DeltaTime
{
private:
	std::chrono::time_point <std::chrono::steady_clock> preTime;
	std::chrono::time_point <std::chrono::steady_clock> currentTime;
public:
	DeltaTime();
	~DeltaTime();
	void Init();
	double GetDeltaTimeInSeconds();
};

