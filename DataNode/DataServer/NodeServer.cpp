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
			///< ��ʼ��ҵ��˳����߼�
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

				if( 0 != (nErrorCode=m_oDataCollector.ReInitializeDataCollector()) )
				{
					DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
					m_oInitFlag.RedoInitialize();
					continue;
				}

				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : [NOTICE] Service is Initialized ......" );
				continue;
			}

			///< ���д�����
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

		int									nErrorCode = Configuration::GetConfigObj().Load();	///< ����������Ϣ
		const tagServicePlug_StartInParam&	refStartInParam = Configuration::GetConfigObj().GetStartInParam();

		if( 0 != nErrorCode )	{
			DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : invalid configuration file, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		if( (nErrorCode=MServicePlug::Instance( &refStartInParam )) < 0 )	{					///< ��ʼ��������
			::printf( "DataNodeService::Activate() : failed 2 initialize serviceIO framework, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		if( 0 != (nErrorCode=m_oLinkSessions.Instance()) )	{									///< ��ʼ���Ự��·����
			::printf( "DataNodeService::Activate() : failed 2 initialize link session set, errorcode=%d", nErrorCode );
			return -2;
		}

		MServicePlug::RegisterSpi( &m_oLinkSessions );											///< ע������ܵĻص�����

		MServicePlug::WriteInfo( "DataNodeService::Activate() : serviceIO framework initialized!" );
		MServicePlug::WriteInfo( "DataNodeService::Activate() : Configuration As Follow:\n\
								 MaxLinkCount:%d\nListenPort:%d\nListenCount:%d\nSendBufCount:%d\nThreadCount:%d\nSendTryTimes:%d\n\
								 LinkTimeOut:%d\nCompressFlag:%d\nSSLFlag:%d\nPfxFilePasswrod:%s\nDetailLog:%d\nPageSize:%d\nPageCount:%d"
								, refStartInParam.uiMaxLinkCount, refStartInParam.uiListenPort, refStartInParam.uiListenCount
								, refStartInParam.uiSendBufCount, refStartInParam.uiThreadCount, refStartInParam.uiSendTryTimes
								, refStartInParam.uiLinkTimeOut, refStartInParam.bCompress, refStartInParam.bSSL, refStartInParam.szPfxFilePasswrod
								, refStartInParam.bDetailLog, refStartInParam.uiPageSize, refStartInParam.uiPageCount );

		///< ........................ ��ʼ�������ڵ����� .............................
		if( 0 != (nErrorCode = DataIOEngine::Initialize( Configuration::GetConfigObj().GetDataCollectorPluginPath()
													, Configuration::GetConfigObj().GetMemPluginPath()
													, Configuration::GetConfigObj().GetHolidayFilePath() )) )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : failed 2 initialize service engine, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

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

int DataNodeService::OnIdle()
{
	bool			bInitPoint = false;

	if( 0 == ImageRebuilder::GetObj().GetReqSessionCount() )	{
		SimpleTask::Sleep( 1000 );
	}

	///< ����Ƿ����µ����ӵ��������ʼ�������������͵�
	ImageRebuilder::GetObj().Flush2ReqSessions( m_oDatabaseIO, 0 );

	///< �ڷǽ���ʱ�Σ������ڴ����е�������������
	if( -1 == m_oInitFlag.InTradingPeriod( bInitPoint ) && true == m_oDatabaseIO.IsBuilded() )	{
		OnBackupDatabase();
	}

	///< ��ѯ����ɼ�ģ���״̬
	OnInquireStatus();

	return 0;
}

void DataNodeService::OnInquireStatus()
{
	bool					bDataBuilded = m_oDatabaseIO.IsBuilded();
	const CollectorStatus&	refDataCollectorStatus = m_oDataCollector.InquireDataCollectorStatus();

}

void DataNodeService::OnBackupDatabase()
{
	static time_t	nLastTimeT = ::time( NULL );
	time_t			nTimeNowT = ::time( NULL );

	///< ÿʮ��������һ��
	if( nTimeNowT - nLastTimeT < 60 * 10 )
	{
		return;
	}

	///< ��ʼ���̱��ݲ���
	if( 0 != m_oDatabaseIO.BackupDatabase() )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::BackupDatabase() : failed 2 backup quotation data" );
		return;
	}

	nLastTimeT = nTimeNowT;
}






