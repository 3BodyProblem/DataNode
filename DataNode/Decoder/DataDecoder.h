#ifndef __DATA_DECODER_H__
#define	__DATA_DECODER_H__


#include "../../../DataXCode/DataXCode/IXCode.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Dll.h"


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
	 * @param[in]			pData						�������ַ(Body����)
	 * @param[in]			nLen						�����峤��
	 * @note				ֻ��ͬһ��message���͵����������ѹ��
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Prepare4AUncompression( const char* pData, unsigned int nLen );

	/**
	 * @brief				���л����ݵ�����
	 * @param[in]			nMsgID						Message ID
	 * @param[in]			pData						��Ϣ���ݵ�ַ
	 * @param[in]			nLen						��Ϣ����
	 * @return				>= 0						�ɹ����������ε��ú�,δ���л����ֵĳ���
	 */
	int						UncompressData( unsigned short nMsgID, char *pData, unsigned int nLen );

private:
	Dll						m_oDllPlugin;					///< ���������
	InterfaceDecode*		m_pDecoderApi;					///< �����л�����ָ��
};







#endif










