#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#pragma warning(disable:4018)
#include <set>
#include <string>
#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"
#include "../../MemoryDB/MemoryDatabase.h"


#pragma pack(1)

/**
 * @class							tagPackageHead
 * @brief							���ݰ��İ�ͷ�ṹ����
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< �������
	unsigned int					nBodyLen;			///< ���ݲ��ֳ���
	unsigned int					nMsgCount;			///< ���ڵ�Message����
} tagPackageHead;


/**
 * @class							tagBlockHead
 * @brief							���ݿ��ͷ���Ķ���
 * @author							barry
 */
typedef struct
{
	unsigned int					nDataType;			///< ���ݿ�����
	unsigned int					nDataLen;			///< ���ݿ鳤��
} tagBlockHead;

#pragma pack()


/**
 * @class							PackagesBuffer
 * @brief							���ݰ����л���
 * @detail							struct PkgHead + data block1 + data block2 + ...
 * @author							barry
 */
class PackagesBuffer
{
public:
	PackagesBuffer();
	~PackagesBuffer();

public:
	/**
	 * @brief						��ʼ���������
	 * @param[in]					nMaxBufSize				������Ļ����С
	 * @return						==0						�ɹ�
	 */
	int								Initialize( unsigned long nMaxBufSize );

	/**
	 * @brief						�ͷŻ���ռ�
	 */
	void							Release();

public:
	/**
	 * @brief						�洢����
	 * @param[in]					nDataID					����ID
	 * @param[in]					pData					����ָ��
	 * @param[in]					nDataSize				���ݳ���
	 * @param[in]					nSeqNo					��ǰ���ݿ�ĸ������
	 * @return						==0						�ɹ�
	 */
	int								PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo );

	/**
	 * @brief						��ȡһ�����ݰ�
	 * @param[out]					pBuff					������ݻ����ַ
	 * @param[in]					nBuffSize				���ݻ��泤��
	 * @return						>0						���ݳ���
									==0						������
									<0						����
	 */
	int								GetOnePkg( char* pBuff, unsigned int nBuffSize );

	/**
	 * @brief						�Ƿ�Ϊ��
	 * @return						true					Ϊ��
	 */
	bool							IsEmpty();

protected:
	CriticalObject					m_oLock;				///< ��
	char*							m_pPkgBuffer;			///< ���ݰ������ַ
	unsigned int					m_nMaxPkgBufSize;		///< ���ݰ������С
	unsigned long					m_nFirstPosition;		///< ��ʼλ������
	unsigned long					m_nLastPosition;		///< ����λ������
};


/**
 * @class						QuotationStream
 * @brief						����������
 * @author						barry
 */
class QuotationStream : public SimpleTask
{
public:
	/**
	 * @brief					���캯��
	 */
	QuotationStream();
	~QuotationStream();

	/**
	 * @brief					��ʼ�����������ͻ���
	 * @param[in]				nNewBuffSize			Ҫ����Ļ����С
	 * @return					!= 0					ʧ��
	 */
	int							Initialize( unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					�ͷŸ���Դ
	 */
	void						Release();

protected:
	/**
	 * @brief					������(��ѭ��)
	 * @return					==0						�ɹ�
								!=0						ʧ��
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					����������ѹ������л���
	 * @param[in]				nMsgID					Message ID
	 * @param[in]				pData					��Ϣ���ݵ�ַ
	 * @param[in]				nLen					��Ϣ����
	 * @return					> 0						�ɹ����������ε����ۻ������л��ĳ���
								<= 0					ʧ��
	 */
	int							PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen, unsigned __int64 nSeqNo );

	/**
	 * @brief					�ӻ�����ȡ���� & ���͸��¼��ͻ���
	 * @param[in]				lpLinkNoSet				��·�Ŷ��е�ַ����Ҫ���㲥��������·��ID����
	 * @param[in]				uiLinkNoCount			��·�Ŷ��г���
	 */
	void						FlushQuotation2Client();

protected:
	WaitEvent					m_oWaitEvent;			///< �����ȴ�
	PackagesBuffer				m_oDataBuffer;			///< ���ݻ������
	char*						m_pSendBuffer;			///< ���ݷ��ͻ���
	unsigned int				m_nMaxSendBufSize;		///< ���ͻ�����󳤶�
};


/**
 * @class						ImageRebuilder
 * @brief						��ʼ������������
 * @author						barry
 */
class ImageRebuilder
{
private:
	ImageRebuilder();

public:
	/**
	 * @brief					��ȡ�������������
	 */
	static ImageRebuilder&		GetRebuilder();

	/**
	 * @brief					��ʼ�������ʼ��������
	 * @return					!= 0					ʧ��
	 */
	int							Initialize();

	/**
	 * @brief					�ͷŸ���Դ
	 */
	void						Release();

public:
	/**
	 * @brief					��ȡ�ڴ����ݿ�ĳ���ݱ�ĵ�������Ʒ����
	 * @param[in]				nDataID					���ݱ�ID
	 * @param[in]				refDatabaseIO			���ݿ�������
	 * @param[out]				setCode					���ݱ���������
	 * @return					>=0						�����е�Ԫ������
								<0						����
	 */
	int							QueryCodeListInDatabase( unsigned int nDataID, DatabaseIO& refDatabaseIO, std::set<std::string>& setCode );

	/**
	 * @brief					����������ͬ��/��ʼ�������пͻ�����·
	 * @param[in]				refDatabaseIO			���ݿ�������
	 * @param[in]				nSerialNo				���Ͳ�ѯ���(��Ҫ>nSerialNo)
	 * @return					>=0						ͬ������·��
								<0						����
	 */
	int							Flush2ReqSessions( DatabaseIO& refDatabaseIO, unsigned __int64 nSerialNo = 0 );

	/**
	 * @brief					��ȡ����ʼ��������·����
	 */
	unsigned int				GetReqSessionCount();

	/**
	 * @brief					����һ���µĴ���ʼ�����͵���·��
	 * @param[in]				nLinkNo					��·��
	 * @return					true					���ӳɹ�
								false					ʧ�ܣ����ظ���
	 */
	bool						AddNewReqSession( unsigned int nLinkNo );

protected:
	CriticalObject				m_oBuffLock;			///< ��ʼ���������ͻ�����
	std::set<unsigned int>		m_setNewReqLinkID;		///< ����ʼ����·ID����
	unsigned int				m_nReqLinkCount;		///< �����ʼ������·����
	PackagesBuffer				m_oDataBuffer;			///< ���ݻ������
	char*						m_pSendBuffer;			///< ���ݷ��ͻ���
	unsigned int				m_nMaxSendBufSize;		///< ���ͻ�����󳤶�
};




#endif



