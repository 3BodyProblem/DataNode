#ifndef __COMMUNICATION_COMM_IO_H__
#define	__COMMUNICATION_COMM_IO_H__


#include <set>
#include "DataStream.h"
#include "../../Infrastructure/Lock.h"
#include "../ServiceIO/MServicePlug.h"
#include "../ServiceIO/MServicePlug.hpp"
#include "../../MemoryDB/MemoryDatabase.h"


/**
 * @class						LinkIDRegister
 * @brief						管理/维护各链路的ID号
 * @detail						新连接到达时添加新ID，删除失效的链路ID
 * @author						barry
 */
class LinkIDRegister
{
public:
	LinkIDRegister();

public:
	/**
	 * @brief					添加一个新到的链路ID
	 * @detail					只需要添加新ID，不需要关心是否有重复，内部有作防预性判断
	 * @param[in]				nNewLinkID			新链路ID
	 */
	int							NewLinkID( unsigned int nNewLinkID );

	/**
	 * @brief					移除一个失效的链路ID
	 * @param[in]				nRemoveLinkID		失效的ID
	 */
	void						RemoveLinkID( unsigned int nRemoveLinkID );

	/**
	 * @brief					获取当前有效的链路号列表
	 * @param[in]				lpLinkNoArray		链路号数据列表地址
	 * @param[in]				uiArraySize			列表长度
	 * @return					返回链路号数量
	 */
	unsigned int				FetchLinkIDList( unsigned int* lpLinkNoArray, unsigned int uiArraySize );

	/**
	 * @brief					获取链路数量
	 */
	int							GetLinkCount();

private:
	CriticalObject				m_oLock;				///< 锁
	std::set<unsigned int>		m_setLinkID;			///< 链路号集合,用于方便处理重复ID,和判断ID是否已经存在
	int							nLinkIDCount;			///< 链路数量
};


#define		MAX_IMAGE_BUFFER_SIZE		(1024*1024*8)


/**
 * @class						Spi4LinkCollection
 * @brief						链路数据回调/推送类
 * @detail						通讯链路会话集合事件回调逻辑
								+
								实时行情推送业务
 * @author						barry
 */
class Spi4LinkCollection : public MServicePlug_Spi
{
public:///< 构造和初始化
	Spi4LinkCollection();
	~Spi4LinkCollection();

	/**
	 * @brief					初始化
	 * @return					!= 0				失败
	 */
	int							Instance( DatabaseIO& refDbIO );

	/**
	 * @brief					释放资源
	 */
	void						Release();

public:///< 行情推送接口
	/**
	 * @brief					将实时推送的数据放进缓存
	 */
	void						PushQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize, bool bPushFlag, unsigned __int64 nSerialNo );

protected:///< 功能成员对象相关
	LinkIDRegister				m_oLinkNoTable;			///< 链路号集合
	DatabaseIO*					m_pDatabase;			///< 数据操作对象指针
	QuotationSynchronizer		m_oQuotationBuffer;		///< 实时行情推送缓存（带推送线程)
};


/**
 * @class						SessionCollection
 * @brief						继承了实时行情推送类 + 并封装了初始化行情推送
 * @author						barry
 */
class SessionCollection : public Spi4LinkCollection
{
public:
	SessionCollection();
	~SessionCollection();

	/**
	 * @brief					初始化
	 * @return					!= 0				失败
	 */
	int							Instance( DatabaseIO& refDbIO );

	/**
	 * @brief					释放资源
	 */
	void						Release();

public:///< 初始化行情推送接口
	/**
	 * @brief					将全幅初始化行情发到新到达的链路
	 * @param[in]				nSerialNo				推送查询序号(需要>nSerialNo)
	 * @return					>=0						同步的链路数
								<0						出错
	 */
	int							FlushImageData2NewSessions( unsigned __int64 nSerialNo = 0 );

	/**
	 * @brief					获取内存数据库某数据表的的所有商品主键
	 * @param[in]				nDataID					数据表ID
	 * @param[in]				nRecordLen				数据表对应数据包长度
	 * @param[out]				setCode					数据表主键集合
	 * @return					>=0						集合中的元素数量
								<0						出错
	 */
	int							QueryCodeListInDatabase( unsigned int nDataID, unsigned int nRecordLen, std::set<std::string>& setCode );

protected:///< 网络框架事件回调
	/**
	 * @brief					新连接到达响应函数，返回false表示不接受该连接，服务器会断开该连接
	 * @param[in]				uiLinkNo				链路描述号
	 * @param[in]				uiIpAddr				ip
	 * @param[in]				uiPort					端口
	 * @return					false					返回false表示不接受该连接，服务器会断开该连接
	 */
	virtual bool				OnNewLink( unsigned int uiLinkNo, unsigned int uiIpAddr, unsigned int uiPort );

	/**
	 * @brief					本进程状态响应函数，报告状态时回调
	 * @param[out]				szStatusInfo			服务器状态信息（字符串形式），服务器状态信息可填写最大控件
	 * @param[in]				uiSize					状态信息缓存的最大长度
	 * @note					具体格式如“服务单元信息\n + 服务单元信息\n + 服务单元信息\n + ...”，回车换行分割
	 *							其中 服务单元信息 = “服务单元名称:属性1,属性2,属性3,属性4,属性5,...”，逗号分割
	 *							其中 属性 = “[分组名]” 或 属性 = “working = true” 或 属性 = "keyname = value" 或 属性 = "(n)keyname = value" ，其中n表示所需单元格数量，默认1个
	 */
	virtual void				OnReportStatus( char* szStatusInfo, unsigned int uiSize );

	/**
	 * @brief					本服务进程command响应函数(用于控制运行/监视数据)
	 * @param[in]				szSrvUnitName			服务单元名称
	 * @param[in]				szCommand				命令内容
	 * @param[out]				szResult				命令返回结果
	 * @param[in]				uiSize					命令返回缓存最大长度
	 * @return					true					执行成功
	 */
	virtual bool				OnCommand( const char* szSrvUnitName, const char* szCommand, char* szResult, unsigned int uiSize );

	/**
	 * @brief					连接关闭响应函数
	 * @param[in]				uiLinkNo				链路描述号
	 * @param[in]				iCloseType				关闭类型: 0 结束通讯服务 1 WSARECV发生错误 2 服务端主动关闭 3 客户端主动关闭 4 处理数据错误而关闭
	 */
	virtual void				OnCloseLink( unsigned int uiLinkNo, int iCloseType );

	/**
	 * @brief					接收到数据响应函数
	 * @param[in]				uiLinkNo				链路描述号
	 * @param[in]				usMessageNo				消息编号
	 * @param[in]				usFunctionID			消息功能号
	 * @param[in]				bErrorFlag				错误标识
	 * @param[in]				lpData					数据缓存指针
	 * @param[in]				uiSize					数据有效长度
	 * @param[in,out]			uiAddtionData			附加数据
	 * @return					返回false表示处理数据错误，服务器会断开该链接
	 */
	virtual bool				OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData );

protected:///< 新到达的链路初始化逻辑相关
	CriticalObject				m_oLock;				///< 初始化数据推送缓存锁
	std::set<unsigned int>		m_setNewReqLinkID;		///< 待初始化链路ID集合
	unsigned int				m_nReqLinkCount;		///< 请求初始化的链路数量
	char*						m_pSendBuffer;			///< 数据发送缓存
	unsigned int				m_nMaxSendBufSize;		///< 发送缓存最大长度
};




#endif









