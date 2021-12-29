#include "../client.h"

void command_killallnpcs(Client *c, const Seperator *sep)
{
	std::string search_string;
	if (sep->arg[1]) {
		search_string = str_tolower(sep->arg[1]);
	}

	int killed_count = 0;
	for (auto& npc_entity : entity_list.GetNPCList()) {
		auto entity_id = npc_entity.first;
		if (!entity_id) {
			continue;
		}

		auto npc = npc_entity.second;
		if (!npc) {
			continue;
		}
		
		std::string entity_name = str_tolower(npc->GetName());
		if (
			(
				!search_string.empty() &&
				entity_name.find(search_string) == std::string::npos
			) ||
			!npc->IsAttackAllowed(c)
		) {
			continue;
		}

		npc->Damage(
			c,
			npc->GetHP(),
			SPELL_UNKNOWN,
			EQ::skills::SkillDragonPunch
		);

		killed_count++;
	}

	if (killed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"Killed {} NPC{}{}.",
				killed_count,
				killed_count != 1 ? "s" : "",
				(
					!search_string.empty() ?
					fmt::format(
						" that matched '{}'",
						search_string
					) :
					""
				)
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"There were no NPCs to kill{}.",
				(
					!search_string.empty() ?
					fmt::format(
						" that matched '{}'",
						search_string
					) :
					""
				)
			).c_str()
		);
	}
}
