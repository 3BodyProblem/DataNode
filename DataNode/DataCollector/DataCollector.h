#ifndef __DATA_COLLECTOR_H__
#define	__DATA_COLLECTOR_H__


#include <string>
#include "../Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class				CollectorStatus
 * @brief				当前行情会话的状态
 * @detail				服务框架需要通过这个判断（组合初始化策略实例）来判断是否需要重新初始化等动作
 * @author				barry
 */
class CollectorStatus
{
public:
	enum E_QS_STATUS
	{
		E_STATUS_NONE = -1,
		E_STATUS_TODAY_IS_HOLIDAY = 0,
		E_STATUS_CONNECTION_ESTABLISHED,
		E_STATUS_CONNECTION_CLOSED,
		E_STATUS_SESSION_LOGIN,
		E_STATUS_SESSION_REBUILD,
		E_STATUS_SESSION_SUBSCRIBE,
		E_STATUS_SERVICE_IS_AVAILABLE,
	};

	CollectorStatus();

public:
	enum E_QS_STATUS		Get() const;

	bool					Set( enum E_QS_STATUS eNewStatus );

private:
	mutable CriticalObject	m_oCSLock;
	enum E_QS_STATUS		m_eStatus;			///< 当前行情逻辑状态，用于判断当前该做什么操作了
};


/**
 * @class					DataCollector
 * @brief					数据采集模块控制注册接口
 * @note					采集模块只提供三种形式的回调通知( I_DataHandle: 初始化映像数据， 实时行情数据， 初始化完成标识 ) + 重新初始化方法函数
 * @date					2017/5/3
 * @author					barry
 */
class DataCollector
{
public:
	DataCollector();

	/**
	 * @brief				数据采集模块初始化
	 * @param[in]			pIDataCallBack				行情回调接口
	 * @return				==0							成功
							!=0							错误
	 */
	int						Initialize( I_DataHandle* pIDataCallBack );

	/**
	 * @breif				数据采集模块释放退出
	 */
	void					Release();

public:///< 数据采集模块事件定义
	/**
 	 * @brief				初始化/重新初始化回调
	 * @return				==0							成功
							!=0							错误
	 */
	int						ReInitializeDataCollector();

	/**
	 * @biref				取得当前数据采集模块状态
	 */
	const CollectorStatus&	InquireDataCollectorStatus();

private:
	Dll						m_oDllPlugin;					///< 插件加载类
};







#endif








