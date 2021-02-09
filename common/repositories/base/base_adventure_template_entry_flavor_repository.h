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

#ifndef EQEMU_BASE_ADVENTURE_TEMPLATE_ENTRY_FLAVOR_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_TEMPLATE_ENTRY_FLAVOR_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAdventureTemplateEntryFlavorRepository {
public:
	struct AdventureTemplateEntryFlavor {
		int         id;
		std::string text;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"text",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("adventure_template_entry_flavor");
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

	static AdventureTemplateEntryFlavor NewEntity()
	{
		AdventureTemplateEntryFlavor entry{};

		entry.id   = 0;
		entry.text = "";

		return entry;
	}

	static AdventureTemplateEntryFlavor GetAdventureTemplateEntryFlavorEntry(
		const std::vector<AdventureTemplateEntryFlavor> &adventure_template_entry_flavors,
		int adventure_template_entry_flavor_id
	)
	{
		for (auto &adventure_template_entry_flavor : adventure_template_entry_flavors) {
			if (adventure_template_entry_flavor.id == adventure_template_entry_flavor_id) {
				return adventure_template_entry_flavor;
			}
		}

		return NewEntity();
	}

	static AdventureTemplateEntryFlavor FindOne(
		Database& db,
		int adventure_template_entry_flavor_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_template_entry_flavor_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureTemplateEntryFlavor entry{};

			entry.id   = atoi(row[0]);
			entry.text = row[1] ? row[1] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_template_entry_flavor_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_template_entry_flavor_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AdventureTemplateEntryFlavor adventure_template_entry_flavor_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(adventure_template_entry_flavor_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(adventure_template_entry_flavor_entry.text) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				adventure_template_entry_flavor_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureTemplateEntryFlavor InsertOne(
		Database& db,
		AdventureTemplateEntryFlavor adventure_template_entry_flavor_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(adventure_template_entry_flavor_entry.id));
		insert_values.push_back("'" + EscapeString(adventure_template_entry_flavor_entry.text) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_template_entry_flavor_entry.id = results.LastInsertedID();
			return adventure_template_entry_flavor_entry;
		}

		adventure_template_entry_flavor_entry = NewEntity();

		return adventure_template_entry_flavor_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AdventureTemplateEntryFlavor> adventure_template_entry_flavor_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_template_entry_flavor_entry: adventure_template_entry_flavor_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(adventure_template_entry_flavor_entry.id));
			insert_values.push_back("'" + EscapeString(adventure_template_entry_flavor_entry.text) + "'");

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

	static std::vector<AdventureTemplateEntryFlavor> All(Database& db)
	{
		std::vector<AdventureTemplateEntryFlavor> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureTemplateEntryFlavor entry{};

			entry.id   = atoi(row[0]);
			entry.text = row[1] ? row[1] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AdventureTemplateEntryFlavor> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AdventureTemplateEntryFlavor> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureTemplateEntryFlavor entry{};

			entry.id   = atoi(row[0]);
			entry.text = row[1] ? row[1] : "";

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

#endif //EQEMU_BASE_ADVENTURE_TEMPLATE_ENTRY_FLAVOR_REPOSITORY_H
