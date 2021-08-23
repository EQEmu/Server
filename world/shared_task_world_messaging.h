#ifndef EQEMU_SHARED_TASK_WORLD_MESSAGING_H
#define EQEMU_SHARED_TASK_WORLD_MESSAGING_H

#include "../common/types.h"
#include "../common/servertalk.h"
#include "../common/shared_tasks.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/tasks_repository.h"
#include "../common/tasks.h"

class SharedTaskWorldMessaging {
public:
	static void HandleZoneMessage(ServerPacket *pack);
};


#endif //EQEMU_SHARED_TASK_WORLD_MESSAGING_H

