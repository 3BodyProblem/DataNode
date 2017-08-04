#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <algorithm>
#include <winsock2.h>
#include "DataNode.h"
#include "DataServer/SvrConfig.h"
#include "MemoryDB/MemoryDatabase.h"
#include "UnitTest/EchoDataCluster.h"
#pragma comment( lib, "ws2_32.lib" )


/**
 * @class				InitializeEnvironment
 * @brief				���л�����ʼ����
						(socket)
 * @author				barry
 */
class InitializeEnvironment
{
public:
	InitializeEnvironment()
	{
		WSADATA wsd;
		//��ʼ��WinSock2.2
		if( WSAStartup( MAKEWORD(2,2),&wsd ) != 0 )
		{
			::printf( "[FETAL ERROR] InitializeEnvironment::InitializeEnvironment() : failed 2 initialize socket environment... \n" );
		}
	}

	~InitializeEnvironment()
	{
		WSACleanup();
	}
};


/**
 * @brief				���е���ģʽ����
 * @author				barry
 */
void RunDebugFunction( int argc, _TCHAR* argv[] )
{
	int						nErrorCode = -1;
	std::string				sCmd = argv[1];

	std::transform( sCmd.begin(), sCmd.end(), sCmd.begin(), ::tolower );				///< ʵ�ִ�Сд������

	if( sCmd == "help" )
	{
		::printf( "��Ԫ��������:\na) ��������.exe testdb\t�������ݿ����ĵ�Ԫ���ԡ�\nb) ��������.exe testself\t��ģ��ĵ�Ԫ���ԡ�\nc) ��������.exe echocluster [TableID] [Code]\t���ÿͻ��˲�����������\n" );
	}
	else if( sCmd == "testdb" )
	{
		DatabaseIO	oDBPlugin;

		::printf( "--------------- [TestDbPlugin] --------------------\n" );
		if( 0 != (nErrorCode=Configuration::GetConfigObj().Load()) )
		{
			::printf( "failed 2 load configuration\n" );
		}

		oDBPlugin.UnitTest();
		::printf( "--------------- [DONE!] ---------------------------\n" );
	}
	else if( sCmd == "testself" )
	{
		::printf( "--------------- [UnitTest] ------------------------\n" );
		ExecuteUnitTest();
		::printf( "--------------- [DONE!] ---------------------------\n" );
	}
	else if( sCmd == "echocluster" )
	{
		::printf( "--------------- [Echo Data] ------------------------\n" );
		int					nTableID = -1;				///< ���ݱ�ID(MessageID),		[-1��ʾ��������]
		std::string			sRecordKey;					///< ��¼�����ַ���(������),	[""��ʾ��������]
		DataClusterPlugin	objDataCluster;				///< ��·�������ģ��������

		if( argc > 2 )
			nTableID = ::atoi( argv[2] );
		if( argc > 3 )
			sRecordKey = argv[3];

		objDataCluster.TestQuotationEcho( nTableID, sRecordKey );
		::printf( "--------------- [DONE!] ---------------------------\n" );
	}
}


///< �������
int _tmain( int argc, _TCHAR* argv[] )
{
	InitializeEnvironment		objInitEnv;					///< ��ʼ�����л���
	int							nErrorCode = 0;				///< �����ķ�����ֵ

	if( argc > 1 )
	{
		///< --------------------- ��Ԫ���Դ��� (����argc ���� 1) -------------------
		::printf( "--------------- [Debug] -------------------------\n" );
		RunDebugFunction( argc, argv );
		::printf( "-------------------------------------------------\n" );
	}
	else
	{
		///< ------------------------ �������з������ ------------------------------
		::printf( "--------------- [Service] -------------------------\n" );
		int		nErrorCode = RunNodeServer();
		::printf( "--------------- [DONE!] errorcode=%d --------------\n", nErrorCode );
	}

	return nErrorCode;
}




