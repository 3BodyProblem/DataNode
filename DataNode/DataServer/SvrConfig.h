#ifndef __DATA_NODE_SVRCONFIG_H__
#define	__DATA_NODE_SVRCONFIG_H__


#include <string>
#include <vector>
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "ServiceIO/MServicePlug.hpp"
#include "../InitializeFlag/InitFlag.h"


std::string GetModulePath( void* hModule );


/**
 * @class								Configuration
 * @brief								节点服务器的配置信息管理类
 * @date								2017/5/4
 * @author								barry
 */
class Configuration
{
private:
	Configuration();

public:
	/**
	 * @brief							取得配置对象的单键引用
	 */
	static Configuration&				GetConfigObj();

	/**
	 * @brief							初始化加载配置
	 * @return							==0				成功
										!=				失败
	 */
	int									Load();

public:
	/**
	 * @brief							节假日测试标识
	 * @return							返回节假日是否需要测试的标识
	 */
	bool								GetTestFlag() const;

	/**
	 * @brief							取得该市场的所有交易时段信息
	 */
	const T_VECTOR_PERIODS&				GetTradingPeriods() const;

	/**
	 * @brief							行情存储/恢复文件的目录
	 */
	const std::string&					GetRecoveryFolderPath() const;

	/**
	 * @brief							节假文件路径
	 */
	const std::string&					GetHolidayFilePath() const;

	/**
	 * @brief							取得对应的节假日节点名称
	 */
	const std::string&					GetHolidayNodeName() const;

	/**
	 * @brief							内存插件模块路径
	 */
	const std::string&					GetMemPluginPath() const;

	/**
	 * @brief							压缩模块插件路径
	 */
	const std::string&					GetCompressPluginPath() const;

	/**
	 * @brief							数据采集插件路径
	 */
	const std::string&					GetDataCollectorPluginPath() const;

	/**
	 * @brief							数据采集插件配置
	 */
	const std::string&					GetCompressPluginCfg() const;

	/**
	 * @brief							登录名
	 */
	const std::string&					GetCheckName() const;

	/**
	 * @brief							登录密码
	 */
	const std::string&					GetCheckPassword() const;

	/**
	 * @brief							服务框架启动参数
	 */
	const tagServicePlug_StartInParam&	GetStartInParam() const;

	/**
	 * @brief							行情落盘间隔(秒s)
	 */
	unsigned int						GetDumpInterval() const;

	/**
	 * @brief							初始化间隔
	 */
	unsigned int						GetInitInterval() const;

protected:
	tagServicePlug_StartInParam			m_oStartInParam;				///< 服务引擎启动参数

protected:
	std::string							m_sCheckName;					///< 下级登录名
	std::string							m_sCheckPassword;				///< 下级登录密码
	unsigned int						m_nInitializeInterval;			///< 初始化间隔时间(秒)
	unsigned int						m_nQuotaDumpInterval;			///< 数据快照落盘间隔(秒)
	bool								m_bTestFlag;					///< 测试标识
	std::string							m_sNodeInHolidayFile;			///< 当前模块在holiday.ini文件里读的描述节点的名称
	std::string							m_sHolidayFilePath;				///< 节假日文件路径
	std::string							m_sMemPluginPath;				///< 数据内存块插件所在路径
	std::string							m_sCompressPluginPath;			///< 数据压缩插件所在路径
	std::string							m_sCompressPluginConfig;		///< 数据压缩插件配置文件
	std::string							m_sDataCollectorPluginPath;		///< 数据采集插件所在路径
	std::string							m_sRecoveryFolder;				///< 行情数据存储/恢复目录
	T_VECTOR_PERIODS					m_vctTradingPeriods;			///< 交易时段信息表
};


#endif








