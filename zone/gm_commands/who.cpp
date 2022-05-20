#include "../client.h"

void command_who(Client *c, const Seperator *sep)
{
	std::string query = SQL(
		SELECT
		character_data.account_id,
		character_data.name,
		character_data.zone_id,
		character_data.zone_instance,
		COALESCE(
			(
				SELECT guilds.name FROM guilds WHERE id = (
					(
						SELECT guild_id FROM guild_members WHERE char_id = character_data.id
					)
				)
			),
			""
		) AS guild_name,
		character_data.level,
		character_data.race,
		character_data.class,
		COALESCE(
			(
				SELECT account.status FROM account WHERE account.id = character_data.account_id LIMIT 1
			),
			0
		) AS account_status,
		COALESCE(
			(
				SELECT account.name FROM account WHERE account.id = character_data.account_id LIMIT 1
			),
			0
		) AS account_name,
		COALESCE(
			(
				SELECT account_ip.ip FROM account_ip WHERE account_ip.accid = character_data.account_id ORDER BY account_ip.lastused DESC LIMIT 1
			),
			""
		) AS account_ip
		FROM
		character_data
		WHERE
		last_login > (UNIX_TIMESTAMP() - 600)
		ORDER BY
		character_data.name;
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return;
	}

	std::string search_string;

	if (sep->arg[1]) {
		search_string = str_tolower(sep->arg[1]);
	}

	int found_count = 0;

	c->Message(Chat::Who, "Players in EverQuest:");
	c->Message(Chat::Who, "------------------------------");

	for (auto row : results) {
		auto account_id = std::stoul(row[0]);
		std::string player_name = row[1];
		auto zone_id = std::stoul(row[2]);
		std::string zone_short_name = ZoneName(zone_id);
		std::string zone_long_name = ZoneLongName(zone_id);
		auto zone_instance = std::stoul(row[3]);
		std::string guild_name = row[4];
		auto player_level = std::stoul(row[5]);
		auto player_race = std::stoul(row[6]);
		auto player_class = std::stoul(row[7]);
		auto account_status = std::stoul(row[8]);
		std::string account_name = row[9];
		std::string account_ip = row[10];
		std::string base_class_name = GetClassIDName(static_cast<uint8>(player_class));
		std::string displayed_race_name = GetRaceIDName(static_cast<uint16>(player_race));

		if (search_string.length()) {
			bool found_search_term = (
				str_tolower(player_name).find(search_string) != std::string::npos ||
				str_tolower(zone_short_name).find(search_string) != std::string::npos ||
				str_tolower(displayed_race_name).find(search_string) != std::string::npos ||
				str_tolower(base_class_name).find(search_string) != std::string::npos ||
				str_tolower(guild_name).find(search_string) != std::string::npos ||
				str_tolower(account_name).find(search_string) != std::string::npos ||
				str_tolower(account_ip).find(search_string) != std::string::npos
			);

			if (!found_search_term) {
				continue;
			}
		}

		std::string displayed_guild_name;
		if (guild_name.length()) {
			displayed_guild_name = EQ::SayLinkEngine::GenerateQuestSaylink(
				fmt::format(
					"#who \"{}\"",
					guild_name
				),
				false,
				fmt::format(
					"<{}>",
					guild_name
				)
			);
		}

		auto goto_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format(
				"#goto {}",
				player_name
			),
			false,
			"Goto"
		);

		auto summon_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format(
				"#summon {}",
				player_name
			),
			false,
			"Summon"
		);

		std::string display_class_name = GetClassIDName(
			static_cast<uint8>(player_class),
			static_cast<uint8>(player_level)
		);

		auto class_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format(
				"#who {}",
				base_class_name
			),
			false,
			display_class_name
		);

		auto race_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format(
				"#who %s",
				displayed_race_name
			),
			false,
			displayed_race_name
		);

		auto zone_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format(
				"#who {}",
				zone_short_name
			),
			false,
			zone_long_name
		);

		auto account_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format(
				"#who {}",
				account_name
			),
			false,
			account_name
		);

		auto account_ip_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format(
				"#who {}",
				account_ip
			),
			false,
			account_ip
		);

		auto status_level = (
			account_status ?
			fmt::format(
				"* {} * ",
				EQ::constants::GetAccountStatusName(account_status)
			) :
			""
		);

		auto version_string = (
			zone_instance ?
			fmt::format(
				" ({})",
				zone_instance
			) :
			""
		);

		c->Message(
			Chat::Who,
			fmt::format(
				"{}[{} {} ({})] {} ({}) ({}) ({}) {} ZONE: {}{} ({} | {})",
				status_level,
				player_level,
				class_saylink,
				base_class_name,
				player_name,
				race_saylink,
				account_saylink,
				account_ip_saylink,
				displayed_guild_name,
				zone_saylink,
				version_string,
				goto_saylink,
				summon_saylink
			).c_str()
		);

		found_count++;
	}

	std::string count_string = found_count == 1 ? "is" : "are";

	std::string message = (
		found_count ?
		fmt::format(
			"There {} {} player{} in EverQuest.",
			count_string,
			found_count,
			found_count > 1 ? "s" : ""
		) :
		"There are no players in EverQuest that match those filters."
	);

	c->Message(
		Chat::Who,
		message.c_str()
	);
}

