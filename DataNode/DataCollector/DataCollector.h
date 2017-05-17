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
	enum E_QS_STATUS		Get() const;

	bool					Set( enum E_QS_STATUS eNewStatus );

private:
	mutable CriticalObject	m_oCSLock;
	enum E_QS_STATUS		m_eStatus;			///< ��ǰ�����߼�״̬�������жϵ�ǰ����ʲô������
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
	int						ReInitializeDataCollector();

	/**
	 * @biref				ȡ�õ�ǰ���ݲɼ�ģ��״̬
	 */
	const CollectorStatus&	InquireDataCollectorStatus();

private:
	Dll						m_oDllPlugin;					///< ���������
};







#endif








