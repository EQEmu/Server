#include "../client.h"

void command_npcspawn(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update|Clone|Help] [optional 3rd parameter]");
		return;
	}

	if (!strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update|Clone|Help] [optional 3rd parameter]");
		c->Message(Chat::White, "Add: Using the same targeted NPC ID, creates new spawn2 and spawngroup entries [3rd parameter: respawntime]");
		c->Message(Chat::White, "Create: Creates new NPC type copying the data from the targeted NPC, with new spawn2 and spawngroup entries [3rd parameter: respawntime]");
		c->Message(Chat::White, "Delete: Deletes the spawn2, spawngroup, spawnentry and npc_types rows for targeted NPC");
		c->Message(Chat::White, "Remove: Deletes the spawn2, spawngroup and spawnentry rows for targeted NPC");
		c->Message(Chat::White, "Update: Updates NPC appearance in database");
		c->Message(Chat::White, "Clone: Copies targeted NPC and spawngroup, creating only a spawn2 entry [3rd parameter: respawntime]");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();
	uint32 extra = 0;
	bool is_add = !strcasecmp(sep->arg[1], "add");
	bool is_create = !strcasecmp(sep->arg[1], "create");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	bool is_update = !strcasecmp(sep->arg[1], "update");
	bool is_clone = !strcasecmp(sep->arg[1], "clone");
	if (
		!is_add &&
		!is_create &&
		!is_delete &&
		!is_remove &&
		!is_update &&
		!is_clone
	) {
		c->Message(Chat::White, "Command Syntax: #npcspawn [Add|Create|Delete|Remove|Update|Clone|Help] [optional 3rd parameter]");
		return;
	}

	if (is_add || is_create || is_clone) {
		extra = sep->IsNumber(2) ? Strings::ToInt(sep->arg[2]) : 1200; // Currently extra is only used for respawn time in Add/Create/Clone, default to 1200 if not set

		content_db.NPCSpawnDB(
			is_add ? NPCSpawnTypes::AddNewSpawngroup : (is_create ? NPCSpawnTypes::CreateNewSpawn : NPCSpawnTypes::AddSpawnFromSpawngroup),
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
				is_add ?
				"Added" :
				(
					is_create ?
					"Created" :
					"Cloned"
				),
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

