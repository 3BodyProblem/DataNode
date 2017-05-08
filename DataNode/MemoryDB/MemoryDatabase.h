#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include "Interface.h"


/**
 * @class							DatabaseManager
 * @brief							数据库管理类
 * @date							2017/5/4
 * @author							barry
 */
class DatabaseManager
{
private:
	DatabaseManager();

public:
	/**
	 * @brief						取得管理对像单键引用
	 */
	static DatabaseManager&			GetMgrObject();

protected:
	IDBFactory*						m_pIDBFactoryPtr;				///< 工厂类
	I_Database*						m_pIDatabase;					///< 数据库指针
};



#endif









