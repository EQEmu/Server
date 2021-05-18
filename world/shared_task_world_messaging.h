#ifndef EQEMU_SHARED_TASK_WORLD_MESSAGING_H
#define EQEMU_SHARED_TASK_WORLD_MESSAGING_H

class ServerPacket;

class SharedTaskWorldMessaging {
public:
	static void HandleZoneMessage(ServerPacket* pack);
};


#endif //EQEMU_SHARED_TASK_WORLD_MESSAGING_H
