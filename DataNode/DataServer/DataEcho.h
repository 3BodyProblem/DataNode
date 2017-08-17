#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "../Infrastructure/Lock.h"
#include "../../../DataCollector4CTPDL/DataCollector4CTPDL/CTP_DL_QuoProtocal.h"
#include "../../../DataCollector4CTPSH/DataCollector4CTPSH/CTP_SH_QuoProtocal.h"
#include "../../../DataCollector4CTPZZ/DataCollector4CTPZZ/CTP_ZZ_QuoProtocal.h"


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
 * @class						DLFuture_Echo
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
 * @class						DLFuture_Echo
 * @brief						������Ʒ�ڻ�CTP�������������
 * @date						2017/5/19
 * @author						barry
 */
class DLFuture_Echo : public IDataEcho
{
private:
	DLFuture_Echo();
public:
	static DLFuture_Echo&		GetSingleton();

public:///< ��ʽ������
	/**
	 * @brief					������ϢID������ṹ����ת���ɸ�ʽ���ַ������
	 * @param[out]				pszEchoBuffer			�����ʽ���ַ�������
	 * @param[in]				nMsgID					��ϢID
	 * @param[in]				pszInputBuffer			����ṹ��ַ
	 * @return					>0						ת���ɹ�,����ת��������������Ч���ݵĳ���
								==0						ת��ʧ��
	 */
	static int					FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer );

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


/**
 * @class						SHFuture_Echo
 * @brief						�Ϻ���Ʒ�ڻ�CTP�������������
 * @date						2017/5/19
 * @author						barry
 */
class SHFuture_Echo : public IDataEcho
{
private:
	SHFuture_Echo();
public:
	static SHFuture_Echo&		GetSingleton();

public:///< ��ʽ������
	/**
	 * @brief					������ϢID������ṹ����ת���ɸ�ʽ���ַ������
	 * @param[out]				pszEchoBuffer			�����ʽ���ַ�������
	 * @param[in]				nMsgID					��ϢID
	 * @param[in]				pszInputBuffer			����ṹ��ַ
	 * @return					>0						ת���ɹ�,����ת��������������Ч���ݵĳ���
								==0						ת��ʧ��
	 */
	static int					FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer );

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


/**
 * @class						ZZFuture_Echo
 * @brief						֣����Ʒ�ڻ�CTP�������������
 * @date						2017/5/19
 * @author						barry
 */
class ZZFuture_Echo : public IDataEcho
{
private:
	ZZFuture_Echo();
public:
	static ZZFuture_Echo&		GetSingleton();

public:///< ��ʽ������
	/**
	 * @brief					������ϢID������ṹ����ת���ɸ�ʽ���ַ������
	 * @param[out]				pszEchoBuffer			�����ʽ���ַ�������
	 * @param[in]				nMsgID					��ϢID
	 * @param[in]				pszInputBuffer			����ṹ��ַ
	 * @return					>0						ת���ɹ�,����ת��������������Ч���ݵĳ���
								==0						ת��ʧ��
	 */
	static int					FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer );

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






