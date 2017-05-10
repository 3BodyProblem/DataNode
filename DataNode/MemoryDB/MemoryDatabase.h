#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include "Interface.h"
#include "../Infrastructure/Dll.h"


/**
 * @class							DatabaseIO
 * @brief							���ݿ������
 * @date							2017/5/4
 * @author							barry
 */
class DatabaseIO
{
public:
	DatabaseIO();
	~DatabaseIO();

	/**
	 * @brief						��ʼ�����ݿ�������
	 * @return						==0				�ɹ�
									!=0				����
	 */
	int								Initialize();

	/**
	 * @brief						�ͷ�������Դ
	 */
	void							Release();

public:
	/**
	 * @brief						�Ӵ��ָ̻��������ݵ��ڴ���
	 * @return						==0				�ɹ�
									!=0				ʧ��
	 */
	int								RecoverDatabase();

	/**
	 * @brief						���ڴ����е��������ݽ��б���
	 * @return						==0				�ɹ�
									!=0				ʧ��
	 */
	int								BackupDatabase();

protected:
	Dll								m_oDllPlugin;					///< ���������
	IDBFactory*						m_pIDBFactoryPtr;				///< ������
	I_Database*						m_pIDatabase;					///< ���ݿ�ָ��
};



#endif









