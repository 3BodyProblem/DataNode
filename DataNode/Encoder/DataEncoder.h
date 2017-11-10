#ifndef __DATA_ENCODER_H__
#define	__DATA_ENCODER_H__


#include <string>
#include "../Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class					DataEncoder
 * @brief					���ݲɼ�ģ�����ע��ӿ�
 * @note					�ɼ�ģ��ֻ�ṩ������ʽ�Ļص�֪ͨ( I_DataHandle: ��ʼ��ӳ�����ݣ� ʵʱ�������ݣ� ��ʼ����ɱ�ʶ ) + ���³�ʼ����������
 * @date					2017/5/3
 * @author					barry
 */
class DataEncoder
{
public:
	DataEncoder();

	/**
	 * @brief				���ݲɼ�ģ���ʼ��
	 * @param[in]			pIDataCallBack				����ص��ӿ�
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Initialize();

	/**
	 * @breif				���ݲɼ�ģ���ͷ��˳�
	 */
	void					Release();

private:
	Dll						m_oDllPlugin;					///< ���������
/*	T_Func_Initialize		m_pFuncInitialize;				///< ���ݲɼ�����ʼ���ӿ�
	T_Func_Release			m_pFuncRelease;					///< ���ݲɼ����ͷŽӿ�
	T_Func_RecoverQuotation	m_pFuncRecoverQuotation;		///< ���ݲɼ��������������³�ʼ���ӿ�
	T_Func_HaltQuotation	m_pFuncHaltQuotation;			///< ���ݲɼ�����ͣ�ӿ�
	T_Func_GetStatus		m_pFuncGetStatus;				///< ���ݲɼ���״̬��ȡ�ӿ�
	T_Func_GetMarketID		m_pFuncGetMarketID;				///< ���ݲɼ�����Ӧ���г�ID��ȡ�ӿ�
	T_Func_IsProxy			m_pFuncIsProxy;					///< ���ݲɼ�����Ӧ��ģ�����ͻ�ȡ�ӿ�
	T_Echo					m_pFuncEcho;					///< ���ݲɼ������������ݽ��н���/���ԵĽӿ�*/
};







#endif








