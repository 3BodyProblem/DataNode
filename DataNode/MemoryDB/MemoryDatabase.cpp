#include <time.h>
#include <stdio.h>
#include "MemoryDatabase.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


PkgBuffer::PkgBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxBufSize( 0 )
{
}

int PkgBuffer::Initialize( unsigned int nBuffSize )
{
	Release();

	if( NULL == (m_pPkgBuffer = new char[nBuffSize]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "PkgBuffer::Initialize() : failed 2 initialize pkg data buffer, size = %d", nBuffSize );
		return -1;
	}

	m_nMaxBufSize = nBuffSize;

	return 0;
}

void PkgBuffer::Release()
{
	if( NULL != m_pPkgBuffer )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "PkgBuffer::Release() : release pkg data buffer, size = %d", m_nMaxBufSize );
		delete []m_pPkgBuffer;
		m_pPkgBuffer = NULL;
	}

	m_nMaxBufSize = 0;
}

PkgBuffer::operator char*()
{
	if( NULL != m_pPkgBuffer )
	{
		return m_pPkgBuffer;
	}

	throw std::runtime_error( "PkgBuffer::operator char*() : invalid cache buffer pointer ( NULL )" );
}

unsigned int PkgBuffer::MaxBufSize() const
{
	return m_nMaxBufSize;
}


///< -----------------------------------------------------------------------


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
		return -1000;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2000;
	}

	if( 0 > (nAffectNum = pTable->DeleteRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 delete record from table, message id=%d, errorcode=%d", nDataID, nAffectNum );
		return -3000;
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
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::UpdateRecord() : failed 2 update quotation before initialization, message id=%d", nDataID );
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
		DataNodeService::GetSerivceObj().WriteError( "DatabaseIO::QueryRecord() : failed 2 update quotation before initialization, message id=%d", nDataID );
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

		m_mapTableID.clear();				///< 清空数据表的数据结构长度集合
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


///< ----------------------------------------------------------------------------------------------


const unsigned int	MAX_QUERY_BUFFER_LEN = 1024*1024*20;


PowerDB::PowerDB()
{
}

PowerDB::~PowerDB()
{
	BackupDatabase();		///< 先备份数据库
	Release();				///< 再释放所有资源
}

int PowerDB::Initialize()
{
	int			nErrCode = 0;
	DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::Initialize() : initializing powerfull database object ......" );

	Release();
	if( (nErrCode=DatabaseIO::Initialize()) < 0 )
	{
		DataNodeService::GetSerivceObj().WriteError( "PowerDB::Initialize() : failed 2 initialize, errorcode = %d", nErrCode );
		return nErrCode;
	}

	if( m_oQueryBuffer.Initialize( MAX_QUERY_BUFFER_LEN ) != 0 )
	{
		DataNodeService::GetSerivceObj().WriteError( "PowerDB::Initialize() : failed 2 initialize query buffer, size = %d", m_oQueryBuffer.MaxBufSize() );
		return -100;
	}

	if( 0 != m_oEncoder.Initialize( Configuration::GetConfigObj().GetCompressPluginPath(), Configuration::GetConfigObj().GetCompressPluginCfg(), MAX_QUERY_BUFFER_LEN ) )
	{
		DataNodeService::GetSerivceObj().WriteError( "PowerDB::Initialize() : failed 2 initialize data encoder" );
		return -200;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::Initialize() : powerfull database object initialized! ..." );

	return 0;
}

void PowerDB::Release()
{
	DatabaseIO::Release();					///< 释放数据库插件的资源
	m_mapID2CodesInDB.clear();				///< 清空数据表的code集合表
	m_oQueryBuffer.Release();				///< 释放查询内存
}

int PowerDB::RemoveExpiredItem4LoadFromDisk( MAP_TABLEID_CODES& mapID2CodeWhiteList, bool bNeed2Erase )
{
	int					nAffectNum = 0;
	int					nErrorCode = 0;
	CriticalLock		guard( m_oLock );

	if( false == bNeed2Erase )	return 0;	///< 如果是行情传输代理，则不需要删除无效代码，因为码表都是即时从上级更新的

	for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end(); )
	{
		unsigned int					nDataID = it->first;									///< 遍历出从磁盘加载的所有Message ID

		if( mapID2CodeWhiteList.find( nDataID ) == mapID2CodeWhiteList.end() )					///< [ 数据表ID过期的情况 ]
		{
			m_mapTableID.erase( it++ );															///< 删除统计信息中的过期Message ID
			m_mapID2CodesInDB.erase( nDataID );													///< 删除统计信息中的过期Message ID

			if( true == m_pIDatabase->DeleteTable( nDataID ) )	{								///< 删除内存库中的过期Message ID
				DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::RemoveExpiredItem4LoadFromDisk() : DataTable(%d) deleted!", nDataID );
			}	else	{
				DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RemoveExpiredItem4LoadFromDisk() : failed 2 delete DataTable(%d)", nDataID );
			}

			continue;
		}
		else																					///< [ 数据表ID有效的情况 ]
		{
			std::set<std::string>&		setCodeFromDisk = m_mapID2CodesInDB[nDataID];			///< 根据这个Message ID定位出之下的商品Code集合

			for( std::set<std::string>::iterator it = setCodeFromDisk.begin(); it != setCodeFromDisk.end(); it++ )
			{
				std::string					sCodeInDisk = it->c_str();
				std::set<std::string>&		setCodeFromExchange = mapID2CodeWhiteList[nDataID];

				if( setCodeFromExchange.find( sCodeInDisk ) == setCodeFromExchange.end() )		///< 对从磁盘加载的Code，但确不曾在行情接收中收到的Code进行记录删除
				{
					if( (nErrorCode=DeleteRecord( nDataID, (char*)(it->c_str()), 32 )) <= 0 )	///< 删除过期的记录
					{
						DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RemoveExpiredItem4LoadFromDisk() : failed 2 delete code[%s] from table[%d], errorcode = %d", it->c_str(), nDataID, nErrorCode );
						continue;
					}

					nAffectNum++;
					DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::RemoveExpiredItem4LoadFromDisk() : DataType=%d, Code[%s] has erased!", nDataID, it->c_str() );
				}
			}

			it++;
			continue;
		}
	}

	return nAffectNum;
}

int PowerDB::RecoverDatabase( MkHoliday& refHoliday, bool bRecoverFromDisk )
{
	try
	{
		if( false == bRecoverFromDisk )	{	return 0;	}

		if( m_pIDatabase )
		{
			CriticalLock				lock( m_oLock );						///< 锁
			int							nDBLoadDate = 0;						///< 行情落盘文件日期
			unsigned int				nDataID, nRecordLen, nKeyLen;			///< 数据表ID,数据表记录结构长度,数据表的记录主键的长度

			DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::RecoverDatabase() : recovering ......" );
			///< ----------------- 先清理历史缓存 -----------------------------------------------------
			m_bBuilded = false;m_mapTableID.clear();m_mapID2CodesInDB.clear();	///< 清空状态和统计信息
			if( 0 != m_pIDatabase->DeleteTables() )	{							///< 清空数据库的所有内容
				DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RecoverDatabase() : failed 2 clean mem-database" );
				return -1;
			}

			///< ----------------- 从磁盘恢复历史数据 --------------------------------------------------
			if( 0 < (nDBLoadDate=m_pIDatabase->LoadFromDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() )) )
			{
				///< ------------- 检查本地落盘数据是否有效 --------------------------------------------
				if( false == refHoliday.IsValidDatabaseDate( nDBLoadDate ) )
				{
					DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RecoverDatabase() : invalid dump file, file date = %d", nDBLoadDate );
					return -2000;
				}

				///< ------------- 统计从落盘文件加载的数据的数据类型+数据结构长度 ---------------------
				unsigned int	nTableCount = m_pIDatabase->GetTableCount();
				for( unsigned int n = 0; n < nTableCount; n++ )
				{
					if( false == m_pIDatabase->GetTableMetaByPos( n, nDataID, nRecordLen, nKeyLen ) )	{
						DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RecoverDatabase() : cannot fetch table with index (%u)", n );
						return -1000 - n;
					}

					m_mapTableID.insert( std::make_pair(nDataID, nRecordLen) );		///< 统计：{数据表ID : 数据记录结构长度}
				}

				///< ------------- 统计各数据表下的代码集合 -----------------------------------------------
				if( nTableCount > 0 )
				{
					for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end(); it++ )
					{
						std::set<std::string>		setCode;
						int							nDataLen = 0;
						unsigned __int64			nSerialNoOfAnchor = 0;

						if( (nDataLen=QueryBatchRecords( it->first, (char*)m_oQueryBuffer, m_oQueryBuffer.MaxBufSize(), nSerialNoOfAnchor )) < 0 )	{
							DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RecoverDatabase() : failed 2 query batch of records from table" );
							return -2;
						}

						for( int nOffset = 0; nOffset < nDataLen; nOffset+=it->second )	{
							setCode.insert( std::string((char*)m_oQueryBuffer+nOffset) );
						}

						m_mapID2CodesInDB[it->first] = setCode;						///< 统计： {数据表ID : code集合}
						DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::RecoverDatabase() : ID = %d, record count (in disk) = %d", it->first, setCode.size() );
					}

					DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::RecoverDatabase() : fetch table number=%d", m_mapID2CodesInDB.size() );
				}

				///< -------------- 设置数据库构建成功标识 ------------------------------------------------
				m_bBuilded = true;
				DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::RecoverDatabase() : recovered [FileDate=%d], table count=%d ......", nDBLoadDate, nTableCount );
				return 0;
			}
		}

		DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RecoverDatabase() : failed 2 recover quotation data, (%s), DB address:%x" , Configuration::GetConfigObj().GetRecoveryFolderPath().c_str(), m_pIDatabase );
		return -3;
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RecoverDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::RecoverDatabase() : unknow exception" );
	}

	return -4;
}

int PowerDB::BackupDatabase()
{
	try
	{
		if( m_pIDatabase )
		{
			DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::BackupDatabase() : making backup ......" );

			if( true == m_pIDatabase->SaveToDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() ) )
			{
				DataNodeService::GetSerivceObj().WriteInfo( "PowerDB::BackupDatabase() : backup completed ......" );
				return 0;
			}
			else
			{
				DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::BackupDatabase() : miss backup ......" );
				return -2;
			}
		}
	}
	catch( std::exception& err )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::BackupDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::BackupDatabase() : unknow exception" );
	}

	return -1;
}

int PowerDB::FlushDatabase2RequestSessions( unsigned __int64 nSerialNo )
{
	int							nReqLinkID = 0;
	unsigned int				nTableCount = GetTableCount();
	unsigned int				nReqLinkCount = LinkNoRegister::GetRegister().GetReqLinkCount();
	CriticalLock				lock( m_oLock );

	if( nReqLinkCount <= 0 )	return 0;

	for( int nSeqNo = 0; (nReqLinkID = LinkNoRegister::GetRegister().PopReqLinkID()) >= 0; nSeqNo = 0 )			///< 有新到达的链接的情况，需要被发送初始化快照数据
	{
		for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end(); it++, nSeqNo++ )	///< 遍历每个MessageID值
		{
			unsigned __int64	nSerialNoOfAnchor = nSerialNo;													///< 获取数据块查询"锚值"
			int					nFunctionID = ((nSeqNo+1)==nTableCount) ? 100 : 0;								///< 最后一个数据包的标识
			int					nDataLen = QueryBatchRecords( it->first, (char*)m_oQueryBuffer, m_oQueryBuffer.MaxBufSize(), nSerialNoOfAnchor );

			if( nDataLen <= 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::FlushDatabase2RequestSessions() : cannot fetch Table(ID=%d) from D.B., TCP connection will be destroyed! errorcode=%d", it->first, nDataLen );
				DataNodeService::GetSerivceObj().CloseLink( nReqLinkID );										///< 查询MessageID对应的数据表的数据失败,断开对下链路
				return -1 * (nSeqNo*100);
			}

			///< ---------------- 将查询出的数据重新格式到发送缓存 -------------------------------
			tagPackageHead*		pPkgHead = (tagPackageHead*)((char*)m_oQueryBuffer);							///< 构建发送格式数据包
			::memmove( (char*)m_oQueryBuffer+sizeof(tagPackageHead), (char*)m_oQueryBuffer, nDataLen );			///< 数据包头构建
			pPkgHead->nSeqNo = nSeqNo;
			pPkgHead->nMsgLength = it->second;
			pPkgHead->nMarketID = DataCollector::GetMarketID();
			///< ---------------- 将数据编码到缓存 -----------------------------------------------
			if( 0 != m_oEncoder.Prepare4ACompression( (char*)pPkgHead ) )
			{
				DataNodeService::GetSerivceObj().WriteError( "PowerDB::FlushDatabase2RequestSessions() : failed 2 prepare a compression, messageid=%u", it->first );
				return -1;
			}

			for( unsigned int nOffset = sizeof(tagPackageHead); nOffset < (sizeof(tagPackageHead)+nDataLen); nOffset += it->second )
			{
				m_oEncoder.CompressData( it->first, (char*)pPkgHead + nOffset, it->second );
			}
			///< ---------------- 发送数据 --------------------------------------------------------
			int	nErrCode = DataNodeService::GetSerivceObj().SendData( nReqLinkID, it->first, nFunctionID, m_oEncoder.GetBufferPtr(), m_oEncoder.GetBufferLen() );
			if( nErrCode < 0 )
			{
				DataNodeService::GetSerivceObj().CloseLink( nReqLinkID );										///< 发送初始化快照数据失败，则断开对下链路
				DataNodeService::GetSerivceObj().WriteWarning( "PowerDB::FlushDatabase2RequestSessions() : failed 2 send image data, errorcode=%d", nErrCode );
				return -2 * (nSeqNo*10000);
			}

			if( 100 == nFunctionID )																			///< 最后一个数据包的Function ID等于100，则表示初始化成功
			{
				LinkNoRegister::GetRegister().NewPushLinkID( nReqLinkID );										///< 初始化快照数据发送成功后，将新会话的LinkID加入推送列表
			}
		}
	}

	return nReqLinkCount;
}



