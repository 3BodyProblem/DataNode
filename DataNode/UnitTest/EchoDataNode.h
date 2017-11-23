#ifndef __QLX_DATA_NODE_TEST_H__
#define __QLX_DATA_NODE_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include "../DataServer/NodeServer.h"


class XCodeMsgPool : public SendPackagePool
{
protected:
	/**
	 * @brief					将package中的数据整体推送下发
	 * @return					发送的数据大小
	 */
	virtual int					SendAllPkg();
};


/**
 * @class							tagMsgInfo
 * @brief							消息描述信息
 * @author							barry
 */
typedef struct
{
	unsigned short					nMsgLen;		///< 消息长度
	unsigned __int64				nMsgCount;		///< 消息计数
} tagMsgInfo;


/**
 * @class				EchoNodeEngine
 * @brief				行情回显引擎
 * @author				barry
 */
class EchoNodeEngine : public I_DataHandle
{
public:
	EchoNodeEngine();

	/**
 	 * @brief				初始化行情各参数，准备工作
	 * @param[in]			nMsgID						消息ID
	 * @param[in]			sCode						商品代码
	 * @note				流程中，先从本地文件加载内存插件的行情数据，再初始化行情解析插件
	 * @return				==0							成功
							!=0							失败
	 */
	int						Initialize( unsigned int nMsgID, std::string sCode );

	/**
	 * @brief				释放行情模块各资源
	 */
	void					Release();

	/**
	 * @brief				重新加载/初始化行情(内存插件、数据采集器)
	 * @detail				初始化部分的所有业务流程都在这个函数里了
	 * @return				true						初始化成功
							false						失败
	 */
	bool					EchoQuotation();

public:///< I_DataHandle接口实现: 用于给数据采集模块提供行情数据的回调方法
	/**
 	 * @brief				初始化性质的行情数据回调
	 * @note				只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]			nDataID						消息ID
	 * @param[in]			pData						数据内容
	 * @param[in]			nDataLen					长度
	 * @param[in]			bLastFlag					是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				==0							成功
							!=0							错误
	 */
	virtual int				OnImage( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				实行行情数据回调
	 * @note				更新行情内存块，并推送
	 * @param[in]			nDataID						消息ID
	 * @param[in]			pData						数据内容
	 * @param[in]			nDataLen					长度
	 * @param[in]			bLastFlag					是否为最后一个包的标识
	 * @param[in]			bPushFlag					推送标识
	 * @return				==0							成功
							!=0							错误
	 */
	virtual int				OnData( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag, bool bPushFlag = true );

	/**
	 * @brief				行情数据帧直接转发回调接口
	 * @note				不进行数据包的解析，直接转发推送
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataID				消息ID
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnStream( unsigned int nDataID, const char* pData, unsigned int nDataLen );

	/**
	 * @brief				内存数据查询接口
	 * @param[in]			nDataID						消息ID
	 * @param[in,out]		pData						数据内容(包含查询主键)
	 * @param[in]			nDataLen					长度
	 * @return				>0							成功,返回数据结构的大小
							==0							没查到结果
							!=0							错误
	 * @note				如果pData的缓存为“全零”缓存，则返回表内的所有数据
	 */
	virtual int				OnQuery( unsigned int nDataID, const char* pData, unsigned int nDataLen );

	/**
	 * @brief				日志函数
	 * @param[in]			nLogLevel					日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
	 * @param[in]			pszFormat					字符串格式化串
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

public:///< Log Method
	/**
	 * @brief				信息
	 */
	virtual void			WriteInfo( const char * szFormat,... );

	/**
	 * @brief				警告
	 */
	virtual void			WriteWarning( const char * szFormat,... );

	/**
	 * @brief				错误
	 */
	virtual void			WriteError( const char * szFormat,... );

	/**
	 * @brief				明细
	 */
	virtual void			WriteDetail( const char * szFormat,... );

public:
	static int				PrintMessage( unsigned int nMarketID, unsigned int nDataID, const char* pData );

protected:///< 挂载相关插件
	XCodeMsgPool			m_oSendPackage;					///< 数据缓存 + 编/解码模块
	DataCollector			m_oDataCollector;				///< 行情采集模块接口
	int						m_nMessageID;					///< 订阅回显的消息ID( ==0, 表示显示全部 )
	std::string				m_sCode;						///< 订阅回显的消息Code( =="", 表示显示全部 )
};



#endif





