#include "EchoDataNode.h"
#include "../DataServer/DataEcho.h"
#include "../../../../DataCluster/DataCluster/Protocal/DataCluster_Protocal.h"


EchoNodeEngine::EchoNodeEngine()
 : m_nMessageID( 0 )
{
}

void EchoNodeEngine::Release()
{
	m_sCode = "";
	m_nMessageID = 0;
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

	if( 0 != (nErrorCode = m_oDataCollector.Initialize( this )) )									///< 初始化行情插件
	{
		WriteError( "EchoNodeEngine::Initialize() : Failed 2 Initialize Data Collector Plugin, ErrorCode=%d", nErrorCode );
		return nErrorCode;
	}

	WriteInfo( "EchoNodeEngine::Initialize() : [Okey] Engine is initialized ......" );

	return nErrorCode;
}

bool EchoNodeEngine::EchoQuotation()
{
	int				nErrorCode = 0;												///< 错误码
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

int EchoNodeEngine::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	return 0;
}

int EchoNodeEngine::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	if( 0 >= m_nMessageID )
	{
		static	bool	s_bTitle = false;

		if( false == s_bTitle )
		{
			::printf( "%s\n", "Image Message ID, Message Length" );
			s_bTitle = true;
		}
		else
		{
			::printf( "%u,%u\n", nDataID, nDataLen );
		}
	}

	return 0;
}

int EchoNodeEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	if( 0 >= m_nMessageID )
	{
		static	bool	s_bTitle = false;

		if( false == s_bTitle )
		{
			::printf( "%s\n", "Data Message ID, Message Length" );
			s_bTitle = true;
		}
		else
		{
			::printf( "%u,%u\n", nDataID, nDataLen );
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
			return -1;
		}

		::printf( "%s", pszEcho );
	}

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






