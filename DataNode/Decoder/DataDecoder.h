#ifndef __DATA_DECODER_H__
#define	__DATA_DECODER_H__


#include <string>
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../../../DataXCode/DataXCode/IXCode.h"


/**
 * @class					DataDecoder
 * @brief					���������л�ģ��
 * @date					2017/11/12
 * @author					barry
 */
class DataDecoder
{
public:
	DataDecoder();

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
	int						Prepare4AUncompression( const char* pTagHead );

	/**
	 * @brief				���л����ݵ�����
	 * @param[in]			nMsgID						Message ID
	 * @param[in]			pData						��Ϣ���ݵ�ַ
	 * @param[in]			nLen						��Ϣ����
	 * @return				true						�ɹ�
	 */
	bool					UncompressData( unsigned short nMsgID, char *pData, unsigned int nLen );

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
	InterfaceDecode*		m_pDecoderApi;					///< �����л�����ָ��
	char*					m_pXCodeBuffer;					///< ���뻺��
	unsigned int			m_nMaxBufferLen;				///< ������󳤶�
	unsigned int			m_nDataLen;						///< �������ݳ���
};







#endif









