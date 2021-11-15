#include "../client.h"

void command_repop(Client *c, const Seperator *sep)
{
	int timearg = 1;
	int delay   = 0;

	if (sep->arg[1] && strcasecmp(sep->arg[1], "force") == 0) {
		timearg++;

		LinkedListIterator<Spawn2 *> iterator(zone->spawn2_list);
		iterator.Reset();
		while (iterator.MoreElements()) {
			std::string query   = StringFormat(
				"DELETE FROM respawn_times WHERE id = %lu AND instance_id = %lu",
				(unsigned long) iterator.GetData()->GetID(),
				(unsigned long) zone->GetInstanceID()
			);
			auto        results = database.QueryDatabase(query);
			iterator.Advance();
		}
		c->Message(Chat::White, "Zone depop: Force resetting spawn timers.");
	}

	if (!sep->IsNumber(timearg)) {
		c->Message(Chat::White, "Zone depopped - repopping now.");

		zone->Repop();

		/* Force a spawn2 timer trigger so we don't delay actually spawning the NPC's */
		zone->spawn2_timer.Trigger();
		return;
	}

	c->Message(Chat::White, "Zone depoped. Repop in %i seconds", atoi(sep->arg[timearg]));
	zone->Repop(atoi(sep->arg[timearg]) * 1000);

	zone->spawn2_timer.Trigger();
}

