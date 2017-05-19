//-----------------------------------------------------------------------------------------------------------------------------
//                `-.   \    .-'
//        ,-"`````""-\__ |  /							文件名称：MServicePlug
//         '-.._    _.-'` '-o,							文件描述：ServicePlug接口
//             _>--:{{<   ) |)							文件编写：Lumy
//         .-''      '-.__.-o`							创建日期：2017.01.10
//        '-._____..-/`  |  \							更新日期：2017.01.10
//                ,-'   /    `-.
//-----------------------------------------------------------------------------------------------------------------------------
#ifndef __MServicePlug_H__
#define __MServicePlug_H__
//-----------------------------------------------------------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <string>
#include "MServicePlug.hpp"
using namespace std;
//-----------------------------------------------------------------------------------------------------------------------------
class MServicePlug
{
protected:
	HINSTANCE								m_hDllHandle;
	tagServicePlug_StartInParam				m_sInParam;
	tagServicePlug_StartOutParam			m_sOutParam;
	tagServicePlug_StartWork			*	m_lpStartWork;
	tagServicePlug_RegisterSpi			*	m_lpRegisterSpi;
	tagServicePlug_EndWork				*	m_lpEndWork;
protected:
	//获取当前执行文件路径
	__inline string inner_get_application_path(void);
public:
	//构造和析构函数
	MServicePlug(void);
	virtual ~MServicePlug();
public:
	//初始化和释放
	int  Instance(const tagServicePlug_StartInParam * lpParam,char * szErrorString,unsigned int uiErrorSize);
	void Release(void);
	//注册回调响应类
	void RegisterSpi(MServicePlug_Spi * lpSpi);
public:
	//本进程日志输出函数
	virtual void WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent);
	virtual void WriteInfo(const char * szFormat,...);
	virtual void WriteWarning(const char * szFormat,...);
	virtual void WriteError(const char * szFormat,...);
	virtual void WriteDetail(const char * szFormat,...);
	//本服务进程获取结束标志
	virtual bool IsStop(void);
	//发送数据
	virtual int  SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//发送错误
	virtual int  SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo);
	//推送数据
	virtual void PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//关闭连接
	virtual int  CloseLink(unsigned int uiLinkNo);
	//设置连接附加数据
	virtual int  SetAddtionData(unsigned int uiLinkNo,unsigned int uiAddtionData);
	//获取连接附加数据
	virtual int  GetAddtionData(unsigned int uiLinkNo,unsigned int * lpAddtionData);
	//获取连接详细信息
	virtual int  GetLinkInfo(unsigned int uiLinkNo,tagServicePlug_LinkInfo * lpLinkInfo);
	//获取服务状态
	virtual int  GetStatus(tagServicePlug_Status * lpOut);
	//从内存池分配内存
	virtual char * Malloc(unsigned int uiSize);
	//释放从内存池分配的内存
	virtual void Free(char * lpPtr);
};
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------