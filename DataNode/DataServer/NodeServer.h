#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <string>
#include "SvrConfig.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "ServiceIO/MServicePlug.h"
#include "ServiceIO/MServicePlug.hpp"
#include "Communication/LinkSession.h"
#include "../InitializeFlag/InitFlag.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../DataCollector/DataCollector.h"


/**
 * @class					DataIOEngine
 * @brief					�������ݸ��¹�������(��Ҫ��װ���ݳ�ʼ���͸���/���͵�ҵ��)
 * @detail					����/Э������ģ��(���ݲɼ����+�����ڴ���+����ѹ�����)
 * @note					��Ҫ�ṩ��������������صĻ�������: �ɼ�������������µ��ڴ� + �������ݳ�ʼ�������߼� + �������ݶ��¼��������ܷ�װ
							&
							���������ÿ�ճ�ʼ���Ѿ������˽ڼ��յ����
 * @date					2017/5/3
 * @author					barry
 */
class DataIOEngine : public I_DataHandle, public SimpleTask, public MServicePlug
{
public:
	DataIOEngine();

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
	 * @brief				��־����
	 * @param[in]			nLogLevel			��־����[0=��Ϣ��1=������־��2=������־��3=��ϸ��־]
	 * @param[in]			pszFormat			�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

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
	InitializerFlag			m_oInitFlag;					///< ���³�ʼ����ʶ
	LinkSessions			m_oLinkSessions;				///< �¼�����·�Ự

protected:
	DatabaseIO				m_oDatabaseIO;					///< �ڴ����ݲ������
	DataCollector			m_oDataCollector;				///< ����ɼ�ģ��ӿ�
//	XXXCompress				m_oCompressObj;					///< ����ѹ��ģ��
};


/**
 * @class					DataNodeService
 * @brief					�������������	(������)
 * @detail					��չ��Ϊ���������Ҫ��һЩ���ݸ��¹���������߼����ܣ�
							a) ��������/ͣ����
							b) ��������������ݶ�ʱ���̱���
							c) ���������״̬��ʱͨ��
 * @date					2017/5/3
 * @author					barry
 */
class DataNodeService : public DataIOEngine
{
private:
	DataNodeService();
public:
	~DataNodeService();

	/**
	 * @brief				ȡ�÷������ĵ�������
	 */
	static DataNodeService&	GetSerivceObj();

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
};





#endif








