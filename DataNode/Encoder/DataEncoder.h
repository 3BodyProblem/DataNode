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
	 * @param[in]			sModulePath					ѹ��ģ��·��
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Initialize( std::string sModulePath );

	/**
	 * @breif				���ݲɼ�ģ���ͷ��˳�
	 */
	void					Release();

private:
	Dll						m_oDllPlugin;					///< ���������
	T_Func_GetEncodeApi		m_pFuncEncodeApi;				///< ���뵼������
};







#endif









