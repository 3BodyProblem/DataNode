#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include "DataNode.h"
#include "DataServer/SvrConfig.h"
#include "MemoryDB/MemoryDatabase.h"


///< 程序入口
int _tmain( int argc, _TCHAR* argv[] )
{
	int		nErrorCode = 0;

	///< 单元测试代码 (条件argc 大于 1)
	if( argc > 1 )
	{
		if( 0 == ::strncmp( argv[1], "testdb", 6 ) )
		{
			DatabaseIO	oDBPlugin;

			if( 0 != (nErrorCode=Configuration::GetConfigObj().Load()) )
			{
				::printf( "failed 2 load configuration\n" );
				return nErrorCode;
			}

			oDBPlugin.UnitTest();

			return nErrorCode;
		}

		if( 0 == ::strncmp( argv[1], "testself", 8 ) )
		{
			::printf( "--------------- [UnitTest] ------------------------\n" );
			ExecuteUnitTest();
			::printf( "--------------- [DONE!] ---------------------------\n" );

			return nErrorCode;
		}

		::printf( "单元测试命令:\na) 本程序名.exe testdb\t调用数据库插件的单元测试。\nb) 本程序名.exe testself\t本模块的单元测试。\n" );

		return 0;
	}

	///< 正常运行服务程序
	::printf( "--------------- [Service] ------------------------\n" );
	nErrorCode = RunNodeServer();
	::printf( "--------------- [DONE! %d] ------------------------\n", nErrorCode );

	return nErrorCode;
}




