#include "../client.h"

void command_qglobal(Client *c, const Seperator *sep)
{
	//In-game switch for qglobal column
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Syntax: #qglobal [on/off/view]. Requires NPC target.");
		return;
	}

	Mob *target = c->GetTarget();

	if (!target || !target->IsNPC()) {
		c->Message(Chat::Red, "NPC Target Required!");
		return;
	}

	if (!strcasecmp(sep->arg[1], "on")) {
		std::string query   = StringFormat(
			"UPDATE npc_types SET qglobal = 1 WHERE id = '%i'",
			target->GetNPCTypeID());
		auto        results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::Yellow, "Could not update database.");
			return;
		}

		c->Message(Chat::Yellow, "Success! Changes take effect on zone reboot.");
		return;
	}

	if (!strcasecmp(sep->arg[1], "off")) {
		std::string query   = StringFormat(
			"UPDATE npc_types SET qglobal = 0 WHERE id = '%i'",
			target->GetNPCTypeID());
		auto        results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::Yellow, "Could not update database.");
			return;
		}

		c->Message(Chat::Yellow, "Success! Changes take effect on zone reboot.");
		return;
	}

	if (!strcasecmp(sep->arg[1], "view")) {
		const NPCType *type = content_db.LoadNPCTypesData(target->GetNPCTypeID());
		if (!type) {
			c->Message(Chat::Yellow, "Invalid NPC type.");
		}
		else if (type->qglobal) {
			c->Message(Chat::Yellow, "This NPC has quest globals active.");
		}
		else {
			c->Message(Chat::Yellow, "This NPC has quest globals disabled.");
		}
		return;
	}

	c->Message(Chat::Yellow, "Invalid action specified.");
}

