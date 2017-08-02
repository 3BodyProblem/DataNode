#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include <set>
#include <map>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../InitializeFlag/InitFlag.h"
#include "../DataServer/Communication/DataStream.h"


#define			MAX_CODE_LENGTH									32							///< �����볤��
typedef			std::map<unsigned int,unsigned int>				TMAP_DATAID2WIDTH;			///< map[����ID,���ݽṹ����]
typedef			std::map<unsigned int,std::set<std::string>>	MAP_TABLEID_CODES;			///< �����ݱ��е���Ʒ����map[���ݱ�ID,code����]


/**
 * @class							DatabaseIO
 * @brief							���ݿ������
 * @date							2017/5/4
 * @author							barry
 */
class DatabaseIO
{
public:///< ��ʼ��
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

	/**
	 * @brief						��Ԫ����
	 */
	void							UnitTest();

public:///< ״̬��ȡ
	/**
	 * @brief						�ж����ݱ��Ƿ��Ѿ��������
	 * @note						�����ӱ��ؼ��أ��ʹ������м��صĶ�����true�����
									&
									�˱�ʶ�ɵ��������趨
	 */
	bool							IsBuilded();

	/**
	 * @brief						��ȡ���ݱ������
	 */
	unsigned int					GetTableCount();

public:///< ��¼����
	/**
	 * @brief						�����ݱ������ԭ��copy������
	 * @param[in]					nDataID					���ݱ�ID
	 * @param[in]					pBuffer					�����ַ
	 * @param[in]					nBufferSize				���泤��
	 * @param[in,out]				nDbSerialNo				ȡ��>nDbSerialNo������(��Ϊ0,��ȫ��ȡ��) & ���������µ���ˮ��
	 * @return						>=0						�������ݳ���
									<						����
	 */
	int								QueryBatchRecords( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo );

	/**
	 * @brief						��ѯʵʱ��������
	 * @param[in]					nDataID				��ϢID
	 * @param[in,out]				pData				��������
	 * @param[in]					nDataLen			����
	 * @param[out]					nDbSerialNo			���ݿ����������²�����ˮ��
	 * @return						>0					�ɹ�,�������ݽṹ�ĳ���
									==0					δ�鵽����
									!=0					����
	 */
	int								QueryRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

	/**
	 * @brief						����ʵʱ��������
	 * @param[in]					nDataID				��ϢID
	 * @param[in]					pData				��������
	 * @param[in]					nDataLen			����
	 * @param[out]					nDbSerialNo			���ݿ����������²�����ˮ��
	 * @return						>0					�и��µ���¼����
									==0					�ɹ�,��û��ʵ�ʸ���
									!=0					����
	 */
	int								UpdateRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

	/**
 	 * @brief						��ʼ�����ʵ��������ݻص�
	 * @note						ֻ�Ǹ��¹�����������ݵ��ڴ��ʼ�ṹ��������
	 * @param[in]					nDataID				��ϢID
	 * @param[in]					pData				��������
	 * @param[in]					nDataLen			����
	 * @param[in]					bLastFlag			�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @param[out]					nDbSerialNo			���ݿ����������²�����ˮ��
	 * @return						==0					�ɹ�
									!=0					����
	 */
	int								NewRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo );

	/**
	 * @brief						ɾ����¼
	 * @param[in]					nDataID				���ݱ�ID
	 * @param[in]					pData				��Ҫɾ���Ĵ���
	 * @param[in]					nDataLen			���볤��
	 * @return						>=0					������Ӱ��ļ�¼��
									<0					����
	 */
	int								DeleteRecord( unsigned int nDataID, char* pData, unsigned int nDataLen );

protected:///< ״̬��Ԫ����
	CriticalObject					m_oLock;						///< ��
	bool							m_bBuilded;						///< ���ݱ��Ƿ��Ѿ���ʼ�����(�ɶ������������)
	TMAP_DATAID2WIDTH				m_mapTableID;					///< ���ݱ�����ݽṹ����ͳ��
protected:///< DB������
	Dll								m_oDllPlugin;					///< ���������
	IDBFactory*						m_pIDBFactoryPtr;				///< �ڴ����ݲ����Ĺ�����
	I_Database*						m_pIDatabase;					///< ���ݿ�ָ��
};


///< ------------------------------------------------------------------------


/**
 * @class				PowerfullDatabase
 * @brief				���ݿ������չ��
 * @date							2017/8/1
 * @author				barry
 */
class PowerfullDatabase : public DatabaseIO
{
public:///< ��ʼ��
	PowerfullDatabase();
	~PowerfullDatabase();

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

public:///< ���ݿ�ָ��뱸��
	/**
	 * @brief						�Ӵ��ָ̻��������ݵ��ڴ���
	 * @detail						��Ҫ�Ӵ����ļ��ָ����һ����������ݣ���鱾���ļ������Ƿ���Ч��
	 * @note						��������˵ֻ�ܼ��ص�����������ݣ���������˵ֻ�ܼ���ǰһ���������ڵ�����
	 * @param[in]					refHoliday			�ڼٶԼ��϶���
	 * @param[in]					bRecoverFromDisk	�Ƿ���Ҫ���ڴ�����Ϊÿ�����ݱ�ͳ�ƶ�Ӧ��code����(ͨ��ֻ������ݲɼ���һ��)
	 * @return						==0					�ɹ�
									!=0					ʧ��
	 */
	int								RecoverDatabase( MkHoliday& refHoliday, bool bRecoverFromDisk );

	/**
	 * @brief						���ڴ����е��������ݽ��б���
	 * @return						==0					�ɹ�
									!=0					ʧ��
	 */
	int								BackupDatabase();

	/**
	 * @brief						ɾ���ڴ���еķ���Ч��Ʒ��¼
	 * @param[in]					mapCodeWhiteList		��ǰ��Ч�ĸ����ݱ�ID�µ�code������
	 * @param[in]					bNeed2Erase				�Ƿ������ݲɼ����ˣ�����Ҫ������ִ�еı�ʶ
	 * @return						����ɾ��������
	 * @note						���������ģ��ֻ��bNeed2Erase����falseֵ���Զ�·��������
	 */
	int								RemoveCodeExpiredFromDisk( MAP_TABLEID_CODES& mapCodeWhiteList, bool bNeed2Erase );

public:///< ���ݿ������������
	/**
	 * @brief						��ȫ����ʼ�����鷢���µ������·
	 * @param[in]					nSerialNo				���Ͳ�ѯ���(��Ҫ>nSerialNo)
	 * @return						>=0						ͬ������·��
									<0						����
	 */
	int								FlushDatabase2RequestSessions( unsigned __int64 nSerialNo = 0 );

protected:
	PkgBuffer						m_oQueryBuffer;					///< ���ݷ��ͻ���
	MAP_TABLEID_CODES				m_mapID2Codes;					///< �����ݱ��е�code����ͳ��
};



#endif









