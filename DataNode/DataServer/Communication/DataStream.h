#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"


/**
 * @class						QuotationStream
 * @brief						行情流缓存
 * @author						barry
 */
class QuotationStream : public SimpleTask
{
public:
	/**
	 * @brief					构造函数
	 */
	QuotationStream();
	~QuotationStream();

	/**
	 * @brief					初始化行情流推送缓存
	 * @param[in]				nNewBuffSize		要分配的缓存大小
	 * @return					!= 0				失败
	 */
	int							Initialize( unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					释放各资源
	 */
	void						Release();

protected:
	/**
	 * @brief					任务函数(内循环)
	 * @return					==0					成功
								!=0					失败
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					将行情数据压缩后进行缓存
	 * @param[in]				nMsgID				Message ID
	 * @param[in]				pData				消息数据地址
	 * @param[in]				nLen				消息长度
	 * @return					> 0					成功，返回历次调用累积的序列化的长度
								<= 0				失败
	 */
	int							EncodeMessage( unsigned short nMsgID, const char *pData, unsigned int nLen );

	/**
	 * @brief					返回请求数据
	 * @param[in]				uiLinkNo			链路号
	 * @param[in]				eFuncID				请求返回功能号
	 */
	void						ResponseData2Platform( unsigned int uiLinkNo, enum QuotationFunctionID eFuncID );

	/**
	 * @brief					推送行情数据, 推送功能号 = FUNCTIONID_PUSH_QUOTATION
	 * @param[in]				lpLinkNoSet			链路号队列地址，需要被广播的所有链路的ID集合
	 * @param[in]				uiLinkNoCount		链路号队列长度
	 */
	void						FlushData2Platform( const unsigned int * lpLinkNoSet, unsigned int uiLinkNoCount );

protected:
	CriticalObject				m_oLock;				///< 锁
	char*						m_pszBuffer;			///< 压缩缓存空间
	unsigned int				m_nBuffSize;			///< 缓存空间大小
	unsigned int				m_nCurrSize;			///< 当前使用的空间大小
};


#endif



