#pragma warning(disable:4996)
#include <time.h>
#include "LinkSession.h"
#include "../DataEcho.h"
#include "../NodeServer.h"


LinkNoRegister::LinkNoRegister()
 : nLinkIDCount( 0 )
{}

LinkNoRegister& LinkNoRegister::GetRegister()
{
	static LinkNoRegister		obj;

	return obj;
}

int LinkNoRegister::NewPushLinkID( unsigned int nNewLinkID )
{
	CriticalLock	guard( m_oLock );

	///< IDδ��ӣ��������
	if( m_setPushLinkID.find( nNewLinkID ) == m_setPushLinkID.end() )
	{
		m_setPushLinkID.insert( nNewLinkID );
		nLinkIDCount = m_setPushLinkID.size();
		return 1;
	}

	return 0;
}

void LinkNoRegister::RemovePushLinkID( unsigned int nRemoveLinkID )
{
	CriticalLock	guard( m_oLock );

	///< �������ID�������Ƴ�
	if( m_setPushLinkID.find( nRemoveLinkID ) != m_setPushLinkID.end() )
	{
		m_setPushLinkID.erase( nRemoveLinkID );
		nLinkIDCount = m_setPushLinkID.size();
	}
}

int LinkNoRegister::GetPushLinkCount()
{
	return nLinkIDCount;
}

unsigned int LinkNoRegister::FetchPushLinkIDList( unsigned int * lpLinkNoArray, unsigned int uiArraySize )
{
	unsigned int	nLinkNum = 0;				///< ��Ч��·����
	static	int		s_nLastLinkNoNum = 0;		///< ��һ�ε���·����
	CriticalLock	guard( m_oLock );			///< ��

	if( m_setPushLinkID.size() != s_nLastLinkNoNum )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "LinkNoRegister::FetchPushLinkIDList() : TCP connection number of QServer fluctuated! new no. = %d, old no. = %d", m_setPushLinkID.size(), s_nLastLinkNoNum );
		s_nLastLinkNoNum = m_setPushLinkID.size();
	}

	for( std::set<unsigned int>::iterator it = m_setPushLinkID.begin(); it != m_setPushLinkID.end() && nLinkNum < uiArraySize; it++ )
	{
		lpLinkNoArray[nLinkNum++] = *it;
	}

	return nLinkNum;
}

int LinkNoRegister::NewReqLinkID( unsigned int nReqLinkID )
{
	CriticalLock	guard( m_oLock );			///< ��

	///< IDδ��ӣ��������
	if( m_setNewReqLinkID.find( nReqLinkID ) == m_setNewReqLinkID.end() )
	{
		m_setNewReqLinkID.insert( nReqLinkID );
		return 1;
	}

	return 0;
}

int LinkNoRegister::GetReqLinkCount()
{
	CriticalLock	guard( m_oLock );			///< ��

	return m_setNewReqLinkID.size();
}

int LinkNoRegister::PopReqLinkID()
{
	int									nReqLinkID = 0;
	CriticalLock						guard( m_oLock );			///< ��

	if( m_setNewReqLinkID.empty() )
	{
		return -1;
	}

	std::set<unsigned int>::iterator	it = m_setNewReqLinkID.begin();

	nReqLinkID = *it;
	m_setNewReqLinkID.erase( it );		///< ���»Ự��idɾ��

	return nReqLinkID;
}

bool LinkNoRegister::InReqLinkIDSet( unsigned int nLinkID )
{
	CriticalLock						guard( m_oLock );			///< ��

	if( m_setNewReqLinkID.find( nLinkID ) != m_setNewReqLinkID.end() )
	{
		return true;
	}

	return false;
}


ImageDataQuery::ImageDataQuery()
 : m_pSendBuffer( NULL ), m_pDatabase( NULL ), m_pQuotationBuffer( NULL )
{
}

ImageDataQuery::~ImageDataQuery()
{
	Release();
}

int ImageDataQuery::Instance( DatabaseIO* pDbIO, QuotationSynchronizer* pQuotBuf )
{
	m_pDatabase = pDbIO;
	m_pQuotationBuffer = pQuotBuf;

	if( NULL == m_pDatabase || NULL == m_pQuotationBuffer )
	{
		DataNodeService::GetSerivceObj().WriteError( "ImageDataQuery::Instance() : invalid object pointer" );
		return -1;
	}

	if( NULL == (m_pSendBuffer = new char[MAX_IMAGE_BUFFER_SIZE]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "ImageDataQuery::Instance() : failed 2 initialize send data buffer, size = %d", MAX_IMAGE_BUFFER_SIZE );
		return -2;
	}

	return 0;
}

void ImageDataQuery::Release()
{
	if( NULL != m_pSendBuffer )
	{
		delete []m_pSendBuffer;
		m_pSendBuffer = NULL;
	}
}

DatabaseIO* ImageDataQuery::GetMemoDbPtr()
{
	return m_pDatabase;
}

ImageDataQuery& ImageDataQuery::GetImageQuery()
{
	static ImageDataQuery		obj;

	return obj;
}

unsigned int ImageDataQuery::FormatImageBuffer( unsigned int nSeqNo, unsigned int nDataID, unsigned int nDataWidth, unsigned int nBuffDataLen )
{
	tagPackageHead*		pPkgHead = (tagPackageHead*)m_pSendBuffer;

	if( 0 == nBuffDataLen )
	{
		return 0;
	}

	///< �������͸�ʽ���ݰ�
	::memmove( m_pSendBuffer+sizeof(tagPackageHead), m_pSendBuffer, nBuffDataLen );

	///< ���ݰ�ͷ����
	pPkgHead->nSeqNo = nSeqNo;
	pPkgHead->nMsgCount = nBuffDataLen / nDataWidth;
	pPkgHead->nMarketID = g_nMarketID;
	pPkgHead->nMsgLength = nDataWidth;

	return nBuffDataLen + sizeof(tagPackageHead);
}

int ImageDataQuery::FlushImageData2NewSessions( unsigned __int64 nSerialNo )
{
	int							nReqLinkID = 0;
	CriticalLock				lock( m_oLock );

	if( LinkNoRegister::GetRegister().GetReqLinkCount() > 0 )
	{
		unsigned int			lstTableID[64] = { 0 };
		unsigned int			lstTableWidth[64] = { 0 };
		unsigned int			nTableCount = m_pDatabase->GetTablesID( lstTableID, 64, lstTableWidth, 64 );
		unsigned int			nReqLinkCount = LinkNoRegister::GetRegister().GetReqLinkCount();

		while( (nReqLinkID = LinkNoRegister::GetRegister().PopReqLinkID()) >= 0 )
		{
			for( unsigned int n = 0; n < nTableCount && LinkNoRegister::GetRegister().GetReqLinkCount() > 0; n++ )
			{
				unsigned int		nTableID = lstTableID[n];
				unsigned int		nTableWidth = lstTableWidth[n];
				int					nFunctionID = ((n+1)==nTableCount) ? 100 : 0;	///< ���һ�����ݰ��ı�ʶ
				unsigned __int64	nSerialNoOfAnchor = nSerialNo;
				int					nDataLen = m_pDatabase->FetchRecordsByID( nTableID, m_pSendBuffer, MAX_IMAGE_BUFFER_SIZE, nSerialNoOfAnchor );

				if( nDataLen < 0 )
				{
					DataNodeService::GetSerivceObj().WriteWarning( "ImageDataQuery::FlushImageData2NewSessions() : failed 2 fetch image of table, errorcode=%d", nDataLen );
					return -1 * (n*100);
				}

				///< ����ѯ�����������¸�ʽ��m_pSendBuffer���ͻ���
				unsigned int	nSendLen = FormatImageBuffer( n, nTableID, nTableWidth, nDataLen );

				int	nErrCode = DataNodeService::GetSerivceObj().SendData( nReqLinkID, nTableID, nFunctionID, m_pSendBuffer, nSendLen/*, nSerialNo*/ );
				if( nErrCode < 0 )
				{
					DataNodeService::GetSerivceObj().CloseLink( nReqLinkID );
					DataNodeService::GetSerivceObj().WriteWarning( "ImageDataQuery::FlushImageData2NewSessions() : failed 2 send image data, errorcode=%d", nErrCode );
					return -2 * (n*10000);
				}

				if( 100 == nFunctionID )					///< ���һ�����ݰ���function id��100
				{
					LinkNoRegister::GetRegister().NewPushLinkID( nReqLinkID );	///< ���»Ự��id���������б�
				}
			}
		}

		m_pQuotationBuffer->SetLinkNoList();

		return nReqLinkCount;
	}

	return 0;
}

int ImageDataQuery::QueryCodeListInDatabase( unsigned int nDataID, unsigned int nRecordLen, std::set<std::string>& setCode )
{
	unsigned __int64	nSerialNoOfAnchor = 0;
	CriticalLock		lock( m_oLock );
	int					nDataLen = m_pDatabase->FetchRecordsByID( nDataID, m_pSendBuffer, MAX_IMAGE_BUFFER_SIZE, nSerialNoOfAnchor );

	setCode.clear();
	if( nDataLen < 0 )	{
		DataNodeService::GetSerivceObj().WriteWarning( "ImageDataQuery::QueryCodeListInDatabase() : failed 2 fetch image of table, errorcode=%d", nDataLen );
		return -1;
	}

	for( int nOffset = 0; nOffset < nDataLen; nOffset+=nRecordLen )
	{
		setCode.insert( std::string(m_pSendBuffer+nOffset) );
	}

	return setCode.size();
}


SessionCollection::SessionCollection( DatabaseIO& refDbIO )
 : m_refDatabase( refDbIO )
{
}

SessionCollection::~SessionCollection()
{
}

int SessionCollection::Instance()
{
	DataNodeService::GetSerivceObj().WriteInfo( "SessionCollection::Instance() : initializing ......" );

	Release();

	int		nErrCode = ImageDataQuery::GetImageQuery().Instance( &m_refDatabase, &m_oQuotationBuffer );
	if( 0 != nErrCode )
	{
		DataNodeService::GetSerivceObj().WriteError( "SessionCollection::Instance() : failed 2 initialize image query obj ..., errorcode=%d", nErrCode );
		return nErrCode;
	}

	nErrCode = m_oQuotationBuffer.Initialize();
	if( 0 == nErrCode )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "SessionCollection::Instance() : initialized ......" );
	}
	else
	{
		DataNodeService::GetSerivceObj().WriteError( "SessionCollection::Instance() : failed 2 initialize ..., errorcode=%d", nErrCode );
		return nErrCode;
	}

	return 0;
}

void SessionCollection::Release()
{
	m_oQuotationBuffer.Release();
}

void SessionCollection::PushQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize, bool bPushFlag, unsigned __int64 nSerialNo )
{
	m_oQuotationBuffer.PutMessage( usMessageNo, lpInBuf, uiInSize, nSerialNo );
}

void SessionCollection::OnReportStatus( char* szStatusInfo, unsigned int uiSize )
{
	char			pszStatusDesc[1024*2] = { 0 };
	unsigned int	nDescLen = sizeof(pszStatusDesc);
	unsigned int	nModuleVersion = Configuration::GetConfigObj().GetStartInParam().uiVersion;
	float			dFreePer = m_oQuotationBuffer.GetFreePercent();
	int				nUpdateInterval = (int)(::time(NULL)-m_refDatabase.GetLastUpdateTime());

	::sprintf( szStatusInfo
		, ":working = %s,[NodeServer],�汾 = V%.2f B%03d,��������ģʽ = %s,������������ = %u,������·�� = %d(·),\
		  ��ʼ����·�� = %u(·),���ݱ����� = %u(��), ������ = %u(��), ������б��� = %.2f(��)\
		  ,[QuotationPlugin],%s"
		, DataNodeService::GetSerivceObj().OnInquireStatus( pszStatusDesc, nDescLen )==true?"true":"false"
		, (float)(nModuleVersion>>16)/100.f, nModuleVersion&0xFF
		, Configuration::GetConfigObj().GetTestFlag()==true?"��":"��"
		, DataNodeService::GetSerivceObj().OnInquireHeartBeatCount()
		, LinkNoRegister::GetRegister().GetPushLinkCount(), LinkNoRegister::GetRegister().GetReqLinkCount()
		, m_refDatabase.GetTableCount(), nUpdateInterval, dFreePer
		, pszStatusDesc );
}

bool SessionCollection::OnNewLink( unsigned int uiLinkNo, unsigned int uiIpAddr, unsigned int uiPort )
{
	return true;
}

bool SessionCollection::OnCommand( const char* szSrvUnitName, const char* szCommand, char* szResult, unsigned int uiSize )
{
	int							nArgc = 32;
	char*						pArgv[32] = { 0 };
	unsigned int				nMarketID = g_nMarketID;
	static ModuleControl		objControl4Module;		///< ģ������࣬�������������ݵ�
	static CTP_DL_Echo			objEcho4CTPDL;			///< ��Ʒ�ڻ���Ȩ(����)

	///< �����ؼ��ֺͲ����ַ�
	if( false == SplitString( pArgv, nArgc, szCommand ) )
	{
		::sprintf( szResult, "RealTimeQuote4LinksSpi::OnCommand : [ERR] parse command string failed, [%s]", szCommand );
		return true;
	}

	///< ���ж��Ƿ�Ϊϵͳ�����������Ǿ�ִ�У������������ִ�У��ж��Ƿ�Ϊ��������
	if( true == objControl4Module( pArgv, nArgc, szResult, uiSize ) )
	{
		return true;
	}

	///< ���ݹ��ص����ݲɼ�����Ӧ���г�ID��ʹ�ö�Ӧ�����ݼ��
	switch( nMarketID )
	{
	case 14:
		return objEcho4CTPDL( pArgv, nArgc, szResult, uiSize );	///< ִ�л������
	default:
		::sprintf( szResult, "����ʶ������[%s]���г�ID[%u]", szCommand, nMarketID );
		break;
	}

	return true;
}

void SessionCollection::OnCloseLink( unsigned int uiLinkNo, int iCloseType )
{
	DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::OnCloseLink() : link [%u] closed, errorcode=%d", uiLinkNo, iCloseType );

	LinkNoRegister::GetRegister().RemovePushLinkID( uiLinkNo );
	m_oQuotationBuffer.SetLinkNoList();
}

bool SessionCollection::OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData )
{
	if( usMessageNo == MESSAGENO )
	{
		tagCommonLoginData_LF299*	pMsgBody = (tagCommonLoginData_LF299*)( lpData + sizeof(tagPackageHead) );

		if( 299 == usMessageNo )
		{
			::strcpy( pMsgBody->pszActionKey, "success" );

			int	nErrCode = DataNodeService::GetSerivceObj().SendData( uiLinkNo, usMessageNo, usFunctionID, lpData, uiSize );
			if( nErrCode < 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::OnRecvData() : failed 2 reply login request, errorcode=%d", nErrCode );
				return false;
			}

			///< ------------ ��У��ͨ�������󣬼������ʼ���б� ------------------
			if( true == LinkNoRegister::GetRegister().InReqLinkIDSet( uiLinkNo ) )
			{
				DataNodeService::GetSerivceObj().WriteInfo( "SessionCollection::OnRecvData() : [WARNING] duplicate link number & new link will be disconnected..." );
				return false;
			}

			LinkNoRegister::GetRegister().NewReqLinkID( uiLinkNo );
			DataNodeService::GetSerivceObj().WriteInfo( "SessionCollection::OnRecvData() : [NOTICE] link[%u] logged 2 server successfully." );

			return true;
		}
	}

	return false;
}












