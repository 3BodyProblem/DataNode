#ifndef __INFRASTRUCTURE__THREAD_H__
#define __INFRASTRUCTURE__THREAD_H__


#include "Lock.h"
#include <string>


#ifndef	LINUXCODE
	typedef 	unsigned int			eng_tid_t;
#else
	typedef		pthread_t				eng_tid_t;	// pthread_t is not thread id, but the program used it as thread id
#endif


/**
 * @class					SimpleThread
 * @brief					���̰߳�װ��
 * @date					2017/5/3
 * @author					barry
 */
class SimpleThread
{
public:
	typedef					void* (__stdcall * tagSimpleTheadFunction)( void* );			///< �̺߳�������
public:
	SimpleThread();
	~SimpleThread();

public:
	/**
	 * @brief				�����߳�
	 * @param[in]			sThreadName			�߳�����
	 * @param[in]			fpFunction			�̺߳���
	 * @param[in]			lpParam				��������
	 * @return				==0					�ɹ�
	 */
	int						Create( std::string sThreadName, tagSimpleTheadFunction fpFunction, void *lpParam );

	/**
	 * @brief				ֹͣ�߳�
	 * @param[in]			nWaitTime			�˳��ȴ�ʱ��(����)
	 */
	void					Join( unsigned long nWaitTime = 5000 );

public:
	/**
	 * @brief				�߳��Ƿ�����
	 * @return				true				���ڹ�����
							false				�ǹ���״̬
	 */
	bool					IsAlive();

	/**
	 * @brief				�����߳�ֹͣ��ʶ
	 */
	void					StopThread();

	/**
	 * @brief				�������п����̵߳�ֹͣ��ʶ
	 */
	static void				StopAllThread();

	/**
	 * @brief				���ߺ���
	 * @param[in]			nMSec				����ʱ��(����)
	 */
	static void				Sleep( unsigned long nMSec );

protected:
	static bool							s_bAllThreadStopFlag;			///< �����߳�ֹͣ��ʶ
	bool								m_bThreadStopFlag;				///< �߳�ֹͣ��ʶ
	std::string							m_sThreadName;					///< �߳�����
	int									m_nThreadID;					///< �߳�ʶ���
	#ifndef LINUXCODE
		HANDLE							m_hHandle;						///< �߳̾��
	#else
		pthread_t						m_hHandle;						///< �߳̾��
	#endif
};


/**
 * @class					SimpleTask
 * @brief					�߳����������
 * @date					2017/5/3
 * @author					barry
 */
class SimpleTask : public SimpleThread
{
public:
	/**
	 * @brief				����
	 * @param[in]			sTaskName			����������
	 */
	SimpleTask( std::string sTaskName = "Thread:None" );
	~SimpleTask();

	/**
	 * @brief				���������߳�
	 * @return				==0					�ɹ�
							!=0					ʧ��
	 */
	int						Activate();

protected:
	/**
	 * @brief				������(��ѭ��)
	 * @return				==0					�ɹ�
							!=0					ʧ��
	 */
	virtual int				Execute() = 0;

protected:
	/**
	 * @brief				�̺߳���
	 * @param[in]			pParam				�̺߳�������
	 */
	static void* __stdcall pthreadFunction( void* pParam );

protected:
	std::string				m_sTaskName;		///< ��������
};





#endif



