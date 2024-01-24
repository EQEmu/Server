#include "../client.h"
#include "../bot_command.h"

void bot_command_report(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_report", sep->arg[0], "botreport"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::string ab_type_arg = sep->arg[1];
	if (ab_type_arg.empty()) {
		auto t = c->GetTarget();
		if (t && t->IsClient()) {
			if (t->CastToClient() == c) {
				ab_type_arg = "ownergroup";
			} else {
				ab_type_arg = "targetgroup";
			}
		} else {
			ab_type_arg = "spawned";
		}
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, ab_type_arg, sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		std::string report_msg = StringFormat("%s %s reports", GetClassIDName(bot_iter->GetClass()), bot_iter->GetCleanName());
		report_msg.append(StringFormat(": %3.1f%% health", bot_iter->GetHPRatio()));
		if (!IsNonSpellFighterClass(bot_iter->GetClass()))
			report_msg.append(StringFormat(": %3.1f%% mana", bot_iter->GetManaRatio()));

		c->Message(Chat::White, "%s", report_msg.c_str());
	}
}
