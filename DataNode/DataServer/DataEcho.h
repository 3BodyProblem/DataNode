#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "X4BaseLib/MInterface.hpp"
#include "../Server/X4BaseLib/MMutex.h"


std::string&	Str2Lower( std::string& sStr );
bool			SplitString( char** pArgv, int& nArgc, const char* pszStr );


/**
 * @file						SvrMonitor.h
 * @class						ISvrMonitor
 * @brief						�����ؽӿ�
 * @author						barry
 */
class ISvrMonitor
{
public:
	ISvrMonitor( std::string sMarketCode );

	/**
	 * @brief					ȡ���г�����
	 */
	const std::string&			GetMarketCode() const;

public:
	/**
	 * @brief					�������ӿ�
	 * @param[in]				szCommand				���
	 * @param[out]				lpOutString				�����Ϣ��ַ
	 * @param[in]				uiOutSize				������泤��
	 */
	virtual void				Command( const char * szCommand, char * lpOutString, unsigned int uiOutSize ) = 0;

	/**
	 * @brief					���첽����صĻ�����Ϣ���õ�����
		 * @param[in]			nSerial			�����������
		 * @param[in]			pszEcho			Ҫ���õ���Ϣ
	 */
	virtual void				SetAsyncEchoOfCmd( unsigned char nSerial, const char* pszEcho ){};

protected:
	std::string					m_sMarketCode;			///< �г�����
};


/**
 * @class						MonitorRegister
 * @brief						�����ע�������
 * @author						barry
 */
class MonitorRegister
{
private:
	MonitorRegister();
public:
	/**
	 * @brief					ȡ�ñ���ĵ���
	 */
	static MonitorRegister&		GetRegisterObj();

	/**
	 * @brief					����Ӧ���г���ע���ض���
	 * @param[in]				sMarketCode				�г����
	 * @param[in]				IMonitor				��ض����ַ
	 */
	void						Register( std::string sMarketCode, ISvrMonitor* IMonitor );

	/**
	 * @brief					���� �г���� ��ѯ��ض���
	 * @param[in]				MarketCode				�г����
	 * @param[out]				lpOutString				�����ѯʧ�ܣ���Ҫ���Ե���Ϣ�����ַ
	 * @return					��ض����ַ
	 */
	ISvrMonitor*				GetMonitorByMarketCode( std::string &sMarketCode, char * lpOutString, unsigned int uiOutSize );

	/**
	 * @brief					���� ���Ա�� �ҵ���Ӧ �г���� & ��ѯ��ض���
	 * @param[in]				nEchoSerial				���Ա��
	 * @return					��ض����ַ
	 */
	ISvrMonitor*				GetMonitorByEchoSerial( unsigned char nEchoSerial );

	/**
	 * @brief					����ӳ���
	 * @param[in]				nEchoSerial				���Ա��
	 * @param[in]				sMarketCode				�г����
	 */
	void						SetEchoSerialOfMarketCode( unsigned char nEchoSerial, const std::string& sMarketCode );

protected:
	std::map<std::string,ISvrMonitor*>			m_mapMonitor;				///< �г������ض���ӳ���
	std::map<unsigned char,std::string>			m_mapSerial2MarketCode;		///< ������ŵ��г���ŵ�ӳ���
	MCriticalSection							m_oCS;						///< ��
};


/**
 * @class						QuotationEcho
 * @brief						������Ϣ����
 * @detail						�����м�ع��ߣ�ֻ֧��Ǭ¡4x����������ϵ
 */
class Tran2ndMonitor : public ISvrMonitor
{
///< -------------------- �ڲ��� ------------------------------------------------------------------------
	/**
	 * @class					AsyncCmdEchoTable
	 * @brief					�쳣�����������ر�
	 * @detail					��һЩ��Ҫ�첽��ʱ���ص���Ϣ�����棬��֧�ֲ�ѯ/����
	 */
	class AsyncCmdEchoTable
	{
		typedef struct
		{
			time_t			nCmdGenTimeT;		///< ������������ʱ��
			std::string		sEchoString;		///< ������Ϣ��������
		} T_ECHO_INFO;
	public:
		AsyncCmdEchoTable();

		/**
		 * @brief				�����첽��ţ�ȡ�û�����Ϣ����
		 * @param[in]			nSerial			�����������
		 * @param[in]			nTryTimes		��ȡ�첽���Գ��Դ���
		 * @return				�������ݴ�
		 */
		std::string				GetEchoBySerialID( unsigned char nSerial, unsigned int nTryTimes = 1024 );

		/**
		 * @brief				���ݻ�����Ÿ��»�����Ϣ
		 * @detail				���ݻ�����Ÿ��»�����Ϣ���������nTimeTΪ�㣬�򲻸���T_ECHO_INFO::nCmdGenTimeT
		 * @param[in]			nSerial			�����������
		 * @param[in]			pszEcho			Ҫ���õ���Ϣ
		 * @param[in]			nTimeT			����T_ECHO_INFO::nCmdGenTimeTʱ�䣬���Ϊ�㣬�򲻸���
		 */
		void					UpdateEchoBySerialID( unsigned char nSerial, const char* pszEcho, time_t nTimeT = 0 );

	protected:
		MCriticalSection		m_oCS;					///< ��
		std::string				m_sOverTimeEcho;		///< �첽�ȴ���ʱ����
		static std::map<unsigned char,T_ECHO_INFO>		m_mapEchoInfo;
	};

	/**
	 * @class					AtomicUInteger
	 * @brief					ԭ�ӳ���
	 * @detail					�̰߳�ȫ,��װ��������
	 * @note					�������ֵΪ255,�������0�ָ�
	 */
	class AtomicUInteger
	{
	public:
		AtomicUInteger();

		/**
		 * @brief				ȡ�õ�ǰ����ֵ
		 * @return				����unsigned shortֵ
		 */
		operator				short();

		/**
		 * @brief				����ֵ
		 * @return				�����������ֵ
		 */
		short					AddRef();

	protected:
		MCriticalSection		m_oCS;					///< ��
		unsigned char			m_nUInteger;			///< ����
	};
///< ----------------------------------------------------------------------------------------------------
public:
	/**
	 * @brief					���캯��
	 * @param[in]				sMarketCode				�г����
	 * @param[in]				pControl				�����ع�������ַ(�Ӵ���dll�е���)
	 */
	Tran2ndMonitor( std::string sMarketCode );

	/**
	 * @brief					���ÿ��ƶ���ָ��
	 * @param[in]				pControl				�����ع�������ַ(�Ӵ���dll�е���)
	 */
	void						SetControlBaseClassPtr( MControlBaseClass* pControl );

	/**
	 * @brief					�������ӿ�
	 * @param[in]				szCommand				���
	 * @param[out]				lpOutString				�����Ϣ��ַ
	 * @param[in]				uiOutSize				������泤��
	 */
	void						Command( const char * szCommand, char * lpOutString, unsigned int uiOutSize );

	/**
	 * @brief					���첽����صĻ�����Ϣ���õ�����
		 * @param[in]			nEchoSerial				�����������
		 * @param[in]			pszEcho					Ҫ���õ���Ϣ
	 */
	void						SetAsyncEchoOfCmd( unsigned char nEchoSerial, const char* pszEcho );

protected:
	AsyncCmdEchoTable			m_AsyncEchoTb;			///< �첽���Ա�
	static AtomicUInteger		s_AtomicValue;			///< �첽���Ա�ʶ��
	MControlBaseClass*			m_pControl;				///< ����ģ���ض����ַ
};



#endif






