#pragma warning(disable:4996)
#pragma warning(disable:4244)
#include "DataEcho.h"
#include "NodeServer.h"
#include "../Infrastructure/DateTime.h"


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

ModuleControl::ModuleControl()
 : IDataEcho( "模块控制器" )
{
}

ModuleControl& ModuleControl::GetSingleton()
{
	static	ModuleControl obj;

	return obj;
}

bool ModuleControl::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "help" )
	{
		::sprintf( szResult, "%s", "命令字说明：\nhelp	帮助命令。\nreload	重新初始化。" );
		return true;
	}
	else if( sCmd == "push" )
	{
		::sprintf( szResult, "数据已补发! [%u]", DateTime::Now().TimeToLong() );
		return true;
	}

	return false;
}


CTP_DL_Echo::CTP_DL_Echo()
 : IDataEcho( "CTP_大连" )
{
}

CTP_DL_Echo& CTP_DL_Echo::GetSingleton()
{
	static CTP_DL_Echo	obj;

	return obj;
}

unsigned int CTP_DL_Echo::FormatMarketInfo( char* pszEchoBuffer, tagDLMarketInfo_LF1000& refMarketInfo )
{
	return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
}

unsigned int CTP_DL_Echo::FormatMarketStatus( char* pszEchoBuffer, tagDLMarketStatus_HF1007& refMarketStatus )
{
	return ::sprintf( pszEchoBuffer, "行情日期:%u, 时间:%u, 行情状态:%s \n", refMarketStatus.MarketID, refMarketStatus.WareCount, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
}

bool CTP_DL_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagDLMarketInfo_LF1000			tagMkInfo = { 0 };
		tagDLMarketStatus_HF1007		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		int		a = DataNodeService::GetSerivceObj().OnQuery( 1000, (char*)&tagMkInfo, sizeof(tagMkInfo) );
		int		b = DataNodeService::GetSerivceObj().OnQuery( 1007, (char*)&tagMkStatus, sizeof(tagMkStatus) );

		if( a > 0 && b > 0 )
		{
			nWritePos += CTP_DL_Echo::FormatMarketStatus( szResult+nWritePos, tagMkStatus );
			nWritePos += CTP_DL_Echo::FormatMarketInfo( szResult+nWritePos, tagMkInfo );
		}
	}
	else if( sCmd == "nametable" )
	{
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		unsigned int	nIndex = 0;

		int		nDataLen = DataNodeService::GetSerivceObj().OnQuery( 1003, s_pEchoDataBuf, s_nMaxEchoBufLen );
		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen; nOffset+=sizeof(tagDLReferenceData_LF1003), nIndex++ )
		{
			tagDLReferenceData_LF1003*	pEchoData = (tagDLReferenceData_LF1003*)(s_pEchoDataBuf+nOffset);

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
		std::string				sParam1 = pArgv[1];
		tagDLSnapData_LF1004	tagSnapLF = { 0 };
		tagDLSnapData_HF1005	tagSnapHF = { 0 };
		tagDLSnapBuySell_HF1006	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		int		a = DataNodeService::GetSerivceObj().OnQuery( 1005, (char*)&tagSnapHF, sizeof(tagSnapHF) );
		int		b = DataNodeService::GetSerivceObj().OnQuery( 1004, (char*)&tagSnapLF, sizeof(tagSnapLF) );
		int		c = DataNodeService::GetSerivceObj().OnQuery( 1006, (char*)&tagBSHF, sizeof(tagBSHF) );

		if( a > 0 && b > 0 && c > 0 )
		{
			nWritePos += ::sprintf( szResult+nWritePos, "代码:%s \n\n", tagSnapHF.Code );
			nWritePos += ::sprintf( szResult+nWritePos, "最新:%u, 最高:%u, 最低:%u\n", tagSnapHF.Now, tagSnapHF.High, tagSnapHF.Low );
			nWritePos += ::sprintf( szResult+nWritePos, "昨结:%u, 今结:%u\n", tagSnapLF.PreSettlePrice, tagSnapLF.SettlePrice );
			nWritePos += ::sprintf( szResult+nWritePos, "金额:%f, 成交量:%I64d\n\n", tagSnapHF.Amount, tagSnapHF.Volume );

			for( int i = 1; i <= 5; i++ )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i, tagBSHF.Buy[i].Price, i, tagBSHF.Buy[i].Volume, i, tagBSHF.Sell[i].Price, i, tagBSHF.Sell[i].Volume );
			}

			return true;
		}
	}

	return true;
}
















