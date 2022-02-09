#include "../client.h"
#include "../groups.h"

void command_roambox(Client *c, const Seperator *sep)
{
	std::string arg1 = sep->arg[1];

	Mob *target = c->GetTarget();
	if (!target || !target->IsNPC()) {
		c->Message(Chat::Red, "You need a valid NPC target for this command");
		return;
	}

	NPC *npc           = dynamic_cast<NPC *>(target);
	int spawn_group_id = npc->GetSpawnGroupId();
	if (spawn_group_id <= 0) {
		c->Message(Chat::Red, "NPC needs a valid SpawnGroup!");
		return;
	}

	if (arg1 == "set") {
		int box_size = (sep->arg[2] ? atoi(sep->arg[2]) : 0);
		int delay    = (sep->arg[3] ? atoi(sep->arg[3]) : 15000);
		if (box_size > 0) {
			std::string               query = fmt::format(
				SQL(
					UPDATE spawngroup SET
					dist         = {},
					min_x        = {},
					max_x        = {},
					min_y        = {},
					max_y        = {},
					delay        = {}
						WHERE id = {}
				),
				(box_size / 2),
				npc->GetX() - (box_size / 2),
				npc->GetX() + (box_size / 2),
				npc->GetY() - (box_size / 2),
				npc->GetY() + (box_size / 2),
				delay,
				spawn_group_id
			);

			database.QueryDatabase(query);

			c->Message(
				Chat::Yellow,
				"NPC (%s) Roam Box set to box size of [%i] SpawnGroupId [%i] delay [%i]",
				npc->GetCleanName(),
				box_size,
				spawn_group_id,
				delay
			);

			return;
		}

		c->Message(Chat::Red, "Box size must be set!");
	}

	if (arg1 == "remove") {
		std::string query = fmt::format(
			SQL(
				UPDATE spawngroup SET
				dist = 0,
				min_x = 0,
				max_x = 0,
				min_y = 0,
				max_y = 0,
				delay = 0
					WHERE id = {}
			),
			spawn_group_id
		);

		database.QueryDatabase(query);

		c->Message(
			Chat::Yellow,
			"NPC (%s) Roam Box has been removed from SpawnGroupID [%i]",
			npc->GetCleanName(),
			spawn_group_id
		);

		return;
	}

	c->Message(Chat::Yellow, "> Command Usage");
	c->Message(Chat::Yellow, "#roambox set box_size [delay = 0]");
	c->Message(Chat::Yellow, "#roambox remove");
}

