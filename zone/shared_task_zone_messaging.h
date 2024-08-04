#ifndef EQEMU_SHARED_TASK_ZONE_MESSAGING_H
#define EQEMU_SHARED_TASK_ZONE_MESSAGING_H

class ServerPacket;

class SharedTaskZoneMessaging {
public:
	static void HandleWorldMessage(ServerPacket *pack);
};


#endif //EQEMU_SHARED_TASK_ZONE_MESSAGING_H
