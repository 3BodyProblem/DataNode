#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


PackagesLoopBuffer::PackagesLoopBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxPkgBufSize( 0 )
 , m_nFirstPosition( 0 ), m_nLastPosition( 0 )
 , m_nCurFirstPos( 0 )
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
		m_nFirstPosition = 0;
		m_nLastPosition = 0;
		m_nCurFirstPos = 0;
	}
}

float PackagesLoopBuffer::GetPercentOfFreeSize()
{
	CriticalLock		guard( m_oLock );

	float	nFreeSize = ((m_nFirstPosition + m_nMaxPkgBufSize - m_nLastPosition - 1) % m_nMaxPkgBufSize) * 1.0;

	return nFreeSize / m_nMaxPkgBufSize * 100;
}

int PackagesLoopBuffer::PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo )
{
	CriticalLock		guard( m_oLock );

	if( NULL == m_pPkgBuffer || nDataSize == 0 || NULL == pData || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	///< 计算余下的空间
	int	nFreeSize = (m_nFirstPosition + m_nMaxPkgBufSize - m_nLastPosition - 1) % m_nMaxPkgBufSize;

	if( m_nCurFirstPos == m_nLastPosition )
	{
		if( (sizeof(tagPackageHead)+sizeof(unsigned int)+nDataSize) > nFreeSize )
		{
			return -2;
		}
	}
	else
	{
		if( nDataSize > nFreeSize )
		{
			return -3;
		}

		///< 如果数据包的id不等于前面的数据包id，则新启用一个package段
		if( nDataID != *((unsigned int*)(m_pPkgBuffer+m_nCurFirstPos)) )
		{
			m_nCurFirstPos = m_nLastPosition;
		}
	}

	///< 处理包头的信息 m_nCurFirstPos
	if( m_nCurFirstPos == m_nLastPosition )
	{
		*((unsigned int*)(m_pPkgBuffer+m_nCurFirstPos)) = nDataID;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurFirstPos+sizeof(unsigned int)))->nSeqNo = nSeqNo;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurFirstPos+sizeof(unsigned int)))->nMarketID = DataCollector::GetMarketID();
		((tagPackageHead*)(m_pPkgBuffer+m_nCurFirstPos+sizeof(unsigned int)))->nMsgLength = nDataSize;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurFirstPos+sizeof(unsigned int)))->nMsgCount = 1;
		m_nLastPosition += (sizeof(tagPackageHead) + sizeof(unsigned int));
	}
	else
	{
		((tagPackageHead*)(m_pPkgBuffer+m_nCurFirstPos+sizeof(unsigned int)))->nMsgCount++;
	}

	int				nConsecutiveFreeSize = m_nMaxPkgBufSize - m_nLastPosition;
	if( nConsecutiveFreeSize >= nDataSize )
	{
		::memcpy( &m_pPkgBuffer[m_nLastPosition], (char*)pData, nDataSize );
		m_nLastPosition = (m_nLastPosition + nDataSize) % m_nMaxPkgBufSize;
	}
	else
	{
		::memcpy( &m_pPkgBuffer[m_nLastPosition], pData, nConsecutiveFreeSize );
		::memcpy( &m_pPkgBuffer[0], pData + nConsecutiveFreeSize, (nDataSize - nConsecutiveFreeSize) );
		m_nLastPosition = (m_nLastPosition + nDataSize) % m_nMaxPkgBufSize;
	}

	return 0;
}

int PackagesLoopBuffer::GetOnePkg( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID )
{
	CriticalLock		guard( m_oLock );

	tagPackageHead*		pPkgHead = (tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition+sizeof(unsigned int));
	if( NULL == pBuff || nBuffSize == 0 || NULL == m_pPkgBuffer || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	unsigned int		nTotalPkgSize = (pPkgHead->nMsgLength * pPkgHead->nMsgCount) + sizeof(tagPackageHead) + sizeof(unsigned int);	///< 带msgid的长度值
	int					nDataLen = (m_nLastPosition + m_nMaxPkgBufSize - m_nFirstPosition) % m_nMaxPkgBufSize;
	if( nDataLen == 0 )
	{
		return -2;
	}

	nMsgID = *((unsigned int*)(m_pPkgBuffer+m_nFirstPosition));
	if( nBuffSize < nTotalPkgSize )
	{
		return -3;
	}
	else
	{
		nBuffSize = (nTotalPkgSize - sizeof(unsigned int));
	}

	int nConsecutiveSize = m_nMaxPkgBufSize - m_nFirstPosition;
	if( nConsecutiveSize >= nBuffSize )
	{
		::memcpy( pBuff, &m_pPkgBuffer[m_nFirstPosition + sizeof(unsigned int)], nBuffSize );
	}
	else
	{
		::memcpy( pBuff, &m_pPkgBuffer[m_nFirstPosition + sizeof(unsigned int)], nConsecutiveSize );
		::memcpy( pBuff + nConsecutiveSize, m_pPkgBuffer+0, (nBuffSize - nConsecutiveSize) );
	}

	m_nFirstPosition = (m_nFirstPosition + nTotalPkgSize) % m_nMaxPkgBufSize;

	return nBuffSize;
}

bool PackagesLoopBuffer::IsEmpty()
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
		DataNodeService::GetSerivceObj().WriteError( "QuotationSynchronizer::Instance() : failed 2 initialize send data buffer, size = %d", nNewBuffSize/2 );
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
			m_oWaitEvent.Wait();
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

	bool			bNeedActivateEvent = m_oDataBuffer.IsEmpty();							///< 是否需要激活事件对象
	int				nErrorCode = m_oDataBuffer.PushBlock( nMsgID, pData, nLen, nSeqNo );	///< 缓存数据

	if( true == bNeedActivateEvent )
	{
		m_oWaitEvent.Active();
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

		if( nDataSize > 0 && nLinkNoCount > 0 )
		{
			DataNodeService::GetSerivceObj().PushData( vctLinkNo+0, nLinkNoCount, nMsgID, 0, m_oOnePkg, nDataSize );
		}

		if( 0 == nLinkNoCount )
		{
			m_oWaitEvent.Wait();
		}
	}
}












