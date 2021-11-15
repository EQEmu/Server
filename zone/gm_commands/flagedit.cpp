#include "../client.h"

void command_flagedit(Client *c, const Seperator *sep)
{
	//super-command for editing zone flags
	if (sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Syntax: #flagedit [lockzone|unlockzone|listzones|give|take].");
		c->Message(
			Chat::White,
			"...lockzone [zone id/short] [flag name] - Set the specified flag name on the zone, locking the zone"
		);
		c->Message(Chat::White, "...unlockzone [zone id/short] - Removes the flag requirement from the specified zone");
		c->Message(Chat::White, "...listzones - List all zones which require a flag, and their flag's name");
		c->Message(Chat::White, "...give [zone id/short] - Give your target the zone flag for the specified zone.");
		c->Message(
			Chat::White,
			"...take [zone id/short] - Take the zone flag for the specified zone away from your target"
		);
		c->Message(Chat::White, "...Note: use #flags to view flags on a person");
		return;
	}

	if (!strcasecmp(sep->arg[1], "lockzone")) {
		uint32 zoneid = 0;
		if (sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if (zoneid < 1) {
				zoneid = ZoneID(sep->arg[2]);
			}
		}
		if (zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

		char flag_name[128];
		if (sep->argplus[3][0] == '\0') {
			c->Message(Chat::Red, "flag name required. see help.");
			return;
		}
		database.DoEscapeString(flag_name, sep->argplus[3], 64);
		flag_name[127] = '\0';

		std::string query   = StringFormat(
			"UPDATE zone SET flag_needed = '%s' "
			"WHERE zoneidnumber = %d AND version = %d",
			flag_name, zoneid, zone->GetInstanceVersion());
		auto        results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::Red, "Error updating zone: %s", results.ErrorMessage().c_str());
			return;
		}

		c->Message(Chat::Yellow, "Success! Zone %s now requires a flag, named %s", ZoneName(zoneid), flag_name);
		return;
	}

	if (!strcasecmp(sep->arg[1], "unlockzone")) {
		uint32 zoneid = 0;
		if (sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if (zoneid < 1) {
				zoneid = ZoneID(sep->arg[2]);
			}
		}

		if (zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

		std::string query   = StringFormat(
			"UPDATE zone SET flag_needed = '' "
			"WHERE zoneidnumber = %d AND version = %d",
			zoneid, zone->GetInstanceVersion());
		auto        results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::Yellow, "Error updating zone: %s", results.ErrorMessage().c_str());
			return;
		}

		c->Message(Chat::Yellow, "Success! Zone %s no longer requires a flag.", ZoneName(zoneid));
		return;
	}

	if (!strcasecmp(sep->arg[1], "listzones")) {
		std::string query   = "SELECT zoneidnumber, short_name, long_name, version, flag_needed "
							  "FROM zone WHERE flag_needed != ''";
		auto        results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		c->Message(Chat::White, "Zones which require flags:");
		for (auto row = results.begin(); row != results.end(); ++row)
			c->Message(
				Chat::White,
				"Zone %s (%s,%s) version %s requires key %s",
				row[2],
				row[0],
				row[1],
				row[3],
				row[4]
			);

		return;
	}

	if (!strcasecmp(sep->arg[1], "give")) {
		uint32 zoneid = 0;
		if (sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if (zoneid < 1) {
				zoneid = ZoneID(sep->arg[2]);
			}
		}
		if (zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

		Mob *t = c->GetTarget();
		if (t == nullptr || !t->IsClient()) {
			c->Message(Chat::Red, "client target required");
			return;
		}

		t->CastToClient()->SetZoneFlag(zoneid);
		return;
	}

	if (!strcasecmp(sep->arg[1], "give")) {
		uint32 zoneid = 0;
		if (sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if (zoneid < 1) {
				zoneid = ZoneID(sep->arg[2]);
			}
		}
		if (zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

		Mob *t = c->GetTarget();
		if (t == nullptr || !t->IsClient()) {
			c->Message(Chat::Red, "client target required");
			return;
		}

		t->CastToClient()->ClearZoneFlag(zoneid);
		return;
	}

	c->Message(Chat::Yellow, "Invalid action specified. use '#flagedit help' for help");
}

