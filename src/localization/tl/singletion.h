#ifndef SHARED_TL_SINGLETON_H
#define	SHARED_TL_SINGLETON_H

#include <memory>

template<class T>
class CSingleton
{
protected:
	static std::unique_ptr<T> m_pSingleton;

public:
	virtual ~CSingleton() {}

	static T* Get() 
	{
		if(!m_pSingleton.get())
			m_pSingleton.reset(new T());
		return m_pSingleton.get();
	}

	static void Delete()
	{
		if(m_pSingleton.get())
		{
			T* pSingletion = m_pSingleton.release();
			delete pSingletion;
		}
	}
};

template<class T>
std::unique_ptr<T> CSingleton<T>::m_pSingleton = 0;

#endif