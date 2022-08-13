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

#ifndef EQEMU_BASE_SPELL_BUCKETS_REPOSITORY_H
#define EQEMU_BASE_SPELL_BUCKETS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseSpellBucketsRepository {
public:
	struct SpellBuckets {
		int64       spellid;
		std::string key;
		std::string value;
	};

	static std::string PrimaryKey()
	{
		return std::string("spellid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spellid",
			"key",
			"value",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"spellid",
			"key",
			"value",
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
		return std::string("spell_buckets");
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

	static SpellBuckets NewEntity()
	{
		SpellBuckets e{};

		e.spellid = 0;
		e.key     = "";
		e.value   = "";

		return e;
	}

	static SpellBuckets GetSpellBucketse(
		const std::vector<SpellBuckets> &spell_bucketss,
		int spell_buckets_id
	)
	{
		for (auto &spell_buckets : spell_bucketss) {
			if (spell_buckets.spellid == spell_buckets_id) {
				return spell_buckets;
			}
		}

		return NewEntity();
	}

	static SpellBuckets FindOne(
		Database& db,
		int spell_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spell_buckets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpellBuckets e{};

			e.spellid = strtoll(row[0], nullptr, 10);
			e.key     = row[1] ? row[1] : "";
			e.value   = row[2] ? row[2] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spell_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spell_buckets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SpellBuckets e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(e.spellid));
		update_values.push_back(columns[1] + " = '" + Strings::Escape(e.key) + "'");
		update_values.push_back(columns[2] + " = '" + Strings::Escape(e.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				e.spellid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpellBuckets InsertOne(
		Database& db,
		SpellBuckets e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(e.spellid));
		insert_values.push_back("'" + Strings::Escape(e.key) + "'");
		insert_values.push_back("'" + Strings::Escape(e.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			e.spellid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		std::vector<SpellBuckets> entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(e.spellid));
			insert_values.push_back("'" + Strings::Escape(e.key) + "'");
			insert_values.push_back("'" + Strings::Escape(e.value) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<SpellBuckets> All(Database& db)
	{
		std::vector<SpellBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellBuckets e{};

			e.spellid = strtoll(row[0], nullptr, 10);
			e.key     = row[1] ? row[1] : "";
			e.value   = row[2] ? row[2] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SpellBuckets> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SpellBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellBuckets e{};

			e.spellid = strtoll(row[0], nullptr, 10);
			e.key     = row[1] ? row[1] : "";
			e.value   = row[2] ? row[2] : "";

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_SPELL_BUCKETS_REPOSITORY_H
