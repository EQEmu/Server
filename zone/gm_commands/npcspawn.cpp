#include "../client.h"

void command_npcspawn(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update]");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();
	uint32 extra = 0;
	bool is_add = !strcasecmp(sep->arg[1], "add");
	bool is_create = !strcasecmp(sep->arg[1], "create");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	bool is_update = !strcasecmp(sep->arg[1], "update");
	if (
		!is_add &&
		!is_create &&
		!is_delete &&
		!is_remove &&
		!is_update
	) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update]");
		return;
	}

	if (is_add || is_create) {
		extra = (
			sep->IsNumber(2) ?
			(
				is_add ?
				std::stoi(sep->arg[2]) :
				1
			) : (
				is_add ?
				1200 :
				0
			)
		); // Default to 1200 for Add, 0 for Create if not set

		content_db.NPCSpawnDB(
			is_add ? NPCSpawnTypes::AddNewSpawngroup : NPCSpawnTypes::CreateNewSpawn,
			zone->GetShortName(),
			zone->GetInstanceVersion(),
			c,
			target,
			extra
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn {} | Name: {}",
				is_add ? "Added" : "Created",
				c->GetTargetDescription(target)
			).c_str()
		);
	}
	else if (is_delete || is_remove || is_update) {
		uint8 spawn_update_type = (
			is_delete ?
			NPCSpawnTypes::DeleteSpawn :
			(
				is_remove ?
				NPCSpawnTypes::RemoveSpawn :
				NPCSpawnTypes::UpdateAppearance
			)
		);

		std::string spawn_message = (
			is_delete ?
			"Deleted" :
			(
				is_remove ?
				"Removed" :
				"Updated"
			)
		);

		content_db.NPCSpawnDB(
			spawn_update_type,
			zone->GetShortName(),
			zone->GetInstanceVersion(),
			c,
			target
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn {} | Name: {}",
				spawn_message,
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}

