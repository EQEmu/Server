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

#ifndef EQEMU_BASE_OBJECT_REPOSITORY_H
#define EQEMU_BASE_OBJECT_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseObjectRepository {
public:
	struct Object {
		int         id;
		int         zoneid;
		int         version;
		float       xpos;
		float       ypos;
		float       zpos;
		float       heading;
		int         itemid;
		int         charges;
		std::string objectname;
		int         type;
		int         icon;
		int         unknown08;
		int         unknown10;
		int         unknown20;
		int         unknown24;
		int         unknown60;
		int         unknown64;
		int         unknown68;
		int         unknown72;
		int         unknown76;
		int         unknown84;
		float       size;
		float       tilt_x;
		float       tilt_y;
		std::string display_name;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zoneid",
			"version",
			"xpos",
			"ypos",
			"zpos",
			"heading",
			"itemid",
			"charges",
			"objectname",
			"type",
			"icon",
			"unknown08",
			"unknown10",
			"unknown20",
			"unknown24",
			"unknown60",
			"unknown64",
			"unknown68",
			"unknown72",
			"unknown76",
			"unknown84",
			"size",
			"tilt_x",
			"tilt_y",
			"display_name",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("object");
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

	static Object NewEntity()
	{
		Object entry{};

		entry.id                     = 0;
		entry.zoneid                 = 0;
		entry.version                = 0;
		entry.xpos                   = 0;
		entry.ypos                   = 0;
		entry.zpos                   = 0;
		entry.heading                = 0;
		entry.itemid                 = 0;
		entry.charges                = 0;
		entry.objectname             = "";
		entry.type                   = 0;
		entry.icon                   = 0;
		entry.unknown08              = 0;
		entry.unknown10              = 0;
		entry.unknown20              = 0;
		entry.unknown24              = 0;
		entry.unknown60              = 0;
		entry.unknown64              = 0;
		entry.unknown68              = 0;
		entry.unknown72              = 0;
		entry.unknown76              = 0;
		entry.unknown84              = 0;
		entry.size                   = 100;
		entry.tilt_x                 = 0;
		entry.tilt_y                 = 0;
		entry.display_name           = "";
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Object GetObjectEntry(
		const std::vector<Object> &objects,
		int object_id
	)
	{
		for (auto &object : objects) {
			if (object.id == object_id) {
				return object;
			}
		}

		return NewEntity();
	}

	static Object FindOne(
		Database& db,
		int object_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				object_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Object entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.version                = atoi(row[2]);
			entry.xpos                   = static_cast<float>(atof(row[3]));
			entry.ypos                   = static_cast<float>(atof(row[4]));
			entry.zpos                   = static_cast<float>(atof(row[5]));
			entry.heading                = static_cast<float>(atof(row[6]));
			entry.itemid                 = atoi(row[7]);
			entry.charges                = atoi(row[8]);
			entry.objectname             = row[9] ? row[9] : "";
			entry.type                   = atoi(row[10]);
			entry.icon                   = atoi(row[11]);
			entry.unknown08              = atoi(row[12]);
			entry.unknown10              = atoi(row[13]);
			entry.unknown20              = atoi(row[14]);
			entry.unknown24              = atoi(row[15]);
			entry.unknown60              = atoi(row[16]);
			entry.unknown64              = atoi(row[17]);
			entry.unknown68              = atoi(row[18]);
			entry.unknown72              = atoi(row[19]);
			entry.unknown76              = atoi(row[20]);
			entry.unknown84              = atoi(row[21]);
			entry.size                   = static_cast<float>(atof(row[22]));
			entry.tilt_x                 = static_cast<float>(atof(row[23]));
			entry.tilt_y                 = static_cast<float>(atof(row[24]));
			entry.display_name           = row[25] ? row[25] : "";
			entry.min_expansion          = atoi(row[26]);
			entry.max_expansion          = atoi(row[27]);
			entry.content_flags          = row[28] ? row[28] : "";
			entry.content_flags_disabled = row[29] ? row[29] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int object_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				object_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Object object_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(object_entry.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(object_entry.version));
		update_values.push_back(columns[3] + " = " + std::to_string(object_entry.xpos));
		update_values.push_back(columns[4] + " = " + std::to_string(object_entry.ypos));
		update_values.push_back(columns[5] + " = " + std::to_string(object_entry.zpos));
		update_values.push_back(columns[6] + " = " + std::to_string(object_entry.heading));
		update_values.push_back(columns[7] + " = " + std::to_string(object_entry.itemid));
		update_values.push_back(columns[8] + " = " + std::to_string(object_entry.charges));
		update_values.push_back(columns[9] + " = '" + EscapeString(object_entry.objectname) + "'");
		update_values.push_back(columns[10] + " = " + std::to_string(object_entry.type));
		update_values.push_back(columns[11] + " = " + std::to_string(object_entry.icon));
		update_values.push_back(columns[12] + " = " + std::to_string(object_entry.unknown08));
		update_values.push_back(columns[13] + " = " + std::to_string(object_entry.unknown10));
		update_values.push_back(columns[14] + " = " + std::to_string(object_entry.unknown20));
		update_values.push_back(columns[15] + " = " + std::to_string(object_entry.unknown24));
		update_values.push_back(columns[16] + " = " + std::to_string(object_entry.unknown60));
		update_values.push_back(columns[17] + " = " + std::to_string(object_entry.unknown64));
		update_values.push_back(columns[18] + " = " + std::to_string(object_entry.unknown68));
		update_values.push_back(columns[19] + " = " + std::to_string(object_entry.unknown72));
		update_values.push_back(columns[20] + " = " + std::to_string(object_entry.unknown76));
		update_values.push_back(columns[21] + " = " + std::to_string(object_entry.unknown84));
		update_values.push_back(columns[22] + " = " + std::to_string(object_entry.size));
		update_values.push_back(columns[23] + " = " + std::to_string(object_entry.tilt_x));
		update_values.push_back(columns[24] + " = " + std::to_string(object_entry.tilt_y));
		update_values.push_back(columns[25] + " = '" + EscapeString(object_entry.display_name) + "'");
		update_values.push_back(columns[26] + " = " + std::to_string(object_entry.min_expansion));
		update_values.push_back(columns[27] + " = " + std::to_string(object_entry.max_expansion));
		update_values.push_back(columns[28] + " = '" + EscapeString(object_entry.content_flags) + "'");
		update_values.push_back(columns[29] + " = '" + EscapeString(object_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				object_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Object InsertOne(
		Database& db,
		Object object_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(object_entry.id));
		insert_values.push_back(std::to_string(object_entry.zoneid));
		insert_values.push_back(std::to_string(object_entry.version));
		insert_values.push_back(std::to_string(object_entry.xpos));
		insert_values.push_back(std::to_string(object_entry.ypos));
		insert_values.push_back(std::to_string(object_entry.zpos));
		insert_values.push_back(std::to_string(object_entry.heading));
		insert_values.push_back(std::to_string(object_entry.itemid));
		insert_values.push_back(std::to_string(object_entry.charges));
		insert_values.push_back("'" + EscapeString(object_entry.objectname) + "'");
		insert_values.push_back(std::to_string(object_entry.type));
		insert_values.push_back(std::to_string(object_entry.icon));
		insert_values.push_back(std::to_string(object_entry.unknown08));
		insert_values.push_back(std::to_string(object_entry.unknown10));
		insert_values.push_back(std::to_string(object_entry.unknown20));
		insert_values.push_back(std::to_string(object_entry.unknown24));
		insert_values.push_back(std::to_string(object_entry.unknown60));
		insert_values.push_back(std::to_string(object_entry.unknown64));
		insert_values.push_back(std::to_string(object_entry.unknown68));
		insert_values.push_back(std::to_string(object_entry.unknown72));
		insert_values.push_back(std::to_string(object_entry.unknown76));
		insert_values.push_back(std::to_string(object_entry.unknown84));
		insert_values.push_back(std::to_string(object_entry.size));
		insert_values.push_back(std::to_string(object_entry.tilt_x));
		insert_values.push_back(std::to_string(object_entry.tilt_y));
		insert_values.push_back("'" + EscapeString(object_entry.display_name) + "'");
		insert_values.push_back(std::to_string(object_entry.min_expansion));
		insert_values.push_back(std::to_string(object_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(object_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(object_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			object_entry.id = results.LastInsertedID();
			return object_entry;
		}

		object_entry = NewEntity();

		return object_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Object> object_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &object_entry: object_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(object_entry.id));
			insert_values.push_back(std::to_string(object_entry.zoneid));
			insert_values.push_back(std::to_string(object_entry.version));
			insert_values.push_back(std::to_string(object_entry.xpos));
			insert_values.push_back(std::to_string(object_entry.ypos));
			insert_values.push_back(std::to_string(object_entry.zpos));
			insert_values.push_back(std::to_string(object_entry.heading));
			insert_values.push_back(std::to_string(object_entry.itemid));
			insert_values.push_back(std::to_string(object_entry.charges));
			insert_values.push_back("'" + EscapeString(object_entry.objectname) + "'");
			insert_values.push_back(std::to_string(object_entry.type));
			insert_values.push_back(std::to_string(object_entry.icon));
			insert_values.push_back(std::to_string(object_entry.unknown08));
			insert_values.push_back(std::to_string(object_entry.unknown10));
			insert_values.push_back(std::to_string(object_entry.unknown20));
			insert_values.push_back(std::to_string(object_entry.unknown24));
			insert_values.push_back(std::to_string(object_entry.unknown60));
			insert_values.push_back(std::to_string(object_entry.unknown64));
			insert_values.push_back(std::to_string(object_entry.unknown68));
			insert_values.push_back(std::to_string(object_entry.unknown72));
			insert_values.push_back(std::to_string(object_entry.unknown76));
			insert_values.push_back(std::to_string(object_entry.unknown84));
			insert_values.push_back(std::to_string(object_entry.size));
			insert_values.push_back(std::to_string(object_entry.tilt_x));
			insert_values.push_back(std::to_string(object_entry.tilt_y));
			insert_values.push_back("'" + EscapeString(object_entry.display_name) + "'");
			insert_values.push_back(std::to_string(object_entry.min_expansion));
			insert_values.push_back(std::to_string(object_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(object_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(object_entry.content_flags_disabled) + "'");

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

	static std::vector<Object> All(Database& db)
	{
		std::vector<Object> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Object entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.version                = atoi(row[2]);
			entry.xpos                   = static_cast<float>(atof(row[3]));
			entry.ypos                   = static_cast<float>(atof(row[4]));
			entry.zpos                   = static_cast<float>(atof(row[5]));
			entry.heading                = static_cast<float>(atof(row[6]));
			entry.itemid                 = atoi(row[7]);
			entry.charges                = atoi(row[8]);
			entry.objectname             = row[9] ? row[9] : "";
			entry.type                   = atoi(row[10]);
			entry.icon                   = atoi(row[11]);
			entry.unknown08              = atoi(row[12]);
			entry.unknown10              = atoi(row[13]);
			entry.unknown20              = atoi(row[14]);
			entry.unknown24              = atoi(row[15]);
			entry.unknown60              = atoi(row[16]);
			entry.unknown64              = atoi(row[17]);
			entry.unknown68              = atoi(row[18]);
			entry.unknown72              = atoi(row[19]);
			entry.unknown76              = atoi(row[20]);
			entry.unknown84              = atoi(row[21]);
			entry.size                   = static_cast<float>(atof(row[22]));
			entry.tilt_x                 = static_cast<float>(atof(row[23]));
			entry.tilt_y                 = static_cast<float>(atof(row[24]));
			entry.display_name           = row[25] ? row[25] : "";
			entry.min_expansion          = atoi(row[26]);
			entry.max_expansion          = atoi(row[27]);
			entry.content_flags          = row[28] ? row[28] : "";
			entry.content_flags_disabled = row[29] ? row[29] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Object> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Object> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Object entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.version                = atoi(row[2]);
			entry.xpos                   = static_cast<float>(atof(row[3]));
			entry.ypos                   = static_cast<float>(atof(row[4]));
			entry.zpos                   = static_cast<float>(atof(row[5]));
			entry.heading                = static_cast<float>(atof(row[6]));
			entry.itemid                 = atoi(row[7]);
			entry.charges                = atoi(row[8]);
			entry.objectname             = row[9] ? row[9] : "";
			entry.type                   = atoi(row[10]);
			entry.icon                   = atoi(row[11]);
			entry.unknown08              = atoi(row[12]);
			entry.unknown10              = atoi(row[13]);
			entry.unknown20              = atoi(row[14]);
			entry.unknown24              = atoi(row[15]);
			entry.unknown60              = atoi(row[16]);
			entry.unknown64              = atoi(row[17]);
			entry.unknown68              = atoi(row[18]);
			entry.unknown72              = atoi(row[19]);
			entry.unknown76              = atoi(row[20]);
			entry.unknown84              = atoi(row[21]);
			entry.size                   = static_cast<float>(atof(row[22]));
			entry.tilt_x                 = static_cast<float>(atof(row[23]));
			entry.tilt_y                 = static_cast<float>(atof(row[24]));
			entry.display_name           = row[25] ? row[25] : "";
			entry.min_expansion          = atoi(row[26]);
			entry.max_expansion          = atoi(row[27]);
			entry.content_flags          = row[28] ? row[28] : "";
			entry.content_flags_disabled = row[29] ? row[29] : "";

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

#endif //EQEMU_BASE_OBJECT_REPOSITORY_H
