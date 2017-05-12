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
		 *	GUOGUO 2009-08-11 �����ٽ�����������Ϊ�ɵݹ���,֧��ͬһ�߳���������lock
		 */
		//	ADD
		pthread_mutexattr_t attr;

		/*
		 *	2010-11-19��x86_64���������з��ֵ�һ�������BUG
		 *		��ʱ�������,�޷��ݹ���. ��ôҲû�뵽����Ϊ������캯�������
		 *		pthread_mutex_init ����95����, ���������ǵݹ��.
		 *		������pthread_mutex_initʧ�ܵ�ԭ��������Ϊattrû��ʹ��pthread_mutexattr_init��ʼ��.
		 *		�ƺ��� 32λ��RedHat AS4 Update6�Ļ�����,��������..
		 *		�ܽ�����ԭ��,����Ϊ��ͬ��ϵͳ,���µļ��ص��ڴ�ҳû�� ��� zero ����,������ô��,
		 *		���ǲ�������ϵͳΪ����������,��Ӧ�����������ĵ���pthread_mutexattr_init���������ʼ������
		 *		���������ȫ��
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
			throw std::exception("<MWaitEvent>�����ȴ��¼���������");
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
			//	�����ж�,�Ա㰲ȫ���л���,������ܵ��»��ѵĴ�λ
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
			throw std::exception("<MWaitEvent>�ȴ��¼���������");
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
		//	1s=1000000000����
		//	1s=1000000	 ΢��
		//	1s=1000		 ����
		//
		future = tp.millitm  + lMSec % 1000;	//	δ��lMsec�ĺ���ֵ
		future = future * 1000000;				//	���������

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
			//	�õ�����,�����ٴ��ж�m_nCount
			//	��Ϊ 2�ֿ���.
			//	1:timewait�����Ǳ�UNIX ��signal����
			//	2:pthread_cond_signal��������
			rc = pthread_cond_timedwait(&m_hRecordData,&m_hMutex,&abstime);
			if(rc != 0)
			{
				if(rc == ETIMEDOUT)
				{
#if  __GCC_VER__ >= 40201
#else
					m_lInWaitCount--;					
#endif

					//	��ʱ����
					pthread_mutex_unlock(&m_hMutex);

#if  __GCC_VER__ >= 40201
					__sync_fetch_and_sub (&m_lInWaitCount, 1);
#endif

					return;
				}
				else if(rc == EINTR)
				{
					//	������signal �ж��ź�
					//	��ʵӦ����������sttemp��,����,�����.
					continue;
				}
				else
				{
					//	��������
					//pthread_mutex_unlock(&m_hMutex);
					//return;
					continue;
				}
			}
			else
			{
				//	��������
				continue;	//	�ǳ������pthread_cond_timewait==0��ʱ��,
							//	�ƺ�����û�б�pthread_cond_signal����,Ҳ���Ǳ��ж��źŻ���
							//	��̫�������ʲô����ᵼ�����������鷢��
							//	Ϊ�˰�ȫ�����׼������������������m_nCount��ֵ.
			}
		}
		if(m_nCount <= 0)
		{
			//	���Բ����ܷ����Ĵ���,ǰ���Ѿ���������Ŭ��ȥ����m_nCount��ֵ
			//	��������ߵ�����,��ôֻ����ȥ��ѭ����.ϵͳ�������Ѿ��൱������
			//	Ҳ����Կ�������m_nCount = 0,�����Ҳ�����������,
			//	�Ѿ�����������,Ϊ�˱��������һ������,����Ϊ��Ӧ��ȥ��ѭ��.Ȼ��ĵȴ�coderȥ����������
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
			throw exception("<MWaitEvent>�ȴ��¼���������");
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













