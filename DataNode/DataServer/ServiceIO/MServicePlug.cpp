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
int  MServicePlug::Instance(const tagServicePlug_StartInParam * lpParam)
{
	char						dllpath[256];
	int							errorcode;

	Release();
	
	_snprintf(dllpath,sizeof(dllpath),"%sServicePlug.dll",inner_get_application_path().c_str());
	if ( (m_hDllHandle = ::LoadLibraryA(dllpath)) == NULL )
	{
		Release();
		return(-1);
	}

	m_lpStartWork = (tagServicePlug_StartWork *)::GetProcAddress(m_hDllHandle,"StartWork");
	m_lpRegisterSpi = (tagServicePlug_RegisterSpi *)::GetProcAddress(m_hDllHandle,"RegisterSpi");
	m_lpEndWork = (tagServicePlug_EndWork *)::GetProcAddress(m_hDllHandle,"EndWork");
	if ( m_lpStartWork == NULL || m_lpRegisterSpi == NULL || m_lpEndWork == NULL )
	{
		Release();
		return(-2);
	}

	if ( (errorcode = m_lpStartWork(lpParam,&m_sOutParam)) < 0 )
	{
		Release();
		return(-3);
	}

	return(1);
}
//.............................................................................................................................
void MServicePlug::Release(void)
{
	memset(&m_sOutParam,0x00,sizeof(tagServicePlug_StartOutParam));

	if ( m_lpEndWork != NULL )
	{
		m_lpEndWork();
	}

	m_lpStartWork = NULL;
	m_lpRegisterSpi = NULL;
	m_lpEndWork = NULL;

	if ( m_hDllHandle != NULL )
	{
		::FreeLibrary(m_hDllHandle);
		m_hDllHandle = NULL;
	}
}
//.............................................................................................................................
void MServicePlug::RegisterSpi(MServicePlug_Spi * lpSpi)
{
	if ( m_lpRegisterSpi != NULL )
	{
		m_lpRegisterSpi(lpSpi);
	}
}
//.............................................................................................................................
void MServicePlug::WriteReport(const char * szType,const char * szSrvUnitName,const char * szContent)
{
	if ( strcmp(szType,"信息") == 0 )
	{
		::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),10);
		printf("[信息]{%s}%s\n",szSrvUnitName,szContent);
	}
	else if ( strcmp(szType,"警告") == 0 )
	{
		::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),14);
		printf("[警告]{%s}%s\n",szSrvUnitName,szContent);
	}
	else if ( strcmp(szType,"错误") == 0 )
	{
		::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12);
		printf("[错误]{%s}%s\n",szSrvUnitName,szContent);
	}
	else
	{
		::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),8);
		printf("[详细]{%s}%s\n",szSrvUnitName,szContent);
	}

	if ( m_sOutParam.lpWriteReport != NULL )
	{
		m_sOutParam.lpWriteReport(szType,szSrvUnitName,szContent);
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
	if ( m_sOutParam.lpIsStop == NULL )
	{
		return(true);
	}

	return(m_sOutParam.lpIsStop());
}
//.............................................................................................................................
int  MServicePlug::SendData(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize)
{
	if ( m_sOutParam.lpSendData == NULL )
	{
		return(-1);
	}

	return(m_sOutParam.lpSendData(uiLinkNo,usMessageNo,usFunctionID,lpInBuf,uiInSize));
}
//.............................................................................................................................
int  MServicePlug::SendError(unsigned int uiLinkNo,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpErrorInfo)
{
	if ( m_sOutParam.lpSendError == NULL )
	{
		return(-1);
	}

	return(m_sOutParam.lpSendError(uiLinkNo,usMessageNo,usFunctionID,lpErrorInfo));
}
//.............................................................................................................................
void MServicePlug::PushData(const unsigned int * lpLinkNoSet,unsigned int uiLinkNoCount,unsigned short usMessageNo,unsigned short usFunctionID,const char * lpInBuf,unsigned int uiInSize)
{
	if ( m_sOutParam.lpPushData != NULL )
	{
		m_sOutParam.lpPushData(lpLinkNoSet,uiLinkNoCount,usMessageNo,usFunctionID,lpInBuf,uiInSize);
	}
}
//.............................................................................................................................
int  MServicePlug::CloseLink(unsigned int uiLinkNo)
{
	if ( m_sOutParam.lpCloseLink == NULL )
	{
		return(-1);
	}

	return(m_sOutParam.lpCloseLink(uiLinkNo));
}
//.............................................................................................................................
char * MServicePlug::Malloc(unsigned int uiSize)
{
	if ( m_sOutParam.lpMalloc == NULL )
	{
		return(NULL);
	}

	return(m_sOutParam.lpMalloc(uiSize));
}
//.............................................................................................................................
void MServicePlug::Free(char * lpPtr)
{
	if ( m_sOutParam.lpFree != NULL )
	{
		m_sOutParam.lpFree(lpPtr);
	}
}
//-----------------------------------------------------------------------------------------------------------------------------