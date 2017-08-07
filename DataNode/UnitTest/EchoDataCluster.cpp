#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataNode.h"
#include "EchoDataCluster.h"
#include "../DataServer/DataEcho.h"


DataClusterPlugin::DataClusterPlugin()
 : m_nMessageID( -1 )
{
	::printf( "loading datacluster.dll from current folder...\n" );
}

DataClusterPlugin::~DataClusterPlugin()
{
	Release();
}

int DataClusterPlugin::TestQuotationEcho( int nMsgID, std::string sKey )
{
	m_nMessageID = nMsgID;
	m_sMessageKey = sKey;

	if( true == Initialize() )
	{
		while( true )
		{
			::Sleep( 1000*2 );
		}

		Release();

		return 0;
	}

	return -100;
}

bool DataClusterPlugin::Initialize()
{
	int		nErrorCode = m_oDllPlugin.LoadDll( "./DataCluster.dll" );

	if( 0 != nErrorCode )
	{
		::printf( "DataClusterPlugin::Initialize() : failed 2 load ./DataCluster.Dll module from current working folder, errorcode=%d", nErrorCode );
		return false;
	}

	m_funcActivate = (T_Func_Activate)m_oDllPlugin.GetDllFunction( "Activate" );
	m_funcDestroy = (T_Func_Destroy)m_oDllPlugin.GetDllFunction( "Destroy" );
	m_funcQuery = (T_Func_Query)m_oDllPlugin.GetDllFunction( "Query" );
	m_funcUnitTest = (T_Func_ExecuteUnitTest)m_oDllPlugin.GetDllFunction( "ExecuteUnitTest" );

	if( NULL == m_funcActivate || NULL == m_funcDestroy || NULL == m_funcQuery || NULL == m_funcUnitTest )
	{
		::printf( "DataClusterPlugin::Initialize() : invalid export function pointer (NULL)" );
		Release();
		return false;
	}

	if( (nErrorCode=m_funcActivate( this )) != 0 )
	{
		::printf( "DataClusterPlugin::Initialize() : failed 2 commence the module, errorcode=%d", nErrorCode );
		Release();
		return false;
	}

	return true;
}

void DataClusterPlugin::Release()
{
	if( NULL != m_funcDestroy )
	{
		m_funcDestroy();

		m_funcDestroy = NULL;
		m_funcActivate = NULL;
		m_funcUnitTest = NULL;
		m_funcQuery = NULL;
	}

	m_oDllPlugin.CloseDll();
}

void DataClusterPlugin::OnQuotation( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{
	if( m_nMessageID == (int)nMessageID )		///< 只回显指定消息
	{
		bool	bEcho = false;

		if( m_sMessageKey.empty() )				///< 不用key过滤的情况
		{
			bEcho = true;
		}
		else									///< 使用记录主键key进行过滤的情况
		{
			if( m_sMessageKey == pDataPtr )
			{
				bEcho = true;
			}
		}

		if( true == bEcho )
		{
			char	pszOutput[1024] = { 0 };

			switch( nMessageID )
			{
			case 1000:
				CTP_DL_Echo::FormatMarketInfoLF1000( pszOutput, *((tagDLMarketInfo_LF1000*)pDataPtr) );
				break;
			case 1007:
				CTP_DL_Echo::FormatMarketStatusHF1007( pszOutput, *((tagDLMarketStatus_HF1007*)pDataPtr) );
				break;
			case 1003:
				CTP_DL_Echo::FormatReferenceDataLF1003( pszOutput, *((tagDLReferenceData_LF1003*)pDataPtr) );
				break;
			case 1004:
				CTP_DL_Echo::FormatSnapDataLF1004( pszOutput, *((tagDLSnapData_LF1004*)pDataPtr) );
				break;
			case 1005:
				CTP_DL_Echo::FormatSnapDataHF1005( pszOutput, *((tagDLSnapData_HF1005*)pDataPtr) );
				break;
			case 1006:
				CTP_DL_Echo::FormatBuySellDataHF1006( pszOutput, *((tagDLSnapBuySell_HF1006*)pDataPtr) );
				break;
			default:
				::memset( pszOutput, 0, sizeof(pszOutput) );
				break;
			}

			if( pszOutput[0] != '\0' || pszOutput[1] != '\0' || pszOutput[2] != '\0' )
			{
				::printf( "%s", pszOutput );
				return;
			}
		}
	}

	::printf( "DataClusterPlugin::OnQuotation() : MsgID=%u, MsgLen=%u \n", nMessageID, nDataLen );
}

void DataClusterPlugin::OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{
	::printf( "DataClusterPlugin::OnStatusChg() : MsgID=%u, MsgLen=%u \n", nMessageID, nDataLen );
}

void DataClusterPlugin::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{
	unsigned int	nLevel = nLogLevel;

	::printf( "[DataCluster.dll] : LogLevel(%u), %s \n", nLevel, pszLogBuf );
}







