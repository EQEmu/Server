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

#ifndef EQEMU_BASE_DISCORD_WEBHOOKS_REPOSITORY_H
#define EQEMU_BASE_DISCORD_WEBHOOKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseDiscordWebhooksRepository {
public:
	struct DiscordWebhooks {
		int         id;
		std::string webhook_name;
		std::string webhook_url;
		time_t      created_at;
		time_t      deleted_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"webhook_name",
			"webhook_url",
			"created_at",
			"deleted_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"webhook_name",
			"webhook_url",
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(deleted_at)",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("discord_webhooks");
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

	static DiscordWebhooks NewEntity()
	{
		DiscordWebhooks entry{};

		entry.id           = 0;
		entry.webhook_name = "";
		entry.webhook_url  = "";
		entry.created_at   = 0;
		entry.deleted_at   = 0;

		return entry;
	}

	static DiscordWebhooks GetDiscordWebhooksEntry(
		const std::vector<DiscordWebhooks> &discord_webhookss,
		int discord_webhooks_id
	)
	{
		for (auto &discord_webhooks : discord_webhookss) {
			if (discord_webhooks.id == discord_webhooks_id) {
				return discord_webhooks;
			}
		}

		return NewEntity();
	}

	static DiscordWebhooks FindOne(
		Database& db,
		int discord_webhooks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				discord_webhooks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DiscordWebhooks entry{};

			entry.id           = atoi(row[0]);
			entry.webhook_name = row[1] ? row[1] : "";
			entry.webhook_url  = row[2] ? row[2] : "";
			entry.created_at   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.deleted_at   = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int discord_webhooks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				discord_webhooks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		DiscordWebhooks discord_webhooks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(discord_webhooks_entry.webhook_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(discord_webhooks_entry.webhook_url) + "'");
		update_values.push_back(columns[3] + " = FROM_UNIXTIME(" + (discord_webhooks_entry.created_at > 0 ? std::to_string(discord_webhooks_entry.created_at) : "null") + ")");
		update_values.push_back(columns[4] + " = FROM_UNIXTIME(" + (discord_webhooks_entry.deleted_at > 0 ? std::to_string(discord_webhooks_entry.deleted_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				discord_webhooks_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DiscordWebhooks InsertOne(
		Database& db,
		DiscordWebhooks discord_webhooks_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(discord_webhooks_entry.id));
		insert_values.push_back("'" + EscapeString(discord_webhooks_entry.webhook_name) + "'");
		insert_values.push_back("'" + EscapeString(discord_webhooks_entry.webhook_url) + "'");
		insert_values.push_back("FROM_UNIXTIME(" + (discord_webhooks_entry.created_at > 0 ? std::to_string(discord_webhooks_entry.created_at) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (discord_webhooks_entry.deleted_at > 0 ? std::to_string(discord_webhooks_entry.deleted_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			discord_webhooks_entry.id = results.LastInsertedID();
			return discord_webhooks_entry;
		}

		discord_webhooks_entry = NewEntity();

		return discord_webhooks_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<DiscordWebhooks> discord_webhooks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &discord_webhooks_entry: discord_webhooks_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(discord_webhooks_entry.id));
			insert_values.push_back("'" + EscapeString(discord_webhooks_entry.webhook_name) + "'");
			insert_values.push_back("'" + EscapeString(discord_webhooks_entry.webhook_url) + "'");
			insert_values.push_back("FROM_UNIXTIME(" + (discord_webhooks_entry.created_at > 0 ? std::to_string(discord_webhooks_entry.created_at) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (discord_webhooks_entry.deleted_at > 0 ? std::to_string(discord_webhooks_entry.deleted_at) : "null") + ")");

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

	static std::vector<DiscordWebhooks> All(Database& db)
	{
		std::vector<DiscordWebhooks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscordWebhooks entry{};

			entry.id           = atoi(row[0]);
			entry.webhook_name = row[1] ? row[1] : "";
			entry.webhook_url  = row[2] ? row[2] : "";
			entry.created_at   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.deleted_at   = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<DiscordWebhooks> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<DiscordWebhooks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscordWebhooks entry{};

			entry.id           = atoi(row[0]);
			entry.webhook_name = row[1] ? row[1] : "";
			entry.webhook_url  = row[2] ? row[2] : "";
			entry.created_at   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.deleted_at   = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

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

#endif //EQEMU_BASE_DISCORD_WEBHOOKS_REPOSITORY_H
