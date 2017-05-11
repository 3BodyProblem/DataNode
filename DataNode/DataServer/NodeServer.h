#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <string>
#include "SvrConfig.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "ServiceIO/MServicePlug.h"
#include "ServiceIO/MServicePlug.hpp"
#include "../InitializeFlag/InitFlag.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../DataCollector/DataCollector.h"


/**
 * @class					SvrFramework
 * @brief					��������
 * @detail					��װ�˷���ע�ᡢ�����շ���״̬֪ͨ�Ȼ�������
 * @date					2017/5/2
 * @author					barry
 */
class SvrFramework : public MServicePlug
{
private:
	SvrFramework();

public:
	~SvrFramework();

	/**
	 * @brief				ȡ�÷����ܲ��
	 * @return				���ط����ܲ���Ķ�������
	 */
	static SvrFramework&	GetFramework();

	/**
	 * @brief				��ʼ��������
	 * @note				�ڼ����������ļ��󣬽����ϱ�����
	 * @return				==0					�ɹ�
							!=0					ʧ��
	 */
	int						Initialize();
};


/**
 * @class					DataEngine
 * @brief					ȫ���ݻỰ��������
 * @detail					����/Э������ģ��(���ݲɼ����+�����ڴ���+����ѹ�����)
 * @date					2017/5/3
 * @author					barry
 */
class DataEngine : public I_DataHandle, public SimpleTask
{
public:
	DataEngine();

	/**
 	 * @brief				��ʼ�������������׼������
	 * @note				�����У��ȴӱ����ļ������ڴ������������ݣ��ٳ�ʼ������������
	 * @param[in]			sDataCollectorPluginPath	����������·��
	 * @param[in]			sMemPluginPath				���������ڴ���·��
	 * @param[in]			sHolidayPath				�ڼ����ļ�·��
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	int						Initialize( const std::string& sDataCollectorPluginPath, const std::string& sMemPluginPath, const std::string& sHolidayPath );

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
	 * @param[in]			bPushFlag			���ͱ�ʶ
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief				������ϢID/Code��ѯĳ��������(ͨ���ڴ����ݲ���ӿ�)
	 * @param[in]			nDataID				��ϢID
	 * @param[in,out]		pData				��ƷCode[in],���ݲ�ѯ����[out]
	 * @param[in]			nDataLen			���泤��
	 * @param[in]			bLastFlag			�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @return				true				��ѯ�ɹ�
	 */
	virtual bool			OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

protected:
	/**
	 * @brief				������(��ѭ��)
	 * @return				==0					�ɹ�
							!=0					ʧ��
	 */
	virtual int				Execute();

	/**
	 * @brief				����״̬����: �ڴ���������/���鳬ʱ��...
	 * @note				���ԣ������ڴ����������ļ��Ĵ�ȡ����Ҫ�ڴ����ݲ���ı�ʶ�ӿ�֧��
	 */
	virtual int				OnIdle() = 0;

protected:
	DatabaseIO				m_oDatabaseIO;					///< �ڴ����ݲ������
	DataCollector			m_oDataCollector;				///< ����ɼ�ģ��ӿ�
//	XXXCompress				m_oCompressObj;					///< ����ѹ��ģ��
};


/**
 * @class					DataNodeService
 * @brief					�������������	(������)
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
	DataNodeService();
	~DataNodeService();

public:
	/**
	 * @brief				��ʼ��&�����������
	 * @return				==0				�����ɹ�
							!=0				��������
	 */
	int						Activate();

	/**
	 * @brief				�����������
	 */
	void					Destroy();

public:
	/**
	 * @brief				����״̬����: �ڴ���������/���鳬ʱ��...
	 * @note				���ԣ������ڴ����������ļ��Ĵ�ȡ����Ҫ�ڴ����ݲ���ı�ʶ�ӿ�֧��
	 */
	virtual int				OnIdle();

	/**
	 * @brief				�����ڴ����е���������
	 */
	void					OnBackupDatabase();

	/**
	 * @brief				ѯ�����ݲɼ�ģ���״̬
	 */
	void					OnInquireStatus();

	/**
	 * @brief				���������Ƿ���ṩ���·���
	 * @return				true		�����ʼ���ɹ����ɷ���
							false		δ��ʼ��/����ʼ���У����ɷ���
	 */
	bool					IsAvailable();
};





#endif








