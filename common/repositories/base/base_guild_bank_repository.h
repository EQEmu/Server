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

#ifndef EQEMU_BASE_GUILD_BANK_REPOSITORY_H
#define EQEMU_BASE_GUILD_BANK_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseGuildBankRepository {
public:
	struct GuildBank {
		uint32_t    id;
		uint32_t    guildid;
		uint8_t     area;
		uint32_t    slot;
		uint32_t    itemid;
		uint32_t    qty;
		std::string donator;
		uint8_t     permissions;
		std::string whofor;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"guildid",
			"area",
			"slot",
			"itemid",
			"qty",
			"donator",
			"permissions",
			"whofor",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"guildid",
			"area",
			"slot",
			"itemid",
			"qty",
			"donator",
			"permissions",
			"whofor",
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
		return std::string("guild_bank");
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

	static GuildBank NewEntity()
	{
		GuildBank e{};

		e.id          = 0;
		e.guildid     = 0;
		e.area        = 0;
		e.slot        = 0;
		e.itemid      = 0;
		e.qty         = 0;
		e.donator     = "";
		e.permissions = 0;
		e.whofor      = "";

		return e;
	}

	static GuildBank GetGuildBank(
		const std::vector<GuildBank> &guild_banks,
		int guild_bank_id
	)
	{
		for (auto &guild_bank : guild_banks) {
			if (guild_bank.id == guild_bank_id) {
				return guild_bank;
			}
		}

		return NewEntity();
	}

	static GuildBank FindOne(
		Database& db,
		int guild_bank_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				guild_bank_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildBank e{};

			e.id          = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.guildid     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.area        = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.slot        = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.itemid      = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.qty         = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.donator     = row[6] ? row[6] : "";
			e.permissions = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.whofor      = row[8] ? row[8] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guild_bank_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_bank_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GuildBank &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.guildid));
		v.push_back(columns[2] + " = " + std::to_string(e.area));
		v.push_back(columns[3] + " = " + std::to_string(e.slot));
		v.push_back(columns[4] + " = " + std::to_string(e.itemid));
		v.push_back(columns[5] + " = " + std::to_string(e.qty));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.donator) + "'");
		v.push_back(columns[7] + " = " + std::to_string(e.permissions));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.whofor) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildBank InsertOne(
		Database& db,
		GuildBank e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.guildid));
		v.push_back(std::to_string(e.area));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.qty));
		v.push_back("'" + Strings::Escape(e.donator) + "'");
		v.push_back(std::to_string(e.permissions));
		v.push_back("'" + Strings::Escape(e.whofor) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GuildBank> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.guildid));
			v.push_back(std::to_string(e.area));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.qty));
			v.push_back("'" + Strings::Escape(e.donator) + "'");
			v.push_back(std::to_string(e.permissions));
			v.push_back("'" + Strings::Escape(e.whofor) + "'");

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

	static std::vector<GuildBank> All(Database& db)
	{
		std::vector<GuildBank> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildBank e{};

			e.id          = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.guildid     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.area        = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.slot        = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.itemid      = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.qty         = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.donator     = row[6] ? row[6] : "";
			e.permissions = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.whofor      = row[8] ? row[8] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GuildBank> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GuildBank> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildBank e{};

			e.id          = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.guildid     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.area        = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.slot        = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.itemid      = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.qty         = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.donator     = row[6] ? row[6] : "";
			e.permissions = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.whofor      = row[8] ? row[8] : "";

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

#endif //EQEMU_BASE_GUILD_BANK_REPOSITORY_H
