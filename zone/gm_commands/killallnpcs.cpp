#include "../client.h"

void command_killallnpcs(Client *c, const Seperator *sep)
{
	std::string search_string;
	if (sep->arg[1]) {
		search_string = sep->arg[1];
	}

	int       count = 0;
	for (auto &itr : entity_list.GetMobList()) {
		Mob *entity = itr.second;
		if (!entity->IsNPC()) {
			continue;
		}

		std::string entity_name = entity->GetName();

		/**
		 * Filter by name
		 */
		if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
			continue;
		}

		bool is_not_attackable =
				 (
					 entity->IsInvisible() ||
					 !entity->IsAttackAllowed(c) ||
					 entity->GetRace() == 127 ||
					 entity->GetRace() == 240
				 );

		if (is_not_attackable) {
			continue;
		}

		entity->Damage(c, 1000000000, 0, EQ::skills::SkillDragonPunch);

		count++;
	}

	c->Message(Chat::Yellow, "Killed (%i) npc(s)", count);
}

