#include <time.h>
#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "NodeServer.h"


DataIOEngine::DataIOEngine()
 : SimpleTask( "DataIOEngine::Thread" )
{
}

int DataIOEngine::Initialize( const std::string& sDataCollectorPluginPath, const std::string& sMemPluginPath, const std::string& sHolidayPath )
{
	int			nErrorCode = 0;

	Release();
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initializing ......" );

	if( 0 != (nErrorCode = m_oInitFlag.Initialize( Configuration::GetConfigObj().GetTradingPeriods()
													, sHolidayPath, Configuration::GetConfigObj().GetTestFlag())) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize initialize policy flag, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oDatabaseIO.Initialize()) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oDataCollector.Initialize( this )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize data collector plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = SimpleTask::Activate()) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize task thread, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initialized ......" );

	return nErrorCode;
}

void DataIOEngine::Release()
{
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Release() : DataNode Engine is releasing ......" );

	m_oDataCollector.Release();
	m_oDatabaseIO.Release();
	SimpleTask::StopThread();

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Release() : DataNode Engine is released ......" );
}

int DataIOEngine::Execute()
{
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : enter into thread func ..." );

	int			nErrorCode = 0;

	while( true == IsAlive() )
	{
		try
		{
			///< 初始化业务顺序的逻辑
			if( true == m_oInitFlag.GetFlag() )
			{
				SimpleTask::Sleep( 1000 );
				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : [NOTICE] Enter Service Initializing Time ......" );

				if( 0 != (nErrorCode=m_oDatabaseIO.RecoverDatabase()) )
				{
					DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : failed 2 recover quotations data from disk ..., errorcode=%d", nErrorCode );
					m_oInitFlag.RedoInitialize();
					continue;
				}

				if( 0 != (nErrorCode=m_oDataCollector.RecoverDataCollector()) )
				{
					DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
					m_oInitFlag.RedoInitialize();
					continue;
				}

				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : ................. [NOTICE] Service is Available ....................." );
				continue;
			}

			///< 空闲处理函数
			OnIdle();
		}
		catch( std::exception& err )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : exception : %s", err.what() );
		}
		catch( ... )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : unknow exception" );
		}
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : exit thread func ..." );

	return 0;
}

int DataIOEngine::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	unsigned __int64		nSerialNo = 0;
	static	const char		s_pszZeroBuff[32] = { 0 };

	if( 0 == strncmp( pData, s_pszZeroBuff, sizeof(s_pszZeroBuff) ) )
	{
		return m_oDatabaseIO.FetchRecordsByID( nDataID, pData, nDataLen, nSerialNo );
	}
	else
	{
		return m_oDatabaseIO.QueryQuotation( nDataID, pData, nDataLen, nSerialNo );
	}
}

int DataIOEngine::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	unsigned __int64	nSerialNo = 0;

	return m_oDatabaseIO.BuildMessageTable( nDataID, pData, nDataLen, bLastFlag, nSerialNo );
}

int DataIOEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	unsigned __int64	nSerialNo = 0;
	int					nErrorCode = m_oDatabaseIO.UpdateQuotation( nDataID, pData, nDataLen, nSerialNo );

	if( 0 != nErrorCode )
	{
		return nErrorCode;
	}

	m_oLinkSessions.PushData( nDataID, 0, pData, nDataLen, bPushFlag, nSerialNo );

	return nErrorCode;
}

void DataIOEngine::OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
{
    va_list		valist;
    char		pszLogBuf[8000] = { 0 };

    va_start( valist, pszFormat );
    _vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
    va_end( valist );

	switch( nLogLevel )	///< 日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
	{
	case 0:
		MServicePlug::WriteInfo( "[Plugin] %s", pszLogBuf );
		break;
	case 1:
		MServicePlug::WriteWarning( "[Plugin] %s", pszLogBuf );
		break;
	case 2:
		MServicePlug::WriteError( "[Plugin] %s", pszLogBuf );
		break;
	case 3:
		MServicePlug::WriteDetail( "[Plugin] %s", pszLogBuf );
		break;
	default:
		::printf( "[Plugin] unknow log level [%d] \n", nLogLevel );
		break;
	}
}


///< ----------------------------------------------------------------------------


DataNodeService::DataNodeService()
{
}

DataNodeService::~DataNodeService()
{
	Destroy();
}

DataNodeService& DataNodeService::GetSerivceObj()
{
	static DataNodeService	obj;

	return obj;
}

int DataNodeService::Activate()
{
	try
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::Activate() : activating service.............." );

		static	char						pszErrorDesc[8192] = { 0 };
		int									nErrorCode = Configuration::GetConfigObj().Load();	///< 加载配置信息
		const tagServicePlug_StartInParam&	refStartInParam = Configuration::GetConfigObj().GetStartInParam();

		if( 0 != nErrorCode )	{
			DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : invalid configuration file, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		if( (nErrorCode=MServicePlug::Instance( &refStartInParam, pszErrorDesc, sizeof(pszErrorDesc) )) < 0 )	{					///< 初始化服务框架
			::printf( "DataNodeService::Activate() : failed 2 initialize serviceIO framework, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		if( 0 != (nErrorCode=m_oLinkSessions.Instance()) )	{									///< 初始化会话链路管理
			::printf( "DataNodeService::Activate() : failed 2 initialize link session set, errorcode=%d", nErrorCode );
			return -2;
		}

		MServicePlug::WriteInfo( "DataNodeService::Activate() : serviceIO framework initialized!" );
		MServicePlug::WriteInfo( "DataNodeService::Activate() : Configuration As Follow:\n\
								 MaxLinkCount:%d\nListenPort:%d\nListenCount:%d\nSendBufCount:%d\nThreadCount:%d\nSendTryTimes:%d\n\
								 LinkTimeOut:%d\nCompressFlag:%d\nSSLFlag:%d\nPfxFilePasswrod:%s\nDetailLog:%d\nPageSize:%d\nPageCount:%d"
								, refStartInParam.uiMaxLinkCount, refStartInParam.uiListenPort, refStartInParam.uiListenCount
								, refStartInParam.uiSendBufCount, refStartInParam.uiThreadCount, refStartInParam.uiSendTryTimes
								, refStartInParam.uiLinkTimeOut, refStartInParam.bCompress, refStartInParam.bSSL, refStartInParam.szPfxFilePasswrod
								, refStartInParam.bDetailLog, refStartInParam.uiPageSize, refStartInParam.uiPageCount );

		///< ........................ 开始启动本节点引擎 .............................
		if( 0 != (nErrorCode = DataIOEngine::Initialize( Configuration::GetConfigObj().GetDataCollectorPluginPath()
													, Configuration::GetConfigObj().GetMemPluginPath()
													, Configuration::GetConfigObj().GetHolidayFilePath() )) )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : failed 2 initialize service engine, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		MServicePlug::RegisterSpi( &m_oLinkSessions );											///< 注册服务框架的回调对象
		DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::Activate() : service activated.............." );

		return 0;
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : exception : %s\n", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : unknow exception" );
	}

	return -100;
}

void DataNodeService::Destroy()
{
	try
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::Destroy() : destroying service.............." );

		DataIOEngine::Release();
		DataNodeService::GetSerivceObj().Release();

		DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::Destroy() : service destroyed .............." );
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Destroy() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Destroy() : unknow exception" );
	}
}

bool DataNodeService::IsServiceAlive()
{
	if( true == SimpleThread::IsAlive() && false == MServicePlug::IsStop() )
	{
		return true;
	}
	else
	{
		return false;
	}
}

int DataNodeService::OnIdle()
{
	bool			bInitPoint = false;

	if( 0 == ImageRebuilder::GetRebuilder().GetReqSessionCount() )
	{
		SimpleTask::Sleep( 1000 );
	}
	else
	{
	///< 检查是否有新的链接到来请求初始化行情数据推送的
		ImageRebuilder::GetRebuilder().Flush2ReqSessions( m_oDatabaseIO, 0 );
	}

	///< 在非交易时段，进行内存插件中的行情数据落盘
	if( -1 == m_oInitFlag.InTradingPeriod( bInitPoint ) && true == m_oDatabaseIO.IsBuilded() )	{
		OnBackupDatabase();
	}

	return 0;
}

bool DataNodeService::OnInquireStatus()
{
	bool				bDataBuilded = m_oDatabaseIO.IsBuilded();
	enum E_SS_Status	eStatus = m_oDataCollector.InquireDataCollectorStatus();

	if( ET_SS_WORKING == eStatus && true == bDataBuilded )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DataNodeService::OnBackupDatabase()
{
	static time_t	nLastTimeT = ::time( NULL );
	time_t			nTimeNowT = ::time( NULL );

	///< 每十分钟落盘一次
	if( nTimeNowT - nLastTimeT < 60 * 10 )
	{
		return;
	}

	///< 开始落盘备份操作
	if( 0 != m_oDatabaseIO.BackupDatabase() )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::BackupDatabase() : failed 2 backup quotation data" );
		return;
	}

	nLastTimeT = nTimeNowT;
}






