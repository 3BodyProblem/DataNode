//-----------------------------------------------------------------------------------------------------------------------------
//                `-.   \    .-'
//        ,-"`````""-\__ |  /							文件名称：MServicePlug
//         '-.._    _.-'` '-o,							文件描述：服务管理器接口插件
//             _>--:{{<   ) |)							文件编写：Lumy
//         .-''      '-.__.-o`							创建日期：2017.01.05
//        '-._____..-/`  |  \							更新日期：2017.01.05
//                ,-'   /    `-.
//-----------------------------------------------------------------------------------------------------------------------------
#ifndef __ServiceManager_MServicePlug_H__
#define __ServiceManager_MServicePlug_H__
//-----------------------------------------------------------------------------------------------------------------------------
/*
【MServicePlug说明】

MServicePlug主要提供以下两个功能：

1、与ServiceManager服务管理器交互，与ServiceManager交互后，又可以实现以下子功能

	（1）、日志功能（提供统一日志管理和输出）
	（2）、远程控制功能（提供远程控制台执行远程命令和返回，可以使用该功能来打印数据、执行命令或功能、修改数据等等）
	（3）、监视功能（提供远程监视，进行状态管理，以便于在服务器发生问题是及时响应）
	（4）、保姆功能（对程序发生异常，服务管理器会自动重启该进程）
	（5）、配置功能（提供统一的配置界面）

2、服务器通讯功能（可选）

	提供标准通讯或加密通讯组件，使服务器仅处理业务代码。

3、内存池功能（可选）
*/
//-----------------------------------------------------------------------------------------------------------------------------
//ServicePlug回调响应类，使用时从该类继承
class MServicePlug_Spi
{
public:
	//本进程状态响应函数，报告状态时回调
	//参数：服务器状态信息（字符串形式），服务器状态信息可填写最大控件
	//说明：具体格式如“服务单元信息\n + 服务单元信息\n + 服务单元信息\n + ...”，回车换行分割
	//其中 服务单元信息 = “服务单元名称:属性1,属性2,属性3,属性4,属性5,...”，逗号分割
	//其中 属性 = “[分组名]” 或 属性 = “working = true” 或 属性 = "keyname = value" 或 属性 = "(n)keyname = value" ，其中n表示所需单元格数量，默认1个
	virtual void OnReportStatus(char * szStatusInfo,unsigned int uiSize) = 0;

	//本服务进程command响应函数
	//参数：连接编号，消息序号，服务单元名称，命令内容，命令返回结果，命令返回结果最大字节
	virtual bool OnCommand(const char * szSrvUnitName,const char * szCommand,char * szResult,unsigned int uiSize) = 0;

	//新连接到达响应函数，返回false表示不接受该连接，服务器会断开该连接
	virtual bool OnNewLink(unsigned int uiLinkNo,unsigned int uiIpAddr,unsigned int uiPort)
	{
		return(false);
	}

	//连接关闭响应函数，关闭类型 0 结束通讯服务 1 WSARECV发生错误 2 服务端主动关闭 3 客户端主动关闭 4 处理数据错误而关闭
	virtual void OnCloseLink(unsigned int uiLinkNo,int iCloseType)
	{

	}

	//接收到数据响应函数，返回false表示处理数据错误，服务器会断开该链接
	virtual bool OnRecvData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,bool bErrorFlag,const char * lpData,unsigned int uiSize,unsigned int & uiAddtionData)
	{
		return(false);
	}
};
//-----------------------------------------------------------------------------------------------------------------------------
//本进程日志输出函数
//参数：日志类型，服务单元名称，日志内容
typedef void tagServicePlug_WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent);
//.............................................................................................................................
//本服务进程获取结束标志
typedef bool tagServicePlug_IsStop(void);
//.............................................................................................................................
//发送数据
typedef int  tagServicePlug_SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
//.............................................................................................................................
//发送错误
typedef int  tagServicePlug_SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo);
//.............................................................................................................................
//推送数据
typedef void tagServicePlug_PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
//.............................................................................................................................
//关闭连接
typedef int  tagServicePlug_CloseLink(unsigned int uiLinkNo);
//.............................................................................................................................
//从内存池中分配内存，分配失败返回NULL，最大分配空间不能超过内存池总体空间
typedef char * tagServicePlug_Malloc(unsigned int uiSize);
//.............................................................................................................................
//释放从内存池中分配的内存
typedef void tagServicePlug_Free(char * lpPtr);
//.............................................................................................................................
typedef struct													//启动入口参数
{
	//【基本信息】
	unsigned int						uiVersion;				//服务进程版本号
	char								szSrvUnitName[256];		//服务进程包括的单元名称，用','逗号分割，可以包含多个服务单元

	//【通讯相关】
	unsigned int						uiMaxLinkCount;			//支持的最大连接数量【如果为0表示不启动通讯功能，下面参数无效】
	unsigned int						uiListenPort;			//监听端口
	unsigned int						uiListenCount;			//监听队列长度
	unsigned int						uiSendBufCount;			//发送资源数量
	unsigned int						uiThreadCount;			//线程数量
	unsigned int						uiSendTryTimes;			//发送重试次数
	unsigned int						uiLinkTimeOut;			//连接超时时间【秒】
	bool								bCompress;				//是否需要进行压缩
	bool								bSSL;					//是否启用SSL加密
	char								szPfxFilePasswrod[32];	//OpenSSL .pfx证书密码【仅限SSL加密】
	bool								bDetailLog;				//是否打印详细日志

	//【内存池：通讯相关也使用该内存池】
	unsigned int						uiPageSize;				//内存池页面大小
	unsigned int						uiPageCount;			//内存池页面数量

	//保留
	char								szReserved[256];		//保留
} tagServicePlug_StartInParam;
//.............................................................................................................................
typedef struct													//启动出口参数
{
	//【服务进程基本接口】
	unsigned int						uiVersion;				//ServicePlug版本号
	tagServicePlug_WriteReport		*	lpWriteReport;			//日志输出函数
	tagServicePlug_IsStop			*	lpIsStop;				//查询进程是否该结束标志

	//【服务进程通讯接口：如果不启动通讯功能，下面接口返回NULL】
	tagServicePlug_SendData			*	lpSendData;				//发送数据函数
	tagServicePlug_SendError		*	lpSendError;			//发送错误函数
	tagServicePlug_PushData			*	lpPushData;				//推送数据函数
	tagServicePlug_CloseLink		*	lpCloseLink;			//关闭连接函数

	//【内存池】
	tagServicePlug_Malloc			*	lpMalloc;				//从内存池中分配内存
	tagServicePlug_Free				*	lpFree;					//释放从内存池中分配的内存

	//保留
	char								szReserved[256];		//保留
} tagServicePlug_StartOutParam;
//.............................................................................................................................
//动态连接库输出函数
typedef int tagServicePlug_StartWork(const tagServicePlug_StartInParam * lpInParam,tagServicePlug_StartOutParam * lpOutParam);
typedef void tagServicePlug_RegisterSpi(MServicePlug_Spi * lpSpi);
typedef void tagServicePlug_EndWork(void);
//.............................................................................................................................
//【调用基本顺序】
//1、程序启动时，首先就应该装载动态连接库，并且调用tagServicePlug_StartWork函数，以保障后续的日志等功能可用，如果启动错误，lpErrorString中为错误信息
//2、程序本身其他模块的初始化
//3、调用tagServicePlug_RegisterSpi注册回调响应类，此时就可以开始工作了
//4、程序结束时，优先停止本程序其他模块，最后调用tagServicePlug_EndWork结束
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------