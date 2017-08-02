#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include <set>
#include <map>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../InitializeFlag/InitFlag.h"
#include "../DataServer/Communication/DataStream.h"


#define			MAX_CODE_LENGTH									32							///< 最大代码长度
typedef			std::map<unsigned int,unsigned int>				TMAP_DATAID2WIDTH;			///< map[数据ID,数据结构长度]
typedef			std::map<unsigned int,std::set<std::string>>	MAP_TABLEID_CODES;			///< 各数据表中的商品代码map[数据表ID,code集合]


/**
 * @class							DatabaseIO
 * @brief							数据库管理类
 * @date							2017/5/4
 * @author							barry
 */
class DatabaseIO
{
public:///< 初始化
	DatabaseIO();
	~DatabaseIO();

	/**
	 * @brief						初始化数据库管理对象
	 * @return						==0				成功
									!=0				错误
	 */
	int								Initialize();

	/**
	 * @brief						释放所有资源
	 */
	void							Release();

	/**
	 * @brief						单元测试
	 */
	void							UnitTest();

public:///< 状态获取
	/**
	 * @brief						判断数据表是否已经建立完成
	 * @note						包括从本地加载，和从行情中加载的都属于true的情况
									&
									此标识由调用者来设定
	 */
	bool							IsBuilded();

	/**
	 * @brief						获取数据表的数量
	 */
	unsigned int					GetTableCount();

public:///< 记录操作
	/**
	 * @brief						将数据表的数据原样copy到缓存
	 * @param[in]					nDataID					数据表ID
	 * @param[in]					pBuffer					缓存地址
	 * @param[in]					nBufferSize				缓存长度
	 * @param[in,out]				nDbSerialNo				取出>nDbSerialNo的数据(若为0,则全部取出) & 将回填最新的流水号
	 * @return						>=0						返回数据长度
									<						出错
	 */
	int								QueryBatchRecords( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo );

	/**
	 * @brief						查询实时行情数据
	 * @param[in]					nDataID				消息ID
	 * @param[in,out]				pData				数据内容
	 * @param[in]					nDataLen			长度
	 * @param[out]					nDbSerialNo			数据库新增，更新操作流水号
	 * @return						>0					成功,返回数据结构的长度
									==0					未查到数据
									!=0					错误
	 */
	int								QueryRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

	/**
	 * @brief						更新实时行情数据
	 * @param[in]					nDataID				消息ID
	 * @param[in]					pData				数据内容
	 * @param[in]					nDataLen			长度
	 * @param[out]					nDbSerialNo			数据库新增，更新操作流水号
	 * @return						>0					有更新到记录内容
									==0					成功,但没有实际更新
									!=0					错误
	 */
	int								UpdateRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

	/**
 	 * @brief						初始化性质的行情数据回调
	 * @note						只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]					nDataID				消息ID
	 * @param[in]					pData				数据内容
	 * @param[in]					nDataLen			长度
	 * @param[in]					bLastFlag			是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @param[out]					nDbSerialNo			数据库新增，更新操作流水号
	 * @return						==0					成功
									!=0					错误
	 */
	int								NewRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo );

	/**
	 * @brief						删除记录
	 * @param[in]					nDataID				数据表ID
	 * @param[in]					pData				需要删除的代码
	 * @param[in]					nDataLen			代码长度
	 * @return						>=0					返回受影响的记录数
									<0					出错
	 */
	int								DeleteRecord( unsigned int nDataID, char* pData, unsigned int nDataLen );

protected:///< 状态和元数据
	CriticalObject					m_oLock;						///< 锁
	bool							m_bBuilded;						///< 数据表是否已经初始化完成(由对象调用者设置)
	TMAP_DATAID2WIDTH				m_mapTableID;					///< 数据表的数据结构长度统计
protected:///< DB插件相关
	Dll								m_oDllPlugin;					///< 插件加载类
	IDBFactory*						m_pIDBFactoryPtr;				///< 内存数据插件库的工厂类
	I_Database*						m_pIDatabase;					///< 数据库指针
};


///< ------------------------------------------------------------------------


/**
 * @class				PowerfullDatabase
 * @brief				数据库操作扩展类
 * @date							2017/8/1
 * @author				barry
 */
class PowerfullDatabase : public DatabaseIO
{
public:///< 初始化
	PowerfullDatabase();
	~PowerfullDatabase();

	/**
	 * @brief						初始化数据库管理对象
	 * @return						==0				成功
									!=0				错误
	 */
	int								Initialize();

	/**
	 * @brief						释放所有资源
	 */
	void							Release();

public:///< 数据库恢复与备份
	/**
	 * @brief						从磁盘恢复行情数据到内存插件
	 * @detail						需要从磁盘文件恢复最近一天的行情数据（检查本地文件日期是否有效）
	 * @note						对日盘来说只能加载当天的日盘数据，对日盘来说只能加载前一个行情日期的数据
	 * @param[in]					refHoliday			节假对集合对象
	 * @param[in]					bRecoverFromDisk	是否需要从内存插件，为每个数据表统计对应的code集合(通常只针对数据采集器一层)
	 * @return						==0					成功
									!=0					失败
	 */
	int								RecoverDatabase( MkHoliday& refHoliday, bool bRecoverFromDisk );

	/**
	 * @brief						将内存插件中的行情数据进行备份
	 * @return						==0					成功
									!=0					失败
	 */
	int								BackupDatabase();

	/**
	 * @brief						删除内存库中的非有效商品记录
	 * @param[in]					mapCodeWhiteList		当前有效的各数据表ID下的code白名单
	 * @param[in]					bNeed2Erase				是否是数据采集器端，而需要本函数执行的标识
	 * @return						返回删除的数量
	 * @note						传输代码层的模块只对bNeed2Erase传递false值，以短路掉本函数
	 */
	int								RemoveCodeExpiredFromDisk( MAP_TABLEID_CODES& mapCodeWhiteList, bool bNeed2Erase );

public:///< 数据库快照数据推送
	/**
	 * @brief						将全幅初始化行情发到新到达的链路
	 * @param[in]					nSerialNo				推送查询序号(需要>nSerialNo)
	 * @return						>=0						同步的链路数
									<0						出错
	 */
	int								FlushDatabase2RequestSessions( unsigned __int64 nSerialNo = 0 );

protected:
	PkgBuffer						m_oQueryBuffer;					///< 数据发送缓存
	MAP_TABLEID_CODES				m_mapID2Codes;					///< 各数据表中的code集合统计
};



#endif









