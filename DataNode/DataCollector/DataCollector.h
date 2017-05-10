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








