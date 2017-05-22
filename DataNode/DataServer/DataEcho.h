#ifndef __DATA_NODE_DATA_ECHO_H__
#define __DATA_NODE_DATA_ECHO_H__


#include <map>
#include <string>
#include <algorithm>
#include "../Infrastructure/Lock.h"
#include "../Protocal/CTP_DL_QuoProtocal.h"


std::string&			Str2Lower( std::string& sStr );
bool					SplitString( char** pArgv, int& nArgc, const char* pszStr );


/**
 * @class				IDataEcho
 * @brief				行情命令回显接口
 * @date				2017/5/19
 * @author				barry
 */ 
class IDataEcho
{
public:

};



#endif






