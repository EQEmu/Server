#include "tasks_database.h"

TasksDatabase::TasksDatabase()
	: Database()
{

}

TasksDatabase::TasksDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
	: Database(host, user, passwd, database, port)
{

}

TasksDatabase::~TasksDatabase() {

}
