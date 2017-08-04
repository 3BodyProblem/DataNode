#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#pragma warning(disable:4018)
#include <set>
#include <string>
#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"


#define			MSG_HEARTBEAT_ID	0					///< 心跳消息编号
#define			MSG_LOGIN_ID		299					///< 登录消息编号


#pragma pack(1)


/**
 * @class							tagPackageHead
 * @brief							数据包的包头结构定义
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< 自增序号
	unsigned char					nMarketID;			///< 市场编号
	unsigned short					nMsgLength;			///< 数据部分长度
	unsigned short					nMsgCount;			///< Message数量
} tagPackageHead;


/**
 * @class							tagCommonLoginData_LF299
 * @brief							登录数据块
 * @author							barry
 */
typedef struct
{
	char							pszActionKey[20];	///< 指令字符串: request:请求登录 success:登录成功 failure:登录失败
	char							pszUserName[32];	///< 用户名
	char							pszPassword[64];	///< 密码
	unsigned int					nReqDBSerialNo;		///< 请求多少流水号之后的增量数据
	char							Reserve[1024];		///< 保留
} tagCommonLoginData_LF299;


#pragma pack()


///< -------------------------------------------------------------------------------------------


/**
 * @class							PackagesLoopBuffer
 * @brief							数据包队列缓存
 * @detail							struct PkgHead + MessageID1 + data block1 + data block2 + struct PkgHead + MessageID2 + data block1 + data block2 + ...
 * @author							barry
 */
class PackagesLoopBuffer
{
public:
	PackagesLoopBuffer();
	~PackagesLoopBuffer();

public:
	/**
	 * @brief						初始化缓存对象
	 * @param[in]					nMaxBufSize				将分配的缓存大小
	 * @return						==0						成功
	 */
	int								Initialize( unsigned long nMaxBufSize );

	/**
	 * @brief						释放缓存空间
	 */
	void							Release();

public:
	/**
	 * @brief						存储数据
	 * @param[in]					nDataID					数据ID
	 * @param[in]					pData					数据指针
	 * @param[in]					nDataSize				数据长度
	 * @param[in]					nSeqNo					当前数据块的更新序号
	 * @param[out]					nMsgCount				已经压入的消息数量
	 * @param[out]					nBodySize				已经压入的消息大小累计
	 * @return						==0						成功
	 * @note						当nDataID不等于前一个包的nDataID时，将新启用一个Package封装
	 */
	int								PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, unsigned int& nMsgCount, unsigned int& nBodySize );

	/**
	 * @brief						获取一个数据包
	 * @param[out]					pBuff					输出数据缓存地址
	 * @param[in]					nBuffSize				数据缓存长度
	 * @param[out]					nMsgID					数据消息ID
	 * @return						>0						数据长度
									==0						无数据
									<0						出错
	 */
	int								GetOnePkg( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID );

	/**
	 * @brief						是否为空
	 * @return						true					为空
	 */
	bool							IsEmpty();

	/**
	 * @brief						获取余下空间的百分比
	 */
	float							GetPercentOfFreeSize();

protected:
	CriticalObject					m_oLock;				///< 锁
	char*							m_pPkgBuffer;			///< 数据包缓存地址
	unsigned int					m_nMaxPkgBufSize;		///< 数据包缓存大小
	unsigned int					m_nCurrentPkgHeadPos;	///< 当前起始位置(当前所写入包的包头)
	unsigned int					m_nFirstPkgHeadPos;		///< 起始位置索引(在写，或已经写完包的包头，即未发送数据的头部)
	unsigned int					m_nCurrentWritePos;		///< 结束位置索引(正在写入的位置)
};


/**
 * @class							PkgBuffer
 * @brief							用于保存一个完成的Package的数据
 * @author							barry
 */
class PkgBuffer
{
public:
	PkgBuffer();

	/**
	 * @brief					初始化缓存
	 * @param[in]				nBuffSize				要分配的缓存大小
	 * @return					!= 0					失败
	 */
	int							Initialize( unsigned int nBuffSize );

	/**
	 * @brief					释放资源
	 */
	void						Release();

public:
	/**
	 * @brief					将缓存地址转换出来
	 * @return					char*
	 */
	operator					char*();

	/**
	 * @brief					获取数据部分的有效长度
	 * @return					有效长度
	 */
	unsigned int				CalPkgSize() const;

	/**
	 * @brief					获取缓存的最大长度
	 * @return					最大长度
	 */
	unsigned int				MaxBufSize() const;

protected:
	char*						m_pPkgBuffer;			///< 数据发送缓存
	unsigned int				m_nMaxBufSize;			///< 发送缓存最大长度
};


///< -------------------------------------------------------------------------------------------


/**
 * @class						QuotationSynchronizer
 * @brief						行情流实时推送缓存
 * @author						barry
 */
class QuotationSynchronizer : public SimpleTask
{
public:
	/**
	 * @brief					构造函数
	 */
	QuotationSynchronizer();
	~QuotationSynchronizer();

	/**
	 * @brief					初始化行情流推送缓存
	 * @param[in]				nNewBuffSize			要分配的缓存大小
	 * @return					!= 0					失败
	 */
	int							Initialize( unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					释放各资源
	 */
	void						Release();

protected:
	/**
	 * @brief					任务函数(内循环)
	 * @return					==0						成功
								!=0						失败
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					将行情数据压缩后进行缓存
	 * @param[in]				nMsgID					Message ID
	 * @param[in]				pData					消息数据地址
	 * @param[in]				nLen					消息长度
	 * @return					> 0						成功，返回历次调用累积的序列化的长度
								<= 0					失败
	 */
	int							PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen, unsigned __int64 nSeqNo );

	/**
	 * @brief					从缓存中取数据 & 推送给下级客户端
	 * @param[in]				arrayLinkNo				链路号队列地址，需要被广播的所有链路的ID集合
	 * @param[in]				nLinkCount				链路号队列长度
	 */
	void						FlushQuotation2AllClient();

	/**
	 * @brief					获取余下空间的百分比
	 */
	float						GetFreePercent();

protected:
	WaitEvent					m_oWaitEvent;			///< 条件等待
	PackagesLoopBuffer			m_oDataBuffer;			///< 行情数据缓存队列
	PkgBuffer					m_oOnePkg;				///< 单个数据包缓存
};





#endif



