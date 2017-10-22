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
 * @class						SendPackagePool
 * @brief						用于保存一个待发送的Package的数据
 * @author						barry
 */
class SendPackagePool : public SimpleTask
{
public:
	SendPackagePool();

	/**
	 * @brief					初始化缓存
	 * @param[in]				nBuffSize				为每个MessageID要分配的缓存大小
	 * @param[in]				nMsgCount				需要支持的Message的类别数量(需要为每个类别分配一个nBuffSize大小的缓冲区)
	 * @return					!= 0					失败
	 * @note					分配的总缓存大小 = nOneBuffSize * nMsgCount
	 */
	int							Initialize( unsigned int nOneBuffSize = 1024*1024, unsigned int nMsgCount = 20 );

	/**
	 * @brief					释放资源
	 */
	void						Release();

public:
	/**
	 * @brief					存储数据
	 * @param[in]				nDataID					数据ID
	 * @param[in]				pData					数据指针
	 * @param[in]				nDataSize				数据长度
	 * @param[in]				nSeqNo					当前数据块的更新序号
	 * @param[in]				bSendDirect				是否直接网络发送标识
	 * @return					==0						成功
	 * @note					当nDataID不等于前一个包的nDataID时，将新启用一个Package封装
	 */
	int							DispatchMessage( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, bool bSendDirect );

protected:
	/**
	 * @brief					将package中的数据整体推送下发
	 * @return					发送的数据大小
	 */
	int							SendAllPkg();

	/**
	 * @brief					任务函数(内循环)
	 * @return					==0							成功
								!=0							失败
	 */
	virtual int					Execute();

protected:
	CriticalObject				m_oLock;				///< 锁
	std::set<unsigned int>		m_setMsgID;				///< 消息ID集合
	unsigned int				m_nAllocatedTimes;		///< 已经分配过的次数(消息类型数)
	unsigned int				m_nOneMsgBufSize;		///< 一块消息缓冲区的大小
	char*						m_vctAddrMap[512];		///< 将协议号映射为在大缓存中的起始位置
	unsigned int				m_vctCurDataSize[512];	///< 有效发送数据长度
	unsigned int				m_vctMsgCount[512];		///< 每个消息的缓存数量
	unsigned int				m_vctCheckCount[512];	///< 统计每个消息缓存等待了多少次
	char*						m_pPkgBuffer;			///< 数据发送缓存
	unsigned int				m_nMaxBufSize;			///< 发送缓存最大长度
};




#endif








