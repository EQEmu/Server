#include "../client.h"

void command_who(Client *c, const Seperator *sep)
{
	std::string query =
					SQL (
						SELECT
						character_data.account_id,
						character_data.name,
						character_data.zone_id,
						character_data.zone_instance,
						COALESCE(
							(
								select
							guilds.name
							from
				guilds
							where
				id                           = (
					(
						select
							guild_id
				from
							guild_members
				where
							char_id          = character_data.id
						)
						)
						),
							""
						) as guild_name,
						character_data.level,
						character_data.race,
						character_data.class,
						COALESCE(
							(
								select
							account.status
							from
								account
							where
							account.id       = character_data.account_id
							LIMIT
							1
						), 0
						) as account_status,
						COALESCE(
							(
								select
							account.name
							from
								account
							where
							account.id       = character_data.account_id
							LIMIT
							1
						),
							0
						) as account_name,
						COALESCE(
							(
								select
							account_ip.ip
							from
								account_ip
							where
							account_ip.accid = character_data.account_id
							ORDER BY
							account_ip.lastused DESC
							LIMIT
							1
						),
							""
						) as account_ip
							FROM
							character_data
							WHERE
							last_login > (UNIX_TIMESTAMP() - 600)
							ORDER BY
							character_data.name;
					);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	if (results.RowCount() == 0) {
		c->Message(Chat::Yellow, "No results found");
		return;
	}

	std::string search_string;

	if (sep->arg[1]) {
		search_string = str_tolower(sep->arg[1]);
	}

	int found_count = 0;

	c->Message(Chat::Magenta, "Players in EverQuest");
	c->Message(Chat::Magenta, "--------------------");

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto        account_id          = static_cast<uint32>(atoi(row[0]));
		std::string player_name         = row[1];
		auto        zone_id             = static_cast<uint32>(atoi(row[2]));
		std::string zone_short_name     = ZoneName(zone_id);
		auto        zone_instance       = static_cast<uint32>(atoi(row[3]));
		std::string guild_name          = row[4];
		auto        player_level        = static_cast<uint32>(atoi(row[5]));
		auto        player_race         = static_cast<uint32>(atoi(row[6]));
		auto        player_class        = static_cast<uint32>(atoi(row[7]));
		auto        account_status      = static_cast<uint32>(atoi(row[8]));
		std::string account_name        = row[9];
		std::string account_ip          = row[10];
		std::string base_class_name     = GetClassIDName(static_cast<uint8>(player_class), 1);
		std::string displayed_race_name = GetRaceIDName(static_cast<uint16>(player_race));

		if (search_string.length() > 0) {
			bool found_search_term =
					 (
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
		if (guild_name.length() > 0) {
			displayed_guild_name = EQ::SayLinkEngine::GenerateQuestSaylink(
				StringFormat(
					"#who \"%s\"",
					guild_name.c_str()),
				false,
				StringFormat("<%s>", guild_name.c_str())
			);
		}

		std::string goto_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			StringFormat("#goto %s", player_name.c_str()), false, "Goto"
		);

		std::string display_class_name = GetClassIDName(
			static_cast<uint8>(player_class),
			static_cast<uint8>(player_level));

		c->Message(
			5, "%s[%u %s] %s (%s) %s ZONE: %s (%u) (%s) (%s) (%s)",
			(account_status > 0 ? "* GM * " : ""),
			player_level,
			EQ::SayLinkEngine::GenerateQuestSaylink(
				StringFormat("#who %s", base_class_name.c_str()),
				false,
				display_class_name
			).c_str(),
			player_name.c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(
				StringFormat("#who %s", displayed_race_name.c_str()),
				false,
				displayed_race_name
			).c_str(),
			displayed_guild_name.c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(
				StringFormat("#who %s", zone_short_name.c_str()),
				false,
				zone_short_name
			).c_str(),
			zone_instance,
			goto_saylink.c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(
				StringFormat("#who %s", account_name.c_str()),
				false,
				account_name
			).c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(
				StringFormat("#who %s", account_ip.c_str()),
				false,
				account_ip
			).c_str()
		);

		found_count++;
	}

	std::string message = (
		found_count > 0 ?
			StringFormat("There is %i player(s) in EverQuest", found_count).c_str() :
			"There are no players in EverQuest that match those who filters."
	);

	c->Message(Chat::Magenta, message.c_str());
}

