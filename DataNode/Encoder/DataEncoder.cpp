#include "DataEncoder.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


DataEncoder::DataEncoder()
 : m_pFuncEncodeApi( NULL )
{
}

int DataEncoder::Initialize( std::string sModulePath )
{
	Release();
	DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Initialize() : initializing data encoder plugin ......" );

	T_Func_FetchModuleVersion	pFunGetVersion = NULL;
	int							nErrorCode = m_oDllPlugin.LoadDll( sModulePath );

	if( 0 != nErrorCode )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataEncoder::Initialize() : failed 2 load data encoder plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	pFunGetVersion = (T_Func_FetchModuleVersion)m_oDllPlugin.GetDllFunction( "FetchModuleVersion" );
	m_pFuncEncodeApi = (T_Func_GetEncodeApi)m_oDllPlugin.GetDllFunction( "GetEncodeApi" );

	if( NULL == pFunGetVersion || NULL == m_pFuncEncodeApi )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataEncoder::Initialize() : invalid fuction pointer(NULL)" );
		return -100;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Initialize() : DataEncoder Version => [%s] ......", pFunGetVersion() );

/*
	if( 0 != (nErrorCode = m_pFuncInitialize( pIDataCallBack )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataCollector::Initialize() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	s_nMarketID = m_pFuncGetMarketID();
	m_bIsProxyPlugin = m_pFuncIsProxy();

	DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::Initialize() : data collector plugin is initialized ......" );
*/
	return 0;
}

void DataEncoder::Release()
{
	if( NULL != m_pFuncEncodeApi )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Release() : releasing data encoder plugin ......" );
		m_pFuncEncodeApi = NULL;
		DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Release() : data encoder plugin is released ......" );
	}

	m_pFuncEncodeApi = NULL;
	m_oDllPlugin.CloseDll();
}















