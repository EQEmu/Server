#include "../client.h"
#include "../bot_command.h"

void bot_command_stance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_stance", sep->arg[0], "botstance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [current | value: 1-9] ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "value: %u(%s), %u(%s), %u(%s), %u(%s), %u(%s), %u(%s), %u(%s)",
				   EQ::constants::stancePassive, EQ::constants::GetStanceName(EQ::constants::stancePassive),
				   EQ::constants::stanceBalanced, EQ::constants::GetStanceName(EQ::constants::stanceBalanced),
				   EQ::constants::stanceEfficient, EQ::constants::GetStanceName(EQ::constants::stanceEfficient),
				   EQ::constants::stanceReactive, EQ::constants::GetStanceName(EQ::constants::stanceReactive),
				   EQ::constants::stanceAggressive, EQ::constants::GetStanceName(EQ::constants::stanceAggressive),
				   EQ::constants::stanceAssist, EQ::constants::GetStanceName(EQ::constants::stanceAssist),
				   EQ::constants::stanceBurn, EQ::constants::GetStanceName(EQ::constants::stanceBurn),
				   EQ::constants::stanceEfficient2, EQ::constants::GetStanceName(EQ::constants::stanceEfficient2),
				   EQ::constants::stanceBurnAE, EQ::constants::GetStanceName(EQ::constants::stanceBurnAE)
		);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	bool current_flag = false;
	auto bst = EQ::constants::stanceUnknown;

	if (!strcasecmp(sep->arg[1], "current"))
		current_flag = true;
	else if (sep->IsNumber(1)) {
		bst = (EQ::constants::StanceType)Strings::ToInt(sep->arg[1]);
		if (bst < EQ::constants::stanceUnknown || bst > EQ::constants::stanceBurnAE)
			bst = EQ::constants::stanceUnknown;
	}

	if (!current_flag && bst == EQ::constants::stanceUnknown) {
		c->Message(Chat::White, "A [current] argument or valid numeric [value] is required to use this command");
		return;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (!current_flag) {
			bot_iter->SetBotStance(bst);
			bot_iter->Save();
		}

		Bot::BotGroupSay(
			bot_iter,
			fmt::format(
				"My current stance is {} ({}).",
				EQ::constants::GetStanceName(bot_iter->GetBotStance()),
				bot_iter->GetBotStance()
			).c_str()
		);
	}
}
