#pragma once

class IObject //NOLINT
{
protected:
	IObject() = default;
public:
	virtual ~IObject() = default;

	virtual void Init() = 0;
	virtual void Release() = 0;
};