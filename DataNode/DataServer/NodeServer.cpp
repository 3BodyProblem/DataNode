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

DataIOEngine::~DataIOEngine()
{
	Release();
}

int DataIOEngine::Initialize()
{
	static	char						pszError[8192] = { 0 };											///< ServicePlug��ʼ������ʱ����Ϣ
	int									nErrorCode = Configuration::GetConfigObj().Load();				///< ����������Ϣ
	const tagServicePlug_StartInParam&	refInParam = Configuration::GetConfigObj().GetStartInParam();	///< ServicePlug��ʼ������
	bool								bLoadFromDisk = (false == m_oDataCollector.IsProxy());			///< �Ƿ���Ҫ�����ݿ��жԸ�����д��뼯��ͳ��
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : [Version]  1.0.3 , ProxyModule = %d", bLoadFromDisk );
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initializing ......" );

	Release();
	if( 0 != nErrorCode )	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Initialize() : invalid configuration file, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( (nErrorCode=MServicePlug::Instance( &refInParam, pszError, sizeof(pszError) )) < 0 )	{	///< ��ʼ��������
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize serviceIO framework, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode=m_oLinkSessions.Instance()) )	{											///< ��ʼ���Ự��·����
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize link session set, errorcode=%d", nErrorCode );
		return -2;
	}

	if( 0 != (nErrorCode=m_oInitFlag.Initialize( Configuration::GetConfigObj().GetTradingPeriods()	///< ��ʼ������ʼ�����Ա�ʶ����
													, Configuration::GetConfigObj().GetHolidayFilePath(), Configuration::GetConfigObj().GetTestFlag())) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize initialize policy flag, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oDatabaseIO.Initialize()) )											///< ��ʼ�����ݿ���
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_oDatabaseIO.RecoverDatabase( m_oInitFlag.GetHoliday(), bLoadFromDisk );
	if( 0 != (nErrorCode = m_oDataCollector.Initialize( this )) )									///< ��ʼ��������
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize data collector plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	MServicePlug::RegisterSpi( &m_oLinkSessions );													///< ע������ܵĻص�����

	if( 0 != (nErrorCode = SimpleTask::Activate()) )												///< ������������߳�
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize task thread, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initialized ......" );

	return nErrorCode;
}

void DataIOEngine::Release()
{
	SimpleTask::StopAllThread();
	MServicePlug::RegisterSpi( NULL );
	MServicePlug::Release();
	m_nPushSerialNo = 0;
	m_nHeartBeatCount = 0;
	m_oDataCollector.Release();
	m_oLinkSessions.Release();
	m_oDatabaseIO.Release();
	SimpleTask::StopThread();
}

bool DataIOEngine::EnterInitializationProcess()
{
	int				nErrorCode = 0;												///< ������
	LINKID_VECTOR	vctLinkNo = { 0 };											///< ������·��
	MkHoliday&		refHoliday = m_oInitFlag.GetHoliday();						///< �ڼ��ղ��Զ��������ж��Ƿ���Ҫ��ʼ������
	unsigned int	nLinkCount = LinkNoRegister::GetRegister().FetchLinkNoTable( vctLinkNo+0, MAX_LINKID_NUM );
	bool			bLoadFromDisk = (false == m_oDataCollector.IsProxy());		///< �Ƿ���Ҫ�����ݿ��жԸ�����д��뼯��ͳ��(ֻ������ݲɼ������һ��)
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::EnterInitializationProcess() : Service is Initializing (ProxyModule=%d) ......", !bLoadFromDisk );

	///< ----------------- 0) ��������״̬ ------------------------------------
	{
		CriticalLock		guard( m_oLock );
		m_mapRecvID2Codes.clear();													///< ��յ���Ĵ��뼯�ϱ�,�ȴ�����ͳ��
		m_oDataCollector.HaltDataCollector();										///< ������ֹͣ���ݲɼ�ģ��
		for( unsigned int n = 0; n < nLinkCount; n++ )	{
			DataNodeService::GetSerivceObj().CloseLink( vctLinkNo[n] );				///< �Ͽ����ж�����·
			DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::EnterInitializationProcess() : Closing Link(No. %u) ...... ", vctLinkNo[n] );
		}
		LinkNoRegister::GetRegister().ClearAll();									///< ��������·���б�
	}

	///< ----------------- 1) �Ӵ��ָ̻���������(ͳ�Ƽ��ص�"����"��"��������") -------------------------------
	if( 0 != (nErrorCode=m_oDatabaseIO.RecoverDatabase(refHoliday, bLoadFromDisk)) )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::EnterInitializationProcess() : failed 2 recover database from disk, errorcode=%d", nErrorCode );
	}

	///< ----------------- 2) ������ģ���ʼ�� (ͳ�ƽ��յ�"����"��"��������") ----------------------------------
	if( 0 != (nErrorCode=m_oDataCollector.RecoverDataCollector()) )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::EnterInitializationProcess() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
		return false;
	}

	///< ----------------- 3) �Ƚ�(Դ����)���̺�����˵Ĵ��룬ɾ���ǵ����"����"������"����" --------------------------
	if( 0 > (nErrorCode=m_oDatabaseIO.RemoveExpiredItem4LoadFromDisk( m_mapRecvID2Codes, bLoadFromDisk )) )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::EnterInitializationProcess() : failed 2 remove expired code from database, errorcode=%d", nErrorCode );
		return false;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::EnterInitializationProcess() : MarketID(%u), Service is Initialized! ......", DataCollector::GetMarketID() );

	return true;
}

int DataIOEngine::Execute()
{
	bool			bInitPoint = false;
	unsigned int	nInitializationInterval = 1000 * Configuration::GetConfigObj().GetInitInterval();
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : enter into thread func ..." );

	while( true == IsAlive() )
	{
		try
		{
			///< �жϳ�ʼ�������Ƿ�����
			if( true == m_oInitFlag.GetFlag() )
			{
				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : [NOTICE] Enter Service Initializing Time ......" );

				///< �����ʼ������
				if( false == EnterInitializationProcess() )
				{
					m_oInitFlag.RedoInitialize();					///< ����Ϊ��Ҫ��ʼ����ʶΪ
					SimpleTask::Sleep( nInitializationInterval );	///< ���³�ʼ�������Ĭ��Ϊ3��
					continue;
				}

				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : ................. [NOTICE] Service is Available ....................." );
			}

			SimpleTask::Sleep( 1000 );								///< һ��ѭ��һ��
			OnIdle();												///< ���д�����
		}
		catch( std::exception& err )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : exception : %s", err.what() );
		}
		catch( ... )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : unknow exception ..." );
		}
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : exit thread func ..." );

	return 0;
}

int DataIOEngine::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	unsigned __int64		nSerialNo = 0;
	static	const char		s_pszZeroBuff[128*8] = { 0 };

	if( 0 == strncmp( pData, s_pszZeroBuff, min( nDataLen, sizeof(s_pszZeroBuff) ) ) )
	{
		return m_oDatabaseIO.QueryBatchRecords( nDataID, pData, nDataLen, nSerialNo );
	}
	else
	{
		return m_oDatabaseIO.QueryRecord( nDataID, pData, nDataLen, nSerialNo );
	}
}

int DataIOEngine::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	CriticalLock			guard( m_oLock );
	std::set<std::string>&	refSetCode = m_mapRecvID2Codes[nDataID];

	refSetCode.insert( std::string( pData ) );		///< ��¼���е�����Ч����Ʒ����	[���ݱ�ID,���뼯��]

	return m_oDatabaseIO.NewRecord( nDataID, pData, nDataLen, bLastFlag, m_nPushSerialNo );
}

int DataIOEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	int					nErrorCode = m_oDatabaseIO.UpdateRecord( nDataID, pData, nDataLen, m_nPushSerialNo );

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
		::printf( "[Plugin] unknow log level [%d] %s \n", nLogLevel, pszLogBuf );
		break;
	}
}


///< ----------------------------------------------------------------------------


DataNodeService::DataNodeService()
{
}

DataNodeService& DataNodeService::GetSerivceObj()
{
	static DataNodeService	obj;

	return obj;
}

int DataNodeService::OnIdle()
{
	bool				bInitPoint = false;				///< �Ƿ���Ҫ����ʼ����ʶ
	int					nPeriodIndex = m_oInitFlag.InTradingPeriod( bInitPoint );

	m_oDatabaseIO.FlushDatabase2RequestSessions( 0 );	///< ���µ�������ӣ�����"ȫ��"��ʼ����������
	OnHeartBeat();										///< ��·ά�֣����������ͼ������ʱ��������������ά��TCP����
	OnCheckConnection();								///< ��������·���ж��Ƿ���Ҫ�Ͽ����е��¼���

	///< ------------------ ���ݲɼ�ģ�����ڲ��ҵ��(Դ����) ------------------------------------------------
	if( false == m_oDataCollector.IsProxy() )
	{
		///< �ǽ���ʱ�Σ�ֹͣ("Դ����")�����ݲɼ�ģ��Ĺ���
		if( nPeriodIndex < 0 && true == m_oDataCollector.IsAlive() )
		{
			DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::OnIdle() : halting data collector ......" );
			m_oDataCollector.HaltDataCollector();
		}

		///< �ڽ���ʱ�Σ������ڴ����е�������������
		if( 0 <= nPeriodIndex && true == m_oDatabaseIO.IsBuilded() )
		{
			static time_t		s_nLastDumpTime = ::time( NULL );
			int					nNowTime = (int)::time( NULL );

			if( (nNowTime-=(int)s_nLastDumpTime) >= Configuration::GetConfigObj().GetDumpInterval() )
			{
				OnBackupDatabase();						///< ���У������Ա����������ݣ��Ա������Ӳ�ָ̻���ʱ�������
				s_nLastDumpTime = ::time( NULL );
			}
		}
	}

	return 0;
}

void DataNodeService::OnCheckConnection()
{

}

void DataNodeService::OnHeartBeat()
{
	static bool					s_bBeginCheck = false;
	static unsigned __int64		s_nPushSerialNo = 0;
	static time_t				s_nLastTime = ::time( NULL );
	unsigned int				nNowT = (unsigned int)::time( NULL );

	if( s_nPushSerialNo == m_nPushSerialNo )	///< ��������δ�䣬˵�����δ�ж�����������
	{
		if( false == s_bBeginCheck )
		{
			s_bBeginCheck = true;
			s_nLastTime = nNowT;
		}
		else
		{
			time_t	nTimeDiff = nNowT - s_nLastTime;

			if( nTimeDiff >= 5 )
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
		s_nPushSerialNo = m_nPushSerialNo;
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

	if( false == MServicePlug::IsStop() ) {
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

	if( false == MServicePlug::IsStop() ) {
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

	if( false == MServicePlug::IsStop() ) {
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

	if( false == MServicePlug::IsStop() ) {
		MServicePlug::WriteDetail( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}






