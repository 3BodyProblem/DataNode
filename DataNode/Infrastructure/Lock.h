#ifndef __INFRASTRUCTURE__LOCK_H__
#define __INFRASTRUCTURE__LOCK_H__


#include "../targetver.h"
#include <string>
#include "assert.h"
#include "WinBase.h"


class CriticalLock;


/**
 * @class			CriticalObject
 * @brief			临界区对象
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
 * @brief			临界区守卫者
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
 * @brief			等待事件，虽然可以用在多个进程（必须加入事件名称）
 * @note			主要处理激活和非激活，可以用它来替代sleep函数，它能够提早响应。
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
		int									m_nCount;	//	安全计数,避免thread_signal空唤醒,以及保持thread_wait被信号唤醒的处理
		pthread_cond_t						m_hRecordData;
		pthread_mutex_t						m_hMutex;
	#endif
public:
	WaitEvent();
	virtual ~WaitEvent();
public:
	//激活事件
	void Active();
	//等待事件
	void Wait( unsigned long lMSec = 0xFFFFFFFF );
	long GetInWaitCount() {return m_lInWaitCount;}
};



#endif






