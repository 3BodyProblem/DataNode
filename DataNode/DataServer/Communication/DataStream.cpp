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
	m_nMaxBufSize = 0;											///< 大缓冲区大小清零
	m_nOneMsgBufSize = 0;										///< 单个消息分类分配的缓冲大小清零
	m_nAllocatedTimes = 0;										///< 分配消息数清零
	::memset( m_vctAddrMap, 0, sizeof(m_vctAddrMap) );			///< 清空缓存映射表
	::memset( m_vctCurDataSize, 0, sizeof(m_vctCurDataSize) );	///< 有效数据长度清零
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
	int				nSendSize = 0;									///< 发送数据的大小
	LINKID_VECTOR	vctLinkNo = { 0 };								///< 发送链路表
	unsigned int	nLinkCount = LinkNoRegister::GetRegister().FetchLinkNoTable( vctLinkNo+0, MAX_LINKID_NUM );
	CriticalLock	guard( m_oLock );

	for( int n = 0; n < m_MsgIDCount; n++ )
	{
		unsigned int			nMsgID = m_vctMsgID[n];
		int						nBufSize = m_vctCurDataSize[nMsgID];

		if( nBufSize > 0 )
		{
			char*				pMsgBuff = m_vctAddrMap[nMsgID];	///< Message的头结构
			tagPackageHead*		pHead = (tagPackageHead*)pMsgBuff;	///< Package的头结构
			unsigned int		nMsgLen = pHead->nMsgLength;		///< Message结构长度
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

				nSendSize += nBufSize;								///< 累计发送量
			}

			m_vctCurDataSize[nMsgID] = 0;							///< 清空发送缓存
			DataNodeService::GetSerivceObj().PushData( vctLinkNo+0, nLinkCount, nMsgID, 0, m_oEncoder.GetBufferPtr(), m_oEncoder.GetBufferLen() );

			///< ---------------------- Debug模式下的，缩压/解压测试代码 -------------------------------
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
		char*				pMsgBuff = m_vctAddrMap[nDataID];								///< 数据包的头结构
		int					nFreeSize = m_nOneMsgBufSize - m_vctCurDataSize[nDataID];		///< 计算余下的空间

		if( NULL == pMsgBuff )
		{
			if( (m_nAllocatedTimes+1 * m_nOneMsgBufSize) <= m_nMaxBufSize )					///< 判断余下的缓冲区足够做次分配的
			{
				pMsgBuff = m_pPkgBuffer + (m_nAllocatedTimes++ * m_nOneMsgBufSize);			///< 划拔一段缓冲地址给该消息类型
				m_vctAddrMap[nDataID] = (char*)pMsgBuff;									///< 更新缓冲记录体的值

				int	nIndex = 0;
				for( nIndex = 0; nIndex < m_MsgIDCount; nIndex++ )
				{
					if( nDataID == m_vctMsgID[nIndex] )
					{
						break;																///< 如果该Message ID已经存在注册，则跳出
					}
				}

				if( nIndex == m_MsgIDCount )
				{
					m_vctMsgID[m_MsgIDCount++] = nDataID;									///< 注册到消息ID的集合记录
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
			m_oSendNotice.Active();
		}

		return 0;
	}

	return -123;
}














