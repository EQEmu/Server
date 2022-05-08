#include "../client.h"

void command_reloadtraps(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		entity_list.UpdateAllTraps(true, true);
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Traps reloaded for {}{}.",
				fmt::format(
					"{} ({})",
					zone->GetLongName(),
					zone->GetZoneID()
				),
				(
					zone->GetInstanceID() ?
					fmt::format(
						" (Instance ID {})",
						zone->GetInstanceID()
					) :
					""
				)
			).c_str()
		);
		return;
	}

	bool global = false;

	if (sep->IsNumber(1)) {
		global = std::stoi(sep->arg[1]) ? true : false;
	}

	if (!global) {
		entity_list.UpdateAllTraps(true, true);
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Traps reloaded for {}{}.",
				fmt::format(
					"{} ({})",
					zone->GetLongName(),
					zone->GetZoneID()
				),
				(
					zone->GetInstanceID() ?
					fmt::format(
						" (Instance ID {})",
						zone->GetInstanceID()
					) :
					""
				)
			).c_str()
		);
		return;
	}

	c->Message(Chat::White, "Attempting to reload traps globally.");
	auto pack = new ServerPacket(ServerOP_ReloadTraps, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

