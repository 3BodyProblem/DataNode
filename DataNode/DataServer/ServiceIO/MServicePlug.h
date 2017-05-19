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
	//��ȡ��ǰִ���ļ�·��
	__inline string inner_get_application_path(void);
public:
	//�������������
	MServicePlug(void);
	virtual ~MServicePlug();
public:
	//��ʼ�����ͷ�
	int  Instance(const tagServicePlug_StartInParam * lpParam,char * szErrorString,unsigned int uiErrorSize);
	void Release(void);
	//ע��ص���Ӧ��
	void RegisterSpi(MServicePlug_Spi * lpSpi);
public:
	//��������־�������
	virtual void WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent);
	virtual void WriteInfo(const char * szFormat,...);
	virtual void WriteWarning(const char * szFormat,...);
	virtual void WriteError(const char * szFormat,...);
	virtual void WriteDetail(const char * szFormat,...);
	//��������̻�ȡ������־
	virtual bool IsStop(void);
	//��������
	virtual int  SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//���ʹ���
	virtual int  SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo);
	//��������
	virtual void PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
	//�ر�����
	virtual int  CloseLink(unsigned int uiLinkNo);
	//�������Ӹ�������
	virtual int  SetAddtionData(unsigned int uiLinkNo,unsigned int uiAddtionData);
	//��ȡ���Ӹ�������
	virtual int  GetAddtionData(unsigned int uiLinkNo,unsigned int * lpAddtionData);
	//��ȡ������ϸ��Ϣ
	virtual int  GetLinkInfo(unsigned int uiLinkNo,tagServicePlug_LinkInfo * lpLinkInfo);
	//��ȡ����״̬
	virtual int  GetStatus(tagServicePlug_Status * lpOut);
	//���ڴ�ط����ڴ�
	virtual char * Malloc(unsigned int uiSize);
	//�ͷŴ��ڴ�ط�����ڴ�
	virtual void Free(char * lpPtr);
};
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------