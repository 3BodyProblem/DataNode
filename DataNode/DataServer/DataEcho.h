#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "../Infrastructure/Lock.h"
#include "../../../DataCollector4CTPDL/DataCollector4CTPDL/CTP_DL_QuoProtocal.h"


std::string&			Str2Lower( std::string& sStr );
bool					SplitString( char** pArgv, int& nArgc, const char* pszStr );


/**
 * @class						IDataEcho
 * @brief						����������Խӿ�
 * @date						2017/5/19
 * @author						barry
 */ 
class IDataEcho
{
public:
	/**
	 * @brief					����
	 * @param[in]				sMarketName				�г�����
	 */
	IDataEcho( std::string sMarketName );

	/**
	 * @brief					��ȡ�г�����
	 */
	const std::string&			GetMarketName();

public:
	/**
	 * @brief					ִ�н���������л��Բ���
	 * @param[in]				pArgv					�����������ַ
	 * @param[in]				nArgc					���������鳤��
	 * @param[out]				szResult				���Ի����ַ
	 * @param[in]				uiSize					���Ի��泤��
	 * @return					true					ִ�гɹ�
	 */
	bool						operator()( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize );

protected:
	/**
	 * @brief					ִ�н���������л��Բ���
	 * @param[in]				pArgv					�����������ַ
	 * @param[in]				nArgc					���������鳤��
	 * @param[out]				szResult				���Ի����ַ
	 * @param[in]				uiSize					���Ի��泤��
	 * @return					true					ִ�гɹ�
	 */
	virtual bool				ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize ) = 0;

protected:
	std::string					m_sMarketName;			///< �г�����
	static const unsigned int	s_nMaxEchoBufLen;		///< ���Ի��泤��
	static char*				s_pEchoDataBuf;			///< ���Ի����ַ
};


/**
 * @class						CTP_DL_Echo
 * @brief						��������ִ����
 * @date						2017/5/19
 * @author						barry
 */
class ModuleControl : public IDataEcho
{
private:
	ModuleControl();

public:
	static ModuleControl&		GetSingleton();

protected:
	/**
	 * @brief					ִ�н���������л��Բ���
	 * @param[in]				pArgv					�����������ַ
	 * @param[in]				nArgc					���������鳤��
	 * @param[out]				szResult				���Ի����ַ
	 * @param[in]				uiSize					���Ի��泤��
	 * @return					true					ִ�гɹ�
	 */
	virtual bool				ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize );
};


/**
 * @class						CTP_DL_Echo
 * @brief						����CTP�������������
 * @date						2017/5/19
 * @author						barry
 */
class CTP_DL_Echo : public IDataEcho
{
private:
	CTP_DL_Echo();
public:
	static CTP_DL_Echo&			GetSingleton();

public:///< ��ʽ������
	static unsigned int			FormatMarketInfoLF100( char* pszEchoBuffer, tagDLFutureMarketInfo_LF100& refMarketInfo );
	static unsigned int			FormatMarketStatusHF102( char* pszEchoBuffer, tagDLFutureMarketStatus_HF102& refMarketStatus );
	static unsigned int			FormatReferenceDataLF103( char* pszEchoBuffer, tagDLFutureReferenceData_LF103& refRefData );
	static unsigned int			FormatSnapDataLF104( char* pszEchoBuffer, tagDLFutureSnapData_LF104& refSnapDataLF );
	static unsigned int			FormatSnapDataHF105( char* pszEchoBuffer, tagDLFutureSnapData_HF105& refSnapDataHF );
	static unsigned int			FormatBuySellDataHF106( char* pszEchoBuffer, tagDLFutureSnapBuySell_HF106& refBuySellDataHF );

protected:///< ������ýӿ�
	/**
	 * @brief					ִ�н���������л��Բ���
	 * @param[in]				pArgv					�����������ַ
	 * @param[in]				nArgc					���������鳤��
	 * @param[out]				szResult				���Ի����ַ
	 * @param[in]				uiSize					���Ի��泤��
	 * @return					true					ִ�гɹ�
	 */
	virtual bool				ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize );
};



#endif






