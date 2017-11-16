#ifndef __DATA_ENCODER_H__
#define	__DATA_ENCODER_H__


#include <string>
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../../../DataXCode/DataXCode/IXCode.h"


/**
 * @class					DataEncoder
 * @brief					行情数序列化模块
 * @date					2017/11/12
 * @author					barry
 */
class DataEncoder
{
public:
	DataEncoder();

	/**
	 * @brief				数据采集模块初始化
	 * @param[in]			sPluginPath					压缩模块路径
	 * @param[in]			sCnfXml						压缩策略配置xml
	 * @param[in]			nXCodeBuffSize				预分配序列化缓存的长度
	 * @return				==0							成功
							!=0							错误
	 */
	int						Initialize( std::string sPluginPath, std::string sCnfXml, unsigned int nXCodeBuffSize );

	/**
	 * @breif				数据采集模块释放退出
	 */
	void					Release();



public:
	/**
	 * @brief				获取缓存地址
	 */
	const char*				GetBufferPtr();

	/**
	 * @brief				获取取缓存数据长度
	 */
	unsigned int			GetBufferLen();

private:
	Dll						m_oDllPlugin;					///< 插件加载类
	InterfaceEncode*		m_pEncoderApi;					///< 序列化对象指针
	char*					m_pXCodeBuffer;					///< 编码缓存
	unsigned int			m_nMaxBufferLen;				///< 缓存最大长度
	unsigned int			m_nDataLen;						///< 缓存数据长度
};







#endif









