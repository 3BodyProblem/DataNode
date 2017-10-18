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
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 手比率:%u\n", refKind.KindName, refKind.LotSize );
		}
	case 102:
		{
			tagDLFutureMarketStatus_HF102&	refMarketStatus = *((tagDLFutureMarketStatus_HF102*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 103:
		{
			tagDLFutureReferenceData_LF103&	refRefData = *((tagDLFutureReferenceData_LF103*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
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
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
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
		if( DataNodeService::GetSerivceObj().OnQuery( 105, (char*)&tagSnapLF, sizeof(tagDLFutureSnapData_LF104) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 105, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 104, (char*)&tagSnapHF, sizeof(tagDLFutureSnapData_HF105) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 104, (char*)&tagSnapHF );
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
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 109:
		{
			tagSHFutureMarketStatus_HF109&	refMarketStatus = *((tagSHFutureMarketStatus_HF109*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 110:
		{
			tagSHFutureReferenceData_LF110&	refRefData = *((tagSHFutureReferenceData_LF110*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
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
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
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
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 116:
		{
			tagZZFutureMarketStatus_HF116&	refMarketStatus = *((tagZZFutureMarketStatus_HF116*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 117:
		{
			tagZZFutureReferenceData_LF117&	refRefData = *((tagZZFutureReferenceData_LF117*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
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
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
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

		if( DataNodeService::GetSerivceObj().OnQuery( 114, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 114, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 116, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 116, (char*)&tagMkStatus );
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
		if( DataNodeService::GetSerivceObj().OnQuery( 118, (char*)&tagSnapLF, sizeof(tagZZFutureSnapData_LF118) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 118, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 119, (char*)&tagSnapHF, sizeof(tagZZFutureSnapData_HF119) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 119, (char*)&tagSnapHF );
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
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName );
		}
	case 130:
		{
			tagDLOptionMarketStatus_HF130&	refMarketStatus = *((tagDLOptionMarketStatus_HF130*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 131:
		{
			tagDLOptionReferenceData_LF131&	refRefData = *((tagDLOptionReferenceData_LF131*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
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
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
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
		if( DataNodeService::GetSerivceObj().OnQuery( 132, (char*)&tagSnapLF, sizeof(tagDLOptionSnapData_LF132) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 132, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 133, (char*)&tagSnapHF, sizeof(tagDLOptionSnapData_HF133) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 133, (char*)&tagSnapHF );
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
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 137:
		{
			tagSHOptionMarketStatus_HF137&	refMarketStatus = *((tagSHOptionMarketStatus_HF137*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 138:
		{
			tagSHOptionReferenceData_LF138&	refRefData = *((tagSHOptionReferenceData_LF138*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
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
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
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
		if( DataNodeService::GetSerivceObj().OnQuery( 139, (char*)&tagSnapLF, sizeof(tagSHOptionSnapData_LF139) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 139, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 140, (char*)&tagSnapHF, sizeof(tagSHOptionSnapData_HF140) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 140, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 141, (char*)&tagBSHF, sizeof(tagSHOptionSnapBuySell_HF141) ) > 0 )
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
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 144:
		{
			tagZZOptionMarketStatus_HF144&	refMarketStatus = *((tagZZOptionMarketStatus_HF144*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 145:
		{
			tagZZOptionReferenceData_LF145&	refRefData = *((tagZZOptionReferenceData_LF145*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
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
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
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
		if( DataNodeService::GetSerivceObj().OnQuery( 146, (char*)&tagSnapLF, sizeof(tagZZOptionSnapData_LF146) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 146, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 147, (char*)&tagSnapHF, sizeof(tagZZOptionSnapData_HF147) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 147, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 148, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 148, (char*)&tagBSHF );
	}

	return true;
}


CFFFuture_Echo::CFFFuture_Echo()
 : IDataEcho( "中金商品期货" )
{
}

CFFFuture_Echo& CFFFuture_Echo::GetSingleton()
{
	static CFFFuture_Echo	obj;

	return obj;
}

int CFFFuture_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 172:
		{
			tagCFFFutureMarketInfo_LF172&	refMarketInfo = *((tagCFFFutureMarketInfo_LF172*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 173:
		{
			tagCFFFutureKindDetail_LF173&	refKind = *((tagCFFFutureKindDetail_LF173*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 174:
		{
			tagCFFFutureMarketStatus_HF174&	refMarketStatus = *((tagCFFFutureMarketStatus_HF174*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, ('0'==refMarketStatus.StatusFlag[0])?"初始化":"行情中" );
		}
	case 175:
		{
			tagCFFFutureReferenceData_LF175&	refRefData = *((tagCFFFutureReferenceData_LF175*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
		}
	case 176:
		{
			tagCFFFutureSnapData_LF176&		refSnapDataLF = *((tagCFFFutureSnapData_LF176*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 177:
		{
			tagCFFFutureSnapData_HF177&		refSnapDataHF = *((tagCFFFutureSnapData_HF177*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 178:
		{
			unsigned int					nWritePos = 0;
			tagCFFFutureSnapBuySell_HF178&	refBuySellDataHF = *((tagCFFFutureSnapBuySell_HF178*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool CFFFuture_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagCFFFutureMarketInfo_LF172		tagMkInfo = { 0 };
		tagCFFFutureMarketStatus_HF174		tagMkStatus = { 0 };

		if( DataNodeService::GetSerivceObj().OnQuery( 172, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 172, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 174, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 174, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 175, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagCFFFutureReferenceData_LF175), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 175, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagCFFFutureSnapData_LF176		tagSnapLF = { 0 };
		tagCFFFutureSnapData_HF177		tagSnapHF = { 0 };
		tagCFFFutureSnapBuySell_HF178	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 176, (char*)&tagSnapLF, sizeof(tagCFFFutureSnapData_LF176) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 176, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 177, (char*)&tagSnapHF, sizeof(tagCFFFutureSnapData_HF177) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 177, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 178, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 178, (char*)&tagBSHF );
	}

	return true;
}


SHL1_Echo::SHL1_Echo()
 : IDataEcho( "上海Level1" )
{
}

SHL1_Echo& SHL1_Echo::GetSingleton()
{
	static SHL1_Echo	obj;

	return obj;
}

int SHL1_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 149:
		{
			tagSHL1MarketInfo_LF149&	refMarketInfo = *((tagSHL1MarketInfo_LF149*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 150:
		{
			tagSHL1KindDetail_LF150&	refKind = *((tagSHL1KindDetail_LF150*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 151:
		{
			tagSHL1MarketStatus_HF151&	refMarketStatus = *((tagSHL1MarketStatus_HF151*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 152:
		{
			tagSHL1ReferenceData_LF152&	refRefData = *((tagSHL1ReferenceData_LF152*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s\n", refRefData.Code, refRefData.Name );
		}
	case 153:
		{
			tagSHL1ReferenceExtension_LF153&	refExtension = *((tagSHL1ReferenceExtension_LF153*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 停牌标识:%c \n", refExtension.Code, refExtension.StopFlag );
		}
	case 154:
		{
			tagSHL1SnapData_LF154&		refSnapDataLF = *((tagSHL1SnapData_LF154*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 今开:%u, 昨收:%u\n", refSnapDataLF.Code, refSnapDataLF.Open, refSnapDataLF.PreClose );
		}
	case 155:
		{
			tagSHL1SnapData_HF155&		refSnapDataHF = *((tagSHL1SnapData_HF155*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 156:
		{
			unsigned int					nWritePos = 0;
			tagSHL1SnapBuySell_HF156&	refBuySellDataHF = *((tagSHL1SnapBuySell_HF156*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool SHL1_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagSHL1MarketInfo_LF149				tagMkInfo = { 0 };
		tagSHL1MarketStatus_HF151			tagMkStatus = { 0 };

		if( DataNodeService::GetSerivceObj().OnQuery( 149, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 149, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 151, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 151, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 152, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagSHL1ReferenceData_LF152), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 152, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagSHL1SnapData_LF154			tagSnapLF = { 0 };
		tagSHL1SnapData_HF155			tagSnapHF = { 0 };
		tagSHL1SnapBuySell_HF156		tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 154, (char*)&tagSnapLF, sizeof(tagSHL1SnapData_LF154) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 154, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 155, (char*)&tagSnapHF, sizeof(tagSHL1SnapData_HF155) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 155, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 156, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 156, (char*)&tagBSHF );
	}

	return true;
}


SHL1Option_Echo::SHL1Option_Echo()
 : IDataEcho( "上海期权" )
{
}

SHL1Option_Echo& SHL1Option_Echo::GetSingleton()
{
	static SHL1Option_Echo	obj;

	return obj;
}

int SHL1Option_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 157:
		{
			tagSHOptMarketInfo_LF157&	refMarketInfo = *((tagSHOptMarketInfo_LF157*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 158:
		{
			tagSHOptKindDetail_LF158&	refKind = *((tagSHOptKindDetail_LF158*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 159:
		{
			tagSHOptMarketStatus_HF159&	refMarketStatus = *((tagSHOptMarketStatus_HF159*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, ('0'==refMarketStatus.StatusFlag[0])?"初始化":"行情中" );
		}
	case 160:
		{
			tagSHOptReferenceData_LF160&	refRefData = *((tagSHOptReferenceData_LF160*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 名称:%s, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.DeliveryDate );
		}
	case 161:
		{
			tagSHOptSnapData_LF161&		refSnapDataLF = *((tagSHOptSnapData_LF161*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 162:
		{
			tagSHOptSnapData_HF162&		refSnapDataHF = *((tagSHOptSnapData_HF162*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 163:
		{
			unsigned int					nWritePos = 0;
			tagSHOptSnapBuySell_HF163&		refBuySellDataHF = *((tagSHOptSnapBuySell_HF163*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool SHL1Option_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagSHOptMarketInfo_LF157			tagMkInfo = { 0 };
		tagSHOptMarketStatus_HF159			tagMkStatus = { 0 };

		if( DataNodeService::GetSerivceObj().OnQuery( 157, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 157, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 159, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 159, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 160, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagSHOptReferenceData_LF160), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 160, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagSHOptSnapData_LF161			tagSnapLF = { 0 };
		tagSHOptSnapData_HF162			tagSnapHF = { 0 };
		tagSHOptSnapBuySell_HF163		tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 161, (char*)&tagSnapLF, sizeof(tagSHOptSnapData_LF161) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 161, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 162, (char*)&tagSnapHF, sizeof(tagSHOptSnapData_HF162) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 162, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 163, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 163, (char*)&tagBSHF );
	}

	return true;
}


SZL1_Echo::SZL1_Echo()
 : IDataEcho( "深圳Level1" )
{
}

SZL1_Echo& SZL1_Echo::GetSingleton()
{
	static SZL1_Echo	obj;

	return obj;
}

int SZL1_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 164:
		{
			tagSZL1MarketInfo_LF164&	refMarketInfo = *((tagSZL1MarketInfo_LF164*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 165:
		{
			tagSZL1KindDetail_LF165&	refKind = *((tagSZL1KindDetail_LF165*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u\n", refKind.KindName );
		}
	case 166:
		{
			tagSZL1MarketStatus_HF166&	refMarketStatus = *((tagSZL1MarketStatus_HF166*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "时间:%u, 行情状态:%s \n", refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 167:
		{
			tagSZL1ReferenceData_LF167&	refRefData = *((tagSZL1ReferenceData_LF167*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s\n", refRefData.Code, refRefData.Name );
		}
	case 168:
		{
			tagSZL1ReferenceExtension_LF168&	refExtension = *((tagSZL1ReferenceExtension_LF168*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 停牌标识:%c \n", refExtension.Code, refExtension.StopFlag );
		}
	case 169:
		{
			tagSZL1SnapData_LF169&		refSnapDataLF = *((tagSZL1SnapData_LF169*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 今开:%u, 昨收:%u\n", refSnapDataLF.Code, refSnapDataLF.Open, refSnapDataLF.PreClose );
		}
	case 170:
		{
			tagSZL1SnapData_HF170&		refSnapDataHF = *((tagSZL1SnapData_HF170*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 171:
		{
			unsigned int					nWritePos = 0;
			tagSZL1SnapBuySell_HF171&		refBuySellDataHF = *((tagSZL1SnapBuySell_HF171*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i+1, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i+1, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool SZL1_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagSZL1MarketInfo_LF164				tagMkInfo = { 0 };
		tagSZL1MarketStatus_HF166			tagMkStatus = { 0 };

		if( DataNodeService::GetSerivceObj().OnQuery( 164, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 164, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 166, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 166, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 167, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagSZL1ReferenceData_LF167), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 167, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagSZL1SnapData_LF169			tagSnapLF = { 0 };
		tagSZL1SnapData_HF170			tagSnapHF = { 0 };
		tagSZL1SnapBuySell_HF171		tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 169, (char*)&tagSnapLF, sizeof(tagSZL1SnapData_LF169) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 169, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 170, (char*)&tagSnapHF, sizeof(tagSZL1SnapData_HF170) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 170, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 171, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += FormatStruct2OutputBuffer( szResult+nWritePos, 171, (char*)&tagBSHF );
	}

	return true;
}









