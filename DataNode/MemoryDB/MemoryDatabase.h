#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include "Interface.h"
#include "../Infrastructure/Dll.h"


#define		MAX_CODE_LENGTH		32


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
	 * @brief						判断数据表是否已经建立完成
	 */
	bool							IsBuilded();

public:
	/**
	 * @brief						更新实时行情数据
	 * @param[in]					nDataID				消息ID
	 * @param[in]					pData				数据内容
	 * @param[in]					nDataLen			长度
	 * @return						==0					成功
									!=0					错误
	 */
	int								UpdateQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
 	 * @brief						初始化性质的行情数据回调
	 * @note						只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]					nDataID				消息ID
	 * @param[in]					pData				数据内容
	 * @param[in]					nDataLen			长度
	 * @param[in]					bLastFlag			是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return						==0					成功
									!=0					错误
	 */
	int								BuildMessageTable( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

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
	bool							m_bBuilded;						///< 数据表是否已经初始化完成
	Dll								m_oDllPlugin;					///< 插件加载类
	IDBFactory*						m_pIDBFactoryPtr;				///< 内存数据插件库的工厂类
	I_Database*						m_pIDatabase;					///< 数据库指针
};



#endif









