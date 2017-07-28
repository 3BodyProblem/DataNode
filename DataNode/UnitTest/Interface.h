#ifndef __DATE_CLUSTER_INTERFACE_H__
#define	__DATE_CLUSTER_INTERFACE_H__


/**
 * @class					I_QuotationCallBack
 * @brief					数据回调接口
 * @date					2017/6/28
 * @author					barry
 */
class I_QuotationCallBack
{
public:
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
	virtual void			OnQuotation( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen ) = 0;

	/**
	 * @brief				内存数据查询接口
	 * @param[in]			nDataID				消息ID
	 * @param[in,out]		pData				数据内容(包含查询主键)
	 * @param[in]			nDataLen			长度
	 * @return				>0					成功,返回数据结构的大小
							==0					没查到结果
							!=0					错误
	 */
	virtual void			OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen ) = 0;

	/**
	 * @brief				日志函数
	 * @param[in]			nLogLevel			日志类型[0=信息、1=信息报告、2=警告日志、3=错误日志、4=详细日志]
	 * @param[in]			pszFormat			字符串格式化串
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf ) = 0;
};


///< -------------------------- 数据采集插件接口 ----------------------------------


/**
 * @brief					初始化数据采集模块
 * @param[in]				pIDataHandle				行情功能回调
 * @return					==0							初始化成功
							!=							出错
 */
//typedef int					(__stdcall *T_Func_Initialize)( I_DataHandle* pIDataHandle );

/**
 * @brief					释放数据采集模块
 */
//typedef void				(__stdcall *T_Func_Release)();

/**
 * @brief					获取模块的当前状态
 * @param[out]				pszStatusDesc				返回出状态描述串
 * @param[in,out]			nStrLen						输入描述串缓存长度，输出描述串有效内容长度
 * @return					返回模块当前状态值
 */
//typedef int					(__stdcall *T_Func_GetStatus)( char* pszStatusDesc, unsigned int& nStrLen );



#endif









