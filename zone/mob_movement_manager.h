#pragma once
#include <memory>

class Mob;
class Client;

class MobMovementManager
{
public:
	~MobMovementManager();
	void Process();
	void AddMob(Mob *m);
	void RemoveMob(Mob *m);
	void AddClient(Client *c);
	void RemoveClient(Client *c);

	void SendPosition(Mob *who);
	void SendPositionUpdate(Mob *who, bool send_to_self);
	void NavigateTo(Mob *who, float x, float y, float z, float speed);
	void StopNavigation(Mob *who);

	static MobMovementManager &Get() {
		static MobMovementManager inst;
		return inst;
	}

private:
	MobMovementManager();
	MobMovementManager(const MobMovementManager&);
	MobMovementManager& operator=(const MobMovementManager&);

	bool HeadingEqual(float a, float b);
	void SendUpdateTo(Mob *who, Client *c, int anim, float heading);
	void SendUpdate(Mob *who, int anim, float heading);
	void SendUpdateShortDistance(Mob *who, int anim, float heading);
	void SendUpdateLongDistance(Mob *who, int anim, float heading);
	void ProcessMovement(Mob *who, float x, float y, float z, float speed);

	struct Implementation;
	std::unique_ptr<Implementation> _impl;
};
