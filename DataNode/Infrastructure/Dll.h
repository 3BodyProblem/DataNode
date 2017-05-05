#ifndef __MEngine_MDllH__
#define __MEngine_MDllH__


#include "MErrorCode.h"
#include <string>


std::string GetApplicationPath(void * hModule);


class MDll
{
protected:
	#ifndef LINUXCODE
		HINSTANCE					m_hDll;
	#else
		void					*	m_lpDll;
		char						m_szDllSelfPath[MAX_PATH];
	#endif
public:
	MDll(void);
	virtual ~MDll();

public:
	//装载动态连接库
	// modify by yuanjj for dllmain call 2014-03-12
	int  LoadDll(std::string strFileName, void *hModule = NULL, BOOL bdllmain = TRUE );
	// modify end
	//获取函数指针
	void * GetDllFunction(std::string strFunctionName);
	//卸摘动态连接库
	void CloseDll(void);
	void * GetDllHand();
#ifdef LINUXCODE
	char	*GetDllSelfPath();
private:
	void	MergeDllSelfPath(const char *, void *);
#endif
};


#endif




