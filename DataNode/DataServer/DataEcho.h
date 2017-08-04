#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "../Infrastructure/Lock.h"
#include "../Protocal/CTP_DL_QuoProtocal.h"


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
	static unsigned int			FormatMarketInfoLF1000( char* pszEchoBuffer, tagDLMarketInfo_LF1000& refMarketInfo );
	static unsigned int			FormatMarketStatusHF1007( char* pszEchoBuffer, tagDLMarketStatus_HF1007& refMarketStatus );
	static unsigned int			FormatReferenceDataLF1003( char* pszEchoBuffer, tagDLReferenceData_LF1003& refRefData );
	static unsigned int			FormatSnapDataLF1004( char* pszEchoBuffer, tagDLSnapData_LF1004& refSnapDataLF );
	static unsigned int			FormatSnapDataHF1005( char* pszEchoBuffer, tagDLSnapData_HF1005& refSnapDataHF );
	static unsigned int			FormatBuySellDataHF1006( char* pszEchoBuffer, tagDLSnapBuySell_HF1006& refBuySellDataHF );

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






