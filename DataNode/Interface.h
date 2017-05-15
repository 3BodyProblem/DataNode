#ifndef __DATE_NODE_INTERFACE_H__
#define	__DATE_NODE_INTERFACE_H__


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

};





#endif









