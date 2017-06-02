#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include "DataNode.h"
#include "DataServer/SvrConfig.h"
#include "MemoryDB/MemoryDatabase.h"


///< �������
int _tmain( int argc, _TCHAR* argv[] )
{
	int		nErrorCode = 0;

	///< ��Ԫ���Դ��� (����argc ���� 1)
	if( argc > 1 )
	{
		if( 0 == ::strncmp( argv[1], "testdb", 6 ) )
		{
			DatabaseIO	oDBPlugin;

			if( 0 != (nErrorCode=Configuration::GetConfigObj().Load()) )
			{
				::printf( "failed 2 load configuration\n" );
				return nErrorCode;
			}

			oDBPlugin.UnitTest();

			return nErrorCode;
		}

		if( 0 == ::strncmp( argv[1], "testself", 8 ) )
		{
			::printf( "--------------- [UnitTest] ------------------------\n" );
			ExecuteUnitTest();
			::printf( "--------------- [DONE!] ---------------------------\n" );

			return nErrorCode;
		}

		::printf( "��Ԫ��������:\na) ��������.exe testdb\t�������ݿ����ĵ�Ԫ���ԡ�\nb) ��������.exe testself\t��ģ��ĵ�Ԫ���ԡ�\n" );

		return 0;
	}

	///< �������з������
	::printf( "--------------- [Service] ------------------------\n" );
	nErrorCode = RunNodeServer();
	::printf( "--------------- [DONE! %d] ------------------------\n", nErrorCode );

	return nErrorCode;
}




