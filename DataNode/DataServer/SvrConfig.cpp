#include <exception>
#include <algorithm>
#include <functional>
#include "SvrConfig.h"
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
{
}

Configuration& Configuration::GetConfigObj()
{
	static Configuration		obj;

	return obj;
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

int Configuration::Load()
{
	inifile::IniFile	oIniFile;
	int					nErrCode = 0;
	std::string			sIniPath = GetModulePath(NULL) + "srvunit\\NodeServer\\" + "NodeServer.ini";

	if( 0 != (nErrCode=oIniFile.load( sIniPath )) )
	{
		::printf( "Configuration::Load() : failed 2 load configuration file\n" );
		return -1;
	}

	return 0;
}






