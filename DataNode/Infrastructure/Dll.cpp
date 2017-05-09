#include "Dll.h"
#include <exception>


Dll::Dll(void)
{
	#ifndef LINUXCODE
		m_hDll = NULL;
	#else
		m_lpDll = NULL;
		memset(m_szDllSelfPath, 0, sizeof(m_szDllSelfPath));
	#endif
}

Dll::~Dll()
{
	CloseDll();
}

#ifdef LINUXCODE
char	*Dll::GetDllSelfPath()
{
	return m_szDllSelfPath;
}

void	Dll::MergeDllSelfPath(const char *in, void *hModule)
{
	int		iRet, i, n, n1;
	char	szProcessPath[MAX_PATH];
	char	szOldName[MAX_PATH], szNewName[MAX_PATH];
	char	*lpszPath, *lpszName;
	char	*lpszPtr[64];
	char	*saveptr = NULL;
	/**
	 *	strtok 不是线程安全,改用POSIX 建议的strtok_r
	 */

	if(!in)
		return;

		//	合并DLL的自身的路径
	if(*in == '/' || *in == '\\')	//	\\主要为了兼容处理,\上根目录在UNIX系统上面完全就是不合法的
	{
		//	是绝对路径,不需要处理
		strncpy(m_szDllSelfPath, in, sizeof(m_szDllSelfPath));
	}
	else
	{
		memset(szProcessPath, 0, sizeof(szProcessPath));
		//	是相对路径
		if(!hModule)
		{
			//	EXE-->LoadDll
			iRet = readlink("/proc/self/exe", szProcessPath, MAX_PATH);
			if(iRet <= 0)
			{
#ifdef _DEBUG
				printf("call readlink error(%d)\n", errno);fflush(stdout);
#endif
				return;
			}
		}
		else
		{
			//	DLL-->LoadDll
			class MDll	*pModule;
			pModule = (class MDll *)hModule;
			strncpy(szProcessPath, pModule->GetDllSelfPath(), sizeof(szProcessPath));
			if(strlen(szProcessPath) == 0)
			{
#ifdef _DEBUG
				printf("call pModule->GetDllSelfPath() error\n");fflush(stdout);
#endif
				return;
			}
		}
		
		memset(szOldName, 0, sizeof(szOldName));
		strncpy(szOldName, in, sizeof(szOldName));
		
		//	计算文件名中的..个数 并且cut掉.这个目录
		memset(szNewName, 0, sizeof(szNewName));
		n = 0;
		lpszName = strtok_r(szOldName, "/\\", &saveptr);
		while(lpszName)
		{
			if(strcmp(lpszName, "..") == 0)
				n++;
			else if(strcmp(lpszName, ".") == 0)
			{
				//	nothing
			}
			else
			{
				strcat(szNewName, lpszName);
				strcat(szNewName, "/");
			}
			lpszName = strtok_r(NULL, "/\\", &saveptr);
		}
		if(strlen(szNewName) > 0)
			szNewName[strlen(szNewName) - 1] = 0;
		//	根据n值 cut 进程的路径
		n1 = 0;
		lpszPath = strtok_r(szProcessPath, "/\\", &saveptr);
		while(lpszPath)
		{
			//	Save 指针
			lpszPtr[n1++] = lpszPath;
			lpszPath = strtok_r(NULL, "/\\", &saveptr);
		}
		n1--;	//	丢弃最后一个文件名
		n = n1 - n;	//	计算压入的目录层数
		strcpy(m_szDllSelfPath, "/");
		for(i = 0; i < n; i++)
		{
			strcat(m_szDllSelfPath, lpszPtr[i]);strcat(m_szDllSelfPath, "/");
		}
		strcat(m_szDllSelfPath, szNewName);
	}
#ifdef _DEBUG
	printf("MergeDllSelfPath:%s\n", m_szDllSelfPath);fflush(stdout);
#endif
}
#endif

int Dll::LoadDll( std::string strFileName, void *hModule /* = NULL */, BOOL bdllmain /* = TRUE */ )
{
	#ifndef LINUXCODE
		if ( (m_hDll = ::LoadLibrary(strFileName.c_str())) == NULL )
		{
			return -1;
		}

		return 0;
	#else
		if ( (m_lpDll = dlopen(strFileName.c_str(),RTLD_LAZY)) == NULL )
		{
			printf("dlopen: %d , %s\n", errno, dlerror()); fflush(stdout);
			return(MErrorCode::GetSysErr());
		}

		if ( bdllmain )		// add by yuanjj, not all the 'so' need to call 'DllMain'
		{
			typedef int	fnDllMain(void  * , unsigned long, void *);
			fnDllMain	*pfnDllMain;
			pfnDllMain = (fnDllMain *)dlsym(m_lpDll, "DllMain");
			if(!pfnDllMain)
			{
				printf("WARNING:(%s) undefined symbol: DllMain\n", strFileName.c_str());fflush(stdout);
				return ERR_MDLL_LOSTDLLMAIN;
			}

			MergeDllSelfPath(strFileName.c_str(), hModule);

			pfnDllMain(this, 0, NULL);
		}

		return 0;

	#endif
}

void* Dll::GetDllFunction( std::string strFunctionName )
{
	#ifndef LINUXCODE

		assert(m_hDll != NULL);
		return(::GetProcAddress(m_hDll,strFunctionName.c_str()));

	#else

		void	*pFunc;
		assert(m_lpDll != NULL);
		pFunc = dlsym(m_lpDll,strFunctionName.c_str());
#ifdef _DEBUG
		if(!pFunc)
		{
			printf("ERROR:%d,%s\n", errno, dlerror()); fflush(stdout);
		}
#endif
		return pFunc;
	#endif
}

void Dll::CloseDll(void)
{
#ifndef LINUXCODE

	if ( m_hDll != NULL )
	{
	#ifndef	_LINUXTRYOFF
		try
		{
	#endif
			::FreeLibrary(m_hDll);
	#ifndef	_LINUXTRYOFF
		}
		catch( std::exception &e )
		{
			printf("%s-%d处理请求%d-%d发生异常[%s]\n", __FILE__, __LINE__, e.what());
		}
		catch (...)
		{
		}
	#endif
		m_hDll = NULL;
	}

#else

	if ( m_lpDll != NULL )
	{
		dlclose(m_lpDll);
		m_lpDll = NULL;
	}

#endif
}

void* Dll::GetDllHand()
{
#ifndef LINUXCODE
	return reinterpret_cast< void * >(m_hDll);
#else
	return m_lpDll;
#endif

}




