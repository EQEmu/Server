#include <sol.hpp>
#include "../entity.h"
#include "../client.h"
#include "../npc.h"
#include "../corpse.h"
#include "../doors.h"
#include "../object.h"

void lua_register_entity(sol::state *state) {
	state->new_usertype<Entity>("Entity",
		"IsClient", &Entity::IsClient,
		"IsNPC", &Entity::IsNPC,
		"IsMob", &Entity::IsMob,
		"IsMerc", &Entity::IsMerc,
		"IsCorpse", &Entity::IsCorpse,
		"IsPlayerCorpse", &Entity::IsPlayerCorpse,
		"IsNPCCorpse", &Entity::IsNPCCorpse,
		"IsObject", &Entity::IsObject,
		"IsDoor", &Entity::IsDoor,
		"IsTrap", &Entity::IsTrap,
		"IsBeacon", &Entity::IsBeacon,
		"IsEncounter", &Entity::IsEncounter,
		"IsBot", &Entity::IsBot,
		"IsAura", &Entity::IsAura,
		"CastToClient", (Client*(Entity::*)())&Entity::CastToClient,
		"CastToMob", (Mob*(Entity::*)())&Entity::CastToMob,
		//"CastToMerc", (Merc*(Entity::*)())&Entity::CastToMerc,
		"CastToCorpse", (Corpse*(Entity::*)())&Entity::CastToCorpse,
		"CastToObject", (Object*(Entity::*)())&Entity::CastToObject,
		"CastToDoors", (Doors*(Entity::*)())&Entity::CastToDoors,
		//"CastToTrap", (Trap*(Entity::*)())&Entity::CastToTrap,
		//"CastToBeacon", (Beacon*(Entity::*)())&Entity::CastToBeacon,
		//"CastToEncounter", (Encounter*(Entity::*)())&Entity::CastToEncounter,
		"GetID", &Entity::GetID,
		"GetName", &Entity::GetName
	);
}
