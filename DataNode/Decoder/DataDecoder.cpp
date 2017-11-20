#include "DataDecoder.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"
#include "../DataServer/Communication/DataStream.h"


DataDecoder::DataDecoder()
 : m_pDecoderApi( NULL )
{
}

int DataDecoder::Initialize( std::string sPluginPath, std::string sCnfXml, unsigned int nXCodeBuffSize )
{
	Release();
	DataNodeService::GetSerivceObj().WriteInfo( "DataDecoder::Initialize() : initializing data decoder plugin ......" );

	T_Func_FetchModuleVersion	pFunGetVersion = NULL;
	T_Func_GetDecodeApi			pFuncDecodeApi = NULL;
	int							nErrorCode = m_oDllPlugin.LoadDll( sPluginPath );

	if( 0 != nErrorCode )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataDecoder::Initialize() : failed 2 load data decoder plugin [%s], errorcode=%d", sPluginPath.c_str(), nErrorCode );
		return nErrorCode;
	}

	pFunGetVersion = (T_Func_FetchModuleVersion)m_oDllPlugin.GetDllFunction( "FetchModuleVersion" );
	pFuncDecodeApi = (T_Func_GetDecodeApi)m_oDllPlugin.GetDllFunction( "GetDecodeApi" );
	if( NULL == pFunGetVersion || NULL == m_pDecoderApi )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataDecoder::Initialize() : invalid fuction pointer(NULL)" );
		return -100;
	}

	m_pDecoderApi = pFuncDecodeApi();
	DataNodeService::GetSerivceObj().WriteInfo( "DataDecoder::Initialize() : DataDecoder Version => [%s] ......", pFunGetVersion() );
	if( NULL == m_pDecoderApi )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataDecoder::Initialize() : invalid decoder obj. pointer(NULL)" );
		return -200;
	}

	if( 0 != (nErrorCode = m_pDecoderApi->Initialize( sCnfXml.c_str() )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "DataDecoder::Initialize() : failed 2 initialize data decoder, configuration file: %s", sCnfXml.c_str() );
		return nErrorCode;
	}

	DataNodeService::GetSerivceObj().WriteInfo( "DataDecoder::Initialize() : data decoder plugin is initialized ......" );

	return 0;
}

void DataDecoder::Release()
{
	if( NULL != m_pDecoderApi )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "DataDecoder::Release() : releasing data decoder plugin ......" );
		m_pDecoderApi->Release();
		delete m_pDecoderApi;
		m_pDecoderApi = NULL;
		DataNodeService::GetSerivceObj().WriteInfo( "DataDecoder::Release() : data decoder plugin is released ......" );
	}

	m_oDllPlugin.CloseDll();
}

int DataDecoder::Prepare4AUncompression( const char* pData, unsigned int nLen )
{
	if( NULL == m_pDecoderApi || NULL == pData )
	{
		return -1;
	}

	return m_pDecoderApi->Attach2Buffer( (char*)pData, nLen );
}

int DataDecoder::UncompressData( unsigned short nMsgID, char *pData, unsigned int nLen )
{
	int		nErrorCode = m_pDecoderApi->DecodeMessage( nMsgID, pData, nLen );

	if( nErrorCode <= 0 )
	{
		return nErrorCode;
	}

	return nErrorCode;
}















