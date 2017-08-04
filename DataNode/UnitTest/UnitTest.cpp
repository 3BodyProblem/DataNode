#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataNode.h"
#include "../DataServer/SvrConfig.h"
#include "../InitializeFlag/InitFlag.h"
#include "../DataCollector/DataCollector.h"
#include "../DataServer/Communication/LinkSession.h"


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


std::vector<tagTestMessageStruct> TestLoopBuffer::m_vctMsg1;
std::vector<tagTestMessageStruct> TestLoopBuffer::m_vctMsg2;

void TestLoopBuffer::SetUpTestCase()
{
	for( int a = 0; a < 1024; a++ )
	{
		char					pszKey[64] = { 0 };
		char					pszName[64] = { 0 };
		tagTestMessageStruct	data = { 0 };

		::sprintf( pszName, "a_������_%d", a );
		::strcpy( data.CnName, pszName );
		::sprintf( pszKey, "a_key_%d", a );
		::strcpy( data.Key, pszKey );
		data.MarketID  = 1000 + a;
		data.SeqNo = a;

		m_vctMsg1.push_back( data );
	}

	for( int b = 0; b < 1024; b++ )
	{
		char					pszKey[64] = { 0 };
		char					pszName[64] = { 0 };
		tagTestMessageStruct	data = { 0 };

		::sprintf( pszName, "b_������_%d", b );
		::strcpy( data.CnName, pszName );
		::sprintf( pszKey, "b_key_%d", b );
		::strcpy( data.Key, pszKey );
		data.MarketID  = 2000 + b;
		data.SeqNo = b;

		m_vctMsg2.push_back( data );
	}
}

void TestLoopBuffer::TearDownTestCase()
{
}

void TestLoopBuffer::SetUp()
{
}

void TestLoopBuffer::TearDown()
{
}


///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------

DataCollector		oDataCollector;

TEST_F( TestLogic, CheckConfiguration )
{
	///< �����ļ���Ϣ���ز���
	int						n = 0;
	bool					bInitPoint = false;
	InitializerFlag			oInitPolicy( oDataCollector );
	MkHoliday&				setHoliday = oInitPolicy.GetHoliday();
	Configuration&			refCnf = Configuration::GetConfigObj();
	int						nErrCode = refCnf.Load();
	unsigned int			nModuleVersion = Configuration::GetConfigObj().GetStartInParam().uiVersion;
	const T_VECTOR_PERIODS&	vctPeriods = refCnf.GetTradingPeriods();

	::printf( "\n------------ ������Ϣ�б� ---------------\n" );
	::printf( "[�汾]:V%.2f B%03d    [����ģʽ]:%s\n�������Ŀ¼:%s\n�����ļ�:%s; �ڵ���:%s\n�ڴ���·��:%s\n���ݲɼ����·��:%s\nѹ�����·��:%s\n��ʼ�����:%u(s)\n���̼��:%u(s)\n��¼��:%s\t��¼����:%s\n"
		, (float)(nModuleVersion>>16)/100.f, nModuleVersion&0xFF, refCnf.GetTestFlag()==true?"��":"��"
		, refCnf.GetRecoveryFolderPath().c_str(), refCnf.GetHolidayFilePath().c_str() , refCnf.GetHolidayNodeName().c_str()
		, refCnf.GetMemPluginPath().c_str(), refCnf.GetDataCollectorPluginPath().c_str(), refCnf.GetCompressPluginPath().c_str()
		, refCnf.GetInitInterval(), refCnf.GetDumpInterval(), refCnf.GetCheckName().c_str(), refCnf.GetCheckPassword().c_str() );

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


TEST_F( TestLoopBuffer, CheckPackagesLoopBuffer )
{
	unsigned int			nMsgCount = 0;
	unsigned int			nBodySize = 0;
	unsigned int			nSeqNo = 0;
	unsigned int			nMsgID = 0;
	int						nPkgSize = 0;
	tagTestMessageStruct*	pMsgInPkg = NULL;
	PkgBuffer				oOnePkg;
	PackagesLoopBuffer		oLoopBuffer;

	///< ��ʼ��
	ASSERT_EQ( 0 , oOnePkg.Initialize( 1024*1024*2 ) );
	ASSERT_EQ( 0 , oLoopBuffer.Initialize( 1024*1024*2 ) );

	ASSERT_EQ( 0, oLoopBuffer.PushBlock( 100, (char*)&(m_vctMsg1[0]), sizeof(tagTestMessageStruct), nSeqNo++, nMsgCount, nBodySize ) );
	nPkgSize = oLoopBuffer.GetOnePkg( oOnePkg, oOnePkg.MaxBufSize(), nMsgID );
	ASSERT_EQ( 100, nMsgID );
	ASSERT_EQ( 0, oLoopBuffer.PushBlock( 101, (char*)&(m_vctMsg2[0]), sizeof(tagTestMessageStruct), nSeqNo++, nMsgCount, nBodySize ) );
	nPkgSize = oLoopBuffer.GetOnePkg( oOnePkg, oOnePkg.MaxBufSize(), nMsgID );
	ASSERT_EQ( 101, nMsgID );
	ASSERT_EQ( 0, oLoopBuffer.PushBlock( 100, (char*)&(m_vctMsg1[1]), sizeof(tagTestMessageStruct), nSeqNo++, nMsgCount, nBodySize ) );

	for( std::vector<tagTestMessageStruct>::iterator it = m_vctMsg1.begin(); it != m_vctMsg1.end(); it++ )
	{
		ASSERT_EQ( 0, oLoopBuffer.PushBlock( 100, (char*)&(*it), sizeof(tagTestMessageStruct), nSeqNo++, nMsgCount, nBodySize ) );
	}

	for( std::vector<tagTestMessageStruct>::iterator it = m_vctMsg2.begin(); it != m_vctMsg2.end(); it++ )
	{
		ASSERT_EQ( 0, oLoopBuffer.PushBlock( 101, (char*)&(*it), sizeof(tagTestMessageStruct), nSeqNo++, nMsgCount, nBodySize ) );
	}

	///< -------------------- У��ID100�����ݼ��� -----------------------------
	nPkgSize = oLoopBuffer.GetOnePkg( oOnePkg, oOnePkg.MaxBufSize(), nMsgID );
	ASSERT_EQ( 100, nMsgID );
	ASSERT_EQ( nPkgSize, sizeof(tagPackageHead) + sizeof(tagTestMessageStruct) * m_vctMsg1.size() );

	pMsgInPkg = (tagTestMessageStruct*)( (char*)oOnePkg + sizeof(tagPackageHead) );
	for( std::vector<tagTestMessageStruct>::iterator it = m_vctMsg1.begin(); it != m_vctMsg1.end(); it++ )
	{
		ASSERT_EQ( 0 , ::memcmp( pMsgInPkg, &(*it), sizeof(tagTestMessageStruct) ) );

		pMsgInPkg += 1;
	}

	///< --------------------- У��ID101�����ݼ��� -----------------------------
	nPkgSize = oLoopBuffer.GetOnePkg( oOnePkg, oOnePkg.MaxBufSize(), nMsgID );
	ASSERT_EQ( 101, nMsgID );
	ASSERT_EQ( nPkgSize, sizeof(tagPackageHead) + sizeof(tagTestMessageStruct) * m_vctMsg2.size() );

	pMsgInPkg = (tagTestMessageStruct*)( (char*)oOnePkg + sizeof(tagPackageHead) );
	for( std::vector<tagTestMessageStruct>::iterator it = m_vctMsg2.begin(); it != m_vctMsg2.end(); it++ )
	{
		ASSERT_EQ( 0 , ::memcmp( pMsgInPkg, &(*it), sizeof(tagTestMessageStruct) ) );

		pMsgInPkg += 1;
	}

	///< �ͷ���Դ
	oLoopBuffer.Release();
	oOnePkg.Release();
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











