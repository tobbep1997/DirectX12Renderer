#pragma once
#include <string>

namespace Converter
{
	using namespace System::Runtime::InteropServices;
	static const char* SystemStringToCharArr(System::String^ string)
	{
		const char* str = static_cast<const char*>((Marshal::StringToHGlobalAnsi(string)).ToPointer());
		return str;
	}

	static System::String^ StdStringToSystemString(std::string string)
	{
		return gcnew System::String(string.c_str());
	}
}