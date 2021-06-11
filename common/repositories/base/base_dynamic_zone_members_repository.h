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

#ifndef EQEMU_BASE_DYNAMIC_ZONE_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_DYNAMIC_ZONE_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseDynamicZoneMembersRepository {
public:
	struct DynamicZoneMembers {
		int id;
		int dynamic_zone_id;
		int character_id;
		int is_current_member;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"dynamic_zone_id",
			"character_id",
			"is_current_member",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("dynamic_zone_members");
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

	static DynamicZoneMembers NewEntity()
	{
		DynamicZoneMembers entry{};

		entry.id                = 0;
		entry.dynamic_zone_id   = 0;
		entry.character_id      = 0;
		entry.is_current_member = 1;

		return entry;
	}

	static DynamicZoneMembers GetDynamicZoneMembersEntry(
		const std::vector<DynamicZoneMembers> &dynamic_zone_memberss,
		int dynamic_zone_members_id
	)
	{
		for (auto &dynamic_zone_members : dynamic_zone_memberss) {
			if (dynamic_zone_members.id == dynamic_zone_members_id) {
				return dynamic_zone_members;
			}
		}

		return NewEntity();
	}

	static DynamicZoneMembers FindOne(
		Database& db,
		int dynamic_zone_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				dynamic_zone_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DynamicZoneMembers entry{};

			entry.id                = atoi(row[0]);
			entry.dynamic_zone_id   = atoi(row[1]);
			entry.character_id      = atoi(row[2]);
			entry.is_current_member = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int dynamic_zone_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				dynamic_zone_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		DynamicZoneMembers dynamic_zone_members_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(dynamic_zone_members_entry.dynamic_zone_id));
		update_values.push_back(columns[2] + " = " + std::to_string(dynamic_zone_members_entry.character_id));
		update_values.push_back(columns[3] + " = " + std::to_string(dynamic_zone_members_entry.is_current_member));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				dynamic_zone_members_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DynamicZoneMembers InsertOne(
		Database& db,
		DynamicZoneMembers dynamic_zone_members_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(dynamic_zone_members_entry.id));
		insert_values.push_back(std::to_string(dynamic_zone_members_entry.dynamic_zone_id));
		insert_values.push_back(std::to_string(dynamic_zone_members_entry.character_id));
		insert_values.push_back(std::to_string(dynamic_zone_members_entry.is_current_member));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			dynamic_zone_members_entry.id = results.LastInsertedID();
			return dynamic_zone_members_entry;
		}

		dynamic_zone_members_entry = NewEntity();

		return dynamic_zone_members_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<DynamicZoneMembers> dynamic_zone_members_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &dynamic_zone_members_entry: dynamic_zone_members_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(dynamic_zone_members_entry.id));
			insert_values.push_back(std::to_string(dynamic_zone_members_entry.dynamic_zone_id));
			insert_values.push_back(std::to_string(dynamic_zone_members_entry.character_id));
			insert_values.push_back(std::to_string(dynamic_zone_members_entry.is_current_member));

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

	static std::vector<DynamicZoneMembers> All(Database& db)
	{
		std::vector<DynamicZoneMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneMembers entry{};

			entry.id                = atoi(row[0]);
			entry.dynamic_zone_id   = atoi(row[1]);
			entry.character_id      = atoi(row[2]);
			entry.is_current_member = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<DynamicZoneMembers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<DynamicZoneMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneMembers entry{};

			entry.id                = atoi(row[0]);
			entry.dynamic_zone_id   = atoi(row[1]);
			entry.character_id      = atoi(row[2]);
			entry.is_current_member = atoi(row[3]);

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

#endif //EQEMU_BASE_DYNAMIC_ZONE_MEMBERS_REPOSITORY_H
