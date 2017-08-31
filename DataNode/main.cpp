#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <algorithm>
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


class MockDataHandle : public I_DataHandle
{
public:
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag ){	return 0;	};
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag ){	return 0;	};
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen ){	return 0;	};
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
	{
		va_list		valist;
		char		pszLogBuf[8000] = { 0 };

		va_start( valist, pszFormat );
		_vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
		va_end( valist );

		::printf( "[Lv%d] %s\n", nLogLevel, pszLogBuf );
	};
};


/**
 * @brief				运行调试模式函数
 * @author				barry
 */
void RunDebugFunction( int argc, _TCHAR* argv[] )
{
	int						nErrorCode = -1;
	std::string				sCmd = argv[1];

	std::transform( sCmd.begin(), sCmd.end(), sCmd.begin(), ::tolower );				///< 实现大小写不敏感

	if( sCmd == "help" )
	{
		::printf( "单元测试命令:\na) 本程序名.exe testdb\t调用数据库插件的单元测试。\nb) 本程序名.exe testself\t本模块的单元测试。\nc) 本程序名.exe echocluster [TableID] [Code]\t调用客户端插件的行情回显\nd) 本程序名.exe echo\t回显落盘数据文件\n" );
	}
	else if( sCmd == "testdb" )
	{
		DatabaseIO	oDBPlugin;

		::printf( "--------------- [TestDbPlugin] --------------------\n" );
		if( 0 != (nErrorCode=Configuration::GetConfigObj().Load()) )
		{
			::printf( "failed 2 load configuration\n" );
		}

		oDBPlugin.UnitTest();
		::printf( "--------------- [DONE!] ---------------------------\n" );
	}
	else if( sCmd == "testself" )
	{
		::printf( "--------------- [UnitTest] ------------------------\n" );
		ExecuteUnitTest();
		::printf( "--------------- [DONE!] ---------------------------\n" );
	}
	else if( sCmd == "echocluster" )
	{
		::printf( "--------------- [Echo Data] ------------------------\n" );
		int					nTableID = -1;				///< 数据表ID(MessageID),		[-1表示不做过滤]
		std::string			sRecordKey;					///< 记录主键字符串(索引串),	[""表示不做过滤]
		DataClusterPlugin	objDataCluster;				///< 多路行情接入模块管理对象

		if( argc > 2 )
			nTableID = ::atoi( argv[2] );
		if( argc > 3 )
			sRecordKey = argv[3];

		objDataCluster.TestQuotationEcho( nTableID, sRecordKey );
		::printf( "--------------- [DONE!] ---------------------------\n" );
	}
	else if( sCmd == "echo" )
	{
		DataCollector		objDLLNode;
		MockDataHandle		objMockHandle;

		if( 0 != Configuration::GetConfigObj().Load() )
		{
			::printf( "[ERROR] failed 2 load configuration file of DataNode.exe\n" );
			return;
		}

		if( 0 == objDLLNode.Initialize( &objMockHandle ) )
		{
			objDLLNode.EchoDumpFile();
		}
	}
}


///< 程序入口
int _tmain( int argc, _TCHAR* argv[] )
{
	InitializeEnvironment		objInitEnv;					///< 初始化运行环境
	int							nErrorCode = 0;				///< 函数的返回码值

	if( argc > 1 )
	{
		///< --------------------- 单元测试代码 (条件argc 大于 1) -------------------
		::printf( "--------------- [Debug] -------------------------\n" );
		RunDebugFunction( argc, argv );
		::printf( "-------------------------------------------------\n" );
	}
	else
	{
		///< ------------------------ 正常运行服务程序 ------------------------------
		::printf( "--------------- [Service] -------------------------\n" );
		int		nErrorCode = RunNodeServer();
		::printf( "--------------- [DONE!] errorcode=%d --------------\n", nErrorCode );
	}

	return nErrorCode;
}




