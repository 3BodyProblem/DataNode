#include <time.h>
#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "NodeServer.h"
#include "Communication/LinkSession.h"


SvrFramework::SvrFramework()
{
}

SvrFramework::~SvrFramework()
{
	MServicePlug::Release();
}

SvrFramework& SvrFramework::GetFramework()
{
	static SvrFramework		objFramework;

	return objFramework;
}

int SvrFramework::Initialize()
{
	int									nErrorCode = 0;
	const tagServicePlug_StartInParam&	refStartInParam = Configuration::GetConfigObj().GetStartInParam();
	
	if( (nErrorCode=MServicePlug::Instance( &refStartInParam )) < 0 )	{			///< 初始化服务框架
		::printf( "SvrFramework::Initialize() : failed 2 initialize serviceIO framework, errorcode=%d", nErrorCode );
		return -1;
	}

	if( 0 != (nErrorCode=LinkSessionSet::GetSessionSet().Instance()) )	{			///< 初始化会话链路管理
		::printf( "SvrFramework::Initialize() : failed 2 initialize link session set, errorcode=%d", nErrorCode );
		return -2;
	}

	MServicePlug::RegisterSpi( &(LinkSessionSet::GetSessionSet()) );				///< 注册服务框架的回调对象

	MServicePlug::WriteInfo( "SvrFramework::Initialize() : serviceIO framework initialized!" );
	MServicePlug::WriteInfo( "SvrFramework::Initialize() : Configuration As Follow:\n\
							 MaxLinkCount:%d\nListenPort:%d\nListenCount:%d\nSendBufCount:%d\nThreadCount:%d\nSendTryTimes:%d\n\
							 LinkTimeOut:%d\nCompressFlag:%d\nSSLFlag:%d\nPfxFilePasswrod:%s\nDetailLog:%d\nPageSize:%d\nPageCount:%d"
							, refStartInParam.uiMaxLinkCount, refStartInParam.uiListenPort, refStartInParam.uiListenCount
							, refStartInParam.uiSendBufCount, refStartInParam.uiThreadCount, refStartInParam.uiSendTryTimes
							, refStartInParam.uiLinkTimeOut, refStartInParam.bCompress, refStartInParam.bSSL, refStartInParam.szPfxFilePasswrod
							, refStartInParam.bDetailLog, refStartInParam.uiPageSize, refStartInParam.uiPageCount );

	return 0;
}


DataEngine::DataEngine()
 : SimpleTask( "DataEngine::Thread" )
{
}

int DataEngine::Initialize( const std::string& sDataCollectorPluginPath, const std::string& sMemPluginPath, const std::string& sHolidayPath )
{
	int			nErrorCode = 0;

	Release();
	SvrFramework::GetFramework().WriteInfo( "DataEngine::Initialize() : DataNode Engine is initializing ......" );

	if( 0 != (nErrorCode = InitializerFlag::GetFlagObject().Initialize( Configuration::GetConfigObj().GetTradingPeriods()
																		, sHolidayPath, Configuration::GetConfigObj().GetTestFlag())) )
	{
		SvrFramework::GetFramework().WriteError( "DataEngine::Initialize() : failed 2 initialize initialize policy flag, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oDatabaseIO.Initialize()) )
	{
		SvrFramework::GetFramework().WriteError( "DataEngine::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oDataCollector.Initialize( this )) )
	{
		SvrFramework::GetFramework().WriteError( "DataEngine::Initialize() : failed 2 initialize data collector plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = SimpleTask::Activate()) )
	{
		SvrFramework::GetFramework().WriteError( "DataEngine::Initialize() : failed 2 initialize task thread, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	SvrFramework::GetFramework().WriteInfo( "DataEngine::Initialize() : DataNode Engine is initialized ......" );

	return nErrorCode;
}

void DataEngine::Release()
{
	SvrFramework::GetFramework().WriteInfo( "DataEngine::Release() : DataNode Engine is releasing ......" );

	m_oDataCollector.Release();
	m_oDatabaseIO.Release();
	SimpleTask::StopThread();

	SvrFramework::GetFramework().WriteInfo( "DataEngine::Release() : DataNode Engine is released ......" );
}

DatabaseIO& DataEngine::GetDatabaseIO()
{
	return m_oDatabaseIO;
}

int DataEngine::Execute()
{
	SvrFramework::GetFramework().WriteInfo( "DataEngine::Execute() : enter into thread func ..." );

	int			nErrorCode = 0;

	while( true == IsAlive() )
	{
		try
		{
			SimpleTask::Sleep( 1000 );

			///< 初始化业务顺序的逻辑
			if( true == InitializerFlag::GetFlagObject().GetFlag() )
			{
				SvrFramework::GetFramework().WriteInfo( "InitializerFlag::Execute() : [NOTICE] Enter Service Initializing Time ......" );

				if( 0 != (nErrorCode=m_oDatabaseIO.RecoverDatabase()) )
				{
					SvrFramework::GetFramework().WriteWarning( "DataEngine::Execute() : failed 2 recover quotations data from disk ..., errorcode=%d", nErrorCode );
					InitializerFlag::GetFlagObject().RedoInitialize();
					continue;
				}

				if( 0 != (nErrorCode=m_oDataCollector.ReInitializeDataCollector()) )
				{
					SvrFramework::GetFramework().WriteWarning( "DataEngine::Execute() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
					InitializerFlag::GetFlagObject().RedoInitialize();
					continue;
				}

				SvrFramework::GetFramework().WriteInfo( "InitializerFlag::Execute() : [NOTICE] Service is Initialized ......" );
				continue;
			}

			///< 空闲处理函数
			OnIdle();
		}
		catch( std::exception& err )
		{
			SvrFramework::GetFramework().WriteWarning( "DataEngine::Execute() : exception : %s", err.what() );
		}
		catch( ... )
		{
			SvrFramework::GetFramework().WriteWarning( "DataEngine::Execute() : unknow exception" );
		}
	}

	SvrFramework::GetFramework().WriteInfo( "DataEngine::Execute() : exit thread func ..." );

	return 0;
}

int DataEngine::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	return m_oDatabaseIO.BuildMessageTable( nDataID, pData, nDataLen, bLastFlag );
}

int DataEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	int		nErrorCode = m_oDatabaseIO.UpdateQuotation( nDataID, pData, nDataLen );

	if( 0 != nErrorCode )
	{
		return nErrorCode;
	}

	LinkSessionSet::GetSessionSet().PushData( nDataID, 0, pData, nDataLen, bPushFlag );

	return nErrorCode;
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
		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Activate() : activating service.............." );

		int			nErrorCode = Configuration::GetConfigObj().Load();		///< 加载配置信息

		if( 0 != nErrorCode )
		{
			SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : invalid configuration file, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		if( 0 != (nErrorCode = SvrFramework::GetFramework().Initialize()) )	///< 使用配置信息启动服务框架插件
		{
			SvrFramework::GetFramework().WriteWarning( "Configuration::Load() : failed 2 initialize server framework, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		///< ........................ 开始启动本节点引擎 .............................
		if( 0 != (nErrorCode = DataEngine::Initialize( Configuration::GetConfigObj().GetDataCollectorPluginPath()
													, Configuration::GetConfigObj().GetMemPluginPath()
													, Configuration::GetConfigObj().GetHolidayFilePath() )) )
		{
			SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : failed 2 initialize service engine, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Activate() : service activated.............." );

		return 0;
	}
	catch( std::exception& err )
	{
		SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : exception : %s\n", err.what() );
	}
	catch( ... )
	{
		SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : unknow exception" );
	}

	return -100;
}

void DataNodeService::Destroy()
{
	try
	{
		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Destroy() : destroying service.............." );

		DataEngine::Release();
		SvrFramework::GetFramework().Release();

		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Destroy() : service destroyed .............." );
	}
	catch( std::exception& err )
	{
		SvrFramework::GetFramework().WriteWarning( "DataNodeService::Destroy() : exception : %s", err.what() );
	}
	catch( ... )
	{
		SvrFramework::GetFramework().WriteWarning( "DataNodeService::Destroy() : unknow exception" );
	}
}

int DataNodeService::OnIdle()
{
	bool			bInitPoint = false;

	///< 在非交易时段，进行内存插件中的行情数据落盘
	if( -1 == InitializerFlag::GetFlagObject().InTradingPeriod( bInitPoint ) && true == m_oDatabaseIO.IsBuilded() )	{
		OnBackupDatabase();
	}

	///< 轮询行情采集模块的状态
	OnInquireStatus();

	return 0;
}

void DataNodeService::OnInquireStatus()
{
	const CollectorStatus&	refDataCollectorStatus = m_oDataCollector.InquireDataCollectorStatus();
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
		SvrFramework::GetFramework().WriteWarning( "DataNodeService::BackupDatabase() : failed 2 backup quotation data" );
		return;
	}

	nLastTimeT = nTimeNowT;
}

bool DataNodeService::IsAvailable()
{
	return m_oDatabaseIO.IsBuilded();
}




