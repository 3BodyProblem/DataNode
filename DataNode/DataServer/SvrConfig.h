#ifndef __DATA_NODE_SVRCONFIG_H__
#define	__DATA_NODE_SVRCONFIG_H__


#include <string>
#include <vector>
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "ServiceIO/MServicePlug.hpp"
#include "../InitializeFlag/InitFlag.h"


std::string GetModulePath( void* hModule );


/**
 * @class								Configuration
 * @brief								�ڵ��������������Ϣ������
 * @date								2017/5/4
 * @author								barry
 */
class Configuration
{
private:
	Configuration();

public:
	/**
	 * @brief							ȡ�����ö���ĵ�������
	 */
	static Configuration&				GetConfigObj();

	/**
	 * @brief							��ʼ����������
	 * @return							==0				�ɹ�
										!=				ʧ��
	 */
	int									Load();

public:
	/**
	 * @brief							�ڼ��ղ��Ա�ʶ
	 * @return							���ؽڼ����Ƿ���Ҫ���Եı�ʶ
	 */
	bool								GetTestFlag() const;

	/**
	 * @brief							ȡ�ø��г������н���ʱ����Ϣ
	 */
	const T_VECTOR_PERIODS&				GetTradingPeriods() const;

	/**
	 * @brief							����洢/�ָ��ļ���Ŀ¼
	 */
	const std::string&					GetRecoveryFolderPath() const;

	/**
	 * @brief							�ڼ��ļ�·��
	 */
	const std::string&					GetHolidayFilePath() const;

	/**
	 * @brief							�ڴ���ģ��·��
	 */
	const std::string&					GetMemPluginPath() const;

	/**
	 * @brief							ѹ��ģ����·��
	 */
	const std::string&					GetCompressPluginPath() const;

	/**
	 * @brief							���ݲɼ����·��
	 */
	const std::string&					GetDataCollectorPluginPath() const;

	/**
	 * @brief							��������������
	 */
	const tagServicePlug_StartInParam&	GetStartInParam() const;

protected:
	tagServicePlug_StartInParam			m_oStartInParam;				///< ����������������

protected:
	bool								m_bTestFlag;					///< ���Ա�ʶ
	std::string							m_sHolidayFilePath;				///< �ڼ����ļ�·��
	std::string							m_sMemPluginPath;				///< �����ڴ��������·��
	std::string							m_sCompressPluginPath;			///< ����ѹ���������·��
	std::string							m_sDataCollectorPluginPath;		///< ���ݲɼ��������·��
	std::string							m_sRecoveryFolder;				///< �������ݴ洢/�ָ�Ŀ¼
	T_VECTOR_PERIODS					m_vctTradingPeriods;			///< ����ʱ����Ϣ��
};


#endif








