#include "../client.h"

void command_zstats(Client *c, const Seperator *sep)
{
	// Zone
	c->Message(
		Chat::White,
		fmt::format(
			"Zone | {}",
			zone->GetZoneDescription()
		).c_str()
	);

	// Type
	c->Message(
		Chat::White,
		fmt::format(
			"Type: {}",
			zone->newzone_data.ztype
		).c_str()
	);

	// Fog Data
	for (int fog_index = 0; fog_index < 4; fog_index++) {
		int fog_number = (fog_index + 1);
		c->Message(
			Chat::White,
			fmt::format(
				"Fog {} Colors | Red: {} Blue: {} Green: {} ",
				fog_number,
				zone->newzone_data.fog_red[fog_index],
				zone->newzone_data.fog_green[fog_index],
				zone->newzone_data.fog_blue[fog_index]
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Fog {} Clipping Distance | Min: {} Max: {}",
				fog_number,
				zone->newzone_data.fog_minclip[fog_index],
				zone->newzone_data.fog_maxclip[fog_index]
			).c_str()
		);
	}

	// Fog Density
	c->Message(
		Chat::White,
		fmt::format(
			"Fog Density: {}",
			zone->newzone_data.fog_density
		).c_str()
	);


	// Gravity
	c->Message(
		Chat::White,
		fmt::format(
			"Gravity: {}",
			zone->newzone_data.gravity
		).c_str()
	);

	// Time Type
	c->Message(
		Chat::White,
		fmt::format(
			"Time Type: {}",
			zone->newzone_data.time_type
		).c_str()
	);

	// Experience Multiplier
	c->Message(
		Chat::White,
		fmt::format(
			"Experience Multiplier: {}",
			zone->newzone_data.zone_exp_multiplier
		).c_str()
	);

	// Safe Coordinates
	c->Message(
		Chat::White,
		fmt::format(
			"Safe Coordinates: {}, {}, {}",
			zone->newzone_data.safe_x,
			zone->newzone_data.safe_y,
			zone->newzone_data.safe_z
		).c_str()
	);

	// Max Z
	c->Message(
		Chat::White,
		fmt::format(
			"Max Z: {}",
			zone->newzone_data.max_z
		).c_str()
	);

	// Underworld Z
	c->Message(
		Chat::White,
		fmt::format(
			"Underworld Z: {}",
			zone->newzone_data.underworld
		).c_str()
	);

	// Clipping Distance
	c->Message(
		Chat::White,
		fmt::format(
			"Clipping Distance | Min: {} Max: {}",
			zone->newzone_data.minclip,
			zone->newzone_data.maxclip
		).c_str()
	);

	// Weather Data
	for (int weather_index = 0; weather_index < 4; weather_index++) {
		int weather_number = (weather_index + 1);
		c->Message(
			Chat::White,
			fmt::format(
				"Rain {} | Chance: {} Duration: {} ",
				weather_number,
				zone->newzone_data.rain_chance[weather_index],
				zone->newzone_data.rain_duration[weather_index]
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Snow {} | Chance: {} Duration: {}",
				weather_number,
				zone->newzone_data.snow_chance[weather_index],
				zone->newzone_data.snow_duration[weather_index]
			).c_str()
		);
	}

	// Sky Type
	c->Message(
		Chat::White,
		fmt::format(
			"Sky Type: {}",
			zone->newzone_data.sky
		).c_str()
	);

	// Suspend Buffs
	c->Message(
		Chat::White,
		fmt::format(
			"Suspend Buffs: {}",
			zone->newzone_data.SuspendBuffs
		).c_str()
	);

	// Regeneration Data
	c->Message(
		Chat::White,
		fmt::format(
			"Regen | Health: {} Mana: {} Endurance: {}",
			zone->newzone_data.FastRegenHP,
			zone->newzone_data.FastRegenMana,
			zone->newzone_data.FastRegenEndurance
		).c_str()
	);

	// NPC Max Aggro Distance
	c->Message(
		Chat::White,
		fmt::format(
			"NPC Max Aggro Distance: {}",
			zone->newzone_data.NPCAggroMaxDist
		).c_str()
	);

	// Underworld Teleport Index
	c->Message(
		Chat::White,
		fmt::format(
			"Underworld Teleport Index: {}",
			zone->newzone_data.underworld_teleport_index
		).c_str()
	);

	// Lava Damage
	c->Message(
		Chat::White,
		fmt::format(
			"Lava Damage | Min: {} Max: {}",
			zone->newzone_data.MinLavaDamage,
			zone->newzone_data.LavaDamage
		).c_str()
	);
}

