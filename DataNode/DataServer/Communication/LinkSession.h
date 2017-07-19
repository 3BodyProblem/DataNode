#ifndef __COMMUNICATION_COMM_IO_H__
#define	__COMMUNICATION_COMM_IO_H__


#include <set>
#include "DataStream.h"
#include "../../Infrastructure/Lock.h"
#include "../ServiceIO/MServicePlug.h"
#include "../ServiceIO/MServicePlug.hpp"
#include "../../MemoryDB/MemoryDatabase.h"
#include "../../DataCollector/DataCollector.h"


#define							MAX_LINKID_NUM		32
typedef							unsigned int		LINKID_VECTOR[MAX_LINKID_NUM];


/**
 * @class						LinkNoRegister
 * @brief						����/ά������·��ID��
 * @detail						�����ӵ���ʱ�����ID��ɾ��ʧЧ����·ID
 * @author						barry
 */
class LinkNoRegister
{
private:
	LinkNoRegister();

public:
	static LinkNoRegister&		GetRegister();

public:///< ������·��
	/**
	 * @brief					���һ���µ�����·ID
	 * @detail					ֻ��Ҫ�����ID������Ҫ�����Ƿ����ظ����ڲ�������Ԥ���ж�
	 * @param[in]				nNewLinkID			����·ID
	 */
	int							NewPushLinkID( unsigned int nNewLinkID );

	/**
	 * @brief					�Ƴ�һ��ʧЧ����·ID
	 * @param[in]				nRemoveLinkID		ʧЧ��ID
	 */
	void						RemovePushLinkID( unsigned int nRemoveLinkID );

	/**
	 * @brief					��ȡ�Ựid��
	 */
	unsigned int				FetchLinkNoTable( unsigned int* pIDTable, unsigned int nBuffSize );

	/**
	 * @brief					��ȡ������·����
	 */
	int							GetPushLinkCount();

public:///< ������·��
	/**
	 * @brief					����һ��������·��
	 */
	int							NewReqLinkID( unsigned int nReqLinkID );

	/**
	 * @brief					��ȡ������·����
	 */
	int							GetReqLinkCount();

	/**
	 * @brief					����һ��������·��
	 */
	int							PopReqLinkID();

	/**
	 * @brief					�Ƿ���������·������
	 */
	bool						InReqLinkIDSet( unsigned int nLinkID );

private:
	CriticalObject				m_oLock;				///< ��
	LINKID_VECTOR				m_vctLinkNo;			///< ������·��
	std::set<unsigned int>		m_setPushLinkID;		///< ʵʱ������·�ż���,���ڷ��㴦���ظ�ID,���ж�ID�Ƿ��Ѿ�����
	int							m_nLinkCount;			///< ��·����
	std::set<unsigned int>		m_setNewReqLinkID;		///< ����ʼ����·ID����
};


///< -------------------------------------------------------------------------------------------


/**
 * @class						ImageDataQuery
 * @brief						����������ݲ�ѯ����
 * @author						barry
 */
class ImageDataQuery
{
private:
	ImageDataQuery();
public:
	~ImageDataQuery();

	static ImageDataQuery&		GetImageQuery();

	/**
	 * @brief					��ʼ��
	 * @return					!= 0				ʧ��
	 */
	int							Instance( DatabaseIO* pDbIO, QuotationSynchronizer* pQuotBuf );

	/**
	 * @brief					�ͷ���Դ
	 */
	void						Release();

public:///< ��ʼ���������ͽӿ�
	/**
	 * @brief					��ȫ����ʼ�����鷢���µ������·
	 * @param[in]				nSerialNo				���Ͳ�ѯ���(��Ҫ>nSerialNo)
	 * @return					>=0						ͬ������·��
								<0						����
	 */
	int							FlushImageData2NewSessions( unsigned __int64 nSerialNo = 0 );

	/**
	 * @brief					��ȡ�ڴ����ݿ�ĳ���ݱ�ĵ�������Ʒ����
	 * @param[in]				nDataID					���ݱ�ID
	 * @param[in]				nRecordLen				���ݱ��Ӧ���ݰ�����
	 * @param[out]				setCode					���ݱ���������
	 * @return					>=0						�����е�Ԫ������
								<0						����
	 */
	int							QueryCodeListInDatabase( unsigned int nDataID, unsigned int nRecordLen, std::set<std::string>& setCode );

public:
	/**
	 * @brief					��ʽ���������ݻ���
	 * @return					���ظ�ʽ��������ݳ���
	 */
	unsigned int				FormatImageBuffer( unsigned int nSeqNo, unsigned int nDataID, unsigned int nDataWidth, unsigned int nBuffDataLen );

	/**
	 * @brief					��ȡ�ڴ����ݿ�ָ��
	 */
	DatabaseIO*					GetMemoDbPtr();

protected:
	CriticalObject				m_oLock;				///< ��ʼ���������ͻ�����
	PkgBuffer					m_oOnePkg;				///< ���ݷ��ͻ���
protected:
	DatabaseIO*					m_pDatabase;			///< ���ݲ�������ָ��
	QuotationSynchronizer*		m_pQuotationBuffer;		///< ʵʱ�������ͻ��棨�������߳�)
};


/**
 * @class						SessionCollection
 * @brief						�¼���·���ݻص��¼� + ʵʱ����������
 * @author						barry
 */
class SessionCollection : public MServicePlug_Spi
{
public:
	SessionCollection( DatabaseIO& refDbIO );
	~SessionCollection();

	/**
	 * @brief					��ʼ��
	 * @return					!= 0				ʧ��
	 */
	int							Instance();

	/**
	 * @brief					�ͷ���Դ
	 */
	void						Release();

public:///< �������ͽӿ�
	/**
	 * @brief					��ʵʱ���͵����ݷŽ�����
	 */
	void						PushQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize, bool bPushFlag, unsigned __int64 nSerialNo );

protected:///< �������¼��ص�
	/**
	 * @brief					�����ӵ�����Ӧ����������false��ʾ�����ܸ����ӣ���������Ͽ�������
	 * @param[in]				uiLinkNo				��·������
	 * @param[in]				uiIpAddr				ip
	 * @param[in]				uiPort					�˿�
	 * @return					false					����false��ʾ�����ܸ����ӣ���������Ͽ�������
	 */
	virtual bool				OnNewLink( unsigned int uiLinkNo, unsigned int uiIpAddr, unsigned int uiPort );

	/**
	 * @brief					������״̬��Ӧ����������״̬ʱ�ص�
	 * @param[out]				szStatusInfo			������״̬��Ϣ���ַ�����ʽ����������״̬��Ϣ����д���ؼ�
	 * @param[in]				uiSize					״̬��Ϣ�������󳤶�
	 * @note					�����ʽ�硰����Ԫ��Ϣ\n + ����Ԫ��Ϣ\n + ����Ԫ��Ϣ\n + ...�����س����зָ�
	 *							���� ����Ԫ��Ϣ = ������Ԫ����:����1,����2,����3,����4,����5,...�������ŷָ�
	 *							���� ���� = ��[������]�� �� ���� = ��working = true�� �� ���� = "keyname = value" �� ���� = "(n)keyname = value" ������n��ʾ���赥Ԫ��������Ĭ��1��
	 */
	virtual void				OnReportStatus( char* szStatusInfo, unsigned int uiSize );

	/**
	 * @brief					���������command��Ӧ����(���ڿ�������/��������)
	 * @param[in]				szSrvUnitName			����Ԫ����
	 * @param[in]				szCommand				��������
	 * @param[out]				szResult				����ؽ��
	 * @param[in]				uiSize					����ػ�����󳤶�
	 * @return					true					ִ�гɹ�
	 */
	virtual bool				OnCommand( const char* szSrvUnitName, const char* szCommand, char* szResult, unsigned int uiSize );

	/**
	 * @brief					���ӹر���Ӧ����
	 * @param[in]				uiLinkNo				��·������
	 * @param[in]				iCloseType				�ر�����: 0 ����ͨѶ���� 1 WSARECV�������� 2 ����������ر� 3 �ͻ��������ر� 4 �������ݴ�����ر�
	 */
	virtual void				OnCloseLink( unsigned int uiLinkNo, int iCloseType );

	/**
	 * @brief					���յ�������Ӧ����
	 * @param[in]				uiLinkNo				��·������
	 * @param[in]				usMessageNo				��Ϣ���
	 * @param[in]				usFunctionID			��Ϣ���ܺ�
	 * @param[in]				bErrorFlag				�����ʶ
	 * @param[in]				lpData					���ݻ���ָ��
	 * @param[in]				uiSize					������Ч����
	 * @param[in,out]			uiAddtionData			��������
	 * @return					����false��ʾ�������ݴ��󣬷�������Ͽ�������
	 */
	virtual bool				OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData );

protected:///< �µ������·��ʼ���߼����
	DatabaseIO&					m_refDatabase;			///< �����ڴ��
	QuotationSynchronizer		m_oQuotationBuffer;		///< ʵʱ�������ͻ��棨�������߳�)
};




#endif









