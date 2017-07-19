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
	CollectorStatus();

public:
	enum E_SS_Status		Get() const;

	bool					Set( enum E_SS_Status eNewStatus );

private:
	mutable CriticalObject	m_oCSLock;
	enum E_SS_Status		m_eStatus;			///< 当前行情逻辑状态，用于判断当前该做什么操作了
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
	 * @note				同步函数，即函数返回后，即初始化操作已经做完，可以判断执行结果是否为“成功”
	 * @return				==0							成功
							!=0							错误
	 */
	int						RecoverDataCollector();

	/**
	 * @brief				暂停数据采集器
	 */
	void					HaltDataCollector();

	/**
	 * @biref				取得当前数据采集模块状态
	 * @param[out]			pszStatusDesc				返回出状态描述串
	 * @param[in,out]		nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return				E_SS_Status状态值
	 */
	enum E_SS_Status		InquireDataCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief				获取市场编号
	 */
	static unsigned int		GetMarketID();

	/**
	 * @brief				是否为行情传输的采集插件
	 */
	bool					IsProxy();

	/**
	 * @brief				是否在活动中
	 */
	bool					IsAlive();

private:
	static unsigned int		s_nMarketID;					///< 数据采集器对应的市场ID
	CollectorStatus			m_oCollectorStatus;				///< 数据采集模块的状态
	bool					m_bActivated;					///< 是否已经激活
	bool					m_bIsProxyPlugin;				///< 是否为传输代理插件
private:
	Dll						m_oDllPlugin;					///< 插件加载类
	T_Func_Initialize		m_pFuncInitialize;				///< 数据采集器初始化接口
	T_Func_Release			m_pFuncRelease;					///< 数据采集器释放接口
	T_Func_RecoverQuotation	m_pFuncRecoverQuotation;		///< 数据采集器行情数据重新初始化接口
	T_Func_HaltQuotation	m_pFuncHaltQuotation;			///< 数据采集器暂停接口
	T_Func_GetStatus		m_pFuncGetStatus;				///< 数据采集器状态获取接口
	T_Func_GetMarketID		m_pFuncGetMarketID;				///< 数据采集器对应的市场ID获取接口
	T_Func_IsProxy			m_pFuncIsProxy;					///< 数据采集器对应的模块类型获取接口
};







#endif








