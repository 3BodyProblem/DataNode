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
	while( true == IsAlive() )
	{
		try
		{
			SimpleTask::Sleep( 1000 );


		}
		catch( std::exception& err )
		{
			::printf( "DataEngine::Execute() : exception : %s\n", err.what() );
		}
		catch( ... )
		{
			::printf( "DataEngine::Execute() : unknow exception\n" );
		}
	}

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
		::printf( "DataNodeService::Activate() : activating service..............\n" );

		int			nErrorCode = Configuration::GetConfigObj().Load();	///< ¼ÓÔØÅäÖÃÐÅÏ¢

		if( 0 != nErrorCode )
		{
			::printf( "DataNodeService::Activate() : invalid configuration file, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		nErrorCode = DataEngine::Initialize( Configuration::GetConfigObj().GetDataCollectorPluginPath()
											, Configuration::GetConfigObj().GetMemPluginPath()
											, Configuration::GetConfigObj().GetHolidayFilePath() );
		if( 0 != nErrorCode )
		{
			::printf( "DataNodeService::Activate() : failed 2 initialize service engine, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		::printf( "DataNodeService::Activate() : service activated..............\n" );

		return 0;
	}
	catch( std::exception& err )
	{
		::printf( "DataNodeService::Activate() : exception : %s\n", err.what() );
	}
	catch( ... )
	{
		::printf( "DataNodeService::Activate() : unknow exception\n" );
	}

	return -100;
}

void DataNodeService::Destroy()
{
	try
	{
		DataEngine::Release();
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




