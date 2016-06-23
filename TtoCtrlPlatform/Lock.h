
#ifndef _LOCK_H_
#define _LOCK_H_

//Mutex m_Mutex;
//Guard<Mutex> __guard(m_Mutex);
#include <windows.h>
class Mutex
{
private:
	CRITICAL_SECTION section;
	
public:
	Mutex()
	{
		InitializeCriticalSection(&section);
	}
	
	~Mutex()
	{
		DeleteCriticalSection(&section);
	}
	
	void enter()
	{
		EnterCriticalSection(&section);
	}
	
	void leave()
	{
		LeaveCriticalSection(&section);
	}
};

class Mutex2
{
private:
	HANDLE m_hMutex;
public:
	bool isBe; 
	Mutex2( const char* lpName )
	{
		isBe = false;
		m_hMutex = CreateMutex(NULL, FALSE, lpName);
		if ( GetLastError() == ERROR_ALREADY_EXISTS )	isBe = true;
	}

	~Mutex2()
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL; 
	}

	void enter()
	{
		WaitForSingleObject(m_hMutex, DWORD(-1));
	}

	void leave()
	{
		ReleaseMutex(m_hMutex);
	}
};

template<typename T>
class Guard
{
private:
	T& t_;
public:
	Guard(T& t) : t_(t)
	{
		t_.enter();
	}
	~Guard()
	{
		t_.leave();
	}
};
#endif