#ifndef __DATA_NODE_SVRCONFIG_H__
#define	__DATA_NODE_SVRCONFIG_H__


#include <string>
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"


/**
 * @class				Configuration
 * @brief				�ڵ��������������Ϣ������
 * @date				2017/5/4
 * @author				barry
 */
class Configuration
{
private:
	Configuration();

public:
	/**
	 * @brief				ȡ�����ö���ĵ�������
	 */
	static Configuration&	GetConfigObj();

	/**
	 * @brief				��ʼ����������
	 * @return				==0				�ɹ�
							!=				ʧ��
	 */
	int						Load();

	const std::string&		GetMemPluginPath() const;
	const std::string&		GetCompressPluginPath() const;
	const std::string&		GetDataCollectorPluginPath() const;

protected:
	std::string				m_sMemPluginPath;				///< �����ڴ��������·��
	std::string				m_sCompressPluginPath;			///< ����ѹ���������·��
	std::string				m_sDataCollectorPluginPath;		///< ���ݲɼ��������·��
};


#endif








