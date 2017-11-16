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
	 * @param[in]			sModulePath					压缩模块路径
	 * @return				==0							成功
							!=0							错误
	 */
	int						Initialize( std::string sModulePath );

	/**
	 * @breif				数据采集模块释放退出
	 */
	void					Release();

private:
	Dll						m_oDllPlugin;					///< 插件加载类
	T_Func_GetEncodeApi		m_pFuncEncodeApi;				///< 编码导出函数
};







#endif









