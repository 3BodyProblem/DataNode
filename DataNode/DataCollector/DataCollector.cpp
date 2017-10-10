#include "DataCollector.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


CollectorStatus::CollectorStatus()
: m_eStatus( ET_SS_UNACTIVE ), m_nMarketID( -1 )
{
}

CollectorStatus::CollectorStatus( const CollectorStatus& obj )
{
	m_eStatus = obj.m_eStatus;
	m_nMarketID = obj.m_nMarketID;
}

enum E_SS_Status CollectorStatus::Get() const
{
	CriticalLock			lock( m_oCSLock );

	return m_eStatus;
}

bool CollectorStatus::Set( enum E_SS_Status eNewStatus )
{
	CriticalLock			lock( m_oCSLock );

	m_eStatus = eNewStatus;

	return true;
}


unsigned int DataCollector::s_nMarketID = 0;


DataCollector::DataCollector()
 : m_pFuncInitialize( NULL ), m_pFuncRelease( NULL ), m_pFuncIsProxy( NULL )
 , m_pFuncRecoverQuotation( NULL ), m_pFuncGetStatus( NULL ), m_pFuncEcho( NULL )
 , m_bActivated( false ), m_bIsProxyPlugin( false )
{
}

unsigned int DataCollector::GetMarketID()
{
	return s_nMarketID;
}

bool DataCollector::IsProxy()
{
	return m_bIsProxyPlugin;
}

int DataCollector::Initialize( I_DataHandle* pIDataCallBack )
{
	Release();
	DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::Initialize() : initializing data collector plugin ......" );

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

	return 0;
}

void DataCollector::Release()
{
	if( NULL != m_pFuncRelease )
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
	m_oDllPlugin.CloseDll();
}

bool DataCollector::IsAlive()
{
	return m_bActivated;
}

void DataCollector::EchoDumpFile()
{
	if( NULL == m_pFuncEcho )
	{
		::printf( "invalid DLL::Echo() pointer\n" );
	}

	m_pFuncEcho();
}

void DataCollector::HaltDataCollector()
{
	if( NULL != m_pFuncRelease && true == m_bActivated )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::HaltDataCollector() : [NOTICE] data collector is Halting ......" );

		m_pFuncHaltQuotation();
		m_bActivated = false;

		DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::HaltDataCollector() : [NOTICE] data collector Halted ......" );
	}
}

int DataCollector::RecoverDataCollector()
{
	DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::RecoverDataCollector() : recovering data collector ......" );

	if( NULL == m_pFuncRecoverQuotation )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataCollector::RecoverDataCollector() : invalid fuction pointer(NULL)" );
		return -1;
	}

	int		nErrorCode = m_pFuncRecoverQuotation();

	if( 0 != nErrorCode )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataCollector::RecoverDataCollector() : failed 2 recover quotation" );
		return nErrorCode;
	}

	m_bActivated = true;
	s_nMarketID = m_pFuncGetMarketID();
	DataNodeService::GetSerivceObj().WriteInfo( "DataCollector::RecoverDataCollector() : data collector recovered ......" );

	return nErrorCode;
}

enum E_SS_Status DataCollector::InquireDataCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen )
{
	if( NULL == m_pFuncGetStatus )
	{
		return ET_SS_UNACTIVE;
	}

	m_oCollectorStatus.Set( (enum E_SS_Status)m_pFuncGetStatus( pszStatusDesc, nStrLen ) );

	return m_oCollectorStatus.Get();
}














