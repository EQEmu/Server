/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_PERL_EVENT_EXPORT_SETTINGS_REPOSITORY_H
#define EQEMU_BASE_PERL_EVENT_EXPORT_SETTINGS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("perl_event_export_settings");
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
			InsertColumnsRaw()
		);
	}

	static PerlEventExportSettings NewEntity()
	{
		PerlEventExportSettings entry{};

		entry.event_id          = 0;
		entry.event_description = "";
		entry.export_qglobals   = 0;
		entry.export_mob        = 0;
		entry.export_zone       = 0;
		entry.export_item       = 0;
		entry.export_event      = 0;

		return entry;
	}

	static PerlEventExportSettings GetPerlEventExportSettingsEntry(
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
		int perl_event_export_settings_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				perl_event_export_settings_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PerlEventExportSettings entry{};

			entry.event_id          = atoi(row[0]);
			entry.event_description = row[1] ? row[1] : "";
			entry.export_qglobals   = atoi(row[2]);
			entry.export_mob        = atoi(row[3]);
			entry.export_zone       = atoi(row[4]);
			entry.export_item       = atoi(row[5]);
			entry.export_event      = atoi(row[6]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int perl_event_export_settings_id
	)
	{
		auto results = database.QueryDatabase(
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
		PerlEventExportSettings perl_event_export_settings_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(perl_event_export_settings_entry.event_id));
		update_values.push_back(columns[1] + " = '" + EscapeString(perl_event_export_settings_entry.event_description) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(perl_event_export_settings_entry.export_qglobals));
		update_values.push_back(columns[3] + " = " + std::to_string(perl_event_export_settings_entry.export_mob));
		update_values.push_back(columns[4] + " = " + std::to_string(perl_event_export_settings_entry.export_zone));
		update_values.push_back(columns[5] + " = " + std::to_string(perl_event_export_settings_entry.export_item));
		update_values.push_back(columns[6] + " = " + std::to_string(perl_event_export_settings_entry.export_event));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				perl_event_export_settings_entry.event_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PerlEventExportSettings InsertOne(
		PerlEventExportSettings perl_event_export_settings_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(perl_event_export_settings_entry.event_id));
		insert_values.push_back("'" + EscapeString(perl_event_export_settings_entry.event_description) + "'");
		insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_qglobals));
		insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_mob));
		insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_zone));
		insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_item));
		insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_event));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			perl_event_export_settings_entry.event_id = results.LastInsertedID();
			return perl_event_export_settings_entry;
		}

		perl_event_export_settings_entry = NewEntity();

		return perl_event_export_settings_entry;
	}

	static int InsertMany(
		std::vector<PerlEventExportSettings> perl_event_export_settings_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &perl_event_export_settings_entry: perl_event_export_settings_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(perl_event_export_settings_entry.event_id));
			insert_values.push_back("'" + EscapeString(perl_event_export_settings_entry.event_description) + "'");
			insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_qglobals));
			insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_mob));
			insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_zone));
			insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_item));
			insert_values.push_back(std::to_string(perl_event_export_settings_entry.export_event));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<PerlEventExportSettings> All()
	{
		std::vector<PerlEventExportSettings> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PerlEventExportSettings entry{};

			entry.event_id          = atoi(row[0]);
			entry.event_description = row[1] ? row[1] : "";
			entry.export_qglobals   = atoi(row[2]);
			entry.export_mob        = atoi(row[3]);
			entry.export_zone       = atoi(row[4]);
			entry.export_item       = atoi(row[5]);
			entry.export_event      = atoi(row[6]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<PerlEventExportSettings> GetWhere(std::string where_filter)
	{
		std::vector<PerlEventExportSettings> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PerlEventExportSettings entry{};

			entry.event_id          = atoi(row[0]);
			entry.event_description = row[1] ? row[1] : "";
			entry.export_qglobals   = atoi(row[2]);
			entry.export_mob        = atoi(row[3]);
			entry.export_zone       = atoi(row[4]);
			entry.export_item       = atoi(row[5]);
			entry.export_event      = atoi(row[6]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_PERL_EVENT_EXPORT_SETTINGS_REPOSITORY_H
