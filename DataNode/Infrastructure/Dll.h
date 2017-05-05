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
	//װ�ض�̬���ӿ�
	// modify by yuanjj for dllmain call 2014-03-12
	int  LoadDll(std::string strFileName, void *hModule = NULL, BOOL bdllmain = TRUE );
	// modify end
	//��ȡ����ָ��
	void * GetDllFunction(std::string strFunctionName);
	//жժ��̬���ӿ�
	void CloseDll(void);
	void * GetDllHand();
#ifdef LINUXCODE
	char	*GetDllSelfPath();
private:
	void	MergeDllSelfPath(const char *, void *);
#endif
};


#endif




