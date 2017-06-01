#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataNode.h"
#include "../DataServer/SvrConfig.h"
#include "../InitializeFlag/InitFlag.h"


///< --------------------- ��Ԫ�����ඨ�� --------------------------------



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


///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------


TEST_F( TestLogic, CheckConfiguration )
{
	///< �����ļ���Ϣ���ز���
	int						n = 0;
	bool					bInitPoint = false;
	InitializerFlag			oInitPolicy;
	MkHoliday&				setHoliday = oInitPolicy.GetHoliday();
	Configuration&			refCnf = Configuration::GetConfigObj();
	int						nErrCode = refCnf.Load();
	unsigned int			nModuleVersion = Configuration::GetConfigObj().GetStartInParam().uiVersion;
	const T_VECTOR_PERIODS&	vctPeriods = refCnf.GetTradingPeriods();

	::printf( "\n------------ ������Ϣ�б� ---------------\n" );
	::printf( "[�汾]:V%.2f B%03d    [����ģʽ]:%s\n�������Ŀ¼:%s\n�����ļ�:%s; �ڵ���:%s\n�ڴ���·��:%s\n���ݲɼ����·��:%s\nѹ�����·��:%s\n��ʼ�����:%u(s)\n���̼��:%u(s)\n"
		, (float)(nModuleVersion>>16)/100.f, nModuleVersion&0xFF, refCnf.GetTestFlag()==true?"��":"��"
		, refCnf.GetRecoveryFolderPath().c_str(), refCnf.GetHolidayFilePath().c_str() , refCnf.GetHolidayNodeName().c_str()
		, refCnf.GetMemPluginPath().c_str(), refCnf.GetDataCollectorPluginPath().c_str(), refCnf.GetCompressPluginPath().c_str()
		, refCnf.GetInitInterval(), refCnf.GetDumpInterval() );

	::printf( "\n------------ ����ʱ���б� ---------------\n" );
	for( T_VECTOR_PERIODS::const_iterator it = vctPeriods.begin(); it != vctPeriods.end(); it++, n++ )
	{
		const T_TRADING_PERIOD&	period = *it;
		::printf( "(%d) [%d] --- [%d], %s��ʼ�� \n", n, period.nBeginTime, period.nEndTime, true==period.bInitializePoint?"��Ҫ":"����" );
	}

	ASSERT_EQ( 0, nErrCode );

	///< ����Ϊ�ڼ��պͽ���ʱ�β���
	nErrCode = oInitPolicy.Initialize( refCnf.GetTradingPeriods(), refCnf.GetHolidayFilePath(), refCnf.GetTestFlag() );
	ASSERT_EQ( 0, nErrCode );

	int					nPeriodIndex = oInitPolicy.InTradingPeriod( bInitPoint );
	::printf( "****** ��ǰʱ�����ڽ���ʱ������: (%d) ===> %s��ʼ�� ******\n", nPeriodIndex, true==bInitPoint?"��Ҫ":"����" );
	::printf( "\n--------------- �����б� ----------------\n" );
	n = 1;
	for( MkHoliday::iterator it = setHoliday.begin(); it != setHoliday.end(); it++, n++ )	{
		::printf( "[%d]\t", *it );
		if( n % 4 == 0 ) {
			::printf( "\n" );
		}
	}
	::printf( "\n\n" );
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











