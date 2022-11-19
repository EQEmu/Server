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
#include "../../strings.h"
#include <ctime>

class BaseGuildRanksRepository {
public:
	struct GuildRanks {
		uint32_t    guild_id;
		uint8_t     rank;
		std::string title;
		uint8_t     can_hear;
		uint8_t     can_speak;
		uint8_t     can_invite;
		uint8_t     can_remove;
		uint8_t     can_promote;
		uint8_t     can_demote;
		uint8_t     can_motd;
		uint8_t     can_warpeace;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("guild_ranks");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
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
		GuildRanks e{};

		e.guild_id     = 0;
		e.rank         = 0;
		e.title        = "";
		e.can_hear     = 0;
		e.can_speak    = 0;
		e.can_invite   = 0;
		e.can_remove   = 0;
		e.can_promote  = 0;
		e.can_demote   = 0;
		e.can_motd     = 0;
		e.can_warpeace = 0;

		return e;
	}

	static GuildRanks GetGuildRanks(
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
			GuildRanks e{};

			e.guild_id     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.rank         = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.title        = row[2] ? row[2] : "";
			e.can_hear     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.can_speak    = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.can_invite   = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.can_remove   = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.can_promote  = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.can_demote   = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.can_motd     = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.can_warpeace = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));

			return e;
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
		const GuildRanks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.guild_id));
		v.push_back(columns[1] + " = " + std::to_string(e.rank));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.title) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.can_hear));
		v.push_back(columns[4] + " = " + std::to_string(e.can_speak));
		v.push_back(columns[5] + " = " + std::to_string(e.can_invite));
		v.push_back(columns[6] + " = " + std::to_string(e.can_remove));
		v.push_back(columns[7] + " = " + std::to_string(e.can_promote));
		v.push_back(columns[8] + " = " + std::to_string(e.can_demote));
		v.push_back(columns[9] + " = " + std::to_string(e.can_motd));
		v.push_back(columns[10] + " = " + std::to_string(e.can_warpeace));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.guild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildRanks InsertOne(
		Database& db,
		GuildRanks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.rank));
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back(std::to_string(e.can_hear));
		v.push_back(std::to_string(e.can_speak));
		v.push_back(std::to_string(e.can_invite));
		v.push_back(std::to_string(e.can_remove));
		v.push_back(std::to_string(e.can_promote));
		v.push_back(std::to_string(e.can_demote));
		v.push_back(std::to_string(e.can_motd));
		v.push_back(std::to_string(e.can_warpeace));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.guild_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GuildRanks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.rank));
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back(std::to_string(e.can_hear));
			v.push_back(std::to_string(e.can_speak));
			v.push_back(std::to_string(e.can_invite));
			v.push_back(std::to_string(e.can_remove));
			v.push_back(std::to_string(e.can_promote));
			v.push_back(std::to_string(e.can_demote));
			v.push_back(std::to_string(e.can_motd));
			v.push_back(std::to_string(e.can_warpeace));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			GuildRanks e{};

			e.guild_id     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.rank         = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.title        = row[2] ? row[2] : "";
			e.can_hear     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.can_speak    = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.can_invite   = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.can_remove   = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.can_promote  = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.can_demote   = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.can_motd     = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.can_warpeace = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GuildRanks> GetWhere(Database& db, const std::string &where_filter)
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
			GuildRanks e{};

			e.guild_id     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.rank         = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.title        = row[2] ? row[2] : "";
			e.can_hear     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.can_speak    = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.can_invite   = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.can_remove   = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.can_promote  = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.can_demote   = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.can_motd     = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.can_warpeace = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_GUILD_RANKS_REPOSITORY_H
