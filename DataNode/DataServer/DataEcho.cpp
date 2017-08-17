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


DLFuture_Echo::DLFuture_Echo()
 : IDataEcho( "CTP_大连商品期货" )
{
}

DLFuture_Echo& DLFuture_Echo::GetSingleton()
{
	static DLFuture_Echo	obj;

	return obj;
}

int DLFuture_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 100:
		{
			tagDLFutureMarketInfo_LF100&	refMarketInfo = *((tagDLFutureMarketInfo_LF100*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 101:
		{
			tagDLFutureKindDetail_LF101&	refKind = *((tagDLFutureKindDetail_LF101*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 102:
		{
			tagDLFutureMarketStatus_HF102&	refMarketStatus = *((tagDLFutureMarketStatus_HF102*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 103:
		{
			tagDLFutureReferenceData_LF103&	refRefData = *((tagDLFutureReferenceData_LF103*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 合约乖数:%u, 手比率:%u, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 104:
		{
			tagDLFutureSnapData_LF104&		refSnapDataLF = *((tagDLFutureSnapData_LF104*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 105:
		{
			tagDLFutureSnapData_HF105&		refSnapDataHF = *((tagDLFutureSnapData_HF105*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 106:
		{
			unsigned int					nWritePos = 0;
			tagDLFutureSnapBuySell_HF106&	refBuySellDataHF = *((tagDLFutureSnapBuySell_HF106*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool DLFuture_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagDLFutureMarketInfo_LF100			tagMkInfo = { 0 };
		tagDLFutureMarketStatus_HF102		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		if( DataNodeService::GetSerivceObj().OnQuery( 100, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 100, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 102, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 102, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 103, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagDLFutureReferenceData_LF103), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 103, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagDLFutureSnapData_LF104		tagSnapLF = { 0 };
		tagDLFutureSnapData_HF105		tagSnapHF = { 0 };
		tagDLFutureSnapBuySell_HF106	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 105, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 105, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 104, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 104, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 106, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 106, (char*)&tagBSHF );
	}

	return true;
}


SHFuture_Echo::SHFuture_Echo()
 : IDataEcho( "上海商品期货" )
{
}

SHFuture_Echo& SHFuture_Echo::GetSingleton()
{
	static SHFuture_Echo	obj;

	return obj;
}

int SHFuture_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 107:
		{
			tagSHFutureMarketInfo_LF107&	refMarketInfo = *((tagSHFutureMarketInfo_LF107*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 108:
		{
			tagSHFutureKindDetail_LF108&	refKind = *((tagSHFutureKindDetail_LF108*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 109:
		{
			tagSHFutureMarketStatus_HF109&	refMarketStatus = *((tagSHFutureMarketStatus_HF109*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 110:
		{
			tagSHFutureReferenceData_LF110&	refRefData = *((tagSHFutureReferenceData_LF110*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 合约乖数:%u, 手比率:%u, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 111:
		{
			tagSHFutureSnapData_LF111&		refSnapDataLF = *((tagSHFutureSnapData_LF111*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 112:
		{
			tagSHFutureSnapData_HF112&		refSnapDataHF = *((tagSHFutureSnapData_HF112*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 113:
		{
			unsigned int					nWritePos = 0;
			tagSHFutureSnapBuySell_HF113&	refBuySellDataHF = *((tagSHFutureSnapBuySell_HF113*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool SHFuture_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagSHFutureMarketInfo_LF107			tagMkInfo = { 0 };
		tagSHFutureMarketStatus_HF109		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		if( DataNodeService::GetSerivceObj().OnQuery( 107, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 107, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 109, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 109, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 110, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagSHFutureReferenceData_LF110), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 110, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagSHFutureSnapData_LF111		tagSnapLF = { 0 };
		tagSHFutureSnapData_HF112		tagSnapHF = { 0 };
		tagSHFutureSnapBuySell_HF113	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 111, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 111, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 112, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 112, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 113, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 113, (char*)&tagBSHF );
	}

	return true;
}


ZZFuture_Echo::ZZFuture_Echo()
 : IDataEcho( "郑州商品期货" )
{
}

ZZFuture_Echo& ZZFuture_Echo::GetSingleton()
{
	static ZZFuture_Echo	obj;

	return obj;
}

int ZZFuture_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 114:
		{
			tagZZFutureMarketInfo_LF114&	refMarketInfo = *((tagZZFutureMarketInfo_LF114*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 115:
		{
			tagZZFutureKindDetail_LF115&	refKind = *((tagZZFutureKindDetail_LF115*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 116:
		{
			tagZZFutureMarketStatus_HF116&	refMarketStatus = *((tagZZFutureMarketStatus_HF116*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 117:
		{
			tagZZFutureReferenceData_LF117&	refRefData = *((tagZZFutureReferenceData_LF117*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 合约乖数:%u, 手比率:%u, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 118:
		{
			tagZZFutureSnapData_LF118&		refSnapDataLF = *((tagZZFutureSnapData_LF118*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 119:
		{
			tagZZFutureSnapData_HF119&		refSnapDataHF = *((tagZZFutureSnapData_HF119*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 120:
		{
			unsigned int					nWritePos = 0;
			tagZZFutureSnapBuySell_HF120&	refBuySellDataHF = *((tagZZFutureSnapBuySell_HF120*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool ZZFuture_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagZZFutureMarketInfo_LF114			tagMkInfo = { 0 };
		tagZZFutureMarketStatus_HF116		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		if( DataNodeService::GetSerivceObj().OnQuery( 114, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 107, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 116, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 109, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 117, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagZZFutureReferenceData_LF117), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 117, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagZZFutureSnapData_LF118		tagSnapLF = { 0 };
		tagZZFutureSnapData_HF119		tagSnapHF = { 0 };
		tagZZFutureSnapBuySell_HF120	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 118, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 118, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 119, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 119, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 120, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 120, (char*)&tagBSHF );
	}

	return true;
}


DLOption_Echo::DLOption_Echo()
 : IDataEcho( "大连商品期权" )
{
}

DLOption_Echo& DLOption_Echo::GetSingleton()
{
	static DLOption_Echo	obj;

	return obj;
}

int DLOption_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 128:
		{
			tagDLOptionMarketInfo_LF128&	refMarketInfo = *((tagDLOptionMarketInfo_LF128*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 129:
		{
			tagDLOptionKindDetail_LF129&	refKind = *((tagDLOptionKindDetail_LF129*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 130:
		{
			tagDLOptionMarketStatus_HF130&	refMarketStatus = *((tagDLOptionMarketStatus_HF130*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 131:
		{
			tagDLOptionReferenceData_LF131&	refRefData = *((tagDLOptionReferenceData_LF131*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 合约乖数:%u, 手比率:%u, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 132:
		{
			tagDLOptionSnapData_LF132&		refSnapDataLF = *((tagDLOptionSnapData_LF132*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 133:
		{
			tagDLOptionSnapData_HF133&		refSnapDataHF = *((tagDLOptionSnapData_HF133*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 134:
		{
			unsigned int					nWritePos = 0;
			tagDLOptionSnapBuySell_HF134&	refBuySellDataHF = *((tagDLOptionSnapBuySell_HF134*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool DLOption_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagDLOptionMarketInfo_LF128			tagMkInfo = { 0 };
		tagDLOptionMarketStatus_HF130		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		if( DataNodeService::GetSerivceObj().OnQuery( 128, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 128, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 130, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 130, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 131, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagDLOptionReferenceData_LF131), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 131, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagDLOptionSnapData_LF132		tagSnapLF = { 0 };
		tagDLOptionSnapData_HF133		tagSnapHF = { 0 };
		tagDLOptionSnapBuySell_HF134	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 132, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 132, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 133, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 133, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 134, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 134, (char*)&tagBSHF );
	}

	return true;
}


SHOption_Echo::SHOption_Echo()
 : IDataEcho( "上海商品期权" )
{
}

SHOption_Echo& SHOption_Echo::GetSingleton()
{
	static SHOption_Echo	obj;

	return obj;
}

int SHOption_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 135:
		{
			tagSHOptionMarketInfo_LF135&	refMarketInfo = *((tagSHOptionMarketInfo_LF135*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 136:
		{
			tagSHOptionKindDetail_LF136&	refKind = *((tagSHOptionKindDetail_LF136*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 137:
		{
			tagSHOptionMarketStatus_HF137&	refMarketStatus = *((tagSHOptionMarketStatus_HF137*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 138:
		{
			tagSHOptionReferenceData_LF138&	refRefData = *((tagSHOptionReferenceData_LF138*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 合约乖数:%u, 手比率:%u, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 139:
		{
			tagSHOptionSnapData_LF139&		refSnapDataLF = *((tagSHOptionSnapData_LF139*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 140:
		{
			tagSHOptionSnapData_HF140&		refSnapDataHF = *((tagSHOptionSnapData_HF140*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 141:
		{
			unsigned int					nWritePos = 0;
			tagSHOptionSnapBuySell_HF141&	refBuySellDataHF = *((tagSHOptionSnapBuySell_HF141*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool SHOption_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagSHOptionMarketInfo_LF135			tagMkInfo = { 0 };
		tagSHOptionMarketStatus_HF137		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		if( DataNodeService::GetSerivceObj().OnQuery( 135, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 135, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 137, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 137, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 138, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagSHOptionReferenceData_LF138), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 138, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagSHOptionSnapData_LF139		tagSnapLF = { 0 };
		tagSHOptionSnapData_HF140		tagSnapHF = { 0 };
		tagSHOptionSnapBuySell_HF141	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 139, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 139, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 140, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 140, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 141, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 141, (char*)&tagBSHF );
	}

	return true;
}


ZZOption_Echo::ZZOption_Echo()
 : IDataEcho( "郑州商品期权" )
{
}

ZZOption_Echo& ZZOption_Echo::GetSingleton()
{
	static ZZOption_Echo	obj;

	return obj;
}

int ZZOption_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 142:
		{
			tagZZOptionMarketInfo_LF142&	refMarketInfo = *((tagZZOptionMarketInfo_LF142*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 143:
		{
			tagZZOptionKindDetail_LF143&	refKind = *((tagZZOptionKindDetail_LF143*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 144:
		{
			tagZZOptionMarketStatus_HF144&	refMarketStatus = *((tagZZOptionMarketStatus_HF144*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 145:
		{
			tagZZOptionReferenceData_LF145&	refRefData = *((tagZZOptionReferenceData_LF145*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 合约乖数:%u, 手比率:%u, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 146:
		{
			tagZZOptionSnapData_LF146&		refSnapDataLF = *((tagZZOptionSnapData_LF146*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 147:
		{
			tagZZOptionSnapData_HF147&		refSnapDataHF = *((tagZZOptionSnapData_HF147*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 148:
		{
			unsigned int					nWritePos = 0;
			tagZZOptionSnapBuySell_HF148&	refBuySellDataHF = *((tagZZOptionSnapBuySell_HF148*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool ZZOption_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagZZOptionMarketInfo_LF142			tagMkInfo = { 0 };
		tagZZOptionMarketStatus_HF144		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		if( DataNodeService::GetSerivceObj().OnQuery( 142, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 142, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 144, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 144, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 145, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagZZOptionReferenceData_LF145), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 145, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagZZOptionSnapData_LF146		tagSnapLF = { 0 };
		tagZZOptionSnapData_HF147		tagSnapHF = { 0 };
		tagZZOptionSnapBuySell_HF148	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 146, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 146, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 147, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 147, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 148, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 148, (char*)&tagBSHF );
	}

	return true;
}







