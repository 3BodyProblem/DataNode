#include "LinkSession.h"
#include "../NodeServer.h"


LinkIDHolder::LinkIDHolder()
{}

LinkIDHolder& LinkIDHolder::GetLinkHolder()
{
	static	LinkIDHolder	obj;
	return obj;
}

void LinkIDHolder::NewLinkID( unsigned int nNewLinkID )
{
	CriticalLock	guard( m_oLock );

	///< IDδ��ӣ��������
	if( m_setLinkID.find( nNewLinkID ) == m_setLinkID.end() )
	{
		m_setLinkID.insert( nNewLinkID );
	}
}

void LinkIDHolder::RemoveLinkID( unsigned int nRemoveLinkID )
{
	CriticalLock	guard( m_oLock );

	///< �������ID�������Ƴ�
	if( m_setLinkID.find( nRemoveLinkID ) != m_setLinkID.end() )
	{
		m_setLinkID.erase( nRemoveLinkID );
	}
}

int LinkIDHolder::FetchLinkIDList( unsigned int * lpLinkNoArray, unsigned int uiArraySize )
{
	unsigned int	nLinkNum = 0;				///< ��Ч��·����
	static	int		s_nLastLinkNoNum = 0;		///< ��һ�ε���·����
	CriticalLock	guard( m_oLock );			///< ��

	if( m_setLinkID.size() != s_nLastLinkNoNum )
	{
		SvrFramework::GetFramework().WriteInfo( "LinkIDHolder::FetchLinkIDList() : TCP connection number of QServer fluctuated! new no. = %d, old no. = %d", m_setLinkID.size(), s_nLastLinkNoNum );
		s_nLastLinkNoNum = m_setLinkID.size();
	}

	for( std::set<unsigned int>::iterator it = m_setLinkID.begin(); it != m_setLinkID.end() && nLinkNum < uiArraySize; it++ )
	{
		lpLinkNoArray[nLinkNum++] = *it;
	}

	return nLinkNum;
}


LinkSessionSet::LinkSessionSet()
{
}

LinkSessionSet& LinkSessionSet::GetSessionSet()
{
	static	LinkSessionSet	obj;

	return obj;
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

	return false;
}

void LinkSessionSet::OnCloseLink( unsigned int uiLinkNo, int iCloseType )
{

}

bool LinkSessionSet::OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData )
{

	return false;
}





