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



#endif






