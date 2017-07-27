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



#endif






