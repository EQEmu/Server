#include "../bot_command.h"

void bot_command_mesmerize(Client *c, const Seperator *sep)
{
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <enemy_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Mesmerize);
		return;
	}

	bool isSuccess = false;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto bot_iter : sbl) {
		std::list<BotSpell_wPriority> botSpellList = bot_iter->GetPrioritizedBotSpellsBySpellType(bot_iter, BotSpellTypes::Mez, c->GetTarget(), IsAEBotSpellType(BotSpellTypes::Mez));

		for (const auto& s : botSpellList) {
			if (!IsValidSpell(s.SpellId)) {
				continue;
			}

			if (!bot_iter->IsInGroupOrRaid()) {
				continue;
			}

			if (!bot_iter->CastChecks(s.SpellId, c->GetTarget(), BotSpellTypes::Mez, false, false)) {
				continue;
			}

			if (bot_iter->CommandedDoSpellCast(s.SpellIndex, c->GetTarget(), s.ManaCost)) {
				bot_iter->BotGroupSay(bot_iter, "Casting %s [%s] on %s.", GetSpellName(s.SpellId), bot_iter->GetSpellTypeNameByID(BotSpellTypes::Mez), c->GetTarget()->GetCleanName());
				isSuccess = true;
			}
		}
	}

	if (!isSuccess) {
		helper_no_available_bots(c);
	}
}
