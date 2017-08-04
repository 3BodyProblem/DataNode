#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


PackagesLoopBuffer::PackagesLoopBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxPkgBufSize( 0 )
 , m_nFirstPkgHeadPos( 0 ), m_nCurrentWritePos( 0 )
 , m_nCurrentPkgHeadPos( 0 )
{
}

PackagesLoopBuffer::~PackagesLoopBuffer()
{
	Release();
}

int PackagesLoopBuffer::Initialize( unsigned long nMaxBufSize )
{
	Release();

	CriticalLock	guard( m_oLock );

	if( NULL == (m_pPkgBuffer = new char[nMaxBufSize]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "PackagesLoopBuffer::Instance() : failed 2 initialize package buffer, size = %d", nMaxBufSize );
		return -1;
	}

	m_nMaxPkgBufSize = nMaxBufSize;

	return 0;
}

void PackagesLoopBuffer::Release()
{
	if( NULL != m_pPkgBuffer )
	{
		CriticalLock	guard( m_oLock );

		delete [] m_pPkgBuffer;
		m_pPkgBuffer = NULL;
		m_nMaxPkgBufSize = 0;
		m_nFirstPkgHeadPos = 0;
		m_nCurrentWritePos = 0;
		m_nCurrentPkgHeadPos = 0;
	}
}

float PackagesLoopBuffer::GetPercentOfFreeSize()
{
	CriticalLock		guard( m_oLock );

	float	nFreeSize = ((m_nFirstPkgHeadPos + m_nMaxPkgBufSize - m_nCurrentWritePos - 1) % m_nMaxPkgBufSize) * 1.0;

	return nFreeSize / m_nMaxPkgBufSize * 100;
}

int PackagesLoopBuffer::PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, unsigned int& nMsgCount, unsigned int& nBodySize )
{
	CriticalLock		guard( m_oLock );

	nMsgCount = 0;
	nBodySize = 0;
	if( NULL == m_pPkgBuffer || nDataSize == 0 || NULL == pData || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	///< 计算余下的空间
	int	nFreeSize = (m_nFirstPkgHeadPos + m_nMaxPkgBufSize - m_nCurrentWritePos - 1) % m_nMaxPkgBufSize;

	///< 判断当前空闲空间是否足够
	if( m_nCurrentPkgHeadPos == m_nCurrentWritePos )
	{
		///< (需启用新pkg的情况):	需要考虑message体以外因素的空间占用
		if( (sizeof(tagPackageHead)+sizeof(unsigned int)+nDataSize) > nFreeSize )
		{
			return -2;	///< 空间不足
		}
	}
	else///< (在当前pkg追加的情况):
	{
		if( nDataSize > nFreeSize )
		{
			return -3;	///< 空间不足
		}

		///< 如果数据包的id不等于前面的数据包id，则 (新启用一个pkg包)
		if( nDataID != *((unsigned int*)(m_pPkgBuffer+m_nCurrentPkgHeadPos)) )
		{
			m_nCurrentPkgHeadPos = m_nCurrentWritePos;
		}
	}

	///< 处理包头的信息 m_nCurrentPkgHeadPos
	if( m_nCurrentPkgHeadPos == m_nCurrentWritePos )
	{
		*((unsigned int*)(m_pPkgBuffer+m_nCurrentPkgHeadPos)) = nDataID;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurrentPkgHeadPos+sizeof(unsigned int)))->nSeqNo = nSeqNo;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurrentPkgHeadPos+sizeof(unsigned int)))->nMarketID = DataCollector::GetMarketID();
		((tagPackageHead*)(m_pPkgBuffer+m_nCurrentPkgHeadPos+sizeof(unsigned int)))->nMsgLength = nDataSize;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurrentPkgHeadPos+sizeof(unsigned int)))->nMsgCount = 1;
		m_nCurrentWritePos += (sizeof(tagPackageHead) + sizeof(unsigned int));
	}
	else
	{
		((tagPackageHead*)(m_pPkgBuffer+m_nCurrentPkgHeadPos+sizeof(unsigned int)))->nMsgCount++;
	}

	nMsgCount = ((tagPackageHead*)(m_pPkgBuffer+m_nCurrentPkgHeadPos+sizeof(unsigned int)))->nMsgCount;
	nBodySize = ((tagPackageHead*)(m_pPkgBuffer+m_nCurrentPkgHeadPos+sizeof(unsigned int)))->nMsgLength * nMsgCount;

	int				nConsecutiveFreeSize = m_nMaxPkgBufSize - m_nCurrentWritePos;
	if( nConsecutiveFreeSize >= nDataSize )
	{
		::memcpy( &m_pPkgBuffer[m_nCurrentWritePos], (char*)pData, nDataSize );
		m_nCurrentWritePos = (m_nCurrentWritePos + nDataSize) % m_nMaxPkgBufSize;
	}
	else
	{
		::memcpy( &m_pPkgBuffer[m_nCurrentWritePos], pData, nConsecutiveFreeSize );
		::memcpy( &m_pPkgBuffer[0], pData + nConsecutiveFreeSize, (nDataSize - nConsecutiveFreeSize) );
		m_nCurrentWritePos = (m_nCurrentWritePos + nDataSize) % m_nMaxPkgBufSize;
	}

	return 0;
}

int PackagesLoopBuffer::GetOnePkg( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID )
{
	CriticalLock		guard( m_oLock );
	tagPackageHead*		pPkgHead = (tagPackageHead*)(m_pPkgBuffer+m_nFirstPkgHeadPos+sizeof(unsigned int));
	if( NULL == pBuff || nBuffSize == 0 || NULL == m_pPkgBuffer || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	unsigned int		nTotalPkgSize = (pPkgHead->nMsgLength * pPkgHead->nMsgCount) + sizeof(tagPackageHead) + sizeof(unsigned int);	///< 带msgid的长度值
	int					nDataLen = (m_nCurrentWritePos + m_nMaxPkgBufSize - m_nFirstPkgHeadPos) % m_nMaxPkgBufSize;
	if( nDataLen == 0 )
	{
		return -2;
	}

	nMsgID = *((unsigned int*)(m_pPkgBuffer+m_nFirstPkgHeadPos));
	if( nBuffSize < nTotalPkgSize )
	{
		return -3;
	}
	else
	{
		nBuffSize = (nTotalPkgSize - sizeof(unsigned int));
	}

	int nConsecutiveSize = m_nMaxPkgBufSize - m_nFirstPkgHeadPos;
	if( nConsecutiveSize >= nBuffSize )
	{
		::memcpy( pBuff, &m_pPkgBuffer[m_nFirstPkgHeadPos + sizeof(unsigned int)], nBuffSize );
	}
	else
	{
		::memcpy( pBuff, &m_pPkgBuffer[m_nFirstPkgHeadPos + sizeof(unsigned int)], nConsecutiveSize );
		::memcpy( pBuff + nConsecutiveSize, m_pPkgBuffer+0, (nBuffSize - nConsecutiveSize) );
	}

	unsigned int		nLastFirstPkgHeadPos = m_nFirstPkgHeadPos;

	m_nFirstPkgHeadPos = (m_nFirstPkgHeadPos + nTotalPkgSize) % m_nMaxPkgBufSize;
	if( m_nCurrentPkgHeadPos == nLastFirstPkgHeadPos )
	{
		m_nCurrentPkgHeadPos = m_nFirstPkgHeadPos;
	}


	return nBuffSize;
}

bool PackagesLoopBuffer::IsEmpty()
{
	if( m_nFirstPkgHeadPos == m_nCurrentWritePos )
	{
		return true;
	}
	else
	{
		return false;
	}
}


PkgBuffer::PkgBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxBufSize( 0 )
{
}

int PkgBuffer::Initialize( unsigned int nBuffSize )
{
	Release();

	if( NULL == (m_pPkgBuffer = new char[nBuffSize]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "PkgBuffer::Initialize() : failed 2 initialize pkg data buffer, size = %d", nBuffSize );
		return -1;
	}

	m_nMaxBufSize = nBuffSize;

	return 0;
}

void PkgBuffer::Release()
{
	if( NULL != m_pPkgBuffer )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "PkgBuffer::Release() : release pkg data buffer, size = %d", m_nMaxBufSize );
		delete []m_pPkgBuffer;
		m_pPkgBuffer = NULL;
	}

	m_nMaxBufSize = 0;
}

PkgBuffer::operator char*()
{
	if( NULL != m_pPkgBuffer )
	{
		return m_pPkgBuffer;
	}

	throw std::runtime_error( "PkgBuffer::operator char*() : invalid cache buffer pointer ( NULL )" );
}

unsigned int PkgBuffer::CalPkgSize() const
{
	if( NULL != m_pPkgBuffer )
	{
		tagPackageHead*		pPkgHead = (tagPackageHead*)m_pPkgBuffer;

		return (pPkgHead->nMsgLength * pPkgHead->nMsgCount) + sizeof(tagPackageHead);
	}

	throw std::runtime_error( "PkgBuffer::GetDataSize() : invalid cache buffer pointer ( NULL )" );
}

unsigned int PkgBuffer::MaxBufSize() const
{
	return m_nMaxBufSize;
}


QuotationSynchronizer::QuotationSynchronizer()
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
	m_oOnePkg.Release();
}

int QuotationSynchronizer::Initialize( unsigned int nNewBuffSize )
{
	Release();

	if( 0 != m_oOnePkg.Initialize( nNewBuffSize ) )
	{
		DataNodeService::GetSerivceObj().WriteError( "QuotationSynchronizer::Instance() : failed 2 initialize send data buffer, size = %d", nNewBuffSize );
		return -1;
	}

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
		if( true == m_oDataBuffer.IsEmpty() )
		{
			m_oWaitEvent.Wait( 1000 * 1 );
		}

		FlushQuotation2AllClient();		///< 循环发送缓存中的数据
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

	unsigned int	nMsgCount = 0;			///< 已经缓存的消息数量
	unsigned int	nBodySize = 0;			///< 已经缓存的消息Size
	int				nErrorCode = m_oDataBuffer.PushBlock( nMsgID, pData, nLen, nSeqNo, nMsgCount, nBodySize );	///< 缓存数据

	if( nErrorCode < 0 )
	{
		DataNodeService::GetSerivceObj().WriteError( "QuotationSynchronizer::PutMessage() : failed 2 push message data 2 buffer, errorcode = %d", nErrorCode );
		return nErrorCode;
	}

	if( false == m_oDataBuffer.IsEmpty() )
	{
		if( nMsgCount > 9 || nBodySize > 128*8 )
		{
			m_oWaitEvent.Active();
		}
	}

	return nErrorCode;
}

void QuotationSynchronizer::FlushQuotation2AllClient()
{
	if( false == m_oDataBuffer.IsEmpty() )
	{
		unsigned int	nMsgID = 0;
		LINKID_VECTOR	vctLinkNo = { 0 };			///< 发送链路表
		int				nDataSize = m_oDataBuffer.GetOnePkg( m_oOnePkg, m_oOnePkg.MaxBufSize(), nMsgID );
		unsigned int	nLinkNoCount = LinkNoRegister::GetRegister().FetchLinkNoTable( vctLinkNo+0, MAX_LINKID_NUM );

		if( nDataSize < 0 )
		{
			DataNodeService::GetSerivceObj().WriteError( "QuotationSynchronizer::FlushQuotation2AllClient() : failed 2 fetch package from buffer, errorcode = %d", nDataSize );
			m_oWaitEvent.Wait( 1000 * 1 );
			return;
		}

		if( nLinkNoCount > 0 )
		{
			DataNodeService::GetSerivceObj().PushData( vctLinkNo+0, nLinkNoCount, nMsgID, 0, m_oOnePkg, nDataSize );
		}
		else
		{
			m_oWaitEvent.Wait( 1000 * 1 );
		}
	}
}












