#include "DataStream.h"
#include "LinkSession.h"
#include "../NodeServer.h"
#pragma warning(disable:4244)


SendPackagePool::SendPackagePool()
 : m_pPkgBuffer( NULL ), m_nMaxBufSize( 0 ), m_nOneMsgBufSize( 0 ), m_nAllocatedTimes( 0 )
{
}

int SendPackagePool::Initialize( unsigned int nOneBuffSize, unsigned int nMsgCount )
{
	Release();

	m_setMsgID.clear();											///< ��Ҫ�����͵���ϢID����
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

	return 0;
}

void SendPackagePool::Release()
{
	if( NULL != m_pPkgBuffer )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "SendPackagePool::Release() : release pkg data buffer, size = %d", m_nMaxBufSize );
		delete []m_pPkgBuffer;
		m_pPkgBuffer = NULL;
	}

	m_setMsgID.clear();											///< ��Ҫ�����͵���ϢID����
	m_nMaxBufSize = 0;											///< �󻺳�����С����
	m_nOneMsgBufSize = 0;										///< ������Ϣ�������Ļ����С����
	m_nAllocatedTimes = 0;										///< ������Ϣ������
	::memset( m_vctAddrMap, 0, sizeof(m_vctAddrMap) );			///< ��ջ���ӳ���
	::memset( m_vctCurDataSize, 0, sizeof(m_vctCurDataSize) );	///< ��Ч���ݳ�������
}

int SendPackagePool::SendAllPkg()
{
	int				nSendSize = 0;									///< �������ݵĴ�С
	LINKID_VECTOR	vctLinkNo = { 0 };								///< ������·��
	unsigned int	nLinkCount = LinkNoRegister::GetRegister().FetchLinkNoTable( vctLinkNo+0, MAX_LINKID_NUM );

	for( std::set<unsigned int>::iterator it = m_setMsgID.begin(); it != m_setMsgID.end(); it++ )
	{
		unsigned int			nMsgID = *it;

		if( m_vctCurDataSize[nMsgID] > 0 )
		{
			char*				pMsgBuff = m_vctAddrMap[nMsgID];	///< ���ݰ���ͷ�ṹ

			if( nLinkCount > 0 && NULL != pMsgBuff )
			{
				DataNodeService::GetSerivceObj().PushData( vctLinkNo+0, nLinkCount, nMsgID, 0, pMsgBuff, m_vctCurDataSize[nMsgID] );
				nSendSize += m_vctCurDataSize[nMsgID];				///< �ۼƷ�����
			}

			m_vctCurDataSize[nMsgID] = 0;							///< ��շ��ͻ���
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
		int					nFreeSize = m_nMaxBufSize - m_vctCurDataSize[nDataID];			///< �������µĿռ�

		if( NULL == pMsgBuff )
		{
			if( (m_nAllocatedTimes+1 * m_nOneMsgBufSize) <= m_nMaxBufSize )					///< �ж����µĻ������㹻���η����
			{
				pMsgBuff = m_pPkgBuffer + (m_nAllocatedTimes++ * m_nOneMsgBufSize);			///< ����һ�λ����ַ������Ϣ����
				m_vctAddrMap[nDataID] = (char*)pMsgBuff;									///< ���»����¼���ֵ
				m_setMsgID.insert( nDataID );												///< ��ϢID�ļ��ϼ�¼
			}
		}

		if( NULL == pMsgBuff || m_nMaxBufSize == 0 || nDataSize == 0 || NULL == pData )
		{
			return -1;
		}

		if( nFreeSize < nDataSize )
		{
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
			SendAllPkg();
		}

		return 0;
	}

	return -123;
}














