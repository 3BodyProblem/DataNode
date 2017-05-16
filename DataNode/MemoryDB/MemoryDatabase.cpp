#include <stdio.h>
#include "MemoryDatabase.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


typedef IDBFactory& __stdcall		TFunc_GetFactoryObject();


DatabaseIO::DatabaseIO()
: m_pIDBFactoryPtr( NULL ), m_pIDatabase( NULL ), m_bBuilded( false )
{
}

DatabaseIO::~DatabaseIO()
{
	Release();
}

bool DatabaseIO::IsBuilded()
{
	return m_bBuilded;
}

unsigned int DatabaseIO::GetTablesID( unsigned int* pIDList, unsigned int nMaxListSize )
{
	unsigned int			nIndex = 0;
	unsigned int*			pIDListPtr = pIDList;
	CriticalLock			lock( m_oLock );

	for( std::set<unsigned int>::iterator it = m_setTableID.begin(); it != m_setTableID.end() && nIndex < nMaxListSize; it++ )
	{
		*(pIDListPtr+nIndex) = *it;
		nIndex++;
	}

	return nIndex;
}

int DatabaseIO::FetchDataBlockByID( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo )
{
	unsigned __int64		nTmpVal = 0;
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	nSerialNo = 0;
	if( NULL == pBuffer )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::FetchDataBlockByID() : invalid buffer pointer(NULL)" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::FetchDataBlockByID() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	nTmpVal = m_pIDatabase->GetUpdateSequence();
	nAffectNum = pTable->CopyToBuffer( pBuffer, nBufferSize );
	if( nAffectNum < 0 )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::FetchDataBlockByID() : failed 2 copy data from table, errorcode = %d", nAffectNum );
		return -3;
	}

	nSerialNo = nTmpVal;
	return nAffectNum;
}

int DatabaseIO::BuildMessageTable( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo )
{
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	nDbSerialNo = 0;
	m_bBuilded = bLastFlag;
	if( false == m_pIDatabase->CreateTable( nDataID, nDataLen, MAX_CODE_LENGTH ) )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 create data table 4 message, message id=%d", nDataID );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->InsertRecord( pData, nDataLen, nDbSerialNo )) )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	m_setTableID.insert( nDataID );		///< ���ݱ�ID���ϣ����

	return 0;
}

int DatabaseIO::UpdateQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( false == m_bBuilded )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 update quotation before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->UpdateRecord( pData, nDataLen, nDbSerialNo )) )
	{
		SvrFramework::GetFramework().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	return 0;
}

int DatabaseIO::RecoverDatabase()
{
	try
	{
		if( m_pIDatabase )
		{
			SvrFramework::GetFramework().WriteInfo( "DatabaseIO::RecoverDatabase() : recovering ......" );

			if( true == m_pIDatabase->LoadFromDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() ) )
			{
				SvrFramework::GetFramework().WriteInfo( "DatabaseIO::RecoverDatabase() : recovered ......" );
				return 0;
			}
			else
			{
				SvrFramework::GetFramework().WriteWarning( "DatabaseIO::RecoverDatabase() : failed 2 recover quotation data ......" );
				return -1;
			}
		}

		return 0;
	}
	catch( std::exception& err )
	{
		SvrFramework::GetFramework().WriteWarning( "DatabaseIO::BackupDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		SvrFramework::GetFramework().WriteWarning( "DatabaseIO::BackupDatabase() : unknow exception" );
	}

	return -1;
}

int DatabaseIO::BackupDatabase()
{
	try
	{
		if( m_pIDatabase )
		{
			SvrFramework::GetFramework().WriteInfo( "DatabaseIO::BackupDatabase() : making backup ......" );

			if( true == m_pIDatabase->SaveToDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() ) )
			{
				SvrFramework::GetFramework().WriteInfo( "DatabaseIO::BackupDatabase() : backup completed ......" );
				return 0;
			}
			else
			{
				SvrFramework::GetFramework().WriteWarning( "DatabaseIO::BackupDatabase() : miss backup ......" );
				return -2;
			}
		}
	}
	catch( std::exception& err )
	{
		SvrFramework::GetFramework().WriteWarning( "DatabaseIO::BackupDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		SvrFramework::GetFramework().WriteWarning( "DatabaseIO::BackupDatabase() : unknow exception" );
	}

	return -1;
}

int DatabaseIO::Initialize()
{
	Release();

	SvrFramework::GetFramework().WriteInfo( "DatabaseIO::Initialize() : initializing memory database plugin ......" );

	TFunc_GetFactoryObject*	m_funcFactory = NULL;
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

		m_setTableID.clear();				///< ������ݱ�ID����
		BackupDatabase();					///< �����������ݵ�����
		m_pIDatabase->DeleteTables();		///< �����ڴ����е����ݱ�
		m_pIDatabase = NULL;				///< �����ڴ������ݿ�ָ��

		if( m_pIDBFactoryPtr )				///< �������ڲ���е��������ݿ�
		{
			m_pIDBFactoryPtr->ReleaseAllDatabase();
			m_pIDBFactoryPtr = NULL;		///< �����ڴ����Ĺ�������ָ��
		}

		m_oDllPlugin.CloseDll();			///< ж���ڴ������DLL

		SvrFramework::GetFramework().WriteInfo( "DatabaseIO::Release() : memory database plugin is released ......" );
	}
}












