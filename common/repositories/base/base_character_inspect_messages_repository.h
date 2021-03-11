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

#ifndef EQEMU_BASE_CHARACTER_INSPECT_MESSAGES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_INSPECT_MESSAGES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterInspectMessagesRepository {
public:
	struct CharacterInspectMessages {
		int         id;
		std::string inspect_message;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"inspect_message",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_inspect_messages");
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

	static CharacterInspectMessages NewEntity()
	{
		CharacterInspectMessages entry{};

		entry.id              = 0;
		entry.inspect_message = "";

		return entry;
	}

	static CharacterInspectMessages GetCharacterInspectMessagesEntry(
		const std::vector<CharacterInspectMessages> &character_inspect_messagess,
		int character_inspect_messages_id
	)
	{
		for (auto &character_inspect_messages : character_inspect_messagess) {
			if (character_inspect_messages.id == character_inspect_messages_id) {
				return character_inspect_messages;
			}
		}

		return NewEntity();
	}

	static CharacterInspectMessages FindOne(
		Database& db,
		int character_inspect_messages_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_inspect_messages_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterInspectMessages entry{};

			entry.id              = atoi(row[0]);
			entry.inspect_message = row[1] ? row[1] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_inspect_messages_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_inspect_messages_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterInspectMessages character_inspect_messages_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_inspect_messages_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(character_inspect_messages_entry.inspect_message) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_inspect_messages_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterInspectMessages InsertOne(
		Database& db,
		CharacterInspectMessages character_inspect_messages_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_inspect_messages_entry.id));
		insert_values.push_back("'" + EscapeString(character_inspect_messages_entry.inspect_message) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_inspect_messages_entry.id = results.LastInsertedID();
			return character_inspect_messages_entry;
		}

		character_inspect_messages_entry = NewEntity();

		return character_inspect_messages_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterInspectMessages> character_inspect_messages_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_inspect_messages_entry: character_inspect_messages_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_inspect_messages_entry.id));
			insert_values.push_back("'" + EscapeString(character_inspect_messages_entry.inspect_message) + "'");

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

	static std::vector<CharacterInspectMessages> All(Database& db)
	{
		std::vector<CharacterInspectMessages> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterInspectMessages entry{};

			entry.id              = atoi(row[0]);
			entry.inspect_message = row[1] ? row[1] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterInspectMessages> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterInspectMessages> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterInspectMessages entry{};

			entry.id              = atoi(row[0]);
			entry.inspect_message = row[1] ? row[1] : "";

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

#endif //EQEMU_BASE_CHARACTER_INSPECT_MESSAGES_REPOSITORY_H
