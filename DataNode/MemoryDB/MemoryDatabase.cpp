#include <stdio.h>
#include "MemoryDatabase.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


typedef IDBFactory& __stdcall		TFunc_GetFactoryObject();


DatabaseIO::DatabaseIO()
: m_pIDBFactoryPtr( NULL ), m_pIDatabase( NULL )
{
}

DatabaseIO::~DatabaseIO()
{
	Release();
}

int DatabaseIO::Initialize()
{
	Release();

	SvrFramework::GetFramework().WriteInfo( "DatabaseIO::Initialize() : initializing memory database plugin ......" );

	TFunc_GetFactoryObject*	m_funcFactory = NULL;
	std::string				sPluginPath = GetModulePath( NULL ) + "srvunit\\DataNode\\" + "MemDatabase.dll";
	int						nErrorCode = m_oDllPlugin.LoadDll( Configuration::GetConfigObj().GetMemPluginPath() );

	if( 0 != nErrorCode )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::Initialize() : failed 2 load memoryplugin module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_funcFactory = (TFunc_GetFactoryObject*)m_oDllPlugin.GetDllFunction( "GetFactoryObject" );
	m_pIDBFactoryPtr = &(m_funcFactory());
	m_pIDatabase = m_pIDBFactoryPtr->GrapDatabaseInterface();
	if( NULL == m_pIDatabase )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::Initialize() : invalid database interface pointer(NULL)" );
		Release();
		return -100;
	}

	SvrFramework::GetFramework().WriteInfo( "DatabaseIO::Initialize() : memory database plugin is initialized ......" );

	return 0;
}

void DatabaseIO::Release()
{
	if( NULL != m_pIDatabase || NULL != m_pIDBFactoryPtr )
	{
		SvrFramework::GetFramework().WriteInfo( "DatabaseIO::Release() : releasing memory database plugin ......" );

		if( m_pIDatabase )
		{
			m_pIDatabase->SaveToDisk( "./path" );
			m_pIDatabase = NULL;
		}

		if( m_pIDBFactoryPtr )
		{
			m_pIDBFactoryPtr->ReleaseAllDatabase();
			m_pIDBFactoryPtr = NULL;
		}

		m_oDllPlugin.CloseDll();

		SvrFramework::GetFramework().WriteInfo( "DatabaseIO::Release() : memory database plugin is released ......" );
	}
}












