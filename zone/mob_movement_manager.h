#pragma once
#include <memory>

class Mob;
class Client;

struct RotateCommand;
struct PlayerPositionUpdateServer_Struct;

class MobMovementManager
{
public:
	~MobMovementManager();
	void Process();
	void AddMob(Mob *m);
	void RemoveMob(Mob *m);
	void AddClient(Client *c);
	void RemoveClient(Client *c);

	void RotateTo(Mob *who, float to, float speed);
	void Teleport(Mob *who, float x, float y, float z, float heading);
	void NavigateTo(Mob *who, float x, float y, float z, float speed);
	void StopNavigation(Mob *who);
	//void Dump(Mob *m, Client *to);
	//void DumpStats(Client *to);
	//void ClearStats();

	static MobMovementManager &Get() {
		static MobMovementManager inst;
		return inst;
	}

private:
	MobMovementManager();
	MobMovementManager(const MobMovementManager&);
	MobMovementManager& operator=(const MobMovementManager&);

	void ProcessRotateCommand(Mob *m, RotateCommand &cmd);
	void SendCommandToAllClients(Mob *m, float dx, float dy, float dz, float dh, int anim);
	void FillCommandStruct(PlayerPositionUpdateServer_Struct *spu, Mob *m, float dx, float dy, float dz, float dh, int anim);
	float FixHeading(float in);

	struct Implementation;
	std::unique_ptr<Implementation> _impl;
};
