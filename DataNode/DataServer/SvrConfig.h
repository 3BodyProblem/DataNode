#ifndef __DATA_NODE_SVRCONFIG_H__
#define	__DATA_NODE_SVRCONFIG_H__


#include <string>
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "ServiceIO/MServicePlug.hpp"


/**
 * @class						Configuration
 * @brief						�ڵ��������������Ϣ������
 * @date						2017/5/4
 * @author						barry
 */
class Configuration
{
private:
	Configuration();

public:
	/**
	 * @brief					ȡ�����ö���ĵ�������
	 */
	static Configuration&		GetConfigObj();

	/**
	 * @brief					��ʼ����������
	 * @return					==0				�ɹ�
								!=				ʧ��
	 */
	int							Load();

	/**
	 * @brief					�ڼ��ղ��Ա�ʶ
	 * @return					���ؽڼ����Ƿ���Ҫ���Եı�ʶ
	 */
	bool						GetTestFlag() const;

	/**
	 * @brief					�ڼ��ļ�·��
	 */
	const std::string&			GetHolidayFilePath() const;

	/**
	 * @brief					�ڴ���ģ��·��
	 */
	const std::string&			GetMemPluginPath() const;

	/**
	 * @brief					ѹ��ģ����·��
	 */
	const std::string&			GetCompressPluginPath() const;

	/**
	 * @brief					���ݲɼ����·��
	 */
	const std::string&			GetDataCollectorPluginPath() const;

	/**
	 * @brief					��������������
	 */
	const tagServicePlug_StartInParam&	GetStartInParam() const;

protected:
	tagServicePlug_StartInParam	m_oStartInParam;				///< ����������������

protected:
	bool						m_bTestFlag;					///< ���Ա�ʶ
	std::string					m_sHolidayFilePath;				///< �ڼ����ļ�·��
	std::string					m_sMemPluginPath;				///< �����ڴ��������·��
	std::string					m_sCompressPluginPath;			///< ����ѹ���������·��
	std::string					m_sDataCollectorPluginPath;		///< ���ݲɼ��������·��

};


#endif








