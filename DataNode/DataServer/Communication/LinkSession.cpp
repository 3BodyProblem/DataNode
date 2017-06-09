#pragma warning(disable:4996)
#include <time.h>
#include "LinkSession.h"
#include "../DataEcho.h"
#include "../NodeServer.h"


LinkNoRegister::LinkNoRegister()
 : nLinkIDCount( 0 )
{}

int LinkNoRegister::NewLinkID( unsigned int nNewLinkID )
{
	CriticalLock	guard( m_oLock );

	///< IDδ��ӣ��������
	if( m_setLinkID.find( nNewLinkID ) == m_setLinkID.end() )
	{
		m_setLinkID.insert( nNewLinkID );
		nLinkIDCount = m_setLinkID.size();
		return 1;
	}

	return 0;
}

void LinkNoRegister::RemoveLinkID( unsigned int nRemoveLinkID )
{
	CriticalLock	guard( m_oLock );

	///< �������ID�������Ƴ�
	if( m_setLinkID.find( nRemoveLinkID ) != m_setLinkID.end() )
	{
		m_setLinkID.erase( nRemoveLinkID );
		nLinkIDCount = m_setLinkID.size();
	}
}

int LinkNoRegister::GetLinkCount()
{
	return nLinkIDCount;
}

unsigned int LinkNoRegister::FetchLinkIDList( unsigned int * lpLinkNoArray, unsigned int uiArraySize )
{
	unsigned int	nLinkNum = 0;				///< ��Ч��·����
	static	int		s_nLastLinkNoNum = 0;		///< ��һ�ε���·����
	CriticalLock	guard( m_oLock );			///< ��

	if( m_setLinkID.size() != s_nLastLinkNoNum )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "LinkNoRegister::FetchLinkIDList() : TCP connection number of QServer fluctuated! new no. = %d, old no. = %d", m_setLinkID.size(), s_nLastLinkNoNum );
		s_nLastLinkNoNum = m_setLinkID.size();
	}

	for( std::set<unsigned int>::iterator it = m_setLinkID.begin(); it != m_setLinkID.end() && nLinkNum < uiArraySize; it++ )
	{
		lpLinkNoArray[nLinkNum++] = *it;
	}

	return nLinkNum;
}


RealTimeQuote4LinksSpi::RealTimeQuote4LinksSpi()
{
}

RealTimeQuote4LinksSpi::~RealTimeQuote4LinksSpi()
{
	Release();
}

int RealTimeQuote4LinksSpi::Instance()
{
	DataNodeService::GetSerivceObj().WriteInfo( "RealTimeQuote4LinksSpi::Instance() : initializing ......" );

	Release();

	int		nErrCode = m_oQuotationBuffer.Initialize();
	if( 0 == nErrCode )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "RealTimeQuote4LinksSpi::Instance() : initialized ......" );
	}
	else
	{
		DataNodeService::GetSerivceObj().WriteError( "RealTimeQuote4LinksSpi::Instance() : failed 2 initialize ..., errorcode=%d", nErrCode );
		return nErrCode;
	}

	return 0;
}

void RealTimeQuote4LinksSpi::Release()
{
	m_oQuotationBuffer.Release();
}

void RealTimeQuote4LinksSpi::PushQuotation( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize, bool bPushFlag, unsigned __int64 nSerialNo )
{
	m_oQuotationBuffer.PutMessage( usMessageNo, lpInBuf, uiInSize, nSerialNo );
}


SessionCollection::SessionCollection( DataCollector& oDataCollector )
 : m_pSendBuffer( NULL ), m_pDatabase( NULL ), m_refDataCollector( oDataCollector )
{
}

SessionCollection::~SessionCollection()
{
}

int SessionCollection::Instance( DatabaseIO& refDbIO )
{
	DataNodeService::GetSerivceObj().WriteInfo( "SessionCollection::Instance() : initializing ......" );

	Release();
	m_pDatabase = &refDbIO;

	if( NULL == (m_pSendBuffer = new char[MAX_IMAGE_BUFFER_SIZE]) )
	{
		DataNodeService::GetSerivceObj().WriteError( "SessionCollection::Instance() : failed 2 initialize send data buffer, size = %d", MAX_IMAGE_BUFFER_SIZE );
		return -1;
	}

	return RealTimeQuote4LinksSpi::Instance();
}

void SessionCollection::Release()
{
	if( NULL != m_pSendBuffer )
	{
		delete []m_pSendBuffer;
		m_pSendBuffer = NULL;
	}

	RealTimeQuote4LinksSpi::Release();
}

void SessionCollection::SetMkID()
{
	m_oQuotationBuffer.SetMkID( m_refDataCollector.GetMarketID() );
}

unsigned int SessionCollection::FormatImageBuffer( unsigned int nSeqNo, unsigned int nDataID, unsigned int nDataWidth, unsigned int nBuffDataLen )
{
	unsigned int		nOffset = 0;
	unsigned int		nMsgCount = 0;
	tagPackageHead*		pPkgHead = (tagPackageHead*)m_pSendBuffer;

	///< �������͸�ʽ���ݰ�
	::memmove( m_pSendBuffer+sizeof(tagPackageHead)+sizeof(tagBlockHead), m_pSendBuffer, nBuffDataLen );
	while( nBuffDataLen > 0 )
	{
		tagBlockHead*	pMsgHead = (tagBlockHead*)(m_pSendBuffer + sizeof(tagPackageHead) + nOffset);

		pMsgHead->nDataType = nDataID;
		pMsgHead->nDataLen = nDataWidth;

		nBuffDataLen -= pMsgHead->nDataLen;
		nOffset += (pMsgHead->nDataLen + sizeof(tagBlockHead));
		::memmove( m_pSendBuffer+sizeof(tagPackageHead)+nOffset, m_pSendBuffer+sizeof(tagPackageHead)+nOffset+sizeof(tagBlockHead), nBuffDataLen );
		nMsgCount++;
	}

	///< ���ݰ�ͷ����
	pPkgHead->nSeqNo = nSeqNo;
	pPkgHead->nMarketID = m_refDataCollector.GetMarketID();
	pPkgHead->nBodyLen = sizeof(tagPackageHead) + nOffset;
	pPkgHead->nMsgCount = nMsgCount;

	return nOffset;
}

int SessionCollection::FlushImageData2NewSessions( unsigned __int64 nSerialNo )
{
	CriticalLock		lock( m_oLock );

	if( 0 == m_nReqLinkCount ) {
		return 0;
	}

	unsigned int		lstTableID[64] = { 0 };
	unsigned int		lstTableWidth[64] = { 0 };
	unsigned int		nTableCount = m_pDatabase->GetTablesID( lstTableID, 64, lstTableWidth, 64 );
	unsigned int		nReqLinkCount = m_setNewReqLinkID.size();

	for( unsigned int n = 0; n < nTableCount && m_setNewReqLinkID.size() > 0; n++ )
	{
		unsigned int		nTableID = lstTableID[n];
		unsigned int		nTableWidth = lstTableWidth[n];
		unsigned __int64	nSerialNoOfAnchor = nSerialNo;
		int					nDataLen = m_pDatabase->FetchRecordsByID( nTableID, m_pSendBuffer, MAX_IMAGE_BUFFER_SIZE, nSerialNoOfAnchor );

		if( nDataLen < 0 )
		{
			DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::FlushImageData2NewSessions() : failed 2 fetch image of table, errorcode=%d", nDataLen );
			return -1 * (n*100);
		}

		unsigned int	nSendLen = FormatImageBuffer( n, nTableID, nTableWidth, nDataLen );

		for( std::set<unsigned int>::iterator it = m_setNewReqLinkID.begin(); it != m_setNewReqLinkID.end(); it++ )
		{
			int	nErrCode = DataNodeService::GetSerivceObj().SendData( *it, MESSAGENO, 0, m_pSendBuffer, nSendLen/*, nSerialNo*/ );
			if( nErrCode < 0 )
			{
				DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::FlushImageData2NewSessions() : failed 2 send image data, errorcode=%d", nErrCode );
				return -2 * (n*10000);
			}
		}
	}

	for( std::set<unsigned int>::iterator it = m_setNewReqLinkID.begin(); it != m_setNewReqLinkID.end(); it++ )
	{
		///< �·���ʼ�����֪ͨ
		tagBlockHead	tagMsg = { 0 };
		int				nErrorCode = DataNodeService::GetSerivceObj().SendData( *it, MESSAGENO, 1, (char*)&tagMsg, sizeof(tagMsg) );
		if( nErrorCode < 0 )	{
			DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::FlushImageData2NewSessions() : failed 2 send image data, errorcode=%d", nErrorCode );
			return -200;
		}

		m_oLinkNoTable.NewLinkID( *it );
	}

	m_setNewReqLinkID.clear();
	m_nReqLinkCount = 0;

	LINKID_VECTOR	vctLinkNo = { 0 };
	unsigned int	nLinkNoCount = m_oLinkNoTable.FetchLinkIDList( vctLinkNo+0, MAX_LINKID_NUM );
	if( nLinkNoCount > 0 ) {
		m_oQuotationBuffer.SetLinkNoList( vctLinkNo+0, nLinkNoCount );
	}

	return nReqLinkCount;
}

int SessionCollection::QueryCodeListInDatabase( unsigned int nDataID, unsigned int nRecordLen, std::set<std::string>& setCode )
{
	unsigned __int64	nSerialNoOfAnchor = 0;
	CriticalLock		lock( m_oLock );
	int					nDataLen = m_pDatabase->FetchRecordsByID( nDataID, m_pSendBuffer, MAX_IMAGE_BUFFER_SIZE, nSerialNoOfAnchor );

	setCode.clear();
	if( nDataLen < 0 )	{
		DataNodeService::GetSerivceObj().WriteWarning( "SessionCollection::QueryCodeListInDatabase() : failed 2 fetch image of table, errorcode=%d", nDataLen );
		return -1;
	}

	for( int nOffset = 0; nOffset < nDataLen; nOffset+=nRecordLen )
	{
		setCode.insert( std::string(m_pSendBuffer+nOffset) );
	}

	return setCode.size();
}

void SessionCollection::OnReportStatus( char* szStatusInfo, unsigned int uiSize )
{
	if( NULL == m_pDatabase ) {
		return;
	}

	unsigned int	nModuleVersion = Configuration::GetConfigObj().GetStartInParam().uiVersion;
	float			dFreePer = m_oQuotationBuffer.GetFreePercent();
	int				nUpdateInterval = (int)(::time(NULL)-m_pDatabase->GetLastUpdateTime());

	::sprintf( szStatusInfo
		, ":working = %s, �汾 = V%.2f B%03d, ��������ģʽ = %s, ������·�� = %d(·), \
		 ��ʼ����·�� = %u(·), ���ݱ����� = %u(��), ������ = %u(��), ������б��� = %.2f(��)\n"
		, DataNodeService::GetSerivceObj().OnInquireStatus()==true?"true":"false"
		, (float)(nModuleVersion>>16)/100.f, nModuleVersion&0xFF
		, Configuration::GetConfigObj().GetTestFlag()==true?"��":"��"
		, m_oLinkNoTable.GetLinkCount(), m_nReqLinkCount
		, m_pDatabase->GetTableCount(), nUpdateInterval, dFreePer );
}

bool SessionCollection::OnNewLink( unsigned int uiLinkNo, unsigned int uiIpAddr, unsigned int uiPort )
{
	CriticalLock		lock( m_oLock );

	if( m_setNewReqLinkID.find( uiLinkNo ) != m_setNewReqLinkID.end() )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "SessionCollection::AddNewReqSession() : [WARNING] duplicate link number & new link will be disconnected..." );

		return false;
	}

	m_setNewReqLinkID.insert( uiLinkNo );
	m_nReqLinkCount = m_setNewReqLinkID.size();

	return true;
}

bool SessionCollection::OnCommand( const char* szSrvUnitName, const char* szCommand, char* szResult, unsigned int uiSize )
{
	int							nArgc = 32;
	char*						pArgv[32] = { 0 };
	unsigned int				nMarketID = m_refDataCollector.GetMarketID();
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
	m_oLinkNoTable.RemoveLinkID( uiLinkNo );

	CriticalLock		lock( m_oLock );
	LINKID_VECTOR		vctLinkNo = { 0 };
	unsigned int		nLinkNoCount = m_oLinkNoTable.FetchLinkIDList( vctLinkNo+0, MAX_LINKID_NUM );

	if( nLinkNoCount > 0 )
	{
		m_oQuotationBuffer.SetLinkNoList( vctLinkNo+0, nLinkNoCount );
	}
}

bool SessionCollection::OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData )
{

	return false;
}












