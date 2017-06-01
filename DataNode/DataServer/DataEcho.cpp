#include "DataEcho.h"
#include "NodeServer.h"


#pragma warning(disable:4996)
#pragma warning(disable:4244)


///< 字符串转小写
std::string& Str2Lower( std::string& sStr )
{
	std::transform( sStr.begin(), sStr.end(), sStr.begin(), ::tolower );

	return sStr;
}

bool SplitString( char** pArgv, int& nArgc, const char* pszStr )
{
	static std::string		vctArgv[16];
	int						nRawArgc = nArgc;
	bool					bLastCharIsNotSeperator = true;
	int						nCmdStrLen = ::strlen( pszStr );

	///< 清空缓存
	nArgc = 0;
	for( int j = 0; j < 16; j++ )
	{
		vctArgv[j] = "";
	}

	///< split命令字符串
	for( int n = 0; n < nCmdStrLen; n++ )
	{
		char	ch = pszStr[n];

		if( ch > '!' && ch < '~' )
		{
			vctArgv[nArgc] += ch;
			bLastCharIsNotSeperator = true;
		}
		else				///< 过滤掉分隔符
		{
			if( true == bLastCharIsNotSeperator && vctArgv[nArgc] != "" )
			{
				if( nArgc >= (nRawArgc-1) )
				{
					return false;
				}

				nArgc++;	///< 新启一行字符子项
			}

			bLastCharIsNotSeperator = false;
		}
	}

	nArgc++;

	///< 构造新的命令方法的参数
	for( int i = 0; i < nArgc; i++ )
	{
		pArgv[i] = (char*)vctArgv[i].c_str();
	}

	return true;
}


const unsigned int	IDataEcho::s_nMaxEchoBufLen = 1024*1024*10;
char*				IDataEcho::s_pEchoDataBuf = new char[s_nMaxEchoBufLen];


IDataEcho::IDataEcho( std::string sMarketName )
 : m_sMarketName( sMarketName )
{
}

const std::string& IDataEcho::GetMarketName()
{
	return m_sMarketName;
}

bool IDataEcho::operator()( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	if( NULL == s_pEchoDataBuf )
	{
		::sprintf( szResult, "IDataEcho::operator() : [ERR] invalid buffer pointer." );
		return true;
	}

	::memset( s_pEchoDataBuf, 0, s_nMaxEchoBufLen );

	return ExcuteCommand( pArgv, nArgc, szResult, uiSize );
}

CTP_DL_Echo::CTP_DL_Echo()
 : IDataEcho( "CTP_大连" )
{
}

bool CTP_DL_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "nametable" )
	{
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		unsigned int	nIndex = 0;

		int		nDataLen = DataNodeService::GetSerivceObj().OnQuery( 1000, s_pEchoDataBuf, s_nMaxEchoBufLen );
		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen; nOffset+=sizeof(tagCTPReferenceData), nIndex++ )
		{
			tagCTPReferenceData*	pEchoData = (tagCTPReferenceData*)(s_pEchoDataBuf+nOffset);

			if( nIndex >= nEndPos )
			{
				return true;
			}

			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. 代码:%s 名称:%s, 合约乖数:%u", nIndex+1, pEchoData->Code, pEchoData->Name, pEchoData->ContractMult );
				nWritePos += ::sprintf( szResult+nWritePos, ", 手比率:%u, 交割日:%u\n", pEchoData->LotSize, pEchoData->DeliveryDate );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string		sParam1 = pArgv[1];

		::memcpy( s_pEchoDataBuf, sParam1.c_str(), sParam1.length() );
		int		nDataLen = DataNodeService::GetSerivceObj().OnQuery( 1002, s_pEchoDataBuf, s_nMaxEchoBufLen );
		if( nDataLen > 0 )
		{
			tagCTPSnapData*		pEchoData = (tagCTPSnapData*)s_pEchoDataBuf;

			nWritePos += ::sprintf( szResult+nWritePos, "代码:%s   时间:%u\n\n", pEchoData->Code, pEchoData->DataTimeStamp );
			nWritePos += ::sprintf( szResult+nWritePos, "最新:%u, 最高:%u, 最低:%u, 昨收:%u\n", pEchoData->Now, pEchoData->High, pEchoData->Low, pEchoData->PreClose );
			nWritePos += ::sprintf( szResult+nWritePos, "昨结:%u, 今结:%u\n", pEchoData->PreSettlePrice, pEchoData->SettlePrice );
			nWritePos += ::sprintf( szResult+nWritePos, "金额:%f, 成交量:%I64d\n\n", pEchoData->Amount, pEchoData->Volume );

			for( int i = 1; i <= 5; i++ )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i, pEchoData->Buy[i].Price, i, pEchoData->Buy[i].Volume, i, pEchoData->Sell[i].Price, i, pEchoData->Sell[i].Volume );
			}

			return true;
		}
	}

	return true;
}
















