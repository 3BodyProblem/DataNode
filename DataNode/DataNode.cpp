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
		DataNodeService		objService;								///< 节点服务器对象
		int					nErrorCode = objService.Activate();		///< 启动服务器

		if( 0 != nErrorCode )
		{
			::printf( "RunNodeServer() : failed 2 activate DataNode Service!, errorcode=%d\n", nErrorCode );
			return nErrorCode;
		}

		while( true == objService.IsAlive() )
		{
			SimpleThread::Sleep( 1000*2 );
		}

		::printf( "RunNodeServer() : joining thread..... \n" );
		objService.Join();											///< 等待退出服务
		objService.Destroy();										///< 释放所有资源
		::printf( "RunNodeServer() : thread ended....... \n" );

		return nErrorCode;
	}

}




