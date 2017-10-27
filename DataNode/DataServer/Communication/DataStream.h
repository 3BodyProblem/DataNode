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
 * @class						SendPackagePool
 * @brief						���ڱ���һ�������͵�Package������
 * @author						barry
 */
class SendPackagePool : public SimpleTask
{
public:
	SendPackagePool();

	/**
	 * @brief					��ʼ������
	 * @param[in]				nBuffSize				Ϊÿ��MessageIDҪ����Ļ����С
	 * @param[in]				nMsgCount				��Ҫ֧�ֵ�Message���������(��ҪΪÿ��������һ��nBuffSize��С�Ļ�����)
	 * @return					!= 0					ʧ��
	 * @note					������ܻ����С = nOneBuffSize * nMsgCount
	 */
	int							Initialize( unsigned int nOneBuffSize = 1024*1024*8, unsigned int nMsgCount = 16 );

	/**
	 * @brief					�ͷ���Դ
	 */
	void						Release();

public:
	/**
	 * @brief					�洢����
	 * @param[in]				nDataID					����ID
	 * @param[in]				pData					����ָ��
	 * @param[in]				nDataSize				���ݳ���
	 * @param[in]				nSeqNo					��ǰ���ݿ�ĸ������
	 * @param[in]				bSendDirect				�Ƿ�ֱ�����緢�ͱ�ʶ
	 * @return					==0						�ɹ�
	 * @note					��nDataID������ǰһ������nDataIDʱ����������һ��Package��װ
	 */
	int							DispatchMessage( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, bool bSendDirect );

protected:
	/**
	 * @brief					��package�е��������������·�
	 * @return					���͵����ݴ�С
	 */
	int							SendAllPkg();

	/**
	 * @brief					������(��ѭ��)
	 * @return					==0							�ɹ�
								!=0							ʧ��
	 */
	virtual int					Execute();

protected:
	WaitEvent					m_oSendNotice;			///< ����֪ͨ�¼�
	CriticalObject				m_oLock;				///< ��
	unsigned int				m_vctMsgID[128];		///< ���������Ϣ�б�
	unsigned int				m_MsgIDCount;			///< ��Ϣ�б���
	unsigned int				m_nAllocatedTimes;		///< �Ѿ�������Ĵ���(��Ϣ������)
	unsigned int				m_nOneMsgBufSize;		///< һ����Ϣ�������Ĵ�С
	char*						m_vctAddrMap[512];		///< ��Э���ӳ��Ϊ�ڴ󻺴��е���ʼλ��
	unsigned int				m_vctCurDataSize[512];	///< ��Ч�������ݳ���
	char*						m_pPkgBuffer;			///< ���ݷ��ͻ���
	unsigned int				m_nMaxBufSize;			///< ���ͻ�����󳤶�
};




#endif








