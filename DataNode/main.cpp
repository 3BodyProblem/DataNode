#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include "DataNode.h"


///< ³ÌÐòÈë¿Ú
int _tmain( int argc, _TCHAR* argv[] )
{
	int		nErrorCode = 0;

	if( argc <= 1 )
	{
		::printf( "--------------- [Service] ------------------------\n" );
		nErrorCode = RunNodeServer();
		::printf( "--------------- [DONE! %d] ------------------------\n", nErrorCode );
	}
	else
	{
		::printf( "--------------- [UnitTest] ------------------------\n" );
		ExecuteUnitTest();
		::printf( "--------------- [DONE!] ---------------------------\n" );
	}

	return nErrorCode;
}




