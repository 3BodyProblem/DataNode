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
using namespace std;
#include "MServicePlug.hpp"
//-----------------------------------------------------------------------------------------------------------------------------
class MServicePlug
{
protected:
	HINSTANCE								m_hDllHandle;
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
	int  Instance(const tagServicePlug_StartInParam * lpParam);
	void Release(void);
	//注册回调响应类
	void RegisterSpi(MServicePlug_Spi * lpSpi);
public:
	//本进程日志输出函数
	void WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent);
	void WriteInfo(const char * szFormat,...);
	void WriteWarning(const char * szFormat,...);
	void WriteError(const char * szFormat,...);
	void WriteDetail(const char * szFormat,...);
	//本服务进程获取结束标志
	bool IsStop(void);
	//发送数据
	int  SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//发送错误
	int  SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo);
	//推送数据
	void PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//关闭连接
	int  CloseLink(unsigned int uiLinkNo);
	//从内存池分配内存
	char * Malloc(unsigned int uiSize);
	//释放从内存池分配的内存
	void Free(char * lpPtr);
};
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------