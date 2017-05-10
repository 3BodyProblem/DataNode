#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "NodeServer.h"


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
	const tagServicePlug_StartInParam&	refStartInParam = Configuration::GetConfigObj().GetStartInParam();
	int									nErrorCode = MServicePlug::Instance( &refStartInParam );

	if( nErrorCode < 0 )
	{
		::printf( "SvrFramework::Initialize() : failed 2 initialize serviceIO framework, errorcode=%d", nErrorCode );
		return -1;
	}

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

	if( 0 != (nErrorCode = m_oInitFlag.Initialize( Configuration::GetConfigObj().GetTradingPeriods()
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

int DataEngine::Execute()
{
	SvrFramework::GetFramework().WriteInfo( "DataEngine::Execute() : enter into thread func ..." );

	int					nErrorCode = 0;

	while( true == IsAlive() )
	{
		try
		{
			SimpleTask::Sleep( 1000 );

			if( true == m_oInitFlag.GetFlag() )
			{
				SvrFramework::GetFramework().WriteInfo( "InitializerFlag::Execute() : [NOTICE] Enter Service Initializing Time ......" );

				if( 0 != (nErrorCode=m_oDatabaseIO.RecoverDatabase()) )
				{
					SvrFramework::GetFramework().WriteWarning( "DataEngine::Execute() : failed 2 recover quotations data from disk ..., errorcode=%d", nErrorCode );
					m_oInitFlag.RedoInitialize();
					continue;
				}

				if( 0 != (nErrorCode=m_oDataCollector.ReInitializeDriver()) )
				{
					SvrFramework::GetFramework().WriteWarning( "DataEngine::Execute() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
					m_oInitFlag.RedoInitialize();
					continue;
				}

				SvrFramework::GetFramework().WriteInfo( "InitializerFlag::Execute() : [NOTICE] Service is Initialized ......" );
			}
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

	return -1;
}

int DataEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen )
{

	return -1;
}

bool DataEngine::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
{

	return true;
}


///< ----------------------------------------------------------------------------


int DataNodeService::Activate()
{
	try
	{
		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Activate() : activating service..............\n" );

		int			nErrorCode = Configuration::GetConfigObj().Load();	///< ¼ÓÔØÅäÖÃÐÅÏ¢

		if( 0 != nErrorCode )
		{
			SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : invalid configuration file, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		nErrorCode = DataEngine::Initialize( Configuration::GetConfigObj().GetDataCollectorPluginPath()
											, Configuration::GetConfigObj().GetMemPluginPath()
											, Configuration::GetConfigObj().GetHolidayFilePath() );
		if( 0 != nErrorCode )
		{
			SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : failed 2 initialize service engine, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Activate() : service activated..............\n" );

		return 0;
	}
	catch( std::exception& err )
	{
		SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : exception : %s\n", err.what() );
	}
	catch( ... )
	{
		SvrFramework::GetFramework().WriteWarning( "DataNodeService::Activate() : unknow exception\n" );
	}

	return -100;
}

void DataNodeService::Destroy()
{
	try
	{
		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Destroy() : destroying service..............\n" );

		DataEngine::Release();

		SvrFramework::GetFramework().WriteInfo( "DataNodeService::Destroy() : service destroyed ..............\n" );
	}
	catch( std::exception& err )
	{
		::printf( "DataNodeService::Destroy() : exception : %s\n", err.what() );
	}
	catch( ... )
	{
		::printf( "DataNodeService::Destroy() : unknow exception\n" );
	}
}

int DataNodeService::OnIdle()
{
	return -1;
}

unsigned long DataNodeService::VersionOfEngine()
{
	return 123;
}

bool DataNodeService::IsAvailable()
{
	return true;
}




