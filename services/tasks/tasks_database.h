#pragma once

#include "../../common/global_define.h"
#include "../../common/types.h"
#include "../../common/database.h"

class TasksDatabase : public Database {
public:
	TasksDatabase();
	TasksDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port);
	~TasksDatabase();
};
