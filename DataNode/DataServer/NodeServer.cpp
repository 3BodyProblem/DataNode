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
	static	char						pszError[8192] = { 0 };											///< ServicePlug初始化出错时的信息
	int									nErrorCode = Configuration::GetConfigObj().Load();				///< 加载配置信息
	const tagServicePlug_StartInParam&	refInParam = Configuration::GetConfigObj().GetStartInParam();	///< ServicePlug初始化参数
	bool								bLoadFromDisk = (false == m_oDataCollector.IsProxy());			///< 是否需要从数据库中对各表进行代码集合统计
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : [Version]  1.0.3 , ProxyModule = %d", bLoadFromDisk );
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initializing ......" );

	Release();
	if( 0 != nErrorCode )	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::Initialize() : invalid configuration file, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( (nErrorCode=MServicePlug::Instance( &refInParam, pszError, sizeof(pszError) )) < 0 )	{	///< 初始化服务框架
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize serviceIO framework, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode=m_oLinkSessions.Instance()) )	{											///< 初始化会话链路管理
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize link session set, errorcode=%d", nErrorCode );
		return -2;
	}

	if( 0 != (nErrorCode=m_oInitFlag.Initialize( Configuration::GetConfigObj().GetTradingPeriods()	///< 初始化，初始化策略标识对象
													, Configuration::GetConfigObj().GetHolidayFilePath(), Configuration::GetConfigObj().GetTestFlag())) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize initialize policy flag, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oDatabaseIO.Initialize()) )											///< 初始化数据库插件
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_oDatabaseIO.RecoverDatabase( m_oInitFlag.GetHoliday(), bLoadFromDisk );
	if( 0 != (nErrorCode = m_oDataCollector.Initialize( this )) )									///< 初始化行情插件
	{
		DataNodeService::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize data collector plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	MServicePlug::RegisterSpi( &m_oLinkSessions );													///< 注册服务框架的回调对象

	if( 0 != (nErrorCode = SimpleTask::Activate()) )												///< 启动服务管理线程
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
	int				nErrorCode = 0;												///< 错误码
	LINKID_VECTOR	vctLinkNo = { 0 };											///< 发送链路表
	MkHoliday&		refHoliday = m_oInitFlag.GetHoliday();						///< 节假日策略对象，用于判断是否需要初始化行情
	unsigned int	nLinkCount = LinkNoRegister::GetRegister().FetchLinkNoTable( vctLinkNo+0, MAX_LINKID_NUM );
	bool			bLoadFromDisk = (false == m_oDataCollector.IsProxy());		///< 是否需要从数据库中对各表进行代码集合统计(只针对数据采集插件这一层)
	DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::EnterInitializationProcess() : Service is Initializing (ProxyModule=%d) ......", !bLoadFromDisk );

	///< ----------------- 0) 清理所有状态 ------------------------------------
	{
		CriticalLock		guard( m_oLock );
		m_mapRecvID2Codes.clear();													///< 清空当天的代码集合表,等待重新统计
		m_oDataCollector.HaltDataCollector();										///< 先事先停止数据采集模块
		for( unsigned int n = 0; n < nLinkCount; n++ )	{
			DataNodeService::GetSerivceObj().CloseLink( vctLinkNo[n] );				///< 断开所有对下链路
			DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::EnterInitializationProcess() : Closing Link(No. %u) ...... ", vctLinkNo[n] );
		}
		LinkNoRegister::GetRegister().ClearAll();									///< 清理所链路号列表
	}

	///< ----------------- 1) 从磁盘恢复行情数据(统计加载的"数据"和"数据类型") -------------------------------
	if( 0 != (nErrorCode=m_oDatabaseIO.RecoverDatabase(refHoliday, bLoadFromDisk)) )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::EnterInitializationProcess() : failed 2 recover database from disk, errorcode=%d", nErrorCode );
	}

	///< ----------------- 2) 从行情模块初始化 (统计接收的"数据"和"数据类型") ----------------------------------
	if( 0 != (nErrorCode=m_oDataCollector.RecoverDataCollector()) )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "DataIOEngine::EnterInitializationProcess() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
		return false;
	}

	///< ----------------- 3) 比较(源驱动)磁盘和行情端的代码，删除非当天的"数据"或数据"类型" --------------------------
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
			///< 判断初始化条件是否满足
			if( true == m_oInitFlag.GetFlag() )
			{
				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : [NOTICE] Enter Service Initializing Time ......" );

				///< 进入初始化流程
				if( false == EnterInitializationProcess() )
				{
					m_oInitFlag.RedoInitialize();					///< 重置为需要初始化标识为
					SimpleTask::Sleep( nInitializationInterval );	///< 重新初始化间隔，默认为3秒
					continue;
				}

				DataNodeService::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : ................. [NOTICE] Service is Available ....................." );
			}

			SimpleTask::Sleep( 1000 );								///< 一秒循环一次
			OnIdle();												///< 空闲处理函数
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

	refSetCode.insert( std::string( pData ) );		///< 记录所有当天有效的商品代码	[数据表ID,代码集合]

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

	switch( nLogLevel )	///< 日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
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
	bool				bInitPoint = false;				///< 是否需要做初始化标识
	int					nPeriodIndex = m_oInitFlag.InTradingPeriod( bInitPoint );

	m_oDatabaseIO.FlushDatabase2RequestSessions( 0 );	///< 对新到达的链接，推送"全量"初始化快照行情
	OnHeartBeat();										///< 链路维持：在行情推送间隔过大时，发送心跳包以维持TCP连接
	OnCheckConnection();								///< 检查对下链路（判断是否需要断开所有的下级）

	///< ------------------ 数据采集模块所在层的业务(源驱动) ------------------------------------------------
	if( false == m_oDataCollector.IsProxy() )
	{
		///< 非交易时段，停止("源驱动")的数据采集模块的工作
		if( nPeriodIndex < 0 && true == m_oDataCollector.IsAlive() )
		{
			DataNodeService::GetSerivceObj().WriteInfo( "DataNodeService::OnIdle() : halting data collector ......" );
			m_oDataCollector.HaltDataCollector();
		}

		///< 在交易时段，进行内存插件中的行情数据落盘
		if( 0 <= nPeriodIndex && true == m_oDatabaseIO.IsBuilded() )
		{
			static time_t		s_nLastDumpTime = ::time( NULL );
			int					nNowTime = (int)::time( NULL );

			if( (nNowTime-=(int)s_nLastDumpTime) >= Configuration::GetConfigObj().GetDumpInterval() )
			{
				OnBackupDatabase();						///< 盘中，周期性备份落盘数据，以备行情从硬盘恢复的时候加载用
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

	if( s_nPushSerialNo == m_nPushSerialNo )	///< 推送续号未变，说明最近未有对下行情推送
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

	///< 非交易时段的工作状态
	if( m_oInitFlag.InTradingPeriod( bInitPoint ) < 0 )
	{
		return  bDataBuilded;
	}

	///< 交易时段的工作状态
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
	///< 开始落盘备份操作
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






