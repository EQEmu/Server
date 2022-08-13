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
		int         id;
		int         doorid;
		std::string zone;
		int         version;
		std::string name;
		float       pos_y;
		float       pos_x;
		float       pos_z;
		float       heading;
		int         opentype;
		int         guild;
		int         lockpick;
		int         keyitem;
		int         nokeyring;
		int         triggerdoor;
		int         triggertype;
		int         disable_timer;
		int         doorisopen;
		int         door_param;
		std::string dest_zone;
		int         dest_instance;
		float       dest_x;
		float       dest_y;
		float       dest_z;
		float       dest_heading;
		int         invert_state;
		int         incline;
		int         size;
		float       buffer;
		int         client_version_mask;
		int         is_ldon_door;
		int         dz_switch_id;
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

	static Doors GetDoorse(
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

			e.id                     = atoi(row[0]);
			e.doorid                 = atoi(row[1]);
			e.zone                   = row[2] ? row[2] : "";
			e.version                = atoi(row[3]);
			e.name                   = row[4] ? row[4] : "";
			e.pos_y                  = static_cast<float>(atof(row[5]));
			e.pos_x                  = static_cast<float>(atof(row[6]));
			e.pos_z                  = static_cast<float>(atof(row[7]));
			e.heading                = static_cast<float>(atof(row[8]));
			e.opentype               = atoi(row[9]);
			e.guild                  = atoi(row[10]);
			e.lockpick               = atoi(row[11]);
			e.keyitem                = atoi(row[12]);
			e.nokeyring              = atoi(row[13]);
			e.triggerdoor            = atoi(row[14]);
			e.triggertype            = atoi(row[15]);
			e.disable_timer          = atoi(row[16]);
			e.doorisopen             = atoi(row[17]);
			e.door_param             = atoi(row[18]);
			e.dest_zone              = row[19] ? row[19] : "";
			e.dest_instance          = atoi(row[20]);
			e.dest_x                 = static_cast<float>(atof(row[21]));
			e.dest_y                 = static_cast<float>(atof(row[22]));
			e.dest_z                 = static_cast<float>(atof(row[23]));
			e.dest_heading           = static_cast<float>(atof(row[24]));
			e.invert_state           = atoi(row[25]);
			e.incline                = atoi(row[26]);
			e.size                   = atoi(row[27]);
			e.buffer                 = static_cast<float>(atof(row[28]));
			e.client_version_mask    = atoi(row[29]);
			e.is_ldon_door           = atoi(row[30]);
			e.dz_switch_id           = atoi(row[31]);
			e.min_expansion          = atoi(row[32]);
			e.max_expansion          = atoi(row[33]);
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
		Doors doors_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(doors_e.doorid));
		update_values.push_back(columns[2] + " = '" + Strings::Escape(doors_e.zone) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(doors_e.version));
		update_values.push_back(columns[4] + " = '" + Strings::Escape(doors_e.name) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(doors_e.pos_y));
		update_values.push_back(columns[6] + " = " + std::to_string(doors_e.pos_x));
		update_values.push_back(columns[7] + " = " + std::to_string(doors_e.pos_z));
		update_values.push_back(columns[8] + " = " + std::to_string(doors_e.heading));
		update_values.push_back(columns[9] + " = " + std::to_string(doors_e.opentype));
		update_values.push_back(columns[10] + " = " + std::to_string(doors_e.guild));
		update_values.push_back(columns[11] + " = " + std::to_string(doors_e.lockpick));
		update_values.push_back(columns[12] + " = " + std::to_string(doors_e.keyitem));
		update_values.push_back(columns[13] + " = " + std::to_string(doors_e.nokeyring));
		update_values.push_back(columns[14] + " = " + std::to_string(doors_e.triggerdoor));
		update_values.push_back(columns[15] + " = " + std::to_string(doors_e.triggertype));
		update_values.push_back(columns[16] + " = " + std::to_string(doors_e.disable_timer));
		update_values.push_back(columns[17] + " = " + std::to_string(doors_e.doorisopen));
		update_values.push_back(columns[18] + " = " + std::to_string(doors_e.door_param));
		update_values.push_back(columns[19] + " = '" + Strings::Escape(doors_e.dest_zone) + "'");
		update_values.push_back(columns[20] + " = " + std::to_string(doors_e.dest_instance));
		update_values.push_back(columns[21] + " = " + std::to_string(doors_e.dest_x));
		update_values.push_back(columns[22] + " = " + std::to_string(doors_e.dest_y));
		update_values.push_back(columns[23] + " = " + std::to_string(doors_e.dest_z));
		update_values.push_back(columns[24] + " = " + std::to_string(doors_e.dest_heading));
		update_values.push_back(columns[25] + " = " + std::to_string(doors_e.invert_state));
		update_values.push_back(columns[26] + " = " + std::to_string(doors_e.incline));
		update_values.push_back(columns[27] + " = " + std::to_string(doors_e.size));
		update_values.push_back(columns[28] + " = " + std::to_string(doors_e.buffer));
		update_values.push_back(columns[29] + " = " + std::to_string(doors_e.client_version_mask));
		update_values.push_back(columns[30] + " = " + std::to_string(doors_e.is_ldon_door));
		update_values.push_back(columns[31] + " = " + std::to_string(doors_e.dz_switch_id));
		update_values.push_back(columns[32] + " = " + std::to_string(doors_e.min_expansion));
		update_values.push_back(columns[33] + " = " + std::to_string(doors_e.max_expansion));
		update_values.push_back(columns[34] + " = '" + Strings::Escape(doors_e.content_flags) + "'");
		update_values.push_back(columns[35] + " = '" + Strings::Escape(doors_e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				doors_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Doors InsertOne(
		Database& db,
		Doors doors_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(doors_e.id));
		insert_values.push_back(std::to_string(doors_e.doorid));
		insert_values.push_back("'" + Strings::Escape(doors_e.zone) + "'");
		insert_values.push_back(std::to_string(doors_e.version));
		insert_values.push_back("'" + Strings::Escape(doors_e.name) + "'");
		insert_values.push_back(std::to_string(doors_e.pos_y));
		insert_values.push_back(std::to_string(doors_e.pos_x));
		insert_values.push_back(std::to_string(doors_e.pos_z));
		insert_values.push_back(std::to_string(doors_e.heading));
		insert_values.push_back(std::to_string(doors_e.opentype));
		insert_values.push_back(std::to_string(doors_e.guild));
		insert_values.push_back(std::to_string(doors_e.lockpick));
		insert_values.push_back(std::to_string(doors_e.keyitem));
		insert_values.push_back(std::to_string(doors_e.nokeyring));
		insert_values.push_back(std::to_string(doors_e.triggerdoor));
		insert_values.push_back(std::to_string(doors_e.triggertype));
		insert_values.push_back(std::to_string(doors_e.disable_timer));
		insert_values.push_back(std::to_string(doors_e.doorisopen));
		insert_values.push_back(std::to_string(doors_e.door_param));
		insert_values.push_back("'" + Strings::Escape(doors_e.dest_zone) + "'");
		insert_values.push_back(std::to_string(doors_e.dest_instance));
		insert_values.push_back(std::to_string(doors_e.dest_x));
		insert_values.push_back(std::to_string(doors_e.dest_y));
		insert_values.push_back(std::to_string(doors_e.dest_z));
		insert_values.push_back(std::to_string(doors_e.dest_heading));
		insert_values.push_back(std::to_string(doors_e.invert_state));
		insert_values.push_back(std::to_string(doors_e.incline));
		insert_values.push_back(std::to_string(doors_e.size));
		insert_values.push_back(std::to_string(doors_e.buffer));
		insert_values.push_back(std::to_string(doors_e.client_version_mask));
		insert_values.push_back(std::to_string(doors_e.is_ldon_door));
		insert_values.push_back(std::to_string(doors_e.dz_switch_id));
		insert_values.push_back(std::to_string(doors_e.min_expansion));
		insert_values.push_back(std::to_string(doors_e.max_expansion));
		insert_values.push_back("'" + Strings::Escape(doors_e.content_flags) + "'");
		insert_values.push_back("'" + Strings::Escape(doors_e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			doors_e.id = results.LastInsertedID();
			return doors_e;
		}

		doors_e = NewEntity();

		return doors_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Doors> doors_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &doors_e: doors_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(doors_e.id));
			insert_values.push_back(std::to_string(doors_e.doorid));
			insert_values.push_back("'" + Strings::Escape(doors_e.zone) + "'");
			insert_values.push_back(std::to_string(doors_e.version));
			insert_values.push_back("'" + Strings::Escape(doors_e.name) + "'");
			insert_values.push_back(std::to_string(doors_e.pos_y));
			insert_values.push_back(std::to_string(doors_e.pos_x));
			insert_values.push_back(std::to_string(doors_e.pos_z));
			insert_values.push_back(std::to_string(doors_e.heading));
			insert_values.push_back(std::to_string(doors_e.opentype));
			insert_values.push_back(std::to_string(doors_e.guild));
			insert_values.push_back(std::to_string(doors_e.lockpick));
			insert_values.push_back(std::to_string(doors_e.keyitem));
			insert_values.push_back(std::to_string(doors_e.nokeyring));
			insert_values.push_back(std::to_string(doors_e.triggerdoor));
			insert_values.push_back(std::to_string(doors_e.triggertype));
			insert_values.push_back(std::to_string(doors_e.disable_timer));
			insert_values.push_back(std::to_string(doors_e.doorisopen));
			insert_values.push_back(std::to_string(doors_e.door_param));
			insert_values.push_back("'" + Strings::Escape(doors_e.dest_zone) + "'");
			insert_values.push_back(std::to_string(doors_e.dest_instance));
			insert_values.push_back(std::to_string(doors_e.dest_x));
			insert_values.push_back(std::to_string(doors_e.dest_y));
			insert_values.push_back(std::to_string(doors_e.dest_z));
			insert_values.push_back(std::to_string(doors_e.dest_heading));
			insert_values.push_back(std::to_string(doors_e.invert_state));
			insert_values.push_back(std::to_string(doors_e.incline));
			insert_values.push_back(std::to_string(doors_e.size));
			insert_values.push_back(std::to_string(doors_e.buffer));
			insert_values.push_back(std::to_string(doors_e.client_version_mask));
			insert_values.push_back(std::to_string(doors_e.is_ldon_door));
			insert_values.push_back(std::to_string(doors_e.dz_switch_id));
			insert_values.push_back(std::to_string(doors_e.min_expansion));
			insert_values.push_back(std::to_string(doors_e.max_expansion));
			insert_values.push_back("'" + Strings::Escape(doors_e.content_flags) + "'");
			insert_values.push_back("'" + Strings::Escape(doors_e.content_flags_disabled) + "'");

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

			e.id                     = atoi(row[0]);
			e.doorid                 = atoi(row[1]);
			e.zone                   = row[2] ? row[2] : "";
			e.version                = atoi(row[3]);
			e.name                   = row[4] ? row[4] : "";
			e.pos_y                  = static_cast<float>(atof(row[5]));
			e.pos_x                  = static_cast<float>(atof(row[6]));
			e.pos_z                  = static_cast<float>(atof(row[7]));
			e.heading                = static_cast<float>(atof(row[8]));
			e.opentype               = atoi(row[9]);
			e.guild                  = atoi(row[10]);
			e.lockpick               = atoi(row[11]);
			e.keyitem                = atoi(row[12]);
			e.nokeyring              = atoi(row[13]);
			e.triggerdoor            = atoi(row[14]);
			e.triggertype            = atoi(row[15]);
			e.disable_timer          = atoi(row[16]);
			e.doorisopen             = atoi(row[17]);
			e.door_param             = atoi(row[18]);
			e.dest_zone              = row[19] ? row[19] : "";
			e.dest_instance          = atoi(row[20]);
			e.dest_x                 = static_cast<float>(atof(row[21]));
			e.dest_y                 = static_cast<float>(atof(row[22]));
			e.dest_z                 = static_cast<float>(atof(row[23]));
			e.dest_heading           = static_cast<float>(atof(row[24]));
			e.invert_state           = atoi(row[25]);
			e.incline                = atoi(row[26]);
			e.size                   = atoi(row[27]);
			e.buffer                 = static_cast<float>(atof(row[28]));
			e.client_version_mask    = atoi(row[29]);
			e.is_ldon_door           = atoi(row[30]);
			e.dz_switch_id           = atoi(row[31]);
			e.min_expansion          = atoi(row[32]);
			e.max_expansion          = atoi(row[33]);
			e.content_flags          = row[34] ? row[34] : "";
			e.content_flags_disabled = row[35] ? row[35] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Doors> GetWhere(Database& db, std::string where_filter)
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

			e.id                     = atoi(row[0]);
			e.doorid                 = atoi(row[1]);
			e.zone                   = row[2] ? row[2] : "";
			e.version                = atoi(row[3]);
			e.name                   = row[4] ? row[4] : "";
			e.pos_y                  = static_cast<float>(atof(row[5]));
			e.pos_x                  = static_cast<float>(atof(row[6]));
			e.pos_z                  = static_cast<float>(atof(row[7]));
			e.heading                = static_cast<float>(atof(row[8]));
			e.opentype               = atoi(row[9]);
			e.guild                  = atoi(row[10]);
			e.lockpick               = atoi(row[11]);
			e.keyitem                = atoi(row[12]);
			e.nokeyring              = atoi(row[13]);
			e.triggerdoor            = atoi(row[14]);
			e.triggertype            = atoi(row[15]);
			e.disable_timer          = atoi(row[16]);
			e.doorisopen             = atoi(row[17]);
			e.door_param             = atoi(row[18]);
			e.dest_zone              = row[19] ? row[19] : "";
			e.dest_instance          = atoi(row[20]);
			e.dest_x                 = static_cast<float>(atof(row[21]));
			e.dest_y                 = static_cast<float>(atof(row[22]));
			e.dest_z                 = static_cast<float>(atof(row[23]));
			e.dest_heading           = static_cast<float>(atof(row[24]));
			e.invert_state           = atoi(row[25]);
			e.incline                = atoi(row[26]);
			e.size                   = atoi(row[27]);
			e.buffer                 = static_cast<float>(atof(row[28]));
			e.client_version_mask    = atoi(row[29]);
			e.is_ldon_door           = atoi(row[30]);
			e.dz_switch_id           = atoi(row[31]);
			e.min_expansion          = atoi(row[32]);
			e.max_expansion          = atoi(row[33]);
			e.content_flags          = row[34] ? row[34] : "";
			e.content_flags_disabled = row[35] ? row[35] : "";

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

#endif //EQEMU_BASE_DOORS_REPOSITORY_H
