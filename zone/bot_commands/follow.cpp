#include "../bot_command.h"

void bot_command_follow(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_follow", sep->arg[0], "follow"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<friendly_target>) %s ([option: reset]) [actionable: byname | ownergroup | ownerraid | namesgroup | mmr | byclass | byrace | spawned]] ([actionable_name])", sep->arg[0]);
		c->Message(Chat::White, "usage: %s chain", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type2;

	bool reset = false;
	int ab_arg = 1;
	int name_arg = 2;
	Mob* target_mob = nullptr;

	std::string optional_arg = sep->arg[1];
	if (!optional_arg.compare("chain")) {

		auto chain_count = helper_bot_follow_option_chain(c);
		c->Message(Chat::White, "%i of your bots %s now chain following you", chain_count, (chain_count == 1 ? "is" : "are"));

		return;
	}
	else if (!optional_arg.compare("reset")) {
		reset = true;
		++ab_arg;
		++name_arg ;
	}
	else {
		//target_mob = ActionableTarget::VerifyFriendly(c, BCEnum::TT_Single);
		target_mob = c->GetTarget();
		if (!target_mob) {
			c->Message(Chat::White, "You must <target> a friendly player or bot within your group or raid to use this command");
			return;
		}
		else if (!target_mob->IsBot() && !target_mob->IsClient()) {
			c->Message(Chat::White, "You must <target> a friendly player or bot within your group or raid to use this command");
			return;
		}
		else if ((target_mob->GetGroup() && target_mob->GetGroup() != c->GetGroup()) || (target_mob->GetRaid() && target_mob->GetRaid() != c->GetRaid())) {
			c->Message(Chat::White, "You must <target> a friendly player or bot within your group or raid to use this command");
			return;
		}
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[name_arg] : nullptr, class_race_check ? atoi(sep->arg[name_arg]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.remove(nullptr);
	for (auto bot_iter : sbl) {
		bot_iter->WipeHateList();

		if (!bot_iter->GetGroup() && !bot_iter->GetRaid()) {
			bot_iter->SetFollowID(0);
			bot_iter->SetManualFollow(false);
		}
		else {
			if (reset) {
				bot_iter->SetFollowID(c->GetID());
				bot_iter->SetManualFollow(false);
			}
			else {
				if (target_mob->IsGrouped() || target_mob->IsRaidGrouped()) {
					bot_iter->SetFollowID(target_mob->GetID());
					bot_iter->SetManualFollow(true);
				}
				else if (bot_iter == target_mob) {
					bot_iter->SetFollowID(c->GetID());
					bot_iter->SetManualFollow(true);
				}
				else {
					bot_iter->SetFollowID(0);
					bot_iter->SetManualFollow(false);
				}
			}
		}
		//auto my_group = bot_iter->GetGroup();
		//if (my_group) {
		//	if (reset) {
		//		if (!my_group->GetLeader() || my_group->GetLeader() == bot_iter)
		//			bot_iter->SetFollowID(c->GetID());
		//		else
		//			bot_iter->SetFollowID(my_group->GetLeader()->GetID());
		//
		//		bot_iter->SetManualFollow(false);
		//	}
		//	else {
		//		if (bot_iter == target_mob)
		//			bot_iter->SetFollowID(c->GetID());
		//		else
		//			bot_iter->SetFollowID(target_mob->GetID());
		//
		//		bot_iter->SetManualFollow(true);
		//	}
		//}
		//else {
		//	bot_iter->SetFollowID(0);
		//	bot_iter->SetManualFollow(false);
		//}
		if (!bot_iter->GetPet())
			continue;

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->SetFollowID(bot_iter->GetID());
	}

	Mob* follow_mob = nullptr;
	if (sbl.size() == 1) {
		follow_mob = entity_list.GetMob(sbl.front()->GetFollowID());
		c->Message(
			Chat::White,
			fmt::format(
				"Following {}.",
				follow_mob ? follow_mob->GetCleanName() : "you"
			).c_str()
		);
	}
	else {
		if (reset) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} of your bots are following you.",
					sbl.size()
				).c_str()
			);
		}
		else {
			if (!sbl.empty()) {
				follow_mob = entity_list.GetMob(sbl.front()->GetFollowID());
			}
			c->Message(
				Chat::White,
				fmt::format(
					"{} of your bots are following {}.",
					sbl.size(),
					follow_mob ? follow_mob->GetCleanName() : "you"
				).c_str()
			);
		}
	}
}
