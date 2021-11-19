#include "../client.h"
#include "../data_bucket.h"

void command_gmzone(Client *c, const Seperator *sep)
{
	if (!sep->arg[1]) {
		c->Message(Chat::White, "Usage");
		c->Message(Chat::White, "-------");
		c->Message(Chat::White, "#gmzone [zone_short_name] [zone_version=0]");
		return;
	}

	std::string zone_short_name_string = sep->arg[1];
	const char  *zone_short_name       = sep->arg[1];
	auto        zone_version           = static_cast<uint32>(sep->arg[2] ? atoi(sep->arg[2]) : 0);
	std::string identifier             = "gmzone";
	uint32      zone_id                = ZoneID(zone_short_name);
	uint32      duration               = 100000000;
	uint16      instance_id            = 0;

	if (zone_id == 0) {
		c->Message(Chat::Red, "Invalid zone specified");
		return;
	}

	if (sep->arg[3] && sep->arg[3][0]) {
		identifier = sep->arg[3];
	}

	std::string bucket_key             = StringFormat(
		"%s-%s-%u-instance",
		zone_short_name,
		identifier.c_str(),
		zone_version
	);
	std::string existing_zone_instance = DataBucket::GetData(bucket_key);

	if (existing_zone_instance.length() > 0) {
		instance_id = std::stoi(existing_zone_instance);

		c->Message(Chat::Yellow, "Found already created instance (%s) (%u)", zone_short_name, instance_id);
	}

	if (instance_id == 0) {
		if (!database.GetUnusedInstanceID(instance_id)) {
			c->Message(Chat::Red, "Server was unable to find a free instance id.");
			return;
		}

		if (!database.CreateInstance(instance_id, zone_id, zone_version, duration)) {
			c->Message(Chat::Red, "Server was unable to create a new instance.");
			return;
		}

		c->Message(
			Chat::Yellow,
			"New private GM instance %s was created with id %lu.",
			zone_short_name,
			(unsigned long) instance_id
		);
		DataBucket::SetData(bucket_key, std::to_string(instance_id));
	}

	if (instance_id > 0) {
		float target_x   = -1, target_y = -1, target_z = -1, target_heading = -1;
		int16 min_status = AccountStatus::Player;
		uint8 min_level  = 0;

		if (!content_db.GetSafePoints(
			zone_short_name,
			zone_version,
			&target_x,
			&target_y,
			&target_z,
			&target_heading,
			&min_status,
			&min_level
		)) {
			c->Message(Chat::Red, "Failed to find safe coordinates for specified zone");
		}

		c->Message(Chat::Yellow, "Zoning to private GM instance (%s) (%u)", zone_short_name, instance_id);

		c->AssignToInstance(instance_id);
		c->MovePC(zone_id, instance_id, target_x, target_y, target_z, target_heading, 1);
	}
}

