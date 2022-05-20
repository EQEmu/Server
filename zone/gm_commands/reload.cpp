#include "../client.h"

void command_reload(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->SendReloadCommandMessages();
		return;
	}

	bool is_aa = !strcasecmp(sep->arg[1], "aa");
	bool is_alternate_currencies = !strcasecmp(sep->arg[1], "alternate_currencies");
	bool is_blocked_spells = !strcasecmp(sep->arg[1], "blocked_spells");
	bool is_content_flags = !strcasecmp(sep->arg[1], "content_flags");
	bool is_doors = !strcasecmp(sep->arg[1], "doors");
	bool is_ground_spawns = !strcasecmp(sep->arg[1], "ground_spawns");
	bool is_level_mods = !strcasecmp(sep->arg[1], "level_mods");
	bool is_logs = !strcasecmp(sep->arg[1], "logs");
	bool is_merchants = !strcasecmp(sep->arg[1], "merchants");
	bool is_npc_emotes = !strcasecmp(sep->arg[1], "npc_emotes");
	bool is_objects = !strcasecmp(sep->arg[1], "objects");
	bool is_perl_export = !strcasecmp(sep->arg[1], "perl_export");
	bool is_quest = !strcasecmp(sep->arg[1], "quest");
	bool is_rules = !strcasecmp(sep->arg[1], "rules");
	bool is_static = !strcasecmp(sep->arg[1], "static");
	bool is_tasks = !strcasecmp(sep->arg[1], "tasks");
	bool is_titles = !strcasecmp(sep->arg[1], "titles");
	bool is_traps = !strcasecmp(sep->arg[1], "traps");
	bool is_variables = !strcasecmp(sep->arg[1], "variables");
	bool is_veteran_rewards = !strcasecmp(sep->arg[1], "veteran_rewards");
	bool is_world = !strcasecmp(sep->arg[1], "world");
	bool is_zone = !strcasecmp(sep->arg[1], "zone");
	bool is_zone_points = !strcasecmp(sep->arg[1], "zone_points");

	if (
		!is_aa &&
		!is_alternate_currencies &&
		!is_blocked_spells &&
		!is_content_flags &&
		!is_doors &&
		!is_ground_spawns &&
		!is_level_mods &&
		!is_logs &&
		!is_merchants &&
		!is_npc_emotes &&
		!is_objects &&
		!is_perl_export &&
		!is_quest &&
		!is_rules &&
		!is_static &&
		!is_tasks &&
		!is_titles &&
		!is_traps &&
		!is_variables &&
		!is_veteran_rewards &&
		!is_world &&
		!is_zone &&
		!is_zone_points
	) {
		c->SendReloadCommandMessages();
		return;
	}

	auto pack = new ServerPacket;

	if (is_aa) {
		c->Message(Chat::White, "Attempting to reload Alternate Advancement Data globally.");	
		pack = new ServerPacket(ServerOP_ReloadAAData, 0);
	} else if (is_alternate_currencies) {
		c->Message(Chat::White, "Attempting to reload Alternate Currencies globally.");	
		pack = new ServerPacket(ServerOP_ReloadAlternateCurrencies, 0);
	} else if (is_blocked_spells) {
		c->Message(Chat::White, "Attempting to reload Blocked Spells globally.");	
		pack = new ServerPacket(ServerOP_ReloadBlockedSpells, 0);
	} else if (is_content_flags) {		
		c->Message(Chat::White, "Attempting to reload Content Flags globally.");
		pack = new ServerPacket(ServerOP_ReloadContentFlags, 0);
	} else if (is_doors) {
		c->Message(Chat::White, "Attempting to reload Doors globally.");	
		pack = new ServerPacket(ServerOP_ReloadDoors, 0);
	} else if (is_ground_spawns) {
		c->Message(Chat::White, "Attempting to reload Ground Spawns globally.");	
		pack = new ServerPacket(ServerOP_ReloadGroundSpawns, 0);
	} else if (is_level_mods) {
		if (!RuleB(Zone, LevelBasedEXPMods)) {
			c->Message(Chat::White, "Level Based Experience Modifiers are disabled.");
			return;
		}
			
		c->Message(Chat::White, "Attempting to reload Level Based Experience Modifiers globally.");
		pack = new ServerPacket(ServerOP_ReloadLevelEXPMods, 0);
	} else if (is_logs) {
		c->Message(Chat::White, "Attempting to reload Log Settings globally.");
		pack = new ServerPacket(ServerOP_ReloadLogs, 0);
	} else if (is_merchants) {
		c->Message(Chat::White, "Attempting to reload Merchants globally.");
		pack = new ServerPacket(ServerOP_ReloadMerchants, 0);
	} else if (is_npc_emotes) {
		c->Message(Chat::White, "Attempting to reload NPC Emotes globally.");
		pack = new ServerPacket(ServerOP_ReloadNPCEmotes, 0);
	} else if (is_objects) {
		c->Message(Chat::White, "Attempting to reload Objects globally.");
		pack = new ServerPacket(ServerOP_ReloadObjects, 0);
	} else if (is_perl_export) {
		c->Message(Chat::White, "Attempting to reload Perl Event Export Settings globally.");
		pack = new ServerPacket(ServerOP_ReloadPerlExportSettings, 0);
	} else if (is_quest) {
		bool stop_timers = false;

		if (sep->IsNumber(2)) {
			stop_timers = std::stoi(sep->arg[2]) != 0 ? true : false;
		}

		std::string stop_timers_message = stop_timers ? " and timers stopped" : "";

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Quests reloaded{} for {}.",
				stop_timers_message,
				zone->GetZoneDescription()
			).c_str()
		);

		entity_list.ClearAreas();
		parse->ReloadQuests(stop_timers);
	} else if (is_rules) {
		c->Message(Chat::White, "Attempting to reload Rules globally.");
		pack = new ServerPacket(ServerOP_ReloadRules, 0);
	} else if (is_static) {		
		c->Message(Chat::White, "Attempting to reload Static Zone Data globally.");
		pack = new ServerPacket(ServerOP_ReloadStaticZoneData, 0);
	} else if (is_tasks) {
		uint32 task_id = 0;
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Attempting to reload Tasks globally.");
			pack = new ServerPacket(ServerOP_ReloadTasks, sizeof(ReloadTasks_Struct));
		} else {
			task_id = std::stoul(sep->arg[2]);
		}
		
		auto rts = (ReloadTasks_Struct*) pack->pBuffer;
		rts->reload_type = RELOADTASKS;
		rts->task_id = task_id;
	} else if (is_titles) {		
		c->Message(Chat::White, "Attempting to reload Titles globally.");
		pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	} else if (is_traps) {
		if (arguments < 2 || !sep->IsNumber(2)) {
			entity_list.UpdateAllTraps(true, true);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Traps reloaded for {}.",
					zone->GetZoneDescription()
				).c_str()
			);
			return;
		}

		bool global = std::stoi(sep->arg[2]) ? true : false;

		if (!global) {
			entity_list.UpdateAllTraps(true, true);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Traps reloaded for {}.",
					zone->GetZoneDescription()
				).c_str()
			);
			return;
		}

		c->Message(Chat::White, "Attempting to reload Traps globally.");
		pack = new ServerPacket(ServerOP_ReloadTraps, 0);
	} else if (is_variables) {
		c->Message(Chat::White, "Attempting to reload Variables globally.");
		pack = new ServerPacket(ServerOP_ReloadVariables, 0);
	} else if (is_veteran_rewards) {
		c->Message(Chat::White, "Attempting to reload Veteran Rewards globally.");
		pack = new ServerPacket(ServerOP_ReloadVeteranRewards, 0);
	} else if (is_world) {
		uint8 global_repop = ReloadWorld::NoRepop;

		if (sep->IsNumber(2)) {
			global_repop = static_cast<uint8>(std::stoul(sep->arg[2]));

			if (global_repop > ReloadWorld::ForceRepop) {
				global_repop = ReloadWorld::ForceRepop;
			}
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Attempting to reload Quests {}worldwide.",
				(
					global_repop ?
					(
						global_repop == ReloadWorld::Repop ?
						"and repop NPCs " :
						"and forcefully repop NPCs "
					) :
					""
				)
			).c_str()
		);

		pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
		auto RW  = (ReloadWorld_Struct *) pack->pBuffer;
		RW->global_repop = global_repop;
	} else if (is_zone) {
		if (arguments < 2) {
			c->Message(
				Chat::White,
				fmt::format(
					"Zone Header Load {} | Zone: {}",
					(
						zone->LoadZoneCFG(zone->GetShortName(), zone->GetInstanceVersion()) ?
						"Suceeded" :
						"Failed"
					),
					zone->GetZoneDescription()
				).c_str()
			);
			return;
		}

		auto zone_id = (
			sep->IsNumber(2) ?
			std::stoul(sep->arg[2]) :
			ZoneID(sep->arg[2])
		);
		if (!zone_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Zone ID {} could not be found.",
					zone_id
				).c_str()
			);
			return;
		}

		auto zone_short_name = ZoneName(zone_id);
		auto zone_long_name = ZoneLongName(zone_id);
		auto version = (
			sep->IsNumber(3) ?
			std::stoul(sep->arg[3]) :
			0
		);
		
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);

		c->Message(
			Chat::White,
			fmt::format(
				"Zone Header Load {} | Zone: {} ({}){}",
				(
					zone->LoadZoneCFG(zone_short_name, version) ?
					"Suceeded" :
					"Failed"
				),
				zone_long_name,
				zone_short_name,
				(
					version ?
					fmt::format(
						" Version: {}",
						version
					) :
					""
				)
			).c_str()
		);
	} else if (is_zone_points) {		
		c->Message(Chat::White, "Attempting to reloading Zone Points globally.");
		pack = new ServerPacket(ServerOP_ReloadZonePoints, 0);
	}
	
	if (pack->opcode) {
		worldserver.SendPacket(pack);
	}

	safe_delete(pack);
}
