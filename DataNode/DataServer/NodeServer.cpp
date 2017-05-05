#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "NodeServer.h"


DataEngine::DataEngine()
	: SimpleTask( "DataEngine:Thread" )
{
}

int DataEngine::Initialize( const std::string& sDataDriverPluginPath, const std::string& sMemPluginPath, const std::string& sHolidayPath )
{
	return Activate();
}

void DataEngine::Release()
{

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
	return -1;
}

int DataNodeService::Destroy()
{
	return -1;
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




