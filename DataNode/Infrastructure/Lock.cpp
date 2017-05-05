//------------------------------------------------------------------------------------------------------------------------------
#include "Lock.h"
#include <exception>
#ifdef LINUXCODE
#include <pthread.h>
#endif


#pragma warning(disable:4018)
#pragma warning(disable:4800)


CriticalObject::CriticalObject()
{
	#ifndef LINUXCODE
		::InitializeCriticalSection(&m_stSection);
	#else	
		/*
		 *	GUOGUO 2009-08-11 设置临界区锁的属性为可递归锁,支持同一线程连续调用lock
		 */
		//	ADD
		pthread_mutexattr_t attr;

		/*
		 *	2010-11-19在x86_64机器上运行发现的一个程序的BUG
		 *		当时的情况是,无法递归锁. 怎么也没想到是因为这个构造函数里面的
		 *		pthread_mutex_init 出现95错误, 导致锁不是递归的.
		 *		而导致pthread_mutex_init失败的原因又是因为attr没有使用pthread_mutexattr_init初始化.
		 *		似乎在 32位的RedHat AS4 Update6的机器上,正常运行..
		 *		总结了下原因,是因为不同的系统,对新的加载的内存页没有 填充 zero 所以,无论怎么样,
		 *		我们不能依赖系统为我们做工作,而应该我们主动的调用pthread_mutexattr_init做完这个初始化工作
		 *		这样才是最安全的
		 *
		 *		https://bugzilla.redhat.com/show_bug.cgi?id=204863
		 *															---		GUOGUO 2010-11-10
		 */
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		//	ADD END

		pthread_mutex_init(&m_stSection,&attr);

		//	ADD
		pthread_mutexattr_destroy(&attr);
		//	ADD END
	#endif
}
//..............................................................................................................................
CriticalObject::~CriticalObject()
{
	#ifndef LINUXCODE
		::DeleteCriticalSection(&m_stSection);
	#else
		pthread_mutex_destroy(&m_stSection);
	#endif
}
//..............................................................................................................................
bool CriticalObject::Lock(void)
{
	#ifndef LINUXCODE
		::EnterCriticalSection(&m_stSection);
	#else
		if(0 > pthread_mutex_lock(&m_stSection))
			return false;
	#endif

	return true;
}

void CriticalObject::UnLock(void)
{
	#ifndef LINUXCODE
		::LeaveCriticalSection(&m_stSection);
	#else
		pthread_mutex_unlock(&m_stSection);
	#endif
}

bool CriticalObject::TryLock(void)
{
	bool blRet = false;
	#ifndef LINUXCODE
		blRet = ::TryEnterCriticalSection(&m_stSection);
	#else
		if(0 == pthread_mutex_trylock(&m_stSection))
			blRet = true;
	#endif
	return blRet;
}


CriticalLock::CriticalLock( CriticalObject& refLock )
 : m_refLock( refLock )
{
	m_refLock.Lock();
}

CriticalLock::~CriticalLock()
{
	m_refLock.UnLock();
}







