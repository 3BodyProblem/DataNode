#ifndef __QLX_DATA_CLUSTER_TEST_H__
#define __QLX_DATA_CLUSTER_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class				DataClusterPlugin
 * @brief				�����ռ��ͻ��˲��Է�װ
 * @author				barry
 */
class DataClusterPlugin
{
public:
	DataClusterPlugin();

public:
	int					TestQuotationEcho();

protected:
	Dll					m_oDllPlugin;					///< ���������
};




#endif





