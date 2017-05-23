#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


PackagesBuffer::PackagesBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxPkgBufSize( 0 )
 , m_nFirstPosition( 0 ), m_nLastPosition( 0 )
{
}

PackagesBuffer::~PackagesBuffer()
{
	Release();
}

int PackagesBuffer::Initialize( unsigned long nMaxBufSize )
{
	Release();

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
		delete [] m_pPkgBuffer;
		m_pPkgBuffer = NULL;
		m_nMaxPkgBufSize = 0;
		m_nFirstPosition = 0;
		m_nLastPosition = 0;
	}
}

int PackagesBuffer::PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, bool bEnclosePkg )
{
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
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nMsgCount = 1;
	}
	else
	{
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nMsgCount++;
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
		::memcpy( pszFullData, pData, nDataSize );

		::memcpy( &m_pPkgBuffer[m_nLastPosition], pszFullData, nConsecutiveFreeSize );
		::memcpy( &m_pPkgBuffer[0], pszFullData + nConsecutiveFreeSize, (nBlockSize - nConsecutiveFreeSize) );
		m_nLastPosition = (m_nLastPosition + nBlockSize) % m_nMaxPkgBufSize;
	}

	return 0;
}

int PackagesBuffer::GetOnePkg( char* pBuff, unsigned int nBuffSize )
{
	if( NULL == pBuff || nBuffSize == 0 || NULL == m_pPkgBuffer || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	int	nDataLen = (m_nLastPosition + m_nMaxPkgBufSize - m_nFirstPosition) % m_nMaxPkgBufSize;
	if( nDataLen == 0 )
	{
		return -2;
	}
	else if( lInSize > nDataLen )
	{
		nBuffSize = nDataLen;
	}

	icopysize = m_lMaxRecord - m_lFirstRecord;
	if( icopysize >= lInSize )
	{
		memcpy( lpOut,&m_lpRecordData[m_lFirstRecord],sizeof(tempclass) * lInSize );
	}
	else
	{
		memcpy( lpOut,&m_lpRecordData[m_lFirstRecord],sizeof(tempclass) * icopysize );
		memcpy( lpOut + icopysize,&m_lpRecordData[0],sizeof(tempclass) * (lInSize - icopysize) );
	}

	m_nFirstPosition = (m_nFirstPosition + nBuffSize) % m_nMaxPkgBufSize;
	
	return nBuffSize;
}


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
		DataNodeService::GetSerivceObj().WriteError( "QuotationStream::Instance() : failed 2 initialize send data buffer, size = %d", nNewBuffSize );
		return -1;
	}
	m_nMaxSendBufSize = nNewBuffSize;

	if( 0 != m_oDataBuffer.Initialize( nNewBuffSize ) )	///< 从内存池申请一块缓存
	{
		DataNodeService::GetSerivceObj().WriteError( "QuotationStream::Instance() : failed 2 allocate cache, size = %d", nNewBuffSize );
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
	bool			bNeedActivateEvent = m_oDataBuffer.IsEmpty();				///< 是否需要激活事件对象
	int				nErrorCode = m_oDataBuffer.PutData( nMsgID, pData, nLen );	///< 缓存数据

	if( true == bNeedActivateEvent )
	{
		m_oWaitEvent.Active();
	}

	return nErrorCode;
}

void QuotationStream::FlushQuotation2Client()
{
	LinkIDSet::LINKID_VECTOR	vctLinkID;
	CriticalLock				guard( m_oLock );
	unsigned int				nLinkCount = LinkIDSet::GetSetObject().FetchLinkIDList( vctLinkID+0, 32 );

	if( false == m_oDataBuffer.IsEmpty() && nLinkCount > 0 )
	{
		unsigned int	nDataID = 0;
		int				nDataSize = m_oDataBuffer.GetData( nDataID, m_pSendBuffer, m_nMaxSendBufSize );
		DataNodeService::GetSerivceObj().PushData( vctLinkID+0, nLinkCount, 0, 0, m_pSendBuffer, nDataSize );
	}
}


#define		MAX_IMAGE_BUFFER_SIZE			(1024*1024*10)


ImageRebuilder::ImageRebuilder()
 : m_pImageDataBuffer( NULL )
{
}

ImageRebuilder& ImageRebuilder::GetRebuilder()
{
	static ImageRebuilder		obj;

	return obj;
}

void ImageRebuilder::Release()
{
	if( NULL != m_pImageDataBuffer )
	{
		delete [] m_pImageDataBuffer;
		m_pImageDataBuffer = NULL;
	}
}

int ImageRebuilder::Initialize()
{
	Release();
	m_pImageDataBuffer = new char[MAX_IMAGE_BUFFER_SIZE];	///< 分配10M的快照数据缓存(用于对下初始化)

	if( NULL == m_pImageDataBuffer )
	{
		DataNodeService::GetSerivceObj().WriteError( "ImageRebuilder::Initialize() : failed 2 initialize Image buffer ..." );
		return -1;
	}

	return 0;
}

int ImageRebuilder::Flush2ReqSessions( DatabaseIO& refDatabaseIO, unsigned __int64 nSerialNo )
{
	CriticalLock		lock( m_oBuffLock );
	unsigned int		lstTableID[64] = { 0 };
	int					nSetSize = GetReqSessionCount();
	unsigned int		nTableCount = refDatabaseIO.GetTablesID( lstTableID, 64 );

	for( std::set<unsigned int>::iterator it = m_setNewReqLinkID.begin(); it != m_setNewReqLinkID.end(); it++ )
	{
		for( int n = 0; n < nTableCount && nSetSize > 0; n++ )
		{
			unsigned int		nTableID = lstTableID[n];
			unsigned __int64	nSerialNoOfAnchor = nSerialNo;
			int					nDataLen = refDatabaseIO.FetchRecordsByID( nTableID, m_pImageDataBuffer, MAX_IMAGE_BUFFER_SIZE, nSerialNoOfAnchor );

			if( nDataLen < 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "LinkSessions::OnNewLink() : failed 2 fetch image of table, errorcode=%d", nDataLen );
				return -1 * (n*100);
			}

			nDataLen = DataNodeService::GetSerivceObj().SendData( *it, 0, 0, m_pImageDataBuffer, nDataLen/*, nSerialNo*/ );
			if( nDataLen < 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "LinkSessions::OnNewLink() : failed 2 send image data, errorcode=%d", nDataLen );
				return -2 * (n*100);
			}
		}

		LinkIDSet::GetSetObject().NewLinkID( *it );
		m_setNewReqLinkID.erase( it++ );
	}

	return nSetSize;
}

unsigned int ImageRebuilder::GetReqSessionCount()
{
	CriticalLock		lock( m_oBuffLock );

	return m_setNewReqLinkID.size();
}

bool ImageRebuilder::AddNewReqSession( unsigned int nLinkNo )
{
	CriticalLock		lock( m_oBuffLock );

	if( m_setNewReqLinkID.find( nLinkNo ) == m_setNewReqLinkID.end() )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "ImageRebuilder::AddNewReqSession() : [WARNING] duplicate link number & new link will be disconnected..." );

		return false;
	}

	m_setNewReqLinkID.insert( nLinkNo );

	return true;
}








