#include "../client.h"

void command_grid(Client *c, const Seperator *sep)
{
	auto command_type = sep->arg[1];
	auto zone_id      = zone->GetZoneID();
	if (strcasecmp("max", command_type) == 0) {
		c->Message(
			Chat::White,
			fmt::format(
				"Highest grid ID in this zone is {}.",
				content_db.GetHighestGrid(zone_id)
			).c_str()
		);
	}
	else if (strcasecmp("add", command_type) == 0) {
		auto grid_id     = atoi(sep->arg[2]);
		auto wander_type = atoi(sep->arg[3]);
		auto pause_type  = atoi(sep->arg[4]);
		if (!content_db.GridExistsInZone(zone_id, grid_id)) {
			content_db.ModifyGrid(c, false, grid_id, wander_type, pause_type, zone_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Grid {} added to zone ID {} with wander type {} and pause type {}.",
					grid_id,
					zone_id,
					wander_type,
					pause_type
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Grid {} already exists in zone ID {}.",
					grid_id,
					zone_id
				).c_str()
			);
			return;
		}
	}
	else if (strcasecmp("show", command_type) == 0) {
		Mob *target = c->GetTarget();
		if (!target || !target->IsNPC()) {
			c->Message(Chat::White, "You need to target an NPC!");
			return;
		}

		auto        grid_id = target->CastToNPC()->GetGrid();
		std::string query   = fmt::format(
			"SELECT `x`, `y`, `z`, `heading`, `number` "
			"FROM `grid_entries` "
			"WHERE `zoneid` = {} AND `gridid` = {} "
			"ORDER BY `number`",
			zone_id,
			grid_id
		);

		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Error querying database.");
			c->Message(Chat::White, query.c_str());
		}

		if (results.RowCount() == 0) {
			c->Message(Chat::White, "No grid found.");
			return;
		}

		// Depop any node npc's already spawned
		entity_list.DespawnGridNodes(grid_id);

		// Spawn grid nodes
		std::map<std::vector<float>, int32> zoffset;
		for (auto                           row : results) {
			glm::vec4          node_position = glm::vec4(atof(row[0]), atof(row[1]), atof(row[2]), atof(row[3]));
			std::vector<float> node_loc{
				node_position.x,
				node_position.y,
				node_position.z
			};

			// If we already have a node at this location, set the z offset
			// higher from the existing one so we can see it.  Adjust so if
			// there is another at the same spot we adjust again.
			auto search = zoffset.find(node_loc);
			if (search != zoffset.end()) {
				search->second = search->second + 3;
			}
			else {
				zoffset[node_loc] = 0.0;
			}

			node_position.z += zoffset[node_loc];
			NPC::SpawnGridNodeNPC(node_position, grid_id, atoi(row[4]), zoffset[node_loc]);
		}
		c->Message(
			Chat::White,
			fmt::format(
				"Spawning nodes for grid {}.",
				grid_id
			).c_str()
		);
	}
	else if (strcasecmp("hide", command_type) == 0) {
		Mob *target = c->GetTarget();
		if (!target || !target->IsNPC()) {
			c->Message(Chat::White, "You need to target an NPC!");
			return;
		}

		auto grid_id = target->CastToNPC()->GetGrid();
		entity_list.DespawnGridNodes(grid_id);
		c->Message(
			Chat::White,
			fmt::format(
				"Depawning nodes for grid {}.",
				grid_id
			).c_str()
		);
	}
	else if (strcasecmp("delete", command_type) == 0) {
		auto grid_id = atoi(sep->arg[2]);
		content_db.ModifyGrid(c, true, grid_id, 0, 0, zone_id);
		c->Message(
			Chat::White,
			fmt::format(
				"Grid {} deleted from zone ID {}.",
				grid_id,
				zone_id
			).c_str()
		);
	}
	else {
		c->Message(Chat::White, "Usage: #grid [add|delete] [grid_id] [wander_type] [pause_type]");
		c->Message(Chat::White, "Usage: #grid [max] - displays the highest grid ID used in this zone (for add)");
		c->Message(Chat::White, "Usage: #grid [show] - displays wp nodes as boxes");
	}
}

