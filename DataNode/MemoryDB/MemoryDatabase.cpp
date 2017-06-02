#include <time.h>
#include <stdio.h>
#include "MemoryDatabase.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


typedef IDBFactory& __stdcall		TFunc_GetFactoryObject();
typedef void						(__stdcall *T_Func_DBUnitTest)();


DatabaseIO::DatabaseIO()
: m_pIDBFactoryPtr( NULL ), m_pIDatabase( NULL ), m_bBuilded( false )
{
	m_nUpdateTimeT = ::time( NULL );
}

DatabaseIO::~DatabaseIO()
{
	Release();
}

bool DatabaseIO::IsBuilded()
{
	return m_bBuilded;
}

time_t DatabaseIO::GetLastUpdateTime()
{
	return m_nUpdateTimeT;
}

unsigned int DatabaseIO::GetTableCount()
{
	if( false == IsBuilded() )
	{
		return 0;
	}

	return m_mapTableID.size();
}

unsigned int DatabaseIO::GetTablesID( unsigned int* pIDList, unsigned int nMaxListSize, unsigned int* pWidthList, unsigned int nMaxWidthlistLen )
{
	unsigned int			nIndex = 0;
	unsigned int*			pIDListPtr = pIDList;
	CriticalLock			lock( m_oLock );

	for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end() && nIndex < nMaxListSize; it++ )
	{
		*(pIDListPtr+nIndex) = it->first;

		if( NULL != pWidthList && 0 != nMaxWidthlistLen )
		{
			*(pWidthList+nIndex) = it->second;
		}

		nIndex++;
	}

	return nIndex;
}

int DatabaseIO::FetchRecordsByID( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo )
{
	unsigned __int64		nTmpVal = 0;
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	if( NULL == pBuffer )
	{
		nSerialNo = 0;
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::FetchRecordsByID() : invalid buffer pointer(NULL)" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		nSerialNo = 0;
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::FetchRecordsByID() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	nTmpVal = m_pIDatabase->GetUpdateSequence();
	nAffectNum = pTable->CopyToBuffer( pBuffer, nBufferSize, nSerialNo );
	if( nAffectNum < 0 )
	{
		nSerialNo = 0;
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::FetchRecordsByID() : failed 2 copy data from table, errorcode = %d", nAffectNum );
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
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 create data table 4 message, message id=%d", nDataID );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->InsertRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	m_mapTableID.insert( std::make_pair(nDataID, nDataLen) );		///< 数据表ID集合，添加

	return 0;
}

int DatabaseIO::DeleteRecord( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	I_Table*			pTable = NULL;
	int					nAffectNum = 0;
	unsigned __int64	nDbSerialNo = 0;

	if( false == m_bBuilded )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 delete record before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->DeleteRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 delete record from table, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	return nAffectNum;
}

int DatabaseIO::UpdateQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( false == m_bBuilded )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 update quotation before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->UpdateRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	m_nUpdateTimeT = ::time( NULL );

	return 0;
}

int DatabaseIO::QueryQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( false == m_bBuilded )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryQuotation() : failed 2 update quotation before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryQuotation() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	RecordBlock&	refRecord = pTable->SelectRecord( pData, ::strlen(pData) );
	if( true == refRecord.IsNone() )
	{
		return 0;
	}

	::memcpy( pData, refRecord.GetPtr(), refRecord.Length() );

	return refRecord.Length();
}

int DatabaseIO::RecoverDatabase( MkHoliday& refHoliday )
{
	try
	{
		if( m_pIDatabase )
		{
			int				nDBLoadDate = 0;		///< 行情落盘文件日期

			DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::RecoverDatabase() : recovering ......" );
			m_mapTableID.clear();
			m_nUpdateTimeT = ::time( NULL );

			if( 0 != m_pIDatabase->DeleteTables() )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::RecoverDatabase() : failed 2 clean mem-database" );
				return -1;
			}

			if( 0 < (nDBLoadDate=m_pIDatabase->LoadFromDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() )) )
			{
				unsigned int	nDataID, nRecordLen, nKeyLen;
				unsigned int	nTableCount = m_pIDatabase->GetTableCount();

				///< 检查本地落盘数据是否有效
				if( false == refHoliday.IsValidDatabaseDate( nDBLoadDate ) )
				{
					m_mapTableID.clear();
					DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::RecoverDatabase() : invalid dump file, file date = %d", nDBLoadDate );
					return -2000;
				}

				///< 统计从落盘文件加载的数据的数据类型+数据结构长度
				for( unsigned int n = 0; n < nTableCount; n++ )
				{
					if( false == m_pIDatabase->GetTableMetaByPos( n, nDataID, nRecordLen, nKeyLen ) )	{
						DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::RecoverDatabase() : cannot fetch table with index (%u)", n );
						return -1000 - n;
					}

					m_mapTableID.insert( std::make_pair(nDataID, nRecordLen) );		///< 数据表ID集合，添加
				}

				DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::RecoverDatabase() : recovered [FileDate=%d], table count=%d ......", nDBLoadDate, nTableCount );
				return 0;
			}
			else
			{
				DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::RecoverDatabase() : failed 2 recover quotation data, %s"
																, Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() );
				return -2;
			}
		}

		DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::RecoverDatabase() : invalid database pointer(NULL)" );

		return -3;
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::BackupDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::BackupDatabase() : unknow exception" );
	}

	return -3;
}

int DatabaseIO::BackupDatabase()
{
	try
	{
		if( m_pIDatabase )
		{
			DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::BackupDatabase() : making backup ......" );

			if( true == m_pIDatabase->SaveToDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() ) )
			{
				DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::BackupDatabase() : backup completed ......" );
				return 0;
			}
			else
			{
				DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::BackupDatabase() : miss backup ......" );
				return -2;
			}
		}
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::BackupDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DatabaseIO::BackupDatabase() : unknow exception" );
	}

	return -1;
}

void DatabaseIO::UnitTest()
{
	T_Func_DBUnitTest		funcUnitTest = NULL;
	int						nErrorCode = m_oDllPlugin.LoadDll( Configuration::GetConfigObj().GetMemPluginPath() );

	if( 0 != nErrorCode )
	{
		::printf( "DatabaseIO::Initialize() : failed 2 load memoryplugin module, errorcode=%d", nErrorCode );
		return;
	}

	funcUnitTest = (T_Func_DBUnitTest)m_oDllPlugin.GetDllFunction( "ExecuteUnitTest" );
	funcUnitTest();

	m_oDllPlugin.CloseDll();
}

int DatabaseIO::Initialize()
{
	Release();

	DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::Initialize() : initializing memory database plugin ......" );

	TFunc_GetFactoryObject*	m_funcFactory = NULL;
	int						nErrorCode = m_oDllPlugin.LoadDll( Configuration::GetConfigObj().GetMemPluginPath() );

	if( 0 != nErrorCode )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::Initialize() : failed 2 load memoryplugin module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_funcFactory = (TFunc_GetFactoryObject*)m_oDllPlugin.GetDllFunction( "GetFactoryObject" );
	m_pIDBFactoryPtr = &(m_funcFactory());
	m_pIDatabase = m_pIDBFactoryPtr->GrapDatabaseInterface();
	if( NULL == m_pIDatabase )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::Initialize() : invalid database interface pointer(NULL)" );
		Release();
		return -100;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::Initialize() : memory database plugin is initialized ......" );

	return 0;
}

void DatabaseIO::Release()
{
	if( NULL != m_pIDatabase || NULL != m_pIDBFactoryPtr )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::Release() : releasing memory database plugin ......" );

		m_mapTableID.clear();				///< 清空数据表ID集合
		BackupDatabase();					///< 备份行情数据到磁盘
		m_pIDatabase->DeleteTables();		///< 清理内存插件中的数据表
		m_pIDatabase = NULL;				///< 重置内存插件数据库指针

		if( m_pIDBFactoryPtr )				///< 清理内在插件中的所有数据库
		{
			m_pIDBFactoryPtr->ReleaseAllDatabase();
			m_pIDBFactoryPtr = NULL;		///< 重置内存插件的工厂对象指针
		}

		m_oDllPlugin.CloseDll();			///< 卸载内存库插件的DLL

		DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::Release() : memory database plugin is released ......" );
	}
}












