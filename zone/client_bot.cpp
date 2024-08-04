#include "bot.h"
#include "client.h"

bool Client::GetBotOption(BotOwnerOption boo) const {
	if (boo < _booCount) {
		return bot_owner_options[boo];
	}

	return false;
}

void Client::SetBotOption(BotOwnerOption boo, bool flag) {
	if (boo < _booCount) {
		bot_owner_options[boo] = flag;
	}
}

uint32 Client::GetBotCreationLimit(uint8 class_id)
{
	uint32 bot_creation_limit = RuleI(Bots, CreationLimit);

	const auto bucket_name = fmt::format(
		"bot_creation_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	auto bucket_value = GetBucket(bucket_name);
	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_creation_limit = Strings::ToUnsignedInt(bucket_value);
	}

	return bot_creation_limit;
}

int Client::GetBotRequiredLevel(uint8 class_id)
{
	int bot_character_level = RuleI(Bots, BotCharacterLevel);

	const auto bucket_name = fmt::format(
		"bot_required_level{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	auto bucket_value = GetBucket(bucket_name);
	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_character_level = Strings::ToInt(bucket_value);
	}

	return bot_character_level;
}

int Client::GetBotSpawnLimit(uint8 class_id)
{
	int bot_spawn_limit = RuleI(Bots, SpawnLimit);

	const auto bucket_name = fmt::format(
		"bot_spawn_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	auto bucket_value = GetBucket(bucket_name);
	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_spawn_limit = Strings::ToInt(bucket_value);
		return bot_spawn_limit;
	}

	if (RuleB(Bots, QuestableSpawnLimit)) {
		const auto query = fmt::format(
			"SELECT `value` FROM `quest_globals` WHERE `name` = '{}' AND `charid` = {} LIMIT 1",
			bucket_name,
			CharacterID()
		);

		auto results = database.QueryDatabase(query); // use 'database' for non-bot table calls
		if (!results.Success() || !results.RowCount()) {
			return bot_spawn_limit;
		}

		auto row = results.begin();
		bot_spawn_limit = Strings::ToInt(row[0]);
	}

	return bot_spawn_limit;
}

void Client::SetBotCreationLimit(uint32 new_creation_limit, uint8 class_id)
{
	const auto bucket_name = fmt::format(
		"bot_creation_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_creation_limit));
}

void Client::SetBotRequiredLevel(int new_required_level, uint8 class_id)
{
	const auto bucket_name = fmt::format(
		"bot_required_level{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_required_level));
}

void Client::SetBotSpawnLimit(int new_spawn_limit, uint8 class_id)
{
	const auto bucket_name = fmt::format(
		"bot_spawn_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_spawn_limit));
}

void Client::CampAllBots(uint8 class_id)
{
	Bot::BotOrderCampAll(this, class_id);
}
