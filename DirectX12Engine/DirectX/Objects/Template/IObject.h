#pragma once

class IObject //NOLINT
{
protected:
	IObject() = default;
public:
	virtual ~IObject() = default;

	virtual BOOL Init() = 0;
	virtual void Update() = 0;
	virtual void Release() = 0;

};