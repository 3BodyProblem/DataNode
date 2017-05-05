#include "Thread.h"
#include <process.h>


#pragma warning(disable:4244)


bool SimpleThread::s_bAllThreadStopFlag = false;


SimpleThread::SimpleThread()
 : m_bThreadStopFlag( true ), m_hHandle( 0 ), m_nThreadID( -1 )
{
}

SimpleThread::~SimpleThread()
{
	Join();
}

int  SimpleThread::Create( std::string sThreadName, tagSimpleTheadFunction fpFunction, void *lpParam )
{
	m_bThreadStopFlag = false;
	s_bAllThreadStopFlag = false;
	m_sThreadName = sThreadName.c_str();

	Join();

	#ifndef LINUXCODE
		m_hHandle = (HANDLE)::_beginthreadex(	0,
												0,
												(unsigned int (__stdcall *)(void *))fpFunction,
												lpParam,
												0,
												(unsigned int*)&m_nThreadID	);
		if ( 0 == m_hHandle )
		{
			m_nThreadID = -1;
			return -1;
		}

		return 0;
	#else
		///< LINUX pthread_create创建的线程的堆栈是10MB,我们的程序通常用不到那么大,所以调整为2MB.这样可以减少逻辑地址
		pthread_attr_t thread_attr;
		size_t stack_size;

		if( pthread_attr_init(&thread_attr) != 0 )	{
			return -1;
		}

		stack_size = 2048*1024;		//设置线程栈空间为2M.
		if( ::pthread_attr_setstacksize(&thread_attr, stack_size) != 0 )	{
			return -2;
		}

		if( ::pthread_create(&m_hHandle,&thread_attr,fpFunction,lpParam) != 0 )	{
			return -3;
		}

		m_ulThreadID = m_hHandle;

		return 0;
	#endif
}

void SimpleThread::Join( unsigned long nWaitTime )
{
	m_nThreadID = -1;
	m_sThreadName = "";
	m_bThreadStopFlag = true;

#ifndef LINUXCODE
		if ( m_hHandle != 0 )
		{
			int	errorcode = ::WaitForSingleObject( m_hHandle, nWaitTime );
			if( errorcode == WAIT_TIMEOUT || errorcode == WAIT_FAILED )	{
				::TerminateThread( m_hHandle, 0 );
			}

			::CloseHandle( m_hHandle );
			m_hHandle = NULL;
		}
#else
		if( m_hHandle != 0 )	{
			int	i, iRet, smt = 1;

			for( i = 0; i < nWaitTime; i++ )
			{
				iRet = pthread_kill( m_hHandle, 0 );	///< 发送0信号,这通常是用来检查线程是否存在
				if( iRet == 0 ) {
					//	线程还存在
					this->rv_CpuYield();
				}
				else if( iRet == ESRCH ) {
					//	线程已经自我了断了.
					break;
				}
				else {
					this->rv_CpuYield();
					continue;
				}
			}
			if( i == nWaitTime )	{
				pthread_cancel( m_hHandle );			///< 超时了,KILL
			}

			m_hHandle = 0;
		}
#endif
}

bool SimpleThread::IsAlive()
{
	if( true == s_bAllThreadStopFlag && true == m_bThreadStopFlag )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SimpleThread::StopThread()
{
	m_bThreadStopFlag = true;
}

void SimpleThread::StopAllThread()
{
	s_bAllThreadStopFlag = true;
}

void SimpleThread::Sleep( unsigned long nMSec )
{
	#ifndef LINUXCODE
		::Sleep( nMSec );
	#else
		struct timeval stdelay;
		lMSec = lMSec * 1000;
		stdelay.tv_sec = lMSec / 1000000;
		stdelay.tv_usec = lMSec % 1000000;
		select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &stdelay);
	#endif
}


//------------------------------------------------------------------------------------------------------------------------------


SimpleTask::SimpleTask( std::string sTaskName )
 : m_sTaskName( sTaskName )
{
}

SimpleTask::~SimpleTask()
{

}

int SimpleTask::Activate()
{
	::printf( "SimpleTask::pthreadFunction() : task[%s] activating........................\n", m_sTaskName.c_str() );

	int		nResult = Create( m_sTaskName, pthreadFunction, this );

	if( 0 == nResult )
	{
		::printf( "SimpleTask::pthreadFunction() : task[%s] activated, errorcode(%d)..........\n", m_sTaskName.c_str(), nResult );
	}
	else
	{
		::printf( "SimpleTask::pthreadFunction() : task[%s] failed 2 activate, error(%d)......\n", m_sTaskName.c_str(), nResult );
	}

	return nResult;
}

void* __stdcall SimpleTask::pthreadFunction( void* pParam )
{
	if( NULL == pParam )
	{
		::printf( "SimpleTask::pthreadFunction() : task[%s], invalid arguments(NULL)..........\n" );
		return NULL;
	}

	SimpleTask&		objTask = *((SimpleTask*)pParam);

	::printf( "SimpleTask::pthreadFunction() : task[%s] is running........................\n", objTask.m_sTaskName.c_str() );

	int				nResult = objTask.Execute();

	::printf( "SimpleTask::pthreadFunction() : task[%s] completed, errorcode(%d)..........\n", objTask.m_sTaskName.c_str(), nResult );

	return NULL;
}








