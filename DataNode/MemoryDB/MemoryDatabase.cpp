#include <stdio.h>
#include "MemoryDatabase.h"


DatabaseManager::DatabaseManager()
: m_pIDBFactoryPtr( NULL )
{
}

DatabaseManager& DatabaseManager::GetMgrObject()
{
	static DatabaseManager		obj;

	return obj;
}












