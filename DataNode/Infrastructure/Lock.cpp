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


WaitEvent::WaitEvent()
{
	m_lInWaitCount = 0;
	m_blRunFlag = true;
	#ifndef LINUXCODE
		if ( (m_hRecordData = ::CreateEvent(NULL,FALSE,FALSE,NULL)) == NULL )
		{
			assert(0);
			throw std::exception("<MWaitEvent>创建等待事件发生错误");
		}
	#else
		m_nCount	 = 0;
		m_IsStartWait = 0;
		pthread_mutex_init(&m_hMutex, NULL);
		pthread_cond_init(&m_hRecordData,NULL);
	#endif
}

WaitEvent::~WaitEvent()
{
	m_blRunFlag = false;
	if(0 < m_lInWaitCount)
	{
#if defined(WIN32) || defined(MS_WINDOWS)
		Sleep(0);
		Sleep(0);
		Sleep(0);
#else
		sched_yield();
		sched_yield();
		sched_yield();
#endif
		
	}
	#ifndef LINUXCODE
		if ( m_hRecordData != NULL )
		{
			::CloseHandle(m_hRecordData);
			m_hRecordData = NULL;
		}
	#else
		pthread_mutex_destroy(&m_hMutex);
		pthread_cond_destroy(&m_hRecordData);
		m_IsStartWait = 0;
		m_nCount = 0;
	#endif
}

void WaitEvent::Active()
{
	if( !m_blRunFlag	)
		return;
	#ifndef LINUXCODE
		::SetEvent(m_hRecordData);
	#else
		pthread_mutex_lock(&m_hMutex);
		if(m_nCount == 0 && m_IsStartWait > 0)
		{
			//	进行判断,以便安全进行唤醒,否则可能导致唤醒的错位
			pthread_cond_signal(&m_hRecordData);
		}
		m_nCount++;
		pthread_mutex_unlock(&m_hMutex);
	#endif
}

void WaitEvent::Wait( unsigned long lMSec )
{
	if(!m_blRunFlag	)	{
		return;
	}

	#ifndef LINUXCODE
		register int				errorcode;
		InterlockedIncrement((PLONG)&m_lInWaitCount);
		errorcode = ::WaitForSingleObject(m_hRecordData,lMSec);
		InterlockedDecrement((PLONG)&m_lInWaitCount);
		if ( errorcode == WAIT_FAILED )
		{
			assert(0);
			throw std::exception("<MWaitEvent>等待事件发生错误");
		}
	#else
		int							rc, future;
		struct timespec				abstime;	//GUOGUO 20090723
		struct timeb				tp;

#if  __GCC_VER__ >= 40201
		__sync_fetch_and_add (&m_lInWaitCount, 1);		
#else
		pthread_mutex_lock(&m_hMutex);
		m_lInWaitCount++;
		pthread_mutex_unlock(&m_hMutex);

#endif
		ftime(&tp);
		//	Note:
		//	1s=1000000000纳秒
		//	1s=1000000	 微秒
		//	1s=1000		 毫秒
		//
		future = tp.millitm  + lMSec % 1000;	//	未来lMsec的毫秒值
		future = future * 1000000;				//	换算成纳秒

		if(future >= 1000000000)
		{
			abstime.tv_nsec = future % 1000000000;
			abstime.tv_sec = tp.time + lMSec / 1000 + future / 1000000000;
		}
		else
		{
			abstime.tv_nsec = future;
			abstime.tv_sec = tp.time + lMSec / 1000;
		}


		//sttemp = lMSec;
		pthread_mutex_lock(&m_hMutex);
		m_IsStartWait = 1;
		while(m_nCount == 0 && m_blRunFlag)
		{
			//	得到锁后,必须再次判断m_nCount
			//	因为 2种可能.
			//	1:timewait可能是被UNIX 的signal唤醒
			//	2:pthread_cond_signal不唤醒它
			rc = pthread_cond_timedwait(&m_hRecordData,&m_hMutex,&abstime);
			if(rc != 0)
			{
				if(rc == ETIMEDOUT)
				{
#if  __GCC_VER__ >= 40201
#else
					m_lInWaitCount--;					
#endif

					//	超时错误
					pthread_mutex_unlock(&m_hMutex);

#if  __GCC_VER__ >= 40201
					__sync_fetch_and_sub (&m_lInWaitCount, 1);
#endif

					return;
				}
				else if(rc == EINTR)
				{
					//	发生了signal 中断信号
					//	其实应该重新设置sttemp的,算了,简单起见.
					continue;
				}
				else
				{
					//	其它错误
					//pthread_mutex_unlock(&m_hMutex);
					//return;
					continue;
				}
			}
			else
			{
				//	被唤醒了
				continue;	//	非常不理解pthread_cond_timewait==0的时候,
							//	似乎好象并没有被pthread_cond_signal唤醒,也不是被中断信号唤醒
							//	不太清楚还有什么情况会导致这样的事情发生
							//	为了安全起见我准备继续测试条件变量m_nCount的值.
			}
		}
		if(m_nCount <= 0)
		{
			//	绝对不可能发生的错误,前面已经尽了最大的努力去控制m_nCount的值
			//	如果还能走到这里,那么只能是去死循环了.系统的问题已经相当的严重
			//	也许可以考虑修正m_nCount = 0,但是我不建议这样做,
			//	已经是有问题了,为了避免问题进一步扩大,我认为它应该去死循环.然后的等待coder去解决这个问题
			//
			//						--	2010 GUOGUO
#ifdef _DEBUG
			while(1)
			{
				printf("rc=%d, m_nCount=%d, m_IsStartWait=%d, errno = %d\n", rc, m_nCount, m_IsStartWait, errno);
				MThread::Sleep(1);
			}
#else
			usleep(1000);
			char	szException[256];
			memset(szException, 0, sizeof(szException));
			FILE	*fp;
			fp = fopen("/tmp/MMutex.log", "a+");
			if(fp)
			{
				fprintf(fp, "time=%d, rc=%d, m_nCount=%d, m_IsStartWait=%d, errno = %d", time(NULL), rc, m_nCount, m_IsStartWait, errno);
				fclose(fp);
			}
			pthread_mutex_unlock(&m_hMutex);
			throw exception("<MWaitEvent>等待事件发生错误");
#endif
		}
		m_nCount--;

#if  __GCC_VER__ >= 40201
#else
		m_lInWaitCount--;					
#endif

		pthread_mutex_unlock(&m_hMutex);

#if  __GCC_VER__ >= 40201
		__sync_fetch_and_sub (&m_lInWaitCount, 1);
#endif

	#endif
}













