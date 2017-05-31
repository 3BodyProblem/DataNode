#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#pragma warning(disable:4018)
#include <set>
#include <string>
#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"
#include "../../MemoryDB/MemoryDatabase.h"


#pragma pack(1)

/**
 * @class							tagPackageHead
 * @brief							数据包的包头结构定义
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< 自增序号
	unsigned int					nBodyLen;			///< 数据部分长度
	unsigned int					nMsgCount;			///< 包内的Message数量
} tagPackageHead;


/**
 * @class							tagBlockHead
 * @brief							数据块的头部的定义
 * @author							barry
 */
typedef struct
{
	unsigned int					nDataType;			///< 数据块类型
	unsigned int					nDataLen;			///< 数据块长度
} tagBlockHead;

#pragma pack()


/**
 * @class							PackagesBuffer
 * @brief							数据包队列缓存
 * @detail							struct PkgHead + data block1 + data block2 + ...
 * @author							barry
 */
class PackagesBuffer
{
public:
	PackagesBuffer();
	~PackagesBuffer();

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
	 * @return						==0						成功
	 */
	int								PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo );

	/**
	 * @brief						获取一个数据包
	 * @param[out]					pBuff					输出数据缓存地址
	 * @param[in]					nBuffSize				数据缓存长度
	 * @return						>0						数据长度
									==0						无数据
									<0						出错
	 */
	int								GetOnePkg( char* pBuff, unsigned int nBuffSize );

	/**
	 * @brief						是否为空
	 * @return						true					为空
	 */
	bool							IsEmpty();

protected:
	CriticalObject					m_oLock;				///< 锁
	char*							m_pPkgBuffer;			///< 数据包缓存地址
	unsigned int					m_nMaxPkgBufSize;		///< 数据包缓存大小
	unsigned long					m_nFirstPosition;		///< 起始位置索引
	unsigned long					m_nLastPosition;		///< 结束位置索引
};


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
	 * @param[in]				lpLinkNoSet				链路号队列地址，需要被广播的所有链路的ID集合
	 * @param[in]				uiLinkNoCount			链路号队列长度
	 */
	void						FlushQuotation2Client();

protected:
	WaitEvent					m_oWaitEvent;			///< 条件等待
	PackagesBuffer				m_oDataBuffer;			///< 数据缓存队列
	char*						m_pSendBuffer;			///< 数据发送缓存
	unsigned int				m_nMaxSendBufSize;		///< 发送缓存最大长度
};


/**
 * @class						ImageRebuilder
 * @brief						初始化行情流缓存
 * @author						barry
 */
class ImageRebuilder
{
private:
	ImageRebuilder();

public:
	/**
	 * @brief					获取单键对象的引用
	 */
	static ImageRebuilder&		GetRebuilder();

	/**
	 * @brief					初始化行情初始化流缓存
	 * @return					!= 0					失败
	 */
	int							Initialize();

	/**
	 * @brief					释放各资源
	 */
	void						Release();

public:
	/**
	 * @brief					获取内存数据库某数据表的的所有商品主键
	 * @param[in]				nDataID					数据表ID
	 * @param[in]				refDatabaseIO			数据库插件引用
	 * @param[out]				setCode					数据表主键集合
	 * @return					>=0						集合中的元素数量
								<0						出错
	 */
	int							QueryCodeListInDatabase( unsigned int nDataID, DatabaseIO& refDatabaseIO, std::set<std::string>& setCode );

	/**
	 * @brief					将所有数据同步/初始化到所有客户端链路
	 * @param[in]				refDatabaseIO			数据库插件引用
	 * @param[in]				nSerialNo				推送查询序号(需要>nSerialNo)
	 * @return					>=0						同步的链路数
								<0						出错
	 */
	int							Flush2ReqSessions( DatabaseIO& refDatabaseIO, unsigned __int64 nSerialNo = 0 );

	/**
	 * @brief					获取待初始化的新链路数量
	 */
	unsigned int				GetReqSessionCount();

	/**
	 * @brief					增加一个新的待初始化推送的链路号
	 * @param[in]				nLinkNo					链路号
	 * @return					true					增加成功
								false					失败，有重复项
	 */
	bool						AddNewReqSession( unsigned int nLinkNo );

protected:
	CriticalObject				m_oBuffLock;			///< 初始化数据推送缓存锁
	std::set<unsigned int>		m_setNewReqLinkID;		///< 待初始化链路ID集合
	unsigned int				m_nReqLinkCount;		///< 请求初始化的链路数量
	PackagesBuffer				m_oDataBuffer;			///< 数据缓存队列
	char*						m_pSendBuffer;			///< 数据发送缓存
	unsigned int				m_nMaxSendBufSize;		///< 发送缓存最大长度
};




#endif



