//-----------------------------------------------------------------------------------------------------------------------------
//                `-.   \    .-'
//        ,-"`````""-\__ |  /							�ļ����ƣ�MServicePlug
//         '-.._    _.-'` '-o,							�ļ�����������������ӿڲ��
//             _>--:{{<   ) |)							�ļ���д��Lumy
//         .-''      '-.__.-o`							�������ڣ�2017.01.05
//        '-._____..-/`  |  \							�������ڣ�2017.01.05
//                ,-'   /    `-.
//-----------------------------------------------------------------------------------------------------------------------------
#ifndef __ServiceManager_MServicePlug_H__
#define __ServiceManager_MServicePlug_H__
//-----------------------------------------------------------------------------------------------------------------------------
/*
��MServicePlug˵����

MServicePlug��Ҫ�ṩ�����������ܣ�

1����ServiceManager�����������������ServiceManager�������ֿ���ʵ�������ӹ���

	��1������־���ܣ��ṩͳһ��־����������
	��2����Զ�̿��ƹ��ܣ��ṩԶ�̿���ִ̨��Զ������ͷ��أ�����ʹ�øù�������ӡ���ݡ�ִ��������ܡ��޸����ݵȵȣ�
	��3�������ӹ��ܣ��ṩԶ�̼��ӣ�����״̬�����Ա����ڷ��������������Ǽ�ʱ��Ӧ��
	��4������ķ���ܣ��Գ������쳣��������������Զ������ý��̣�
	��5�������ù��ܣ��ṩͳһ�����ý��棩

2��������ͨѶ���ܣ���ѡ��

	�ṩ��׼ͨѶ�����ͨѶ�����ʹ������������ҵ����롣

3���ڴ�ع��ܣ���ѡ��
*/
//-----------------------------------------------------------------------------------------------------------------------------
//ServicePlug�ص���Ӧ�࣬ʹ��ʱ�Ӹ���̳�
class MServicePlug_Spi
{
public:
	//������״̬��Ӧ����������״̬ʱ�ص�
	//������������״̬��Ϣ���ַ�����ʽ����������״̬��Ϣ����д���ؼ�
	//˵���������ʽ�硰����Ԫ��Ϣ\n + ����Ԫ��Ϣ\n + ����Ԫ��Ϣ\n + ...�����س����зָ�
	//���� ����Ԫ��Ϣ = ������Ԫ����:����1,����2,����3,����4,����5,...�������ŷָ�
	//���� ���� = ��[������]�� �� ���� = ��working = true�� �� ���� = "keyname = value" �� ���� = "(n)keyname = value" ������n��ʾ���赥Ԫ��������Ĭ��1��
	virtual void OnReportStatus(char * szStatusInfo,unsigned int uiSize) = 0;

	//���������command��Ӧ����
	//���������ӱ�ţ���Ϣ��ţ�����Ԫ���ƣ��������ݣ�����ؽ��������ؽ������ֽ�
	virtual bool OnCommand(const char * szSrvUnitName,const char * szCommand,char * szResult,unsigned int uiSize) = 0;

	//�����ӵ�����Ӧ����������false��ʾ�����ܸ����ӣ���������Ͽ�������
	virtual bool OnNewLink(unsigned int uiLinkNo,unsigned int uiIpAddr,unsigned int uiPort)
	{
		return(false);
	}

	//���ӹر���Ӧ�������ر����� 0 ����ͨѶ���� 1 WSARECV�������� 2 ����������ر� 3 �ͻ��������ر� 4 �������ݴ�����ر�
	virtual void OnCloseLink(unsigned int uiLinkNo,int iCloseType)
	{

	}

	//���յ�������Ӧ����������false��ʾ�������ݴ��󣬷�������Ͽ�������
	virtual bool OnRecvData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,bool bErrorFlag,const char * lpData,unsigned int uiSize,unsigned int & uiAddtionData)
	{
		return(false);
	}
};
//-----------------------------------------------------------------------------------------------------------------------------
//��������־�������
//��������־���ͣ�����Ԫ���ƣ���־����
typedef void tagServicePlug_WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent);
//.............................................................................................................................
//��������̻�ȡ������־
typedef bool tagServicePlug_IsStop(void);
//.............................................................................................................................
//��������
typedef int  tagServicePlug_SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
//.............................................................................................................................
//���ʹ���
typedef int  tagServicePlug_SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo);
//.............................................................................................................................
//��������
typedef void tagServicePlug_PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize);
//.............................................................................................................................
//�ر�����
typedef int  tagServicePlug_CloseLink(unsigned int uiLinkNo);
//.............................................................................................................................
//���ڴ���з����ڴ棬����ʧ�ܷ���NULL��������ռ䲻�ܳ����ڴ������ռ�
typedef char * tagServicePlug_Malloc(unsigned int uiSize);
//.............................................................................................................................
//�ͷŴ��ڴ���з�����ڴ�
typedef void tagServicePlug_Free(char * lpPtr);
//.............................................................................................................................
typedef struct													//������ڲ���
{
	//��������Ϣ��
	unsigned int						uiVersion;				//������̰汾��
	char								szSrvUnitName[256];		//������̰����ĵ�Ԫ���ƣ���','���ŷָ���԰����������Ԫ

	//��ͨѶ��ء�
	unsigned int						uiMaxLinkCount;			//֧�ֵ�����������������Ϊ0��ʾ������ͨѶ���ܣ����������Ч��
	unsigned int						uiListenPort;			//�����˿�
	unsigned int						uiListenCount;			//�������г���
	unsigned int						uiSendBufCount;			//������Դ����
	unsigned int						uiThreadCount;			//�߳�����
	unsigned int						uiSendTryTimes;			//�������Դ���
	unsigned int						uiLinkTimeOut;			//���ӳ�ʱʱ�䡾�롿
	bool								bCompress;				//�Ƿ���Ҫ����ѹ��
	bool								bSSL;					//�Ƿ�����SSL����
	char								szPfxFilePasswrod[32];	//OpenSSL .pfx֤�����롾����SSL���ܡ�
	bool								bDetailLog;				//�Ƿ��ӡ��ϸ��־

	//���ڴ�أ�ͨѶ���Ҳʹ�ø��ڴ�ء�
	unsigned int						uiPageSize;				//�ڴ��ҳ���С
	unsigned int						uiPageCount;			//�ڴ��ҳ������

	//����
	char								szReserved[256];		//����
} tagServicePlug_StartInParam;
//.............................................................................................................................
typedef struct													//�������ڲ���
{
	//��������̻����ӿڡ�
	unsigned int						uiVersion;				//ServicePlug�汾��
	tagServicePlug_WriteReport		*	lpWriteReport;			//��־�������
	tagServicePlug_IsStop			*	lpIsStop;				//��ѯ�����Ƿ�ý�����־

	//���������ͨѶ�ӿڣ����������ͨѶ���ܣ�����ӿڷ���NULL��
	tagServicePlug_SendData			*	lpSendData;				//�������ݺ���
	tagServicePlug_SendError		*	lpSendError;			//���ʹ�����
	tagServicePlug_PushData			*	lpPushData;				//�������ݺ���
	tagServicePlug_CloseLink		*	lpCloseLink;			//�ر����Ӻ���

	//���ڴ�ء�
	tagServicePlug_Malloc			*	lpMalloc;				//���ڴ���з����ڴ�
	tagServicePlug_Free				*	lpFree;					//�ͷŴ��ڴ���з�����ڴ�

	//����
	char								szReserved[256];		//����
} tagServicePlug_StartOutParam;
//.............................................................................................................................
//��̬���ӿ��������
typedef int tagServicePlug_StartWork(const tagServicePlug_StartInParam * lpInParam,tagServicePlug_StartOutParam * lpOutParam);
typedef void tagServicePlug_RegisterSpi(MServicePlug_Spi * lpSpi);
typedef void tagServicePlug_EndWork(void);
//.............................................................................................................................
//�����û���˳��
//1����������ʱ�����Ⱦ�Ӧ��װ�ض�̬���ӿ⣬���ҵ���tagServicePlug_StartWork�������Ա��Ϻ�������־�ȹ��ܿ��ã������������lpErrorString��Ϊ������Ϣ
//2������������ģ��ĳ�ʼ��
//3������tagServicePlug_RegisterSpiע��ص���Ӧ�࣬��ʱ�Ϳ��Կ�ʼ������
//4���������ʱ������ֹͣ����������ģ�飬������tagServicePlug_EndWork����
//-----------------------------------------------------------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------------------------------------------------------