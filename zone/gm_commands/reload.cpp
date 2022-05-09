#include "../client.h"

void command_reload(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->SendReloadCommandMessages();
		return;
	}

	bool is_aa = !strcasecmp(sep->arg[1], "aa");
	bool is_content_flags = !strcasecmp(sep->arg[1], "content_flags");
	bool is_level_mods = !strcasecmp(sep->arg[1], "level_mods");
	bool is_logs = !strcasecmp(sep->arg[1], "logs");
	bool is_merchants = !strcasecmp(sep->arg[1], "merchants");
	bool is_npc_emotes = !strcasecmp(sep->arg[1], "npc_emotes");
	bool is_perl_export = !strcasecmp(sep->arg[1], "perl_export");
	bool is_quest = !strcasecmp(sep->arg[1], "quest");
	bool is_rules = !strcasecmp(sep->arg[1], "rules");
	bool is_static = !strcasecmp(sep->arg[1], "static");
	bool is_tasks = !strcasecmp(sep->arg[1], "tasks");
	bool is_titles = !strcasecmp(sep->arg[1], "titles");
	bool is_traps = !strcasecmp(sep->arg[1], "traps");
	bool is_variables = !strcasecmp(sep->arg[1], "variables");
	bool is_world = !strcasecmp(sep->arg[1], "world");
	bool is_zone_points = !strcasecmp(sep->arg[1], "zone_points");

	if (
		!is_aa &&
		!is_content_flags &&
		!is_level_mods &&
		!is_logs &&
		!is_merchants &&
		!is_npc_emotes &&
		!is_perl_export &&
		!is_quest &&
		!is_rules &&
		!is_static &&
		!is_tasks &&
		!is_titles &&
		!is_traps &&
		!is_variables &&
		!is_world &&
		!is_zone_points
	) {
		c->SendReloadCommandMessages();
		return;
	}

	auto pack = new ServerPacket;

	if (is_aa) {
		c->Message(Chat::White, "Attempting to reload Alternate Advancement Data globally.");	
		pack = new ServerPacket(ServerOP_ReloadAAData, 0);
	} else if (is_content_flags) {		
		c->Message(Chat::White, "Attempting to reload Content Flags globally.");
		pack = new ServerPacket(ServerOP_ReloadContentFlags, 0);
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
	} else if (is_zone_points) {		
		c->Message(Chat::White, "Attempting to reloading Zone Points globally.");
		pack = new ServerPacket(ServerOP_ReloadZonePoints, 0);
	}
	
	if (pack->opcode) {
		worldserver.SendPacket(pack);
	}

	safe_delete(pack);
}
