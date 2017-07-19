#ifndef __DATA_COLLECTOR_H__
#define	__DATA_COLLECTOR_H__


#include <string>
#include "../Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class				CollectorStatus
 * @brief				��ǰ����Ự��״̬
 * @detail				��������Ҫͨ������жϣ���ϳ�ʼ������ʵ�������ж��Ƿ���Ҫ���³�ʼ���ȶ���
 * @author				barry
 */
class CollectorStatus
{
public:
	CollectorStatus();

public:
	enum E_SS_Status		Get() const;

	bool					Set( enum E_SS_Status eNewStatus );

private:
	mutable CriticalObject	m_oCSLock;
	enum E_SS_Status		m_eStatus;			///< ��ǰ�����߼�״̬�������жϵ�ǰ����ʲô������
};


/**
 * @class					DataCollector
 * @brief					���ݲɼ�ģ�����ע��ӿ�
 * @note					�ɼ�ģ��ֻ�ṩ������ʽ�Ļص�֪ͨ( I_DataHandle: ��ʼ��ӳ�����ݣ� ʵʱ�������ݣ� ��ʼ����ɱ�ʶ ) + ���³�ʼ����������
 * @date					2017/5/3
 * @author					barry
 */
class DataCollector
{
public:
	DataCollector();

	/**
	 * @brief				���ݲɼ�ģ���ʼ��
	 * @param[in]			pIDataCallBack				����ص��ӿ�
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Initialize( I_DataHandle* pIDataCallBack );

	/**
	 * @breif				���ݲɼ�ģ���ͷ��˳�
	 */
	void					Release();

public:///< ���ݲɼ�ģ���¼�����
	/**
 	 * @brief				��ʼ��/���³�ʼ���ص�
	 * @note				ͬ�����������������غ󣬼���ʼ�������Ѿ����꣬�����ж�ִ�н���Ƿ�Ϊ���ɹ���
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						RecoverDataCollector();

	/**
	 * @brief				��ͣ���ݲɼ���
	 */
	void					HaltDataCollector();

	/**
	 * @biref				ȡ�õ�ǰ���ݲɼ�ģ��״̬
	 * @param[out]			pszStatusDesc				���س�״̬������
	 * @param[in,out]		nStrLen						�������������泤�ȣ������������Ч���ݳ���
	 * @return				E_SS_Status״ֵ̬
	 */
	enum E_SS_Status		InquireDataCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief				��ȡ�г����
	 */
	static unsigned int		GetMarketID();

	/**
	 * @brief				�Ƿ�Ϊ���鴫��Ĳɼ����
	 */
	bool					IsProxy();

	/**
	 * @brief				�Ƿ��ڻ��
	 */
	bool					IsAlive();

private:
	static unsigned int		s_nMarketID;					///< ���ݲɼ�����Ӧ���г�ID
	CollectorStatus			m_oCollectorStatus;				///< ���ݲɼ�ģ���״̬
	bool					m_bActivated;					///< �Ƿ��Ѿ�����
	bool					m_bIsProxyPlugin;				///< �Ƿ�Ϊ���������
private:
	Dll						m_oDllPlugin;					///< ���������
	T_Func_Initialize		m_pFuncInitialize;				///< ���ݲɼ�����ʼ���ӿ�
	T_Func_Release			m_pFuncRelease;					///< ���ݲɼ����ͷŽӿ�
	T_Func_RecoverQuotation	m_pFuncRecoverQuotation;		///< ���ݲɼ��������������³�ʼ���ӿ�
	T_Func_HaltQuotation	m_pFuncHaltQuotation;			///< ���ݲɼ�����ͣ�ӿ�
	T_Func_GetStatus		m_pFuncGetStatus;				///< ���ݲɼ���״̬��ȡ�ӿ�
	T_Func_GetMarketID		m_pFuncGetMarketID;				///< ���ݲɼ�����Ӧ���г�ID��ȡ�ӿ�
	T_Func_IsProxy			m_pFuncIsProxy;					///< ���ݲɼ�����Ӧ��ģ�����ͻ�ȡ�ӿ�
};







#endif








