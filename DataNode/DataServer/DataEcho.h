#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "../Infrastructure/Lock.h"
#include "../../../DataCollector4CTPDL/DataCollector4CTPDL/CTP_DL_QuoProtocal.h"
#include "../../../DataCollector4CTPSH/DataCollector4CTPSH/CTP_SH_QuoProtocal.h"
#include "../../../DataCollector4CTPZZ/DataCollector4CTPZZ/CTP_ZZ_QuoProtocal.h"


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
 * @class						DLFuture_Echo
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
 * @class						DLFuture_Echo
 * @brief						大连商品期货CTP行情回显命令类
 * @date						2017/5/19
 * @author						barry
 */
class DLFuture_Echo : public IDataEcho
{
private:
	DLFuture_Echo();
public:
	static DLFuture_Echo&		GetSingleton();

public:///< 格式化方法
	/**
	 * @brief					根据消息ID将输入结构内容转换成格式化字符串输出
	 * @param[out]				pszEchoBuffer			输出格式化字符串缓存
	 * @param[in]				nMsgID					消息ID
	 * @param[in]				pszInputBuffer			行情结构地址
	 * @return					>0						转换成功,返回转换后的输出缓存有效数据的长度
								==0						转换失败
	 */
	static int					FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer );

protected:///< 命令调用接口
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
 * @class						SHFuture_Echo
 * @brief						上海商品期货CTP行情回显命令类
 * @date						2017/5/19
 * @author						barry
 */
class SHFuture_Echo : public IDataEcho
{
private:
	SHFuture_Echo();
public:
	static SHFuture_Echo&		GetSingleton();

public:///< 格式化方法
	/**
	 * @brief					根据消息ID将输入结构内容转换成格式化字符串输出
	 * @param[out]				pszEchoBuffer			输出格式化字符串缓存
	 * @param[in]				nMsgID					消息ID
	 * @param[in]				pszInputBuffer			行情结构地址
	 * @return					>0						转换成功,返回转换后的输出缓存有效数据的长度
								==0						转换失败
	 */
	static int					FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer );

protected:///< 命令调用接口
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
 * @class						ZZFuture_Echo
 * @brief						郑州商品期货CTP行情回显命令类
 * @date						2017/5/19
 * @author						barry
 */
class ZZFuture_Echo : public IDataEcho
{
private:
	ZZFuture_Echo();
public:
	static ZZFuture_Echo&		GetSingleton();

public:///< 格式化方法
	/**
	 * @brief					根据消息ID将输入结构内容转换成格式化字符串输出
	 * @param[out]				pszEchoBuffer			输出格式化字符串缓存
	 * @param[in]				nMsgID					消息ID
	 * @param[in]				pszInputBuffer			行情结构地址
	 * @return					>0						转换成功,返回转换后的输出缓存有效数据的长度
								==0						转换失败
	 */
	static int					FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer );

protected:///< 命令调用接口
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






