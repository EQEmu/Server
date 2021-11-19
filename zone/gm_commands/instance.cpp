#include "../client.h"

void command_instance(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	//options:
	//help
	//create [zone_id] [version]
	//destroy [instance_id]
	//add [instance_id] [player_name]
	//remove [instance_id] [player_name]
	//list [player_name]

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "#instance usage:");
		c->Message(
			Chat::White, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
						 "zone with id matching zone_id, will last for duration seconds."
		);
		c->Message(Chat::White, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
		c->Message(
			Chat::White, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
						 "with id matching instance_id."
		);
		c->Message(
			Chat::White, "#instance remove instance_id player_name - removes the player 'player_name' from the "
						 "instance with id matching instance_id."
		);
		c->Message(Chat::White, "#instance list player_name - lists all the instances 'player_name' is apart of.");
		return;
	}
	else if (strcasecmp(sep->arg[1], "create") == 0) {
		if (!sep->IsNumber(3) || !sep->IsNumber(4)) {
			c->Message(
				Chat::White,
				"#instance create zone_id version duration - Creates an instance of version 'version' in the "
				"zone with id matching zone_id, will last for duration seconds."
			);
			return;
		}

		const char *zn     = nullptr;
		uint32     zone_id = 0;

		if (sep->IsNumber(2)) {
			zone_id = atoi(sep->arg[2]);
		}
		else {
			zone_id = ZoneID(sep->arg[2]);
		}

		uint32 version  = atoi(sep->arg[3]);
		uint32 duration = atoi(sep->arg[4]);
		zn = ZoneName(zone_id);

		if (!zn) {
			c->Message(Chat::White, "Zone with id %lu was not found by the server.", (unsigned long) zone_id);
			return;
		}

		uint16 id = 0;
		if (!database.GetUnusedInstanceID(id)) {
			c->Message(Chat::White, "Server was unable to find a free instance id.");
			return;
		}

		if (!database.CreateInstance(id, zone_id, version, duration)) {
			c->Message(Chat::White, "Server was unable to create a new instance.");
			return;
		}

		c->Message(Chat::White, "New instance %s was created with id %lu.", zn, (unsigned long) id);
	}
	else if (strcasecmp(sep->arg[1], "destroy") == 0) {
		if (!sep->IsNumber(2)) {
			c->Message(
				Chat::White,
				"#instance destroy instance_id - Destroys the instance with id matching instance_id."
			);
			return;
		}

		uint16 id = atoi(sep->arg[2]);
		database.DeleteInstance(id);
		c->Message(Chat::White, "Destroyed instance with id %lu.", (unsigned long) id);
	}
	else if (strcasecmp(sep->arg[1], "add") == 0) {
		if (!sep->IsNumber(2)) {
			c->Message(
				Chat::White, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
							 "with id matching instance_id."
			);
			return;
		}

		uint16 id     = atoi(sep->arg[2]);
		uint32 charid = database.GetCharacterID(sep->arg[3]);

		if (id <= 0 || charid <= 0) {
			c->Message(Chat::White, "Must enter a valid instance id and player name.");
			return;
		}

		if (!database.CheckInstanceExists(id)) {
			c->Message(Chat::White, "Instance does not exist.");
			return;
		}

		uint32 zone_id = database.ZoneIDFromInstanceID(id);
		uint32 version = database.VersionFromInstanceID(id);
		uint32 cur_id  = database.GetInstanceID(zone_id, charid, version);
		if (cur_id == 0) {
			if (database.AddClientToInstance(id, charid)) {
				c->Message(Chat::White, "Added client to instance.");
			}
			else {
				c->Message(Chat::White, "Failed to add client to instance.");
			}
		}
		else {
			c->Message(
				Chat::White,
				"Client was already saved to %u which has uses the same zone and version as that instance.",
				cur_id
			);
		}
	}
	else if (strcasecmp(sep->arg[1], "remove") == 0) {
		if (!sep->IsNumber(2)) {
			c->Message(
				Chat::White, "#instance remove instance_id player_name - removes the player 'player_name' from the "
							 "instance with id matching instance_id."
			);
			return;
		}

		uint16 id     = atoi(sep->arg[2]);
		uint32 charid = database.GetCharacterID(sep->arg[3]);

		if (id <= 0 || charid <= 0) {
			c->Message(Chat::White, "Must enter a valid instance id and player name.");
		}

		if (database.RemoveClientFromInstance(id, charid)) {
			c->Message(Chat::White, "Removed client from instance.");
		}
		else {
			c->Message(Chat::White, "Failed to remove client from instance.");
		}
	}
	else if (strcasecmp(sep->arg[1], "list") == 0) {
		uint32 charid = database.GetCharacterID(sep->arg[2]);
		if (charid <= 0) {
			if (c->GetTarget() == nullptr || (c->GetTarget() && !c->GetTarget()->IsClient())) {
				c->Message(Chat::White, "Character not found.");
				return;
			}
			else {
				charid = c->GetTarget()->CastToClient()->CharacterID();
			}
		}

		database.ListAllInstances(c, charid);
	}
	else {
		c->Message(Chat::White, "Invalid Argument.");
		c->Message(Chat::White, "#instance usage:");
		c->Message(
			Chat::White, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
						 "zone with id matching zone_id, will last for duration seconds."
		);
		c->Message(Chat::White, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
		c->Message(
			Chat::White, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
						 "with id matching instance_id."
		);
		c->Message(
			Chat::White, "#instance remove instance_id player_name - removes the player 'player_name' from the "
						 "instance with id matching instance_id."
		);
		c->Message(Chat::White, "#instance list player_name - lists all the instances 'player_name' is apart of.");
		return;
	}
}

