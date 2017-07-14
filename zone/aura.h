#ifndef AURA_H
#define AURA_H

#include <functional>
#include <set>

#include "mob.h"
#include "npc.h"
#include "../common/types.h"
#include "../common/timer.h"

class Group;
class Raid;
class Mob;
struct NPCType;

enum class AuraType {
	OnAllFriendlies,		// AE PC/Pet basically (ex. Circle of Power)
	OnAllGroupMembers,		// Normal buffing aura (ex. Champion's Aura)
	OnGroupMembersPets,		// Hits just pets (ex. Rathe's Strength)
	Totem,					// Starts pulsing on a timer when an enemy enters (ex. Idol of Malos)
	EnterTrap,				// Casts once when an enemy enters (ex. Fire Rune)
	ExitTrap,				// Casts when they start to flee (ex. Poison Spikes Trap)
	FullyScripted,			// We just call script function not a predefined
	Max
};

enum class AuraSpawns {
	GroupMembers,			// most auras use this
	Everyone,				// this is like traps and clickies who cast on everyone
	Noone,					// custom!
	Max
};

enum class AuraMovement {
	Follow,					// follows caster
	Stationary,
	Pathing,				// some sorted pathing TODO: implement
	Max
};

class Aura : public NPC
{
public:
	Aura(NPCType *type_data, Mob *owner, AuraRecord &record);
	~Aura() { };

	bool IsAura() const { return true; }
	bool Process();
	void Depop(bool unused = false);
	Mob *GetOwner();

	void ProcessOnAllFriendlies(Mob *owner);
	void ProcessOnAllGroupMembers(Mob *owner);
	void ProcessOnGroupMembersPets(Mob *owner);
	void ProcessTotem(Mob *owner);
	void ProcessEnterTrap(Mob *owner);
	void ProcessExitTrap(Mob *owner);

private:
	int m_owner;
	int spell_id; // spell we cast
	int distance; // distance we remove
	Timer remove_timer; // when we depop
	Timer process_timer; // rate limit process calls
	Timer cast_timer; // some auras pulse
	Timer movement_timer; // rate limit movement updates
	AuraType type;
	AuraSpawns spawn_type;
	AuraMovement movement_type;

	std::function<void(Aura &, Mob *)> process_func;
	std::set<int> casted_on; // we keep track of the other entities we've casted on
};

#endif /* !AURA_H */

