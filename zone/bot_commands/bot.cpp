#include "../bot_command.h"

void bot_command_bot(Client *c, const Seperator *sep)
{

	std::list<const char*> subcommand_list;
	subcommand_list.push_back("botappearance");
	subcommand_list.push_back("botcamp");
	subcommand_list.push_back("botclone");
	subcommand_list.push_back("botcreate");
	subcommand_list.push_back("botdelete");
	subcommand_list.push_back("botfollowdistance");
	subcommand_list.push_back("botinspectmessage");
	subcommand_list.push_back("botlist");
	subcommand_list.push_back("botoutofcombat");
	subcommand_list.push_back("botreport");
	subcommand_list.push_back("botspawn");
	subcommand_list.push_back("botstance");
	subcommand_list.push_back("botstopmeleelevel");
	subcommand_list.push_back("botsummon");
	subcommand_list.push_back("bottogglearcher");
	subcommand_list.push_back("bottogglehelm");
	subcommand_list.push_back("botupdate");

	if (helper_command_alias_fail(c, "bot_command_bot", sep->arg[0], "bot"))
		return;

	helper_send_available_subcommands(c, "bot", subcommand_list);
}

void bot_command_camp(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_camp", sep->arg[0], "botcamp"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;

	std::string class_race_arg = sep->arg[1];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, !class_race_check ? sep->arg[2] : nullptr, class_race_check ? atoi(sep->arg[2]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl)
		bot_iter->Camp();
}

void bot_command_clone(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_clone", sep->arg[0], "botclone")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: <target_bot> {} [clone_name]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command!");
		return;
	}

	if (!my_bot->GetBotID()) {
		c->Message(
			Chat::White,
			fmt::format(
				"An unknown error has occured with {} (Bot ID {}).",
				my_bot->GetCleanName(),
				my_bot->GetBotID()
			).c_str()
		);
		LogCommands(
			"bot_command_clone(): - Error: Active bot reported invalid ID (BotName: [{}], BotID: [{}], OwnerName: [{}], OwnerID: [{}], AcctName: [{}], AcctID: [{}])",
			my_bot->GetCleanName(),
			my_bot->GetBotID(),
			c->GetCleanName(),
			c->CharacterID(),
			c->AccountName(),
			c->AccountID()
		);
		return;
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must name your bot clone.");
		return;
	}

	std::string bot_name = sep->arg[1];

	if (!Bot::IsValidName(bot_name)) {
		c->Message(
			Chat::White,
			fmt::format(
				"'{}' is an invalid name. You may only use characters 'A-Z', 'a-z' and '_'.",
				bot_name
			).c_str()
		);
		return;
	}

	bool available_flag = false;
	if (!database.botdb.QueryNameAvailablity(bot_name, available_flag)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to query name availability for '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	if (!available_flag) {
		c->Message(
			Chat::White,
			fmt::format(
				"The name '{}' is already being used. Please choose a different name.",
				bot_name
			).c_str()
		);
		return;
	}

	auto bot_creation_limit = c->GetBotCreationLimit();
	auto bot_creation_limit_class = c->GetBotCreationLimit(my_bot->GetClass());

	uint32 bot_count = 0;
	uint32 bot_class_count = 0;
	if (!database.botdb.QueryBotCount(c->CharacterID(), my_bot->GetClass(), bot_count, bot_class_count)) {
		c->Message(Chat::White, "Failed to query bot count.");
		return;
	}

	if (bot_creation_limit >= 0 && bot_count >= bot_creation_limit) {
		std::string message;

		if (bot_creation_limit) {
			message =  fmt::format(
				"You have reached the maximum limit of {} bot{}.",
				bot_creation_limit,
				bot_creation_limit != 1 ? "s" : ""
			);
		} else {
			message = "You cannot create any bots.";
		}

		c->Message(Chat::White, message.c_str());
		return;
	}

	if (bot_creation_limit_class >= 0 && bot_class_count >= bot_creation_limit_class) {
		std::string message;

		if (bot_creation_limit_class) {
			message = fmt::format(
				"You cannot create anymore than {} {} bot{}.",
				bot_creation_limit_class,
				GetClassIDName(my_bot->GetClass()),
				bot_creation_limit_class != 1 ? "s" : ""
			);
		} else {
			message = fmt::format(
				"You cannot create any {} bots.",
				GetClassIDName(my_bot->GetClass())
			);
		}

		c->Message(Chat::White, message.c_str());
		return;
	}

	uint32 clone_id = 0;
	if (!database.botdb.CreateCloneBot(my_bot->GetBotID(), bot_name, clone_id) || !clone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to create clone bot '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	int clone_stance = Stance::Passive;
	if (!database.botdb.LoadStance(my_bot->GetBotID(), clone_stance)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to load stance from '{}'.",
				my_bot->GetCleanName()
			).c_str()
		);
	}

	if (!database.botdb.SaveStance(clone_id, clone_stance)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to save stance for clone '{}'.",
				bot_name
			).c_str()
		);
	}

	if (!database.botdb.CreateCloneBotInventory(my_bot->GetBotID(), clone_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to create clone bot inventory for clone '{}'.",
				bot_name
			).c_str()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Bot Cloned | From: {} To: {}",
			my_bot->GetCleanName(),
			bot_name
		).c_str()
	);
}

void bot_command_create(Client *c, const Seperator *sep)
{
	const std::string class_substrs[17] = {
		"",
		"WAR", "CLR", "PAL", "RNG",
		"SHD", "DRU", "MNK", "BRD",
		"ROG", "SHM", "NEC", "WIZ",
		"MAG", "ENC", "BST", "BER"
	};

	const std::string race_substrs[17] = {
		"",
		"HUM", "BAR", "ERU", "ELF",
		"HIE", "DEF", "HEF", "DWF",
		"TRL", "OGR", "HFL", "GNM",
		"IKS", "VAH", "FRG", "DRK"
	};

	const uint16 race_values[17] = {
		Race::Doug,
		Race::Human, Race::Barbarian, Race::Erudite, Race::WoodElf,
		Race::HighElf, Race::DarkElf, Race::HalfElf, Race::Dwarf,
		Race::Troll, Race::Ogre, Race::Halfling, Race::Gnome,
		Race::Iksar, Race::VahShir, Race::Froglok2, Race::Drakkin
	};

	const std::string gender_substrs[2] = {
		"Male", "Female",
	};

	if (helper_command_alias_fail(c, "bot_command_create", sep->arg[0], "botcreate")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Name] [Class] [Race] [Gender]",
				sep->arg[0]
			).c_str()
		);

		std::string window_text;
		std::string message_separator;
		int object_count = 0;
		const int object_max = 4;

		window_text.append(
			fmt::format(
				"Classes{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		object_count = 0;
		for (int i = 0; i <= 15; ++i) {
			window_text.append(message_separator);

			if (object_count >= object_max) {
				window_text.append(DialogueWindow::Break());
				object_count = 0;
			}

			window_text.append(
				fmt::format("{} ({})",
							class_substrs[i + 1],
							(i + 1)
				)
			);

			++object_count;
			message_separator = ", ";
		}

		window_text.append(DialogueWindow::Break(2));

		window_text.append(
			fmt::format(
				"<c \"#FFFFFF\">Races{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		object_count = 0;
		for (int i = 0; i <= 15; ++i) {
			window_text.append(message_separator);

			if (object_count >= object_max) {
				window_text.append(DialogueWindow::Break());
				object_count = 0;
			}

			window_text.append(
				fmt::format("{} ({})",
							race_substrs[i + 1],
							race_values[i + 1]
				)
			);

			++object_count;
			message_separator = ", ";
		}

		window_text.append(DialogueWindow::Break(2));

		window_text.append(
			fmt::format(
				"<c \"#FFFFFF\">Genders{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		for (int i = 0; i <= 1; ++i) {
			window_text.append(message_separator);

			window_text.append(
				fmt::format("{} ({})",
							gender_substrs[i],
							i
				)
			);

			message_separator = ", ";
		}

		c->SendPopupToClient("Bot Creation Options", window_text.c_str());

		return;
	}

	const auto arguments = sep->argnum;
	if (!arguments || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must name your bot!");
		return;
	}

	std::string bot_name = sep->arg[1];
	bot_name = Strings::UcFirst(bot_name);

	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Invalid class!");
		return;
	}

	auto bot_class = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));

	if (arguments < 3 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Invalid race!");
		return;
	}

	auto bot_race = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[3]));

	if (arguments < 4) {
		c->Message(Chat::White, "Invalid gender!");
		return;
	}

	auto bot_gender = Gender::Male;

	if (sep->IsNumber(4)) {
		bot_gender = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[4]));
		if (bot_gender == Gender::Neuter) {
			bot_gender = Gender::Male;
		}
	} else {
		if (!strcasecmp(sep->arg[4], "m") || !strcasecmp(sep->arg[4], "male")) {
			bot_gender = Gender::Male;
		} else if (!strcasecmp(sep->arg[4], "f") || !strcasecmp(sep->arg[4], "female")) {
			bot_gender = Gender::Female;
		}
	}

	helper_bot_create(c, bot_name, bot_class, bot_race, bot_gender);
}

void bot_command_delete(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_delete", sep->arg[0], "botdelete"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s", sep->arg[0]);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!my_bot->DeleteBot()) {
		c->Message(Chat::White, "Failed to delete '%s' due to database error", my_bot->GetCleanName());
		return;
	}

	auto bid = my_bot->GetBotID();
	std::string bot_name = my_bot->GetCleanName();

	my_bot->Camp(false);

	c->Message(Chat::White, "Successfully deleted bot '%s' (id: %i)", bot_name.c_str(), bid);
}

void bot_command_follow_distance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_follow_distance", sep->arg[0], "botfollowdistance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [set] [distance] ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "usage: %s [clear] ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	uint32 bfd = BOT_FOLLOW_DISTANCE_DEFAULT;
	bool set_flag = false;
	int ab_arg = 2;

	if (!strcasecmp(sep->arg[1], "set")) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "A numeric [distance] is required to use this command");
			return;
		}

		bfd = Strings::ToInt(sep->arg[2]);
		if (bfd < 1)
			bfd = 1;
		if (bfd > BOT_FOLLOW_DISTANCE_DEFAULT_MAX)
			bfd = BOT_FOLLOW_DISTANCE_DEFAULT_MAX;
		set_flag = true;
		ab_arg = 3;
	}
	else if (strcasecmp(sep->arg[1], "clear")) {
		c->Message(Chat::White, "This command requires a [set | clear] argument");
		return;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		bot_iter->SetFollowDistance(bfd);
		if (ab_type != ActionableBots::ABT_All && !database.botdb.SaveFollowDistance(bot_iter->GetBotID(), bfd)) {
			return;
		}

		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (!database.botdb.SaveAllFollowDistances(c->CharacterID(), bfd)) {
			return;
		}

		c->Message(Chat::White, "%s all of your bot follow distances", set_flag ? "Set" : "Cleared");
	}
	else {
		c->Message(Chat::White, "%s %i of your spawned bot follow distances", (set_flag ? "Set" : "Cleared"), bot_count);
	}
}

void bot_command_inspect_message(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_inspect_message", sep->arg[0], "botinspectmessage"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [set | clear] ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "Notes:");
		if (c->ClientVersion() >= EQ::versions::ClientVersion::SoF) {
			c->Message(Chat::White, "- Self-inspect and type your bot's inspect message");
			c->Message(Chat::White, "- Close the self-inspect window to update the server");
			c->Message(Chat::White, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		else {
			c->Message(Chat::White, "- Self-inspect and type your bot's inspect message");
			c->Message(Chat::White, "- Close the self-inspect window");
			c->Message(Chat::White, "- Self-inspect again to update the server");
			c->Message(Chat::White, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	bool set_flag = false;
	if (!strcasecmp(sep->arg[1], "set")) {
		set_flag = true;
	}
	else if (strcasecmp(sep->arg[1], "clear")) {
		c->Message(Chat::White, "This command requires a [set | clear] argument");
		return;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	const auto client_message_struct = &c->GetInspectMessage();

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		auto bot_message_struct = &bot_iter->GetInspectMessage();
		memset(bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (ab_type != ActionableBots::ABT_All && !database.botdb.SaveInspectMessage(bot_iter->GetBotID(), *bot_message_struct)) {
			return;
		}

		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		InspectMessage_Struct bot_message_struct;
		memset(&bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(&bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (!database.botdb.SaveAllInspectMessages(c->CharacterID(), bot_message_struct)) {
			return;
		}

		c->Message(Chat::White, "%s all of your bot inspect messages", set_flag ? "Set" : "Cleared");
	}
	else {
		c->Message(Chat::White, "%s %i of your spawned bot inspect messages", set_flag ? "Set" : "Cleared", bot_count);
	}
}

void bot_command_list_bots(Client *c, const Seperator *sep)
{
	enum {
		FilterClass,
		FilterRace,
		FilterName,
		FilterCount,
		MaskClass = (1 << FilterClass),
		MaskRace = (1 << FilterRace),
		MaskName = (1 << FilterName)
	};

	if (helper_command_alias_fail(c, "bot_command_list", sep->arg[0], "botlist")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} (account) ([class] [value]) ([race] [value]) ([name] [partial-full])",
				sep->arg[0]
			).c_str()
		);
		c->Message(Chat::White, "Note: Filter criteria is orderless and optional.");
		return;
	}

	bool Account = false;
	int seps = 1;
	uint32 filter_value[FilterCount];
	int name_criteria_arg = 0;
	memset(&filter_value, 0, sizeof(uint32) * FilterCount);

	int filter_mask = 0;
	if (!strcasecmp(sep->arg[1], "account")) {
		Account = true;
		seps = 2;
	}

	for (int i = seps; i < (FilterCount * 2); i += 2) {
		if (sep->arg[i][0] == '\0') {
			break;
		}

		if (!strcasecmp(sep->arg[i], "class")) {
			filter_mask |= MaskClass;
			filter_value[FilterClass] = Strings::ToInt(sep->arg[i + 1]);
			continue;
		}

		if (!strcasecmp(sep->arg[i], "race")) {
			filter_mask |= MaskRace;
			filter_value[FilterRace] = Strings::ToInt(sep->arg[i + 1]);
			continue;
		}

		if (!strcasecmp(sep->arg[i], "name")) {
			filter_mask |= MaskName;
			name_criteria_arg = (i + 1);
			continue;
		}

		c->Message(Chat::White, "A numeric value or name is required to use the filter property of this command (f: '%s', v: '%s')", sep->arg[i], sep->arg[i + 1]);
		return;
	}

	std::list<BotsAvailableList> bots_list;
	if (!database.botdb.LoadBotsList(c->CharacterID(), bots_list, Account)) {
		return;
	}

	if (bots_list.empty()) {
		c->Message(Chat::White, "You have no bots.");
		return;
	}

	auto bot_count = 0;
	auto bots_owned = 0;
	auto bot_number = 1;
	for (auto bots_iter : bots_list) {
		if (filter_mask) {
			if ((filter_mask & MaskClass) && filter_value[FilterClass] != bots_iter.class_) {
				continue;
			}

			if ((filter_mask & MaskRace) && filter_value[FilterRace] != bots_iter.race) {
				continue;
			}

			if (filter_mask & MaskName) {
				std::string name_criteria = sep->arg[name_criteria_arg];
				std::transform(name_criteria.begin(), name_criteria.end(), name_criteria.begin(), ::tolower);
				std::string name_check = bots_iter.bot_name;
				std::transform(name_check.begin(), name_check.end(), name_check.begin(), ::tolower);
				if (name_check.find(name_criteria) == std::string::npos) {
					continue;
				}
			}
		}

		auto* bot = entity_list.GetBotByBotName(bots_iter.bot_name);

		c->Message(
			Chat::White,
			fmt::format(
				"Bot {} | {} is a Level {} {} {} {} owned by {}.",
				bot_number,
				(
					(c->CharacterID() == bots_iter.owner_id && !bot) ?
						Saylink::Silent(
							fmt::format("^spawn {}", bots_iter.bot_name),
							bots_iter.bot_name
						) :
						bots_iter.bot_name
				),
				bots_iter.level,
				GetGenderName(bots_iter.gender),
				GetRaceIDName(bots_iter.race),
				GetClassIDName(bots_iter.class_),
				bots_iter.owner_name
			).c_str()
		);

		if (c->CharacterID() == bots_iter.owner_id) {
			bots_owned++;
		}

		bot_count++;
		bot_number++;
	}

	if (!bot_count) {
		c->Message(Chat::White, "You have no bots meeting this criteria.");
		return;
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} Of {} bot{} shown, {} {} owned by you.",
				bot_count,
				bots_list.size(),
				bot_count != 1 ? "s" : "",
				bots_owned,
				bots_owned != 1 ? "are" : "is"
			).c_str()
		);

		c->Message(Chat::White, "Note: You can spawn any owned bots by clicking their name if they are not already spawned.");

		c->Message(Chat::White, "Your bot creation limits are as follows:");

		const auto overall_bot_creation_limit = c->GetBotCreationLimit();

		c->Message(
			Chat::White,
			fmt::format(
				"Overall | {} Bot{}",
				overall_bot_creation_limit,
				overall_bot_creation_limit != 1 ? "s" : ""
			).c_str()
		);

		for (uint8 class_id = Class::Warrior; class_id <= Class::Berserker; class_id++) {
			auto class_creation_limit = c->GetBotCreationLimit(class_id);

			if (class_creation_limit != overall_bot_creation_limit) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} | {} Bot{}",
						GetClassIDName(class_id),
						class_creation_limit,
						class_creation_limit != 1 ? "s" : ""
					).c_str()
				);
			}
		}
	}
}

void bot_command_out_of_combat(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_out_of_combat", sep->arg[0], "botoutofcombat"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([option: on | off]) ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::string arg1 = sep->arg[1];

	bool behavior_state = false;
	bool toggle_behavior = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		behavior_state = true;
		toggle_behavior = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_behavior = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (toggle_behavior)
			bot_iter->SetAltOutOfCombatBehavior(!bot_iter->GetAltOutOfCombatBehavior());
		else
			bot_iter->SetAltOutOfCombatBehavior(behavior_state);

		helper_bot_out_of_combat(c, bot_iter);
	}
}

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

void bot_command_spawn(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spawn", sep->arg[0], "botspawn")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [bot_name] [optional: silent]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto bot_character_level = c->GetBotRequiredLevel();
	if (
		bot_character_level >= 0 &&
		c->GetLevel() < bot_character_level &&
		!c->GetGM()
		) {
		c->Message(
			Chat::White,
			fmt::format(
				"You must be level {} to spawn bots.",
				bot_character_level
			).c_str()
		);
		return;
	}

	if (!Bot::CheckSpawnConditions(c)) {
		return;
	}

	auto bot_spawn_limit = c->GetBotSpawnLimit();
	auto spawned_bot_count = Bot::SpawnedBotCount(c->CharacterID());

	if (
		bot_spawn_limit >= 0 &&
		spawned_bot_count >= bot_spawn_limit &&
		!c->GetGM()
		) {
		std::string message;
		if (bot_spawn_limit) {
			message = fmt::format(
				"You cannot have more than {} spawned bot{}.",
				bot_spawn_limit,
				bot_spawn_limit != 1 ? "s" : ""
			);
		} else {
			message = "You are not currently allowed to spawn any bots.";
		}

		c->Message(Chat::White, message.c_str());
		return;
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must specify a name to use this command.");
		return;
	}

	std::string bot_name = sep->arg[1];

	uint32 bot_id = 0;
	uint8 bot_class = Class::None;
	if (!database.botdb.LoadBotID(bot_name, bot_id, bot_class)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to load bot ID for '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	auto bot_spawn_limit_class = c->GetBotSpawnLimit(bot_class);
	auto spawned_bot_count_class = Bot::SpawnedBotCount(c->CharacterID(), bot_class);

	if (
		bot_spawn_limit_class >= 0 &&
		spawned_bot_count_class >= bot_spawn_limit_class &&
		!c->GetGM()
		) {
		std::string message;

		if (bot_spawn_limit_class) {
			message = fmt::format(
				"You cannot have more than {} spawned {} bot{}.",
				bot_spawn_limit_class,
				GetClassIDName(bot_class),
				bot_spawn_limit_class != 1 ? "s" : ""
			);
		} else {
			message = fmt::format(
				"You are not currently allowed to spawn any {} bots.",
				GetClassIDName(bot_class)
			);
		}

		c->Message(Chat::White, message.c_str());
		return;
	}

	auto bot_character_level_class = c->GetBotRequiredLevel(bot_class);
	if (
		bot_character_level_class >= 0 &&
		c->GetLevel() < bot_character_level_class &&
		!c->GetGM()
		) {
		c->Message(
			Chat::White,
			fmt::format(
				"You must be level {} to spawn {} bots.",
				bot_character_level_class,
				GetClassIDName(bot_class)
			).c_str()
		);
		return;
	}

	if (!bot_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"You don't own a bot named '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	if (entity_list.GetMobByBotID(bot_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"'{}' is already spawned.",
				bot_name
			).c_str()
		);
		return;
	}

	auto my_bot = Bot::LoadBot(bot_id);
	if (!my_bot) {
		c->Message(
			Chat::White,
			fmt::format(
				"Invalid bot '{}' (ID {})",
				bot_name,
				bot_id
			).c_str()
		);
		return;
	}

	if (!my_bot->Spawn(c)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to spawn '{}' (ID {})",
				bot_name,
				bot_id
			).c_str()
		);
		safe_delete(my_bot);
		return;
	}

	static std::string bot_spawn_message[17] = {
		"I am ready to fight!", // DEFAULT
		"A solid weapon is my ally!", // Class::Warrior
		"The pious shall never die!", // Class::Cleric
		"I am the symbol of Light!", // Class::Paladin
		"There are enemies near!", // Class::Ranger
		"Out of the shadows, I step!", // Class::ShadowKnight
		"Nature's fury shall be wrought!", // Class::Druid
		"Your punishment will be my fist!", // Class::Monk
		"Music is the overture of battle! ", // BARD
		"Daggers into the backs of my enemies!", // Class::Rogue
		"More bones to grind!", // Class::Shaman
		"Death is only the beginning!", // Class::Necromancer
		"I am the harbinger of demise!", // Class::Wizard
		"The elements are at my command!", // Class::Magician
		"No being can resist my charm!", // Class::Enchanter
		"Battles are won by hand and paw!", // Class::Beastlord
		"My bloodthirst shall not be quenched!" // Class::Berserker
	};

	uint8 message_index = 0;
	if (c->GetBotOption(Client::booSpawnMessageClassSpecific)) {
		message_index = VALIDATECLASSID(my_bot->GetClass());
	}

	std::string silent_confirm = sep->arg[2];
	bool silentTell = false;

	if (!silent_confirm.compare("silent")) {
		silentTell = true;
	}

	if (!silentTell && c->GetBotOption(Client::booSpawnMessageSay)) {
		Bot::BotGroupSay(my_bot, bot_spawn_message[message_index].c_str());
	}
	else if (!silentTell && c->GetBotOption(Client::booSpawnMessageTell)) {
		my_bot->OwnerMessage(bot_spawn_message[message_index]);
	}
}

void bot_command_stance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_stance", sep->arg[0], "botstance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [current | value: 1-9] ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		c->Message(
			Chat::White,
			fmt::format(
				"Value: {} ({}), {} ({}), {} ({}), {} ({}), {} ({}), {} ({}), {} ({}), {} ({}), {} ({})",
				Stance::Passive,
				Stance::GetName(Stance::Passive),
				Stance::Balanced,
				Stance::GetName(Stance::Balanced),
				Stance::Efficient,
				Stance::GetName(Stance::Efficient),
				Stance::Reactive,
				Stance::GetName(Stance::Reactive),
				Stance::Aggressive,
				Stance::GetName(Stance::Aggressive),
				Stance::Assist,
				Stance::GetName(Stance::Assist),
				Stance::Burn,
				Stance::GetName(Stance::Burn),
				Stance::Efficient2,
				Stance::GetName(Stance::Efficient2),
				Stance::AEBurn,
				Stance::GetName(Stance::AEBurn)
			).c_str()
		);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	bool current_flag = false;
	uint8 bst = Stance::Unknown;

	if (!strcasecmp(sep->arg[1], "current"))
		current_flag = true;
	else if (sep->IsNumber(1)) {
		bst = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1]));
		if (!Stance::IsValid(bst)) {
			bst = Stance::Unknown;
		}
	}

	if (!current_flag && bst == Stance::Unknown) {
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
				Stance::GetName(bot_iter->GetBotStance()),
				bot_iter->GetBotStance()
			).c_str()
		);
	}
}

void bot_command_stop_melee_level(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_stop_melee_level", sep->arg[0], "botstopmeleelevel"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [current | reset | sync | value: 0-255]", sep->arg[0]);
		c->Message(Chat::White, "note: Only caster or hybrid class bots may be modified");
		c->Message(Chat::White, "note: Use [reset] to set stop melee level to server rule");
		c->Message(Chat::White, "note: Use [sync] to set stop melee level to current bot level");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}
	if (!IsCasterClass(my_bot->GetClass()) && !IsHybridClass(my_bot->GetClass())) {
		c->Message(Chat::White, "You must <target> a caster or hybrid class bot to use this command");
		return;
	}

	uint8 sml = RuleI(Bots, CasterStopMeleeLevel);

	if (sep->IsNumber(1)) {
		sml = Strings::ToInt(sep->arg[1]);
	}
	else if (!strcasecmp(sep->arg[1], "sync")) {
		sml = my_bot->GetLevel();
	}
	else if (!strcasecmp(sep->arg[1], "current")) {
		c->Message(Chat::White, "My current melee stop level is %u", my_bot->GetStopMeleeLevel());
		return;
	}
	else if (strcasecmp(sep->arg[1], "reset")) {
		c->Message(Chat::White, "A [current] or [reset] argument, or numeric [value] is required to use this command");
		return;
	}
	// [reset] falls through with initialization value

	my_bot->SetStopMeleeLevel(sml);
	database.botdb.SaveStopMeleeLevel(my_bot->GetBotID(), sml);

	c->Message(Chat::White, "Successfully set stop melee level for %s to %u", my_bot->GetCleanName(), sml);
}

void bot_command_summon(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_summon", sep->arg[0], "botsummon")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;

	std::string class_race_arg = sep->arg[1];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, !class_race_check ? sep->arg[2] : nullptr, class_race_check ? atoi(sep->arg[2]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.remove(nullptr);

	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		bot_iter->WipeHateList();
		bot_iter->SetTarget(nullptr);
		bot_iter->Teleport(c->GetPosition());
		bot_iter->DoAnim(0);

		if (!bot_iter->HasPet()) {
			continue;
		}

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->SetTarget(nullptr);
		bot_iter->GetPet()->Teleport(c->GetPosition());
	}

	if (sbl.size() == 1) {
		c->Message(
			Chat::White,
			fmt::format(
				"Summoned {} to you.",
				sbl.front() ? sbl.front()->GetCleanName() : "no one"
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::White,
			fmt::format(
				"Summoned {} bots to you.",
				sbl.size()
			).c_str()
		);
	}
}

void bot_command_toggle_archer(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_toggle_archer", sep->arg[0], "bottogglearcher")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([option: on | off]) ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::string arg1 = sep->arg[1];

	bool archer_state = false;
	bool toggle_archer = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		archer_state = true;
		toggle_archer = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_archer = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]) == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		if (toggle_archer) {
			bot_iter->SetBotArcherySetting(!bot_iter->IsBotArcher(), true);
		}
		else {
			bot_iter->SetBotArcherySetting(archer_state, true);
		}
		bot_iter->ChangeBotArcherWeapons(bot_iter->IsBotArcher());

		if (bot_iter->GetClass() == Class::Ranger && bot_iter->GetLevel() >= 61) {
			bot_iter->SetRangerAutoWeaponSelect(bot_iter->IsBotArcher());
		}
	}
}

void bot_command_toggle_helm(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_toggle_helm", sep->arg[0], "bottogglehelm"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([option: on | off]) ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::string arg1 = sep->arg[1];

	bool helm_state = false;
	bool toggle_helm = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		helm_state = true;
		toggle_helm = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_helm = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (toggle_helm)
			bot_iter->SetShowHelm(!bot_iter->GetShowHelm());
		else
			bot_iter->SetShowHelm(helm_state);

		if (ab_type != ActionableBots::ABT_All) {
			if (!database.botdb.SaveHelmAppearance(bot_iter->GetBotID(), bot_iter->GetShowHelm())) {
				return;
			}

			EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
			SpawnAppearance_Struct* saptr = (SpawnAppearance_Struct*)outapp->pBuffer;
			saptr->spawn_id = bot_iter->GetID();
			saptr->type = AppearanceType::ShowHelm;
			saptr->parameter = bot_iter->GetShowHelm();

			entity_list.QueueClients(bot_iter, outapp);
			safe_delete(outapp);

			//helper_bot_appearance_form_update(bot_iter);
		}
		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (toggle_helm) {
			database.botdb.ToggleAllHelmAppearances(c->CharacterID());
		}
		else {
			database.botdb.SaveAllHelmAppearances(c->CharacterID(), helm_state);
		}

		c->Message(Chat::White, "%s all of your bot show helm flags", toggle_helm ? "Toggled" : (helm_state ? "Set" : "Cleared"));
	}
	else {
		c->Message(Chat::White, "%s %i of your spawned bot show helm flags", toggle_helm ? "Toggled" : (helm_state ? "Set" : "Cleared"), bot_count);
	}

	// Notes:
	/*
	[CLIENT OPCODE TEST]
	[10-16-2015 :: 14:57:56] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)
	[10-16-2015 :: 14:57:56] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)

	[10-16-2015 :: 14:58:02] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)
	[10-16-2015 :: 14:58:02] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)

	[BOT OPCODE TEST]
	[10-16-2015 :: 22:15:34] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 167]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 6E 00                                     | m on.

	[10-16-2015 :: 22:15:34] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 01 00 00 00 - showhelm = true

	[10-16-2015 :: 22:15:40] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 168]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 66 66 00                                  | m off.

	[10-16-2015 :: 22:15:40] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 00 00 00 00 - showhelm = false

	*** Bot did not update using the OP_SpawnAppearance packet with AppearanceType::ShowHelm appearance type ***
	*/
}

void bot_command_update(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_update", sep->arg[0], "botupdate"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);
	if (sbl.empty()) {
		c->Message(Chat::White, "You currently have no spawned bots");
		return;
	}

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter || bot_iter->IsEngaged() || bot_iter->GetLevel() == c->GetLevel())
			continue;

		bot_iter->SetPetChooser(false);
		bot_iter->CalcBotStats(c->GetBotOption(Client::booStatsUpdate));
		bot_iter->SendAppearancePacket(AppearanceType::WhoLevel, bot_iter->GetLevel(), true, true);
		++bot_count;
	}

	c->Message(Chat::White, "Updated %i of your %i spawned bots", bot_count, sbl.size());
}
