#ifndef __MEMORYDB_MEMORYDATABASE_H__
#define	__MEMORYDB_MEMORYDATABASE_H__


#include <set>
#include <map>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../InitializeFlag/InitFlag.h"
#include "../DataServer/Communication/DataStream.h"


#define			MAX_CODE_LENGTH							32						///< �����볤��
typedef			std::map<unsigned int,unsigned int>		TMAP_DATAID2WIDTH;		///< map[����ID,���ݽṹ����]


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

private:	
	/**
	 * @brief						��ʽ���������ݻ���
	 * @return						���ظ�ʽ��������ݳ���
	 */
	unsigned int					FormatImageBuffer( unsigned int nSeqNo, unsigned int nDataID, unsigned int nDataWidth, unsigned int nBuffDataLen );

public:///< ���ܺ���
	/**
	 * @brief						��ȫ����ʼ�����鷢���µ������·
	 * @param[in]					nSerialNo				���Ͳ�ѯ���(��Ҫ>nSerialNo)
	 * @return						>=0						ͬ������·��
									<0						����
	 */
	int								FlushImageData2NewSessions( unsigned __int64 nSerialNo = 0 );

	/**
	 * @brief						��ȡ�ڴ����ݿ�ĳ���ݱ�ĵ�������Ʒ����
	 * @param[in]					nDataID					���ݱ�ID
	 * @param[in]					nRecordLen				���ݱ��Ӧ���ݰ�����
	 * @param[out]					setCode					���ݱ���������
	 * @return						>=0						�����е�Ԫ������
									<0						����
	 */
	int								QueryCodeListInImage( unsigned int nDataID, unsigned int nRecordLen, std::set<std::string>& setCode );

	/**
	 * @brief						ȡ�ô��ڵ����ݱ�id�б�
	 * @param[out]					pIDList			���ݱ�id�б�ָ��
	 * @param[in]					nMaxListSize	���ݱ�ĳ���
	 * @param[out]					pWidthList		�����ݱ�ṹ�����Ϣ�б�
	 * @param[in]					nMaxWidthlistLen	�б���󳤶�
	 * @return						����ʵ�ʵ����ݱ�����
	 */
	unsigned int					GetTablesID( unsigned int* pIDList, unsigned int nMaxListSize, unsigned int* pWidthList = NULL, unsigned int nMaxWidthlistLen = 0 );

	/**
	 * @brief						�����ݱ������ԭ��copy������
	 * @param[in]					nDataID					���ݱ�ID
	 * @param[in]					pBuffer					�����ַ
	 * @param[in]					nBufferSize				���泤��
	 * @param[in,out]				nDbSerialNo				ȡ��>nDbSerialNo������(��Ϊ0,��ȫ��ȡ��) & ���������µ���ˮ��
	 * @return						>=0						�������ݳ���
									<						����
	 */
	int								FetchRecordsByID( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo );

	/**
	 * @brief						�ж����ݱ��Ƿ��Ѿ��������
	 * @note						�����ӱ��ؼ��أ��ʹ������м��صĶ�����true�����
	 */
	bool							IsBuilded();

	/**
	 * @brief						��ȡ���ݱ������
	 */
	unsigned int					GetTableCount();

	/**
	 * @brief						��ȡ���һ�θ���ʱ��
	 */
	time_t							GetLastUpdateTime();

public:///< ��¼����
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
	int								QueryQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

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
	int								UpdateQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo );

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
	int								BuildMessageTable( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo );

	/**
	 * @brief						ɾ����¼
	 * @param[in]					nDataID				���ݱ�ID
	 * @param[in]					pData				��Ҫɾ���Ĵ���
	 * @param[in]					nDataLen			���볤��
	 * @return						>=0					������Ӱ��ļ�¼��
									<0					����
	 */
	int								DeleteRecord( unsigned int nDataID, char* pData, unsigned int nDataLen );

public:///< ���ݿ����
	/**
	 * @brief						�Ӵ��ָ̻��������ݵ��ڴ���
	 * @detail						��Ҫ�Ӵ����ļ��ָ����һ����������ݣ���鱾���ļ������Ƿ���Ч��
	 * @note						��������˵ֻ�ܼ��ص�����������ݣ���������˵ֻ�ܼ���ǰһ���������ڵ�����
	 * @param[in]					refHoliday			�ڼٶԼ��϶���
	 * @return						==0					�ɹ�
									!=0					ʧ��
	 */
	int								RecoverDatabase( MkHoliday& refHoliday );

	/**
	 * @brief						���ڴ����е��������ݽ��б���
	 * @return						==0					�ɹ�
									!=0					ʧ��
	 */
	int								BackupDatabase();

protected:
	CriticalObject					m_oLock;						///< ��
	TMAP_DATAID2WIDTH				m_mapTableID;					///< ���ݱ�ID���ϱ�
	bool							m_bBuilded;						///< ���ݱ��Ƿ��Ѿ���ʼ�����
	time_t							m_nUpdateTimeT;					///< ���ݿ����һ�θ���time_t
	Dll								m_oDllPlugin;					///< ���������
	IDBFactory*						m_pIDBFactoryPtr;				///< �ڴ����ݲ����Ĺ�����
	I_Database*						m_pIDatabase;					///< ���ݿ�ָ��
	PkgBuffer						m_oQueryBuffer;					///< ���ݷ��ͻ���
};



#endif









