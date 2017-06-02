#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include <set>
#include <map>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../InitializeFlag/InitFlag.h"


#define			MAX_CODE_LENGTH							32						///< 最大代码长度
typedef			std::map<unsigned int,unsigned int>		TMAP_DATAID2WIDTH;		///< map[数据ID,数据结构长度]


/**
 * @class							DatabaseIO
 * @brief							数据库管理类
 * @date							2017/5/4
 * @author							barry
 */
class DatabaseIO
{
public:
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

public:
	/**
	 * @brief						取得存在的数据表id列表
	 * @param[out]					pIDList			数据表id列表指针
	 * @param[in]					nMaxListSize	数据表的长度
	 * @param[out]					pWidthList		各数据表结构宽度信息列表
	 * @param[in]					nMaxWidthlistLen	列表最大长度
	 * @return						返回实际的数据表数量
	 */
	unsigned int					GetTablesID( unsigned int* pIDList, unsigned int nMaxListSize, unsigned int* pWidthList = NULL, unsigned int nMaxWidthlistLen = 0 );

	/**
	 * @brief						将数据表的数据原样copy到缓存
	 * @param[in]					nDataID					数据表ID
	 * @param[in]					pBuffer					缓存地址
	 * @param[in]					nBufferSize				缓存长度
	 * @param[in,out]				nDbSerialNo				取出>nDbSerialNo的数据(若为0,则全部取出) & 将回填最新的流水号
	 * @return						>=0						返回数据长度
									<						出错
	 */
	int								FetchRecordsByID( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo );

	/**
	 * @brief						判断数据表是否已经建立完成
	 */
	bool							IsBuilded();

	/**
	 * @brief						获取数据表的数量
	 */
	unsigned int					GetTableCount();

	/**
	 * @brief						获取最后一次更新时间
	 */
	time_t							GetLastUpdateTime();

public:
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
	int								QueryQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

	/**
	 * @brief						更新实时行情数据
	 * @param[in]					nDataID				消息ID
	 * @param[in]					pData				数据内容
	 * @param[in]					nDataLen			长度
	 * @param[out]					nDbSerialNo			数据库新增，更新操作流水号
	 * @return						==0					成功
									!=0					错误
	 */
	int								UpdateQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

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
	int								BuildMessageTable( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo );

	/**
	 * @brief						删除记录
	 * @param[in]					nDataID				数据表ID
	 * @param[in]					pData				需要删除的代码
	 * @param[in]					nDataLen			代码长度
	 * @return						>=0					返回受影响的记录数
									<0					出错
	 */
	int								DeleteRecord( unsigned int nDataID, char* pData, unsigned int nDataLen );

public:
	/**
	 * @brief						从磁盘恢复行情数据到内存插件
	 * @detail						需要从磁盘文件恢复最近一天的行情数据（检查本地文件日期是否有效）
	 * @note						对日盘来说只能加载当天的日盘数据，对日盘来说只能加载前一个行情日期的数据
	 * @param[in]					refHoliday			节假对集合对象
	 * @return						==0					成功
									!=0					失败
	 */
	int								RecoverDatabase( MkHoliday& refHoliday );

	/**
	 * @brief						将内存插件中的行情数据进行备份
	 * @return						==0					成功
									!=0					失败
	 */
	int								BackupDatabase();

protected:
	CriticalObject					m_oLock;						///< 锁
	TMAP_DATAID2WIDTH				m_mapTableID;					///< 数据表ID集合表
	bool							m_bBuilded;						///< 数据表是否已经初始化完成
	time_t							m_nUpdateTimeT;					///< 数据库最后一次更新time_t
	Dll								m_oDllPlugin;					///< 插件加载类
	IDBFactory*						m_pIDBFactoryPtr;				///< 内存数据插件库的工厂类
	I_Database*						m_pIDatabase;					///< 数据库指针
};



#endif









