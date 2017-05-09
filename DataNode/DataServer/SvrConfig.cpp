#include <exception>
#include <algorithm>
#include <functional>
#include "SvrConfig.h"
#include "NodeServer.h"
#include "../Infrastructure/IniFile.h"


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
 : m_bTestFlag( false )
{
	///< ------------- config start arguments -----------------------
	::memset( &m_oStartInParam, 0, sizeof(m_oStartInParam) );
	m_oStartInParam.uiVersion = ( (100 << 16) | 1 );							///< 服务模块版本号
	::sprintf( m_oStartInParam.szSrvUnitName, "{%s}", "DataNodeEngine" );		///< 服务模块显示名
}

int Configuration::Load()
{
	inifile::IniFile	oIniFile;
	int					nErrCode = 0;
	std::string			sIniPath = GetModulePath(NULL) + "srvunit\\DataNode\\" + "DataNode.ini";

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

	m_sHolidayFilePath = oIniFile.getStringValue( std::string("Holiday"), std::string("file"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid holiday file path\n" );
		return -5;
	}
	int	nTestFlag = oIniFile.getIntValue( std::string("Holiday"), std::string("testflag"), nErrCode );
	m_bTestFlag = nTestFlag==1?true:false;

	///< [service framework configuration]
	m_sRecoveryFolder = oIniFile.getStringValue( std::string("ServerIO"), std::string("dumpfolder"), nErrCode );
	if( false == m_sRecoveryFolder.empty() )	{
		::printf( "Configuration::Load() : dump folder = %s\n", m_sRecoveryFolder.c_str() );
	}
	m_oStartInParam.uiMaxLinkCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("maxlinkcount"), nErrCode );
	if( 0 == m_oStartInParam.uiMaxLinkCount )	{
		m_oStartInParam.uiMaxLinkCount = 8;
	}
	m_oStartInParam.uiListenPort = oIniFile.getIntValue( std::string("ServerIO"), std::string("listenport"), nErrCode );
	if( 0 == m_oStartInParam.uiListenPort )	{
		m_oStartInParam.uiListenPort = 31256;
	}
	m_oStartInParam.uiListenCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("listenqueuesize"), nErrCode );
	if( 0 == m_oStartInParam.uiListenCount )	{
		m_oStartInParam.uiListenCount = m_oStartInParam.uiMaxLinkCount;
	}
	m_oStartInParam.uiSendBufCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("sendbufcount"), nErrCode );
	if( 0 == m_oStartInParam.uiSendBufCount )	{
		m_oStartInParam.uiSendBufCount = 1024*1024*8;
	}
	m_oStartInParam.uiThreadCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("threadcount"), nErrCode );
	if( 0 == m_oStartInParam.uiThreadCount )	{
		m_oStartInParam.uiThreadCount = 1;
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
	int	nSSLFlag = oIniFile.getIntValue( std::string("ServerIO"), std::string("sslflag"), nErrCode );
	m_oStartInParam.bSSL = nSSLFlag==1?true:false;
	std::string	sPfxFilePswd = oIniFile.getStringValue( std::string("ServerIO"), std::string("pfxfilepassword"), nErrCode );
	::strncpy( m_oStartInParam.szPfxFilePasswrod, sPfxFilePswd.c_str(), sPfxFilePswd.length() );
	int	nDetailLog = oIniFile.getIntValue( std::string("ServerIO"), std::string("isdetaillog"), nErrCode );
	m_oStartInParam.bDetailLog = nDetailLog==1?true:false;
	m_oStartInParam.uiPageSize = oIniFile.getIntValue( std::string("ServerIO"), std::string("pagesize"), nErrCode );
	if( 0 == m_oStartInParam.uiPageSize )	{
		m_oStartInParam.uiPageSize = 1024*1024;
	}
	m_oStartInParam.uiPageCount = oIniFile.getIntValue( std::string("ServerIO"), std::string("pagecount"), nErrCode );
	if( 0 == m_oStartInParam.uiPageCount )	{
		m_oStartInParam.uiPageCount = 10;
	}

	///< ---------- initialize framework -------------------------
	if( 0 != SvrFramework::GetFramework().Initialize() )
	{
		::printf( "Configuration::Load() : failed 2 initialize server framework\n" );
		return -100;
	}

	return 0;
}

Configuration& Configuration::GetConfigObj()
{
	static Configuration		obj;

	return obj;
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






