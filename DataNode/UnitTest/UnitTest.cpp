#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataNode.h"


///< --------------------- ��Ԫ�����ඨ�� --------------------------------



void TestTableOperation::SetUpTestCase()
{
}

void TestTableOperation::TearDownTestCase()
{
}

void TestTableOperation::SetUp()
{
}

void TestTableOperation::TearDown()
{
}


///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------


TEST_F( TestTableOperation, DumpEmptyTablesAndLoad )
{
//	ASSERT_EQ( UnitTestEnv::GetDatabasePtr()->SaveToDisk( "./DataRecover/" ), false );
//	ASSERT_EQ( UnitTestEnv::GetDatabasePtr()->LoadFromDisk( "./DataRecover/" ), false );
}


///< ------------ ��Ԫ���Գ�ʼ���ඨ�� ------------------------------------


void UnitTestEnv::SetUp()
{
	///< ����һ�����ݿ�
//	ASSERT_NE( m_pIDatabase, (I_Database*)NULL );
}

void UnitTestEnv::TearDown()
{
	///< �ͷ�һ�����ݿ���Դ
//	for( int n = 0; n < 10; n++ )	{
//		ASSERT_EQ( GetFactoryObject().ReleaseAllDatabase(), true );
//	}
}


///< ---------------- ��Ԫ���Ե����������� -------------------------------


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











