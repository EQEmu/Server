#pragma once
#include <memory>

class Mob;
class Client;

struct RotateCommand;
struct MovementCommand;
struct MobMovementEntry;
struct PlayerPositionUpdateServer_Struct;

enum ClientRange : int
{
	ClientRangeNone = 0,
	ClientRangeClose = 1,
	ClientRangeMedium = 2,
	ClientRangeCloseMedium = 3,
	ClientRangeLong = 4,
	ClientRangeCloseLong = 5,
	ClientRangeMediumLong = 6,
	ClientRangeAny = 7
};

enum MobMovementMode : int
{
	MovementWalking = 0,
	MovementRunning = 1
};

enum MobStuckBehavior : int
{
	RunToTarget,
	WarpToTarget,
	TakeNoAction,
	EvadeCombat,
	MaxStuckBehavior
};

class MobMovementManager
{
public:
	~MobMovementManager();
	void Process();
	void AddMob(Mob *m);
	void RemoveMob(Mob *m);
	void AddClient(Client *c);
	void RemoveClient(Client *c);

	void RotateTo(Mob *who, float to, MobMovementMode mode = MovementRunning);
	void Teleport(Mob *who, float x, float y, float z, float heading);
	void NavigateTo(Mob *who, float x, float y, float z, MobMovementMode mode = MovementRunning);
	void StopNavigation(Mob *who);
	void SendCommandToClients(Mob *m, float dx, float dy, float dz, float dh, int anim, ClientRange range);
	float FixHeading(float in);
	void DumpStats(Client *to);
	void ClearStats();

	static MobMovementManager &Get() {
		static MobMovementManager inst;
		return inst;
	}

private:
	MobMovementManager();
	MobMovementManager(const MobMovementManager&);
	MobMovementManager& operator=(const MobMovementManager&);

	void FillCommandStruct(PlayerPositionUpdateServer_Struct *spu, Mob *m, float dx, float dy, float dz, float dh, int anim);
	void UpdatePath(Mob *who, float x, float y, float z, MobMovementMode mode);
	void UpdatePathGround(Mob *who, float x, float y, float z, MobMovementMode mode);
	void UpdatePathUnderwater(Mob *who, float x, float y, float z, MobMovementMode mode);
	void UpdatePathBoat(Mob *who, float x, float y, float z, MobMovementMode mode);
	void PushTeleportTo(MobMovementEntry &ent, float x, float y, float z, float heading);
	void PushMoveTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mode);
	void PushSwimTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mode);
	void PushRotateTo(MobMovementEntry &ent, Mob *who, float to, MobMovementMode mode);
	void PushStopMoving(MobMovementEntry &ent);
	void PushEvadeCombat(MobMovementEntry &ent);
	void HandleStuckBehavior(Mob *who, float x, float y, float z, MobMovementMode mode);

	struct Implementation;
	std::unique_ptr<Implementation> _impl;
};
