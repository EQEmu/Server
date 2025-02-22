#ifndef CHATSERVER_DATABASE_H
#define CHATSERVER_DATABASE_H

#define AUTHENTICATION_TIMEOUT	60
#define INVALID_ID				0xFFFFFFFF

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include "../common/types.h"
#include "../common/database.h"
#include "../common/linked_list.h"
#include "../common/servertalk.h"
#include <string>
#include <vector>
#include <map>

class QSDatabase : public Database {
public:
	void GeneralQueryReceive(ServerPacket *pack);
};

#endif

