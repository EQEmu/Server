#ifndef EQEMU_ZONE_EVENT_SCHEDULER_H
#define EQEMU_ZONE_EVENT_SCHEDULER_H

#include "../common/server_event_scheduler.h"
#include "zone.h"
#include "../common/content/world_content_service.h"

class ZoneEventScheduler : public ServerEventScheduler {
public:
	void Process(Zone *zone, WorldContentService *content_service);
	void SyncEventDataWithActiveEvents();
};

#endif //EQEMU_ZONE_EVENT_SCHEDULER_H
