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

#ifndef EQEMU_BASE_CHARACTER_BUFFS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_BUFFS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterBuffsRepository {
public:
	struct CharacterBuffs {
		int         character_id;
		int         slot_id;
		int         spell_id;
		int         caster_level;
		std::string caster_name;
		int         ticsremaining;
		int         counters;
		int         numhits;
		int         melee_rune;
		int         magic_rune;
		int         persistent;
		int         dot_rune;
		int         caston_x;
		int         caston_y;
		int         caston_z;
		int         ExtraDIChance;
		int         instrument_mod;
	};

	static std::string PrimaryKey()
	{
		return std::string("character_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"slot_id",
			"spell_id",
			"caster_level",
			"caster_name",
			"ticsremaining",
			"counters",
			"numhits",
			"melee_rune",
			"magic_rune",
			"persistent",
			"dot_rune",
			"caston_x",
			"caston_y",
			"caston_z",
			"ExtraDIChance",
			"instrument_mod",
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
		return std::string("character_buffs");
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

	static CharacterBuffs NewEntity()
	{
		CharacterBuffs entry{};

		entry.character_id   = 0;
		entry.slot_id        = 0;
		entry.spell_id       = 0;
		entry.caster_level   = 0;
		entry.caster_name    = "";
		entry.ticsremaining  = 0;
		entry.counters       = 0;
		entry.numhits        = 0;
		entry.melee_rune     = 0;
		entry.magic_rune     = 0;
		entry.persistent     = 0;
		entry.dot_rune       = 0;
		entry.caston_x       = 0;
		entry.caston_y       = 0;
		entry.caston_z       = 0;
		entry.ExtraDIChance  = 0;
		entry.instrument_mod = 10;

		return entry;
	}

	static CharacterBuffs GetCharacterBuffsEntry(
		const std::vector<CharacterBuffs> &character_buffss,
		int character_buffs_id
	)
	{
		for (auto &character_buffs : character_buffss) {
			if (character_buffs.character_id == character_buffs_id) {
				return character_buffs;
			}
		}

		return NewEntity();
	}

	static CharacterBuffs FindOne(
		int character_buffs_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterBuffs entry{};

			entry.character_id   = atoi(row[0]);
			entry.slot_id        = atoi(row[1]);
			entry.spell_id       = atoi(row[2]);
			entry.caster_level   = atoi(row[3]);
			entry.caster_name    = row[4] ? row[4] : "";
			entry.ticsremaining  = atoi(row[5]);
			entry.counters       = atoi(row[6]);
			entry.numhits        = atoi(row[7]);
			entry.melee_rune     = atoi(row[8]);
			entry.magic_rune     = atoi(row[9]);
			entry.persistent     = atoi(row[10]);
			entry.dot_rune       = atoi(row[11]);
			entry.caston_x       = atoi(row[12]);
			entry.caston_y       = atoi(row[13]);
			entry.caston_z       = atoi(row[14]);
			entry.ExtraDIChance  = atoi(row[15]);
			entry.instrument_mod = atoi(row[16]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int character_buffs_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		CharacterBuffs character_buffs_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_buffs_entry.character_id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_buffs_entry.slot_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_buffs_entry.spell_id));
		update_values.push_back(columns[3] + " = " + std::to_string(character_buffs_entry.caster_level));
		update_values.push_back(columns[4] + " = '" + EscapeString(character_buffs_entry.caster_name) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(character_buffs_entry.ticsremaining));
		update_values.push_back(columns[6] + " = " + std::to_string(character_buffs_entry.counters));
		update_values.push_back(columns[7] + " = " + std::to_string(character_buffs_entry.numhits));
		update_values.push_back(columns[8] + " = " + std::to_string(character_buffs_entry.melee_rune));
		update_values.push_back(columns[9] + " = " + std::to_string(character_buffs_entry.magic_rune));
		update_values.push_back(columns[10] + " = " + std::to_string(character_buffs_entry.persistent));
		update_values.push_back(columns[11] + " = " + std::to_string(character_buffs_entry.dot_rune));
		update_values.push_back(columns[12] + " = " + std::to_string(character_buffs_entry.caston_x));
		update_values.push_back(columns[13] + " = " + std::to_string(character_buffs_entry.caston_y));
		update_values.push_back(columns[14] + " = " + std::to_string(character_buffs_entry.caston_z));
		update_values.push_back(columns[15] + " = " + std::to_string(character_buffs_entry.ExtraDIChance));
		update_values.push_back(columns[16] + " = " + std::to_string(character_buffs_entry.instrument_mod));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_buffs_entry.character_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterBuffs InsertOne(
		CharacterBuffs character_buffs_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_buffs_entry.character_id));
		insert_values.push_back(std::to_string(character_buffs_entry.slot_id));
		insert_values.push_back(std::to_string(character_buffs_entry.spell_id));
		insert_values.push_back(std::to_string(character_buffs_entry.caster_level));
		insert_values.push_back("'" + EscapeString(character_buffs_entry.caster_name) + "'");
		insert_values.push_back(std::to_string(character_buffs_entry.ticsremaining));
		insert_values.push_back(std::to_string(character_buffs_entry.counters));
		insert_values.push_back(std::to_string(character_buffs_entry.numhits));
		insert_values.push_back(std::to_string(character_buffs_entry.melee_rune));
		insert_values.push_back(std::to_string(character_buffs_entry.magic_rune));
		insert_values.push_back(std::to_string(character_buffs_entry.persistent));
		insert_values.push_back(std::to_string(character_buffs_entry.dot_rune));
		insert_values.push_back(std::to_string(character_buffs_entry.caston_x));
		insert_values.push_back(std::to_string(character_buffs_entry.caston_y));
		insert_values.push_back(std::to_string(character_buffs_entry.caston_z));
		insert_values.push_back(std::to_string(character_buffs_entry.ExtraDIChance));
		insert_values.push_back(std::to_string(character_buffs_entry.instrument_mod));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_buffs_entry.character_id = results.LastInsertedID();
			return character_buffs_entry;
		}

		character_buffs_entry = NewEntity();

		return character_buffs_entry;
	}

	static int InsertMany(
		std::vector<CharacterBuffs> character_buffs_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_buffs_entry: character_buffs_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_buffs_entry.character_id));
			insert_values.push_back(std::to_string(character_buffs_entry.slot_id));
			insert_values.push_back(std::to_string(character_buffs_entry.spell_id));
			insert_values.push_back(std::to_string(character_buffs_entry.caster_level));
			insert_values.push_back("'" + EscapeString(character_buffs_entry.caster_name) + "'");
			insert_values.push_back(std::to_string(character_buffs_entry.ticsremaining));
			insert_values.push_back(std::to_string(character_buffs_entry.counters));
			insert_values.push_back(std::to_string(character_buffs_entry.numhits));
			insert_values.push_back(std::to_string(character_buffs_entry.melee_rune));
			insert_values.push_back(std::to_string(character_buffs_entry.magic_rune));
			insert_values.push_back(std::to_string(character_buffs_entry.persistent));
			insert_values.push_back(std::to_string(character_buffs_entry.dot_rune));
			insert_values.push_back(std::to_string(character_buffs_entry.caston_x));
			insert_values.push_back(std::to_string(character_buffs_entry.caston_y));
			insert_values.push_back(std::to_string(character_buffs_entry.caston_z));
			insert_values.push_back(std::to_string(character_buffs_entry.ExtraDIChance));
			insert_values.push_back(std::to_string(character_buffs_entry.instrument_mod));

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

	static std::vector<CharacterBuffs> All()
	{
		std::vector<CharacterBuffs> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterBuffs entry{};

			entry.character_id   = atoi(row[0]);
			entry.slot_id        = atoi(row[1]);
			entry.spell_id       = atoi(row[2]);
			entry.caster_level   = atoi(row[3]);
			entry.caster_name    = row[4] ? row[4] : "";
			entry.ticsremaining  = atoi(row[5]);
			entry.counters       = atoi(row[6]);
			entry.numhits        = atoi(row[7]);
			entry.melee_rune     = atoi(row[8]);
			entry.magic_rune     = atoi(row[9]);
			entry.persistent     = atoi(row[10]);
			entry.dot_rune       = atoi(row[11]);
			entry.caston_x       = atoi(row[12]);
			entry.caston_y       = atoi(row[13]);
			entry.caston_z       = atoi(row[14]);
			entry.ExtraDIChance  = atoi(row[15]);
			entry.instrument_mod = atoi(row[16]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterBuffs> GetWhere(std::string where_filter)
	{
		std::vector<CharacterBuffs> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterBuffs entry{};

			entry.character_id   = atoi(row[0]);
			entry.slot_id        = atoi(row[1]);
			entry.spell_id       = atoi(row[2]);
			entry.caster_level   = atoi(row[3]);
			entry.caster_name    = row[4] ? row[4] : "";
			entry.ticsremaining  = atoi(row[5]);
			entry.counters       = atoi(row[6]);
			entry.numhits        = atoi(row[7]);
			entry.melee_rune     = atoi(row[8]);
			entry.magic_rune     = atoi(row[9]);
			entry.persistent     = atoi(row[10]);
			entry.dot_rune       = atoi(row[11]);
			entry.caston_x       = atoi(row[12]);
			entry.caston_y       = atoi(row[13]);
			entry.caston_z       = atoi(row[14]);
			entry.ExtraDIChance  = atoi(row[15]);
			entry.instrument_mod = atoi(row[16]);

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

#endif //EQEMU_BASE_CHARACTER_BUFFS_REPOSITORY_H
