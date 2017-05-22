#include "SvrMonitor.h"
#include "../CommandIO/MCommandIO.h"


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


ISvrMonitor::ISvrMonitor( std::string sMarketCode )
 : m_sMarketCode( sMarketCode )
{
}

const std::string& ISvrMonitor::GetMarketCode() const
{
	return m_sMarketCode;
}


MonitorRegister::MonitorRegister()
{
}

MonitorRegister& MonitorRegister::GetRegisterObj()
{
	static	MonitorRegister		obj;

	return obj;
}

void MonitorRegister::Register( std::string sMarketCode, ISvrMonitor* IMonitor )
{
	if( IMonitor )
	{
		std::string	sLowerCode = Str2Lower(sMarketCode);
		m_mapMonitor[sLowerCode] = IMonitor;
	}
}

void MonitorRegister::SetEchoSerialOfMarketCode( unsigned char nEchoSerial, const std::string& sMarketCode )
{
	MLocalSection			lock( &m_oCS );
	std::string				sMkCode = sMarketCode;
	std::string				sLowerCode = Str2Lower( sMkCode );

	m_mapSerial2MarketCode[nEchoSerial] = sLowerCode;
}

ISvrMonitor* MonitorRegister::GetMonitorByMarketCode( std::string &sMarketCode, char * lpOutString, unsigned int uiOutSize )
{
	MLocalSection									lock( &m_oCS );
	std::map<std::string,ISvrMonitor*>::iterator	it = m_mapMonitor.find( sMarketCode );

	if( it == m_mapMonitor.end() )		///< 未找到
	{
		ISvrMonitor* pMonitor = m_mapMonitor.begin()->second;
		if( pMonitor )
		{
			static std::string	sPrompt = "无效的市场代号,有效的市场编码列表：";

			for( it = m_mapMonitor.begin(); it != m_mapMonitor.end(); it++ )
			{
				sPrompt += it->first;
				sPrompt += " ";
			}

			::strncpy( lpOutString, sPrompt.c_str(), sPrompt.length() );
		}

		return NULL;
	}

	return it->second;					///< 找到
}

ISvrMonitor* MonitorRegister::GetMonitorByEchoSerial( unsigned char nEchoSerial )
{
	MLocalSection									lock( &m_oCS );
	std::string&									sMarketCode = m_mapSerial2MarketCode[nEchoSerial];
	std::map<std::string,ISvrMonitor*>::iterator	it = m_mapMonitor.find( sMarketCode );

	if( it == m_mapMonitor.end() )		///< 未找到
	{
		ISvrMonitor* pMonitor = m_mapMonitor.begin()->second;
		if( pMonitor )
		{
			static std::string	sPrompt = "无效的回显消息序号，有效的市场编码列表：";

			for( it = m_mapMonitor.begin(); it != m_mapMonitor.end(); it++ )
			{
				sPrompt += it->first;
				sPrompt += " ";
			}

			pMonitor->SetAsyncEchoOfCmd( nEchoSerial, sPrompt.c_str() );
		}

		return NULL;
	}

	return it->second;					///< 找到
}


std::map<unsigned char,Tran2ndMonitor::AsyncCmdEchoTable::T_ECHO_INFO> Tran2ndMonitor::AsyncCmdEchoTable::m_mapEchoInfo;

Tran2ndMonitor::AsyncCmdEchoTable::AsyncCmdEchoTable()
{
}

void Tran2ndMonitor::AsyncCmdEchoTable::UpdateEchoBySerialID( unsigned char nSerial, const char* pszEcho, time_t nTimeT )
{
	MLocalSection		lock( &m_oCS );
	T_ECHO_INFO&		refEcho = m_mapEchoInfo[nSerial];

	refEcho.sEchoString = pszEcho;
	if( nTimeT > 0 ) {
		refEcho.nCmdGenTimeT = nTimeT;
	}
}

std::string Tran2ndMonitor::AsyncCmdEchoTable::GetEchoBySerialID( unsigned char nSerial, unsigned int nTryTimes )
{
	unsigned int		n = 0;
	char				pszTmp[128] = { 0 };
	MLocalSection		lock( &m_oCS );

	for( n = 0; n < nTryTimes; n++ )
	{
		std::map<unsigned char,T_ECHO_INFO>::iterator it = m_mapEchoInfo.begin();
		if( it == m_mapEchoInfo.end() )
		{
			m_mapEchoInfo[nSerial] = T_ECHO_INFO();
			return m_mapEchoInfo[nSerial].sEchoString;
		}

		int	nDiffTime = ::time( NULL ) - it->second.nCmdGenTimeT;
		if( nDiffTime > 60 || nDiffTime < 0 )
		{
			::sprintf( pszTmp, "[echo] information expired, difftime=%d", nDiffTime );
			it->second.sEchoString = pszTmp;
		}

		return it->second.sEchoString;
	}

	::sprintf( pszTmp, "[echo] failed 2 fetch echo. trytimes=%d", n );
	m_sOverTimeEcho = pszTmp;
	return m_sOverTimeEcho;
}


Tran2ndMonitor::AtomicUInteger::AtomicUInteger()
 : m_nUInteger( 0 )
{
}

Tran2ndMonitor::AtomicUInteger::operator short()
{
	MLocalSection	lock( &m_oCS );

	return m_nUInteger;
}

short Tran2ndMonitor::AtomicUInteger::AddRef()
{
	MLocalSection	lock( &m_oCS );

	return ++m_nUInteger;
}


Tran2ndMonitor::AtomicUInteger Tran2ndMonitor::s_AtomicValue;

Tran2ndMonitor::Tran2ndMonitor( std::string sMarketCode )
 : ISvrMonitor( sMarketCode ), m_pControl( NULL )
{
}

void Tran2ndMonitor::SetAsyncEchoOfCmd( unsigned char nEchoSerial, const char* pszEcho )
{
	m_AsyncEchoTb.UpdateEchoBySerialID( nEchoSerial, pszEcho );
}

void Tran2ndMonitor::SetControlBaseClassPtr( MControlBaseClass* pControl )
{
	if( pControl )
	{
		m_pControl = pControl;
	}
}

void Tran2ndMonitor::Command( const char * szCommand, char * lpOutString, unsigned int uiOutSize )
{
	if( NULL == m_pControl || NULL == szCommand || NULL == lpOutString ) {
		return;
	}

	int				nArgc = 32;
	char*			pArgv[32] = { 0 };

	if( false == SplitString( pArgv, nArgc, szCommand ) )
	{
		::sprintf( lpOutString, "Tran2ndMonitor::Command() : [ERR] parse command string failed", uiOutSize );
		return;
	}

	///< 发送命令
	unsigned short		nEchoSerial = s_AtomicValue.AddRef();										///< 取出一个回显编号待用
	MonitorRegister::GetRegisterObj().SetEchoSerialOfMarketCode( nEchoSerial, GetMarketCode() );	///< 建立回显序号到市场编号的映射
	m_AsyncEchoTb.UpdateEchoBySerialID( nEchoSerial, "", ::time( NULL ) );							///< 记录更新命令生成时间
	m_pControl->ControlCommand( nEchoSerial, pArgv, nArgc );										///< 向传输驱动发送出命令

	///< 获取 & 设置 异步回显
	std::string&		sEcho = m_AsyncEchoTb.GetEchoBySerialID( nEchoSerial );						///< 轮询并且取得回显信息
	::strncpy( lpOutString, sEcho.c_str(), sEcho.length() );										///< 设置回显信息
}
















