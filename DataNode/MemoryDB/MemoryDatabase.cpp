#include <vector>
#include "Interface.h"


RecordBlock::RecordBlock()
 : m_pRecordData( NULL ), m_nRecordLen( 0 )
{
}

RecordBlock::RecordBlock( const RecordBlock& record )
{
	m_pRecordData = record.m_pRecordData;
	m_nRecordLen = record.m_nRecordLen;
}

RecordBlock::RecordBlock( char* pRecord, unsigned int nRecordLen )
 : m_pRecordData( pRecord ), m_nRecordLen( nRecordLen )
{
}

unsigned int RecordBlock::Length() const
{
	return m_nRecordLen;
}

const char* RecordBlock::GetPtr() const
{
	return m_pRecordData;
}

bool RecordBlock::IsNone() const
{
	if( NULL == m_pRecordData || m_nRecordLen <= 0 )
	{
		return true;
	}

	return false;
}

bool RecordBlock::Compare( const RecordBlock& refRecord )
{
	if( true == IsNone() || refRecord.IsNone() )
	{
		return false;
	}

	if( GetPtr() == refRecord.GetPtr() && Length() == refRecord.Length() )
	{
		return true;
	}

	if( 0 == ::memcmp( GetPtr(), refRecord.GetPtr(), refRecord.Length() ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

int RecordBlock::CloneFrom( const RecordBlock& refRecord )
{
	if( true == IsNone() || true == refRecord.IsNone() )
	{
		return -1;
	}

	if( 0 != ::memcmp( m_pRecordData, refRecord.GetPtr(), refRecord.Length() ) )
	{
		memcpy( m_pRecordData, refRecord.GetPtr(), refRecord.Length() );

		return 1;
	}

	return 0;
}












