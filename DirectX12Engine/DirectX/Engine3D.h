#pragma once

class RenderingManager;

class Engine3D
{
private:

public:
	Engine3D();
	~Engine3D();

	void InitDirectX();

	void Flush();
	void Present();
};

