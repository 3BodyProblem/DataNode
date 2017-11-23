#ifndef __QLX_DATA_NODE_TEST_H__
#define __QLX_DATA_NODE_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include "../DataServer/NodeServer.h"


class XCodeMsgPool : public SendPackagePool
{
protected:
	/**
	 * @brief					��package�е��������������·�
	 * @return					���͵����ݴ�С
	 */
	virtual int					SendAllPkg();
};


/**
 * @class							tagMsgInfo
 * @brief							��Ϣ������Ϣ
 * @author							barry
 */
typedef struct
{
	unsigned short					nMsgLen;		///< ��Ϣ����
	unsigned __int64				nMsgCount;		///< ��Ϣ����
} tagMsgInfo;


/**
 * @class				EchoNodeEngine
 * @brief				�����������
 * @author				barry
 */
class EchoNodeEngine : public I_DataHandle
{
public:
	EchoNodeEngine();

	/**
 	 * @brief				��ʼ�������������׼������
	 * @param[in]			nMsgID						��ϢID
	 * @param[in]			sCode						��Ʒ����
	 * @note				�����У��ȴӱ����ļ������ڴ������������ݣ��ٳ�ʼ������������
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	int						Initialize( unsigned int nMsgID, std::string sCode );

	/**
	 * @brief				�ͷ�����ģ�����Դ
	 */
	void					Release();

	/**
	 * @brief				���¼���/��ʼ������(�ڴ��������ݲɼ���)
	 * @detail				��ʼ�����ֵ�����ҵ�����̶��������������
	 * @return				true						��ʼ���ɹ�
							false						ʧ��
	 */
	bool					EchoQuotation();

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
	virtual int				OnImage( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				ʵ���������ݻص�
	 * @note				���������ڴ�飬������
	 * @param[in]			nDataID						��ϢID
	 * @param[in]			pData						��������
	 * @param[in]			nDataLen					����
	 * @param[in]			bLastFlag					�Ƿ�Ϊ���һ�����ı�ʶ
	 * @param[in]			bPushFlag					���ͱ�ʶ
	 * @return				==0							�ɹ�
							!=0							����
	 */
	virtual int				OnData( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag, bool bPushFlag = true );

	/**
	 * @brief				��������ֱ֡��ת���ص��ӿ�
	 * @note				���������ݰ��Ľ�����ֱ��ת������
	 * @param[in]			pData				��������
	 * @param[in]			nDataID				��ϢID
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual int				OnStream( unsigned int nDataID, const char* pData, unsigned int nDataLen );

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
	virtual int				OnQuery( unsigned int nDataID, const char* pData, unsigned int nDataLen );

	/**
	 * @brief				��־����
	 * @param[in]			nLogLevel					��־����[0=��Ϣ��1=������־��2=������־��3=��ϸ��־]
	 * @param[in]			pszFormat					�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

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

public:
	static int				PrintMessage( unsigned int nMarketID, unsigned int nDataID, const char* pData );

protected:///< ������ز��
	XCodeMsgPool			m_oSendPackage;					///< ���ݻ��� + ��/����ģ��
	DataCollector			m_oDataCollector;				///< ����ɼ�ģ��ӿ�
	int						m_nMessageID;					///< ���Ļ��Ե���ϢID( ==0, ��ʾ��ʾȫ�� )
	std::string				m_sCode;						///< ���Ļ��Ե���ϢCode( =="", ��ʾ��ʾȫ�� )
};



#endif





