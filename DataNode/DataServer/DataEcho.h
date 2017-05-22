#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "X4BaseLib/MInterface.hpp"
#include "../Server/X4BaseLib/MMutex.h"


std::string&	Str2Lower( std::string& sStr );
bool			SplitString( char** pArgv, int& nArgc, const char* pszStr );


/**
 * @file						SvrMonitor.h
 * @class						ISvrMonitor
 * @brief						服务监控接口
 * @author						barry
 */
class ISvrMonitor
{
public:
	ISvrMonitor( std::string sMarketCode );

	/**
	 * @brief					取得市场代号
	 */
	const std::string&			GetMarketCode() const;

public:
	/**
	 * @brief					监控命令接口
	 * @param[in]				szCommand				命令串
	 * @param[out]				lpOutString				输出信息地址
	 * @param[in]				uiOutSize				输出缓存长度
	 */
	virtual void				Command( const char * szCommand, char * lpOutString, unsigned int uiOutSize ) = 0;

	/**
	 * @brief					将异步命令返回的回显信息设置到对象
		 * @param[in]			nSerial			回显命令序号
		 * @param[in]			pszEcho			要设置的信息
	 */
	virtual void				SetAsyncEchoOfCmd( unsigned char nSerial, const char* pszEcho ){};

protected:
	std::string					m_sMarketCode;			///< 市场代号
};


/**
 * @class						MonitorRegister
 * @brief						监控器注册管理类
 * @author						barry
 */
class MonitorRegister
{
private:
	MonitorRegister();
public:
	/**
	 * @brief					取得本类的单键
	 */
	static MonitorRegister&		GetRegisterObj();

	/**
	 * @brief					给对应的市场，注册监控对象
	 * @param[in]				sMarketCode				市场编号
	 * @param[in]				IMonitor				监控对象地址
	 */
	void						Register( std::string sMarketCode, ISvrMonitor* IMonitor );

	/**
	 * @brief					根据 市场编号 查询监控对象
	 * @param[in]				MarketCode				市场编号
	 * @param[out]				lpOutString				如果查询失败，需要回显的信息缓存地址
	 * @return					监控对象地址
	 */
	ISvrMonitor*				GetMonitorByMarketCode( std::string &sMarketCode, char * lpOutString, unsigned int uiOutSize );

	/**
	 * @brief					根据 回显编号 找到对应 市场编号 & 查询监控对象
	 * @param[in]				nEchoSerial				回显编号
	 * @return					监控对象地址
	 */
	ISvrMonitor*				GetMonitorByEchoSerial( unsigned char nEchoSerial );

	/**
	 * @brief					设置映射对
	 * @param[in]				nEchoSerial				回显编号
	 * @param[in]				sMarketCode				市场编号
	 */
	void						SetEchoSerialOfMarketCode( unsigned char nEchoSerial, const std::string& sMarketCode );

protected:
	std::map<std::string,ISvrMonitor*>			m_mapMonitor;				///< 市场行情监控对象映射表
	std::map<unsigned char,std::string>			m_mapSerial2MarketCode;		///< 回显序号到市场编号的映射表
	MCriticalSection							m_oCS;						///< 锁
};


/**
 * @class						QuotationEcho
 * @brief						行情信息回显
 * @detail						命令行监控工具，只支持乾隆4x传输驱动体系
 */
class Tran2ndMonitor : public ISvrMonitor
{
///< -------------------- 内部类 ------------------------------------------------------------------------
	/**
	 * @class					AsyncCmdEchoTable
	 * @brief					异常行情回显命令返回表
	 * @detail					对一些需要异步延时返回的信息做缓存，并支持查询/管理
	 */
	class AsyncCmdEchoTable
	{
		typedef struct
		{
			time_t			nCmdGenTimeT;		///< 回显命令输入时间
			std::string		sEchoString;		///< 回显信息内容描述
		} T_ECHO_INFO;
	public:
		AsyncCmdEchoTable();

		/**
		 * @brief				根据异步序号，取得回显信息内容
		 * @param[in]			nSerial			回显命令序号
		 * @param[in]			nTryTimes		获取异步回显尝试次数
		 * @return				回显内容串
		 */
		std::string				GetEchoBySerialID( unsigned char nSerial, unsigned int nTryTimes = 1024 );

		/**
		 * @brief				根据回显序号更新回显信息
		 * @detail				根据回显序号更新回显信息，如果参数nTimeT为零，则不更新T_ECHO_INFO::nCmdGenTimeT
		 * @param[in]			nSerial			回显命令序号
		 * @param[in]			pszEcho			要设置的信息
		 * @param[in]			nTimeT			更新T_ECHO_INFO::nCmdGenTimeT时间，如果为零，则不更新
		 */
		void					UpdateEchoBySerialID( unsigned char nSerial, const char* pszEcho, time_t nTimeT = 0 );

	protected:
		MCriticalSection		m_oCS;					///< 锁
		std::string				m_sOverTimeEcho;		///< 异步等待超时回显
		static std::map<unsigned char,T_ECHO_INFO>		m_mapEchoInfo;
	};

	/**
	 * @class					AtomicUInteger
	 * @brief					原子常数
	 * @detail					线程安全,封装自增操作
	 * @note					自增最大值为255,超过后从0恢复
	 */
	class AtomicUInteger
	{
	public:
		AtomicUInteger();

		/**
		 * @brief				取得当前计数值
		 * @return				返回unsigned short值
		 */
		operator				short();

		/**
		 * @brief				自增值
		 * @return				返回自增后的值
		 */
		short					AddRef();

	protected:
		MCriticalSection		m_oCS;					///< 锁
		unsigned char			m_nUInteger;			///< 计数
	};
///< ----------------------------------------------------------------------------------------------------
public:
	/**
	 * @brief					构造函数
	 * @param[in]				sMarketCode				市场编号
	 * @param[in]				pControl				行情监控管理对象地址(从传输dll中导出)
	 */
	Tran2ndMonitor( std::string sMarketCode );

	/**
	 * @brief					设置控制对象指针
	 * @param[in]				pControl				行情监控管理对象地址(从传输dll中导出)
	 */
	void						SetControlBaseClassPtr( MControlBaseClass* pControl );

	/**
	 * @brief					监控命令接口
	 * @param[in]				szCommand				命令串
	 * @param[out]				lpOutString				输出信息地址
	 * @param[in]				uiOutSize				输出缓存长度
	 */
	void						Command( const char * szCommand, char * lpOutString, unsigned int uiOutSize );

	/**
	 * @brief					将异步命令返回的回显信息设置到对象
		 * @param[in]			nEchoSerial				回显命令序号
		 * @param[in]			pszEcho					要设置的信息
	 */
	void						SetAsyncEchoOfCmd( unsigned char nEchoSerial, const char* pszEcho );

protected:
	AsyncCmdEchoTable			m_AsyncEchoTb;			///< 异步回显表
	static AtomicUInteger		s_AtomicValue;			///< 异步回显标识号
	MControlBaseClass*			m_pControl;				///< 传输模块监控对象地址
};



#endif






