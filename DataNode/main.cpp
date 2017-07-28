#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include "DataNode.h"
#include "DataServer/SvrConfig.h"
#include "MemoryDB/MemoryDatabase.h"
#include "UnitTest/EchoDataCluster.h"
#pragma comment( lib, "ws2_32.lib" )


/**
 * @class				InitializeEnvironment
 * @brief				运行环境初始化类
						(socket)
 * @author				barry
 */
class InitializeEnvironment
{
public:
	InitializeEnvironment()
	{
		WSADATA wsd;
		//初始化WinSock2.2
		if( WSAStartup( MAKEWORD(2,2),&wsd ) != 0 )
		{
			::printf( "[FETAL ERROR] InitializeEnvironment::InitializeEnvironment() : failed 2 initialize socket environment... \n" );
		}
	}

	~InitializeEnvironment()
	{
		WSACleanup();
	}
};


///< 程序入口
int _tmain( int argc, _TCHAR* argv[] )
{
	InitializeEnvironment		objInitEnv;					///< 初始化运行环境
	int							nErrorCode = 0;				///< 错误码

	///< --------------------- 单元测试代码 (条件argc 大于 1) --------------------
	if( argc > 1 )
	{
		if( 0 == ::strncmp( argv[1], "testdb", 6 ) )
		{
			DatabaseIO	oDBPlugin;

			::printf( "--------------- [TestDbPlugin] --------------------\n" );
			if( 0 != (nErrorCode=Configuration::GetConfigObj().Load()) )
			{
				::printf( "failed 2 load configuration\n" );
				return nErrorCode;
			}

			oDBPlugin.UnitTest();
			::printf( "--------------- [DONE!] ---------------------------\n" );

			return nErrorCode;
		}

		if( 0 == ::strncmp( argv[1], "testself", 8 ) )
		{
			::printf( "--------------- [UnitTest] ------------------------\n" );
			ExecuteUnitTest();
			::printf( "--------------- [DONE!] ---------------------------\n" );

			return nErrorCode;
		}

		if( 0 == ::strncmp( argv[1], "echocluster", 10 ) )
		{
			::printf( "--------------- [Echo Data] ------------------------\n" );
			DataClusterPlugin	objDataCluster;
			objDataCluster.TestQuotationEcho();
			::printf( "--------------- [DONE!] ---------------------------\n" );

			return nErrorCode;
		}

		::printf( "单元测试命令:\na) 本程序名.exe testdb\t调用数据库插件的单元测试。\nb) 本程序名.exe testself\t本模块的单元测试。\nc) 本程序名.exe echocluster\t调用客户端插件的行情回显\n" );

		return 0;
	}

	///< ------------------------ 正常运行服务程序 ------------------------------
	::printf( "--------------- [Service] -------------------------\n" );
	nErrorCode = RunNodeServer();
	::printf( "--------------- [DONE! %d] ------------------------\n", nErrorCode );

	return nErrorCode;
}




