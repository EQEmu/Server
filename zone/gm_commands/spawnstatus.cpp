#include "../client.h"

void command_spawnstatus(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #spawnstatus all - Show all spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus disabled - Show all disabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus enabled - Show all enabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus [Spawn ID] - Show spawn status by ID for your current zone");
		return;
	}

	bool is_all = !strcasecmp(sep->arg[1], "all");
	bool is_disabled = !strcasecmp(sep->arg[1], "disabled");
	bool is_enabled = !strcasecmp(sep->arg[1], "enabled");

	if (
		!is_all &&
		!is_disabled &&
		!is_enabled &&
		!sep->IsNumber(1)
	) {
		c->Message(Chat::White, "Usage: #spawnstatus all - Show all spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus disabled - Show all disabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus enabled - Show all enabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus [Spawn ID] - Show spawn status by ID for your current zone");
		return;
	}

	if (is_all) {
		zone->SpawnStatus(c);
	} else if (is_disabled) {
		zone->SpawnStatus(c, "Disabled");
	} else if (is_enabled) {
		zone->SpawnStatus(c, "Enabled");
	} else if (sep->IsNumber(1)) {
		auto spawn_id = std::stoul(sep->arg[1]);
		zone->ShowSpawnStatusByID(c, spawn_id);
	}
}

