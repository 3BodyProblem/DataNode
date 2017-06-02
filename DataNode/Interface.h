#ifndef __DATE_NODE_INTERFACE_H__
#define	__DATE_NODE_INTERFACE_H__


/**
 * @class	E_SS_Status
 * @brief	CTP行情会话状态枚举
 */
enum E_SS_Status
{
	ET_SS_UNACTIVE = 0,				///< 未激活:	需要对Session调用Initialize()
	ET_SS_DISCONNECTED,				///< 断开状态
	ET_SS_CONNECTED,				///< 连通状态
	ET_SS_LOGIN,					///< 登录成功
    ET_SS_INITIALIZING,				///< 初始化码表/快照中
	ET_SS_INITIALIZED,				///< 初始化完成
	ET_SS_WORKING,					///< 正常工作中
};


/**
 * @class					I_DataHandle
 * @brief					数据回调接口
 * @date					2017/5/3
 * @author					barry
 */
class I_DataHandle
{
public:
	/**
 	 * @brief				初始化性质的行情数据回调
	 * @note				只是更新构造好行情数据的内存初始结构，不推送
							&
							会导致框架模块的重新初始化状态
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @param[in]			bLastFlag			是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag ) = 0;

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
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag ) = 0;

	/**
	 * @brief				内存数据查询接口
	 * @param[in]			nDataID				消息ID
	 * @param[in,out]		pData				数据内容(包含查询主键)
	 * @param[in]			nDataLen			长度
	 * @return				>0					成功,返回数据结构的大小
							==0					没查到结果
							!=0					错误
	 */
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen ) = 0;

	/**
	 * @brief				日志函数
	 * @param[in]			nLogLevel			日志类型[0=信息、1=信息报告、2=警告日志、3=错误日志、4=详细日志]
	 * @param[in]			pszFormat			字符串格式化串
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... ) = 0;
};





typedef int					(__stdcall *T_Func_Initialize)( I_DataHandle* pIDataHandle );
typedef void				(__stdcall *T_Func_Release)();
typedef int					(__stdcall *T_Func_RecoverQuotation)();
typedef void				(__stdcall *T_Func_HaltQuotation)();
typedef int					(__stdcall *T_Func_GetStatus)();
typedef int					(__stdcall *T_Func_GetMarketID)();




#endif









