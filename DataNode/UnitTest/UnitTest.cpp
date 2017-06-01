#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataNode.h"
#include "../DataServer/SvrConfig.h"
#include "../InitializeFlag/InitFlag.h"


///< --------------------- 单元测试类定义 --------------------------------



void TestLogic::SetUpTestCase()
{
}

void TestLogic::TearDownTestCase()
{
}

void TestLogic::SetUp()
{
}

void TestLogic::TearDown()
{
}


///< ------------------------ 测试用例定义 ----------------------------------------------------
///< ------------------------ 测试用例定义 ----------------------------------------------------
///< ------------------------ 测试用例定义 ----------------------------------------------------
///< ------------------------ 测试用例定义 ----------------------------------------------------
///< ------------------------ 测试用例定义 ----------------------------------------------------
///< ------------------------ 测试用例定义 ----------------------------------------------------
///< ------------------------ 测试用例定义 ----------------------------------------------------


TEST_F( TestLogic, CheckConfiguration )
{
	///< 配置文件信息加载测试
	int						n = 0;
	bool					bInitPoint = false;
	InitializerFlag			oInitPolicy;
	MkHoliday&				setHoliday = oInitPolicy.GetHoliday();
	Configuration&			refCnf = Configuration::GetConfigObj();
	int						nErrCode = refCnf.Load();
	unsigned int			nModuleVersion = Configuration::GetConfigObj().GetStartInParam().uiVersion;
	const T_VECTOR_PERIODS&	vctPeriods = refCnf.GetTradingPeriods();

	::printf( "\n------------ 配置信息列表 ---------------\n" );
	::printf( "[版本]:V%.2f B%03d    [测试模式]:%s\n行情存盘目录:%s\n假日文件:%s; 节点名:%s\n内存插件路径:%s\n数据采集插件路径:%s\n压缩插件路径:%s\n初始化间隔:%u(s)\n落盘间隔:%u(s)\n"
		, (float)(nModuleVersion>>16)/100.f, nModuleVersion&0xFF, refCnf.GetTestFlag()==true?"是":"否"
		, refCnf.GetRecoveryFolderPath().c_str(), refCnf.GetHolidayFilePath().c_str() , refCnf.GetHolidayNodeName().c_str()
		, refCnf.GetMemPluginPath().c_str(), refCnf.GetDataCollectorPluginPath().c_str(), refCnf.GetCompressPluginPath().c_str()
		, refCnf.GetInitInterval(), refCnf.GetDumpInterval() );

	::printf( "\n------------ 交易时段列表 ---------------\n" );
	for( T_VECTOR_PERIODS::const_iterator it = vctPeriods.begin(); it != vctPeriods.end(); it++, n++ )
	{
		const T_TRADING_PERIOD&	period = *it;
		::printf( "(%d) [%d] --- [%d], %s初始化 \n", n, period.nBeginTime, period.nEndTime, true==period.bInitializePoint?"需要":"不需" );
	}

	ASSERT_EQ( 0, nErrCode );

	///< 以下为节假日和交易时段测试
	nErrCode = oInitPolicy.Initialize( refCnf.GetTradingPeriods(), refCnf.GetHolidayFilePath(), refCnf.GetTestFlag() );
	ASSERT_EQ( 0, nErrCode );

	int					nPeriodIndex = oInitPolicy.InTradingPeriod( bInitPoint );
	::printf( "****** 当前时间所在交易时段索引: (%d) ===> %s初始化 ******\n", nPeriodIndex, true==bInitPoint?"需要":"不需" );
	::printf( "\n--------------- 假日列表 ----------------\n" );
	n = 1;
	for( MkHoliday::iterator it = setHoliday.begin(); it != setHoliday.end(); it++, n++ )	{
		::printf( "[%d]\t", *it );
		if( n % 4 == 0 ) {
			::printf( "\n" );
		}
	}
	::printf( "\n\n" );
}



///< ------------ 单元测试初始化类定义 ------------------------------------


void UnitTestEnv::SetUp()
{
	///< 创建一个数据库
//	ASSERT_NE( m_pIDatabase, (I_Database*)NULL );
}

void UnitTestEnv::TearDown()
{
	///< 释放一堆数据库资源
//	for( int n = 0; n < 10; n++ )	{
//		ASSERT_EQ( GetFactoryObject().ReleaseAllDatabase(), true );
//	}
}


///< ---------------- 单元测试导出函数定义 -------------------------------


void ExecuteTestCase()
{
	static	bool	s_bInit = false;

	if( false == s_bInit )	{
		int			nArgc = 1;
		char*		pszArgv[32] = { "DataNode.EXE", };

		s_bInit = true;
		testing::AddGlobalTestEnvironment( new UnitTestEnv() );
		testing::InitGoogleTest( &nArgc, pszArgv );
		RUN_ALL_TESTS();
	}
}











