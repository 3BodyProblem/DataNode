#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


SendPackagePool::SendPackagePool()
 : m_pPkgBuffer( NULL ), m_nMaxBufSize( 0 ), m_nOneMsgBufSize( 0 ), m_nAllocatedTimes( 0 ), m_MsgIDCount( 0 )
{
	::memset( m_vctMsgID, 0, sizeof(m_vctMsgID) );
}

int SendPackagePool::Initialize( unsigned int nOneBuffSize, unsigned int nMsgCount )
{
	Release();

	m_MsgIDCount = 0;
	::memset( m_vctMsgID, 0, sizeof(m_vctMsgID) );
	m_nAllocatedTimes = 0;										///< ������Ϣ������
	m_nOneMsgBufSize = nOneBuffSize;							///< Ϊ������Ϣ�������Ļ����С
	m_nMaxBufSize = nOneBuffSize*nMsgCount;						///< �󻺳�����С
	::memset( m_vctAddrMap, 0, sizeof(m_vctAddrMap) );			///< ��ջ���ӳ���
	::memset( m_vctCurDataSize, 0, sizeof(m_vctCurDataSize) );	///< ��Ч���ݳ�������
	if( NULL == (m_pPkgBuffer = new char[m_nMaxBufSize]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::Initialize() : failed 2 initialize pkg data buffer, size = %d", m_nMaxBufSize );
		return -1;
	}

	if( 0 != m_oEncoder.Initialize( Configuration::GetConfigObj().GetCompressPluginPath(), Configuration::GetConfigObj().GetCompressPluginCfg(), nOneBuffSize * 3 ) )
	{
		DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::Initialize() : failed 2 initialize data encoder" );
		return -2;
	}

#ifdef _DEBUG
	if( 0 != m_oDecoder.Initialize( Configuration::GetConfigObj().GetCompressPluginPath(), Configuration::GetConfigObj().GetCompressPluginCfg(), nOneBuffSize * 3 ) )
	{
		DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::Initialize() : failed 2 initialize data decoder" );
		return -3;
	}
#endif

	return SimpleTask::Activate();
}

void SendPackagePool::Release()
{
	SimpleTask::StopThread();
	if( NULL != m_pPkgBuffer )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "SendPackagePool::Release() : release pkg data buffer, size = %d", m_nMaxBufSize );
		delete []m_pPkgBuffer;
		m_pPkgBuffer = NULL;
	}

	m_MsgIDCount = 0;
	::memset( m_vctMsgID, 0, sizeof(m_vctMsgID) );
	m_nMaxBufSize = 0;											///< �󻺳�����С����
	m_nOneMsgBufSize = 0;										///< ������Ϣ�������Ļ����С����
	m_nAllocatedTimes = 0;										///< ������Ϣ������
	::memset( m_vctAddrMap, 0, sizeof(m_vctAddrMap) );			///< ��ջ���ӳ���
	::memset( m_vctCurDataSize, 0, sizeof(m_vctCurDataSize) );	///< ��Ч���ݳ�������
}

int SendPackagePool::Execute()
{
	DataNodeService::GetSerivceObj().WriteInfo( "SendPackagePool::Execute() : enter into thread func ..." );

	while( true == IsAlive() )
	{
		try
		{
			m_oSendNotice.Wait( 1000 );

			SendAllPkg();
		}
		catch( std::exception& err )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "SendPackagePool::Execute() : exception : %s", err.what() );
		}
		catch( ... )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "SendPackagePool::Execute() : unknow exception ..." );
		}
	}

	DataNodeService::GetSerivceObj().WriteInfo( "SendPackagePool::Execute() : exit thread func ..." );

	return 0;
}

int SendPackagePool::SendAllPkg()
{
	int				nSendSize = 0;									///< �������ݵĴ�С
	LINKID_VECTOR	vctLinkNo = { 0 };								///< ������·��
	unsigned int	nLinkCount = LinkNoRegister::GetRegister().FetchLinkNoTable( vctLinkNo+0, MAX_LINKID_NUM );
	CriticalLock	guard( m_oLock );

	for( int n = 0; n < m_MsgIDCount; n++ )
	{
		unsigned int			nMsgID = m_vctMsgID[n];
		int						nBufSize = m_vctCurDataSize[nMsgID];

		if( nBufSize > 0 )
		{
			char*				pMsgBuff = m_vctAddrMap[nMsgID];	///< Message��ͷ�ṹ
			tagPackageHead*		pHead = (tagPackageHead*)pMsgBuff;	///< Package��ͷ�ṹ
			unsigned int		nMsgLen = pHead->nMsgLength;		///< Message�ṹ����
#ifdef _DEBUG
			if( nLinkCount > 0 && NULL != pMsgBuff )
#endif
			{
				if( 0 != m_oEncoder.Prepare4ACompression( (char*)pHead ) )
				{
					DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::SendAllPkg() : failed 2 prepare a compression, messageid=%u", nMsgID );
					return -1;
				}

				for( unsigned int nOffset = sizeof(tagPackageHead); nOffset < nBufSize; nOffset += nMsgLen )
				{
					if( false == m_oEncoder.CompressData( nMsgID, pMsgBuff + nOffset, nMsgLen ) )
					{
						DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::SendAllPkg() : failed 2 compress message, messageid=%u", nMsgID );
						return -2;
					}
				}

				nSendSize += nBufSize;								///< �ۼƷ�����
			}

			m_vctCurDataSize[nMsgID] = 0;							///< ��շ��ͻ���
			DataNodeService::GetSerivceObj().PushData( vctLinkNo+0, nLinkCount, nMsgID, 0, m_oEncoder.GetBufferPtr(), m_oEncoder.GetBufferLen() );

			///< ---------------------- Debugģʽ�µģ���ѹ/��ѹ���Դ��� -------------------------------
#ifdef _DEBUG
			if( 0 != m_oDecoder.Prepare4AUncompression( m_oEncoder.GetBufferPtr(), m_oEncoder.GetBufferLen() ) )
			{
				DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::SendAllPkg() : failed 2 prepare a uncompression, messageid=%u", nMsgID );
				return -100;
			}

			for( unsigned int nOffset = sizeof(tagPackageHead); nOffset < nBufSize; nOffset += nMsgLen )
			{
				if( false == m_oDecoder.UncompressData( nMsgID, (char*)(m_oEncoder.GetBufferPtr()) + nOffset, nMsgLen ) )
				{
					DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::SendAllPkg() : failed 2 compress message, messageid=%u", nMsgID );
					return -200;
				}
			}
#endif

		}
	}

	return nSendSize;
}

int SendPackagePool::DispatchMessage( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, bool bSendDirect )
{
	if( nDataID < 512 )
	{
		CriticalLock		guard( m_oLock );
		char*				pMsgBuff = m_vctAddrMap[nDataID];								///< ���ݰ���ͷ�ṹ
		int					nFreeSize = m_nOneMsgBufSize - m_vctCurDataSize[nDataID];		///< �������µĿռ�

		if( NULL == pMsgBuff )
		{
			if( (m_nAllocatedTimes+1 * m_nOneMsgBufSize) <= m_nMaxBufSize )					///< �ж����µĻ������㹻���η����
			{
				pMsgBuff = m_pPkgBuffer + (m_nAllocatedTimes++ * m_nOneMsgBufSize);			///< ����һ�λ����ַ������Ϣ����
				m_vctAddrMap[nDataID] = (char*)pMsgBuff;									///< ���»����¼���ֵ

				int	nIndex = 0;
				for( nIndex = 0; nIndex < m_MsgIDCount; nIndex++ )
				{
					if( nDataID == m_vctMsgID[nIndex] )
					{
						break;																///< �����Message ID�Ѿ�����ע�ᣬ������
					}
				}

				if( nIndex == m_MsgIDCount )
				{
					m_vctMsgID[m_MsgIDCount++] = nDataID;									///< ע�ᵽ��ϢID�ļ��ϼ�¼
				}
			}
		}

		if( NULL == pMsgBuff || m_nMaxBufSize == 0 || nDataSize == 0 || NULL == pData )
		{
			DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::DispatchMessage() : msgid=%d, Send Pool is not ready.", nDataID );
			return -1;
		}

		if( nFreeSize < nDataSize )
		{
			SendAllPkg();
		}

		if( nFreeSize < nDataSize )
		{
			DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::DispatchMessage() : msgid=%d, not enough buffer, %d < %d", nDataID, nFreeSize, nDataSize );
			m_oSendNotice.Active();
			return -2;
		}

		if( 0 == m_vctCurDataSize[nDataID] )												///< �ջ��棬������Ҫ�ȹ�����ͷ���ṹ
		{
			((tagPackageHead*)pMsgBuff)->nSeqNo = nSeqNo;									///< ��ֵ,���
			((tagPackageHead*)pMsgBuff)->nMarketID = DataCollector::GetMarketID();
			((tagPackageHead*)pMsgBuff)->nMsgLength = nDataSize;							///< ��ֵ,ÿ��Message�ĳ���
			m_vctCurDataSize[nDataID] += sizeof(tagPackageHead);							///< ƫ�Ƴ�һ��sizeof(tagPackageHead) + sizeof(unsigned int)Message ID�ľ���
		}

		::memcpy( pMsgBuff + m_vctCurDataSize[nDataID], (char*)pData, nDataSize );			///< Copy�����岿��(Message)
		m_vctCurDataSize[nDataID] += nDataSize;												///< ������Ч���ݳ��ȼ���

		if( true == bSendDirect )															///< �����ʶΪtrue����ֱ�ӷ��͵����е�ǰ����
		{
			m_oSendNotice.Active();
		}

		return 0;
	}

	return -123;
}














