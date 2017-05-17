#ifndef __DATE_NODE_INTERFACE_H__
#define	__DATE_NODE_INTERFACE_H__


/**
 * @class					E_QS_STATUS
 * @brief					���ݲɼ�ģ��ĵ�ǰ״̬
 * @date					2017/5/3
 * @author					barry
 */
enum E_QS_STATUS
{
	E_STATUS_NONE = -1,
	E_STATUS_TODAY_IS_HOLIDAY = 0,
	E_STATUS_CONNECTION_ESTABLISHED,
	E_STATUS_CONNECTION_CLOSED,
	E_STATUS_SESSION_LOGIN,
	E_STATUS_SESSION_REBUILD,
	E_STATUS_SESSION_SUBSCRIBE,
	E_STATUS_SERVICE_IS_AVAILABLE,
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
	 * @brief				��־����
	 * @param[in]			nLogLevel			��־����[0=��Ϣ��1=��Ϣ���桢2=������־��3=������־��4=��ϸ��־]
	 * @param[in]			pszFormat			�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... ) = 0;
};





#endif









