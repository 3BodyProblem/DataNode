#pragma warning(disable:4996)
#include <time.h>
#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "NodeServer.h"


DataIOEngine::DataIOEngine()
 : m_oLinkSessions( m_oDatabaseIO ), SimpleTask( "DataIOEngine::Thread" )
 , m_nPushSerialNo( 0 ), m_nHeartBeatCount( 0 ), m_oInitFlag( m_oDataCollector )
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

	m_oDatabaseIO.RecoverDatabase( m_oInitFlag.GetHoliday() );
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
	m_nPushSerialNo = 0;
	m_nHeartBeatCount = 0;
	m_oDataCollector.Release();
	m_oDatabaseIO.Release();
	SimpleTask::StopThread();
}

bool DataIOEngine::PrepareQuotation()
{
	int			nErrorCode = 0;
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::PrepareQuotation() : reloading quotation........" );

	m_oDataCollector.HaltDataCollector();												///< 1) ������ֹͣ���ݲɼ�ģ��
	///< ����ɼ����: ���ȼ������̿�����������,�ٽ����зǵ���ļ�¼ɾ��
	if( false == m_oDataCollector.IsProxy() )
	{
		if( 0 == (nErrorCode=m_oDatabaseIO.RecoverDatabase(m_oInitFlag.GetHoliday())) )	///< 2) �ӱ����ļ��ָ���ʷ�������ݵ��ڴ���
		{
			if( 0 >= (nErrorCode=LoadCodesListInDatabase()) )							///< ���ڴ������Ѵ��ڵ���Ʒ���룬���Ƿ��й��ڴ�������ɾ���ж���
			{
				DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::PrepareQuotation() : failed 2 code list from database ..., errorcode=%d", nErrorCode );
				return false;
			}
		}
	}

	if( 0 != (nErrorCode=m_oDataCollector.RecoverDataCollector()) )						///< 3) ���³�ʼ������ɼ�ģ��
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::PrepareQuotation() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
		return false;;
	}

	if( (nErrorCode=RemoveCodeExpiredInDatabase()) < 0 )								///< 4) ɾ���ڴ��зǵ���Ĺ��ڵ���Ʒ
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::PrepareQuotation() : failed 2 remove expired code in database, errorcode=%d", nErrorCode );
		return false;;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::PrepareQuotation() : quotation reloaded, MarketID=%u ........", DataCollector::GetMarketID() );

	return true;
}

int DataIOEngine::Execute()
{
	bool			bInitPoint = false;
	unsigned int	nInitInterval = 1000*Configuration::GetConfigObj().GetInitInterval();
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : enter into thread func ..." );

	while( true == IsAlive() )
	{
		try
		{
			if( true == m_oInitFlag.GetFlag() )			///< ��ʼ��ҵ��˳����߼�
			{
				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : [NOTICE] Enter Service Initializing Time ......" );

				if( false == PrepareQuotation() )		///< ���¼�����������
				{
					m_oInitFlag.RedoInitialize();		///< ����Ϊ��Ҫ��ʼ����ʶΪ
					SimpleTask::Sleep( nInitInterval );	///< ���³�ʼ�������Ĭ��Ϊ3��
					continue;
				}

				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : ................. [NOTICE] Service is Available ....................." );
				continue;
			}

			OnIdle();									///< ���д�����
			SimpleTask::Sleep( 1000 );					///< һ��ѭ��һ��
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

InitializerFlag& DataIOEngine::GetInitFlag()
{
	return m_oInitFlag;
}

int DataIOEngine::LoadCodesListInDatabase()
{
	int					nErrorCode = 0;
	unsigned int		lstTableID[64] = { 0 };
	unsigned int		lstRecordWidth[64] = { 0 };
	unsigned int		nTableCount = m_oDatabaseIO.GetTablesID( lstTableID, 64, lstRecordWidth, 64 );
	CriticalLock		guard( m_oCodeMapLock );

	if( 0 == nTableCount ) {
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::LoadCodesListInDatabase() : database is empty " );
		return -1;
	}

	m_mapID2Codes.clear();
	for( unsigned int n = 0; n < nTableCount; n++ )
	{
		std::set<std::string>		setCode;
		unsigned int				nDataID = lstTableID[n];
		unsigned int				nRecordLen = lstRecordWidth[n];

		if( (nErrorCode = m_oDatabaseIO.QueryCodeListInImage( nDataID, nRecordLen, setCode )) < 0 )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::LoadCodesListInDatabase() : failed fetch code list in table [%d] ", nDataID );
			return -100 - n;
		}

		m_mapID2Codes[nDataID] = setCode;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::LoadCodesListInDatabase() : fetch codes number=%d", m_mapID2Codes.size() );

	return m_mapID2Codes.size();
}

int DataIOEngine::RemoveCodeExpiredInDatabase()
{
	if( false == m_oDataCollector.IsProxy() )
	{
		return 0;		///< ��������鴫���������Ҫɾ����Ч���룬��Ϊ����Ǽ�ʱ���ϼ����µ�
	}

	int					nAffectNum = 0;
	int					nErrorCode = 0;
	unsigned int		lstTableID[64] = { 0 };
	unsigned int		nTableCount = m_oDatabaseIO.GetTablesID( lstTableID, 64, NULL, 0 );
	CriticalLock		guard( m_oCodeMapLock );

	for( unsigned int n = 0; n < nTableCount; n++ )
	{
		unsigned int				nDataID = lstTableID[n];
		std::set<std::string>&		setCode = m_mapID2Codes[nDataID];

		for( std::set<std::string>::iterator it = setCode.begin(); it != setCode.end(); it++ )
		{
			if( (nErrorCode=m_oDatabaseIO.DeleteRecord( nDataID, (char*)(it->c_str()), 32 )) < 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::RemoveCodeExpiredInDatabase() : failed delete code[] from table [%s] ", it->c_str(), nDataID );
				return -1000 - nErrorCode;
			}

			DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::RemoveCodeExpiredInDatabase() : DataType=%d, Code[%s] has erased!", nDataID, it->c_str() );

			nAffectNum++;
		}
	}

	return nAffectNum;
}

int DataIOEngine::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	unsigned __int64		nSerialNo = 0;
	static	const char		s_pszZeroBuff[128] = { 0 };

	if( 0 == strncmp( pData, s_pszZeroBuff, sizeof(s_pszZeroBuff) ) )
	{
		return m_oDatabaseIO.FetchRecordsByID( nDataID, pData, nDataLen, nSerialNo );
	}
	else
	{
		return m_oDatabaseIO.QueryQuotation( nDataID, pData, nDataLen, nSerialNo );
	}
}

int DataIOEngine::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	CriticalLock		guard( m_oCodeMapLock );

	///< ɾ�����кϷ�����Ʒ����¼�¹��ڴ����б�
	if( m_mapID2Codes.find( nDataID ) != m_mapID2Codes.end() )
	{
		std::set<std::string>&		setCode = m_mapID2Codes[nDataID];

		setCode.erase( std::string( pData ) );
	}

	return m_oDatabaseIO.BuildMessageTable( nDataID, pData, nDataLen, bLastFlag, m_nPushSerialNo );
}

int DataIOEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	int					nErrorCode = m_oDatabaseIO.UpdateQuotation( nDataID, pData, nDataLen, m_nPushSerialNo );

	if( 0 >= nErrorCode )
	{
		return nErrorCode;
	}

	m_oLinkSessions.PushQuotation( nDataID, 0, pData, nDataLen, bPushFlag, m_nPushSerialNo );

	return nErrorCode;
}

void DataIOEngine::OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
{
    va_list		valist;
    char		pszLogBuf[8000] = { 0 };

    va_start( valist, pszFormat );
    _vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
    va_end( valist );

	switch( nLogLevel )	///< ��־����[0=��Ϣ��1=������־��2=������־��3=��ϸ��־]
	{
	case 0:
		MServicePlug::WriteInfo( "[Plugin] %s", pszLogBuf );
		break;
	case 1:
		MServicePlug::WriteWarning( "[Plugin] %s", pszLogBuf );
		break;
	case 2:
		MServicePlug::WriteError( "[Plugin] %s", pszLogBuf );
		break;
	case 3:
		MServicePlug::WriteDetail( "[Plugin] %s", pszLogBuf );
		break;
	default:
		::printf( "[Plugin] unknow log level [%d] \n", nLogLevel );
		break;
	}
}


///< ----------------------------------------------------------------------------


DataNodeService::DataNodeService()
 : m_bActivated( false )
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
		m_bActivated = true;
		DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::Activate() : Activating Service" );

		static	char						pszErrorDesc[8192] = { 0 };
		int									nErrorCode = Configuration::GetConfigObj().Load();	///< ����������Ϣ
		const tagServicePlug_StartInParam&	refStartInParam = Configuration::GetConfigObj().GetStartInParam();

		if( 0 != nErrorCode )	{
			DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : invalid configuration file, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		MServicePlug::WriteInfo( "DataNodeService::Activate() : [ServicePlugin] Configuration As Follow:\n\
								 MaxLinkCount:%d\nListenPort:%d\nListenCount:%d\nSendBufCount:%d\nThreadCount:%d\nSendTryTimes:%d\n\
								 LinkTimeOut:%d\nCompressFlag:%d\nSSLFlag:%d\nPfxFilePasswrod:%s\nDetailLog:%d\nPageSize:%d\nPageCount:%d"
								, refStartInParam.uiMaxLinkCount, refStartInParam.uiListenPort, refStartInParam.uiListenCount
								, refStartInParam.uiSendBufCount, refStartInParam.uiThreadCount, refStartInParam.uiSendTryTimes
								, refStartInParam.uiLinkTimeOut, refStartInParam.bCompress, refStartInParam.bSSL, refStartInParam.szPfxFilePasswrod
								, refStartInParam.bDetailLog, refStartInParam.uiPageSize, refStartInParam.uiPageCount );

		if( (nErrorCode=MServicePlug::Instance( &refStartInParam, pszErrorDesc, sizeof(pszErrorDesc) )) < 0 )	{///< ��ʼ��������
			::printf( "DataNodeService::Activate() : failed 2 initialize serviceIO framework, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		if( 0 != (nErrorCode=m_oLinkSessions.Instance()) )	{									///< ��ʼ���Ự��·����
			::printf( "DataNodeService::Activate() : failed 2 initialize link session set, errorcode=%d", nErrorCode );
			return -2;
		}

		///< ........................ ��ʼ�������ڵ����� .............................
		if( 0 != (nErrorCode = DataIOEngine::Initialize( Configuration::GetConfigObj().GetDataCollectorPluginPath()
													, Configuration::GetConfigObj().GetMemPluginPath()
													, Configuration::GetConfigObj().GetHolidayFilePath() )) )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::Activate() : failed 2 initialize service engine, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		MServicePlug::RegisterSpi( &m_oLinkSessions );											///< ע������ܵĻص�����
		DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::Activate() : [OK] Service Activated ! " );

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
		DataIOEngine::Release();
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

bool DataNodeService::IsServiceAlive()
{
	if( true == SimpleThread::IsAlive() && false == MServicePlug::IsStop() )
	{
		return true;
	}
	else
	{
		return false;
	}
}

int DataNodeService::OnIdle()
{
	static time_t		s_nLastDumpTime = ::time( NULL );
	bool				bInitPoint = false;
	unsigned int		nDumpInterval = Configuration::GetConfigObj().GetDumpInterval();
	int					nPertiodIndex = m_oInitFlag.InTradingPeriod( bInitPoint );

	///< ����Ƿ����µ����ӵ��������ʼ�������������͵�
	m_oDatabaseIO.FlushImageData2NewSessions( 0 );///< ���µ�������ӣ�����"ȫ��"��ʼ����������
	///< ��·ά�֣�����������
	OnHeartBeat();

	///< �ڽ���ʱ�Σ������ڴ����е�������������
	if( 0 <= nPertiodIndex && true == m_oDatabaseIO.IsBuilded() )
	{
		int		nNowTime = (int)::time( NULL );

		if( (nNowTime-=(int)s_nLastDumpTime) >= nDumpInterval )
		{
			OnBackupDatabase();
			s_nLastDumpTime = ::time( NULL );
		}
	}

	///< �ǽ���ʱ�Σ�ֹͣԴ���������ݲɼ�ģ��Ĺ���
	if( nPertiodIndex < 0 && false == m_oDataCollector.IsProxy() && true == m_oDataCollector.IsAlive() )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::OnIdle() : halting data collector ......" );
		m_oDataCollector.HaltDataCollector();
	}

	return 0;
}

void DataNodeService::OnHeartBeat()
{
	static bool					s_bBeginCheck = false;
	static unsigned __int64		s_nPushSerialNo = 0;
	static time_t				s_nLastTime = ::time( NULL );
	unsigned int				nNowT = (unsigned int)::time( NULL );

	if( s_nPushSerialNo == m_nPushSerialNo )
	{
		if( false == s_bBeginCheck )
		{
			s_bBeginCheck = true;
			s_nLastTime = nNowT;
		}
		else
		{
			time_t	nTimeDiff = nNowT - s_nLastTime;

			if( nTimeDiff >= 16 )
			{
				m_nHeartBeatCount++;
				s_bBeginCheck = false;
				m_oLinkSessions.PushQuotation( MSG_HEARTBEAT_ID, 0, (char*)&nNowT, sizeof(nNowT), true, m_nPushSerialNo );
				::printf( "------------------- heart beat message --------------------------\n" );
			}
		}
	}
	else
	{
		s_bBeginCheck = false;
	}

	return;
}

unsigned int DataNodeService::OnInquireHeartBeatCount()
{
	return m_nHeartBeatCount;
}

bool DataNodeService::OnInquireStatus( char* pszStatusDesc, unsigned int& nStrLen )
{
	bool				bInitPoint = false;
	bool				bDataBuilded = m_oDatabaseIO.IsBuilded();
	enum E_SS_Status	eStatus = m_oDataCollector.InquireDataCollectorStatus( pszStatusDesc, nStrLen );

	///< �ǽ���ʱ�εĹ���״̬
	if( m_oInitFlag.InTradingPeriod( bInitPoint ) < 0 )
	{
		return  bDataBuilded;
	}

	///< ����ʱ�εĹ���״̬
	if( ET_SS_WORKING == eStatus && true == bDataBuilded )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DataNodeService::OnBackupDatabase()
{
	///< ��ʼ���̱��ݲ���
	if( 0 != m_oDatabaseIO.BackupDatabase() )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataNodeService::BackupDatabase() : failed 2 backup quotation data" );
	}
}

void DataNodeService::WriteInfo( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
		MServicePlug::WriteInfo( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataNodeService::WriteWarning( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
		MServicePlug::WriteWarning( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataNodeService::WriteError( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
		MServicePlug::WriteError( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataNodeService::WriteDetail( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
		MServicePlug::WriteDetail( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}






