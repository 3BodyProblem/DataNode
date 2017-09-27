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
}

DatabaseIO::~DatabaseIO()
{
	Release();
}

bool DatabaseIO::IsBuilded()
{
	CriticalLock			lock( m_oLock );

	return m_bBuilded;
}

unsigned int DatabaseIO::GetTableCount()
{
	if( NULL == m_pIDatabase )
	{
		return 0;
	}

	if( false == IsBuilded() )
	{
		return 0;
	}

	return m_pIDatabase->GetTableCount();
}

int DatabaseIO::NewRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo )
{
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	nDbSerialNo = 0;
	m_bBuilded = bLastFlag;
	if( false == m_pIDatabase->CreateTable( nDataID, nDataLen, MAX_CODE_LENGTH ) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::NewRecord() : failed 2 create data table 4 message, message id=%d", nDataID );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::NewRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->InsertRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::NewRecord() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	if( 0 == nAffectNum )
	{
		if( 0 > (nAffectNum = pTable->UpdateRecord( pData, nDataLen, nDbSerialNo )) )
		{
			DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::NewRecord() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
			return -3;
		}
	}

	m_mapTableID.insert( std::make_pair(nDataID, nDataLen) );		///< ���ݱ�ID���ϣ����

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

int DatabaseIO::UpdateRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( false == m_bBuilded )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::UpdateRecord() : failed 2 update quotation before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::UpdateRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->UpdateRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::UpdateRecord() : failed 2 update data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	return nAffectNum;
}

int DatabaseIO::QueryRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( false == m_bBuilded )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryRecord() : failed 2 update quotation before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	RecordBlock	oRecord = pTable->SelectRecord( pData, ::strlen(pData) );
	if( true == oRecord.IsNone() )
	{
		return 0;
	}

	::memcpy( pData, oRecord.GetPtr(), oRecord.Length() );

	return oRecord.Length();
}

int DatabaseIO::QueryBatchRecords( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo )
{
	unsigned __int64		nTmpVal = 0;
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	if( NULL == pBuffer )
	{
		nSerialNo = 0;
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryBatchRecords() : invalid buffer pointer(NULL)" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		nSerialNo = 0;
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryBatchRecords() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	nTmpVal = m_pIDatabase->GetUpdateSequence();
	nAffectNum = pTable->CopyToBuffer( pBuffer, nBufferSize, nSerialNo );
	if( nAffectNum < 0 )
	{
		nSerialNo = 0;
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryBatchRecords() : failed 2 copy data from table, errorcode = %d", nAffectNum );
		return -3;
	}

	nSerialNo = nTmpVal;
	return nAffectNum;
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

		m_mapTableID.clear();				///< ������ݱ�����ݽṹ���ȼ���
		m_pIDatabase->DeleteTables();		///< �����ڴ����е����ݱ�
		m_pIDatabase = NULL;				///< �����ڴ������ݿ�ָ��

		if( m_pIDBFactoryPtr )				///< �������ڲ���е��������ݿ�
		{
			m_pIDBFactoryPtr->ReleaseAllDatabase();
			m_pIDBFactoryPtr = NULL;		///< �����ڴ����Ĺ�������ָ��
		}

		m_oDllPlugin.CloseDll();			///< ж���ڴ������DLL

		DataNodeService::GetSerivceObj().WriteInfo( "DatabaseIO::Release() : memory database plugin is released ......" );
	}
}


///< ----------------------------------------------------------------------------------------------


PowerfullDatabase::PowerfullDatabase()
{
}

PowerfullDatabase::~PowerfullDatabase()
{
	BackupDatabase();		///< �ȱ������ݿ�
	Release();				///< ���ͷ�������Դ
}

int PowerfullDatabase::Initialize()
{
	int			nErrCode = 0;
	DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::Initialize() : initializing powerfull database object ......" );

	if( (nErrCode=DatabaseIO::Initialize()) < 0 )
	{
		DataNodeService::GetSerivceObj().WriteError( "PowerfullDatabase::Initialize() : failed 2 initialize, errorcode = %d", nErrCode );
		return nErrCode;
	}

	if( m_oQueryBuffer.Initialize( 1024*1024*8 ) != 0 )
	{
		DataNodeService::GetSerivceObj().WriteError( "PowerfullDatabase::Initialize() : failed 2 initialize query buffer, size = %d", m_oQueryBuffer.MaxBufSize() );
		return -100;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::Initialize() : powerfull database object initialized! ..." );

	return 0;
}

void PowerfullDatabase::Release()
{
	DatabaseIO::Release();				///< �ͷ����ݿ�������Դ
	m_mapID2Codes.clear();				///< ������ݱ��code���ϱ�
	m_oQueryBuffer.Release();			///< �ͷŲ�ѯ�ڴ�
}

int PowerfullDatabase::RemoveCodeExpiredFromDisk( MAP_TABLEID_CODES& mapCodeWhiteList, bool bNeed2Erase )
{
	if( true == bNeed2Erase )
	{
		int					nAffectNum = 0;
		int					nErrorCode = 0;
		CriticalLock		guard( m_oLock );

		for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end(); it++ )
		{
			unsigned int				nDataID = it->first;
			std::set<std::string>&		setCodeFromDisk = m_mapID2Codes[nDataID];

			if( mapCodeWhiteList.find( nDataID ) == mapCodeWhiteList.end() )		///< ���ݱ�ID���ڵ����
			{
				m_mapTableID.erase( it++ );
				m_mapID2Codes.erase( nDataID );
				if( true == m_pIDatabase->DeleteTable( nDataID ) )
				{
					DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::RemoveCodeExpiredFromDisk() : DataTable(%d) deleted!", nDataID );
				}
				else
				{
					DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RemoveCodeExpiredFromDisk() : failed 2 delete DataTable(%d)", nDataID );
				}
			}
			else																	///< ���ݱ�ID��Ч�����
			{
				std::set<std::string>&	setCodeFromExchange = mapCodeWhiteList[nDataID];

				for( std::set<std::string>::iterator it = setCodeFromDisk.begin(); it != setCodeFromDisk.end(); it++ )
				{
					std::string			sCodeInDisk = it->c_str();

					if( setCodeFromExchange.find( sCodeInDisk ) != setCodeFromExchange.end() )
					{
						continue;
					}

					if( (nErrorCode=DeleteRecord( nDataID, (char*)(it->c_str()), 32 )) < 0 )
					{
						DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RemoveCodeExpiredFromDisk() : failed delete code[%s] from table[%d] ", it->c_str(), nDataID );
						return -2000 - nErrorCode;
					}

					DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::RemoveCodeExpiredFromDisk() : DataType=%d, Code[%s] has erased!", nDataID, it->c_str() );

					nAffectNum++;
				}
			}
		}

		return nAffectNum;		
	}

	return 0;		///< ��������鴫���������Ҫɾ����Ч���룬��Ϊ����Ǽ�ʱ���ϼ����µ�
}

int PowerfullDatabase::RecoverDatabase( MkHoliday& refHoliday, bool bRecoverFromDisk )
{
	try
	{
		if( false == bRecoverFromDisk )	{
			return 0;
		}

		if( m_pIDatabase )
		{
			CriticalLock				lock( m_oLock );					///< ��
			int							nDBLoadDate = 0;					///< ���������ļ�����
			unsigned int				nDataID, nRecordLen, nKeyLen;		///< ���ݱ�ID,���ݱ��¼�ṹ����,���ݱ�ļ�¼�����ĳ���

			DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::RecoverDatabase() : recovering ......" );
			///< ----------------- ��������ʷ���� -----------------------------------------------------
			m_bBuilded = false;m_mapTableID.clear();m_mapID2Codes.clear();	///< ���״̬��ͳ����Ϣ
			if( 0 != m_pIDatabase->DeleteTables() )	{						///< ������ݿ����������
				DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RecoverDatabase() : failed 2 clean mem-database" );
				return -1;
			}

			///< ----------------- �Ӵ��ָ̻���ʷ���� --------------------------------------------------
			if( 0 < (nDBLoadDate=m_pIDatabase->LoadFromDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() )) )
			{
				///< ------------- ��鱾�����������Ƿ���Ч --------------------------------------------
				if( false == refHoliday.IsValidDatabaseDate( nDBLoadDate ) )
				{
					DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RecoverDatabase() : invalid dump file, file date = %d", nDBLoadDate );
					return -2000;
				}

				///< ------------- ͳ�ƴ������ļ����ص����ݵ���������+���ݽṹ���� ---------------------
				unsigned int	nTableCount = m_pIDatabase->GetTableCount();
				for( unsigned int n = 0; n < nTableCount; n++ )
				{
					if( false == m_pIDatabase->GetTableMetaByPos( n, nDataID, nRecordLen, nKeyLen ) )	{
						DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RecoverDatabase() : cannot fetch table with index (%u)", n );
						return -1000 - n;
					}

					m_mapTableID.insert( std::make_pair(nDataID, nRecordLen) );		///< ͳ�ƣ�{���ݱ�ID : ���ݼ�¼�ṹ����}
				}

				///< ------------- ͳ�Ƹ����ݱ��µĴ��뼯�� -----------------------------------------------
				if( nTableCount > 0 )
				{
					for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end(); it++ )
					{
						std::set<std::string>		setCode;
						int							nDataLen = 0;
						unsigned __int64			nSerialNoOfAnchor = 0;

						if( (nDataLen=QueryBatchRecords( it->first, (char*)m_oQueryBuffer, m_oQueryBuffer.MaxBufSize(), nSerialNoOfAnchor )) < 0 )	{
							DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RecoverDatabase() : failed 2 query batch of records from table" );
							return -2;
						}

						for( int nOffset = 0; nOffset < nDataLen; nOffset+=it->second )	{
							setCode.insert( std::string((char*)m_oQueryBuffer+nOffset) );
						}

						m_mapID2Codes[nDataID] = setCode;							///< ͳ�ƣ� {���ݱ�ID : code����}
					}

					DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::RecoverDatabase() : fetch table number=%d", m_mapID2Codes.size() );
				}

				///< -------------- �������ݿ⹹���ɹ���ʶ ------------------------------------------------
				m_bBuilded = true;
				DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::RecoverDatabase() : recovered [FileDate=%d], table count=%d ......", nDBLoadDate, nTableCount );
				return 0;
			}
		}

		DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RecoverDatabase() : failed 2 recover quotation data, (%s), DB address:%x" , Configuration::GetConfigObj().GetRecoveryFolderPath().c_str(), m_pIDatabase );
		return -3;
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RecoverDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::RecoverDatabase() : unknow exception" );
	}

	return -4;
}

int PowerfullDatabase::BackupDatabase()
{
	try
	{
		if( m_pIDatabase )
		{
			DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::BackupDatabase() : making backup ......" );

			if( true == m_pIDatabase->SaveToDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() ) )
			{
				DataNodeService::GetSerivceObj().WriteInfo( "PowerfullDatabase::BackupDatabase() : backup completed ......" );
				return 0;
			}
			else
			{
				DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::BackupDatabase() : miss backup ......" );
				return -2;
			}
		}
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::BackupDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::BackupDatabase() : unknow exception" );
	}

	return -1;
}

int PowerfullDatabase::FlushDatabase2RequestSessions( unsigned __int64 nSerialNo )
{
	int							nReqLinkID = 0;
	unsigned int				nTableCount = GetTableCount();
	CriticalLock				lock( m_oLock );
	unsigned int				nReqLinkCount = LinkNoRegister::GetRegister().GetReqLinkCount();

	if( nReqLinkCount > 0 )
	{
		while( (nReqLinkID = LinkNoRegister::GetRegister().PopReqLinkID()) >= 0 )
		{
			int					n = 0;
			for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end(); it++, n++ )
			{
				int					nFunctionID = ((n+1)==nTableCount) ? 100 : 0;	///< ���һ�����ݰ��ı�ʶ
				unsigned __int64	nSerialNoOfAnchor = nSerialNo;
				int					nDataLen = QueryBatchRecords( it->first, (char*)m_oQueryBuffer, m_oQueryBuffer.MaxBufSize(), nSerialNoOfAnchor );

				if( nDataLen <= 0 )
				{
					DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::FlushDatabase2RequestSessions() : cannot fetch image from database, TCP connection will be destroyed! errorcode=%d", nDataLen );
					DataNodeService::GetSerivceObj().CloseLink( nReqLinkID );
					return -1 * (n*100);
				}

				///< ---------------- ����ѯ�����������¸�ʽ�����ͻ��� -------------------------------
				tagPackageHead*		pPkgHead = (tagPackageHead*)((char*)m_oQueryBuffer);
				///< �������͸�ʽ���ݰ�
				::memmove( (char*)m_oQueryBuffer+sizeof(tagPackageHead), (char*)m_oQueryBuffer, nDataLen );
				///< ���ݰ�ͷ����
				pPkgHead->nSeqNo = n;
				pPkgHead->nMsgLength = it->second;
				pPkgHead->nMsgCount = nDataLen / it->second;
				pPkgHead->nMarketID = DataCollector::GetMarketID();
				unsigned int	nSendLen = nDataLen + sizeof(tagPackageHead);
				///< ---------------------------------------------------------------------------------

				int	nErrCode = DataNodeService::GetSerivceObj().SendData( nReqLinkID, it->first, nFunctionID, (char*)m_oQueryBuffer, nSendLen/*, nSerialNo*/ );
				if( nErrCode < 0 )
				{
					DataNodeService::GetSerivceObj().CloseLink( nReqLinkID );
					DataNodeService::GetSerivceObj().WriteWarning( "PowerfullDatabase::FlushDatabase2RequestSessions() : failed 2 send image data, errorcode=%d", nErrCode );
					return -2 * (n*10000);
				}

				if( 100 == nFunctionID )					///< ���һ�����ݰ���function id��100
				{
					LinkNoRegister::GetRegister().NewPushLinkID( nReqLinkID );	///< ���»Ự��id���������б�
				}
			}
		}

		return nReqLinkCount;
	}

	return 0;
}



