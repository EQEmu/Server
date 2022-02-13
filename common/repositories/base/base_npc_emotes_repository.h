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

#ifndef EQEMU_BASE_NPC_EMOTES_REPOSITORY_H
#define EQEMU_BASE_NPC_EMOTES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcEmotesRepository {
public:
	struct NpcEmotes {
		int         id;
		int         emoteid;
		int         event_;
		int         type;
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
			"emoteid",
			"event_",
			"type",
			"text",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("npc_emotes");
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

	static NpcEmotes NewEntity()
	{
		NpcEmotes entry{};

		entry.id      = 0;
		entry.emoteid = 0;
		entry.event_  = 0;
		entry.type    = 0;
		entry.text    = "";

		return entry;
	}

	static NpcEmotes GetNpcEmotesEntry(
		const std::vector<NpcEmotes> &npc_emotess,
		int npc_emotes_id
	)
	{
		for (auto &npc_emotes : npc_emotess) {
			if (npc_emotes.id == npc_emotes_id) {
				return npc_emotes;
			}
		}

		return NewEntity();
	}

	static NpcEmotes FindOne(
		Database& db,
		int npc_emotes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_emotes_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcEmotes entry{};

			entry.id      = atoi(row[0]);
			entry.emoteid = atoi(row[1]);
			entry.event_  = atoi(row[2]);
			entry.type    = atoi(row[3]);
			entry.text    = row[4] ? row[4] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_emotes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_emotes_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcEmotes npc_emotes_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(npc_emotes_entry.emoteid));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_emotes_entry.event_));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_emotes_entry.type));
		update_values.push_back(columns[4] + " = '" + EscapeString(npc_emotes_entry.text) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_emotes_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcEmotes InsertOne(
		Database& db,
		NpcEmotes npc_emotes_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_emotes_entry.id));
		insert_values.push_back(std::to_string(npc_emotes_entry.emoteid));
		insert_values.push_back(std::to_string(npc_emotes_entry.event_));
		insert_values.push_back(std::to_string(npc_emotes_entry.type));
		insert_values.push_back("'" + EscapeString(npc_emotes_entry.text) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_emotes_entry.id = results.LastInsertedID();
			return npc_emotes_entry;
		}

		npc_emotes_entry = NewEntity();

		return npc_emotes_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcEmotes> npc_emotes_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_emotes_entry: npc_emotes_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_emotes_entry.id));
			insert_values.push_back(std::to_string(npc_emotes_entry.emoteid));
			insert_values.push_back(std::to_string(npc_emotes_entry.event_));
			insert_values.push_back(std::to_string(npc_emotes_entry.type));
			insert_values.push_back("'" + EscapeString(npc_emotes_entry.text) + "'");

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

	static std::vector<NpcEmotes> All(Database& db)
	{
		std::vector<NpcEmotes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcEmotes entry{};

			entry.id      = atoi(row[0]);
			entry.emoteid = atoi(row[1]);
			entry.event_  = atoi(row[2]);
			entry.type    = atoi(row[3]);
			entry.text    = row[4] ? row[4] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcEmotes> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcEmotes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcEmotes entry{};

			entry.id      = atoi(row[0]);
			entry.emoteid = atoi(row[1]);
			entry.event_  = atoi(row[2]);
			entry.type    = atoi(row[3]);
			entry.text    = row[4] ? row[4] : "";

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

#endif //EQEMU_BASE_NPC_EMOTES_REPOSITORY_H
