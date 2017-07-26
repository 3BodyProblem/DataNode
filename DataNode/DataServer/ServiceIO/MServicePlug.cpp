//-----------------------------------------------------------------------------------------------------------------------------
#include "MServicePlug.h"
//-----------------------------------------------------------------------------------------------------------------------------
MServicePlug::MServicePlug(void)
{
	memset(&m_sOutParam,0x00,sizeof(tagServicePlug_StartOutParam));

	m_hDllHandle = NULL;
	m_lpStartWork = NULL;
	m_lpRegisterSpi = NULL;
	m_lpEndWork = NULL;
}
//.............................................................................................................................
MServicePlug::~MServicePlug()
{
	Release();
}
//.............................................................................................................................
string MServicePlug::inner_get_application_path(void)
{
	char						tempbuf[256];
	int							i;

	::GetModuleFileNameA(NULL,tempbuf,sizeof(tempbuf));
	for ( i=strlen(tempbuf)-1;i>=0;i-- )
	{
		if ( tempbuf[i] == '\\' || tempbuf[i] == '/' )
		{
			tempbuf[i+1] = 0;
			break;
		}
	}

	return(tempbuf);
}
//.............................................................................................................................
int  MServicePlug::Instance(const tagServicePlug_StartInParam * lpParam,char * szErrorString,unsigned int uiErrorSize)
{
	char						dllpath[256];
	int							errorcode;

	Release();
	
	_snprintf(dllpath,sizeof(dllpath),"%sServicePlug.dll",inner_get_application_path().c_str());
	if ( (m_hDllHandle = ::LoadLibraryA(dllpath)) == NULL )
	{
		_snprintf(szErrorString,uiErrorSize,"{MServicePlug}装载动态链接库%s发生错误（GetLastError() = %d）",dllpath,::GetLastError());
		Release();
		return(-1);
	}

	m_lpStartWork = (tagServicePlug_StartWork *)::GetProcAddress(m_hDllHandle,"StartWork");
	m_lpRegisterSpi = (tagServicePlug_RegisterSpi *)::GetProcAddress(m_hDllHandle,"RegisterSpi");
	m_lpEndWork = (tagServicePlug_EndWork *)::GetProcAddress(m_hDllHandle,"EndWork");
	if ( m_lpStartWork == NULL || m_lpRegisterSpi == NULL || m_lpEndWork == NULL )
	{
		_snprintf(szErrorString,uiErrorSize,"{MServicePlug}装载动态链接库%s发生错误（函数不匹配）",dllpath);
		Release();
		return(-2);
	}

	memcpy(&m_sInParam,lpParam,sizeof(tagServicePlug_StartInParam));
	try
	{
		if ( (errorcode = m_lpStartWork(&m_sInParam,&m_sOutParam)) < 0 )
		{
			_snprintf(szErrorString,uiErrorSize,"{MServicePlug}装载动态链接库%s发生错误（StartWork函数返回错误，ERR = %d）",dllpath,errorcode);
			Release();
			return(-3);
		}
	}
	catch(...)
	{
		_snprintf(szErrorString,uiErrorSize,"{MServicePlug}装载动态链接库%s发生错误（StartWork函数发生未知异常）",dllpath);
		Release();
		return(-4);
	}

	return(1);
}
//.............................................................................................................................
void MServicePlug::Release(void)
{
	memset(&m_sOutParam,0x00,sizeof(tagServicePlug_StartOutParam));

	if ( m_lpEndWork != NULL )
	{
		try
		{
			m_lpEndWork();
		}
		catch(...)
		{

		}
	}

	m_lpStartWork = NULL;
	m_lpRegisterSpi = NULL;
	m_lpEndWork = NULL;

	if ( m_hDllHandle != NULL )
	{
		try
		{
			::FreeLibrary(m_hDllHandle);
		}
		catch(...)
		{

		}
		m_hDllHandle = NULL;
	}
}
//.............................................................................................................................
void MServicePlug::RegisterSpi(MServicePlug_Spi * lpSpi)
{
	try
	{
		if ( m_lpRegisterSpi != NULL )
		{
			m_lpRegisterSpi(lpSpi);
		}
	}
	catch(...)
	{

	}
}
//.............................................................................................................................
void MServicePlug::WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent)
{
	try
	{
		if ( m_sOutParam.lpWriteReport != NULL )
		{
			m_sOutParam.lpWriteReport(szType,szSrvUnitName,szContent);
		}
	}
	catch(...)
	{

	}
}
//.............................................................................................................................
void MServicePlug::WriteInfo(const char * szFormat,...)
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	WriteReport("信息","",tempbuf);
}
//.............................................................................................................................
void MServicePlug::WriteWarning(const char * szFormat,...)
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	WriteReport("警告","",tempbuf);
}
//.............................................................................................................................
void MServicePlug::WriteError(const char * szFormat,...)
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	WriteReport("错误","",tempbuf);
}
//.............................................................................................................................
void MServicePlug::WriteDetail(const char * szFormat,...)
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	WriteReport("详细","",tempbuf);
}
//.............................................................................................................................
bool MServicePlug::IsStop(void)
{
	try
	{
		if ( m_sOutParam.lpIsStop == NULL )
		{
			return(true);
		}

		return(m_sOutParam.lpIsStop());
	}
	catch(...)
	{
		return(true);
	}
}
//.............................................................................................................................
int  MServicePlug::SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize)
{
	try
	{
		if ( m_sOutParam.lpSendData == NULL )
		{
			return(-1);
		}

		return(m_sOutParam.lpSendData(uiLinkNo,usMessageNo,usFunctionID,lpInBuf,uiInSize));
	}
	catch(...)
	{
		return(-2);
	}
}
//.............................................................................................................................
int  MServicePlug::SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo)
{
	try
	{
		if ( m_sOutParam.lpSendError == NULL )
		{
			return(-1);
		}

		return(m_sOutParam.lpSendError(uiLinkNo,usMessageNo,usFunctionID,lpErrorInfo));
	}
	catch(...)
	{
		return(-2);
	}
}
//.............................................................................................................................
void MServicePlug::PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize)
{
	try
	{
		if ( m_sOutParam.lpPushData != NULL )
		{
			m_sOutParam.lpPushData(lpLinkNoSet,uiLinkNoCount,usMessageNo,usFunctionID,lpInBuf,uiInSize);
		}
	}
	catch(...)
	{

	}
}
//.............................................................................................................................
int  MServicePlug::CloseLink(unsigned int uiLinkNo)
{
	try
	{
		if ( m_sOutParam.lpCloseLink == NULL )
		{
			return(-1);
		}

		return(m_sOutParam.lpCloseLink(uiLinkNo));
	}
	catch(...)
	{
		return(-2);
	}
}
//.............................................................................................................................
int  MServicePlug::SetAddtionData(unsigned int uiLinkNo,unsigned int uiAddtionData)
{
	try
	{
		if ( m_sOutParam.lpSetAddtionData == NULL )
		{
			return(-1);
		}

		return(m_sOutParam.lpSetAddtionData(uiLinkNo,uiAddtionData));
	}
	catch(...)
	{
		return(-2);
	}
}
//.............................................................................................................................
int  MServicePlug::GetAddtionData(unsigned int uiLinkNo,unsigned int * lpAddtionData)
{
	try
	{
		if ( m_sOutParam.lpGetAddtionData == NULL )
		{
			return(-1);
		}

		return(m_sOutParam.lpGetAddtionData(uiLinkNo,lpAddtionData));
	}
	catch(...)
	{
		return(-2);
	}
}
//.............................................................................................................................
int  MServicePlug::GetLinkInfo(unsigned int uiLinkNo,tagServicePlug_LinkInfo * lpLinkInfo)
{
	try
	{
		if ( m_sOutParam.lpGetLinkInfo == NULL )
		{
			return(-1);
		}

		return(m_sOutParam.lpGetLinkInfo(uiLinkNo,lpLinkInfo));
	}
	catch(...)
	{
		return(-2);
	}
}
//.............................................................................................................................
int  MServicePlug::GetStatus(tagServicePlug_Status * lpOut)
{
	try
	{
		if ( m_sOutParam.lpGetStatus == NULL )
		{
			return(-1);
		}

		return(m_sOutParam.lpGetStatus(lpOut));
	}
	catch(...)
	{
		return(-2);
	}
}
//.............................................................................................................................
char * MServicePlug::Malloc(unsigned int uiSize)
{
	try
	{
		if ( m_sOutParam.lpMalloc == NULL )
		{
			return(NULL);
		}

		return(m_sOutParam.lpMalloc(uiSize));
	}
	catch(...)
	{
		return(NULL);
	}
}
//.............................................................................................................................
void MServicePlug::Free(char * lpPtr)
{
	try
	{
		if ( m_sOutParam.lpFree != NULL )
		{
			m_sOutParam.lpFree(lpPtr);
		}
	}
	catch(...)
	{

	}
}
//-----------------------------------------------------------------------------------------------------------------------------