#include "DataEncoder.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"


DataEncoder::DataEncoder()
 : m_pEncoderApi( NULL ), m_pXCodeBuffer( NULL ), m_nMaxBufferLen( 0 ), m_nDataLen( 0 )
{
}

int DataEncoder::Initialize( std::string sPluginPath, std::string sCnfXml, unsigned int nXCodeBuffSize )
{
	Release();
	DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Initialize() : initializing data encoder plugin ......" );

	T_Func_FetchModuleVersion	pFunGetVersion = NULL;
	T_Func_GetEncodeApi			pFuncEncodeApi = NULL;
	int							nErrorCode = m_oDllPlugin.LoadDll( sPluginPath );

	if( 0 != nErrorCode )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataEncoder::Initialize() : failed 2 load data encoder plugin [%s], errorcode=%d", sPluginPath.c_str(), nErrorCode );
		return nErrorCode;
	}

	m_pXCodeBuffer = new char[nXCodeBuffSize];
	if( NULL == m_pXCodeBuffer )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataEncoder::Initialize() : failed 2 allocate encoder buffer" );
		return -1;
	}

	m_nMaxBufferLen = nXCodeBuffSize;
	pFunGetVersion = (T_Func_FetchModuleVersion)m_oDllPlugin.GetDllFunction( "FetchModuleVersion" );
	pFuncEncodeApi = (T_Func_GetEncodeApi)m_oDllPlugin.GetDllFunction( "GetEncodeApi" );

	if( NULL == pFunGetVersion || NULL == pFuncEncodeApi )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataEncoder::Initialize() : invalid fuction pointer(NULL)" );
		return -100;
	}

	m_pEncoderApi = pFuncEncodeApi();
	DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Initialize() : DataEncoder Version => [%s] ......", pFunGetVersion() );
	if( NULL == m_pEncoderApi )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataEncoder::Initialize() : invalid encoder obj. pointer(NULL)" );
		return -200;
	}

	if( 0 != (nErrorCode = m_pEncoderApi->Initialize( sCnfXml.c_str() )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataEncoder::Initialize() : failed 2 initialize data encoder, configuration file: %s", sCnfXml.c_str() );
		return nErrorCode;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Initialize() : data encoder plugin is initialized ......" );

	return 0;
}

void DataEncoder::Release()
{
	if( NULL != m_pEncoderApi )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Release() : releasing data encoder plugin ......" );
		m_pEncoderApi->Release();
		delete m_pEncoderApi;
		m_pEncoderApi = NULL;
		DataNodeService::GetSerivceObj().WriteInfo( "DataEncoder::Release() : data encoder plugin is released ......" );
	}

	if( NULL != m_pXCodeBuffer )
	{
		delete [] m_pXCodeBuffer;
		m_pXCodeBuffer = NULL;
		m_nMaxBufferLen = 0;
		m_nDataLen = 0;
	}

	m_oDllPlugin.CloseDll();
}

const char* DataEncoder::GetBufferPtr()
{
	return m_pXCodeBuffer;
}

unsigned int DataEncoder::GetBufferLen()
{
	return m_nDataLen;
}















