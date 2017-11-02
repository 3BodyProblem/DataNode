#ifndef __QLX_DATA_CLUSTER_TEST_H__
#define __QLX_DATA_CLUSTER_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "../../../DataCluster/DataCluster/Interface.h"
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
	 * @param[in]			nMsgID				ֻ��ʾ��MessageID, -1��ʾ������
	 * @param[in]			sKey				ֻ��ʾ�ļ�¼������ ""��ʾ������
	 * @return				>=0					�ɹ�
							<0					����
	 */
	int						TestQuotationEcho( int nMsgID, std::string sKey );

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
	virtual void			OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus );
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf );

protected:
	int								m_nMessageID;					///< ֻ��ʾ����ϢID
	std::string						m_sMessageKey;					///< ֻ��ʾ�ļ�¼Code

protected:
	Dll								m_oDllPlugin;					///< ���������
	tagQUOFun_StartWork				m_funcActivate;					///< �����������
	tagQUOFun_EndWork				m_funcDestroy;					///< ���ֹͣ����
	tagQUOFun_GetMarketID			m_funcGetMarketID;				///< ����г�ID���ѯ����
	tagQUOFun_GetMarketInfo			m_funcGetMarketInfo;			///< �����ѯ����
	tagQUOFun_GetAllReferenceData	m_funcGetAllRefData;			///< ��ȡ���вο�����
	tagQUOFun_GetReferenceData		m_funcGetRefData;				///< ��ȡ�ο�����
	tagQUOFun_GetAllSnapData		m_funcGetAllSnapData;			///< ��ȡ���п�������
	tagQUOFun_GetSnapData			m_funcGetSnapData;				///< ��ȡ��������
	T_Func_ExecuteUnitTest			m_funcUnitTest;					///< ������Ժ���
};




#endif





