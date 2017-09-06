#ifndef __QLX_DATA_CLUSTER_TEST_H__
#define __QLX_DATA_CLUSTER_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class					DataClusterPlugin
 * @brief					数据收集客户端测试封装
 * @author					barry
 */
class DataClusterPlugin : public I_QuotationCallBack
{
public:
	DataClusterPlugin();
	~DataClusterPlugin();

public:
	/**
	 * @brief				数据收集客户端的行情回显测试
	 * @param[in]			nMsgID				只显示的MessageID, -1表示不过滤
	 * @param[in]			sKey				只显示的记录主键， ""表示不过滤
	 * @return				>=0					成功
							<0					出错
	 */
	int						TestQuotationEcho( int nMsgID, std::string sKey );

protected:
	/**
	 * @brief				初始化并启动模块
	 * @return				true				成功
	 */
	bool					Initialize();

	/**
	 * @brief				停止并释放模块
	 */
	void					Release();

protected:
	virtual void			OnQuotation( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf );

protected:
	int						m_nMessageID;					///< 只显示的消息ID
	std::string				m_sMessageKey;					///< 只显示的记录Code

protected:
	Dll						m_oDllPlugin;					///< 插件加载类
	T_Func_Activate			m_funcActivate;					///< 插件启动函数
	T_Func_Destroy			m_funcDestroy;					///< 插件停止函数
	T_Func_Query			m_funcQuery;					///< 插件查询函数
	T_Func_ExecuteUnitTest	m_funcUnitTest;					///< 插件测试函数
};




#endif





