#include "LinkSession.h"
#include "../NodeServer.h"


LinkIDSet::LinkIDSet()
{}

LinkIDSet& LinkIDSet::GetSetObject()
{
	static	LinkIDSet	obj;
	return obj;
}

int LinkIDSet::NewLinkID( unsigned int nNewLinkID )
{
	CriticalLock	guard( m_oLock );

	///< IDδ��ӣ��������
	if( m_setLinkID.find( nNewLinkID ) == m_setLinkID.end() )
	{
		m_setLinkID.insert( nNewLinkID );
		return 1;
	}

	return 0;
}

void LinkIDSet::RemoveLinkID( unsigned int nRemoveLinkID )
{
	CriticalLock	guard( m_oLock );

	///< �������ID�������Ƴ�
	if( m_setLinkID.find( nRemoveLinkID ) != m_setLinkID.end() )
	{
		m_setLinkID.erase( nRemoveLinkID );
	}
}

unsigned int LinkIDSet::FetchLinkIDList( unsigned int * lpLinkNoArray, unsigned int uiArraySize )
{
	unsigned int	nLinkNum = 0;				///< ��Ч��·����
	static	int		s_nLastLinkNoNum = 0;		///< ��һ�ε���·����
	CriticalLock	guard( m_oLock );			///< ��

	if( m_setLinkID.size() != s_nLastLinkNoNum )
	{
		SvrFramework::GetFramework().WriteInfo( "LinkIDSet::FetchLinkIDList() : TCP connection number of QServer fluctuated! new no. = %d, old no. = %d", m_setLinkID.size(), s_nLastLinkNoNum );
		s_nLastLinkNoNum = m_setLinkID.size();
	}

	for( std::set<unsigned int>::iterator it = m_setLinkID.begin(); it != m_setLinkID.end() && nLinkNum < uiArraySize; it++ )
	{
		lpLinkNoArray[nLinkNum++] = *it;
	}

	return nLinkNum;
}


LinkSessionSet::LinkSessionSet()
 : m_pDatabase( NULL )
{
}

LinkSessionSet& LinkSessionSet::GetSessionSet()
{
	static	LinkSessionSet	obj;

	return obj;
}

int LinkSessionSet::Instance()
{
	SvrFramework::GetFramework().WriteInfo( "LinkSessionSet::Instance() : initializing ......" );

	int		nErrCode = m_oQuotationBuffer.Initialize();

	if( 0 == nErrCode )
	{
		SvrFramework::GetFramework().WriteInfo( "LinkSessionSet::Instance() : initialized ......" );
	}
	else
	{
		SvrFramework::GetFramework().WriteError( "LinkSessionSet::Instance() : failed 2 initialize ..." );
	}

	return nErrCode;
}

int LinkSessionSet::SendData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize )
{

	return 0;
}

int LinkSessionSet::SendError( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, const char* lpErrorInfo )
{

	return 0;
}

void LinkSessionSet::PushData( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize, bool bPushFlag )
{
	m_oQuotationBuffer.PutMessage( usMessageNo, lpInBuf, uiInSize );
}

int LinkSessionSet::CloseLink( unsigned int uiLinkNo )
{
	LinkIDSet::GetSetObject().RemoveLinkID( uiLinkNo );

	return 0;
}

void LinkSessionSet::OnReportStatus( char* szStatusInfo, unsigned int uiSize )
{
//	cs_format(szStatusInfo,uiSize, _T( ":[API/SPI ����],(2)API/SPI ������Ŀ=%d,(2)API/SPI ��֤��Ŀ=%d,[API ���ü���],�µ�Ƶ��=%.2f,����Ƶ��=%.2f,��ѯƵ��=%.2f"), spi_total, spi_auth, freq_insertOrder, freq_cancelOrder, freq_queryOrder);
}

bool LinkSessionSet::OnCommand( const char* szSrvUnitName, const char* szCommand, char* szResult, unsigned int uiSize )
{

	return false;
}

bool LinkSessionSet::OnNewLink( unsigned int uiLinkNo, unsigned int uiIpAddr, unsigned int uiPort )
{
	if( 1 == LinkIDSet::GetSetObject().NewLinkID( uiLinkNo ) )
	{
		///< to do : send snap table ......
		DatabaseIO&			refDatabaseIO = DataNodeService::GetSerivceObj().GetDatabaseIO();



		return true;
	}

	SvrFramework::GetFramework().WriteInfo( "LinkSessionSet::OnNewLink() : [WARNING] duplicate link number & new link will be disconnected..." );

	return false;
}

void LinkSessionSet::OnCloseLink( unsigned int uiLinkNo, int iCloseType )
{
	LinkIDSet::GetSetObject().RemoveLinkID( uiLinkNo );
}

bool LinkSessionSet::OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData )
{

	return false;
}





