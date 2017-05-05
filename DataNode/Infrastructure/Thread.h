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
 * @brief					简单线程包装类
 * @date					2017/5/3
 * @author					barry
 */
class SimpleThread
{
public:
	typedef					void* (__stdcall * tagSimpleTheadFunction)( void* );			///< 线程函数定义
public:
	SimpleThread();
	~SimpleThread();

public:
	/**
	 * @brief				启动线程
	 * @param[in]			sThreadName			线程名称
	 * @param[in]			fpFunction			线程函数
	 * @param[in]			lpParam				函数参数
	 * @return				==0					成功
	 */
	int						Create( std::string sThreadName, tagSimpleTheadFunction fpFunction, void *lpParam );

	/**
	 * @brief				停止线程
	 * @param[in]			nWaitTime			退出等待时间(毫秒)
	 */
	void					Join( unsigned long nWaitTime = 5000 );

public:
	/**
	 * @brief				线程是否工作中
	 * @return				true				还在工作中
							false				非工作状态
	 */
	bool					IsAlive();

	/**
	 * @brief				设置线程停止标识
	 */
	void					StopThread();

	/**
	 * @brief				设置所有开启线程的停止标识
	 */
	static void				StopAllThread();

	/**
	 * @brief				休眠函数
	 * @param[in]			nMSec				休眠时间(毫秒)
	 */
	static void				Sleep( unsigned long nMSec );

protected:
	static bool							s_bAllThreadStopFlag;			///< 所有线程停止标识
	bool								m_bThreadStopFlag;				///< 线程停止标识
	std::string							m_sThreadName;					///< 线程名称
	int									m_nThreadID;					///< 线程识别号
	#ifndef LINUXCODE
		HANDLE							m_hHandle;						///< 线程句柄
	#else
		pthread_t						m_hHandle;						///< 线程句柄
	#endif
};


/**
 * @class					SimpleTask
 * @brief					线程任务对象类
 * @date					2017/5/3
 * @author					barry
 */
class SimpleTask : public SimpleThread
{
public:
	/**
	 * @brief				构造
	 * @param[in]			sTaskName			任务函数名称
	 */
	SimpleTask( std::string sTaskName = "Thread:None" );
	~SimpleTask();

	/**
	 * @brief				激活任务线程
	 * @return				==0					成功
							!=0					失败
	 */
	int						Activate();

protected:
	/**
	 * @brief				任务函数(内循环)
	 * @return				==0					成功
							!=0					失败
	 */
	virtual int				Execute() = 0;

protected:
	/**
	 * @brief				线程函数
	 * @param[in]			pParam				线程函数参数
	 */
	static void* __stdcall pthreadFunction( void* pParam );

protected:
	std::string				m_sTaskName;		///< 任务名称
};





#endif



