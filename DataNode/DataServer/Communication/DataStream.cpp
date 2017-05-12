#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


QuotationStream::QuotationStream()
 : m_pSendBuffer( NULL ), m_nMaxSendBufSize( 0 )
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
	m_oDataBuffer.Release();	///< 释放所有资源

	if( NULL != m_pSendBuffer )
	{
		delete []m_pSendBuffer;
		m_pSendBuffer = NULL;
	}

	m_nMaxSendBufSize = 0;
}

int QuotationStream::Initialize( unsigned int nNewBuffSize )
{
	Release();

	CriticalLock	guard( m_oLock );

	if( NULL == (m_pSendBuffer = new char[nNewBuffSize]) )
	{
		SvrFramework::GetFramework().WriteError( "QuotationStream::Instance() : failed 2 initialize send data buffer, size = %d", nNewBuffSize );
		return -1;
	}
	m_nMaxSendBufSize = nNewBuffSize;

	if( 0 != m_oDataBuffer.Initialize( nNewBuffSize ) )	///< 从内存池申请一块缓存
	{
		SvrFramework::GetFramework().WriteError( "QuotationStream::Instance() : failed 2 allocate cache, size = %d", nNewBuffSize );
		return -2;
	}

	return 0;
}

int QuotationStream::Execute()
{
	while( true )
	{
		if( true == m_oDataBuffer.IsEmpty() )	{
			m_oWaitEvent.Wait();
		}

		FlushQuotation2Client();		///< 循环发送缓存中的数据
	}

	return 0;
}

int QuotationStream::PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen )
{
	if( NULL == pData || 0 == nLen )
	{
		return -12345;
	}

	CriticalLock	guard( m_oLock );
	bool			bNeedActivateEvent = m_oDataBuffer.IsEmpty();			///< 是否需要激活事件对象
	int				nErrorCode = m_oDataBuffer.PutData( pData, nLen );		///< 缓存数据

	if( true == bNeedActivateEvent )
	{
		m_oWaitEvent.Active();
	}

	return nErrorCode;
}

void QuotationStream::FlushQuotation2Client()
{
	CriticalLock				guard( m_oLock );
	LinkIDSet::LINKID_VECTOR	vctLinkID;
	unsigned int				nLinkCount = LinkIDSet::GetSetObject().FetchLinkIDList( vctLinkID+0, 32 );

	if( false == m_oDataBuffer.IsEmpty() && nLinkCount > 0 )
	{
		int	nDataSize = m_oDataBuffer.GetData( m_pSendBuffer, m_nMaxSendBufSize );
		SvrFramework::GetFramework().PushData( vctLinkID+0, nLinkCount, 0, 0, m_pSendBuffer, nDataSize );
	}
}






