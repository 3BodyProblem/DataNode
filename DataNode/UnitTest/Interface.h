#ifndef __DATE_CLUSTER_INTERFACE_H__
#define	__DATE_CLUSTER_INTERFACE_H__


/**
 * @class					I_QuotationCallBack
 * @brief					���ݻص��ӿ�
 * @date					2017/6/28
 * @author					barry
 */
class I_QuotationCallBack
{
public:
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
	virtual void			OnQuotation( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen ) = 0;

	/**
	 * @brief				�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]			nDataID				��ϢID
	 * @param[in,out]		pData				��������(������ѯ����)
	 * @param[in]			nDataLen			����
	 * @return				>0					�ɹ�,�������ݽṹ�Ĵ�С
							==0					û�鵽���
							!=0					����
	 */
	virtual void			OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen ) = 0;

	/**
	 * @brief				��־����
	 * @param[in]			nLogLevel			��־����[0=��Ϣ��1=��Ϣ���桢2=������־��3=������־��4=��ϸ��־]
	 * @param[in]			pszFormat			�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf ) = 0;
};


///< -------------------------- ���ݲɼ�����ӿ� ----------------------------------


/**
 * @brief					��ʼ�����ݲɼ�ģ��
 * @param[in]				pIDataHandle				���鹦�ܻص�
 * @return					==0							��ʼ���ɹ�
							!=							����
 */
//typedef int					(__stdcall *T_Func_Initialize)( I_DataHandle* pIDataHandle );

/**
 * @brief					�ͷ����ݲɼ�ģ��
 */
//typedef void				(__stdcall *T_Func_Release)();

/**
 * @brief					��ȡģ��ĵ�ǰ״̬
 * @param[out]				pszStatusDesc				���س�״̬������
 * @param[in,out]			nStrLen						�������������泤�ȣ������������Ч���ݳ���
 * @return					����ģ�鵱ǰ״ֵ̬
 */
//typedef int					(__stdcall *T_Func_GetStatus)( char* pszStatusDesc, unsigned int& nStrLen );



#endif









