#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include "Interface.h"


/**
 * @class							DatabaseManager
 * @brief							���ݿ������
 * @date							2017/5/4
 * @author							barry
 */
class DatabaseManager
{
private:
	DatabaseManager();

public:
	/**
	 * @brief						ȡ�ù�����񵥼�����
	 */
	static DatabaseManager&			GetMgrObject();

protected:
	IDBFactory*						m_pIDBFactoryPtr;				///< ������
	I_Database*						m_pIDatabase;					///< ���ݿ�ָ��
};



#endif









