#include "../client.h"

void command_worldwide(Client *c, const Seperator *sep)
{
	std::string sub_command;
	if (sep->arg[1]) {
		sub_command = sep->arg[1];
	}

	if (sub_command == "cast") {
		if (sep->arg[2] && Seperator::IsNumber(sep->arg[2])) {
			uint8 update_type     = WWSpellUpdateType_Cast;
			auto  spell_id        = std::stoul(sep->arg[2]);
			bool  disable_message = false;
			if (sep->arg[3] && Seperator::IsNumber(sep->arg[3])) {
				disable_message = std::stoi(sep->arg[3]) ? true : false;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"World Wide Cast Spell | Spell: {} ({})",
					GetSpellName(spell_id),
					spell_id
				).c_str()
			);

			quest_manager.WorldWideSpell(update_type, spell_id);
			if (!disable_message) {
				quest_manager.WorldWideMessage(
					Chat::Yellow,
					fmt::format(
						"[SYSTEM] A GM has cast [{}] world-wide!",
						GetSpellName(spell_id)
					).c_str()
				);
			}
		}
		else {
			c->Message(Chat::White, "Usage: #worldwide cast [Spell ID] [Disable Message]");
		}
	}
	else if (sub_command == "remove") {
		if (sep->arg[2] && Seperator::IsNumber(sep->arg[2])) {
			uint8 update_type = WWSpellUpdateType_Remove;
			auto  spell_id    = std::stoul(sep->arg[2]);

			c->Message(
				Chat::White,
				fmt::format(
					"World Wide Remove Spell | Spell: {} ({})",
					GetSpellName(spell_id),
					spell_id
				).c_str()
			);

			quest_manager.WorldWideSpell(update_type, spell_id);
		}
		else {
			c->Message(Chat::White, "Usage: #worldwide remove [Spell ID]");
		}
	}
	else if (sub_command == "message") {
		if (sep->arg[2]) {
			std::string message = sep->argplus[2];
			quest_manager.WorldWideMessage(
				Chat::White,
				fmt::format(
					"{}",
					message
				).c_str()
			);
		}
		else {
			c->Message(Chat::White, "Usage: #worldwide message [Message]");
		}
	}
	else if (sub_command == "move") {
		if (sep->arg[2]) {
			uint8       update_type = WWMoveUpdateType_MoveZone;
			uint32      zone_id     = 0;
			std::string zone_short_name;
			if (Seperator::IsNumber(sep->arg[2])) {
				zone_id = std::stoul(sep->arg[2]);
			}

			if (zone_id) {
				zone_short_name = ZoneName(zone_id);
			}
			else {
				zone_short_name = sep->arg[2];
			}

			c->Message(
				Chat::White,
				fmt::format(
					"World Wide Zone | Zone: {} ({}) ID: {}",
					ZoneLongName(
						ZoneID(zone_short_name)
					),
					zone_short_name,
					ZoneID(zone_short_name)
				).c_str()
			);

			quest_manager.WorldWideMove(update_type, zone_short_name.c_str());
		}
		else {
			c->Message(
				Chat::White,
				"Usage: #worldwide move [Zone ID] or #worldwide move [Zone Short Name]"
			);
		}
	}
	else if (sub_command == "moveinstance") {
		if (Seperator::IsNumber(sep->arg[2])) {
			uint8      update_type      = WWMoveUpdateType_MoveZoneInstance;
			const char *zone_short_name = "";
			uint16     instance_id      = std::stoi(sep->arg[2]);

			c->Message(
				Chat::White,
				fmt::format(
					"World Wide Zone Instance | Instance ID: {}",
					instance_id
				).c_str()
			);

			quest_manager.WorldWideMove(update_type, zone_short_name, instance_id);
		}
		else {
			c->Message(Chat::White, "Usage: #worldwide moveinstance [Instance ID]");
		}
	}

	if (!sep->arg[1]) {
		c->Message(Chat::White, "This command is used to perform world-wide tasks.");
		c->Message(Chat::White, "Usage: #worldwide cast [Spell ID] [Disable Message]");
		c->Message(Chat::White, "Usage: #worldwide remove [Spell ID]");
		c->Message(Chat::White, "Usage: #worldwide message [Message]");
		c->Message(
			Chat::White,
			"Usage: #worldwide move [Zone ID] or #worldwide move [Zone Short Name]"
		);
		c->Message(Chat::White, "Usage: #worldwide moveinstance [Instance ID]");
	}
}

