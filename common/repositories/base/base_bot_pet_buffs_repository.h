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

#ifndef EQEMU_BASE_BOT_PET_BUFFS_REPOSITORY_H
#define EQEMU_BASE_BOT_PET_BUFFS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseBotPetBuffsRepository {
public:
	struct BotPetBuffs {
		uint32_t pet_buffs_index;
		uint32_t pets_index;
		uint32_t spell_id;
		uint32_t caster_level;
		uint32_t duration;
	};

	static std::string PrimaryKey()
	{
		return std::string("pet_buffs_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"pet_buffs_index",
			"pets_index",
			"spell_id",
			"caster_level",
			"duration",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"pet_buffs_index",
			"pets_index",
			"spell_id",
			"caster_level",
			"duration",
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
		return std::string("bot_pet_buffs");
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

	static BotPetBuffs NewEntity()
	{
		BotPetBuffs e{};

		e.pet_buffs_index = 0;
		e.pets_index      = 0;
		e.spell_id        = 0;
		e.caster_level    = 0;
		e.duration        = 0;

		return e;
	}

	static BotPetBuffs GetBotPetBuffs(
		const std::vector<BotPetBuffs> &bot_pet_buffss,
		int bot_pet_buffs_id
	)
	{
		for (auto &bot_pet_buffs : bot_pet_buffss) {
			if (bot_pet_buffs.pet_buffs_index == bot_pet_buffs_id) {
				return bot_pet_buffs;
			}
		}

		return NewEntity();
	}

	static BotPetBuffs FindOne(
		Database& db,
		int bot_pet_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				bot_pet_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotPetBuffs e{};

			e.pet_buffs_index = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.pets_index      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spell_id        = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.caster_level    = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.duration        = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_pet_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_pet_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotPetBuffs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.pets_index));
		v.push_back(columns[2] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[3] + " = " + std::to_string(e.caster_level));
		v.push_back(columns[4] + " = " + std::to_string(e.duration));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.pet_buffs_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotPetBuffs InsertOne(
		Database& db,
		BotPetBuffs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.pet_buffs_index));
		v.push_back(std::to_string(e.pets_index));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.caster_level));
		v.push_back(std::to_string(e.duration));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.pet_buffs_index = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotPetBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.pet_buffs_index));
			v.push_back(std::to_string(e.pets_index));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.caster_level));
			v.push_back(std::to_string(e.duration));

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

	static std::vector<BotPetBuffs> All(Database& db)
	{
		std::vector<BotPetBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotPetBuffs e{};

			e.pet_buffs_index = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.pets_index      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spell_id        = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.caster_level    = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.duration        = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotPetBuffs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotPetBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotPetBuffs e{};

			e.pet_buffs_index = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.pets_index      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spell_id        = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.caster_level    = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.duration        = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

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

#endif //EQEMU_BASE_BOT_PET_BUFFS_REPOSITORY_H
