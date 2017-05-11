#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <string>
#include "SvrConfig.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "ServiceIO/MServicePlug.h"
#include "ServiceIO/MServicePlug.hpp"
#include "../InitializeFlag/InitFlag.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../DataCollector/DataCollector.h"


/**
 * @class					SvrFramework
 * @brief					服务框架类
 * @detail					封装了服务注册、数据收发、状态通知等基础功能
 * @date					2017/5/2
 * @author					barry
 */
class SvrFramework : public MServicePlug
{
private:
	SvrFramework();

public:
	~SvrFramework();

	/**
	 * @brief				取得服务框架插件
	 * @return				返回服务框架插件的对象引用
	 */
	static SvrFramework&	GetFramework();

	/**
	 * @brief				初始化服务框架
	 * @note				在加载完配置文件后，将马上被调用
	 * @return				==0					成功
							!=0					失败
	 */
	int						Initialize();
};


/**
 * @class					DataEngine
 * @brief					全数据会话管理引擎
 * @detail					集成/协调各子模块(数据采集插件+数据内存插件+数据压缩插件)
 * @date					2017/5/3
 * @author					barry
 */
class DataEngine : public I_DataHandle, public SimpleTask
{
public:
	DataEngine();

	/**
 	 * @brief				初始化行情各参数，准备工作
	 * @note				流程中，先从本地文件加载内存插件的行情数据，再初始化行情解析插件
	 * @param[in]			sDataCollectorPluginPath	行情解析插件路径
	 * @param[in]			sMemPluginPath				行情数据内存插件路径
	 * @param[in]			sHolidayPath				节假日文件路径
	 * @return				==0							成功
							!=0							失败
	 */
	int						Initialize( const std::string& sDataCollectorPluginPath, const std::string& sMemPluginPath, const std::string& sHolidayPath );

	/**
	 * @brief				释放行情模块各资源
	 */
	void					Release();

public:///< I_DataHandle接口实现: 用于给数据采集模块提供行情数据的回调方法
	/**
 	 * @brief				初始化性质的行情数据回调
	 * @note				只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @param[in]			bLastFlag			是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				实行行情数据回调
	 * @note				更新行情内存块，并推送
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @param[in]			bPushFlag			推送标识
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief				根据消息ID/Code查询某数据内容(通过内存数据插件接口)
	 * @param[in]			nDataID				消息ID
	 * @param[in,out]		pData				商品Code[in],数据查询返回[out]
	 * @param[in]			nDataLen			缓存长度
	 * @param[in]			bLastFlag			是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				true				查询成功
	 */
	virtual bool			OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

protected:
	/**
	 * @brief				任务函数(内循环)
	 * @return				==0					成功
							!=0					失败
	 */
	virtual int				Execute();

	/**
	 * @brief				空闲状态任务: 内存数据落盘/行情超时等...
	 * @note				所以，关于内存数据落盘文件的存取，需要内存数据插件的标识接口支持
	 */
	virtual int				OnIdle() = 0;

protected:
	DatabaseIO				m_oDatabaseIO;					///< 内存数据插件管理
	DataCollector			m_oDataCollector;				///< 行情采集模块接口
//	XXXCompress				m_oCompressObj;					///< 行情压缩模块
};


/**
 * @class					DataNodeService
 * @brief					行情服务器引擎	(主干类)
 * @detail					涉及的业务方向：
							a) 数据采集插件的每天自动初始化
							b) 数据采集插件通知回调方式，将初始化数据+行情数据通知给Engine对象
							c) Engine对象维持好各阶段的行情业务状态，并对状态作出正确的反应(如重连，重新初始化,空闲...)
							d) Engine对象自动将内存中的行情数据落盘
							e) Engine对象初始化时自动将行情落盘数据更新到内存对象(注：落盘数据无差别载入，即内存中保存的一定是最后一次可以拿到的行情数据)
							f) Engine对象提供对Dll导出接口的支持
							g) 需要有节假日逻辑支持
							h) 服务除了初始化阶段，其他时间都应该为可服务状态
 * @date					2017/5/3
 * @author					barry
 */
class DataNodeService : public DataEngine
{
public:
	DataNodeService();
	~DataNodeService();

public:
	/**
	 * @brief				初始化&启动行情服务
	 * @return				==0				启动成功
							!=0				启动出错
	 */
	int						Activate();

	/**
	 * @brief				销毁行情服务
	 */
	void					Destroy();

public:
	/**
	 * @brief				空闲状态任务: 内存数据落盘/行情超时等...
	 * @note				所以，关于内存数据落盘文件的存取，需要内存数据插件的标识接口支持
	 */
	virtual int				OnIdle();

	/**
	 * @brief				备份内存插件中的行情数据
	 */
	void					OnBackupDatabase();

	/**
	 * @brief				询问数据采集模块的状态
	 */
	void					OnInquireStatus();

	/**
	 * @brief				行情引擎是否可提供对下服务
	 * @return				true		行情初始化成功，可服务
							false		未初始化/正初始化中，不可服务
	 */
	bool					IsAvailable();
};





#endif








