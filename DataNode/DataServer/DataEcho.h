#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "../Infrastructure/Lock.h"
#include "../Protocal/CTP_DL_QuoProtocal.h"


std::string&			Str2Lower( std::string& sStr );
bool					SplitString( char** pArgv, int& nArgc, const char* pszStr );


/**
 * @class						IDataEcho
 * @brief						行情命令回显接口
 * @date						2017/5/19
 * @author						barry
 */ 
class IDataEcho
{
public:
	/**
	 * @brief					构造
	 * @param[in]				sMarketName				市场名称
	 */
	IDataEcho( std::string sMarketName );

	/**
	 * @brief					获取市场名称
	 */
	const std::string&			GetMarketName();

public:
	/**
	 * @brief					执行解析命令进行回显操作
	 * @param[in]				pArgv					命令字数组地址
	 * @param[in]				nArgc					命令字数组长度
	 * @param[out]				szResult				回显缓存地址
	 * @param[in]				uiSize					回显缓存长度
	 * @return					true					执行成功
	 */
	bool						operator()( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize );

protected:
	/**
	 * @brief					执行解析命令进行回显操作
	 * @param[in]				pArgv					命令字数组地址
	 * @param[in]				nArgc					命令字数组长度
	 * @param[out]				szResult				回显缓存地址
	 * @param[in]				uiSize					回显缓存长度
	 * @return					true					执行成功
	 */
	virtual bool				ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize ) = 0;

protected:
	std::string					m_sMarketName;			///< 市场名称
	static const unsigned int	s_nMaxEchoBufLen;		///< 回显缓存长度
	static char*				s_pEchoDataBuf;			///< 回显缓存地址
};


/**
 * @class						CTP_DL_Echo
 * @brief						控制命令执行类
 * @date						2017/5/19
 * @author						barry
 */
class ModuleControl : public IDataEcho
{
private:
	ModuleControl();

public:
	static ModuleControl&		GetSingleton();

protected:
	/**
	 * @brief					执行解析命令进行回显操作
	 * @param[in]				pArgv					命令字数组地址
	 * @param[in]				nArgc					命令字数组长度
	 * @param[out]				szResult				回显缓存地址
	 * @param[in]				uiSize					回显缓存长度
	 * @return					true					执行成功
	 */
	virtual bool				ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize );
};


/**
 * @class						CTP_DL_Echo
 * @brief						大连CTP行情回显命令类
 * @date						2017/5/19
 * @author						barry
 */
class CTP_DL_Echo : public IDataEcho
{
private:
	CTP_DL_Echo();

public:
	static CTP_DL_Echo&			GetSingleton();

protected:
	/**
	 * @brief					执行解析命令进行回显操作
	 * @param[in]				pArgv					命令字数组地址
	 * @param[in]				nArgc					命令字数组长度
	 * @param[out]				szResult				回显缓存地址
	 * @param[in]				uiSize					回显缓存长度
	 * @return					true					执行成功
	 */
	virtual bool				ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize );
};



#endif






