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

	m_setMsgID.clear();											///< 需要被推送的消息ID集合
	m_nAllocatedTimes = 0;										///< 分配消息数清零
	m_nOneMsgBufSize = nOneBuffSize;							///< 为单个消息分类分配的缓冲大小
	m_nMaxBufSize = nOneBuffSize*nMsgCount;						///< 大缓冲区大小
	::memset( m_vctAddrMap, 0, sizeof(m_vctAddrMap) );			///< 清空缓存映射表
	::memset( m_vctCurDataSize, 0, sizeof(m_vctCurDataSize) );	///< 有效数据长度清零
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

	m_setMsgID.clear();											///< 需要被推送的消息ID集合
	m_nMaxBufSize = 0;											///< 大缓冲区大小清零
	m_nOneMsgBufSize = 0;										///< 单个消息分类分配的缓冲大小清零
	m_nAllocatedTimes = 0;										///< 分配消息数清零
	::memset( m_vctAddrMap, 0, sizeof(m_vctAddrMap) );			///< 清空缓存映射表
	::memset( m_vctCurDataSize, 0, sizeof(m_vctCurDataSize) );	///< 有效数据长度清零
}

int SendPackagePool::SendAllPkg()
{
	int				nSendSize = 0;									///< 发送数据的大小
	LINKID_VECTOR	vctLinkNo = { 0 };								///< 发送链路表
	unsigned int	nLinkCount = LinkNoRegister::GetRegister().FetchLinkNoTable( vctLinkNo+0, MAX_LINKID_NUM );

	for( std::set<unsigned int>::iterator it = m_setMsgID.begin(); it != m_setMsgID.end(); it++ )
	{
		unsigned int			nMsgID = *it;

		if( m_vctCurDataSize[nMsgID] > 0 )
		{
			char*				pMsgBuff = m_vctAddrMap[nMsgID];	///< 数据包的头结构

			if( nLinkCount > 0 && NULL != pMsgBuff )
			{
				DataNodeService::GetSerivceObj().PushData( vctLinkNo+0, nLinkCount, nMsgID, 0, pMsgBuff, m_vctCurDataSize[nMsgID] );
				nSendSize += m_vctCurDataSize[nMsgID];				///< 累计发送量
			}

			m_vctCurDataSize[nMsgID] = 0;							///< 清空发送缓存
		}
	}

	return nSendSize;
}

int SendPackagePool::DispatchMessage( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, bool bSendDirect )
{
	if( nDataID < 512 )
	{
		CriticalLock		guard( m_oLock );
		char*				pMsgBuff = m_vctAddrMap[nDataID];								///< 数据包的头结构
		int					nFreeSize = m_nMaxBufSize - m_vctCurDataSize[nDataID];			///< 计算余下的空间

		if( NULL == pMsgBuff )
		{
			if( (m_nAllocatedTimes+1 * m_nOneMsgBufSize) <= m_nMaxBufSize )					///< 判断余下的缓冲区足够做次分配的
			{
				pMsgBuff = m_pPkgBuffer + (m_nAllocatedTimes++ * m_nOneMsgBufSize);			///< 划拔一段缓冲地址给该消息类型
				m_vctAddrMap[nDataID] = (char*)pMsgBuff;									///< 更新缓冲记录体的值
				m_setMsgID.insert( nDataID );												///< 消息ID的集合记录
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

		if( 0 == m_vctCurDataSize[nDataID] )												///< 空缓存，所以需要先构建包头部结构
		{
			((tagPackageHead*)pMsgBuff)->nSeqNo = nSeqNo;									///< 赋值,序号
			((tagPackageHead*)pMsgBuff)->nMarketID = DataCollector::GetMarketID();
			((tagPackageHead*)pMsgBuff)->nMsgLength = nDataSize;							///< 赋值,每个Message的长度
			m_vctCurDataSize[nDataID] += sizeof(tagPackageHead);							///< 偏移出一个sizeof(tagPackageHead) + sizeof(unsigned int)Message ID的距离
		}

		::memcpy( pMsgBuff + m_vctCurDataSize[nDataID], (char*)pData, nDataSize );			///< Copy数据体部分(Message)
		m_vctCurDataSize[nDataID] += nDataSize;												///< 更新有效数据长度计数

		if( true == bSendDirect )															///< 如果标识为true，则直接发送掉所有当前数据
		{
			SendAllPkg();
		}

		return 0;
	}

	return -123;
}














