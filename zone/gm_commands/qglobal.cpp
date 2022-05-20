#include "../client.h"

void command_qglobal(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #qglobal on - Enables target NPC's ability to view quest globals");
		c->Message(Chat::White, "Usage: #qglobal off - Disables target NPC's ability to view quest globals");
		c->Message(Chat::White, "Usage: #qglobal view - View target NPC's ability to view quest globals");
		return;
	}


	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();

	bool is_off = !strcasecmp(sep->arg[1], "off");
	bool is_on = !strcasecmp(sep->arg[1], "on");
	bool is_view = !strcasecmp(sep->arg[1], "view");
	if (
		!is_off &&
		!is_on &&
		!is_view
	) {
		c->Message(Chat::White, "Usage: #qglobal on - Enables target NPC's ability to view quest globals");
		c->Message(Chat::White, "Usage: #qglobal off - Disables target NPC's ability to view quest globals");
		c->Message(Chat::White, "Usage: #qglobal view - View target NPC's ability to view quest globals");
		return;		
	}

	if (is_off) {
		auto query = fmt::format(
			"UPDATE npc_types SET qglobal = 0 WHERE id = {}",
			target->GetNPCTypeID()
		);
		auto results = content_db.QueryDatabase(query);

		if (!results.Success()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to disable quest global flag for {}.",
					c->GetTargetDescription(target)
				).c_str()
			);
			return;
		}

		auto repop_link = EQ::SayLinkEngine::GenerateQuestSaylink(
			"#repop",
			false,
			"repop"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} will no longer be able to view quest globals, {} them to apply this change.",
				c->GetTargetDescription(target),
				repop_link
			).c_str()
		);
		return;
	} else if (is_on) {
		auto query = fmt::format(
			"UPDATE npc_types SET qglobal = 1 WHERE id = {}",
			target->GetNPCTypeID()
		);
		auto results = content_db.QueryDatabase(query);

		if (!results.Success()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to enable quest global flag for {}.",
					c->GetTargetDescription(target)
				).c_str()
			);
			return;
		}

		auto repop_link = EQ::SayLinkEngine::GenerateQuestSaylink(
			"#repop",
			false,
			"repop"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} will now be able to view quest globals, {} them to apply this change.",
				c->GetTargetDescription(target),
				repop_link
			).c_str()
		);
		return;
	} else if (!strcasecmp(sep->arg[1], "view")) {
		const NPCType *npc_type = content_db.LoadNPCTypesData(target->GetNPCTypeID());
		if (!npc_type) {
			c->Message(
				Chat::White,
				fmt::format(
					"NPC ID {} was not found.",
					target->GetNPCTypeID()
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} {} view quest globals.",
				c->GetTargetDescription(target),
				npc_type->qglobal ? "can" : "cannot"
			).c_str()
		);
	}
}

