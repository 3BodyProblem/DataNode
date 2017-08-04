#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#pragma warning(disable:4018)
#include <set>
#include <string>
#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"


#define			MSG_HEARTBEAT_ID	0					///< ������Ϣ���
#define			MSG_LOGIN_ID		299					///< ��¼��Ϣ���


#pragma pack(1)


/**
 * @class							tagPackageHead
 * @brief							���ݰ��İ�ͷ�ṹ����
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< �������
	unsigned char					nMarketID;			///< �г����
	unsigned short					nMsgLength;			///< ���ݲ��ֳ���
	unsigned short					nMsgCount;			///< Message����
} tagPackageHead;


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


///< -------------------------------------------------------------------------------------------


/**
 * @class							PackagesLoopBuffer
 * @brief							���ݰ����л���
 * @detail							struct PkgHead + MessageID1 + data block1 + data block2 + struct PkgHead + MessageID2 + data block1 + data block2 + ...
 * @author							barry
 */
class PackagesLoopBuffer
{
public:
	PackagesLoopBuffer();
	~PackagesLoopBuffer();

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
	 * @param[out]					nMsgCount				�Ѿ�ѹ�����Ϣ����
	 * @param[out]					nBodySize				�Ѿ�ѹ�����Ϣ��С�ۼ�
	 * @return						==0						�ɹ�
	 * @note						��nDataID������ǰһ������nDataIDʱ����������һ��Package��װ
	 */
	int								PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, unsigned int& nMsgCount, unsigned int& nBodySize );

	/**
	 * @brief						��ȡһ�����ݰ�
	 * @param[out]					pBuff					������ݻ����ַ
	 * @param[in]					nBuffSize				���ݻ��泤��
	 * @param[out]					nMsgID					������ϢID
	 * @return						>0						���ݳ���
									==0						������
									<0						����
	 */
	int								GetOnePkg( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID );

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
	CriticalObject					m_oLock;				///< ��
	char*							m_pPkgBuffer;			///< ���ݰ������ַ
	unsigned int					m_nMaxPkgBufSize;		///< ���ݰ������С
	unsigned int					m_nCurrentPkgHeadPos;	///< ��ǰ��ʼλ��(��ǰ��д����İ�ͷ)
	unsigned int					m_nFirstPkgHeadPos;		///< ��ʼλ������(��д�����Ѿ�д����İ�ͷ����δ�������ݵ�ͷ��)
	unsigned int					m_nCurrentWritePos;		///< ����λ������(����д���λ��)
};


/**
 * @class							PkgBuffer
 * @brief							���ڱ���һ����ɵ�Package������
 * @author							barry
 */
class PkgBuffer
{
public:
	PkgBuffer();

	/**
	 * @brief					��ʼ������
	 * @param[in]				nBuffSize				Ҫ����Ļ����С
	 * @return					!= 0					ʧ��
	 */
	int							Initialize( unsigned int nBuffSize );

	/**
	 * @brief					�ͷ���Դ
	 */
	void						Release();

public:
	/**
	 * @brief					�������ַת������
	 * @return					char*
	 */
	operator					char*();

	/**
	 * @brief					��ȡ���ݲ��ֵ���Ч����
	 * @return					��Ч����
	 */
	unsigned int				CalPkgSize() const;

	/**
	 * @brief					��ȡ�������󳤶�
	 * @return					��󳤶�
	 */
	unsigned int				MaxBufSize() const;

protected:
	char*						m_pPkgBuffer;			///< ���ݷ��ͻ���
	unsigned int				m_nMaxBufSize;			///< ���ͻ�����󳤶�
};


///< -------------------------------------------------------------------------------------------


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
	void						FlushQuotation2AllClient();

	/**
	 * @brief					��ȡ���¿ռ�İٷֱ�
	 */
	float						GetFreePercent();

protected:
	WaitEvent					m_oWaitEvent;			///< �����ȴ�
	PackagesLoopBuffer			m_oDataBuffer;			///< �������ݻ������
	PkgBuffer					m_oOnePkg;				///< �������ݰ�����
};





#endif



