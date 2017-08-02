#pragma warning(disable : 4996)
#include <exception>
#include <algorithm>
#include <functional>
#include "SvrConfig.h"
#include "NodeServer.h"
#include "../Infrastructure/IniFile.h"
#include "../Infrastructure/DateTime.h"


char*	__BasePath(char *in)
{
	if( !in )
		return NULL;

	int	len = strlen(in);
	for( int i = len-1; i >= 0; i-- )
	{
		if( in[i] == '\\' || in[i] == '/' )
		{
			in[i + 1] = 0;
			break;
		}
	}

	return in;
}

std::string GetModulePath( void* hModule )
{
	char					szPath[MAX_PATH] = { 0 };
#ifndef LINUXCODE
		int	iRet = ::GetModuleFileName( (HMODULE)hModule, szPath, MAX_PATH );
		if( iRet <= 0 )	{
			return "";
		} else {
			return __BasePath( szPath );
		}
#else
		if( !hModule ) {
			int iRet =  readlink( "/proc/self/exe", szPath, MAX_PATH );
			if( iRet <= 0 ) {
				return "";
			} else {
				return __BasePath( szPath );
			}
		} else {
			class MDll	*pModule = (class MDll *)hModule;
			strncpy( szPath, pModule->GetDllSelfPath(), sizeof(szPath) );
			if( strlen(szPath) == 0 ) {
				return "";
			} else {
				return __BasePath(szPath);
			}
		}
#endif
}


Configuration::Configuration()
 : m_bTestFlag( false ), m_nInitializeInterval( 3 ), m_nQuotaDumpInterval( 60*5 )
{
	///< ------------- config start arguments -----------------------
	::memset( &m_oStartInParam, 0, sizeof(m_oStartInParam) );
	m_oStartInParam.uiVersion = ( (100 << 16) | 1 );							///< 服务模块版本号
//	::sprintf( m_oStartInParam.szSrvUnitName, "{%s}", "DataNodeEngine" );		///< 服务模块显示名(注意：不要填写名称)
}

int Configuration::Load()
{
	inifile::IniFile	oIniFile;
	int					nErrCode = 0;
	std::string			sIniPath = GetModulePath(NULL) + "DataNode.ini";

	///< ---------- load .ini -------------------------
	if( 0 != (nErrCode=oIniFile.load( sIniPath )) )
	{
		::printf( "Configuration::Load() : failed 2 load configuration file, %s\n", sIniPath.c_str() );
		return -1;
	}

	///< [service configuration]
	m_sMemPluginPath = oIniFile.getStringValue( std::string("Plugin"), std::string("memdb"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid memory plugin path\n" );
		return -2;
	}

	m_sDataCollectorPluginPath = oIniFile.getStringValue( std::string("Plugin"), std::string("datacollector"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid data collector plugin path\n" );
		return -3;
	}

	m_sCompressPluginPath = oIniFile.getStringValue( std::string("Plugin"), std::string("compressor"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : Without Quotation Data Compressor .............................. \n" );
	}

	m_sHolidayFilePath = GetModulePath(NULL) + oIniFile.getStringValue( std::string("Holiday"), std::string("file"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid holiday file path\n" );
		return -5;
	}
	m_sNodeInHolidayFile = oIniFile.getStringValue( std::string("Holiday"), std::string("PreName"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid section name of holiday.ini\n" );
		return -6;
	}

	int	nTestFlag = oIniFile.getIntValue( std::string("Holiday"), std::string("testflag"), nErrCode );
	m_bTestFlag = nTestFlag==1?true:false;

	///< [service framework configuration]
	m_sRecoveryFolder = oIniFile.getStringValue( std::string("ServerIO"), std::string("dumpfolder"), nErrCode );
	if( false == m_sRecoveryFolder.empty() )	{
		::printf( "Configuration::Load() : dump folder = %s\n", m_sRecoveryFolder.c_str() );
	}
	m_nInitializeInterval = oIniFile.getIntValue( std::string("ServerIO"), std::string("initinterval"), nErrCode );
	if( 0 == m_nInitializeInterval )	{
		m_nInitializeInterval = 3;
	}
	m_nQuotaDumpInterval = oIniFile.getIntValue( std::string("ServerIO"), std::string("dumpinterval"), nErrCode );
	if( 0 == m_nQuotaDumpInterval )	{
		m_nQuotaDumpInterval = 60 * 10;
	}
	m_oStartInParam.uiMaxLinkCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("maxlinkcount"), nErrCode );
	if( 0 == m_oStartInParam.uiMaxLinkCount )	{
		m_oStartInParam.uiMaxLinkCount = 128;
	}
	m_oStartInParam.uiListenPort = oIniFile.getIntValue( std::string("ServerIO"), std::string("listenport"), nErrCode );
	if( 0 == m_oStartInParam.uiListenPort )	{
		m_oStartInParam.uiListenPort = 31256;
	}
	m_oStartInParam.uiListenCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("listenqueuesize"), nErrCode );
	if( 0 == m_oStartInParam.uiListenCount )	{
		m_oStartInParam.uiListenCount = 100;
	}
	m_oStartInParam.uiSendBufCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("sendbufcount"), nErrCode );
	if( 0 == m_oStartInParam.uiSendBufCount )	{
		m_oStartInParam.uiSendBufCount = m_oStartInParam.uiMaxLinkCount*10;
	}
	m_oStartInParam.uiThreadCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("threadcount"), nErrCode );
	if( 0 == m_oStartInParam.uiThreadCount )	{
		m_oStartInParam.uiThreadCount = 20;
	}
	m_oStartInParam.uiSendTryTimes = oIniFile.getIntValue( std::string("ServerIO"), std::string("sendtrytimes"), nErrCode );
	if( 0 == m_oStartInParam.uiSendTryTimes )	{
		m_oStartInParam.uiSendTryTimes = 3;
	}
	m_oStartInParam.uiLinkTimeOut = oIniFile.getIntValue( std::string("ServerIO"), std::string("linktimeout"), nErrCode );
	if( 0 == m_oStartInParam.uiLinkTimeOut )	{
		m_oStartInParam.uiLinkTimeOut = 60;
	}
	int	nCompressFlag = oIniFile.getIntValue( std::string("ServerIO"), std::string("compressflag"), nErrCode );
	m_oStartInParam.bCompress = nCompressFlag==1?true:false;
	m_oStartInParam.bSSL = false;

	int	nDetailLog = oIniFile.getIntValue( std::string("ServerIO"), std::string("isdetaillog"), nErrCode );
	m_oStartInParam.bDetailLog = nDetailLog==1?true:false;
	m_oStartInParam.uiPageSize = oIniFile.getIntValue( std::string("ServerIO"), std::string("pagesize"), nErrCode );
	if( 0 == m_oStartInParam.uiPageSize )	{
		m_oStartInParam.uiPageSize = 1024;
	}
	m_oStartInParam.uiPageCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("pagecount"), nErrCode );
	if( 0 == m_oStartInParam.uiPageCount )	{
		m_oStartInParam.uiPageCount = 1024 * 100;
	}
	m_sCheckName = oIniFile.getStringValue( std::string("ServerIO"), std::string("loginname"), nErrCode );
	if( 0 != nErrCode )	{
		m_sCheckName = "default";
	}
	m_sCheckPassword = oIniFile.getStringValue( std::string("ServerIO"), std::string("loginpswd"), nErrCode );
	if( 0 != nErrCode )	{
		m_sCheckPassword = "default";
	}

	///< [trading periods configuration]
	int	nTradingPeriodsCount = oIniFile.getIntValue( std::string("TradingPeriods"), std::string("count"), nErrCode );
	if( nTradingPeriodsCount <= 0 )
	{
		::printf( "Configuration::Load() : missing trading periods node, size = 0 \n" );
		return -100;
	}

	m_vctTradingPeriods.clear();
	for( int n = 0; n < nTradingPeriodsCount; n++ )
	{
		int					nRet1, nRet2, nRet3;
		T_TRADING_PERIOD	tagPeriodPair;
		char				pszBeginTime[32] = { 0 };
		char				pszEndTime[32] = { 0 };
		char				pszInitPoint[32] = { 0 };

		::sprintf( pszBeginTime, "%d_begintime", n );
		::sprintf( pszEndTime, "%d_endtime", n );
		::sprintf( pszInitPoint, "%d_initflag", n );
		tagPeriodPair.nBeginTime = oIniFile.getIntValue(std::string("TradingPeriods"), std::string(pszBeginTime), nRet1 );
		tagPeriodPair.nEndTime = oIniFile.getIntValue(std::string("TradingPeriods"), std::string(pszEndTime), nRet2 );
		int	nInitPoint = oIniFile.getIntValue(std::string("TradingPeriods"), std::string(pszInitPoint), nRet3 );
		tagPeriodPair.bInitializePoint = (0!=nInitPoint)?true:false;
		if( 0 != nRet1 || 0 != nRet2 ) {
			::printf( "Configuration::Load() : failed 2 load trading periods, %s~%s [%d:%d]\n", pszBeginTime, pszEndTime, nRet1, nRet2 );
			return -101;
		}
		m_vctTradingPeriods.push_back( tagPeriodPair );
	}

	DataNodeService::GetSerivceObj().WriteInfo( "Configuration::Load() : [ServicePlugin] Configuration As Follow:\n\
												 MaxLinkCount:%d\nListenPort:%d\nListenCount:%d\nSendBufCount:%d\nThreadCount:%d\nSendTryTimes:%d\n\
												 LinkTimeOut:%d\nCompressFlag:%d\nSSLFlag:%d\nPfxFilePasswrod:%s\nDetailLog:%d\nPageSize:%d\nPageCount:%d"
												, m_oStartInParam.uiMaxLinkCount, m_oStartInParam.uiListenPort, m_oStartInParam.uiListenCount
												, m_oStartInParam.uiSendBufCount, m_oStartInParam.uiThreadCount, m_oStartInParam.uiSendTryTimes
												, m_oStartInParam.uiLinkTimeOut, m_oStartInParam.bCompress, m_oStartInParam.bSSL, m_oStartInParam.szPfxFilePasswrod
												, m_oStartInParam.bDetailLog, m_oStartInParam.uiPageSize, m_oStartInParam.uiPageCount );

	return 0;
}

Configuration& Configuration::GetConfigObj()
{
	static Configuration		obj;

	return obj;
}

unsigned int Configuration::GetDumpInterval() const
{
	return m_nQuotaDumpInterval;
}

unsigned int Configuration::GetInitInterval() const
{
	return m_nInitializeInterval;
}

bool Configuration::GetTestFlag() const
{
	return m_bTestFlag;
}

const T_VECTOR_PERIODS& Configuration::GetTradingPeriods() const
{
	return m_vctTradingPeriods;
}

const std::string& Configuration::GetRecoveryFolderPath() const
{
	return m_sRecoveryFolder;
}

const std::string& Configuration::GetHolidayFilePath() const
{
	return m_sHolidayFilePath;
}

const std::string& Configuration::GetHolidayNodeName() const
{
	return m_sNodeInHolidayFile;
}

const std::string& Configuration::GetMemPluginPath() const
{
	return m_sMemPluginPath;
}

const std::string& Configuration::GetCompressPluginPath() const
{
	return m_sCompressPluginPath;
}

const std::string& Configuration::GetDataCollectorPluginPath() const
{
	return m_sDataCollectorPluginPath;
}

const tagServicePlug_StartInParam& Configuration::GetStartInParam() const
{
	return m_oStartInParam;
}

const std::string& Configuration::GetCheckName() const
{
	return m_sCheckName;
}

const std::string& Configuration::GetCheckPassword() const
{
	return m_sCheckPassword;
}






