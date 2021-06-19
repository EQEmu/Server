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
#include "../../string_util.h"

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
		return std::string("doors");
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

	static Doors NewEntity()
	{
		Doors entry{};

		entry.id                     = 0;
		entry.doorid                 = 0;
		entry.zone                   = "";
		entry.version                = 0;
		entry.name                   = "";
		entry.pos_y                  = 0;
		entry.pos_x                  = 0;
		entry.pos_z                  = 0;
		entry.heading                = 0;
		entry.opentype               = 0;
		entry.guild                  = 0;
		entry.lockpick               = 0;
		entry.keyitem                = 0;
		entry.nokeyring              = 0;
		entry.triggerdoor            = 0;
		entry.triggertype            = 0;
		entry.disable_timer          = 0;
		entry.doorisopen             = 0;
		entry.door_param             = 0;
		entry.dest_zone              = "NONE";
		entry.dest_instance          = 0;
		entry.dest_x                 = 0;
		entry.dest_y                 = 0;
		entry.dest_z                 = 0;
		entry.dest_heading           = 0;
		entry.invert_state           = 0;
		entry.incline                = 0;
		entry.size                   = 100;
		entry.buffer                 = 0;
		entry.client_version_mask    = 4294967295;
		entry.is_ldon_door           = 0;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Doors GetDoorsEntry(
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
			Doors entry{};

			entry.id                     = atoi(row[0]);
			entry.doorid                 = atoi(row[1]);
			entry.zone                   = row[2] ? row[2] : "";
			entry.version                = atoi(row[3]);
			entry.name                   = row[4] ? row[4] : "";
			entry.pos_y                  = static_cast<float>(atof(row[5]));
			entry.pos_x                  = static_cast<float>(atof(row[6]));
			entry.pos_z                  = static_cast<float>(atof(row[7]));
			entry.heading                = static_cast<float>(atof(row[8]));
			entry.opentype               = atoi(row[9]);
			entry.guild                  = atoi(row[10]);
			entry.lockpick               = atoi(row[11]);
			entry.keyitem                = atoi(row[12]);
			entry.nokeyring              = atoi(row[13]);
			entry.triggerdoor            = atoi(row[14]);
			entry.triggertype            = atoi(row[15]);
			entry.disable_timer          = atoi(row[16]);
			entry.doorisopen             = atoi(row[17]);
			entry.door_param             = atoi(row[18]);
			entry.dest_zone              = row[19] ? row[19] : "";
			entry.dest_instance          = atoi(row[20]);
			entry.dest_x                 = static_cast<float>(atof(row[21]));
			entry.dest_y                 = static_cast<float>(atof(row[22]));
			entry.dest_z                 = static_cast<float>(atof(row[23]));
			entry.dest_heading           = static_cast<float>(atof(row[24]));
			entry.invert_state           = atoi(row[25]);
			entry.incline                = atoi(row[26]);
			entry.size                   = atoi(row[27]);
			entry.buffer                 = static_cast<float>(atof(row[28]));
			entry.client_version_mask    = atoi(row[29]);
			entry.is_ldon_door           = atoi(row[30]);
			entry.min_expansion          = atoi(row[31]);
			entry.max_expansion          = atoi(row[32]);
			entry.content_flags          = row[33] ? row[33] : "";
			entry.content_flags_disabled = row[34] ? row[34] : "";

			return entry;
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
		Doors doors_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(doors_entry.doorid));
		update_values.push_back(columns[2] + " = '" + EscapeString(doors_entry.zone) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(doors_entry.version));
		update_values.push_back(columns[4] + " = '" + EscapeString(doors_entry.name) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(doors_entry.pos_y));
		update_values.push_back(columns[6] + " = " + std::to_string(doors_entry.pos_x));
		update_values.push_back(columns[7] + " = " + std::to_string(doors_entry.pos_z));
		update_values.push_back(columns[8] + " = " + std::to_string(doors_entry.heading));
		update_values.push_back(columns[9] + " = " + std::to_string(doors_entry.opentype));
		update_values.push_back(columns[10] + " = " + std::to_string(doors_entry.guild));
		update_values.push_back(columns[11] + " = " + std::to_string(doors_entry.lockpick));
		update_values.push_back(columns[12] + " = " + std::to_string(doors_entry.keyitem));
		update_values.push_back(columns[13] + " = " + std::to_string(doors_entry.nokeyring));
		update_values.push_back(columns[14] + " = " + std::to_string(doors_entry.triggerdoor));
		update_values.push_back(columns[15] + " = " + std::to_string(doors_entry.triggertype));
		update_values.push_back(columns[16] + " = " + std::to_string(doors_entry.disable_timer));
		update_values.push_back(columns[17] + " = " + std::to_string(doors_entry.doorisopen));
		update_values.push_back(columns[18] + " = " + std::to_string(doors_entry.door_param));
		update_values.push_back(columns[19] + " = '" + EscapeString(doors_entry.dest_zone) + "'");
		update_values.push_back(columns[20] + " = " + std::to_string(doors_entry.dest_instance));
		update_values.push_back(columns[21] + " = " + std::to_string(doors_entry.dest_x));
		update_values.push_back(columns[22] + " = " + std::to_string(doors_entry.dest_y));
		update_values.push_back(columns[23] + " = " + std::to_string(doors_entry.dest_z));
		update_values.push_back(columns[24] + " = " + std::to_string(doors_entry.dest_heading));
		update_values.push_back(columns[25] + " = " + std::to_string(doors_entry.invert_state));
		update_values.push_back(columns[26] + " = " + std::to_string(doors_entry.incline));
		update_values.push_back(columns[27] + " = " + std::to_string(doors_entry.size));
		update_values.push_back(columns[28] + " = " + std::to_string(doors_entry.buffer));
		update_values.push_back(columns[29] + " = " + std::to_string(doors_entry.client_version_mask));
		update_values.push_back(columns[30] + " = " + std::to_string(doors_entry.is_ldon_door));
		update_values.push_back(columns[31] + " = " + std::to_string(doors_entry.min_expansion));
		update_values.push_back(columns[32] + " = " + std::to_string(doors_entry.max_expansion));
		update_values.push_back(columns[33] + " = '" + EscapeString(doors_entry.content_flags) + "'");
		update_values.push_back(columns[34] + " = '" + EscapeString(doors_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				doors_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Doors InsertOne(
		Database& db,
		Doors doors_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(doors_entry.id));
		insert_values.push_back(std::to_string(doors_entry.doorid));
		insert_values.push_back("'" + EscapeString(doors_entry.zone) + "'");
		insert_values.push_back(std::to_string(doors_entry.version));
		insert_values.push_back("'" + EscapeString(doors_entry.name) + "'");
		insert_values.push_back(std::to_string(doors_entry.pos_y));
		insert_values.push_back(std::to_string(doors_entry.pos_x));
		insert_values.push_back(std::to_string(doors_entry.pos_z));
		insert_values.push_back(std::to_string(doors_entry.heading));
		insert_values.push_back(std::to_string(doors_entry.opentype));
		insert_values.push_back(std::to_string(doors_entry.guild));
		insert_values.push_back(std::to_string(doors_entry.lockpick));
		insert_values.push_back(std::to_string(doors_entry.keyitem));
		insert_values.push_back(std::to_string(doors_entry.nokeyring));
		insert_values.push_back(std::to_string(doors_entry.triggerdoor));
		insert_values.push_back(std::to_string(doors_entry.triggertype));
		insert_values.push_back(std::to_string(doors_entry.disable_timer));
		insert_values.push_back(std::to_string(doors_entry.doorisopen));
		insert_values.push_back(std::to_string(doors_entry.door_param));
		insert_values.push_back("'" + EscapeString(doors_entry.dest_zone) + "'");
		insert_values.push_back(std::to_string(doors_entry.dest_instance));
		insert_values.push_back(std::to_string(doors_entry.dest_x));
		insert_values.push_back(std::to_string(doors_entry.dest_y));
		insert_values.push_back(std::to_string(doors_entry.dest_z));
		insert_values.push_back(std::to_string(doors_entry.dest_heading));
		insert_values.push_back(std::to_string(doors_entry.invert_state));
		insert_values.push_back(std::to_string(doors_entry.incline));
		insert_values.push_back(std::to_string(doors_entry.size));
		insert_values.push_back(std::to_string(doors_entry.buffer));
		insert_values.push_back(std::to_string(doors_entry.client_version_mask));
		insert_values.push_back(std::to_string(doors_entry.is_ldon_door));
		insert_values.push_back(std::to_string(doors_entry.min_expansion));
		insert_values.push_back(std::to_string(doors_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(doors_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(doors_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			doors_entry.id = results.LastInsertedID();
			return doors_entry;
		}

		doors_entry = NewEntity();

		return doors_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Doors> doors_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &doors_entry: doors_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(doors_entry.id));
			insert_values.push_back(std::to_string(doors_entry.doorid));
			insert_values.push_back("'" + EscapeString(doors_entry.zone) + "'");
			insert_values.push_back(std::to_string(doors_entry.version));
			insert_values.push_back("'" + EscapeString(doors_entry.name) + "'");
			insert_values.push_back(std::to_string(doors_entry.pos_y));
			insert_values.push_back(std::to_string(doors_entry.pos_x));
			insert_values.push_back(std::to_string(doors_entry.pos_z));
			insert_values.push_back(std::to_string(doors_entry.heading));
			insert_values.push_back(std::to_string(doors_entry.opentype));
			insert_values.push_back(std::to_string(doors_entry.guild));
			insert_values.push_back(std::to_string(doors_entry.lockpick));
			insert_values.push_back(std::to_string(doors_entry.keyitem));
			insert_values.push_back(std::to_string(doors_entry.nokeyring));
			insert_values.push_back(std::to_string(doors_entry.triggerdoor));
			insert_values.push_back(std::to_string(doors_entry.triggertype));
			insert_values.push_back(std::to_string(doors_entry.disable_timer));
			insert_values.push_back(std::to_string(doors_entry.doorisopen));
			insert_values.push_back(std::to_string(doors_entry.door_param));
			insert_values.push_back("'" + EscapeString(doors_entry.dest_zone) + "'");
			insert_values.push_back(std::to_string(doors_entry.dest_instance));
			insert_values.push_back(std::to_string(doors_entry.dest_x));
			insert_values.push_back(std::to_string(doors_entry.dest_y));
			insert_values.push_back(std::to_string(doors_entry.dest_z));
			insert_values.push_back(std::to_string(doors_entry.dest_heading));
			insert_values.push_back(std::to_string(doors_entry.invert_state));
			insert_values.push_back(std::to_string(doors_entry.incline));
			insert_values.push_back(std::to_string(doors_entry.size));
			insert_values.push_back(std::to_string(doors_entry.buffer));
			insert_values.push_back(std::to_string(doors_entry.client_version_mask));
			insert_values.push_back(std::to_string(doors_entry.is_ldon_door));
			insert_values.push_back(std::to_string(doors_entry.min_expansion));
			insert_values.push_back(std::to_string(doors_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(doors_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(doors_entry.content_flags_disabled) + "'");

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
			Doors entry{};

			entry.id                     = atoi(row[0]);
			entry.doorid                 = atoi(row[1]);
			entry.zone                   = row[2] ? row[2] : "";
			entry.version                = atoi(row[3]);
			entry.name                   = row[4] ? row[4] : "";
			entry.pos_y                  = static_cast<float>(atof(row[5]));
			entry.pos_x                  = static_cast<float>(atof(row[6]));
			entry.pos_z                  = static_cast<float>(atof(row[7]));
			entry.heading                = static_cast<float>(atof(row[8]));
			entry.opentype               = atoi(row[9]);
			entry.guild                  = atoi(row[10]);
			entry.lockpick               = atoi(row[11]);
			entry.keyitem                = atoi(row[12]);
			entry.nokeyring              = atoi(row[13]);
			entry.triggerdoor            = atoi(row[14]);
			entry.triggertype            = atoi(row[15]);
			entry.disable_timer          = atoi(row[16]);
			entry.doorisopen             = atoi(row[17]);
			entry.door_param             = atoi(row[18]);
			entry.dest_zone              = row[19] ? row[19] : "";
			entry.dest_instance          = atoi(row[20]);
			entry.dest_x                 = static_cast<float>(atof(row[21]));
			entry.dest_y                 = static_cast<float>(atof(row[22]));
			entry.dest_z                 = static_cast<float>(atof(row[23]));
			entry.dest_heading           = static_cast<float>(atof(row[24]));
			entry.invert_state           = atoi(row[25]);
			entry.incline                = atoi(row[26]);
			entry.size                   = atoi(row[27]);
			entry.buffer                 = static_cast<float>(atof(row[28]));
			entry.client_version_mask    = atoi(row[29]);
			entry.is_ldon_door           = atoi(row[30]);
			entry.min_expansion          = atoi(row[31]);
			entry.max_expansion          = atoi(row[32]);
			entry.content_flags          = row[33] ? row[33] : "";
			entry.content_flags_disabled = row[34] ? row[34] : "";

			all_entries.push_back(entry);
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
			Doors entry{};

			entry.id                     = atoi(row[0]);
			entry.doorid                 = atoi(row[1]);
			entry.zone                   = row[2] ? row[2] : "";
			entry.version                = atoi(row[3]);
			entry.name                   = row[4] ? row[4] : "";
			entry.pos_y                  = static_cast<float>(atof(row[5]));
			entry.pos_x                  = static_cast<float>(atof(row[6]));
			entry.pos_z                  = static_cast<float>(atof(row[7]));
			entry.heading                = static_cast<float>(atof(row[8]));
			entry.opentype               = atoi(row[9]);
			entry.guild                  = atoi(row[10]);
			entry.lockpick               = atoi(row[11]);
			entry.keyitem                = atoi(row[12]);
			entry.nokeyring              = atoi(row[13]);
			entry.triggerdoor            = atoi(row[14]);
			entry.triggertype            = atoi(row[15]);
			entry.disable_timer          = atoi(row[16]);
			entry.doorisopen             = atoi(row[17]);
			entry.door_param             = atoi(row[18]);
			entry.dest_zone              = row[19] ? row[19] : "";
			entry.dest_instance          = atoi(row[20]);
			entry.dest_x                 = static_cast<float>(atof(row[21]));
			entry.dest_y                 = static_cast<float>(atof(row[22]));
			entry.dest_z                 = static_cast<float>(atof(row[23]));
			entry.dest_heading           = static_cast<float>(atof(row[24]));
			entry.invert_state           = atoi(row[25]);
			entry.incline                = atoi(row[26]);
			entry.size                   = atoi(row[27]);
			entry.buffer                 = static_cast<float>(atof(row[28]));
			entry.client_version_mask    = atoi(row[29]);
			entry.is_ldon_door           = atoi(row[30]);
			entry.min_expansion          = atoi(row[31]);
			entry.max_expansion          = atoi(row[32]);
			entry.content_flags          = row[33] ? row[33] : "";
			entry.content_flags_disabled = row[34] ? row[34] : "";

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

#endif //EQEMU_BASE_DOORS_REPOSITORY_H
