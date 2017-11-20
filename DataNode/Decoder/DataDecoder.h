#ifndef __DATA_DECODER_H__
#define	__DATA_DECODER_H__


#include "../../../DataXCode/DataXCode/IXCode.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Dll.h"


/**
 * @class					DataDecoder
 * @brief					行情数序列化模块
 * @date					2017/11/12
 * @author					barry
 */
class DataDecoder
{
public:
	DataDecoder();

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

	/**
	 * @brief				准备一次新的压缩
	 * @param[in]			pData						数据体地址(Body部分)
	 * @param[in]			nLen						数据体长度
	 * @note				只对同一个message类型的数据组进行压缩
	 * @return				==0							成功
							!=0							错误
	 */
	int						Prepare4AUncompression( const char* pData, unsigned int nLen );

	/**
	 * @brief				序列化数据到缓存
	 * @param[in]			nMsgID						Message ID
	 * @param[in]			pData						消息数据地址
	 * @param[in]			nLen						消息长度
	 * @return				>= 0						成功，返回历次调用后,未序列化部分的长度
	 */
	int						UncompressData( unsigned short nMsgID, char *pData, unsigned int nLen );

private:
	Dll						m_oDllPlugin;					///< 插件加载类
	InterfaceDecode*		m_pDecoderApi;					///< 反序列化对象指针
};







#endif










