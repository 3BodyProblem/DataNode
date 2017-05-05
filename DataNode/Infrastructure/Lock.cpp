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







