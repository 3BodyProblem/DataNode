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

int CTP_DL_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 100:
		{
			tagDLFutureMarketInfo_LF100&	refMarketInfo = *((tagDLFutureMarketInfo_LF100*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "�г�ID:%u, ��Ʒ����:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 101:
		{
			tagDLFutureKindDetail_LF101&	refKind = *((tagDLFutureKindDetail_LF101*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "��������:%u, ������Ʒ��:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 102:
		{
			tagDLFutureMarketStatus_HF102&	refMarketStatus = *((tagDLFutureMarketStatus_HF102*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "��������:%u, ʱ��:%u, ����״̬:%s \n", refMarketStatus.MarketDate, refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"��ʼ��":"������" );
		}
	case 103:
		{
			tagDLFutureReferenceData_LF103&	refRefData = *((tagDLFutureReferenceData_LF103*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "����:%s ����:%s, ��Լ����:%u, �ֱ���:%u, ������:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 104:
		{
			tagDLFutureSnapData_LF104&		refSnapDataLF = *((tagDLFutureSnapData_LF104*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "����:%s ���:%u, ���:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 105:
		{
			tagDLFutureSnapData_HF105&		refSnapDataHF = *((tagDLFutureSnapData_HF105*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "����:%s, ����:%u, ���:%u, ���:%u, ���:%f, �ɽ���:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 106:
		{
			unsigned int					nWritePos = 0;
			tagDLFutureSnapBuySell_HF106&	refBuySellDataHF = *((tagDLFutureSnapBuySell_HF106*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "����:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "��%d��:%u, ��%d��:%I64d\t��%d��:%d,��%i��:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool CTP_DL_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
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
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 100, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 102, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 102, (char*)&tagMkStatus );
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
				nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 103, s_pEchoDataBuf+nOffset );
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
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 105, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 104, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 104, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 106, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 106, (char*)&tagBSHF );
	}

	return true;
}
















