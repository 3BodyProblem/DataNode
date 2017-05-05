//-----------------------------------------------------------------------------------------------------------------------------
//                `-.   \    .-'
//        ,-"`````""-\__ |  /							�ļ����ƣ�MServicePlug
//         '-.._    _.-'` '-o,							�ļ�������ServicePlug�ӿ�
//             _>--:{{<   ) |)							�ļ���д��Lumy
//         .-''      '-.__.-o`							�������ڣ�2017.01.10
//        '-._____..-/`  |  \							�������ڣ�2017.01.10
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
	//��ȡ��ǰִ���ļ�·��
	__inline string inner_get_application_path(void);
public:
	//�������������
	MServicePlug(void);
	virtual ~MServicePlug();
public:
	//��ʼ�����ͷ�
	int  Instance(const tagServicePlug_StartInParam * lpParam);
	void Release(void);
	//ע��ص���Ӧ��
	void RegisterSpi(MServicePlug_Spi * lpSpi);
public:
	//��������־�������
	void WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent);
	void WriteInfo(const char * szFormat,...);
	void WriteWarning(const char * szFormat,...);
	void WriteError(const char * szFormat,...);
	void WriteDetail(const char * szFormat,...);
	//��������̻�ȡ������־
	bool IsStop(void);
	//��������
	int  SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//���ʹ���
	int  SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo);
	//��������
	void PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//�ر�����
	int  CloseLink(unsigned int uiLinkNo);
	//���ڴ�ط����ڴ�
	char * Malloc(unsigned int uiSize);
	//�ͷŴ��ڴ�ط�����ڴ�
	void Free(char * lpPtr);
};
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------