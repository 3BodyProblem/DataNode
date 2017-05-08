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





#endif









