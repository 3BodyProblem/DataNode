#ifndef __DATA_ENCODER_H__
#define	__DATA_ENCODER_H__


#include <string>
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../../../DataXCode/DataXCode/IXCode.h"


/**
 * @class					DataEncoder
 * @brief					���������л�ģ��
 * @date					2017/11/12
 * @author					barry
 */
class DataEncoder
{
public:
	DataEncoder();

	/**
	 * @brief				���ݲɼ�ģ���ʼ��
	 * @param[in]			sPluginPath					ѹ��ģ��·��
	 * @param[in]			sCnfXml						ѹ����������xml
	 * @param[in]			nXCodeBuffSize				Ԥ�������л�����ĳ���
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Initialize( std::string sPluginPath, std::string sCnfXml, unsigned int nXCodeBuffSize );

	/**
	 * @breif				���ݲɼ�ģ���ͷ��˳�
	 */
	void					Release();

	/**
	 * @brief				׼��һ���µ�ѹ��
	 * @param[in]			pTagHead					���ݰ�ͷ(��ѹ���Ĳ���)
	 * @note				ֻ��ͬһ��message���͵����������ѹ��
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Prepare4ACompression( char* pTagHead );

	/**
	 * @brief				���л����ݵ�����
	 * @param[in]			nMsgID						Message ID
	 * @param[in]			pData						��Ϣ���ݵ�ַ
	 * @param[in]			nLen						��Ϣ����
	 * @return				true						�ɹ�
	 */
	bool					CompressData( unsigned short nMsgID, const char *pData, unsigned int nLen );

public:
	/**
	 * @brief				��ȡ�����ַ
	 */
	const char*				GetBufferPtr();

	/**
	 * @brief				��ȡȡ�������ݳ���
	 */
	unsigned int			GetBufferLen();

private:
	Dll						m_oDllPlugin;					///< ���������
	InterfaceEncode*		m_pEncoderApi;					///< ���л�����ָ��
	char*					m_pXCodeBuffer;					///< ���뻺��
	unsigned int			m_nMaxBufferLen;				///< ������󳤶�
	unsigned int			m_nDataLen;						///< �������ݳ���
};







#endif









