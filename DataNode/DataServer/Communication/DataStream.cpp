#include "DataStream.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


QuotationStream::QuotationStream()
 : m_pszBuffer( NULL ), m_nBuffSize( 0 ), m_nCurrSize( 0 )
{
}

QuotationStream::~QuotationStream()
{
	Release();
}

void QuotationStream::Release()
{
	CriticalLock	guard( m_oLock );

	SimpleTask::StopThread();	///< 停止线程
	SimpleTask::Join();			///< 退出等待

	///< 释放所有资源
	if( NULL != m_pszBuffer )
	{
		delete [] m_pszBuffer;
		m_pszBuffer = NULL;
	}

	m_nBuffSize = 0;
	m_nCurrSize = 0;
}

int QuotationStream::Initialize( unsigned int nNewBuffSize )
{
	Release();

	CriticalLock	guard( m_oLock );

	m_pszBuffer = new char[nNewBuffSize];			///< 从内存池申请一块缓存
	if( NULL == m_pszBuffer )
	{
		SvrFramework::GetFramework().WriteError( "QuotationStream::Instance() : failed 2 allocate cache, size = %d", nNewBuffSize );
		return -1;
	}

	m_nCurrSize = 0;
	m_nBuffSize = nNewBuffSize;						///< 缓存空间的大小

	return 0;
}

int QuotationStream::Execute()
{
//	while( true )
	{

	}

	return 0;
}

int QuotationStream::EncodeMessage( unsigned short nMsgID, const char *pData, unsigned int nLen )
{
	if( NULL == pData || 0 == nLen )
	{
		return -12345;
	}

//	m_nCurrSize = sizeof(m_cMarketID) + m_oEncoder->EncodeMessage( nMsgID, pData, nLen );

	return m_nCurrSize;
}

void QuotationStream::ResponseData2Platform( unsigned int uiLinkNo, enum QuotationFunctionID eFuncID )
{
/*	if( m_nCurrSize > 0 )
	{
		Global_mCommIO.SendData( uiLinkNo, 0, eFuncID, m_pszBuffer, m_nCurrSize );
	}

	*m_pszBuffer = m_cMarketID;
	m_oEncoder->Attach2Buffer( m_pszBuffer + sizeof m_cMarketID, m_nBuffSize - sizeof m_cMarketID );
	m_nCurrSize = sizeof m_cMarketID;*/
}

void QuotationStream::FlushData2Platform( const unsigned int * lpLinkNoSet, unsigned int uiLinkNoCount )
{
/*	if( m_nCurrSize > 0 && uiLinkNoCount > 0 )
	{
		Global_mCommIO.PushData( lpLinkNoSet, uiLinkNoCount, 0, FUNCTIONID_PUSH_QUOTATION, m_pszBuffer, m_nCurrSize );
	}

	*m_pszBuffer = m_cMarketID;
	m_oEncoder->Attach2Buffer( m_pszBuffer + sizeof m_cMarketID, m_nBuffSize - sizeof m_cMarketID );
	m_nCurrSize = sizeof m_cMarketID;*/
}






