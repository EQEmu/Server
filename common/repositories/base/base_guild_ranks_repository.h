/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 * 
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
 */

#ifndef EQEMU_BASE_GUILD_RANKS_REPOSITORY_H
#define EQEMU_BASE_GUILD_RANKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGuildRanksRepository {
public:
	struct GuildRanks {
		int         guild_id;
		int         rank;
		std::string title;
		int         can_hear;
		int         can_speak;
		int         can_invite;
		int         can_remove;
		int         can_promote;
		int         can_demote;
		int         can_motd;
		int         can_warpeace;
	};

	static std::string PrimaryKey()
	{
		return std::string("guild_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"guild_id",
			"rank",
			"title",
			"can_hear",
			"can_speak",
			"can_invite",
			"can_remove",
			"can_promote",
			"can_demote",
			"can_motd",
			"can_warpeace",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("guild_ranks");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			ColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static GuildRanks NewEntity()
	{
		GuildRanks entry{};

		entry.guild_id     = 0;
		entry.rank         = 0;
		entry.title        = "";
		entry.can_hear     = 0;
		entry.can_speak    = 0;
		entry.can_invite   = 0;
		entry.can_remove   = 0;
		entry.can_promote  = 0;
		entry.can_demote   = 0;
		entry.can_motd     = 0;
		entry.can_warpeace = 0;

		return entry;
	}

	static GuildRanks GetGuildRanksEntry(
		const std::vector<GuildRanks> &guild_rankss,
		int guild_ranks_id
	)
	{
		for (auto &guild_ranks : guild_rankss) {
			if (guild_ranks.guild_id == guild_ranks_id) {
				return guild_ranks;
			}
		}

		return NewEntity();
	}

	static GuildRanks FindOne(
		Database& db,
		int guild_ranks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				guild_ranks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildRanks entry{};

			entry.guild_id     = atoi(row[0]);
			entry.rank         = atoi(row[1]);
			entry.title        = row[2] ? row[2] : "";
			entry.can_hear     = atoi(row[3]);
			entry.can_speak    = atoi(row[4]);
			entry.can_invite   = atoi(row[5]);
			entry.can_remove   = atoi(row[6]);
			entry.can_promote  = atoi(row[7]);
			entry.can_demote   = atoi(row[8]);
			entry.can_motd     = atoi(row[9]);
			entry.can_warpeace = atoi(row[10]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guild_ranks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_ranks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		GuildRanks guild_ranks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(guild_ranks_entry.guild_id));
		update_values.push_back(columns[1] + " = " + std::to_string(guild_ranks_entry.rank));
		update_values.push_back(columns[2] + " = '" + EscapeString(guild_ranks_entry.title) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(guild_ranks_entry.can_hear));
		update_values.push_back(columns[4] + " = " + std::to_string(guild_ranks_entry.can_speak));
		update_values.push_back(columns[5] + " = " + std::to_string(guild_ranks_entry.can_invite));
		update_values.push_back(columns[6] + " = " + std::to_string(guild_ranks_entry.can_remove));
		update_values.push_back(columns[7] + " = " + std::to_string(guild_ranks_entry.can_promote));
		update_values.push_back(columns[8] + " = " + std::to_string(guild_ranks_entry.can_demote));
		update_values.push_back(columns[9] + " = " + std::to_string(guild_ranks_entry.can_motd));
		update_values.push_back(columns[10] + " = " + std::to_string(guild_ranks_entry.can_warpeace));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				guild_ranks_entry.guild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildRanks InsertOne(
		Database& db,
		GuildRanks guild_ranks_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(guild_ranks_entry.guild_id));
		insert_values.push_back(std::to_string(guild_ranks_entry.rank));
		insert_values.push_back("'" + EscapeString(guild_ranks_entry.title) + "'");
		insert_values.push_back(std::to_string(guild_ranks_entry.can_hear));
		insert_values.push_back(std::to_string(guild_ranks_entry.can_speak));
		insert_values.push_back(std::to_string(guild_ranks_entry.can_invite));
		insert_values.push_back(std::to_string(guild_ranks_entry.can_remove));
		insert_values.push_back(std::to_string(guild_ranks_entry.can_promote));
		insert_values.push_back(std::to_string(guild_ranks_entry.can_demote));
		insert_values.push_back(std::to_string(guild_ranks_entry.can_motd));
		insert_values.push_back(std::to_string(guild_ranks_entry.can_warpeace));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			guild_ranks_entry.guild_id = results.LastInsertedID();
			return guild_ranks_entry;
		}

		guild_ranks_entry = NewEntity();

		return guild_ranks_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GuildRanks> guild_ranks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &guild_ranks_entry: guild_ranks_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(guild_ranks_entry.guild_id));
			insert_values.push_back(std::to_string(guild_ranks_entry.rank));
			insert_values.push_back("'" + EscapeString(guild_ranks_entry.title) + "'");
			insert_values.push_back(std::to_string(guild_ranks_entry.can_hear));
			insert_values.push_back(std::to_string(guild_ranks_entry.can_speak));
			insert_values.push_back(std::to_string(guild_ranks_entry.can_invite));
			insert_values.push_back(std::to_string(guild_ranks_entry.can_remove));
			insert_values.push_back(std::to_string(guild_ranks_entry.can_promote));
			insert_values.push_back(std::to_string(guild_ranks_entry.can_demote));
			insert_values.push_back(std::to_string(guild_ranks_entry.can_motd));
			insert_values.push_back(std::to_string(guild_ranks_entry.can_warpeace));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<GuildRanks> All(Database& db)
	{
		std::vector<GuildRanks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildRanks entry{};

			entry.guild_id     = atoi(row[0]);
			entry.rank         = atoi(row[1]);
			entry.title        = row[2] ? row[2] : "";
			entry.can_hear     = atoi(row[3]);
			entry.can_speak    = atoi(row[4]);
			entry.can_invite   = atoi(row[5]);
			entry.can_remove   = atoi(row[6]);
			entry.can_promote  = atoi(row[7]);
			entry.can_demote   = atoi(row[8]);
			entry.can_motd     = atoi(row[9]);
			entry.can_warpeace = atoi(row[10]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GuildRanks> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<GuildRanks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildRanks entry{};

			entry.guild_id     = atoi(row[0]);
			entry.rank         = atoi(row[1]);
			entry.title        = row[2] ? row[2] : "";
			entry.can_hear     = atoi(row[3]);
			entry.can_speak    = atoi(row[4]);
			entry.can_invite   = atoi(row[5]);
			entry.can_remove   = atoi(row[6]);
			entry.can_promote  = atoi(row[7]);
			entry.can_demote   = atoi(row[8]);
			entry.can_motd     = atoi(row[9]);
			entry.can_warpeace = atoi(row[10]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, std::string where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_GUILD_RANKS_REPOSITORY_H
