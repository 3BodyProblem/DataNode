#ifndef __DATE_NODE_INTERFACE_H__
#define	__DATE_NODE_INTERFACE_H__


/**
 * @class					E_QS_STATUS
 * @brief					数据采集模块的当前状态
 * @date					2017/5/3
 * @author					barry
 */
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
	 * @brief				日志函数
	 * @param[in]			nLogLevel			日志类型[0=信息、1=信息报告、2=警告日志、3=错误日志、4=详细日志]
	 * @param[in]			pszFormat			字符串格式化串
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... ) = 0;
};





#endif









