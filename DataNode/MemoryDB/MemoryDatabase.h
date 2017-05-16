#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include <set>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


#define		MAX_CODE_LENGTH		32	///< 最大代码长度


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

public:
	/**
	 * @brief						取得存在的数据表id列表
	 * @param[in]					pIDList			数据表id列表指针
	 * @param[in]					nMaxListSize	数据表的长度
	 * @return						返回实际的数据表数量
	 */
	unsigned int					GetTablesID( unsigned int* pIDList, unsigned int nMaxListSize );

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

public:
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

public:
	/**
	 * @brief						从磁盘恢复行情数据到内存插件
	 * @return						==0				成功
									!=0				失败
	 */
	int								RecoverDatabase();

	/**
	 * @brief						将内存插件中的行情数据进行备份
	 * @return						==0				成功
									!=0				失败
	 */
	int								BackupDatabase();

protected:
	CriticalObject					m_oLock;						///< 锁
	std::set<unsigned int>			m_setTableID;					///< 数据表ID集合表
	bool							m_bBuilded;						///< 数据表是否已经初始化完成
	Dll								m_oDllPlugin;					///< 插件加载类
	IDBFactory*						m_pIDBFactoryPtr;				///< 内存数据插件库的工厂类
	I_Database*						m_pIDatabase;					///< 数据库指针
};



#endif









