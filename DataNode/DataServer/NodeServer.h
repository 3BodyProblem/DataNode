#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <map>
#include <string>
#include "SvrConfig.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "ServiceIO/MServicePlug.h"
#include "ServiceIO/MServicePlug.hpp"
#include "Communication/LinkSession.h"
#include "../InitializeFlag/InitFlag.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../DataCollector/DataCollector.h"


/**
 * @class					DataIOEngine
 * @brief					行情数据更新管理引擎(主要封装数据初始化和更新/推送的业务)
 * @detail					集成/协调各子模块(数据采集插件+数据内存插件+数据压缩插件)
 * @note					主要提供三块行情数据相关的基础功能: 采集进来的行情更新到内存 + 行情数据初始化控制逻辑 + 行情数据对下级的网络框架封装
							&
							其中行情的每日初始化已经考虑了节假日的情况
 * @date					2017/5/3
 * @author					barry
 */
class DataIOEngine : public I_DataHandle, public SimpleTask, public MServicePlug
{
public:///< 引擎构造和初始化相关功能
	DataIOEngine();
	~DataIOEngine();

	/**
 	 * @brief				初始化行情各参数，准备工作
	 * @note				流程中，先从本地文件加载内存插件的行情数据，再初始化行情解析插件
	 * @return				==0							成功
							!=0							失败
	 */
	int						Initialize();

	/**
	 * @brief				释放行情模块各资源
	 */
	void					Release();

public:///< I_DataHandle接口实现: 用于给数据采集模块提供行情数据的回调方法
	/**
 	 * @brief				初始化性质的行情数据回调
	 * @note				只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]			nDataID						消息ID
	 * @param[in]			pData						数据内容
	 * @param[in]			nDataLen					长度
	 * @param[in]			bLastFlag					是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				==0							成功
							!=0							错误
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				实行行情数据回调
	 * @note				更新行情内存块，并推送
	 * @param[in]			nDataID						消息ID
	 * @param[in]			pData						数据内容
	 * @param[in]			nDataLen					长度
	 * @param[in]			bPushFlag					推送标识
	 * @return				==0							成功
							!=0							错误
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief				内存数据查询接口
	 * @param[in]			nDataID						消息ID
	 * @param[in,out]		pData						数据内容(包含查询主键)
	 * @param[in]			nDataLen					长度
	 * @return				>0							成功,返回数据结构的大小
							==0							没查到结果
							!=0							错误
	 * @note				如果pData的缓存为“全零”缓存，则返回表内的所有数据
	 */
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
	 * @brief				日志函数
	 * @param[in]			nLogLevel					日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
	 * @param[in]			pszFormat					字符串格式化串
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

protected:///< 线程任务相关函数
	/**
	 * @brief				任务函数(内循环)
	 * @return				==0							成功
							!=0							失败
	 */
	virtual int				Execute();

	/**
	 * @brief				空闲状态任务: 内存数据落盘/行情超时等...
	 * @note				所以，关于内存数据落盘文件的存取，需要内存数据插件的标识接口支持
	 */
	virtual int				OnIdle() = 0;

	/**
	 * @brief				重新加载/初始化行情(内存插件、数据采集器)
	 * @detail				初始化部分的所有业务流程都在这个函数里了
	 * @return				true						初始化成功
							false						失败
	 */
	bool					PrepareQuotation();

protected:///< 私有功能函数
	/**
	 * @brief				从内存中加载所有数据表下关联的商品代码
	 * @return				>=0							成功,返回数据表数量
							<0							失败
	 */
	int						LoadCodesListInDatabase();

	/**
	 * @brief				删除内存库中的非有效商品记录
	 * @return				返回删除的数量
	 */
	int						RemoveCodeExpiredInDatabase();

protected:///< 统计成员变量
	unsigned __int64		m_nPushSerialNo;				///< 实时行情更新流水
	unsigned int			m_nHeartBeatCount;				///< 发送的心跳包数量
	CriticalObject			m_oCodeMapLock;					///< CodeMap锁
	MAP_TABLEID_CODES		m_mapID2Codes;					///< 记录各消息ID下的关联codes
protected:///< 功能成员对象
	InitializerFlag			m_oInitFlag;					///< 重新初始化标识
	SessionCollection		m_oLinkSessions;				///< 下级的链路会话
protected:///< 挂载相关插件
	PowerfullDatabase		m_oDatabaseIO;					///< 内存数据插件管理
	DataCollector			m_oDataCollector;				///< 行情采集模块接口
//	XXXCompress				m_oCompressObj;					///< 行情压缩模块
};


/**
 * @class					DataNodeService
 * @brief					行情服务器引擎	(主干类)
 * @detail					扩展作为行情服务需要的一些数据更新功能以外的逻辑功能：
							a) 服务器启/停控制
							b) 行情服务器的数据定时落盘备份
							c) 行情服务器状态定时通报
 * @note					对于挂载了传输代理插件的情况，连接原则是除了初始化，其余时间都一直保持连接不断开!
 * @date					2017/5/3
 * @author					barry
 */
class DataNodeService : public DataIOEngine
{
private:
	DataNodeService();
public:
	/**
	 * @brief				取得服务对象的单键引用
	 */
	static DataNodeService&	GetSerivceObj();

public:///< Scheduled Task
	/**
	 * @brief				空闲状态任务: 内存数据落盘/行情超时等...
	 * @note				所以，关于内存数据落盘文件的存取，需要内存数据插件的标识接口支持
	 */
	virtual int				OnIdle();

	/**
	 * @brief				心跳包：链路维持
	 */
	void					OnHeartBeat();

	/**
	 * @brief				备份内存插件中的行情数据
	 */
	void					OnBackupDatabase();

public:///< Inquiry Interface
	/**
	 * @brief				询问数据采集模块的状态
	 * @param[out]			pszStatusDesc			返回出状态描述串
	 * @param[in,out]		nStrLen					输入描述串缓存长度，输出描述串有效内容长度
	 * @return				true					可服务
	 */
	bool					OnInquireStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief				获取发送的心跳包数量
	 * @return				心跳包统计数量
	 */
	unsigned int			OnInquireHeartBeatCount();

public:///< Log Method
	/**
	 * @brief				信息
	 */
	virtual void			WriteInfo( const char * szFormat,... );

	/**
	 * @brief				警告
	 */
	virtual void			WriteWarning( const char * szFormat,... );

	/**
	 * @brief				错误
	 */
	virtual void			WriteError( const char * szFormat,... );

	/**
	 * @brief				明细
	 */
	virtual void			WriteDetail( const char * szFormat,... );
};





#endif








