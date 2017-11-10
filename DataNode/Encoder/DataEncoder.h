#ifndef __DATA_ENCODER_H__
#define	__DATA_ENCODER_H__


#include <string>
#include "../Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class					DataEncoder
 * @brief					数据采集模块控制注册接口
 * @note					采集模块只提供三种形式的回调通知( I_DataHandle: 初始化映像数据， 实时行情数据， 初始化完成标识 ) + 重新初始化方法函数
 * @date					2017/5/3
 * @author					barry
 */
class DataEncoder
{
public:
	DataEncoder();

	/**
	 * @brief				数据采集模块初始化
	 * @param[in]			pIDataCallBack				行情回调接口
	 * @return				==0							成功
							!=0							错误
	 */
	int						Initialize();

	/**
	 * @breif				数据采集模块释放退出
	 */
	void					Release();

private:
	Dll						m_oDllPlugin;					///< 插件加载类
/*	T_Func_Initialize		m_pFuncInitialize;				///< 数据采集器初始化接口
	T_Func_Release			m_pFuncRelease;					///< 数据采集器释放接口
	T_Func_RecoverQuotation	m_pFuncRecoverQuotation;		///< 数据采集器行情数据重新初始化接口
	T_Func_HaltQuotation	m_pFuncHaltQuotation;			///< 数据采集器暂停接口
	T_Func_GetStatus		m_pFuncGetStatus;				///< 数据采集器状态获取接口
	T_Func_GetMarketID		m_pFuncGetMarketID;				///< 数据采集器对应的市场ID获取接口
	T_Func_IsProxy			m_pFuncIsProxy;					///< 数据采集器对应的模块类型获取接口
	T_Echo					m_pFuncEcho;					///< 数据采集器对落盘数据进行解析/回显的接口*/
};







#endif








