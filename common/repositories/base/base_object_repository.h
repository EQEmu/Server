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
#include "../../strings.h"
#include <ctime>

class BaseObjectRepository {
public:
	struct Object {
		int32_t     id;
		uint32_t    zoneid;
		int16_t     version;
		float       xpos;
		float       ypos;
		float       zpos;
		float       heading;
		int32_t     itemid;
		uint16_t    charges;
		std::string objectname;
		int32_t     type;
		int32_t     icon;
		int32_t     unknown08;
		int32_t     unknown10;
		int32_t     unknown20;
		int32_t     unknown24;
		int32_t     unknown60;
		int32_t     unknown64;
		int32_t     unknown68;
		int32_t     unknown72;
		int32_t     unknown76;
		int32_t     unknown84;
		float       size;
		float       tilt_x;
		float       tilt_y;
		std::string display_name;
		int8_t      min_expansion;
		int8_t      max_expansion;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("object");
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

	static Object NewEntity()
	{
		Object e{};

		e.id                     = 0;
		e.zoneid                 = 0;
		e.version                = 0;
		e.xpos                   = 0;
		e.ypos                   = 0;
		e.zpos                   = 0;
		e.heading                = 0;
		e.itemid                 = 0;
		e.charges                = 0;
		e.objectname             = "";
		e.type                   = 0;
		e.icon                   = 0;
		e.unknown08              = 0;
		e.unknown10              = 0;
		e.unknown20              = 0;
		e.unknown24              = 0;
		e.unknown60              = 0;
		e.unknown64              = 0;
		e.unknown68              = 0;
		e.unknown72              = 0;
		e.unknown76              = 0;
		e.unknown84              = 0;
		e.size                   = 100;
		e.tilt_x                 = 0;
		e.tilt_y                 = 0;
		e.display_name           = "";
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Object GetObject(
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
			Object e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.zoneid                 = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.version                = static_cast<int16_t>(atoi(row[2]));
			e.xpos                   = strtof(row[3], nullptr);
			e.ypos                   = strtof(row[4], nullptr);
			e.zpos                   = strtof(row[5], nullptr);
			e.heading                = strtof(row[6], nullptr);
			e.itemid                 = static_cast<int32_t>(atoi(row[7]));
			e.charges                = static_cast<uint16_t>(strtoul(row[8], nullptr, 10));
			e.objectname             = row[9] ? row[9] : "";
			e.type                   = static_cast<int32_t>(atoi(row[10]));
			e.icon                   = static_cast<int32_t>(atoi(row[11]));
			e.unknown08              = static_cast<int32_t>(atoi(row[12]));
			e.unknown10              = static_cast<int32_t>(atoi(row[13]));
			e.unknown20              = static_cast<int32_t>(atoi(row[14]));
			e.unknown24              = static_cast<int32_t>(atoi(row[15]));
			e.unknown60              = static_cast<int32_t>(atoi(row[16]));
			e.unknown64              = static_cast<int32_t>(atoi(row[17]));
			e.unknown68              = static_cast<int32_t>(atoi(row[18]));
			e.unknown72              = static_cast<int32_t>(atoi(row[19]));
			e.unknown76              = static_cast<int32_t>(atoi(row[20]));
			e.unknown84              = static_cast<int32_t>(atoi(row[21]));
			e.size                   = strtof(row[22], nullptr);
			e.tilt_x                 = strtof(row[23], nullptr);
			e.tilt_y                 = strtof(row[24], nullptr);
			e.display_name           = row[25] ? row[25] : "";
			e.min_expansion          = static_cast<int8_t>(atoi(row[26]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[27]));
			e.content_flags          = row[28] ? row[28] : "";
			e.content_flags_disabled = row[29] ? row[29] : "";

			return e;
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
		const Object &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[2] + " = " + std::to_string(e.version));
		v.push_back(columns[3] + " = " + std::to_string(e.xpos));
		v.push_back(columns[4] + " = " + std::to_string(e.ypos));
		v.push_back(columns[5] + " = " + std::to_string(e.zpos));
		v.push_back(columns[6] + " = " + std::to_string(e.heading));
		v.push_back(columns[7] + " = " + std::to_string(e.itemid));
		v.push_back(columns[8] + " = " + std::to_string(e.charges));
		v.push_back(columns[9] + " = '" + Strings::Escape(e.objectname) + "'");
		v.push_back(columns[10] + " = " + std::to_string(e.type));
		v.push_back(columns[11] + " = " + std::to_string(e.icon));
		v.push_back(columns[12] + " = " + std::to_string(e.unknown08));
		v.push_back(columns[13] + " = " + std::to_string(e.unknown10));
		v.push_back(columns[14] + " = " + std::to_string(e.unknown20));
		v.push_back(columns[15] + " = " + std::to_string(e.unknown24));
		v.push_back(columns[16] + " = " + std::to_string(e.unknown60));
		v.push_back(columns[17] + " = " + std::to_string(e.unknown64));
		v.push_back(columns[18] + " = " + std::to_string(e.unknown68));
		v.push_back(columns[19] + " = " + std::to_string(e.unknown72));
		v.push_back(columns[20] + " = " + std::to_string(e.unknown76));
		v.push_back(columns[21] + " = " + std::to_string(e.unknown84));
		v.push_back(columns[22] + " = " + std::to_string(e.size));
		v.push_back(columns[23] + " = " + std::to_string(e.tilt_x));
		v.push_back(columns[24] + " = " + std::to_string(e.tilt_y));
		v.push_back(columns[25] + " = '" + Strings::Escape(e.display_name) + "'");
		v.push_back(columns[26] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[27] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[28] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[29] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Object InsertOne(
		Database& db,
		Object e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.xpos));
		v.push_back(std::to_string(e.ypos));
		v.push_back(std::to_string(e.zpos));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.charges));
		v.push_back("'" + Strings::Escape(e.objectname) + "'");
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.icon));
		v.push_back(std::to_string(e.unknown08));
		v.push_back(std::to_string(e.unknown10));
		v.push_back(std::to_string(e.unknown20));
		v.push_back(std::to_string(e.unknown24));
		v.push_back(std::to_string(e.unknown60));
		v.push_back(std::to_string(e.unknown64));
		v.push_back(std::to_string(e.unknown68));
		v.push_back(std::to_string(e.unknown72));
		v.push_back(std::to_string(e.unknown76));
		v.push_back(std::to_string(e.unknown84));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.tilt_x));
		v.push_back(std::to_string(e.tilt_y));
		v.push_back("'" + Strings::Escape(e.display_name) + "'");
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Object> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.xpos));
			v.push_back(std::to_string(e.ypos));
			v.push_back(std::to_string(e.zpos));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.charges));
			v.push_back("'" + Strings::Escape(e.objectname) + "'");
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.icon));
			v.push_back(std::to_string(e.unknown08));
			v.push_back(std::to_string(e.unknown10));
			v.push_back(std::to_string(e.unknown20));
			v.push_back(std::to_string(e.unknown24));
			v.push_back(std::to_string(e.unknown60));
			v.push_back(std::to_string(e.unknown64));
			v.push_back(std::to_string(e.unknown68));
			v.push_back(std::to_string(e.unknown72));
			v.push_back(std::to_string(e.unknown76));
			v.push_back(std::to_string(e.unknown84));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.tilt_x));
			v.push_back(std::to_string(e.tilt_y));
			v.push_back("'" + Strings::Escape(e.display_name) + "'");
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			Object e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.zoneid                 = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.version                = static_cast<int16_t>(atoi(row[2]));
			e.xpos                   = strtof(row[3], nullptr);
			e.ypos                   = strtof(row[4], nullptr);
			e.zpos                   = strtof(row[5], nullptr);
			e.heading                = strtof(row[6], nullptr);
			e.itemid                 = static_cast<int32_t>(atoi(row[7]));
			e.charges                = static_cast<uint16_t>(strtoul(row[8], nullptr, 10));
			e.objectname             = row[9] ? row[9] : "";
			e.type                   = static_cast<int32_t>(atoi(row[10]));
			e.icon                   = static_cast<int32_t>(atoi(row[11]));
			e.unknown08              = static_cast<int32_t>(atoi(row[12]));
			e.unknown10              = static_cast<int32_t>(atoi(row[13]));
			e.unknown20              = static_cast<int32_t>(atoi(row[14]));
			e.unknown24              = static_cast<int32_t>(atoi(row[15]));
			e.unknown60              = static_cast<int32_t>(atoi(row[16]));
			e.unknown64              = static_cast<int32_t>(atoi(row[17]));
			e.unknown68              = static_cast<int32_t>(atoi(row[18]));
			e.unknown72              = static_cast<int32_t>(atoi(row[19]));
			e.unknown76              = static_cast<int32_t>(atoi(row[20]));
			e.unknown84              = static_cast<int32_t>(atoi(row[21]));
			e.size                   = strtof(row[22], nullptr);
			e.tilt_x                 = strtof(row[23], nullptr);
			e.tilt_y                 = strtof(row[24], nullptr);
			e.display_name           = row[25] ? row[25] : "";
			e.min_expansion          = static_cast<int8_t>(atoi(row[26]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[27]));
			e.content_flags          = row[28] ? row[28] : "";
			e.content_flags_disabled = row[29] ? row[29] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Object> GetWhere(Database& db, const std::string &where_filter)
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
			Object e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.zoneid                 = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.version                = static_cast<int16_t>(atoi(row[2]));
			e.xpos                   = strtof(row[3], nullptr);
			e.ypos                   = strtof(row[4], nullptr);
			e.zpos                   = strtof(row[5], nullptr);
			e.heading                = strtof(row[6], nullptr);
			e.itemid                 = static_cast<int32_t>(atoi(row[7]));
			e.charges                = static_cast<uint16_t>(strtoul(row[8], nullptr, 10));
			e.objectname             = row[9] ? row[9] : "";
			e.type                   = static_cast<int32_t>(atoi(row[10]));
			e.icon                   = static_cast<int32_t>(atoi(row[11]));
			e.unknown08              = static_cast<int32_t>(atoi(row[12]));
			e.unknown10              = static_cast<int32_t>(atoi(row[13]));
			e.unknown20              = static_cast<int32_t>(atoi(row[14]));
			e.unknown24              = static_cast<int32_t>(atoi(row[15]));
			e.unknown60              = static_cast<int32_t>(atoi(row[16]));
			e.unknown64              = static_cast<int32_t>(atoi(row[17]));
			e.unknown68              = static_cast<int32_t>(atoi(row[18]));
			e.unknown72              = static_cast<int32_t>(atoi(row[19]));
			e.unknown76              = static_cast<int32_t>(atoi(row[20]));
			e.unknown84              = static_cast<int32_t>(atoi(row[21]));
			e.size                   = strtof(row[22], nullptr);
			e.tilt_x                 = strtof(row[23], nullptr);
			e.tilt_y                 = strtof(row[24], nullptr);
			e.display_name           = row[25] ? row[25] : "";
			e.min_expansion          = static_cast<int8_t>(atoi(row[26]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[27]));
			e.content_flags          = row[28] ? row[28] : "";
			e.content_flags_disabled = row[29] ? row[29] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_OBJECT_REPOSITORY_H
