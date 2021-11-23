#include "../client.h"

void command_zclip(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (
		!arguments ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2)
	) {		
		c->Message(Chat::White, "Usage: #zclip [Minimum Clip] [Maximum Clip] [Fog Minimum Clip] [Fog Maximum Clip] [Permanent (0 = False, 1 = True)]");
	}

	auto minimum_clip = std::stof(sep->arg[1]);
	auto maximum_clip = std::stof(sep->arg[2]);
	auto minimum_fog_clip = sep->arg[3] ? std::stof(sep->arg[3]) : 0;
	auto maximum_fog_clip = sep->arg[4] ? std::stof(sep->arg[4]) : 0;
	auto permanent = sep->arg[5] ? atobool(sep->arg[5]) : false;
	if (minimum_clip <= 0 || maximum_clip <= 0) {
		c->Message(Chat::White, "Minimum Clip and Maximum Clip must be greater than 0.");
		return;
	} else if (minimum_clip > maximum_clip) {
		c->Message(Chat::White, "Minimum Clip must be less than or equal to Maximum Clip!");
		return;
	} else {
		zone->newzone_data.minclip = minimum_clip;
		zone->newzone_data.maxclip = maximum_clip;

		if (minimum_fog_clip) {
			for (int fog_index = 0; fog_index < 4; fog_index++) {
				zone->newzone_data.fog_minclip[fog_index] = minimum_fog_clip;
			}
		}

		if (maximum_fog_clip) {
			for (int fog_index = 0; fog_index < 4; fog_index++) {
				zone->newzone_data.fog_maxclip[fog_index] = maximum_fog_clip;
			}
		}

		if (permanent) {
			auto query = fmt::format(
				"UPDATE zone SET minclip = {:.2f}, maxclip = {:.2f} WHERE zoneidnumber = {} AND version = {}",
				minimum_clip,
				maximum_clip,
				zone->GetZoneID(),
				zone->GetInstanceVersion()
			);
			database.QueryDatabase(query);

			if (minimum_fog_clip) {
				query = fmt::format(
					"UPDATE zone SET fog_minclip = {:.2f} WHERE zoneidnumber = {} AND version = {}",
					minimum_fog_clip,
					zone->GetZoneID(),
					zone->GetInstanceVersion()
				);
				database.QueryDatabase(query);
			}

			if (maximum_fog_clip) {
				query = fmt::format(
					"UPDATE zone SET fog_maxclip = {:.2f} WHERE zoneidnumber = {} AND version = {}",
					maximum_fog_clip,
					zone->GetZoneID(),
					zone->GetInstanceVersion()
				);
				database.QueryDatabase(query);
			}
		}

		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);

		c->Message(
			Chat::White,
			fmt::format(
				"Clipping Changed | Zone: {} ({}) Permanent: {}",
				zone->GetLongName(),
				zone->GetZoneID(),
				permanent ? "Yes" : "No"
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Clipping Changed | Minimum Clip: {:.2f} Maximum Clip: {:.2f}",
				minimum_clip,
				maximum_clip
			).c_str()
		);

		if (minimum_fog_clip || maximum_fog_clip) {
			c->Message(
				Chat::White,
				fmt::format(
					"Clipping Changed | Fog Minimum Clip: {:.2f} Fog Maximum Clip: {:.2f}",
					minimum_fog_clip,
					maximum_fog_clip
				).c_str()
			);
		}
	}
}

