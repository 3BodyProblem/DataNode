#include "DataEncoder.h"
#include "../DataServer/SvrConfig.h"


DataEncoder::DataEncoder()
{
}

int DataEncoder::Initialize()
{
	Release();
/*	DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::Initialize() : initializing data collector plugin ......" );

	std::string		sModulePath = GetModulePath(NULL) + Configuration::GetConfigObj().GetDataCollectorPluginPath();
	int				nErrorCode = m_oDllPlugin.LoadDll( sModulePath );

	if( 0 != nErrorCode )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataCollector::Initialize() : failed 2 load data collector module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_pFuncInitialize = (T_Func_Initialize)m_oDllPlugin.GetDllFunction( "Initialize" );
	m_pFuncRelease = (T_Func_Release)m_oDllPlugin.GetDllFunction( "Release" );
	m_pFuncRecoverQuotation = (T_Func_RecoverQuotation)m_oDllPlugin.GetDllFunction( "RecoverQuotation" );
	m_pFuncHaltQuotation = (T_Func_HaltQuotation)m_oDllPlugin.GetDllFunction( "HaltQuotation" );
	m_pFuncGetStatus = (T_Func_GetStatus)m_oDllPlugin.GetDllFunction( "GetStatus" );
	m_pFuncGetMarketID = (T_Func_GetMarketID)m_oDllPlugin.GetDllFunction( "GetMarketID" );
	m_pFuncIsProxy = (T_Func_IsProxy)m_oDllPlugin.GetDllFunction( "IsProxy" );
	m_pFuncEcho = (T_Echo)m_oDllPlugin.GetDllFunction( "Echo" );

	if( NULL == m_pFuncInitialize || NULL == m_pFuncRelease || NULL == m_pFuncRecoverQuotation || NULL == m_pFuncGetStatus || NULL == m_pFuncGetMarketID || NULL == m_pFuncHaltQuotation || NULL == m_pFuncIsProxy || NULL == m_pFuncEcho )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataCollector::Initialize() : invalid fuction pointer(NULL)" );
		return -100;
	}

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
/*	if( NULL != m_pFuncRelease )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::Release() : releasing memory database plugin ......" );
		m_pFuncHaltQuotation();
		m_pFuncHaltQuotation = NULL;
		m_pFuncRelease();
		m_pFuncRelease = NULL;
		m_pFuncEcho = NULL;
		m_bActivated = false;
		DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::Release() : memory database plugin is released ......" );
	}

	m_pFuncGetStatus = NULL;
	m_pFuncInitialize = NULL;
	m_pFuncRecoverQuotation = NULL;
	m_oDllPlugin.CloseDll();*/
}















