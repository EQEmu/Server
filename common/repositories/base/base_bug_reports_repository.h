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

#ifndef EQEMU_BASE_BUG_REPORTS_REPOSITORY_H
#define EQEMU_BASE_BUG_REPORTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseBugReportsRepository {
public:
	struct BugReports {
		int         id;
		std::string zone;
		int         client_version_id;
		std::string client_version_name;
		int         account_id;
		int         character_id;
		std::string character_name;
		int         reporter_spoof;
		int         category_id;
		std::string category_name;
		std::string reporter_name;
		std::string ui_path;
		float       pos_x;
		float       pos_y;
		float       pos_z;
		int         heading;
		int         time_played;
		int         target_id;
		std::string target_name;
		int         optional_info_mask;
		int         _can_duplicate;
		int         _crash_bug;
		int         _target_info;
		int         _character_flags;
		int         _unknown_value;
		std::string bug_report;
		std::string system_info;
		std::string report_datetime;
		int         bug_status;
		std::string last_review;
		std::string last_reviewer;
		std::string reviewer_notes;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"client_version_id",
			"client_version_name",
			"account_id",
			"character_id",
			"character_name",
			"reporter_spoof",
			"category_id",
			"category_name",
			"reporter_name",
			"ui_path",
			"pos_x",
			"pos_y",
			"pos_z",
			"heading",
			"time_played",
			"target_id",
			"target_name",
			"optional_info_mask",
			"_can_duplicate",
			"_crash_bug",
			"_target_info",
			"_character_flags",
			"_unknown_value",
			"bug_report",
			"system_info",
			"report_datetime",
			"bug_status",
			"last_review",
			"last_reviewer",
			"reviewer_notes",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("bug_reports");
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

	static BugReports NewEntity()
	{
		BugReports entry{};

		entry.id                  = 0;
		entry.zone                = "Unknown";
		entry.client_version_id   = 0;
		entry.client_version_name = "Unknown";
		entry.account_id          = 0;
		entry.character_id        = 0;
		entry.character_name      = "Unknown";
		entry.reporter_spoof      = 1;
		entry.category_id         = 0;
		entry.category_name       = "Other";
		entry.reporter_name       = "Unknown";
		entry.ui_path             = "Unknown";
		entry.pos_x               = 0;
		entry.pos_y               = 0;
		entry.pos_z               = 0;
		entry.heading             = 0;
		entry.time_played         = 0;
		entry.target_id           = 0;
		entry.target_name         = "Unknown";
		entry.optional_info_mask  = 0;
		entry._can_duplicate      = 0;
		entry._crash_bug          = 0;
		entry._target_info        = 0;
		entry._character_flags    = 0;
		entry._unknown_value      = 0;
		entry.bug_report          = "";
		entry.system_info         = "";
		entry.report_datetime     = "";
		entry.bug_status          = 0;
		entry.last_review         = "";
		entry.last_reviewer       = "None";
		entry.reviewer_notes      = "";

		return entry;
	}

	static BugReports GetBugReportsEntry(
		const std::vector<BugReports> &bug_reportss,
		int bug_reports_id
	)
	{
		for (auto &bug_reports : bug_reportss) {
			if (bug_reports.id == bug_reports_id) {
				return bug_reports;
			}
		}

		return NewEntity();
	}

	static BugReports FindOne(
		Database& db,
		int bug_reports_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				bug_reports_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BugReports entry{};

			entry.id                  = atoi(row[0]);
			entry.zone                = row[1] ? row[1] : "";
			entry.client_version_id   = atoi(row[2]);
			entry.client_version_name = row[3] ? row[3] : "";
			entry.account_id          = atoi(row[4]);
			entry.character_id        = atoi(row[5]);
			entry.character_name      = row[6] ? row[6] : "";
			entry.reporter_spoof      = atoi(row[7]);
			entry.category_id         = atoi(row[8]);
			entry.category_name       = row[9] ? row[9] : "";
			entry.reporter_name       = row[10] ? row[10] : "";
			entry.ui_path             = row[11] ? row[11] : "";
			entry.pos_x               = static_cast<float>(atof(row[12]));
			entry.pos_y               = static_cast<float>(atof(row[13]));
			entry.pos_z               = static_cast<float>(atof(row[14]));
			entry.heading             = atoi(row[15]);
			entry.time_played         = atoi(row[16]);
			entry.target_id           = atoi(row[17]);
			entry.target_name         = row[18] ? row[18] : "";
			entry.optional_info_mask  = atoi(row[19]);
			entry._can_duplicate      = atoi(row[20]);
			entry._crash_bug          = atoi(row[21]);
			entry._target_info        = atoi(row[22]);
			entry._character_flags    = atoi(row[23]);
			entry._unknown_value      = atoi(row[24]);
			entry.bug_report          = row[25] ? row[25] : "";
			entry.system_info         = row[26] ? row[26] : "";
			entry.report_datetime     = row[27] ? row[27] : "";
			entry.bug_status          = atoi(row[28]);
			entry.last_review         = row[29] ? row[29] : "";
			entry.last_reviewer       = row[30] ? row[30] : "";
			entry.reviewer_notes      = row[31] ? row[31] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bug_reports_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bug_reports_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		BugReports bug_reports_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(bug_reports_entry.zone) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(bug_reports_entry.client_version_id));
		update_values.push_back(columns[3] + " = '" + EscapeString(bug_reports_entry.client_version_name) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(bug_reports_entry.account_id));
		update_values.push_back(columns[5] + " = " + std::to_string(bug_reports_entry.character_id));
		update_values.push_back(columns[6] + " = '" + EscapeString(bug_reports_entry.character_name) + "'");
		update_values.push_back(columns[7] + " = " + std::to_string(bug_reports_entry.reporter_spoof));
		update_values.push_back(columns[8] + " = " + std::to_string(bug_reports_entry.category_id));
		update_values.push_back(columns[9] + " = '" + EscapeString(bug_reports_entry.category_name) + "'");
		update_values.push_back(columns[10] + " = '" + EscapeString(bug_reports_entry.reporter_name) + "'");
		update_values.push_back(columns[11] + " = '" + EscapeString(bug_reports_entry.ui_path) + "'");
		update_values.push_back(columns[12] + " = " + std::to_string(bug_reports_entry.pos_x));
		update_values.push_back(columns[13] + " = " + std::to_string(bug_reports_entry.pos_y));
		update_values.push_back(columns[14] + " = " + std::to_string(bug_reports_entry.pos_z));
		update_values.push_back(columns[15] + " = " + std::to_string(bug_reports_entry.heading));
		update_values.push_back(columns[16] + " = " + std::to_string(bug_reports_entry.time_played));
		update_values.push_back(columns[17] + " = " + std::to_string(bug_reports_entry.target_id));
		update_values.push_back(columns[18] + " = '" + EscapeString(bug_reports_entry.target_name) + "'");
		update_values.push_back(columns[19] + " = " + std::to_string(bug_reports_entry.optional_info_mask));
		update_values.push_back(columns[20] + " = " + std::to_string(bug_reports_entry._can_duplicate));
		update_values.push_back(columns[21] + " = " + std::to_string(bug_reports_entry._crash_bug));
		update_values.push_back(columns[22] + " = " + std::to_string(bug_reports_entry._target_info));
		update_values.push_back(columns[23] + " = " + std::to_string(bug_reports_entry._character_flags));
		update_values.push_back(columns[24] + " = " + std::to_string(bug_reports_entry._unknown_value));
		update_values.push_back(columns[25] + " = '" + EscapeString(bug_reports_entry.bug_report) + "'");
		update_values.push_back(columns[26] + " = '" + EscapeString(bug_reports_entry.system_info) + "'");
		update_values.push_back(columns[27] + " = '" + EscapeString(bug_reports_entry.report_datetime) + "'");
		update_values.push_back(columns[28] + " = " + std::to_string(bug_reports_entry.bug_status));
		update_values.push_back(columns[29] + " = '" + EscapeString(bug_reports_entry.last_review) + "'");
		update_values.push_back(columns[30] + " = '" + EscapeString(bug_reports_entry.last_reviewer) + "'");
		update_values.push_back(columns[31] + " = '" + EscapeString(bug_reports_entry.reviewer_notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				bug_reports_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BugReports InsertOne(
		Database& db,
		BugReports bug_reports_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(bug_reports_entry.id));
		insert_values.push_back("'" + EscapeString(bug_reports_entry.zone) + "'");
		insert_values.push_back(std::to_string(bug_reports_entry.client_version_id));
		insert_values.push_back("'" + EscapeString(bug_reports_entry.client_version_name) + "'");
		insert_values.push_back(std::to_string(bug_reports_entry.account_id));
		insert_values.push_back(std::to_string(bug_reports_entry.character_id));
		insert_values.push_back("'" + EscapeString(bug_reports_entry.character_name) + "'");
		insert_values.push_back(std::to_string(bug_reports_entry.reporter_spoof));
		insert_values.push_back(std::to_string(bug_reports_entry.category_id));
		insert_values.push_back("'" + EscapeString(bug_reports_entry.category_name) + "'");
		insert_values.push_back("'" + EscapeString(bug_reports_entry.reporter_name) + "'");
		insert_values.push_back("'" + EscapeString(bug_reports_entry.ui_path) + "'");
		insert_values.push_back(std::to_string(bug_reports_entry.pos_x));
		insert_values.push_back(std::to_string(bug_reports_entry.pos_y));
		insert_values.push_back(std::to_string(bug_reports_entry.pos_z));
		insert_values.push_back(std::to_string(bug_reports_entry.heading));
		insert_values.push_back(std::to_string(bug_reports_entry.time_played));
		insert_values.push_back(std::to_string(bug_reports_entry.target_id));
		insert_values.push_back("'" + EscapeString(bug_reports_entry.target_name) + "'");
		insert_values.push_back(std::to_string(bug_reports_entry.optional_info_mask));
		insert_values.push_back(std::to_string(bug_reports_entry._can_duplicate));
		insert_values.push_back(std::to_string(bug_reports_entry._crash_bug));
		insert_values.push_back(std::to_string(bug_reports_entry._target_info));
		insert_values.push_back(std::to_string(bug_reports_entry._character_flags));
		insert_values.push_back(std::to_string(bug_reports_entry._unknown_value));
		insert_values.push_back("'" + EscapeString(bug_reports_entry.bug_report) + "'");
		insert_values.push_back("'" + EscapeString(bug_reports_entry.system_info) + "'");
		insert_values.push_back("'" + EscapeString(bug_reports_entry.report_datetime) + "'");
		insert_values.push_back(std::to_string(bug_reports_entry.bug_status));
		insert_values.push_back("'" + EscapeString(bug_reports_entry.last_review) + "'");
		insert_values.push_back("'" + EscapeString(bug_reports_entry.last_reviewer) + "'");
		insert_values.push_back("'" + EscapeString(bug_reports_entry.reviewer_notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			bug_reports_entry.id = results.LastInsertedID();
			return bug_reports_entry;
		}

		bug_reports_entry = NewEntity();

		return bug_reports_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<BugReports> bug_reports_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &bug_reports_entry: bug_reports_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(bug_reports_entry.id));
			insert_values.push_back("'" + EscapeString(bug_reports_entry.zone) + "'");
			insert_values.push_back(std::to_string(bug_reports_entry.client_version_id));
			insert_values.push_back("'" + EscapeString(bug_reports_entry.client_version_name) + "'");
			insert_values.push_back(std::to_string(bug_reports_entry.account_id));
			insert_values.push_back(std::to_string(bug_reports_entry.character_id));
			insert_values.push_back("'" + EscapeString(bug_reports_entry.character_name) + "'");
			insert_values.push_back(std::to_string(bug_reports_entry.reporter_spoof));
			insert_values.push_back(std::to_string(bug_reports_entry.category_id));
			insert_values.push_back("'" + EscapeString(bug_reports_entry.category_name) + "'");
			insert_values.push_back("'" + EscapeString(bug_reports_entry.reporter_name) + "'");
			insert_values.push_back("'" + EscapeString(bug_reports_entry.ui_path) + "'");
			insert_values.push_back(std::to_string(bug_reports_entry.pos_x));
			insert_values.push_back(std::to_string(bug_reports_entry.pos_y));
			insert_values.push_back(std::to_string(bug_reports_entry.pos_z));
			insert_values.push_back(std::to_string(bug_reports_entry.heading));
			insert_values.push_back(std::to_string(bug_reports_entry.time_played));
			insert_values.push_back(std::to_string(bug_reports_entry.target_id));
			insert_values.push_back("'" + EscapeString(bug_reports_entry.target_name) + "'");
			insert_values.push_back(std::to_string(bug_reports_entry.optional_info_mask));
			insert_values.push_back(std::to_string(bug_reports_entry._can_duplicate));
			insert_values.push_back(std::to_string(bug_reports_entry._crash_bug));
			insert_values.push_back(std::to_string(bug_reports_entry._target_info));
			insert_values.push_back(std::to_string(bug_reports_entry._character_flags));
			insert_values.push_back(std::to_string(bug_reports_entry._unknown_value));
			insert_values.push_back("'" + EscapeString(bug_reports_entry.bug_report) + "'");
			insert_values.push_back("'" + EscapeString(bug_reports_entry.system_info) + "'");
			insert_values.push_back("'" + EscapeString(bug_reports_entry.report_datetime) + "'");
			insert_values.push_back(std::to_string(bug_reports_entry.bug_status));
			insert_values.push_back("'" + EscapeString(bug_reports_entry.last_review) + "'");
			insert_values.push_back("'" + EscapeString(bug_reports_entry.last_reviewer) + "'");
			insert_values.push_back("'" + EscapeString(bug_reports_entry.reviewer_notes) + "'");

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

	static std::vector<BugReports> All(Database& db)
	{
		std::vector<BugReports> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BugReports entry{};

			entry.id                  = atoi(row[0]);
			entry.zone                = row[1] ? row[1] : "";
			entry.client_version_id   = atoi(row[2]);
			entry.client_version_name = row[3] ? row[3] : "";
			entry.account_id          = atoi(row[4]);
			entry.character_id        = atoi(row[5]);
			entry.character_name      = row[6] ? row[6] : "";
			entry.reporter_spoof      = atoi(row[7]);
			entry.category_id         = atoi(row[8]);
			entry.category_name       = row[9] ? row[9] : "";
			entry.reporter_name       = row[10] ? row[10] : "";
			entry.ui_path             = row[11] ? row[11] : "";
			entry.pos_x               = static_cast<float>(atof(row[12]));
			entry.pos_y               = static_cast<float>(atof(row[13]));
			entry.pos_z               = static_cast<float>(atof(row[14]));
			entry.heading             = atoi(row[15]);
			entry.time_played         = atoi(row[16]);
			entry.target_id           = atoi(row[17]);
			entry.target_name         = row[18] ? row[18] : "";
			entry.optional_info_mask  = atoi(row[19]);
			entry._can_duplicate      = atoi(row[20]);
			entry._crash_bug          = atoi(row[21]);
			entry._target_info        = atoi(row[22]);
			entry._character_flags    = atoi(row[23]);
			entry._unknown_value      = atoi(row[24]);
			entry.bug_report          = row[25] ? row[25] : "";
			entry.system_info         = row[26] ? row[26] : "";
			entry.report_datetime     = row[27] ? row[27] : "";
			entry.bug_status          = atoi(row[28]);
			entry.last_review         = row[29] ? row[29] : "";
			entry.last_reviewer       = row[30] ? row[30] : "";
			entry.reviewer_notes      = row[31] ? row[31] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<BugReports> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<BugReports> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BugReports entry{};

			entry.id                  = atoi(row[0]);
			entry.zone                = row[1] ? row[1] : "";
			entry.client_version_id   = atoi(row[2]);
			entry.client_version_name = row[3] ? row[3] : "";
			entry.account_id          = atoi(row[4]);
			entry.character_id        = atoi(row[5]);
			entry.character_name      = row[6] ? row[6] : "";
			entry.reporter_spoof      = atoi(row[7]);
			entry.category_id         = atoi(row[8]);
			entry.category_name       = row[9] ? row[9] : "";
			entry.reporter_name       = row[10] ? row[10] : "";
			entry.ui_path             = row[11] ? row[11] : "";
			entry.pos_x               = static_cast<float>(atof(row[12]));
			entry.pos_y               = static_cast<float>(atof(row[13]));
			entry.pos_z               = static_cast<float>(atof(row[14]));
			entry.heading             = atoi(row[15]);
			entry.time_played         = atoi(row[16]);
			entry.target_id           = atoi(row[17]);
			entry.target_name         = row[18] ? row[18] : "";
			entry.optional_info_mask  = atoi(row[19]);
			entry._can_duplicate      = atoi(row[20]);
			entry._crash_bug          = atoi(row[21]);
			entry._target_info        = atoi(row[22]);
			entry._character_flags    = atoi(row[23]);
			entry._unknown_value      = atoi(row[24]);
			entry.bug_report          = row[25] ? row[25] : "";
			entry.system_info         = row[26] ? row[26] : "";
			entry.report_datetime     = row[27] ? row[27] : "";
			entry.bug_status          = atoi(row[28]);
			entry.last_review         = row[29] ? row[29] : "";
			entry.last_reviewer       = row[30] ? row[30] : "";
			entry.reviewer_notes      = row[31] ? row[31] : "";

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

#endif //EQEMU_BASE_BUG_REPORTS_REPOSITORY_H
