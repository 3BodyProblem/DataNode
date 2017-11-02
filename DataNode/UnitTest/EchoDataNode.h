#ifndef __QLX_DATA_CLUSTER_TEST_H__
#define __QLX_DATA_CLUSTER_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "../../../DataCluster/DataCluster/Interface.h"
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
	virtual void			OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus );
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf );

protected:
	int								m_nMessageID;					///< 只显示的消息ID
	std::string						m_sMessageKey;					///< 只显示的记录Code

protected:
	Dll								m_oDllPlugin;					///< 插件加载类
	tagQUOFun_StartWork				m_funcActivate;					///< 插件启动函数
	tagQUOFun_EndWork				m_funcDestroy;					///< 插件停止函数
	tagQUOFun_GetMarketID			m_funcGetMarketID;				///< 插件市场ID表查询函数
	tagQUOFun_GetMarketInfo			m_funcGetMarketInfo;			///< 插件查询函数
	tagQUOFun_GetAllReferenceData	m_funcGetAllRefData;			///< 获取所有参考数据
	tagQUOFun_GetReferenceData		m_funcGetRefData;				///< 获取参考数据
	tagQUOFun_GetAllSnapData		m_funcGetAllSnapData;			///< 获取所有快照数据
	tagQUOFun_GetSnapData			m_funcGetSnapData;				///< 获取快照数据
	T_Func_ExecuteUnitTest			m_funcUnitTest;					///< 插件测试函数
};




#endif





