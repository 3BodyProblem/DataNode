#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include <set>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class							DataSequence
 * @brief							�������ά�ֱ�
 * @author							barry
 */
class DataSequence
{
public:
	typedef
public:

protected:
	std::map<>
};


#define		MAX_CODE_LENGTH		32	///< �����볤��


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
	 * @brief						ȡ�ô��ڵ����ݱ�id�б�
	 * @param[in]					pIDList			���ݱ�id�б�ָ��
	 * @param[in]					nMaxListSize	���ݱ�ĳ���
	 * @return						����ʵ�ʵ����ݱ�����
	 */
	unsigned int					GetTablesID( unsigned int* pIDList, unsigned int nMaxListSize );

	/**
	 * @brief						�����ݱ������ԭ��copy������
	 * @param[in]					nDataID					���ݱ�ID
	 * @param[in]					pBuffer					�����ַ
	 * @param[in]					nBufferSize				���泤��
	 * @return						>=0						�������ݳ���
									<						����
	 */
	int								FetchDataBlockByID( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize );

	/**
	 * @brief						�ж����ݱ��Ƿ��Ѿ��������
	 */
	bool							IsBuilded();

public:
	/**
	 * @brief						����ʵʱ��������
	 * @param[in]					nDataID				��ϢID
	 * @param[in]					pData				��������
	 * @param[in]					nDataLen			����
	 * @return						==0					�ɹ�
									!=0					����
	 */
	int								UpdateQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
 	 * @brief						��ʼ�����ʵ��������ݻص�
	 * @note						ֻ�Ǹ��¹�����������ݵ��ڴ��ʼ�ṹ��������
	 * @param[in]					nDataID				��ϢID
	 * @param[in]					pData				��������
	 * @param[in]					nDataLen			����
	 * @param[in]					bLastFlag			�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @return						==0					�ɹ�
									!=0					����
	 */
	int								BuildMessageTable( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

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
	CriticalObject					m_oLock;						///< ��
	std::set<unsigned int>			m_setTableID;					///< ���ݱ�ID���ϱ�
	bool							m_bBuilded;						///< ���ݱ��Ƿ��Ѿ���ʼ�����
	Dll								m_oDllPlugin;					///< ���������
	IDBFactory*						m_pIDBFactoryPtr;				///< �ڴ����ݲ����Ĺ�����
	I_Database*						m_pIDatabase;					///< ���ݿ�ָ��
};



#endif









