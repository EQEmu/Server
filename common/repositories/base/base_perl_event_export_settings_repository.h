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

#ifndef EQEMU_BASE_PERL_EVENT_EXPORT_SETTINGS_REPOSITORY_H
#define EQEMU_BASE_PERL_EVENT_EXPORT_SETTINGS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BasePerlEventExportSettingsRepository {
public:
	struct PerlEventExportSettings {
		int         event_id;
		std::string event_description;
		int         export_qglobals;
		int         export_mob;
		int         export_zone;
		int         export_item;
		int         export_event;
	};

	static std::string PrimaryKey()
	{
		return std::string("event_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"event_id",
			"event_description",
			"export_qglobals",
			"export_mob",
			"export_zone",
			"export_item",
			"export_event",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"event_id",
			"event_description",
			"export_qglobals",
			"export_mob",
			"export_zone",
			"export_item",
			"export_event",
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
		return std::string("perl_event_export_settings");
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

	static PerlEventExportSettings NewEntity()
	{
		PerlEventExportSettings e{};

		e.event_id          = 0;
		e.event_description = "";
		e.export_qglobals   = 0;
		e.export_mob        = 0;
		e.export_zone       = 0;
		e.export_item       = 0;
		e.export_event      = 0;

		return e;
	}

	static PerlEventExportSettings GetPerlEventExportSettingse(
		const std::vector<PerlEventExportSettings> &perl_event_export_settingss,
		int perl_event_export_settings_id
	)
	{
		for (auto &perl_event_export_settings : perl_event_export_settingss) {
			if (perl_event_export_settings.event_id == perl_event_export_settings_id) {
				return perl_event_export_settings;
			}
		}

		return NewEntity();
	}

	static PerlEventExportSettings FindOne(
		Database& db,
		int perl_event_export_settings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				perl_event_export_settings_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PerlEventExportSettings e{};

			e.event_id          = atoi(row[0]);
			e.event_description = row[1] ? row[1] : "";
			e.export_qglobals   = atoi(row[2]);
			e.export_mob        = atoi(row[3]);
			e.export_zone       = atoi(row[4]);
			e.export_item       = atoi(row[5]);
			e.export_event      = atoi(row[6]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int perl_event_export_settings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				perl_event_export_settings_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		PerlEventExportSettings perl_event_export_settings_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(perl_event_export_settings_e.event_id));
		update_values.push_back(columns[1] + " = '" + Strings::Escape(perl_event_export_settings_e.event_description) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(perl_event_export_settings_e.export_qglobals));
		update_values.push_back(columns[3] + " = " + std::to_string(perl_event_export_settings_e.export_mob));
		update_values.push_back(columns[4] + " = " + std::to_string(perl_event_export_settings_e.export_zone));
		update_values.push_back(columns[5] + " = " + std::to_string(perl_event_export_settings_e.export_item));
		update_values.push_back(columns[6] + " = " + std::to_string(perl_event_export_settings_e.export_event));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				perl_event_export_settings_e.event_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PerlEventExportSettings InsertOne(
		Database& db,
		PerlEventExportSettings perl_event_export_settings_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(perl_event_export_settings_e.event_id));
		insert_values.push_back("'" + Strings::Escape(perl_event_export_settings_e.event_description) + "'");
		insert_values.push_back(std::to_string(perl_event_export_settings_e.export_qglobals));
		insert_values.push_back(std::to_string(perl_event_export_settings_e.export_mob));
		insert_values.push_back(std::to_string(perl_event_export_settings_e.export_zone));
		insert_values.push_back(std::to_string(perl_event_export_settings_e.export_item));
		insert_values.push_back(std::to_string(perl_event_export_settings_e.export_event));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			perl_event_export_settings_e.event_id = results.LastInsertedID();
			return perl_event_export_settings_e;
		}

		perl_event_export_settings_e = NewEntity();

		return perl_event_export_settings_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<PerlEventExportSettings> perl_event_export_settings_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &perl_event_export_settings_e: perl_event_export_settings_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(perl_event_export_settings_e.event_id));
			insert_values.push_back("'" + Strings::Escape(perl_event_export_settings_e.event_description) + "'");
			insert_values.push_back(std::to_string(perl_event_export_settings_e.export_qglobals));
			insert_values.push_back(std::to_string(perl_event_export_settings_e.export_mob));
			insert_values.push_back(std::to_string(perl_event_export_settings_e.export_zone));
			insert_values.push_back(std::to_string(perl_event_export_settings_e.export_item));
			insert_values.push_back(std::to_string(perl_event_export_settings_e.export_event));

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

	static std::vector<PerlEventExportSettings> All(Database& db)
	{
		std::vector<PerlEventExportSettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PerlEventExportSettings e{};

			e.event_id          = atoi(row[0]);
			e.event_description = row[1] ? row[1] : "";
			e.export_qglobals   = atoi(row[2]);
			e.export_mob        = atoi(row[3]);
			e.export_zone       = atoi(row[4]);
			e.export_item       = atoi(row[5]);
			e.export_event      = atoi(row[6]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PerlEventExportSettings> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<PerlEventExportSettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PerlEventExportSettings e{};

			e.event_id          = atoi(row[0]);
			e.event_description = row[1] ? row[1] : "";
			e.export_qglobals   = atoi(row[2]);
			e.export_mob        = atoi(row[3]);
			e.export_zone       = atoi(row[4]);
			e.export_item       = atoi(row[5]);
			e.export_event      = atoi(row[6]);

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

#endif //EQEMU_BASE_PERL_EVENT_EXPORT_SETTINGS_REPOSITORY_H
