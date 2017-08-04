#pragma warning(disable:4996)
#pragma warning(disable:4244)
#include "DataEcho.h"
#include "NodeServer.h"
#include "../Infrastructure/DateTime.h"


///< �ַ���תСд
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

	///< ��ջ���
	nArgc = 0;
	for( int j = 0; j < 16; j++ )
	{
		vctArgv[j] = "";
	}

	///< split�����ַ���
	for( int n = 0; n < nCmdStrLen; n++ )
	{
		char	ch = pszStr[n];

		if( ch > '!' && ch < '~' )
		{
			vctArgv[nArgc] += ch;
			bLastCharIsNotSeperator = true;
		}
		else				///< ���˵��ָ���
		{
			if( true == bLastCharIsNotSeperator && vctArgv[nArgc] != "" )
			{
				if( nArgc >= (nRawArgc-1) )
				{
					return false;
				}

				nArgc++;	///< ����һ���ַ�����
			}

			bLastCharIsNotSeperator = false;
		}
	}

	nArgc++;

	///< �����µ�������Ĳ���
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
 : IDataEcho( "ģ�������" )
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
		::sprintf( szResult, "%s", "������˵����\nhelp	�������\nreload	���³�ʼ����" );
		return true;
	}
	else if( sCmd == "push" )
	{
		::sprintf( szResult, "�����Ѳ���! [%u]", DateTime::Now().TimeToLong() );
		return true;
	}

	return false;
}


CTP_DL_Echo::CTP_DL_Echo()
 : IDataEcho( "CTP_����" )
{
}

CTP_DL_Echo& CTP_DL_Echo::GetSingleton()
{
	static CTP_DL_Echo	obj;

	return obj;
}

unsigned int CTP_DL_Echo::FormatMarketInfoLF1000( char* pszEchoBuffer, tagDLMarketInfo_LF1000& refMarketInfo )
{
	return ::sprintf( pszEchoBuffer, "�г�ID:%u, ��Ʒ����:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
}

unsigned int CTP_DL_Echo::FormatMarketStatusHF1007( char* pszEchoBuffer, tagDLMarketStatus_HF1007& refMarketStatus )
{
	return ::sprintf( pszEchoBuffer, "��������:%u, ʱ��:%u, ����״̬:%s \n", refMarketStatus.MarketDate, refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"��ʼ��":"������" );
}

unsigned int CTP_DL_Echo::FormatReferenceDataLF1003( char* pszEchoBuffer, tagDLReferenceData_LF1003& refRefData )
{
	return ::sprintf( pszEchoBuffer, "����:%s ����:%s, ��Լ����:%u, �ֱ���:%u, ������:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
}

unsigned int CTP_DL_Echo::FormatSnapDataLF1004( char* pszEchoBuffer, tagDLSnapData_LF1004& refSnapDataLF )
{
	return ::sprintf( pszEchoBuffer, "����:%s ���:%u, ���:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
}

unsigned int CTP_DL_Echo::FormatSnapDataHF1005( char* pszEchoBuffer, tagDLSnapData_HF1005& refSnapDataHF )
{
	return ::sprintf( pszEchoBuffer, "����:%s, ����:%u, ���:%u, ���:%u, ���:%f, �ɽ���:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
}

unsigned int CTP_DL_Echo::FormatBuySellDataHF1006( char* pszEchoBuffer, tagDLSnapBuySell_HF1006& refBuySellDataHF )
{
	unsigned int	nWritePos = 0;

	nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "����:%s, ", refBuySellDataHF.Code );

	for( int i = 0; i < 5; i++ )
	{
		nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "��%d��:%u, ��%d��:%I64d\t��%d��:%d,��%i��:%I64d\n"
			, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
	}

	return nWritePos;
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

		if( DataNodeService::GetSerivceObj().OnQuery( 1000, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatMarketInfoLF1000( szResult+nWritePos, tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 1007, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatMarketStatusHF1007( szResult+nWritePos, tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 1003, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagDLReferenceData_LF1003), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += CTP_DL_Echo::FormatReferenceDataLF1003( szResult+nWritePos, *((tagDLReferenceData_LF1003*)(s_pEchoDataBuf+nOffset)) );
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
		if( DataNodeService::GetSerivceObj().OnQuery( 1005, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatSnapDataHF1005( szResult+nWritePos, tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 1004, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatSnapDataLF1004( szResult+nWritePos, tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 1006, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatBuySellDataHF1006( szResult+nWritePos, tagBSHF );
	}

	return true;
}
















