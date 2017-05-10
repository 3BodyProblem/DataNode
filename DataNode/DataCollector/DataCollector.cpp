#include "DataCollector.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


CollectorStatus::CollectorStatus()
: m_eStatus( E_STATUS_NONE )
{
}

enum CollectorStatus::E_QS_STATUS CollectorStatus::Get() const
{
	CriticalLock			lock( m_oCSLock );

	return m_eStatus;
}

bool CollectorStatus::Set( enum E_QS_STATUS eNewStatus )
{
	CriticalLock			lock( m_oCSLock );

	m_eStatus = eNewStatus;

	return true;
}


DataCollector::DataCollector()
{
}

int DataCollector::Initialize( I_DataHandle* pIDataCallBack )
{
	Release();

	SvrFramework::GetFramework().WriteInfo( "DataCollector::Initialize() : initializing data collector plugin ......" );

	int						nErrorCode = m_oDllPlugin.LoadDll( Configuration::GetConfigObj().GetDataCollectorPluginPath() );

	if( 0 != nErrorCode )
	{
		SvrFramework::GetFramework().WriteError( "DataCollector::Initialize() : failed 2 load data collector module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	SvrFramework::GetFramework().WriteInfo( "DataCollector::Initialize() : data collector plugin is initialized ......" );

	return 0;
}

void DataCollector::Release()
{
	SvrFramework::GetFramework().WriteInfo( "DataCollector::Release() : releasing memory database plugin ......" );

	m_oDllPlugin.CloseDll();

	SvrFramework::GetFramework().WriteInfo( "DataCollector::Release() : memory database plugin is released ......" );
}

int DataCollector::ReInitializeDriver()
{

	return 0;
}

const CollectorStatus& DataCollector::InquireDriverStatus()
{
	static CollectorStatus	obj;

	return obj;
}














