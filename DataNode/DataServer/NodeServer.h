#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <map>
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
public:///< ���湹��ͳ�ʼ����ع���
	DataIOEngine();
	~DataIOEngine();

	/**
 	 * @brief				��ʼ�������������׼������
	 * @note				�����У��ȴӱ����ļ������ڴ������������ݣ��ٳ�ʼ������������
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	int						Initialize();

	/**
	 * @brief				�ͷ�����ģ�����Դ
	 */
	void					Release();

public:///< I_DataHandle�ӿ�ʵ��: ���ڸ����ݲɼ�ģ���ṩ�������ݵĻص�����
	/**
 	 * @brief				��ʼ�����ʵ��������ݻص�
	 * @note				ֻ�Ǹ��¹�����������ݵ��ڴ��ʼ�ṹ��������
	 * @param[in]			nDataID						��ϢID
	 * @param[in]			pData						��������
	 * @param[in]			nDataLen					����
	 * @param[in]			bLastFlag					�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @return				==0							�ɹ�
							!=0							����
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				ʵ���������ݻص�
	 * @note				���������ڴ�飬������
	 * @param[in]			nDataID						��ϢID
	 * @param[in]			pData						��������
	 * @param[in]			nDataLen					����
	 * @param[in]			bPushFlag					���ͱ�ʶ
	 * @return				==0							�ɹ�
							!=0							����
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief				�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]			nDataID						��ϢID
	 * @param[in,out]		pData						��������(������ѯ����)
	 * @param[in]			nDataLen					����
	 * @return				>0							�ɹ�,�������ݽṹ�Ĵ�С
							==0							û�鵽���
							!=0							����
	 * @note				���pData�Ļ���Ϊ��ȫ�㡱���棬�򷵻ر��ڵ���������
	 */
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
	 * @brief				��־����
	 * @param[in]			nLogLevel					��־����[0=��Ϣ��1=������־��2=������־��3=��ϸ��־]
	 * @param[in]			pszFormat					�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

protected:///< �߳�������غ���
	/**
	 * @brief				������(��ѭ��)
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	virtual int				Execute();

	/**
	 * @brief				����״̬����: �ڴ���������/���鳬ʱ��...
	 * @note				���ԣ������ڴ����������ļ��Ĵ�ȡ����Ҫ�ڴ����ݲ���ı�ʶ�ӿ�֧��
	 */
	virtual int				OnIdle() = 0;

	/**
	 * @brief				���¼���/��ʼ������(�ڴ��������ݲɼ���)
	 * @detail				��ʼ�����ֵ�����ҵ�����̶��������������
	 * @return				true						��ʼ���ɹ�
							false						ʧ��
	 */
	bool					PrepareQuotation();

protected:///< ˽�й��ܺ���
	/**
	 * @brief				���ڴ��м����������ݱ��¹�������Ʒ����
	 * @return				>=0							�ɹ�,�������ݱ�����
							<0							ʧ��
	 */
	int						LoadCodesListInDatabase();

	/**
	 * @brief				ɾ���ڴ���еķ���Ч��Ʒ��¼
	 * @return				����ɾ��������
	 */
	int						RemoveCodeExpiredInDatabase();

protected:///< ͳ�Ƴ�Ա����
	unsigned __int64		m_nPushSerialNo;				///< ʵʱ���������ˮ
	unsigned int			m_nHeartBeatCount;				///< ���͵�����������
	CriticalObject			m_oCodeMapLock;					///< CodeMap��
	MAP_TABLEID_CODES		m_mapID2Codes;					///< ��¼����ϢID�µĹ���codes
protected:///< ���ܳ�Ա����
	InitializerFlag			m_oInitFlag;					///< ���³�ʼ����ʶ
	SessionCollection		m_oLinkSessions;				///< �¼�����·�Ự
protected:///< ������ز��
	PowerfullDatabase		m_oDatabaseIO;					///< �ڴ����ݲ������
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
 * @note					���ڹ����˴�������������������ԭ���ǳ��˳�ʼ��������ʱ�䶼һֱ�������Ӳ��Ͽ�!
 * @date					2017/5/3
 * @author					barry
 */
class DataNodeService : public DataIOEngine
{
private:
	DataNodeService();
public:
	/**
	 * @brief				ȡ�÷������ĵ�������
	 */
	static DataNodeService&	GetSerivceObj();

public:///< Scheduled Task
	/**
	 * @brief				����״̬����: �ڴ���������/���鳬ʱ��...
	 * @note				���ԣ������ڴ����������ļ��Ĵ�ȡ����Ҫ�ڴ����ݲ���ı�ʶ�ӿ�֧��
	 */
	virtual int				OnIdle();

	/**
	 * @brief				����������·ά��
	 */
	void					OnHeartBeat();

	/**
	 * @brief				�����ڴ����е���������
	 */
	void					OnBackupDatabase();

public:///< Inquiry Interface
	/**
	 * @brief				ѯ�����ݲɼ�ģ���״̬
	 * @param[out]			pszStatusDesc			���س�״̬������
	 * @param[in,out]		nStrLen					�������������泤�ȣ������������Ч���ݳ���
	 * @return				true					�ɷ���
	 */
	bool					OnInquireStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief				��ȡ���͵�����������
	 * @return				������ͳ������
	 */
	unsigned int			OnInquireHeartBeatCount();

public:///< Log Method
	/**
	 * @brief				��Ϣ
	 */
	virtual void			WriteInfo( const char * szFormat,... );

	/**
	 * @brief				����
	 */
	virtual void			WriteWarning( const char * szFormat,... );

	/**
	 * @brief				����
	 */
	virtual void			WriteError( const char * szFormat,... );

	/**
	 * @brief				��ϸ
	 */
	virtual void			WriteDetail( const char * szFormat,... );
};





#endif








