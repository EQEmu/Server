#include "../bot_command.h"

void bot_command_aggressive(Client* c, const Seperator* sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Stance];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Stance) ||
		helper_command_alias_fail(c, "bot_command_aggressive", sep->arg[0], "aggressive")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			"usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | byclass | byrace | spawned] ([actionable_name]))",
			sep->arg[0]
		);
		helper_send_usage_required_bots(c, BCEnum::SpT_Stance);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;

	std::string class_race_arg   = sep->arg[1];
	bool        class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(
		c,
		sep->arg[1],
		sbl,
		ab_mask,
		!class_race_check ? sep->arg[2] : nullptr,
		class_race_check ? atoi(sep->arg[2]) : 0
	) == ActionableBots::ABT_None) {
		return;
	}

	sbl.remove(nullptr);

	int       success_count   = 0;
	int       candidate_count = sbl.size();
	for (auto list_iter: *local_list) {
		if (sbl.empty()) {
			break;
		}

		auto local_entry = list_iter->SafeCastToStance();
		if (helper_spell_check_fail(local_entry)) {
			continue;
		}
		if (local_entry->stance_type != BCEnum::StT_Aggressive) {
			continue;
		}

		for (auto bot_iter = sbl.begin(); bot_iter != sbl.end();) {
			Bot* my_bot = *bot_iter;
			if (local_entry->caster_class != my_bot->GetClass()) {
				++bot_iter;
				continue;
			}
			if (local_entry->spell_level > my_bot->GetLevel()) {
				++bot_iter;
				continue;
			}

			my_bot->InterruptSpell();
			if (candidate_count == 1) {
				Bot::BotGroupSay(
					my_bot,
					fmt::format(
						"Using {}.",
						spells[local_entry->spell_id].name
					).c_str()
				);
			}

			my_bot->UseDiscipline(local_entry->spell_id, my_bot->GetID());
			++success_count;

			bot_iter = sbl.erase(bot_iter);
		}
	}

	c->Message(
		Chat::White,
		"%i of %i bots have attempted to use aggressive disciplines",
		success_count,
		candidate_count
	);
}
