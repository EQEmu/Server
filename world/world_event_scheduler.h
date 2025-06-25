#ifndef EQEMU_EVENT_SCHEDULER_H
#define EQEMU_EVENT_SCHEDULER_H

#include "../common/server_event_scheduler.h"
#include "zonelist.h"

class WorldEventScheduler : public ServerEventScheduler {
public:
	void Process(ZSList *zs_list);

	static WorldEventScheduler* Instance()
	{
		static WorldEventScheduler instance;
		return &instance;
	}
};

#endif //EQEMU_EVENT_SCHEDULER_H
