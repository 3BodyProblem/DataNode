#ifndef __MEM_DATABASE_INTERFACE_H__
#define	__MEM_DATABASE_INTERFACE_H__


/**
 * @classs							RecordBlock
 * @brief							��¼���ݿ���
 * @author							barry
 */
class RecordBlock
{
public:
	RecordBlock();
	RecordBlock( const RecordBlock& record );
	RecordBlock( char* pRecord, unsigned int nRecordLen );

public:
	/**
	 * @brief						��¼����Ϊ��Ч
	 * @return						true							��Ч����
	 */
	bool							IsNone() const;

	/**
	 * @brief						��¼��ȽϺ���
	 * @param[in]					refRecord						�Ƚϱ�Ķ���
	 * @return						true							��ͬ
									false							��ͬ
	 */
	bool							Compare( const RecordBlock& refRecord );

	/**
	 * @brief						��¼��deeply copy
	 * @param[in]					refRecord						�ڴ�copy��Դͷ
	 * @return						==1								Ӱ�������
									==0								����Ҫcopy��Ŀ���Դͷ������ȫһ��
									<0								���ִ���
	 */
	int								CloneFrom( const RecordBlock& refRecord );

public:
	/**
	 * @brief						ȡ�����ݵ�ַ
	 */
	const char*						GetPtr() const;

	/**
	 * @brief						���ݳ���
	 */
	unsigned int					Length() const;

protected:
	char*							m_pRecordData;					///< ���ݽṹ��ַ
	unsigned int					m_nRecordLen;					///< ���ݽṹ����
};


/**
 * @class							I_Table
 * @brief							���ݱ�����ӿ�
 * @author							barry
 */
class I_Table
{
public:
	/**
	 * @brief						׷��������
	 * @param[in]					pRecord					��¼���ַ
	 * @param[in]					nRecordLen				��¼�峤��
	 * @param[out]					nDbSerialNo				���ݿ����������²�����ˮ��
	 * @return						>0						���ӳɹ�
									=0						��¼�Ѿ����ڣ�����Ҫ����
									<0						ʧ��
	 */
	virtual int						InsertRecord( char* pRecord, unsigned int nRecordLen, unsigned __int64& nDbSerialNo ) = 0;

	/**
	 * @brief						��������
	 * @param[in]					pRecord					��¼���ַ
	 * @param[in]					nRecordLen				��¼�峤��
	 * @param[out]					nDbSerialNo				���ݿ����������²�����ˮ��
	 * @return						>0						���ӳɹ�
									=0						��¼�Ѿ����ڣ�����Ҫ����
									<0						ʧ��
	 */
	virtual int						UpdateRecord( char* pRecord, unsigned int nRecordLen, unsigned __int64& nDbSerialNo ) = 0; 

	/**
	 * @brief						��������¼����
	 * @param[in]					pKeyStr					������ַ
	 * @param[in]					nKeyLen					��������
	 * @return						���ؼ�¼����
	 */
	virtual RecordBlock				SelectRecord( char* pKeyStr, unsigned int nKeyLen ) = 0;

	/**
	 * @brief						ɾ��ĳ����¼
	 * @param[in]					pKeyStr					������ַ
	 * @param[in]					nKeyLen					��������
	 * @param[in,out]				nDbSerialNo				ȡ��>nDbSerialNo������(��Ϊ0,��ȫ��ȡ��) & ���������µ���ˮ��
	 * @return						>0						���ر��������ļ�¼��
									==0						δ�м�¼��ɾ��
									<0						ɾ��ʱ���ִ���
	 */
	virtual int						DeleteRecord( char* pKeyStr, unsigned int nKeyLen, unsigned __int64& nDbSerialNo ) = 0;

	/**
	 * @brief						�����ݱ������ԭ��copy������
	 * @param[in]					pBuffer					�����ַ
	 * @param[in]					nBufferSize				���泤��
	 * @param[in,out]				nDbSerialNo				ȡ��>nDbSerialNo������(��Ϊ0,��ȫ��ȡ��) & ���������µ���ˮ��
	 * @return						>=0						�������ݳ���
									<						����
	 */
	virtual int						CopyToBuffer( char* pBuffer, unsigned int nBufferSize, unsigned __int64& nDbSerialNo ) = 0;
};


/**
 * @class							I_Database
 * @brief							���ݿ�����ӿ�
 * @author							barry
 */
class I_Database
{
public:
	virtual ~I_Database(){};

	/**
	 * @brief						������Ϣid����Ϣ���ȣ����к��ʵ����ݱ����ã���Ԥ���������ö�Ӧ��ռ�ù�ϵ��
	 * @param[in]					nBindID				�������α�ʶ��
	 * @param[in]					nRecordWidth		���ݳ���
	 * @param[in]					nKeyStrLen			��������
	 * @return						=0					���óɹ�
									>0					���ԣ��ɹ���
									<0					���ó���
	 */
	virtual bool					CreateTable( unsigned int nBindID, unsigned int nRecordWidth, unsigned int nKeyStrLen ) = 0;

	/**
	 * @brief						ɾ��ָ�������ݱ�
	 * @param[in]					nBindID				���ݱ�ID
	 * @return						true				ɾ���ɹ�
	 */
	virtual bool					DeleteTable( unsigned int nBindID ) = 0;

	/**
	 * @brief						����MessageIDȡ���Ѿ����ڵĻ��߷���һ���µ��ڴ�������
	 * @detail						��������ÿ��messageidά��һ��Ψһ�Ҷ�Ӧ���ڴ������nBindIDֵ�����Ѿ����ڵģ����½��󷵻�
	 * @param[in]					nBindID				MessageID
	 * @return						�����Ѿ����ڵ��ڴ����½����ڴ��
	 */
	virtual I_Table*				QueryTable( unsigned int nBindID ) = 0;

	/**
	 * @brief						�����������ݱ�
	 */
	virtual int						DeleteTables() = 0;

	/**
	 * @brief						��Ӳ�ָ̻���������
	 * @return						>0					���ݿ���������ļ�����
									<0					����
	 */
	virtual int						LoadFromDisk( const char* pszDataFile ) = 0;

	/**
	 * @brief						���������ݴ���
	 */
	virtual bool					SaveToDisk( const char* pszDataFile ) = 0;

	/**
	 * @brief						ȡ�����ݸ���������ˮ��
	 */
	virtual unsigned __int64		GetUpdateSequence() = 0;

	/**
	 * @brief						ȡ�����ݱ������
	 * @return						����ͳ��ֵ
	 */
	virtual unsigned int			GetTableCount() = 0;

	/**
	 * @brief						����λ������ȡ�����ݱ�Ԫ��Ϣ
	 * @param[in]					���ȡ���ݱ��λ��
	 * @param[out]					nDataID				���ݱ�ID
	 * @param[out]					nRecordLen			���ݱ��¼����
	 * @param[out]					nKeyStrLen			����������󳤶�
	 * @return						true				��ȡ�ɹ�
									false				��ȡ����
	 */
	virtual bool					GetTableMetaByPos( unsigned int nPos, unsigned int& nDataID, unsigned int& nRecordLen, unsigned int& nKeyStrLen ) = 0;
};


/**
 * @class							IDBFactory
 * @brief							�ڴ����ݷ����������ӿ�
 * @author							barry
 */
class IDBFactory
{
public:
	/**
	 * @brief						�������������ݿ����ָ��
	 * @return						�������ݿ�ָ��ĵ�ַ
	 */
	virtual I_Database*				GrapDatabaseInterface() = 0;

	/**
	 * @brief						�ͷŷ�����������ݿ����
	 */
	virtual bool					ReleaseAllDatabase() = 0;

};





#endif









