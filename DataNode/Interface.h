#ifndef __DATE_NODE_INTERFACE_H__
#define	__DATE_NODE_INTERFACE_H__


/**
 * @class					I_DataHandle
 * @brief					���ݻص��ӿ�
 * @date					2017/5/3
 * @author					barry
 */
class I_DataHandle
{
public:
	/**
 	 * @brief				��ʼ�����ʵ��������ݻص�
	 * @note				ֻ�Ǹ��¹�����������ݵ��ڴ��ʼ�ṹ��������
	 * @param[in]			nDataID				��ϢID
	 * @param[in]			pData				��������
	 * @param[in]			nDataLen			����
	 * @param[in]			bLastFlag			�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag ) = 0;

	/**
	 * @brief				ʵ���������ݻص�
	 * @note				���������ڴ�飬������
	 * @param[in]			nDataID				��ϢID
	 * @param[in]			pData				��������
	 * @param[in]			nDataLen			����
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen ) = 0;

	/**
	 * @brief				������ϢID/Code��ѯĳ��������(ͨ���ڴ����ݲ���ӿ�)
	 * @param[in]			nDataID				��ϢID
	 * @param[in,out]		pData				��ƷCode[in],���ݲ�ѯ����[out]
	 * @param[in]			nDataLen			���泤��
	 * @return				true				��ѯ�ɹ�
	 */
	virtual bool			OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen ) = 0;
};

















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
	 * @return						>0						���ӳɹ�
									=0						��¼�Ѿ����ڣ�����Ҫ����
									<0						ʧ��
	 */
	virtual int						InsertRecord( char* pRecord, unsigned int nRecordLen ) = 0; 

	/**
	 * @brief						��������
	 * @param[in]					pRecord					��¼���ַ
	 * @param[in]					nRecordLen				��¼�峤��
	 * @return						>0						���ӳɹ�
									=0						��¼�Ѿ����ڣ�����Ҫ����
									<0						ʧ��
	 */
	virtual int						UpdateRecord( char* pRecord, unsigned int nRecordLen ) = 0; 

	/**
	 * @brief						��������¼����
	 * @param[in]					pKeyStr					������ַ
	 * @param[in]					nKeyLen					��������
	 * @return						���ؼ�¼����
	 */
	virtual RecordBlock				SelectRecord( char* pKeyStr, unsigned int nKeyLen ) = 0;
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
	 * @brief						����MessageIDȡ���Ѿ����ڵĻ��߷���һ���µ��ڴ�������
	 * @detail						��������ÿ��messageidά��һ��Ψһ�Ҷ�Ӧ���ڴ������nBindIDֵ�����Ѿ����ڵģ����½��󷵻�
	 * @param[in]					nBindID				MessageID
	 * @return						�����Ѿ����ڵ��ڴ����½����ڴ��
	 */
	virtual I_Table*				QueryTable( unsigned int nBindID ) = 0;

	/**
	 * @brief						�����������ݱ�
	 */
	virtual void					DeleteTables() = 0;

	/**
	 * @brief						��Ӳ�ָ̻���������
	 */
	virtual bool					LoadFromDisk( const char* pszDataFile ) = 0;

	/**
	 * @brief						���������ݴ���
	 */
	virtual bool					SaveToDisk( const char* pszDataFile ) = 0;
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









