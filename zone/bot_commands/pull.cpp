#include "../bot_command.h"

void bot_command_pull(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pull", sep->arg[0], "pull")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {

		c->Message(Chat::White, "usage: <enemy_target> %s", sep->arg[0]);
		return;
	}
	int ab_mask = ActionableBots::ABM_OwnerGroup; // existing behavior - need to add c->IsGrouped() check and modify code if different behavior is desired

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, "ownergroup", sbl, ab_mask) == ActionableBots::ABT_None) {
		return;
	}
	sbl.remove(nullptr);

	auto target_mob = ActionableTarget::VerifyEnemy(c, BCEnum::TT_Single);
	if (!target_mob) {

		c->Message(Chat::White, "Your current target is not attackable!");
		return;
	}

	if (target_mob->IsNPC() && target_mob->GetHateList().size()) {

		c->Message(Chat::White, "Your current target is already engaged!");
		return;
	}

	Bot* bot_puller = nullptr;
	for (auto bot_iter : sbl) {

		if (bot_iter->GetAppearance() == eaDead || bot_iter->GetBotStance() == Stance::Passive) {
			continue;
		}

		switch (bot_iter->GetClass()) {
			case Class::Rogue:
			case Class::Monk:
			case Class::Bard:
			case Class::Ranger:
				bot_puller = bot_iter;
				break;
			case Class::Warrior:
			case Class::ShadowKnight:
			case Class::Paladin:
			case Class::Berserker:
			case Class::Beastlord:
				if (!bot_puller) {

					bot_puller = bot_iter;
					continue;
				}

				switch (bot_puller->GetClass()) {
					case Class::Druid:
					case Class::Shaman:
					case Class::Cleric:
					case Class::Wizard:
					case Class::Necromancer:
					case Class::Magician:
					case Class::Enchanter:
						bot_puller = bot_iter;
					default:
						continue;
				}

				continue;
			case Class::Druid:
			case Class::Shaman:
			case Class::Cleric:
				if (!bot_puller) {

					bot_puller = bot_iter;
					continue;
				}

				switch (bot_puller->GetClass()) {
					case Class::Wizard:
					case Class::Necromancer:
					case Class::Magician:
					case Class::Enchanter:
						bot_puller = bot_iter;
					default:
						continue;
				}

				continue;
			case Class::Wizard:
			case Class::Necromancer:
			case Class::Magician:
			case Class::Enchanter:
				if (!bot_puller) {
					bot_puller = bot_iter;
				}

				continue;
			default:
				continue;
		}


		bot_puller = bot_iter;

		break;
	}

	if (bot_puller) {
		bot_puller->SetPullFlag();
	}

	helper_no_available_bots(c, bot_puller);
}
