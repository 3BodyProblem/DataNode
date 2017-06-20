#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#pragma warning(disable:4018)
#include <set>
#include <string>
#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"
#include "../../MemoryDB/MemoryDatabase.h"


#define			MESSAGENO			100


#pragma pack(1)

/**
 * @class							tagPackageHead
 * @brief							���ݰ��İ�ͷ�ṹ����
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< �������
	unsigned int					nMarketID;			///< �г����
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


/**
 * @class							tagCommonLoginData_LF299
 * @brief							��¼���ݿ�
 * @author							barry
 */
typedef struct
{
	char							pszActionKey[20];	///< ָ���ַ���: request:�����¼ success:��¼�ɹ� failure:��¼ʧ��
	char							pszUserName[32];	///< �û���
	char							pszPassword[64];	///< ����
	unsigned int					nReqDBSerialNo;		///< ���������ˮ��֮�����������
	char							Reserve[1024];		///< ����
} tagCommonLoginData_LF299;


#pragma pack()


#define						MAX_LINKID_NUM			32
typedef						unsigned int			LINKID_VECTOR[MAX_LINKID_NUM];


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
	 * @brief						�����г����
	 */
	void							SetMkID( unsigned int nMkID );

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

	/**
	 * @brief						��ȡ���¿ռ�İٷֱ�
	 */
	float							GetPercentOfFreeSize();

protected:
	unsigned int					m_nMarketID;			///< �г�ID
	CriticalObject					m_oLock;				///< ��
	char*							m_pPkgBuffer;			///< ���ݰ������ַ
	unsigned int					m_nMaxPkgBufSize;		///< ���ݰ������С
	unsigned long					m_nFirstPosition;		///< ��ʼλ������
	unsigned long					m_nLastPosition;		///< ����λ������
};


class LinkNoRegister;


/**
 * @class						QuotationSynchronizer
 * @brief						������ʵʱ���ͻ���
 * @author						barry
 */
class QuotationSynchronizer : public SimpleTask
{
public:
	/**
	 * @brief					���캯��
	 */
	QuotationSynchronizer();
	~QuotationSynchronizer();

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
	 * @param[in]				arrayLinkNo				��·�Ŷ��е�ַ����Ҫ���㲥��������·��ID����
	 * @param[in]				nLinkCount				��·�Ŷ��г���
	 */
	void						FlushQuotation2Client();

	/**
	 * @brief					��ȡ���¿ռ�İٷֱ�
	 */
	float						GetFreePercent();

	/**
	 * @brief					���õ�ǰ��·�ŵ��б�
	 */
	void						SetLinkNoList( LinkNoRegister& refLinkNoTable );

	/**
	 * @brief					�����г����
	 */
	void						SetMkID( unsigned int nMkID );

protected:
	WaitEvent					m_oWaitEvent;			///< �����ȴ�
	PackagesBuffer				m_oDataBuffer;			///< ���ݻ������
protected:
	char*						m_pSendBuffer;			///< ���ݷ��ͻ���
	unsigned int				m_nMaxSendBufSize;		///< ���ͻ�����󳤶�
protected:
	CriticalObject				m_oLock;				///< ��
	LINKID_VECTOR				m_vctLinkNo;			///< ������·��
	unsigned int				m_nLinkCount;			///< ������·����
};





#endif



