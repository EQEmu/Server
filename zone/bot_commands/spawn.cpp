#include "../client.h"
#include "../bot_command.h"

void bot_command_spawn(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spawn", sep->arg[0], "botspawn")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [bot_name]",
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
	if (!database.botdb.LoadBotID(c->CharacterID(), bot_name, bot_id, bot_class)) {
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

	if (c->GetBotOption(Client::booSpawnMessageSay)) {
		Bot::BotGroupSay(my_bot, bot_spawn_message[message_index].c_str());
	} else if (c->GetBotOption(Client::booSpawnMessageTell)) {
		my_bot->OwnerMessage(bot_spawn_message[message_index]);
	}
}
