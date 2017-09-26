#include <time.h>
#include "targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "DataNode.h"
#include "UnitTest/UnitTest.h"
#include "DataServer/NodeServer.h"


extern "C"
{
	void	ExecuteUnitTest()
	{
		ExecuteTestCase();
	}

	int		RunNodeServer()
	{
		int	nErrorCode = DataNodeService::GetSerivceObj().Initialize();	///< ����������

		if( 0 != nErrorCode )
		{
			::printf( "RunNodeServer() : failed 2 activate DataNode Service!, errorcode=%d\n", nErrorCode );
			return nErrorCode;
		}

		///< ���"�����߳�"��"��ز��"�Ƿ��ڹ���״̬
		while( true == DataNodeService::GetSerivceObj().IsAlive() && false == DataNodeService::GetSerivceObj().IsStop() )
		{
			SimpleThread::Sleep( 1000*2 );
		}

		time_t	nTimeLast = ::time( NULL );
		::printf( "RunNodeServer() : joining thread..... \n" );
		DataNodeService::GetSerivceObj().Join();						///< �ȴ��˳�����
		DataNodeService::GetSerivceObj().Release();						///< �ͷ�������Դ
		::printf( "RunNodeServer() : thread ended....... (duration=%d, time_t=%d) \n", (::time( NULL )-nTimeLast) );

		return nErrorCode;
	}

}




