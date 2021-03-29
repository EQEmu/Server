#ifndef EQEMU_EVENT_SCHEDULER_H
#define EQEMU_EVENT_SCHEDULER_H

#include "../common/server_event_scheduler.h"
#include "zonelist.h"

class WorldEventScheduler : public ServerEventScheduler {
public:
	void Process(ZSList *zs_list);
};

#endif //EQEMU_EVENT_SCHEDULER_H
