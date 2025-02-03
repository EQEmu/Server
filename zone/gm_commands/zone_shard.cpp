#include "../client.h"

void command_zone_shard(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		if (!RuleB(Zone, ZoneShardQuestMenuOnly)) {
			c->ShowZoneShardMenu();
		}

		return;
	}

	if (c->GetAggroCount() > 0) {
		c->Message(Chat::White, "You cannot request a shard change while in combat.");
		return;
	}

	std::string zone_input = sep->arg[1];
	uint32      zone_id    = 0;

	// if input is id
	if (Strings::IsNumber(zone_input)) {
		zone_id = Strings::ToInt(zone_input);

		// validate
		if (zone_id != 0 && !GetZone(zone_id)) {
			c->Message(Chat::White, fmt::format("Could not find zone by id [{}]", zone_id).c_str());
			return;
		}
	}
	else {
		// validate
		if (!zone_store.GetZone(zone_input)) {
			c->Message(Chat::White, fmt::format("Could not find zone by short_name [{}]", zone_input).c_str());
			return;
		}

		// validate we got id
		zone_id = ZoneID(zone_input);
		if (zone_id == 0) {
			c->Message(Chat::White, fmt::format("Could not find zone id by short_name [{}]", zone_input).c_str());
			return;
		}
	}

	bool char_in_hub = c->IsHubZone(c->GetZoneID());
	bool tar_is_hub = c->IsHubZone(zone_id);

	auto z = GetZone(zone_id);
	if (z && z->shard_at_player_count == 0 && !tar_is_hub) {
		c->Message(Chat::White, "Zone does not have sharding enabled.");
		return;
	}

	auto instance_id = sep->arg[2] ? Strings::ToBigInt(sep->arg[2]) : 0;
	if (instance_id < -1) {
		c->Message(Chat::White, "You must enter a valid Instance ID.");
		return;
	}

	if (zone_id == c->GetZoneID() && c->GetInstanceID() == instance_id) {
		c->Message(Chat::White, "You are already in this shard.");
		return;
	}

	// Are we moving between instances of the same zone or between hubs?
	if (zone_id != c->GetZoneID() && (!tar_is_hub || !char_in_hub)) {
		c->Message(Chat::White, "You must request a shard change from the zone you are currently in.");
		return;
	}

	// Are there even instances to move between?  (Hubs may not have instances so skip this check if they're in a hub)
	auto results = CharacterDataRepository::GetInstanceZonePlayerCounts(database, c->GetZoneID());
	if (!char_in_hub && results.size() <= 1) {
		c->Message(Chat::White, "No shards found.");
		return;
	}

	LogZoning("Zoneshard move request to zone [{}] instance [{}]", zone_id, instance_id);
	if (instance_id > 0) {
		if (!database.CheckInstanceExists(instance_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} does not exist.",
					instance_id
				).c_str()
			);
			return;
		}

		auto instance_zone_id = database.GetInstanceZoneID(instance_id);
		if (!instance_zone_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} not found or zone is set to null.",
					instance_id
				).c_str()
			);
			return;
		}

		if (instance_zone_id != zone_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance Zone ID {} does not match zone ID {}.",
					instance_id,
					zone_id
				).c_str()
			);
			return;
		}

		if (!database.CheckInstanceByCharID(instance_id, c->CharacterID())) {
			database.AddClientToInstance(instance_id, c->CharacterID());
		}

		if (!database.VerifyInstanceAlive(instance_id, c->CharacterID())) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} expired.",
					instance_id
				).c_str()
			);
			return;
		}
	}

	// Moving between versions of same zone = preserve location
	if(zone_id == c->GetZoneID()) {
		c->MovePC(
			zone_id,
			instance_id,
			c->GetX(),
			c->GetY(),
			c->GetZ(),
			c->GetHeading(),
			0,
			ZoneSolicited
		);
	} else {
		switch (zone_id) {
			case Zones::ECOMMONS:
				c->MovePC(zone_id, instance_id, -356, -1603, 3, 0, 0, ZoneSolicited);
				break;
			case Zones::BAZAAR:
				c->MovePC(zone_id, instance_id, 105, -175, -15, 65, 0, ZoneSolicited);
			default: {
				c->MovePC(zone_id, instance_id, 0.0, 0.0, 0.0, 0.0, 0, ZoneToSafeCoords);
			}
		}
	}
}

void command_zone_shard_new(Client *c, const Seperator *sep)
{
	if(sep->argnum < 1) {
		c->Message(Chat::White, fmt::format("Missing zone short_name or id argument").c_str());
		return;
	}

	std::string zone_input = sep->arg[1];
	uint32      zone_id    = 0;

	// if input is id
	if (Strings::IsNumber(zone_input)) {
		zone_id = Strings::ToInt(zone_input);

		// validate
		if (zone_id != 0 && !GetZone(zone_id)) {
			c->Message(Chat::White, fmt::format("Could not find zone by id [{}]", zone_id).c_str());
			return;
		}
	}
	else {
		// validate
		if (!zone_store.GetZone(zone_input)) {
			c->Message(Chat::White, fmt::format("Could not find zone by short_name [{}]", zone_input).c_str());
			return;
		}

		// validate we got id
		zone_id = ZoneID(zone_input);
		if (zone_id == 0) {
			c->Message(Chat::White, fmt::format("Could not find zone id by short_name [{}]", zone_input).c_str());
			return;
		}
	}

	uint64_t shard_instance_duration = 3155760000;
	if(sep->argnum >= 2 && Strings::IsNumber(sep->arg[2])) {
		shard_instance_duration = Strings::ToInt(sep->arg[2]);
	}

	if(zone_id == 0) {
		c->Message(Chat::White, fmt::format("Did not find valid zone id").c_str());
		return;
	}

	auto z = GetZone(zone_id);
	if(!z) {
		c->Message(Chat::White, fmt::format("No zone found for id [{}]", zone_id).c_str());
		return;
	}

	uint16 new_instance_id = 0;
	database.GetUnusedInstanceID(new_instance_id);
	database.CreateInstance(new_instance_id, zone_id, z->version, shard_instance_duration);
	LogZoning(
		"GM [{}] creating new sharded zone > instance_id [{}] zone [{}] ({}) duration [{}]",
		c->GetCleanName(),
		new_instance_id,
		ZoneName(zone_id) ? ZoneName(zone_id) : "Unknown",
		zone_id,
		shard_instance_duration
	);
}
