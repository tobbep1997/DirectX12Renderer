#pragma once

namespace ID3D12
{
	template<class T>
	public ref class IManagedObject
	{
	private:
		bool m_isPointer;
	protected:
		T* p_instance;
	public:
		IManagedObject(T* instance, bool isPointer)
			: p_instance(instance), m_isPointer(isPointer)
		{
		}
		virtual ~IManagedObject()
		{
			if (p_instance != nullptr && m_isPointer)
			{
				delete p_instance;
			}
		}
		!IManagedObject()
		{
			if (p_instance != nullptr && m_isPointer)
			{
				delete p_instance;
			}
		}
		T* GetInstance()
		{
			return p_instance;
		}
	};	

}