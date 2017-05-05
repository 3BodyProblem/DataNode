#include "DataCollector.h"


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
 : m_pIDataCallBack( NULL )
{
}

int DataCollector::Initialize( I_DataHandle* pIDataCallBack )
{

	return -1;
}

void DataCollector::Release()
{

}

int DataCollector::OnReInitializeDriver()
{

	return -1;
}

const CollectorStatus& DataCollector::OnInquireDriverStatus()
{
	static CollectorStatus	obj;

	return obj;
}

int DataCollector::OnIdle()
{

	return -1;
}

int DataCollector::OnOverTime( int nReaseon )
{
	return -1;
}












