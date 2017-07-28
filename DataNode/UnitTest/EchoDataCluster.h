#ifndef __QLX_DATA_CLUSTER_TEST_H__
#define __QLX_DATA_CLUSTER_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class					DataClusterPlugin
 * @brief					�����ռ��ͻ��˲��Է�װ
 * @author					barry
 */
class DataClusterPlugin : public I_QuotationCallBack
{
public:
	DataClusterPlugin();
	~DataClusterPlugin();

public:
	/**
	 * @brief				�����ռ��ͻ��˵�������Բ���
	 * @return				>=0					�ɹ�
							<0					����
	 */
	int						TestQuotationEcho();

protected:
	/**
	 * @brief				��ʼ��������ģ��
	 * @return				true				�ɹ�
	 */
	bool					Initialize();

	/**
	 * @brief				ֹͣ���ͷ�ģ��
	 */
	void					Release();

protected:
	virtual void			OnQuotation( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf );

protected:
	Dll						m_oDllPlugin;					///< ���������
	T_Func_Activate			m_funcActivate;					///< �����������
	T_Func_Destroy			m_funcDestroy;					///< ���ֹͣ����
	T_Func_Query			m_funcQuery;					///< �����ѯ����
	T_Func_ExecuteUnitTest	m_funcUnitTest;					///< ������Ժ���
};




#endif





