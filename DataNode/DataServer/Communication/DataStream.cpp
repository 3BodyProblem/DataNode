#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


unsigned int	g_nMarketID = 0;


PackagesBuffer::PackagesBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxPkgBufSize( 0 )
 , m_nFirstPosition( 0 ), m_nLastPosition( 0 )
 , m_nCurFirstPos( 0 )
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
		m_nCurFirstPos = 0;
	}
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

	if( NULL == m_pPkgBuffer || nDataSize == 0 || NULL == pData || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	///< �������µĿռ�
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

		///< ������ݰ���id������ǰ������ݰ�id����������һ��package��
		if( nDataID != *((unsigned int*)(m_pPkgBuffer+m_nCurFirstPos)) )
		{
			m_nCurFirstPos = m_nLastPosition;
		}
	}

	///< �����ͷ����Ϣ m_nCurFirstPos
	if( m_nCurFirstPos == m_nLastPosition )
	{
		*((unsigned int*)(m_pPkgBuffer+m_nCurFirstPos)) = nDataID;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurFirstPos+sizeof(unsigned int)))->nSeqNo = nSeqNo;
		((tagPackageHead*)(m_pPkgBuffer+m_nCurFirstPos+sizeof(unsigned int)))->nMarketID = g_nMarketID;
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

int PackagesBuffer::GetOnePkg( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID )
{
	CriticalLock		guard( m_oLock );

	tagPackageHead*		pPkgHead = (tagPackageHead*)(m_pPkgBuffer+m_nFirstPosition+sizeof(unsigned int));
	if( NULL == pBuff || nBuffSize == 0 || NULL == m_pPkgBuffer || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	unsigned int		nTotalPkgSize = (pPkgHead->nMsgLength * pPkgHead->nMsgCount) + sizeof(tagPackageHead) + sizeof(unsigned int);	///< ��msgid�ĳ���ֵ
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
	SimpleTask::StopThread();	///< ֹͣ�߳�
	SimpleTask::Join();			///< �˳��ȴ�
	m_oDataBuffer.Release();	///< �ͷ�������Դ

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

	if( 0 != m_oDataBuffer.Initialize( nNewBuffSize ) )	///< ���ڴ������һ�黺��
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

		FlushQuotation2Client();		///< ѭ�����ͻ����е�����
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

	bool			bNeedActivateEvent = m_oDataBuffer.IsEmpty();							///< �Ƿ���Ҫ�����¼�����
	int				nErrorCode = m_oDataBuffer.PushBlock( nMsgID, pData, nLen, nSeqNo );	///< ��������

	if( true == bNeedActivateEvent )
	{
		m_oWaitEvent.Active();
	}

	return nErrorCode;
}

void QuotationSynchronizer::SetLinkNoList()
{
	LINKID_VECTOR		vctLinkNo = { 0 };
	CriticalLock		guard( m_oLock );
	unsigned int		nLinkNoCount = LinkNoRegister::GetRegister().FetchPushLinkIDList( vctLinkNo+0, MAX_LINKID_NUM );

	if( nLinkNoCount > 0 ) {
		::memcpy( m_vctLinkNo+0, vctLinkNo, nLinkNoCount );
		m_nLinkCount = nLinkNoCount;
	}
}

void QuotationSynchronizer::FlushQuotation2Client()
{
	if( false == m_oDataBuffer.IsEmpty() )
	{
		unsigned int	nMsgID = 0;
		CriticalLock	guard( m_oLock );
		int				nDataSize = m_oDataBuffer.GetOnePkg( m_pSendBuffer, m_nMaxSendBufSize, nMsgID );

		DataNodeService::GetSerivceObj().PushData( m_vctLinkNo+0, m_nLinkCount, nMsgID, 0, m_pSendBuffer, nDataSize );
	}
}












