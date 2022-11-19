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

#ifndef EQEMU_BASE_DOORS_REPOSITORY_H
#define EQEMU_BASE_DOORS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseDoorsRepository {
public:
	struct Doors {
		int32_t     id;
		int16_t     doorid;
		std::string zone;
		int16_t     version;
		std::string name;
		float       pos_y;
		float       pos_x;
		float       pos_z;
		float       heading;
		int16_t     opentype;
		int16_t     guild;
		int16_t     lockpick;
		int32_t     keyitem;
		uint8_t     nokeyring;
		int16_t     triggerdoor;
		int16_t     triggertype;
		int8_t      disable_timer;
		int16_t     doorisopen;
		int32_t     door_param;
		std::string dest_zone;
		uint32_t    dest_instance;
		float       dest_x;
		float       dest_y;
		float       dest_z;
		float       dest_heading;
		int32_t     invert_state;
		int32_t     incline;
		uint16_t    size;
		float       buffer;
		uint32_t    client_version_mask;
		int16_t     is_ldon_door;
		int32_t     dz_switch_id;
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
			"doorid",
			"zone",
			"version",
			"name",
			"pos_y",
			"pos_x",
			"pos_z",
			"heading",
			"opentype",
			"guild",
			"lockpick",
			"keyitem",
			"nokeyring",
			"triggerdoor",
			"triggertype",
			"disable_timer",
			"doorisopen",
			"door_param",
			"dest_zone",
			"dest_instance",
			"dest_x",
			"dest_y",
			"dest_z",
			"dest_heading",
			"invert_state",
			"incline",
			"size",
			"buffer",
			"client_version_mask",
			"is_ldon_door",
			"dz_switch_id",
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
			"doorid",
			"zone",
			"version",
			"name",
			"pos_y",
			"pos_x",
			"pos_z",
			"heading",
			"opentype",
			"guild",
			"lockpick",
			"keyitem",
			"nokeyring",
			"triggerdoor",
			"triggertype",
			"disable_timer",
			"doorisopen",
			"door_param",
			"dest_zone",
			"dest_instance",
			"dest_x",
			"dest_y",
			"dest_z",
			"dest_heading",
			"invert_state",
			"incline",
			"size",
			"buffer",
			"client_version_mask",
			"is_ldon_door",
			"dz_switch_id",
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
		return std::string("doors");
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

	static Doors NewEntity()
	{
		Doors e{};

		e.id                     = 0;
		e.doorid                 = 0;
		e.zone                   = "";
		e.version                = 0;
		e.name                   = "";
		e.pos_y                  = 0;
		e.pos_x                  = 0;
		e.pos_z                  = 0;
		e.heading                = 0;
		e.opentype               = 0;
		e.guild                  = 0;
		e.lockpick               = 0;
		e.keyitem                = 0;
		e.nokeyring              = 0;
		e.triggerdoor            = 0;
		e.triggertype            = 0;
		e.disable_timer          = 0;
		e.doorisopen             = 0;
		e.door_param             = 0;
		e.dest_zone              = "NONE";
		e.dest_instance          = 0;
		e.dest_x                 = 0;
		e.dest_y                 = 0;
		e.dest_z                 = 0;
		e.dest_heading           = 0;
		e.invert_state           = 0;
		e.incline                = 0;
		e.size                   = 100;
		e.buffer                 = 0;
		e.client_version_mask    = 4294967295;
		e.is_ldon_door           = 0;
		e.dz_switch_id           = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Doors GetDoors(
		const std::vector<Doors> &doorss,
		int doors_id
	)
	{
		for (auto &doors : doorss) {
			if (doors.id == doors_id) {
				return doors;
			}
		}

		return NewEntity();
	}

	static Doors FindOne(
		Database& db,
		int doors_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				doors_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Doors e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.doorid                 = static_cast<int16_t>(atoi(row[1]));
			e.zone                   = row[2] ? row[2] : "";
			e.version                = static_cast<int16_t>(atoi(row[3]));
			e.name                   = row[4] ? row[4] : "";
			e.pos_y                  = strtof(row[5], nullptr);
			e.pos_x                  = strtof(row[6], nullptr);
			e.pos_z                  = strtof(row[7], nullptr);
			e.heading                = strtof(row[8], nullptr);
			e.opentype               = static_cast<int16_t>(atoi(row[9]));
			e.guild                  = static_cast<int16_t>(atoi(row[10]));
			e.lockpick               = static_cast<int16_t>(atoi(row[11]));
			e.keyitem                = static_cast<int32_t>(atoi(row[12]));
			e.nokeyring              = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.triggerdoor            = static_cast<int16_t>(atoi(row[14]));
			e.triggertype            = static_cast<int16_t>(atoi(row[15]));
			e.disable_timer          = static_cast<int8_t>(atoi(row[16]));
			e.doorisopen             = static_cast<int16_t>(atoi(row[17]));
			e.door_param             = static_cast<int32_t>(atoi(row[18]));
			e.dest_zone              = row[19] ? row[19] : "";
			e.dest_instance          = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.dest_x                 = strtof(row[21], nullptr);
			e.dest_y                 = strtof(row[22], nullptr);
			e.dest_z                 = strtof(row[23], nullptr);
			e.dest_heading           = strtof(row[24], nullptr);
			e.invert_state           = static_cast<int32_t>(atoi(row[25]));
			e.incline                = static_cast<int32_t>(atoi(row[26]));
			e.size                   = static_cast<uint16_t>(strtoul(row[27], nullptr, 10));
			e.buffer                 = strtof(row[28], nullptr);
			e.client_version_mask    = static_cast<uint32_t>(strtoul(row[29], nullptr, 10));
			e.is_ldon_door           = static_cast<int16_t>(atoi(row[30]));
			e.dz_switch_id           = static_cast<int32_t>(atoi(row[31]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[32]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[33]));
			e.content_flags          = row[34] ? row[34] : "";
			e.content_flags_disabled = row[35] ? row[35] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int doors_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				doors_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Doors &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.doorid));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.version));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.pos_y));
		v.push_back(columns[6] + " = " + std::to_string(e.pos_x));
		v.push_back(columns[7] + " = " + std::to_string(e.pos_z));
		v.push_back(columns[8] + " = " + std::to_string(e.heading));
		v.push_back(columns[9] + " = " + std::to_string(e.opentype));
		v.push_back(columns[10] + " = " + std::to_string(e.guild));
		v.push_back(columns[11] + " = " + std::to_string(e.lockpick));
		v.push_back(columns[12] + " = " + std::to_string(e.keyitem));
		v.push_back(columns[13] + " = " + std::to_string(e.nokeyring));
		v.push_back(columns[14] + " = " + std::to_string(e.triggerdoor));
		v.push_back(columns[15] + " = " + std::to_string(e.triggertype));
		v.push_back(columns[16] + " = " + std::to_string(e.disable_timer));
		v.push_back(columns[17] + " = " + std::to_string(e.doorisopen));
		v.push_back(columns[18] + " = " + std::to_string(e.door_param));
		v.push_back(columns[19] + " = '" + Strings::Escape(e.dest_zone) + "'");
		v.push_back(columns[20] + " = " + std::to_string(e.dest_instance));
		v.push_back(columns[21] + " = " + std::to_string(e.dest_x));
		v.push_back(columns[22] + " = " + std::to_string(e.dest_y));
		v.push_back(columns[23] + " = " + std::to_string(e.dest_z));
		v.push_back(columns[24] + " = " + std::to_string(e.dest_heading));
		v.push_back(columns[25] + " = " + std::to_string(e.invert_state));
		v.push_back(columns[26] + " = " + std::to_string(e.incline));
		v.push_back(columns[27] + " = " + std::to_string(e.size));
		v.push_back(columns[28] + " = " + std::to_string(e.buffer));
		v.push_back(columns[29] + " = " + std::to_string(e.client_version_mask));
		v.push_back(columns[30] + " = " + std::to_string(e.is_ldon_door));
		v.push_back(columns[31] + " = " + std::to_string(e.dz_switch_id));
		v.push_back(columns[32] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[33] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[34] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[35] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static Doors InsertOne(
		Database& db,
		Doors e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.doorid));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.pos_y));
		v.push_back(std::to_string(e.pos_x));
		v.push_back(std::to_string(e.pos_z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.opentype));
		v.push_back(std::to_string(e.guild));
		v.push_back(std::to_string(e.lockpick));
		v.push_back(std::to_string(e.keyitem));
		v.push_back(std::to_string(e.nokeyring));
		v.push_back(std::to_string(e.triggerdoor));
		v.push_back(std::to_string(e.triggertype));
		v.push_back(std::to_string(e.disable_timer));
		v.push_back(std::to_string(e.doorisopen));
		v.push_back(std::to_string(e.door_param));
		v.push_back("'" + Strings::Escape(e.dest_zone) + "'");
		v.push_back(std::to_string(e.dest_instance));
		v.push_back(std::to_string(e.dest_x));
		v.push_back(std::to_string(e.dest_y));
		v.push_back(std::to_string(e.dest_z));
		v.push_back(std::to_string(e.dest_heading));
		v.push_back(std::to_string(e.invert_state));
		v.push_back(std::to_string(e.incline));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.buffer));
		v.push_back(std::to_string(e.client_version_mask));
		v.push_back(std::to_string(e.is_ldon_door));
		v.push_back(std::to_string(e.dz_switch_id));
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
		const std::vector<Doors> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.doorid));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.pos_y));
			v.push_back(std::to_string(e.pos_x));
			v.push_back(std::to_string(e.pos_z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.opentype));
			v.push_back(std::to_string(e.guild));
			v.push_back(std::to_string(e.lockpick));
			v.push_back(std::to_string(e.keyitem));
			v.push_back(std::to_string(e.nokeyring));
			v.push_back(std::to_string(e.triggerdoor));
			v.push_back(std::to_string(e.triggertype));
			v.push_back(std::to_string(e.disable_timer));
			v.push_back(std::to_string(e.doorisopen));
			v.push_back(std::to_string(e.door_param));
			v.push_back("'" + Strings::Escape(e.dest_zone) + "'");
			v.push_back(std::to_string(e.dest_instance));
			v.push_back(std::to_string(e.dest_x));
			v.push_back(std::to_string(e.dest_y));
			v.push_back(std::to_string(e.dest_z));
			v.push_back(std::to_string(e.dest_heading));
			v.push_back(std::to_string(e.invert_state));
			v.push_back(std::to_string(e.incline));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.buffer));
			v.push_back(std::to_string(e.client_version_mask));
			v.push_back(std::to_string(e.is_ldon_door));
			v.push_back(std::to_string(e.dz_switch_id));
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

	static std::vector<Doors> All(Database& db)
	{
		std::vector<Doors> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Doors e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.doorid                 = static_cast<int16_t>(atoi(row[1]));
			e.zone                   = row[2] ? row[2] : "";
			e.version                = static_cast<int16_t>(atoi(row[3]));
			e.name                   = row[4] ? row[4] : "";
			e.pos_y                  = strtof(row[5], nullptr);
			e.pos_x                  = strtof(row[6], nullptr);
			e.pos_z                  = strtof(row[7], nullptr);
			e.heading                = strtof(row[8], nullptr);
			e.opentype               = static_cast<int16_t>(atoi(row[9]));
			e.guild                  = static_cast<int16_t>(atoi(row[10]));
			e.lockpick               = static_cast<int16_t>(atoi(row[11]));
			e.keyitem                = static_cast<int32_t>(atoi(row[12]));
			e.nokeyring              = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.triggerdoor            = static_cast<int16_t>(atoi(row[14]));
			e.triggertype            = static_cast<int16_t>(atoi(row[15]));
			e.disable_timer          = static_cast<int8_t>(atoi(row[16]));
			e.doorisopen             = static_cast<int16_t>(atoi(row[17]));
			e.door_param             = static_cast<int32_t>(atoi(row[18]));
			e.dest_zone              = row[19] ? row[19] : "";
			e.dest_instance          = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.dest_x                 = strtof(row[21], nullptr);
			e.dest_y                 = strtof(row[22], nullptr);
			e.dest_z                 = strtof(row[23], nullptr);
			e.dest_heading           = strtof(row[24], nullptr);
			e.invert_state           = static_cast<int32_t>(atoi(row[25]));
			e.incline                = static_cast<int32_t>(atoi(row[26]));
			e.size                   = static_cast<uint16_t>(strtoul(row[27], nullptr, 10));
			e.buffer                 = strtof(row[28], nullptr);
			e.client_version_mask    = static_cast<uint32_t>(strtoul(row[29], nullptr, 10));
			e.is_ldon_door           = static_cast<int16_t>(atoi(row[30]));
			e.dz_switch_id           = static_cast<int32_t>(atoi(row[31]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[32]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[33]));
			e.content_flags          = row[34] ? row[34] : "";
			e.content_flags_disabled = row[35] ? row[35] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Doors> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Doors> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Doors e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.doorid                 = static_cast<int16_t>(atoi(row[1]));
			e.zone                   = row[2] ? row[2] : "";
			e.version                = static_cast<int16_t>(atoi(row[3]));
			e.name                   = row[4] ? row[4] : "";
			e.pos_y                  = strtof(row[5], nullptr);
			e.pos_x                  = strtof(row[6], nullptr);
			e.pos_z                  = strtof(row[7], nullptr);
			e.heading                = strtof(row[8], nullptr);
			e.opentype               = static_cast<int16_t>(atoi(row[9]));
			e.guild                  = static_cast<int16_t>(atoi(row[10]));
			e.lockpick               = static_cast<int16_t>(atoi(row[11]));
			e.keyitem                = static_cast<int32_t>(atoi(row[12]));
			e.nokeyring              = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.triggerdoor            = static_cast<int16_t>(atoi(row[14]));
			e.triggertype            = static_cast<int16_t>(atoi(row[15]));
			e.disable_timer          = static_cast<int8_t>(atoi(row[16]));
			e.doorisopen             = static_cast<int16_t>(atoi(row[17]));
			e.door_param             = static_cast<int32_t>(atoi(row[18]));
			e.dest_zone              = row[19] ? row[19] : "";
			e.dest_instance          = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.dest_x                 = strtof(row[21], nullptr);
			e.dest_y                 = strtof(row[22], nullptr);
			e.dest_z                 = strtof(row[23], nullptr);
			e.dest_heading           = strtof(row[24], nullptr);
			e.invert_state           = static_cast<int32_t>(atoi(row[25]));
			e.incline                = static_cast<int32_t>(atoi(row[26]));
			e.size                   = static_cast<uint16_t>(strtoul(row[27], nullptr, 10));
			e.buffer                 = strtof(row[28], nullptr);
			e.client_version_mask    = static_cast<uint32_t>(strtoul(row[29], nullptr, 10));
			e.is_ldon_door           = static_cast<int16_t>(atoi(row[30]));
			e.dz_switch_id           = static_cast<int32_t>(atoi(row[31]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[32]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[33]));
			e.content_flags          = row[34] ? row[34] : "";
			e.content_flags_disabled = row[35] ? row[35] : "";

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

#endif //EQEMU_BASE_DOORS_REPOSITORY_H
