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

#ifndef EQEMU_BASE_SPELL_GLOBALS_REPOSITORY_H
#define EQEMU_BASE_SPELL_GLOBALS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpellGlobalsRepository {
public:
	struct SpellGlobals {
		int         spellid;
		std::string spell_name;
		std::string qglobal;
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
			"spell_name",
			"qglobal",
			"value",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("spell_globals");
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

	static SpellGlobals NewEntity()
	{
		SpellGlobals entry{};

		entry.spellid    = 0;
		entry.spell_name = "";
		entry.qglobal    = "";
		entry.value      = "";

		return entry;
	}

	static SpellGlobals GetSpellGlobalsEntry(
		const std::vector<SpellGlobals> &spell_globalss,
		int spell_globals_id
	)
	{
		for (auto &spell_globals : spell_globalss) {
			if (spell_globals.spellid == spell_globals_id) {
				return spell_globals;
			}
		}

		return NewEntity();
	}

	static SpellGlobals FindOne(
		Database& db,
		int spell_globals_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spell_globals_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpellGlobals entry{};

			entry.spellid    = atoi(row[0]);
			entry.spell_name = row[1] ? row[1] : "";
			entry.qglobal    = row[2] ? row[2] : "";
			entry.value      = row[3] ? row[3] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spell_globals_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spell_globals_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SpellGlobals spell_globals_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(spell_globals_entry.spellid));
		update_values.push_back(columns[1] + " = '" + EscapeString(spell_globals_entry.spell_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(spell_globals_entry.qglobal) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(spell_globals_entry.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spell_globals_entry.spellid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpellGlobals InsertOne(
		Database& db,
		SpellGlobals spell_globals_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spell_globals_entry.spellid));
		insert_values.push_back("'" + EscapeString(spell_globals_entry.spell_name) + "'");
		insert_values.push_back("'" + EscapeString(spell_globals_entry.qglobal) + "'");
		insert_values.push_back("'" + EscapeString(spell_globals_entry.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spell_globals_entry.spellid = results.LastInsertedID();
			return spell_globals_entry;
		}

		spell_globals_entry = NewEntity();

		return spell_globals_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SpellGlobals> spell_globals_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spell_globals_entry: spell_globals_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spell_globals_entry.spellid));
			insert_values.push_back("'" + EscapeString(spell_globals_entry.spell_name) + "'");
			insert_values.push_back("'" + EscapeString(spell_globals_entry.qglobal) + "'");
			insert_values.push_back("'" + EscapeString(spell_globals_entry.value) + "'");

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

	static std::vector<SpellGlobals> All(Database& db)
	{
		std::vector<SpellGlobals> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellGlobals entry{};

			entry.spellid    = atoi(row[0]);
			entry.spell_name = row[1] ? row[1] : "";
			entry.qglobal    = row[2] ? row[2] : "";
			entry.value      = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SpellGlobals> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SpellGlobals> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellGlobals entry{};

			entry.spellid    = atoi(row[0]);
			entry.spell_name = row[1] ? row[1] : "";
			entry.qglobal    = row[2] ? row[2] : "";
			entry.value      = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_SPELL_GLOBALS_REPOSITORY_H
