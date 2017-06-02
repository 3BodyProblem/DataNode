#ifndef __DATE_NODE_INTERFACE_H__
#define	__DATE_NODE_INTERFACE_H__


/**
 * @class	E_SS_Status
 * @brief	CTP����Ự״̬ö��
 */
enum E_SS_Status
{
	ET_SS_UNACTIVE = 0,				///< δ����:	��Ҫ��Session����Initialize()
	ET_SS_DISCONNECTED,				///< �Ͽ�״̬
	ET_SS_CONNECTED,				///< ��ͨ״̬
	ET_SS_LOGIN,					///< ��¼�ɹ�
    ET_SS_INITIALIZING,				///< ��ʼ�����/������
	ET_SS_INITIALIZED,				///< ��ʼ�����
	ET_SS_WORKING,					///< ����������
};


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
							&
							�ᵼ�¿��ģ������³�ʼ��״̬
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
	 * @param[in]			bPushFlag			���ͱ�ʶ
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag ) = 0;

	/**
	 * @brief				�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]			nDataID				��ϢID
	 * @param[in,out]		pData				��������(������ѯ����)
	 * @param[in]			nDataLen			����
	 * @return				>0					�ɹ�,�������ݽṹ�Ĵ�С
							==0					û�鵽���
							!=0					����
	 */
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen ) = 0;

	/**
	 * @brief				��־����
	 * @param[in]			nLogLevel			��־����[0=��Ϣ��1=��Ϣ���桢2=������־��3=������־��4=��ϸ��־]
	 * @param[in]			pszFormat			�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... ) = 0;
};





typedef int					(__stdcall *T_Func_Initialize)( I_DataHandle* pIDataHandle );
typedef void				(__stdcall *T_Func_Release)();
typedef int					(__stdcall *T_Func_RecoverQuotation)();
typedef void				(__stdcall *T_Func_HaltQuotation)();
typedef int					(__stdcall *T_Func_GetStatus)();
typedef int					(__stdcall *T_Func_GetMarketID)();




#endif









