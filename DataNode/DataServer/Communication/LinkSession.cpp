#pragma warning(disable:4996)
#include <time.h>
#include "LinkSession.h"
#include "../DataEcho.h"
#include "../NodeServer.h"
#include "../../../../DataCluster/DataCluster/Protocal/DataCluster_Protocal.h"


LinkNoRegister::LinkNoRegister()
 : m_nLinkCount( 0 )
{
	::memset( m_vctLinkNo, 0, sizeof(m_vctLinkNo) );
}

LinkNoRegister& LinkNoRegister::GetRegister()
{
	static LinkNoRegister		obj;

	return obj;
}

void LinkNoRegister::ClearAll()
{
	CriticalLock	guard( m_oLock );

	m_nLinkCount = 0;
	m_setPushLinkID.clear();
	::memset( m_vctLinkNo, 0, sizeof(m_vctLinkNo) );
	m_setNewReqLinkID.clear();
}

int LinkNoRegister::NewPushLinkID( unsigned int nNewLinkID )
{
	CriticalLock	guard( m_oLock );

	///< IDδ��ӣ��������
	if( m_setPushLinkID.find( nNewLinkID ) == m_setPushLinkID.end() )
	{
		m_setPushLinkID.insert( nNewLinkID );
		m_nLinkCount = m_setPushLinkID.size();

		int		n = 0;
		for( std::set<unsigned int>::iterator it = m_setPushLinkID.begin(); it != m_setPushLinkID.end() && n < MAX_LINKID_NUM; it++ )
		{
			m_vctLinkNo[n++] = *it;
		}

		return 1;
	}

	return 0;
}

void LinkNoRegister::RemovePushLinkID( unsigned int nRemoveLinkID )
{
	unsigned int	nLinkNum = 0;				///< ��Ч��·����
	CriticalLock	guard( m_oLock );

	///< �������ID�������Ƴ�
	if( m_setPushLinkID.find( nRemoveLinkID ) != m_setPushLinkID.end() )
	{
		m_setPushLinkID.erase( nRemoveLinkID );
		m_nLinkCount = m_setPushLinkID.size();

		int		n = 0;
		for( std::set<unsigned int>::iterator it = m_setPushLinkID.begin(); it != m_setPushLinkID.end() && n < MAX_LINKID_NUM; it++ )
		{
			m_vctLinkNo[n++] = *it;
		}
	}
}

unsigned int LinkNoRegister::FetchLinkNoTable( unsigned int* pIDTable, unsigned int nBuffSize )
{
	CriticalLock	guard( m_oLock );

	::memcpy( pIDTable, m_vctLinkNo+0, m_nLinkCount*sizeof(unsigned int) );

	return m_nLinkCount;
}

int LinkNoRegister::GetPushLinkCount()
{
	return m_nLinkCount;
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


SessionCollection::SessionCollection( PowerDB& refDbIO )
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

	int	nErrCode = m_oSendPackage.Initialize();
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
	m_oSendPackage.Release();
}

void SessionCollection::PushQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize, bool bPushFlag, unsigned __int64 nSerialNo )
{
	m_oSendPackage.DispatchMessage( usMessageNo, lpInBuf, uiInSize, nSerialNo, bPushFlag );
}

void SessionCollection::OnReportStatus( char* szStatusInfo, unsigned int uiSize )
{
	char			pszStatusDesc[1024*2] = { 0 };
	unsigned int	nDescLen = sizeof(pszStatusDesc);
	unsigned int	nModuleVersion = Configuration::GetConfigObj().GetStartInParam().uiVersion;

	::sprintf( szStatusInfo
		, ":working = %s,[DataNode.exe������Ϣ],ServicePlug�汾 = V%.2f B%03d,��������ģʽ = %s,������������ = %u,������·�� = %d(·),\
		  ���ݱ����� = %u(��), [���������Ϣ],%s"
		, DataNodeService::GetSerivceObj().OnInquireStatus( pszStatusDesc, nDescLen )==true?"true":"false"
		, (float)(nModuleVersion>>16)/100.f, nModuleVersion&0xFF
		, Configuration::GetConfigObj().GetTestFlag()==true?"��":"��"
		, DataNodeService::GetSerivceObj().OnInquireHeartBeatCount()
		, LinkNoRegister::GetRegister().GetPushLinkCount()
		, m_refDatabase.GetTableCount(), pszStatusDesc );
}

bool SessionCollection::OnNewLink( unsigned int uiLinkNo, unsigned int uiIpAddr, unsigned int uiPort )
{
	char			pszStatusDesc[1024*2] = { 0 };
	unsigned int	nDescLen = sizeof(pszStatusDesc);
	bool			bStatusIsOkey = DataNodeService::GetSerivceObj().OnInquireStatus( pszStatusDesc, nDescLen );

	if( false == bStatusIsOkey )
	{
		DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::OnNewLink() : TCP connection request has been denied! (linkno. = %u) Service isn\'t available... ", uiLinkNo );
	}

	return bStatusIsOkey;
}

void SessionCollection::OnCloseLink( unsigned int uiLinkNo, int iCloseType )
{
	char	pszDesc[128] = { 0 };

	switch( iCloseType )
	{///< �ر�����: 0 ����ͨѶ���� 1 WSARECV�������� 2 ����������ر� 3 �ͻ��������ر� 4 �������ݴ�����ر�
	case 0:
		::sprintf( pszDesc, "%s", "����ͨѶ����" );
		break;
	case 1:
		::sprintf( pszDesc, "%s", "WSARECV��������" );
		break;
	case 2:
		::sprintf( pszDesc, "%s", "����������ر�" );
		break;
	case 3:
		::sprintf( pszDesc, "%s", "�ͻ��������ر�" );
		break;
	case 4:
		::sprintf( pszDesc, "%s", "�������ݴ�����ر�" );
		break;
	default:
		::sprintf( pszDesc, "%s", "δ֪����" );
		break;
	}

	DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::OnCloseLink() : TCP disconnected! linkno(%u), errorcode=%d, desc=%s", uiLinkNo, iCloseType, pszDesc );

	LinkNoRegister::GetRegister().RemovePushLinkID( uiLinkNo );
}

bool SessionCollection::OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData )
{
	if( MSG_LOGIN_ID == usMessageNo )
	{
		tagPackageHead*				pHead = (tagPackageHead*)lpData;
		tagCommonLoginData_LF299*	pMsgBody = (tagCommonLoginData_LF299*)( lpData + sizeof(tagPackageHead) );

		::strcpy( pMsgBody->pszActionKey, "success" );

		int	nErrCode = DataNodeService::GetSerivceObj().SendData( uiLinkNo, usMessageNo, usFunctionID, lpData, sizeof(tagCommonLoginData_LF299) + sizeof(tagPackageHead) );
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

	return false;
}

bool SessionCollection::OnCommand( const char* szSrvUnitName, const char* szCommand, char* szResult, unsigned int uiSize )
{
	int							nArgc = 32;
	char*						pArgv[32] = { 0 };
	unsigned int				nMarketID = DataCollector::GetMarketID();

	///< �����ؼ��ֺͲ����ַ�
	if( false == SplitString( pArgv, nArgc, szCommand ) )
	{
		::sprintf( szResult, "RealTimeQuote4LinksSpi::OnCommand : [ERR] parse command string failed, [%s]", szCommand );
		return true;
	}

	///< ���ж��Ƿ�Ϊϵͳ�����������Ǿ�ִ�У������������ִ�У��ж��Ƿ�Ϊ��������
	if( true == ModuleControl::GetSingleton()( pArgv, nArgc, szResult, uiSize ) )
	{
		return true;
	}

	///< ���ݹ��ص����ݲɼ�����Ӧ���г�ID��ʹ�ö�Ӧ�����ݼ��
	switch( nMarketID )
	{
	case QUO_MARKET_DCE:
		return DLFuture_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );		///< ִ�л������
	case QUO_MARKET_DCEOPT:
		return DLOption_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );		///< ִ�л������
	case QUO_MARKET_SHFE:
		return SHFuture_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );		///< ִ�л������
	case QUO_MARKET_SHFEOPT:
		return  SHOption_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );	///< ִ�л������
	case QUO_MARKET_CZCE:
		return ZZFuture_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );		///< ִ�л������
	case QUO_MARKET_CZCEOPT:
		return ZZOption_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );		///< ִ�л������
	case QUO_MARKET_CFFEX:
		return CFFFuture_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );	///< ִ�л������
	case QUO_MARKET_SSE:
		return SHL1_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );			///< ִ�л������
	case QUO_MARKET_SSEOPT:
		return SHL1Option_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );	///< ִ�л������
	case QUO_MARKET_SZSE:
		return SZL1_Echo::GetSingleton()( pArgv, nArgc, szResult, uiSize );			///< ִ�л������
	default:
		::sprintf( szResult, "����ʶ������[%s]���г�ID[%u]", szCommand, nMarketID );
		break;
	}

	return true;
}












