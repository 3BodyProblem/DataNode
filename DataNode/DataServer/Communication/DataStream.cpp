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
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nBodyLen = sizeof(tagPackageHead) + nDataSize;
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nMsgCount = 1;
	}
	else
	{
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nMsgCount++;
		((tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition))->nBodyLen += nDataSize;
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

	tagPackageHead*		pPkgHead = (tagPackageHead*)m_pPkgBuffer[m_nFirstPosition];

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

	if( NULL == (m_pSendBuffer = new char[nNewBuffSize/2]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "QuotationStream::Instance() : failed 2 initialize send data buffer, size = %d", nNewBuffSize );
		return -1;
	}
	m_nMaxSendBufSize = nNewBuffSize/2;

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

int QuotationStream::PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen, unsigned __int64 nSeqNo )
{
	if( NULL == pData || 0 == nLen )
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

void QuotationStream::FlushQuotation2Client()
{
	LinkIDSet::LINKID_VECTOR	vctLinkID;
	unsigned int				nLinkCount = LinkIDSet::GetSetObject().FetchLinkIDList( vctLinkID+0, 32 );

	if( false == m_oDataBuffer.IsEmpty() && nLinkCount > 0 )
	{
		unsigned int	nDataID = 0;
		int				nDataSize = m_oDataBuffer.GetOnePkg( m_pSendBuffer, m_nMaxSendBufSize );
		DataNodeService::GetSerivceObj().PushData( vctLinkID+0, nLinkCount, 0, 0, m_pSendBuffer, nDataSize );
	}
}


#define		MAX_IMAGE_BUFFER_SIZE			(1024*1024*5)


ImageRebuilder::ImageRebuilder()
 : m_pSendBuffer( NULL ), m_nMaxSendBufSize( 0 )
{
}

ImageRebuilder& ImageRebuilder::GetRebuilder()
{
	static ImageRebuilder		obj;

	return obj;
}

void ImageRebuilder::Release()
{
	m_oDataBuffer.Release();

	if( NULL != m_pSendBuffer )
	{
		delete []m_pSendBuffer;
		m_pSendBuffer = NULL;
	}

	m_nMaxSendBufSize = 0;
}

int ImageRebuilder::Initialize()
{
	Release();

	if( NULL == (m_pSendBuffer = new char[MAX_IMAGE_BUFFER_SIZE]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "QuotationStream::Instance() : failed 2 initialize send data buffer, size = %d", MAX_IMAGE_BUFFER_SIZE );
		return -1;
	}
	m_nMaxSendBufSize = MAX_IMAGE_BUFFER_SIZE;

	return m_oDataBuffer.Initialize( MAX_IMAGE_BUFFER_SIZE*2 );
}

int ImageRebuilder::QueryCodeListInDatabase( unsigned int nDataID, DatabaseIO& refDatabaseIO, std::set<std::string>& setCode )
{
	bool				bFinded = false;
	CriticalLock		lock( m_oBuffLock );
	unsigned int		lstTableID[64] = { 0 };
	unsigned int		lstRecordWidth[64] = { 0 };
	unsigned int		nTableCount = refDatabaseIO.GetTablesID( lstTableID, 64, lstRecordWidth, 64 );

	if( 0 == nTableCount ) {
		DataNodeService::GetSerivceObj().WriteWarning( "ImageRebuilder::QueryCodeListInDatabase() : database is empty! " );
		return -1;
	}

	setCode.clear();
	for( unsigned int n = 0; n < nTableCount; n++ )
	{
		unsigned int		nTableID = lstTableID[n];
		unsigned int		nRecordLen = lstRecordWidth[n];
		unsigned __int64	nSerialNoOfAnchor = 0;

		if( nDataID != nTableID )	{
			continue;
		}

		int					nDataLen = refDatabaseIO.FetchRecordsByID( nTableID, m_pSendBuffer, MAX_IMAGE_BUFFER_SIZE, nSerialNoOfAnchor );
		if( nDataLen < 0 )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "ImageRebuilder::QueryCodeListInDatabase() : failed 2 fetch image of table, errorcode=%d", nDataLen );
			return -1 * (n*100);
		}

		bFinded = true;
		setCode.insert( std::string(m_pSendBuffer+n*nRecordLen) );
	}

	if( false == bFinded ) {
		DataNodeService::GetSerivceObj().WriteWarning( "ImageRebuilder::QueryCodeListInDatabase() : miss table in database, TableID=%d", nDataID );
	}

	return setCode.size();
}

int ImageRebuilder::Flush2ReqSessions( DatabaseIO& refDatabaseIO, unsigned __int64 nSerialNo )
{
	CriticalLock		lock( m_oBuffLock );
	unsigned int		lstTableID[64] = { 0 };
	int					nSetSize = GetReqSessionCount();
	unsigned int		nTableCount = refDatabaseIO.GetTablesID( lstTableID, 64 );

	for( std::set<unsigned int>::iterator it = m_setNewReqLinkID.begin(); it != m_setNewReqLinkID.end(); it++ )
	{
		for( unsigned int n = 0; n < nTableCount && nSetSize > 0; n++ )
		{
			unsigned int		nTableID = lstTableID[n];
			unsigned __int64	nSerialNoOfAnchor = nSerialNo;
			int					nDataLen = refDatabaseIO.FetchRecordsByID( nTableID, m_pSendBuffer, MAX_IMAGE_BUFFER_SIZE, nSerialNoOfAnchor );

			if( nDataLen < 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "ImageRebuilder::Flush2ReqSessions() : failed 2 fetch image of table, errorcode=%d", nDataLen );
				return -1 * (n*100);
			}

			nDataLen = DataNodeService::GetSerivceObj().SendData( *it, 0, 0, m_pSendBuffer, nDataLen/*, nSerialNo*/ );
			if( nDataLen < 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "ImageRebuilder::Flush2ReqSessions() : failed 2 send image data, errorcode=%d", nDataLen );
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








