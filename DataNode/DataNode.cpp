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
		::printf( "\n\n---------------------- [Begin] -------------------------\n" );
		ExecuteTestCase();
		::printf( "----------------------  [End]  -------------------------\n\n\n" );
	}

	int		RunNodeServer()
	{
		int	nErrorCode = DataNodeService::GetSerivceObj().Activate();	///< ����������

		if( 0 != nErrorCode )
		{
			::printf( "RunNodeServer() : failed 2 activate DataNode Service!, errorcode=%d\n", nErrorCode );
			return nErrorCode;
		}

		while( true == DataNodeService::GetSerivceObj().IsAlive() )
		{
			SimpleThread::Sleep( 1000*2 );
		}

		::printf( "RunNodeServer() : joining thread..... \n" );
		DataNodeService::GetSerivceObj().Join();						///< �ȴ��˳�����
		DataNodeService::GetSerivceObj().Destroy();						///< �ͷ�������Դ
		::printf( "RunNodeServer() : thread ended....... \n" );

		return nErrorCode;
	}

}




