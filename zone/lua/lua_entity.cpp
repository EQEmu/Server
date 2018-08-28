#include <sol.hpp>
#include "../entity.h"
#include "../client.h"
#include "../npc.h"
#include "../corpse.h"
#include "../doors.h"
#include "../object.h"
#include "../trap.h"
#include "../encounter.h"
#include "../beacon.h"

void lua_register_entity(sol::state *state) {
	auto entity = state->create_simple_usertype<Entity>();
	entity.set("IsClient", &Entity::IsClient);
	entity.set("IsNPC", &Entity::IsNPC);
	entity.set("IsMob", &Entity::IsMob);
	entity.set("IsMerc", &Entity::IsMerc);
	entity.set("IsCorpse", &Entity::IsCorpse);
	entity.set("IsPlayerCorpse", &Entity::IsPlayerCorpse);
	entity.set("IsNPCCorpse", &Entity::IsNPCCorpse);
	entity.set("IsObject", &Entity::IsObject);
	entity.set("IsDoor", &Entity::IsDoor);
	entity.set("IsTrap", &Entity::IsTrap);
	entity.set("IsBeacon", &Entity::IsBeacon);
	entity.set("IsEncounter", &Entity::IsEncounter);
	entity.set("IsBot", &Entity::IsBot);
	entity.set("IsAura", &Entity::IsAura);
	entity.set("CastToClient", (Client*(Entity::*)())&Entity::CastToClient);
	entity.set("CastToMob", (Mob*(Entity::*)())&Entity::CastToMob);
	entity.set("CastToMerc", (Merc*(Entity::*)())&Entity::CastToMerc);
	entity.set("CastToCorpse", (Corpse*(Entity::*)())&Entity::CastToCorpse);
	entity.set("CastToObject", (Object*(Entity::*)())&Entity::CastToObject);
	entity.set("CastToDoors", (Doors*(Entity::*)())&Entity::CastToDoors);
	entity.set("CastToTrap", (Trap*(Entity::*)())&Entity::CastToTrap);
	entity.set("CastToBeacon", (Beacon*(Entity::*)())&Entity::CastToBeacon);
	entity.set("CastToEncounter", (Encounter*(Entity::*)())&Entity::CastToEncounter);
	entity.set("GetID", &Entity::GetID);
	entity.set("GetName", &Entity::GetName);
	state->set_usertype("Entity", entity);
}
