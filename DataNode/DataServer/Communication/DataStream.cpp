#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


PackagesBuffer::PackagesBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxPkgBufSize( 0 )
 , m_nFirstPosition( 0 ), m_nLastPosition( 0 )
 , m_nMarketID( 0 )
{
}

PackagesBuffer::~PackagesBuffer()
{
	Release();
}

int PackagesBuffer::Initialize( unsigned long nMaxBufSize )
{
	Release();

	CriticalLock	guard( m_oLock );

	if( NULL == (m_pPkgBuffer = new char[nMaxBufSize]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "PackagesBuffer::Instance() : failed 2 initialize package buffer, size = %d", nMaxBufSize );
		return -1;
	}

	m_nMaxPkgBufSize = nMaxBufSize;

	return 0;
}

void PackagesBuffer::Release()
{
	if( NULL != m_pPkgBuffer )
	{
		CriticalLock	guard( m_oLock );

		delete [] m_pPkgBuffer;
		m_pPkgBuffer = NULL;
		m_nMaxPkgBufSize = 0;
		m_nFirstPosition = 0;
		m_nLastPosition = 0;
	}
}

void PackagesBuffer::SetMkID( unsigned int nMkID )
{
	m_nMarketID = nMkID;
}

float PackagesBuffer::GetPercentOfFreeSize()
{
	CriticalLock		guard( m_oLock );

	float	nFreeSize = ((m_nFirstPosition + m_nMaxPkgBufSize - m_nLastPosition - 1) % m_nMaxPkgBufSize) * 1.0;

	return nFreeSize / m_nMaxPkgBufSize * 100;
}

int PackagesBuffer::PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo )
{
	CriticalLock		guard( m_oLock );
	unsigned int		nBlockSize = nDataSize + sizeof(tagBlockHead);

	if( NULL == m_pPkgBuffer || nDataSize == 0 || NULL == pData || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	///< 计算余下的空间
	int	nFreeSize = (m_nFirstPosition + m_nMaxPkgBufSize - m_nLastPosition - 1) % m_nMaxPkgBufSize;
	if ( nBlockSize > nFreeSize )
	{
		return -2;
	}

	///< 处理包头的信息
	if( m_nFirstPosition == m_nLastPosition )
	{
		m_nLastPosition += sizeof(tagPackageHead);
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nSeqNo = nSeqNo;
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nMarketID = m_nMarketID;
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nBodyLen = nBlockSize;
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nMsgCount = 1;
	}
	else
	{
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nMsgCount++;
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nBodyLen += nBlockSize;
	}

	int				nConsecutiveFreeSize = m_nMaxPkgBufSize - m_nLastPosition;
	if( nConsecutiveFreeSize >= nBlockSize )
	{
		tagBlockHead*			pBlockHead = (tagBlockHead*)(m_pPkgBuffer + m_nLastPosition);
		pBlockHead->nDataType = nDataID;
		pBlockHead->nDataLen = nDataSize;
		m_nLastPosition += sizeof(tagBlockHead);
		::memcpy( &m_pPkgBuffer[m_nLastPosition], (char*)pData, pBlockHead->nDataLen );
		m_nLastPosition = (m_nLastPosition + nDataSize) % m_nMaxPkgBufSize;
	}
	else
	{
		char				pszFullData[1024];
		tagBlockHead*		pBlockHead = (tagBlockHead*)pszFullData;

		pBlockHead->nDataType = nDataID;
		pBlockHead->nDataLen = nDataSize;
		::memcpy( pszFullData+sizeof(tagBlockHead), pData, nDataSize );

		::memcpy( &m_pPkgBuffer[m_nLastPosition], pszFullData, nConsecutiveFreeSize );
		::memcpy( &m_pPkgBuffer[0], pszFullData + nConsecutiveFreeSize, (nBlockSize - nConsecutiveFreeSize) );
		m_nLastPosition = (m_nLastPosition + nBlockSize) % m_nMaxPkgBufSize;
	}

	return 0;
}

int PackagesBuffer::GetOnePkg( char* pBuff, unsigned int nBuffSize )
{
	CriticalLock		guard( m_oLock );

	if( NULL == pBuff || nBuffSize == 0 || NULL == m_pPkgBuffer || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	int	nDataLen = (m_nLastPosition + m_nMaxPkgBufSize - m_nFirstPosition) % m_nMaxPkgBufSize;
	if( nDataLen == 0 )
	{
		return -2;
	}

	tagPackageHead*		pPkgHead = (tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition);

	nBuffSize = pPkgHead->nBodyLen + sizeof(tagPackageHead);
	if( nBuffSize < nDataLen )
	{
		return -3;
	}

	int nConsecutiveSize = m_nMaxPkgBufSize - m_nFirstPosition;
	if( nConsecutiveSize >= nBuffSize )
	{
		::memcpy( pBuff, &m_pPkgBuffer[m_nFirstPosition], nBuffSize );
	}
	else
	{
		::memcpy( pBuff, &m_pPkgBuffer[m_nFirstPosition], nConsecutiveSize );
		::memcpy( pBuff + nConsecutiveSize, &m_pPkgBuffer[0], (nBuffSize - nConsecutiveSize) );
	}

	m_nFirstPosition = (m_nFirstPosition + nBuffSize) % m_nMaxPkgBufSize;
	
	return nBuffSize;
}

bool PackagesBuffer::IsEmpty()
{
	if( m_nFirstPosition == m_nLastPosition )
	{
		return true;
	}
	else
	{
		return false;
	}
}


QuotationSynchronizer::QuotationSynchronizer()
 : m_pSendBuffer( NULL ), m_nMaxSendBufSize( 0 ), m_nLinkCount( 0 )
{
}

QuotationSynchronizer::~QuotationSynchronizer()
{
	Release();
}

void QuotationSynchronizer::Release()
{
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

int QuotationSynchronizer::Initialize( unsigned int nNewBuffSize )
{
	Release();

	if( NULL == (m_pSendBuffer = new char[nNewBuffSize/2]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "QuotationSynchronizer::Instance() : failed 2 initialize send data buffer, size = %d", nNewBuffSize );
		return -1;
	}
	m_nMaxSendBufSize = nNewBuffSize/2;

	if( 0 != m_oDataBuffer.Initialize( nNewBuffSize ) )	///< 从内存池申请一块缓存
	{
		DataNodeService::GetSerivceObj().WriteError( "QuotationSynchronizer::Instance() : failed 2 allocate cache, size = %d", nNewBuffSize );
		return -2;
	}

	return SimpleTask::Activate();
}

int QuotationSynchronizer::Execute()
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

float QuotationSynchronizer::GetFreePercent()
{
	return m_oDataBuffer.GetPercentOfFreeSize();
}

int QuotationSynchronizer::PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen, unsigned __int64 nSeqNo )
{
	if( NULL == pData  )
	{
		return -12345;
	}

	bool			bNeedActivateEvent = m_oDataBuffer.IsEmpty();							///< 是否需要激活事件对象
	int				nErrorCode = m_oDataBuffer.PushBlock( nMsgID, pData, nLen, nSeqNo );	///< 缓存数据

	if( true == bNeedActivateEvent )
	{
		m_oWaitEvent.Active();
	}

	return nErrorCode;
}

void QuotationSynchronizer::SetMkID( unsigned int nMkID )
{
	m_oDataBuffer.SetMkID( nMkID );
}

void QuotationSynchronizer::SetLinkNoList( LinkNoRegister& refLinkNoTable )
{
	LINKID_VECTOR		vctLinkNo = { 0 };
	CriticalLock		guard( m_oLock );
	unsigned int		nLinkNoCount = refLinkNoTable.FetchLinkIDList( vctLinkNo+0, MAX_LINKID_NUM );

	if( nLinkNoCount > 0 ) {
		::memcpy( m_vctLinkNo+0, vctLinkNo, nLinkNoCount );
		m_nLinkCount = nLinkNoCount;
	}
}

void QuotationSynchronizer::FlushQuotation2Client()
{
	if( false == m_oDataBuffer.IsEmpty() && m_nLinkCount > 0 )
	{
		CriticalLock	guard( m_oLock );
		int				nDataSize = m_oDataBuffer.GetOnePkg( m_pSendBuffer, m_nMaxSendBufSize );

		DataNodeService::GetSerivceObj().PushData( m_vctLinkNo+0, m_nLinkCount, MESSAGENO, 0, m_pSendBuffer, nDataSize );
	}
}












