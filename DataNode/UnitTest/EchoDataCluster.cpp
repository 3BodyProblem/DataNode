#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataNode.h"
#include "EchoDataCluster.h"
#include "../DataServer/DataEcho.h"
#include "../../../../DataCluster/DataCluster/Protocal/DataCluster_Protocal.h"


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
		::printf( "DataClusterPlugin::Initialize() : failed 2 load ./DataCluster.Dll module from current working folder, errorcode=%d\n", nErrorCode );
		return false;
	}

	m_funcActivate = (T_Func_Activate)m_oDllPlugin.GetDllFunction( "Activate" );
	m_funcDestroy = (T_Func_Destroy)m_oDllPlugin.GetDllFunction( "Destroy" );
	m_funcQuery = (T_Func_Query)m_oDllPlugin.GetDllFunction( "Query" );
	m_funcUnitTest = (T_Func_ExecuteUnitTest)m_oDllPlugin.GetDllFunction( "ExecuteUnitTest" );

	if( NULL == m_funcActivate || NULL == m_funcDestroy || NULL == m_funcQuery || NULL == m_funcUnitTest )
	{
		::printf( "DataClusterPlugin::Initialize() : invalid export function pointer (NULL)\n" );
		Release();
		return false;
	}

	if( (nErrorCode=m_funcActivate( this )) != 0 )
	{
		::printf( "DataClusterPlugin::Initialize() : failed 2 commence the module, errorcode=%d\n", nErrorCode );
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

void DataClusterPlugin::OnQuotation( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{
	if( m_nMessageID >= 0 )							///< 只回显指定消息
	{
		if( m_nMessageID != (int)nMessageID )	///< 用MessageID进行过滤的情况
		{
			return;
		}

		if( !m_sMessageKey.empty() )				///< 使用记录主键key进行过滤的情况
		{
			if( m_sMessageKey != pDataPtr )
			{
				return;
			}
		}

		static bool		s_bEchoTitle = false;

		switch( nMessageID%100 )
		{
		case 1:
			{
				tagQUO_MarketInfo*		pEcho = (tagQUO_MarketInfo*)pDataPtr;

				if( false == s_bEchoTitle ) {
					s_bEchoTitle = true;
					::printf( "MarketID,Date,Time,KindCount,WareCount\n" );
				}

				::printf( "%d,%u,%u,%u,%u\n", pEcho->eMarketID, pEcho->uiMarketDate, pEcho->uiMarketTime, pEcho->uiKindCount, pEcho->uiWareCount );
				return;
			}
		case 2:
			{
				tagQUO_ReferenceData*	pEcho = (tagQUO_ReferenceData*)pDataPtr;

				if( false == s_bEchoTitle ) {
					s_bEchoTitle = true;
					::printf( "SecurityID,MarketID,CategoryID,Name,ContractID,CallOrPut,ExercisePrice,StartDate,EndDate,ExerciseDate,DeliveryDate,ExpireDate\n" );
				}

				::printf( "%s,%d,%d,%s,%s,%c,%f,%u,%u,%u,%u,%u"
						, pEcho->szCode, pEcho->eMarketID, pEcho->uiKindID, pEcho->szName, pEcho->szContractID, pEcho->cCallOrPut, pEcho->dExercisePrice
						, pEcho->uiStartDate, pEcho->uiEndDate, pEcho->uiExerciseDate, pEcho->uiDeliveryDate, pEcho->uiExpireDate );
				return;
			}
		case 3:
			{
				tagQUO_SnapData*		pEcho = (tagQUO_SnapData*)pDataPtr;

				if( false == s_bEchoTitle ) {
					s_bEchoTitle = true;
					::printf( "SecurityID,LastPrice,HighPx,LowPx,Amount,Volume,Position,OpenPx,ClosePx,PreClosePx,PhaseCode\n" );
				}

				::printf( "%s,%f,%f,%f,%f,%I64d,%I64d,%f,%f,%f,%s\n"
						, pEcho->szCode, pEcho->dNowPx, pEcho->dHighPx, pEcho->dLowPx, pEcho->dAmount, pEcho->ui64Volume, pEcho->ui64OpenInterest
						, pEcho->dOpenPx, pEcho->dClosePx, pEcho->dPreClosePx, pEcho->szTradingPhaseCode );
				return;
			}
		}
	}
	else											///< 回显所有消息的元信息
	{
		static bool	s_bPrintTitle = false;

		if( false == s_bPrintTitle )
		{
			::printf( "[MessageID],[MessageLength]\n" );
			s_bPrintTitle = true;
		}

		::printf( "%u,%u\n", nMessageID, nDataLen );
	}
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







