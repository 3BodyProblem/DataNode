#ifndef __INFRASTRUCTURE__LOCK_H__
#define __INFRASTRUCTURE__LOCK_H__


#include "../targetver.h"
#include <string>
#include "assert.h"
#include "WinBase.h"


class CriticalLock;


/**
 * @class			CriticalObject
 * @brief			�ٽ�������
 * @author			barry
 * @date			2017/4/2
 */
class CriticalObject
{
friend class CriticalLock;
public:
	CriticalObject();
	~CriticalObject();

protected:
	bool									Lock();
	void									UnLock();
	bool									TryLock();

protected:
	#ifndef LINUXCODE
		CRITICAL_SECTION					m_stSection;
	#else
		pthread_mutex_t						m_stSection;
	#endif
};


/**
 * @class			CriticalLock
 * @brief			�ٽ���������
 * @author			barry
 * @date			2017/4/2
 */
class CriticalLock
{
public:
	CriticalLock( CriticalObject& refLock );
	~CriticalLock();

protected:
	CriticalObject&							m_refLock;
};


/**
 * @class			WaitEvent
 * @brief			�ȴ��¼�����Ȼ�������ڶ�����̣���������¼����ƣ�
 * @note			��Ҫ������ͷǼ���������������sleep���������ܹ�������Ӧ��
 */
class WaitEvent
{
protected:
	volatile int							m_lInWaitCount;
	volatile	bool						m_blRunFlag;
	#ifndef LINUXCODE
		HANDLE								m_hRecordData;
	#else
		int									m_IsStartWait;
		int									m_nCount;	//	��ȫ����,����thread_signal�ջ���,�Լ�����thread_wait���źŻ��ѵĴ���
		pthread_cond_t						m_hRecordData;
		pthread_mutex_t						m_hMutex;
	#endif
public:
	WaitEvent();
	virtual ~WaitEvent();
public:
	//�����¼�
	void Active();
	//�ȴ��¼�
	void Wait( unsigned long lMSec = 0xFFFFFFFF );
	long GetInWaitCount() {return m_lInWaitCount;}
};



#endif






