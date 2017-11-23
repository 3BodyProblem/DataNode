#include "EchoDataNode.h"
#include "../DataServer/DataEcho.h"
#include "../../../../DataCluster/DataCluster/Protocal/DataCluster_Protocal.h"


static tagMsgInfo				s_vctEncodeMsgInfo[512] = { 0 };		///< 编码的消息描述信息数据
static tagMsgInfo				s_vctDecodeMsgInfo[512] = { 0 };		///< 解码的消息描述信息数据


int XCodeMsgPool::SendAllPkg()
{
	int							nSendSize = 0;							///< 发送数据的大小
	CriticalLock				guard( m_oLock );

	for( int n = 0; n < m_MsgIDCount; n++ )
	{
		unsigned int			nMsgID = m_vctMsgID[n];					///< Message ID
		int						nBufSize = m_vctCurDataSize[nMsgID];	///< 该类型的Message数据累计长度
		char*					pMsgBuff = m_vctAddrMap[nMsgID];		///< Message的头结构

		if( nBufSize > 0 && 0 != nMsgID )								///< nMsgID如果等于0则是心跳包，忽略即可
		{
			tagPackageHead*		pHead = (tagPackageHead*)pMsgBuff;		///< Package的头结构
			tagMsgInfo&			refEnMsgInfo = s_vctEncodeMsgInfo[nMsgID];
			tagMsgInfo&			refDeMsgInfo = s_vctDecodeMsgInfo[nMsgID];

			///< ---------------------- 缩压代码 -------------------------------
			if( 0 != m_oEncoder.Prepare4ACompression( (char*)pHead ) )
			{
				DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::SendAllPkg() : failed 2 prepare a compression, messageid=%u", nMsgID );
				assert( 0 );
				return -1;
			}

			for( unsigned int nOffset = sizeof(tagPackageHead); nOffset < nBufSize; nOffset += pHead->nMsgLength )
			{
				if( false == m_oEncoder.CompressData( nMsgID, pMsgBuff + nOffset, pHead->nMsgLength ) )
				{
					DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::SendAllPkg() : failed 2 compress message, messageid=%u", nMsgID );
					assert( 0 );
					return -2;
				}
			}

			nSendSize += nBufSize;										///< 累计发送量
			m_vctCurDataSize[nMsgID] = 0;								///< 清空发送缓存
#ifdef _DEBUG
			///< ---------------------- 解压代码 -------------------------------
			if( 0 != m_oDecoder.Prepare4AUncompression( m_oEncoder.GetBufferPtr() + sizeof(tagPackageHead), m_oEncoder.GetBufferLen() - sizeof(tagPackageHead) ) )
			{
				DataNodeService::GetSerivceObj().WriteError( "SendPackagePool::SendAllPkg() : failed 2 prepare a uncompression, messageid=%u", nMsgID );
				assert( 0 );
				return -100;
			}

			for( unsigned int nOffset = sizeof(tagPackageHead); nOffset < nBufSize; nOffset += pHead->nMsgLength )
			{
				char			pszOutput[1024] = { 0 };

				if( 0 > m_oDecoder.UncompressData( nMsgID, pszOutput, sizeof(pszOutput) ) )
				{
					::printf( "SendPackagePool::SendAllPkg() : failed 2 compress message, messageid=%u", nMsgID );
					assert( 0 );
					return -200;
				}
				else
				{
					if( ::memcmp( pMsgBuff + nOffset, pszOutput, pHead->nMsgLength ) != 0 )
					{
						::printf( "[WARNING] Find A Discrepancy As Follow (In Vs Out) : \n" );
						EchoNodeEngine::PrintMessage( pHead->nMarketID, nMsgID, pMsgBuff + nOffset );
						EchoNodeEngine::PrintMessage( pHead->nMarketID, nMsgID, pszOutput );
						/*assert( 0 );return -300;*/
					}

					refDeMsgInfo.nMsgCount++;
					if( refDeMsgInfo.nMsgLen == 0 )
					{
						refDeMsgInfo.nMsgLen = pHead->nMsgLength;
					}
				}
			}

			::printf( "[%u] InNum=%I64d, OutNum=%I64d\n", nMsgID, refEnMsgInfo.nMsgCount, refDeMsgInfo.nMsgCount );
#endif
		}
	}

	return nSendSize;
}


///< --------------------------------------------------------------------------------------


EchoNodeEngine::EchoNodeEngine()
 : m_nMessageID( 0 )
{
}

void EchoNodeEngine::Release()
{
	m_sCode = "";
	m_nMessageID = 0;
	m_oSendPackage.Release();
	::memset( s_vctEncodeMsgInfo, 0, sizeof(s_vctEncodeMsgInfo) );
	::memset( s_vctDecodeMsgInfo, 0, sizeof(s_vctDecodeMsgInfo) );
}

int EchoNodeEngine::Initialize( unsigned int nMsgID, std::string sCode )
{
	static	char						pszError[8192] = { 0 };											///< ServicePlug初始化出错时的信息
	int									nErrorCode = Configuration::GetConfigObj().Load();				///< 加载配置信息
	const tagServicePlug_StartInParam&	refInParam = Configuration::GetConfigObj().GetStartInParam();	///< ServicePlug初始化参数
	WriteInfo( "EchoNodeEngine::Initialize() : [Info] Engine is initializing ......" );

	Release();
	m_sCode = sCode;
	m_nMessageID = nMsgID;
	if( 0 != nErrorCode )	{
		WriteWarning( "EchoNodeEngine::Initialize() : invalid configuration file, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oDataCollector.Initialize( this )) )										///< 初始化行情插件
	{
		WriteError( "EchoNodeEngine::Initialize() : Failed 2 Initialize Data Collector Plugin, ErrorCode=%d", nErrorCode );
		return nErrorCode;
	}

	///< ...... 准备做编/解码测试 .......
	if( m_nMessageID <= 0 )
	{
		if( 0 != (nErrorCode = m_oSendPackage.Initialize()) )
		{
			WriteError( "SessionCollection::Instance() : failed 2 initialize ..., errorcode=%d", nErrorCode );
			return nErrorCode;
		}
	}

	WriteInfo( "EchoNodeEngine::Initialize() : [Okey] Engine is initialized ......" );

	return nErrorCode;
}

bool EchoNodeEngine::EchoQuotation()
{
	int				nErrorCode = 0;																		///< 错误码
	WriteInfo( "EchoNodeEngine::EchoQuotation() : [Info] Service Is Recovering ......" );

	if( 0 != (nErrorCode=m_oDataCollector.RecoverDataCollector()) )
	{
		WriteWarning( "DataIOEngine::EchoQuotation() : [Error] Failed 2 Initialize Data Collector, errorcode=%d", nErrorCode );
		return false;
	}

	WriteInfo( "DataIOEngine::EchoQuotation() : [Okey] MarketID(%u), Service Is Recovered! ......", DataCollector::GetMarketID() );

	while( true )
	{
		SimpleTask::Sleep( 1000 );
	}

	return true;
}

int EchoNodeEngine::OnQuery( unsigned int nDataID, const char* pData, unsigned int nDataLen )
{
	return 1;
}

int EchoNodeEngine::OnImage( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag )
{
	if( 0 >= m_nMessageID )
	{
		static	unsigned __int64	s_nSeq = 0;
		int							nErrorCode = 0;
		tagMsgInfo&					refMsgInfo = s_vctEncodeMsgInfo[nDataID];

		refMsgInfo.nMsgCount++;
		if( refMsgInfo.nMsgLen == 0 )
		{
			refMsgInfo.nMsgLen = nDataLen;
		}

		if( 0 != (nErrorCode = m_oSendPackage.DispatchMessage( nDataID, pData, nDataLen, s_nSeq++, false )) )
		{
			WriteError( "EchoNodeEngine::OnImage() : failed 2 dispatch message 2 buffer, errorcode=%d", nErrorCode );
			return nErrorCode;
		}
	}

	return 0;
}

int EchoNodeEngine::OnStream( unsigned int nDataID, const char* pData, unsigned int nDataLen )
{
	return 0;
}

int EchoNodeEngine::OnData( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag, bool bPushFlag )
{
	if( 0 >= m_nMessageID )
	{
		static	unsigned __int64	s_nSeq = 0;
		int							nErrorCode = 0;
		tagMsgInfo&					refMsgInfo = s_vctEncodeMsgInfo[nDataID];

		refMsgInfo.nMsgCount++;
		if( refMsgInfo.nMsgLen == 0 )
		{
			refMsgInfo.nMsgLen = nDataLen;
		}

		if( 0 != (nErrorCode = m_oSendPackage.DispatchMessage( nDataID, pData, nDataLen, s_nSeq++, bPushFlag )) )
		{
			WriteError( "EchoNodeEngine::OnData() : failed 2 dispatch message 2 buffer, errorcode=%d", nErrorCode );
			return nErrorCode;
		}
	}
	else if( m_nMessageID == nDataID )
	{
		char			pszEcho[1024] = { 0 };
		std::string		sCode = pData;
		unsigned int	nMarketID = DataCollector::GetMarketID();

		if( m_sCode != sCode )
		{
			return 0;
		}

		return PrintMessage( nMarketID, nDataID, pData );
	}

	return 0;
}

int EchoNodeEngine::PrintMessage( unsigned int nMarketID, unsigned int nDataID, const char* pData )
{
	char			pszEcho[1024] = { 0 };

	switch( nMarketID )
	{
	case QUO_MARKET_DCE:
		DLFuture_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_DCEOPT:
		DLOption_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_SHFE:
		SHFuture_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_SHFEOPT:
		SHOption_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_CZCE:
		ZZFuture_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_CZCEOPT:
		ZZOption_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_CFFEX:
		CFFFuture_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_SSE:
		SHL1_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_SSEOPT:
		SHL1Option_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	case QUO_MARKET_SZSE:
		SZL1_Echo::FormatStruct2OutputBuffer( pszEcho, nDataID, pData );
		break;
	default:
		::sprintf( pszEcho, "%s\n", "Unknow Market ID" );
		break;
	}

	::printf( "%s", pszEcho );

	return 0;
}

void EchoNodeEngine::OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
{
    va_list		valist;
    char		pszLogBuf[8000] = { 0 };

    va_start( valist, pszFormat );
    _vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
    va_end( valist );

	switch( nLogLevel )	///< 日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
	{
	case 0:
		::printf( "[Plugin] %s", pszLogBuf );
		break;
	case 1:
		::printf( "[Plugin] %s", pszLogBuf );
		break;
	case 2:
		::printf( "[Plugin] %s", pszLogBuf );
		break;
	case 3:
		::printf( "[Plugin] %s", pszLogBuf );
		break;
	default:
		::printf( "[Plugin] {Unknow Level} [%d] --> %s \n", nLogLevel, pszLogBuf );
		break;
	}
}

void EchoNodeEngine::WriteInfo( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	::printf( "%s\n", tempbuf );
}

void EchoNodeEngine::WriteWarning( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	::printf( "%s\n", tempbuf );
}

void EchoNodeEngine::WriteError( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	::printf( "%s\n", tempbuf );
}

void EchoNodeEngine::WriteDetail( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	::printf( "%s\n", tempbuf );
}






