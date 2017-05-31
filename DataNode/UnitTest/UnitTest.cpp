#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataNode.h"
#include "../DataServer/SvrConfig.h"


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


TEST_F( TestLogic, TestConfiguration )
{
	ASSERT_EQ( 0, Configuration::GetConfigObj().Load() );
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











