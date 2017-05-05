#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <string>
#include "SvrConfig.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Initializer/Initializer.h"
#include "../DataCollector/DataCollector.h"


/**
 * @class					DataEngine
 * @brief					ȫ���ݻỰ��������
 * @detail					����/Э������ģ��(���ݲɼ����+�����ڴ���+����ѹ�����)
 * @date					2017/5/3
 * @author					barry
 */
class DataEngine : public I_DataHandle, protected SimpleTask
{
public:
	DataEngine();

	/**
 	 * @brief				��ʼ�������������׼������
	 * @note				�����У��ȴӱ����ļ������ڴ������������ݣ��ٳ�ʼ������������
	 * @param[in]			sDataDriverPluginPath		����������·��
	 * @param[in]			sMemPluginPath				���������ڴ���·��
	 * @param[in]			sHolidayPath				�ڼ����ļ�·��
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	int						Initialize( const std::string& sDataDriverPluginPath, const std::string& sMemPluginPath, const std::string& sHolidayPath );

	/**
	 * @brief				�ͷ�����ģ�����Դ
	 */
	void					Release();

public:///< I_DataHandle�ӿ�ʵ��: ���ڸ����ݲɼ�ģ���ṩ�������ݵĻص�����
	/**
 	 * @brief				��ʼ�����ʵ��������ݻص�
	 * @note				ֻ�Ǹ��¹�����������ݵ��ڴ��ʼ�ṹ��������
	 * @param[in]			nDataID				��ϢID
	 * @param[in]			pData				��������
	 * @param[in]			nDataLen			����
	 * @param[in]			bLastFlag			�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				ʵ���������ݻص�
	 * @note				���������ڴ�飬������
	 * @param[in]			nDataID				��ϢID
	 * @param[in]			pData				��������
	 * @param[in]			nDataLen			����
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
	 * @brief				������ϢID/Code��ѯĳ��������(ͨ���ڴ����ݲ���ӿ�)
	 * @param[in]			nDataID				��ϢID
	 * @param[in,out]		pData				��ƷCode[in],���ݲ�ѯ����[out]
	 * @param[in]			nDataLen			���泤��
	 * @return				true				��ѯ�ɹ�
	 */
	virtual bool			OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen );

protected:
	/**
	 * @brief				������(��ѭ��)
	 * @return				==0					�ɹ�
							!=0					ʧ��
	 */
	virtual int				Execute();

private:
	EngineInitializer		m_oInitializer;					///< ����������
	DataCollector			m_oDataCollector;				///< ����ɼ�ģ��ӿ�
//	IMemDatabase*			m_pIQueryMemDatabase;			///< �������ݱ������ݿ�ӿ�,�����ṩ��ѯ����

private:
//	XXXCompress				m_oCompressObj;					///< ����ѹ��ģ��
//	pthread					m_oWorkFlowThread;				///< ����Ự�����߳̾��
};


/**
 * @class					DataNodeService
 * @brief					�������������
 * @detail					�漰��ҵ����
							a) ���ݲɼ������ÿ���Զ���ʼ��
							b) ���ݲɼ����֪ͨ�ص���ʽ������ʼ������+��������֪ͨ��Engine����
							c) Engine����ά�ֺø��׶ε�����ҵ��״̬������״̬������ȷ�ķ�Ӧ(�����������³�ʼ��,����...)
							d) Engine�����Զ����ڴ��е�������������
							e) Engine�����ʼ��ʱ�Զ��������������ݸ��µ��ڴ����(ע�����������޲�����룬���ڴ��б����һ�������һ�ο����õ�����������)
							f) Engine�����ṩ��Dll�����ӿڵ�֧��
							g) ��Ҫ�нڼ����߼�֧��
							h) ������˳�ʼ���׶Σ�����ʱ�䶼Ӧ��Ϊ�ɷ���״̬
 * @date					2017/5/3
 * @author					barry
 */
class DataNodeService : public DataEngine
{
public:
	/**
	 * @brief				�����������
	 */
	int						Active( /*tagDll_DataCenterInput* pIn, tagDll_DriverOutput* pOut*/ );

	/**
	 * @brief				�����������
	 */
	int						Destroy();

	/**
	 * @brief				����״̬����: �ڴ���������/���鳬ʱ��...
	 * @note				���ԣ������ڴ����������ļ��Ĵ�ȡ����Ҫ�ڴ����ݲ���ı�ʶ�ӿ�֧��
	 */
	int						OnIdle();

public:///< ��Ҫ֧��DLL���������ķ���
	/**
	 * @brief				������İ汾����
	 */
	unsigned long			VersionOfEngine();

	/**
	 * @brief				���������Ƿ���ṩ���·���
	 * @return				true		�����ʼ���ɹ����ɷ���
							false		δ��ʼ��/����ʼ���У����ɷ���
	 */
	bool					IsAvailable();

	/**
	 * @brief				ȡ�÷�����Ϣ
	 */
//	void					GetServerStatus( tagDll_DriverStatusInfo* pStatusinfo );

	/**
	 * @brief				ȡ�û�����Ϣ (����������̨֧��/��ӡ/������)
	 */
//	void					GetServerBaseInfo( tagDll_DriverBaseInfo* drvinfo );
/*
///< dll �����ӿ�
DLLEXPORT void APIENTRY GetDriverBaseInfo( tagDll_DriverBaseInfo * drvinfo );
DLLEXPORT int APIENTRY GetMarketInfo( tagDll_GetMarketInfoIn* pInInfo, tagDll_GetMarketInfoOut* pOutInfo );

///< dll �����Ŀ��ƽӿ�
DLLEXPORT int APIENTRY GetDataFrameCount( tagDll_GetDataFrameCountIn * pInParam, tagDll_GetFrameCountOut * pOutParam );
DLLEXPORT int APIENTRY GetOneDataFrame( tagDll_GetOneDataFrameIn * pInParam, tagDll_GetOneDataFrameOut * pOutParam );

private:///< ������������Ϣ
	bool					m_bTestFlag;			///< ģ����Ա�ʶ
	std::set<unsigned int>	m_setWhiteTableOfMsgID;		///< MessageID�İ�����, �����ڴ��������
	std::string				m_sHolidayPath;			///< �ڼ����ļ�����·��
	std::string				m_sMemDBPluginPath;		///< �������ݱ�����·��
	std::string				m_sDataDriverPluginPath;	///< �������ݲɼ����·��
	std::string				m_sCompressModulePath;		///< ����ѹ��ģ����·��
*/
};





#endif








