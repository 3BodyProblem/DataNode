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

	m_funcActivate = (tagQUOFun_StartWork)m_oDllPlugin.GetDllFunction( "StartWork" );
	m_funcDestroy = (tagQUOFun_EndWork)m_oDllPlugin.GetDllFunction( "EndWork" );
	m_funcGetMarketID = (tagQUOFun_GetMarketID)m_oDllPlugin.GetDllFunction( "GetMarketID" );
	m_funcGetMarketInfo = (tagQUOFun_GetMarketInfo)m_oDllPlugin.GetDllFunction( "GetMarketInfo" );
	m_funcGetAllRefData = (tagQUOFun_GetAllReferenceData)m_oDllPlugin.GetDllFunction( "GetAllReferenceData" );
	m_funcGetRefData = (tagQUOFun_GetReferenceData)m_oDllPlugin.GetDllFunction( "GetReferenceData" );
	m_funcGetAllSnapData = (tagQUOFun_GetAllSnapData)m_oDllPlugin.GetDllFunction( "GetAllSnapData" );
	m_funcGetSnapData = (tagQUOFun_GetSnapData)m_oDllPlugin.GetDllFunction( "GetSnapData" );
	m_funcUnitTest = (T_Func_ExecuteUnitTest)m_oDllPlugin.GetDllFunction( "ExecuteUnitTest" );

	if( NULL == m_funcActivate || NULL == m_funcDestroy || NULL == m_funcGetMarketID || NULL == m_funcGetMarketInfo
		|| NULL == m_funcGetAllRefData || NULL == m_funcGetRefData || NULL == m_funcGetAllSnapData || NULL == m_funcGetSnapData
		|| NULL == m_funcUnitTest )
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
		m_funcGetMarketID = NULL;
		m_funcGetMarketInfo = NULL;
		m_funcGetAllRefData = NULL;
		m_funcGetRefData = NULL;
		m_funcGetAllSnapData = NULL;
		m_funcGetSnapData = NULL;
	}

	m_oDllPlugin.CloseDll();
}

void DataClusterPlugin::OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
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
					::printf( "SecurityID,LastPrice,HighPx,LowPx,Amount,Volume,Position,OpenPx,ClosePx,PreClosePx,PhaseCode,BidVol,AskVol\n" );
				}

				::printf( "%s,%f,%f,%f,%f,%I64d,%I64d,%f,%f,%f,%s,%I64d,%I64d\n"
						, pEcho->szCode, pEcho->dNowPx, pEcho->dHighPx, pEcho->dLowPx, pEcho->dAmount, pEcho->ui64Volume, pEcho->ui64OpenInterest
						, pEcho->dOpenPx, pEcho->dClosePx, pEcho->dPreClosePx, pEcho->szTradingPhaseCode, pEcho->mBid[0].ui64Volume, pEcho->mAsk[0].ui64Volume );
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

void DataClusterPlugin::OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus )
{
	::printf( "DataClusterPlugin::OnStatusChg() : MarketID=%u, Status=%u \n", eMarketID, eMarketStatus );
/*
	if( eMarketStatus == 2 )
	{
		tagQUO_MarketInfo	tagMk;
		m_funcGetMarketInfo( eMarketID, &tagMk );
	}*/
}

void DataClusterPlugin::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{
	unsigned int	nLevel = nLogLevel;

	::printf( "[DataCluster.dll] : LogLevel(%u), %s \n", nLevel, pszLogBuf );
}







