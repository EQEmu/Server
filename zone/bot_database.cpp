/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifdef BOTS

#include "../common/global_define.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"

#include "bot_database.h"
#include "bot.h"


BotDatabase botdb;


BotDatabase::BotDatabase()
{
	
}

BotDatabase::BotDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
{
	Connect(host, user, passwd, database, port);
}

BotDatabase::~BotDatabase()
{
	
}

bool BotDatabase::Connect(const char* host, const char* user, const char* passwd, const char* database, uint32 port) {
	uint32 errnum = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(host, user, passwd, database, port, &errnum, errbuf)) {
		Log(Logs::General, Logs::Error, "Failed to connect to bot database: Error: %s", errbuf);
		return false;
	}
	else {
		Log(Logs::General, Logs::Status, "Using bot database '%s' at %s:%d", database, host, port);
		return true;
	}
}

bool BotDatabase::LoadBotCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &bot_command_settings)
{
	bot_command_settings.clear();

	query = "SELECT `bot_command`, `access`, `aliases` FROM `bot_command_settings`";
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		bot_command_settings[row[0]].first = atoi(row[1]);
		if (row[2][0] == 0)
			continue;

		auto aliases = SplitString(row[2], '|');
		for (auto iter : aliases) {
			if (!iter.empty())
				bot_command_settings[row[0]].second.push_back(iter);
		}
	}

	return true;
}

static uint8 spell_casting_chances[MaxSpellTypes][PLAYER_CLASS_COUNT][MaxStances][cntHSND];

bool BotDatabase::LoadBotSpellCastingChances()
{
	memset(spell_casting_chances, 0, sizeof(spell_casting_chances));

	query =
		"SELECT"
		" `spell_type_index`,"
		" `class_id`,"
		" `stance_index`,"
		" `nHSND_value`,"
		" `pH_value`,"
		" `pS_value`,"
		" `pHS_value`,"
		" `pN_value`,"
		" `pHN_value`,"
		" `pSN_value`,"
		" `pHSN_value`,"
		" `pD_value`,"
		" `pHD_value`,"
		" `pSD_value`,"
		" `pHSD_value`,"
		" `pND_value`,"
		" `pHND_value`,"
		" `pSND_value`,"
		" `pHSND_value`"
		"FROM"
		" `bot_spell_casting_chances`";

	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		uint8 spell_type_index = atoi(row[0]);
		if (spell_type_index >= MaxSpellTypes)
			continue;
		uint8 class_index = atoi(row[1]);
		if (class_index < WARRIOR || class_index > BERSERKER)
			continue;
		--class_index;
		uint8 stance_index = atoi(row[2]);
		if (stance_index >= MaxStances)
			continue;

		for (uint8 conditional_index = nHSND; conditional_index < cntHSND; ++conditional_index) {
			uint8 value = atoi(row[3 + conditional_index]);
			if (!value)
				continue;
			if (value > 100)
				value = 100;

			spell_casting_chances[spell_type_index][class_index][stance_index][conditional_index] = value;
		}
	}

	return true;
}


/* Bot functions   */
bool BotDatabase::QueryNameAvailablity(const std::string& bot_name, bool& available_flag)
{
	if (bot_name.empty() || bot_name.size() > 60 || !database.CheckUsedName(bot_name.c_str()))
		return false;

	query = StringFormat("SELECT `id` FROM `vw_bot_character_mobs` WHERE `name` LIKE '%s' LIMIT 1", bot_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (results.RowCount())
		return true;

	available_flag = true;

	return true;
}

bool BotDatabase::QueryBotCount(const uint32 owner_id, uint32& bot_count)
{
	if (!owner_id)
		return false;

	query = StringFormat("SELECT COUNT(`bot_id`) FROM `bot_data` WHERE `owner_id` = '%i'", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;
	
	auto row = results.begin();
	bot_count = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadQuestableSpawnCount(const uint32 owner_id, int& spawn_count)
{
	if (!owner_id)
		return false;

	query = StringFormat("SELECT `value` FROM `quest_globals` WHERE `name` = 'bot_spawn_limit' AND `charid` = '%i' LIMIT 1", owner_id);
	auto results = database.QueryDatabase(query); // use 'database' for non-bot table calls
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	spawn_count = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadBotsList(const uint32 owner_id, std::list<BotsAvailableList>& bots_list)
{
	if (!owner_id)
		return false;

	query = StringFormat("SELECT `bot_id`, `name`, `class`, `level`, `race`, `gender` FROM `bot_data` WHERE `owner_id` = '%u'", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row = results.begin(); row != results.end(); ++row) {
		BotsAvailableList bot_entry;

		bot_entry.ID = atoi(row[0]);

		memset(&bot_entry.Name, 0, sizeof(bot_entry.Name));
		std::string bot_name = row[1];
		if (bot_name.size() > 63)
			bot_name = bot_name.substr(0, 63);
		if (!bot_name.empty())
			strcpy(bot_entry.Name, bot_name.c_str());

		bot_entry.Class = atoi(row[2]);
		bot_entry.Level = atoi(row[3]);
		bot_entry.Race = atoi(row[4]);
		bot_entry.Gender = atoi(row[5]);

		bots_list.push_back(bot_entry);
	}

	return true;
}

bool BotDatabase::LoadOwnerID(const std::string& bot_name, uint32& owner_id)
{
	if (bot_name.empty())
		return false;

	query = StringFormat("SELECT `owner_id` FROM `bot_data` WHERE `name` = '%s' LIMIT 1", bot_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	owner_id = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadOwnerID(const uint32 bot_id, uint32& owner_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `owner_id` FROM `bot_data` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	owner_id = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadBotID(const uint32 owner_id, const std::string& bot_name, uint32& bot_id)
{
	if (!owner_id || bot_name.empty())
		return false;

	query = StringFormat("SELECT `bot_id` FROM `bot_data` WHERE `name` = '%s' LIMIT 1", bot_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;
	
	auto row = results.begin();
	bot_id = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadBot(const uint32 bot_id, Bot*& loaded_bot)
{
	if (!bot_id || loaded_bot)
		return false;

	query = StringFormat(
		"SELECT"
		" `owner_id`,"
		" `spells_id`,"
		" `name`,"
		" `last_name`,"
		" `title`,"				/* planned use[4] */
		" `suffix`,"			/* planned use[5] */
		" `zone_id`,"
		" `gender`,"
		" `race`,"
		" `class`,"
		" `level`,"
		" `deity`,"				/* planned use[11] */
		" `creation_day`,"		/* not in-use[12] */
		" `last_spawn`,"		/* not in-use[13] */
		" `time_spawned`,"
		" `size`,"
		" `face`,"
		" `hair_color`,"
		" `hair_style`,"
		" `beard`,"
		" `beard_color`,"
		" `eye_color_1`,"
		" `eye_color_2`,"
		" `drakkin_heritage`,"
		" `drakkin_tattoo`,"
		" `drakkin_details`,"
		" `ac`,"				/* not in-use[26] */
		" `atk`,"				/* not in-use[27] */
		" `hp`,"
		" `mana`,"
		" `str`,"				/* not in-use[30] */
		" `sta`,"				/* not in-use[31] */
		" `cha`,"				/* not in-use[32] */
		" `dex`,"				/* not in-use[33] */
		" `int`,"				/* not in-use[34] */
		" `agi`,"				/* not in-use[35] */
		" `wis`,"				/* not in-use[36] */
		" `fire`,"				/* not in-use[37] */
		" `cold`,"				/* not in-use[38] */
		" `magic`,"				/* not in-use[39] */
		" `poison`,"			/* not in-use[40] */
		" `disease`,"			/* not in-use[41] */
		" `corruption`,"		/* not in-use[42] */
		" `show_helm`," // 43
		" `follow_distance`" // 44
		" FROM `bot_data`"
		" WHERE `bot_id` = '%u'"
		" LIMIT 1",
		bot_id
	);

	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	// TODO: Consider removing resists and basic attributes from the load query above since we're using defaultNPCType values instead
	auto row = results.begin();
	NPCType defaultNPCTypeStruct = Bot::CreateDefaultNPCTypeStructForBot(std::string(row[2]), std::string(row[3]), atoi(row[10]), atoi(row[8]), atoi(row[9]), atoi(row[7]));
	NPCType tempNPCStruct = Bot::FillNPCTypeStruct(
		atoi(row[1]),
		std::string(row[2]),
		std::string(row[3]),
		atoi(row[10]),
		atoi(row[8]),
		atoi(row[9]),
		atoi(row[7]),
		atof(row[15]),
		atoi(row[16]),
		atoi(row[18]),
		atoi(row[17]),
		atoi(row[21]),
		atoi(row[22]),
		atoi(row[20]),
		atoi(row[19]),
		atoi(row[23]),
		atoi(row[24]),
		atoi(row[25]),
		atoi(row[28]),
		atoi(row[29]),
		defaultNPCTypeStruct.MR,
		defaultNPCTypeStruct.CR,
		defaultNPCTypeStruct.DR,
		defaultNPCTypeStruct.FR,
		defaultNPCTypeStruct.PR,
		defaultNPCTypeStruct.Corrup,
		defaultNPCTypeStruct.AC,
		defaultNPCTypeStruct.STR,
		defaultNPCTypeStruct.STA,
		defaultNPCTypeStruct.DEX,
		defaultNPCTypeStruct.AGI,
		defaultNPCTypeStruct.INT,
		defaultNPCTypeStruct.WIS,
		defaultNPCTypeStruct.CHA,
		defaultNPCTypeStruct.ATK
	);

	loaded_bot = new Bot(bot_id, atoi(row[0]), atoi(row[1]), atof(row[14]), atoi(row[6]), tempNPCStruct);
	if (loaded_bot) {
		loaded_bot->SetShowHelm((atoi(row[43]) > 0 ? true : false));
		uint32 bfd = atoi(row[44]);
		if (bfd < 1)
			bfd = 1;
		if (bfd > BOT_FOLLOW_DISTANCE_DEFAULT_MAX)
			bfd = BOT_FOLLOW_DISTANCE_DEFAULT_MAX;
		loaded_bot->SetFollowDistance(bfd);
		
	}

	return true;
}

bool BotDatabase::SaveNewBot(Bot* bot_inst, uint32& bot_id)
{
	if (!bot_inst)
		return false;
	
	query = StringFormat(
		"INSERT INTO `bot_data` ("
		" `owner_id`,"
		" `spells_id`,"
		" `name`,"
		" `last_name`,"
		" `zone_id`,"
		" `gender`,"
		" `race`,"
		" `class`,"
		" `level`,"
		" `creation_day`,"
		" `last_spawn`,"
		" `time_spawned`,"
		" `size`,"
		" `face`,"
		" `hair_color`,"
		" `hair_style`,"
		" `beard`,"
		" `beard_color`,"
		" `eye_color_1`,"
		" `eye_color_2`,"
		" `drakkin_heritage`,"
		" `drakkin_tattoo`,"
		" `drakkin_details`,"
		" `ac`,"
		" `atk`,"
		" `hp`,"
		" `mana`,"
		" `str`,"
		" `sta`,"
		" `cha`,"
		" `dex`,"
		" `int`,"
		" `agi`,"
		" `wis`,"
		" `fire`,"
		" `cold`,"
		" `magic`,"
		" `poison`,"
		" `disease`,"
		" `corruption`,"
		" `show_helm`,"
		" `follow_distance`"
		")"
		" VALUES ("
		"'%u',"					/* owner_id */
		" '%u',"				/* spells_id */
		" '%s',"				/* name */
		" '%s',"				/* last_name */
		" '%i',"				/* zone_id */
		" '%i',"				/* gender */
		" '%i',"				/* race */
		" '%i',"				/* class */
		" '%u',"				/* level */
		" UNIX_TIMESTAMP(),"	/* creation_day */
		" UNIX_TIMESTAMP(),"	/* last_spawn */
		" 0,"					/* time_spawned */
		" '%f',"				/* size */
		" '%i',"				/* face */
		" '%i',"				/* hair_color */
		" '%i',"				/* hair_style */
		" '%i',"				/* beard */
		" '%i',"				/* beard_color */
		" '%i',"				/* eye_color_1 */
		" '%i',"				/* eye_color_2 */
		" '%i',"				/* drakkin_heritage */
		" '%i',"				/* drakkin_tattoo */
		" '%i',"				/* drakkin_details */
		" '%i',"				/* ac */
		" '%i',"				/* atk */
		" '%i',"				/* hp */
		" '%i',"				/* mana */
		" '%i',"				/* str */
		" '%i',"				/* sta */
		" '%i',"				/* cha */
		" '%i',"				/* dex */
		" '%i',"				/* int */
		" '%i',"				/* agi */
		" '%i',"				/* wis */
		" '%i',"				/* fire */
		" '%i',"				/* cold */
		" '%i',"				/* magic */
		" '%i',"				/* poison */
		" '%i',"				/* disease */
		" '%i',"				/* corruption */
		" '1',"					/* show_helm */
		" '%i'"					/* follow_distance */
		")",
		bot_inst->GetBotOwnerCharacterID(),
		bot_inst->GetBotSpellID(),
		bot_inst->GetCleanName(),
		bot_inst->GetLastName(),
		bot_inst->GetLastZoneID(),
		bot_inst->GetGender(),
		bot_inst->GetRace(),
		bot_inst->GetClass(),
		bot_inst->GetLevel(),
		bot_inst->GetSize(),
		bot_inst->GetLuclinFace(),
		bot_inst->GetHairColor(),
		bot_inst->GetHairStyle(),
		bot_inst->GetBeard(),
		bot_inst->GetBeardColor(),
		bot_inst->GetEyeColor1(),
		bot_inst->GetEyeColor2(),
		bot_inst->GetDrakkinHeritage(),
		bot_inst->GetDrakkinTattoo(),
		bot_inst->GetDrakkinDetails(),
		bot_inst->GetAC(),
		bot_inst->GetATK(),
		bot_inst->GetHP(),
		bot_inst->GetMana(),
		bot_inst->GetSTR(),
		bot_inst->GetSTA(),
		bot_inst->GetCHA(),
		bot_inst->GetDEX(),
		bot_inst->GetINT(),
		bot_inst->GetAGI(),
		bot_inst->GetWIS(),
		bot_inst->GetFR(),
		bot_inst->GetCR(),
		bot_inst->GetMR(),
		bot_inst->GetPR(),
		bot_inst->GetDR(),
		bot_inst->GetCorrup(),
		BOT_FOLLOW_DISTANCE_DEFAULT
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	bot_id = results.LastInsertedID();

	return true;
}

bool BotDatabase::SaveBot(Bot* bot_inst)
{
	if (!bot_inst)
		return false;
	
	query = StringFormat(
		"UPDATE `bot_data`"
		" SET"
		" `owner_id` = '%u',"
		" `spells_id` = '%u',"
		" `name` = '%s',"
		" `last_name` = '%s',"
		" `zone_id` = '%i',"
		" `gender` = '%i',"
		" `race` = '%i',"
		" `class` = '%i',"
		" `level` = '%u',"
		" `last_spawn` = UNIX_TIMESTAMP(),"
		" `time_spawned` = '%u',"
		" `size` = '%f',"
		" `face` = '%i',"
		" `hair_color` = '%i',"
		" `hair_style` = '%i',"
		" `beard` = '%i',"
		" `beard_color` = '%i',"
		" `eye_color_1` = '%i',"
		" `eye_color_2` = '%i',"
		" `drakkin_heritage` = '%i',"
		" `drakkin_tattoo` = '%i',"
		" `drakkin_details` = '%i',"
		" `ac` = '%i',"
		" `atk` = '%i',"
		" `hp` = '%i',"
		" `mana` = '%i',"
		" `str` = '%i',"
		" `sta` = '%i',"
		" `cha` = '%i',"
		" `dex` = '%i',"
		" `int` = '%i',"
		" `agi` = '%i',"
		" `wis` = '%i',"
		" `fire` = '%i',"
		" `cold` = '%i',"
		" `magic` = '%i',"
		" `poison` = '%i',"
		" `disease` = '%i',"
		" `corruption` = '%i',"
		" `show_helm` = '%i',"
		" `follow_distance` = '%i'"
		" WHERE `bot_id` = '%u'",
		bot_inst->GetBotOwnerCharacterID(),
		bot_inst->GetBotSpellID(),
		bot_inst->GetCleanName(),
		bot_inst->GetLastName(),
		bot_inst->GetLastZoneID(),
		bot_inst->GetBaseGender(),
		bot_inst->GetBaseRace(),
		bot_inst->GetClass(),
		bot_inst->GetLevel(),
		bot_inst->GetTotalPlayTime(),
		bot_inst->GetBaseSize(),
		bot_inst->GetLuclinFace(),
		bot_inst->GetHairColor(),
		bot_inst->GetHairStyle(),
		bot_inst->GetBeard(),
		bot_inst->GetBeardColor(),
		bot_inst->GetEyeColor1(),
		bot_inst->GetEyeColor2(),
		bot_inst->GetDrakkinHeritage(),
		bot_inst->GetDrakkinTattoo(),
		bot_inst->GetDrakkinDetails(),
		bot_inst->GetBaseAC(),
		bot_inst->GetBaseATK(),
		bot_inst->GetHP(),
		bot_inst->GetMana(),
		bot_inst->GetBaseSTR(),
		bot_inst->GetBaseSTA(),
		bot_inst->GetBaseCHA(),
		bot_inst->GetBaseDEX(),
		bot_inst->GetBaseINT(),
		bot_inst->GetBaseAGI(),
		bot_inst->GetBaseWIS(),
		bot_inst->GetBaseFR(),
		bot_inst->GetBaseCR(),
		bot_inst->GetBaseMR(),
		bot_inst->GetBasePR(),
		bot_inst->GetBaseDR(),
		bot_inst->GetBaseCorrup(),
		((bot_inst->GetShowHelm()) ? (1) : (0)),
		bot_inst->GetFollowDistance(),
		bot_inst->GetBotID()
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::DeleteBot(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("DELETE FROM `bot_data` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadBuffs(Bot* bot_inst)
{
	if (!bot_inst)
		return false;

	query = StringFormat(
		"SELECT"
		" `spell_id`,"
		" `caster_level`,"
		" `duration_formula`,"
		" `tics_remaining`,"
		" `poison_counters`,"
		" `disease_counters`,"
		" `curse_counters`,"
		" `corruption_counters`,"
		" `numhits`,"
		" `melee_rune`,"
		" `magic_rune`,"
		" `dot_rune`,"
		" `persistent`,"
		" `caston_x`,"
		" `caston_y`,"
		" `caston_z`,"
		" `extra_di_chance`,"
		" `instrument_mod`"
		" FROM `bot_buffs`"
		" WHERE `bot_id` = '%u'",
		bot_inst->GetBotID()
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	Buffs_Struct* bot_buffs = bot_inst->GetBuffs();
	if (!bot_buffs)
		return false;

	int buff_count = 0;
	for (auto row = results.begin(); row != results.end() && buff_count < BUFF_COUNT; ++row) {
		bot_buffs[buff_count].spellid = atoi(row[0]);
		bot_buffs[buff_count].casterlevel = atoi(row[1]);
		//row[2] (duration_formula) can probably be removed
		bot_buffs[buff_count].ticsremaining = atoi(row[3]);

		if (CalculatePoisonCounters(bot_buffs[buff_count].spellid) > 0)
			bot_buffs[buff_count].counters = atoi(row[4]);
		else if (CalculateDiseaseCounters(bot_buffs[buff_count].spellid) > 0)
			bot_buffs[buff_count].counters = atoi(row[5]);
		else if (CalculateCurseCounters(bot_buffs[buff_count].spellid) > 0)
			bot_buffs[buff_count].counters = atoi(row[6]);
		else if (CalculateCorruptionCounters(bot_buffs[buff_count].spellid) > 0)
			bot_buffs[buff_count].counters = atoi(row[7]);

		bot_buffs[buff_count].numhits = atoi(row[8]);
		bot_buffs[buff_count].melee_rune = atoi(row[9]);
		bot_buffs[buff_count].magic_rune = atoi(row[10]);
		bot_buffs[buff_count].dot_rune = atoi(row[11]);
		bot_buffs[buff_count].persistant_buff = ((atoi(row[12])) ? (true) : (false));
		bot_buffs[buff_count].caston_x = atoi(row[13]);
		bot_buffs[buff_count].caston_y = atoi(row[14]);
		bot_buffs[buff_count].caston_z = atoi(row[15]);
		bot_buffs[buff_count].ExtraDIChance = atoi(row[16]);
		bot_buffs[buff_count].instrument_mod = atoi(row[17]);
		bot_buffs[buff_count].casterid = 0;
		++buff_count;
	}

	return true;
}

bool BotDatabase::SaveBuffs(Bot* bot_inst)
{
	if (!bot_inst)
		return false;

	if (!DeleteBuffs(bot_inst->GetBotID()))
		return false;

	Buffs_Struct* bot_buffs = bot_inst->GetBuffs();
	if (!bot_buffs)
		return false;

	for (int buff_index = 0; buff_index < BUFF_COUNT; ++buff_index) {
		if (bot_buffs[buff_index].spellid <= 0 || bot_buffs[buff_index].spellid == SPELL_UNKNOWN)
			continue;

		query = StringFormat(
			"INSERT INTO `bot_buffs` ("
			"`bot_id`,"
			" `spell_id`,"
			" `caster_level`,"
			" `duration_formula`,"
			" `tics_remaining`,"
			" `poison_counters`,"
			" `disease_counters`,"
			" `curse_counters`,"
			" `corruption_counters`,"
			" `numhits`,"
			" `melee_rune`,"
			" `magic_rune`,"
			" `dot_rune`,"
			" `persistent`,"
			" `caston_x`,"
			" `caston_y`,"
			" `caston_z`,"
			" `extra_di_chance`"
			")"
			" VALUES ("
			"'%u',"				/* bot_id */
			" '%u',"			/* spell_id */
			" '%u',"			/* caster_level */
			" '%u',"			/* duration_formula */
			" '%u',"			/* tics_remaining */
			" '%u',"			/* poison_counters */
			" '%u',"			/* disease_counters */
			" '%u',"			/* curse_counters */
			" '%u',"			/* corruption_counters */
			" '%u',"			/* numhits */
			" '%u',"			/* melee_rune */
			" '%u',"			/* magic_rune */
			" '%u',"			/* dot_rune */
			" '%u',"			/* persistent */
			" '%i',"			/* caston_x */
			" '%i',"			/* caston_y */
			" '%i',"			/* caston_z */
			" '%i'"				/* extra_di_chance */
			")",
			bot_inst->GetBotID(),
			bot_buffs[buff_index].spellid,
			bot_buffs[buff_index].casterlevel,
			spells[bot_buffs[buff_index].spellid].buffdurationformula,
			bot_buffs[buff_index].ticsremaining,
			((CalculatePoisonCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			((CalculateDiseaseCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			((CalculateCurseCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			((CalculateCorruptionCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			bot_buffs[buff_index].numhits,
			bot_buffs[buff_index].melee_rune,
			bot_buffs[buff_index].magic_rune,
			bot_buffs[buff_index].dot_rune,
			((bot_buffs[buff_index].persistant_buff) ? (1) : (0)),
			bot_buffs[buff_index].caston_x,
			bot_buffs[buff_index].caston_y,
			bot_buffs[buff_index].caston_z,
			bot_buffs[buff_index].ExtraDIChance
		);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			DeleteBuffs(bot_inst->GetBotID());
			return false;
		}
	}

	return true;
}

bool BotDatabase::DeleteBuffs(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("DELETE FROM `bot_buffs` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadStance(const uint32 bot_id, int& bot_stance)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `stance_id` FROM `bot_stances` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	bot_stance = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadStance(Bot* bot_inst, bool& stance_flag)
{
	if (!bot_inst)
		return false;

	bot_inst->SetDefaultBotStance();

	query = StringFormat("SELECT `stance_id` FROM `bot_stances` WHERE `bot_id` = '%u' LIMIT 1", bot_inst->GetBotID());
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	bot_inst->SetBotStance((BotStanceType)atoi(row[0]));
	stance_flag = true;

	return true;
}

bool BotDatabase::SaveStance(const uint32 bot_id, const int bot_stance)
{
	if (!bot_id)
		return false;

	if (!DeleteStance(bot_id))
		return false;

	query = StringFormat("INSERT INTO `bot_stances` (`bot_id`, `stance_id`) VALUES ('%u', '%u')", bot_id, bot_stance);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteStance(bot_id);
		return false;
	}

	return true;
}

bool BotDatabase::SaveStance(Bot* bot_inst)
{
	if (!bot_inst)
		return false;

	if (!DeleteStance(bot_inst->GetBotID()))
		return false;

	query = StringFormat("INSERT INTO `bot_stances` (`bot_id`, `stance_id`) VALUES ('%u', '%u')", bot_inst->GetBotID(), bot_inst->GetBotStance());
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteStance(bot_inst->GetBotID());
		return false;
	}

	return true;
}

bool BotDatabase::DeleteStance(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("DELETE FROM `bot_stances` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadTimers(Bot* bot_inst)
{
	if (!bot_inst)
		return false;

	query = StringFormat(
		"SELECT"
		" IfNull(bt.`timer_id`, '0') As timer_id,"
		" IfNull(bt.`timer_value`, '0') As timer_value,"
		" IfNull(MAX(sn.`recast_time`), '0') AS MaxTimer"
		" FROM `bot_timers` bt, `spells_new` sn"
		" WHERE bt.`bot_id` = '%u' AND sn.`EndurTimerIndex` = ("
		"SELECT case"
		" WHEN timer_id > '%i' THEN timer_id - '%i'"
		" ELSE timer_id END AS timer_id"
		" FROM `bot_timers` WHERE `timer_id` = bt.`timer_id` AND `bot_id` = bt.`bot_id`" // double-check validity
		")"
		" AND sn.`classes%i` <= '%i'",
		bot_inst->GetBotID(),
		(DisciplineReuseStart - 1),
		(DisciplineReuseStart - 1),
		bot_inst->GetClass(),
		bot_inst->GetLevel()
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	uint32* bot_timers = bot_inst->GetTimers();
	if (!bot_timers)
		return false;

	int timer_id = 0;
	uint32 timer_value = 0;
	uint32 max_value = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		timer_id = atoi(row[0]) - 1;
		timer_value = atoi(row[1]);
		max_value = atoi(row[2]);

		if (timer_id >= 0 && timer_id < MaxTimer && timer_value < (Timer::GetCurrentTime() + max_value))
			bot_timers[timer_id] = timer_value;
	}
	
	return true;
}

bool BotDatabase::SaveTimers(Bot* bot_inst)
{
	if (!bot_inst)
		return false;

	if (!DeleteTimers(bot_inst->GetBotID()))
		return false;

	uint32* bot_timers = bot_inst->GetTimers();
	if (!bot_timers)
		return false;

	for (int timer_index = 0; timer_index < MaxTimer; ++timer_index) {
		if (bot_timers[timer_index] <= Timer::GetCurrentTime())
			continue;

		query = StringFormat("INSERT INTO `bot_timers` (`bot_id`, `timer_id`, `timer_value`) VALUES ('%u', '%u', '%u')", bot_inst->GetBotID(), (timer_index + 1), bot_timers[timer_index]);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			DeleteTimers(bot_inst->GetBotID());
			return false;
		}
	}

	return true;
}

bool BotDatabase::DeleteTimers(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("DELETE FROM `bot_timers` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadGuildMembership(const uint32 bot_id, uint32& guild_id, uint8& guild_rank, std::string& guild_name)
{
	if (!bot_id)
		return false;

	query = StringFormat(
		"SELECT"
		" gm.`guild_id`,"
		" gm.`rank`,"
		" g.`name`"
		" FROM `vw_guild_members` AS gm"
		" JOIN `guilds` AS g"
		" ON gm.`guild_id` = g.`id`"
		" WHERE gm.`char_id` = '%u'"
		" AND gm.`mob_type` = 'B'"
		" LIMIT 1",
		bot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	guild_id = atoi(row[0]);
	guild_rank = atoi(row[1]);
	guild_name = row[2];

	return true;
}

bool BotDatabase::SaveGuildMembership(const uint32 bot_id, const uint32 guild_id, const uint8 guild_rank)
{
	if (!bot_id || !guild_id)
		return false;

	if (!DeleteGuildMembership(bot_id))
		return false;

	query = StringFormat("INSERT INTO `bot_guild_members` SET `bot_id` = '%u', `guild_id` = '%u', `rank` = '%u'", bot_id, guild_id, guild_rank);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteGuildMembership(bot_id);
		return false;
	}

	return true;
}

bool BotDatabase::DeleteGuildMembership(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("DELETE FROM `bot_guild_members` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}


/* Bot inventory functions   */
bool BotDatabase::QueryInventoryCount(const uint32 bot_id, uint32& item_count)
{
	if (!bot_id)
		return false;
	
	query = StringFormat("SELECT COUNT(`inventories_index`) FROM `bot_inventories` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	item_count = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadItems(const uint32 bot_id, EQEmu::InventoryProfile& inventory_inst)
{
	if (!bot_id)
		return false;

	query = StringFormat(
		"SELECT"
		" `slot_id`,"
		" `item_id`,"
		" `inst_charges`,"
		" `inst_color`,"
		" `inst_no_drop`,"
		" `inst_custom_data`,"
		" `ornament_icon`,"
		" `ornament_id_file`,"
		" `ornament_hero_model`,"
		" `augment_1`,"
		" `augment_2`,"
		" `augment_3`,"
		" `augment_4`, "
		" `augment_5`,"
		" `augment_6`"
		" FROM `bot_inventories`"
		" WHERE `bot_id` = '%i'"
		" ORDER BY `slot_id`",
		bot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;
	
	for (auto row = results.begin(); row != results.end(); ++row) {
		int16 slot_id = atoi(row[0]);
		if ((slot_id < EQEmu::legacy::EQUIPMENT_BEGIN || slot_id > EQEmu::legacy::EQUIPMENT_END) && slot_id != EQEmu::inventory::slotPowerSource)
			continue;

		uint32 item_id = atoi(row[1]);
		uint16 item_charges = (uint16)atoi(row[2]);

		EQEmu::ItemInstance* item_inst = database.CreateItem(
			item_id,
			item_charges,
			(uint32)atoul(row[9]),
			(uint32)atoul(row[10]),
			(uint32)atoul(row[11]),
			(uint32)atoul(row[12]),
			(uint32)atoul(row[13]),
			(uint32)atoul(row[14])
		);
		if (!item_inst) {
			Log(Logs::General, Logs::Error, "Warning: bot_id '%i' has an invalid item_id '%i' in inventory slot '%i'", bot_id, item_id, slot_id);
			continue;
		}

		if (item_charges == 0x7FFF)
			item_inst->SetCharges(-1);
		else if (item_charges == 0 && item_inst->IsStackable()) // Stackable items need a minimum charge of 1 remain moveable.
			item_inst->SetCharges(1);
		else
			item_inst->SetCharges(item_charges);

		uint32 item_color = atoul(row[3]);
		if (item_color > 0)
			item_inst->SetColor(item_color);

		if (item_inst->GetItem()->Attuneable) {
			if (atoi(row[4]))
				item_inst->SetAttuned(true);
			else if (((slot_id >= EQEmu::legacy::EQUIPMENT_BEGIN) && (slot_id <= EQEmu::legacy::EQUIPMENT_END) || slot_id == EQEmu::inventory::slotPowerSource))
				item_inst->SetAttuned(true);
		}

		if (row[5]) {
			std::string data_str(row[5]);
			std::string idAsString;
			std::string value;
			bool use_id = true;

			for (int i = 0; i < data_str.length(); ++i) {
				if (data_str[i] == '^') {
					if (!use_id) {
						item_inst->SetCustomData(idAsString, value);
						idAsString.clear();
						value.clear();
					}

					use_id = !use_id;
					continue;
				}

				char v = data_str[i];
				if (use_id)
					idAsString.push_back(v);
				else
					value.push_back(v);
			}
		}

		item_inst->SetOrnamentIcon((uint32)atoul(row[6]));
		item_inst->SetOrnamentationIDFile((uint32)atoul(row[7]));
		item_inst->SetOrnamentHeroModel((uint32)atoul(row[8]));

		if (inventory_inst.PutItem(slot_id, *item_inst) == INVALID_INDEX)
			Log(Logs::General, Logs::Error, "Warning: Invalid slot_id for item in inventory: bot_id = '%i', item_id = '%i', slot_id = '%i'", bot_id, item_id, slot_id);

		safe_delete(item_inst);
	}

	return true;
}

bool BotDatabase::SaveItems(Bot* bot_inst)
{
	return false;
}

bool BotDatabase::DeleteItems(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("DELETE FROM `bot_inventories` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadItemBySlot(Bot* bot_inst)
{
	return false;
}

bool BotDatabase::LoadItemBySlot(const uint32 bot_id, const uint32 slot_id, uint32& item_id)
{
	if (!bot_id || (slot_id > EQEmu::legacy::EQUIPMENT_END && slot_id != EQEmu::inventory::slotPowerSource))
		return false;
	
	query = StringFormat("SELECT `item_id` FROM `bot_inventories` WHERE `bot_id` = '%i' AND `slot_id` = '%i' LIMIT 1", bot_id, slot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	item_id = atoi(row[0]);

	return true;
}

bool BotDatabase::SaveItemBySlot(Bot* bot_inst, const uint32 slot_id, const EQEmu::ItemInstance* item_inst)
{
	if (!bot_inst || !bot_inst->GetBotID() || (slot_id > EQEmu::legacy::EQUIPMENT_END && slot_id != EQEmu::inventory::slotPowerSource))
		return false;

	if (!DeleteItemBySlot(bot_inst->GetBotID(), slot_id))
		return false;

	if (!item_inst || !item_inst->GetID())
		return true;
	
	uint32 augment_id[EQEmu::inventory::SocketCount] = { 0, 0, 0, 0, 0, 0 };
	for (int augment_iter = EQEmu::inventory::socketBegin; augment_iter < EQEmu::inventory::SocketCount; ++augment_iter)
		augment_id[augment_iter] = item_inst->GetAugmentItemID(augment_iter);
	
	uint16 item_charges = 0;
	if (item_inst->GetCharges() >= 0)
		item_charges = item_inst->GetCharges();
	else
		item_charges = 0x7FFF;

	query = StringFormat(
		"INSERT INTO `bot_inventories` ("
		"`bot_id`,"
		" `slot_id`,"
		" `item_id`,"
		" `inst_charges`,"
		" `inst_color`,"
		" `inst_no_drop`,"
		" `inst_custom_data`,"
		" `ornament_icon`,"
		" `ornament_id_file`,"
		" `ornament_hero_model`,"
		" `augment_1`,"
		" `augment_2`,"
		" `augment_3`,"
		" `augment_4`,"
		" `augment_5`,"
		" `augment_6`"
		")"
		" VALUES ("
		"'%lu',"			/* bot_id */
		" '%lu',"			/* slot_id */
		" '%lu',"			/* item_id */
		" '%lu',"			/* inst_charges */
		" '%lu',"			/* inst_color */
		" '%lu',"			/* inst_no_drop */
		" '%s',"			/* inst_custom_data */
		" '%lu',"			/* ornament_icon */
		" '%lu',"			/* ornament_id_file */
		" '%lu',"			/* ornament_hero_model */
		" '%lu',"			/* augment_1 */
		" '%lu',"			/* augment_2 */
		" '%lu',"			/* augment_3 */
		" '%lu',"			/* augment_4 */
		" '%lu',"			/* augment_5 */
		" '%lu'"			/* augment_6 */
		")",
		(unsigned long)bot_inst->GetBotID(),
		(unsigned long)slot_id,
		(unsigned long)item_inst->GetID(),
		(unsigned long)item_charges,
		(unsigned long)item_inst->GetColor(),
		(unsigned long)(item_inst->IsAttuned() ? 1 : 0),
		item_inst->GetCustomDataString().c_str(),
		(unsigned long)item_inst->GetOrnamentationIcon(),
		(unsigned long)item_inst->GetOrnamentationIDFile(),
		(unsigned long)item_inst->GetOrnamentHeroModel(),
		(unsigned long)augment_id[0],
		(unsigned long)augment_id[1],
		(unsigned long)augment_id[2],
		(unsigned long)augment_id[3],
		(unsigned long)augment_id[4],
		(unsigned long)augment_id[5]
	);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteItemBySlot(bot_inst->GetBotID(), slot_id);
		return false;
	}

	return true;
}

bool BotDatabase::DeleteItemBySlot(const uint32 bot_id, const uint32 slot_id)
{
	if (!bot_id || (slot_id > EQEmu::legacy::EQUIPMENT_END && slot_id != EQEmu::inventory::slotPowerSource))
		return false;

	query = StringFormat("DELETE FROM `bot_inventories` WHERE `bot_id` = '%u' AND `slot_id` = '%u'", bot_id, slot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadEquipmentColor(const uint32 bot_id, const uint8 material_slot_id, uint32& rgb)
{
	if (!bot_id)
		return false;

	int16 slot_id = EQEmu::InventoryProfile::CalcSlotFromMaterial(material_slot_id);
	if (slot_id == INVALID_INDEX)
		return false;
	
	query = StringFormat("SELECT `inst_color` FROM `bot_inventories` WHERE `bot_id` = '%u' AND `slot_id` = '%u' LIMIT 1", bot_id, slot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	rgb = atoul(row[0]);

	return true;
}

bool BotDatabase::SaveEquipmentColor(const uint32 bot_id, const int16 slot_id, const uint32 rgb)
{
	if (!bot_id)
		return false;

	bool all_flag = (slot_id == -2);
	if ((slot_id < EQEmu::legacy::EQUIPMENT_BEGIN || slot_id > EQEmu::legacy::EQUIPMENT_END) && slot_id != EQEmu::inventory::slotPowerSource && !all_flag)
		return false;

	std::string where_clause;
	if (all_flag)
		where_clause = StringFormat(" AND `slot_id` IN ('%u', '%u', '%u', '%u', '%u', '%u', '%u')", EQEmu::inventory::slotHead, EQEmu::inventory::slotArms, EQEmu::inventory::slotWrist1, EQEmu::inventory::slotHands, EQEmu::inventory::slotChest, EQEmu::inventory::slotLegs, EQEmu::inventory::slotFeet);
	else
		where_clause = StringFormat(" AND `slot_id` = '%u'", slot_id);

	query = StringFormat(
		"UPDATE `bot_inventories`"
		" SET `inst_color` = '%u'"
		" WHERE `bot_id` = '%u'"
		" %s",
		rgb,
		bot_id,
		where_clause.c_str()
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}


/* Bot pet functions   */
bool BotDatabase::LoadPetIndex(const uint32 bot_id, uint32& pet_index)
{
	if (!bot_id)
		return false;
	
	query = StringFormat("SELECT `pets_index` FROM `bot_pets` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	pet_index = atoi(row[0]);
	
	return true;
}

bool BotDatabase::LoadPetSpellID(const uint32 bot_id, uint32& pet_spell_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `spell_id` FROM `bot_pets` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	pet_spell_id = atoi(row[0]);
	
	return true;
}

bool BotDatabase::LoadPetStats(const uint32 bot_id, std::string& pet_name, uint32& pet_mana, uint32& pet_hp, uint32& pet_spell_id)
{
	if (!bot_id)
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	query = StringFormat("SELECT `spell_id`, `name`, `mana`, `hp` FROM `bot_pets` WHERE `pets_index` = '%u' LIMIT 1", saved_pet_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	pet_spell_id = atoi(row[0]);
	pet_name = row[1];
	pet_mana = atoi(row[2]);
	pet_hp = atoi(row[3]);
	
	return true;
}

bool BotDatabase::SavePetStats(const uint32 bot_id, const std::string& pet_name, const uint32 pet_mana, const uint32 pet_hp, const uint32 pet_spell_id)
{
	if (!bot_id || pet_name.empty() || !pet_spell_id || pet_spell_id > SPDAT_RECORDS)
		return false;
	
	if (!DeletePetItems(bot_id))
		return false;
	if (!DeletePetBuffs(bot_id))
		return false;
	if (!DeletePetStats(bot_id))
		return false;

	query = StringFormat(
		"INSERT INTO `bot_pets` ("
		"`spell_id`,"
		" `bot_id`,"
		" `name`,"
		" `mana`,"
		" `hp`"
		")"
		" VALUES ("
		"'%u',"
		" '%u',"
		" '%s',"
		" '%u',"
		" '%u'"
		")",
		pet_spell_id,
		bot_id,
		pet_name.c_str(),
		pet_mana,
		pet_hp
	);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeletePetStats(bot_id);
		return false;
	}

	return true;
}

bool BotDatabase::DeletePetStats(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	query = StringFormat("DELETE FROM `bot_pets` WHERE `pets_index` = '%u'", saved_pet_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadPetBuffs(const uint32 bot_id, SpellBuff_Struct* pet_buffs)
{
	if (!bot_id)
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	query = StringFormat("SELECT `spell_id`, `caster_level`, `duration` FROM `bot_pet_buffs` WHERE `pets_index` = '%u'", saved_pet_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	int buff_index = 0;
	for (auto row = results.begin(); row != results.end() && buff_index < PET_BUFF_COUNT; ++row) {
		pet_buffs[buff_index].spellid = atoi(row[0]);
		pet_buffs[buff_index].level = atoi(row[1]);
		pet_buffs[buff_index].duration = atoi(row[2]);

		// Work around for loading the counters and setting them back to max. Need entry in DB for saved counters
		if (CalculatePoisonCounters(pet_buffs[buff_index].spellid) > 0)
			pet_buffs[buff_index].counters = CalculatePoisonCounters(pet_buffs[buff_index].spellid);
		else if (CalculateDiseaseCounters(pet_buffs[buff_index].spellid) > 0)
			pet_buffs[buff_index].counters = CalculateDiseaseCounters(pet_buffs[buff_index].spellid);
		else if (CalculateCurseCounters(pet_buffs[buff_index].spellid) > 0)
			pet_buffs[buff_index].counters = CalculateCurseCounters(pet_buffs[buff_index].spellid);
		else if (CalculateCorruptionCounters(pet_buffs[buff_index].spellid) > 0)
			pet_buffs[buff_index].counters = CalculateCorruptionCounters(pet_buffs[buff_index].spellid);

		++buff_index;
	}
	
	return true;
}

bool BotDatabase::SavePetBuffs(const uint32 bot_id, const SpellBuff_Struct* pet_buffs, bool delete_flag)
{
	// Only use 'delete_flag' if not invoked after a botdb.SavePetStats() call
	
	if (!bot_id || !pet_buffs)
		return false;

	if (delete_flag && !DeletePetBuffs(bot_id))
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	for (int buff_index = 0; buff_index < PET_BUFF_COUNT; ++buff_index) {
		if (!pet_buffs[buff_index].spellid || pet_buffs[buff_index].spellid == SPELL_UNKNOWN)
			continue;

		query = StringFormat(
			"INSERT INTO `bot_pet_buffs` ("
			"`pets_index`,"
			" `spell_id`,"
			" `caster_level`,"
			" `duration`"
			")"
			" VALUES ("
			"'%u',"
			" '%u',"
			" '%u',"
			" '%u'"
			")",
			saved_pet_index,
			pet_buffs[buff_index].spellid,
			pet_buffs[buff_index].level,
			pet_buffs[buff_index].duration
		);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			DeletePetBuffs(bot_id);
			return false;
		}
	}

	return true;
}

bool BotDatabase::DeletePetBuffs(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	query = StringFormat("DELETE FROM `bot_pet_buffs` WHERE `pets_index` = '%u'", saved_pet_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadPetItems(const uint32 bot_id, uint32* pet_items)
{
	if (!bot_id || !pet_items)
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	query = StringFormat("SELECT `item_id` FROM `bot_pet_inventories` WHERE `pets_index` = '%u'", saved_pet_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	int item_index = 0;
	for (auto row = results.begin(); row != results.end() && item_index < EQEmu::legacy::EQUIPMENT_SIZE; ++row) {
		pet_items[item_index] = atoi(row[0]);
		++item_index;
	}
	
	return true;
}

bool BotDatabase::SavePetItems(const uint32 bot_id, const uint32* pet_items, bool delete_flag)
{
	// Only use 'delete_flag' if not invoked after a botdb.SavePetStats() call
	
	if (!bot_id || !pet_items)
		return false;

	if (delete_flag && !DeletePetItems(bot_id))
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	for (int item_index = 0; item_index < EQEmu::legacy::EQUIPMENT_SIZE; ++item_index) {
		if (!pet_items[item_index])
			continue;

		query = StringFormat("INSERT INTO `bot_pet_inventories` (`pets_index`, `item_id`) VALUES ('%u', '%u')", saved_pet_index, pet_items[item_index]);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			DeletePetItems(bot_id);
			return false;
		}
	}

	return true;
}

bool BotDatabase::DeletePetItems(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	uint32 saved_pet_index = 0;
	if (!LoadPetIndex(bot_id, saved_pet_index))
		return false;
	if (!saved_pet_index)
		return true;

	query = StringFormat("DELETE FROM `bot_pet_inventories` WHERE `pets_index` = '%u'", saved_pet_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}


/* Bot command functions   */
bool BotDatabase::LoadInspectMessage(const uint32 bot_id, InspectMessage_Struct& inspect_message)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `inspect_message` FROM `bot_inspect_messages` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	std::string bot_message = row[0];
	if (bot_message.size() > 255)
		bot_message = bot_message.substr(0, 255);
	if (bot_message.empty())
		return true;

	memcpy(inspect_message.text, bot_message.c_str(), bot_message.size());
	
	return true;
}

bool BotDatabase::SaveInspectMessage(const uint32 bot_id, const InspectMessage_Struct& inspect_message)
{
	if (!bot_id)
		return false;
	
	if (!DeleteInspectMessage(bot_id))
		return false;

	std::string bot_message = inspect_message.text;
	if (bot_message.size() > 255)
		bot_message = bot_message.substr(0, 255);
	if (bot_message.empty())
		return true;

	query = StringFormat("INSERT INTO `bot_inspect_messages` (`bot_id`, `inspect_message`) VALUES ('%u', '%s')", bot_id, bot_message.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteInspectMessage(bot_id);
		return false;
	}

	return true;
}

bool BotDatabase::DeleteInspectMessage(const uint32 bot_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("DELETE FROM `bot_inspect_messages` WHERE `bot_id` = '%u'", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::SaveAllInspectMessages(const uint32 owner_id, const InspectMessage_Struct& inspect_message)
{
	if (!owner_id)
		return false;

	if (!DeleteAllInspectMessages(owner_id))
		return false;
	
	std::string bot_message = inspect_message.text;
	if (bot_message.size() > 255)
		bot_message = bot_message.substr(0, 255);
	if (bot_message.empty())
		return true;

	query = StringFormat("INSERT INTO `bot_inspect_messages` (`bot_id`, `inspect_message`) SELECT `bot_id`, '%s' inspect_message FROM `bot_data` WHERE `owner_id` = '%u'", bot_message.c_str(), owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteAllInspectMessages(owner_id);
		return false;
	}

	return true;
}

bool BotDatabase::DeleteAllInspectMessages(const uint32 owner_id)
{
	if (!owner_id)
		return false;

	query = StringFormat("DELETE FROM `bot_inspect_messages` WHERE `bot_id` IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = '%u')", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::SaveAllArmorColorBySlot(const uint32 owner_id, const int16 slot_id, const uint32 rgb_value)
{
	if (!owner_id)
		return false;

	query = StringFormat(
		"UPDATE `bot_inventories` bi"
		" INNER JOIN `bot_data` bd"
		" ON bd.`owner_id` = '%u'"
		" SET bi.`inst_color` = '%u'"
		" WHERE bi.`bot_id` = bd.`bot_id`"
		" AND bi.`slot_id` IN ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')"
		" AND bi.`slot_id` = '%i'",
		owner_id,
		rgb_value,
		EQEmu::inventory::slotHead, EQEmu::inventory::slotChest, EQEmu::inventory::slotArms, EQEmu::inventory::slotWrist1, EQEmu::inventory::slotWrist2, EQEmu::inventory::slotHands, EQEmu::inventory::slotLegs, EQEmu::inventory::slotFeet,
		slot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::SaveAllArmorColors(const uint32 owner_id, const uint32 rgb_value)
{
	if (!owner_id)
		return false;

	query = StringFormat(
		"UPDATE `bot_inventories` bi"
		" INNER JOIN `bot_data` bd"
		" ON bd.`owner_id` = '%u'"
		" SET bi.`inst_color` = '%u'"
		" WHERE bi.`bot_id` = bd.`bot_id`"
		" AND bi.`slot_id` IN ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')",
		owner_id,
		rgb_value,
		EQEmu::inventory::slotHead, EQEmu::inventory::slotChest, EQEmu::inventory::slotArms, EQEmu::inventory::slotWrist1, EQEmu::inventory::slotWrist2, EQEmu::inventory::slotHands, EQEmu::inventory::slotLegs, EQEmu::inventory::slotFeet
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::SaveHelmAppearance(const uint32 owner_id, const uint32 bot_id, const bool show_flag)
{
	if (!owner_id || !bot_id)
		return false;
	
	query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = '%u'"
		" WHERE `owner_id` = '%u'"
		" AND `bot_id` = '%u'",
		(show_flag ? 1 : 0),
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::SaveAllHelmAppearances(const uint32 owner_id, const bool show_flag)
{
	if (!owner_id)
		return false;

	query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = '%u'"
		" WHERE `owner_id` = '%u'",
		(show_flag ? 1 : 0),
		owner_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::ToggleHelmAppearance(const uint32 owner_id, const uint32 bot_id)
{
	if (!owner_id || !bot_id)
		return false;

	query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = (`show_helm` XOR '1')"
		" WHERE `owner_id` = '%u'"
		" AND `bot_id` = '%u'",
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::ToggleAllHelmAppearances(const uint32 owner_id)
{
	if (!owner_id)
		return false;

	query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = (`show_helm` XOR '1')"
		" WHERE `owner_id` = '%u'",
		owner_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::SaveFollowDistance(const uint32 owner_id, const uint32 bot_id, const uint32 follow_distance)
{
	if (!owner_id || !bot_id || !follow_distance)
		return false;

	query = StringFormat(
		"UPDATE `bot_data`"
		" SET `follow_distance` = '%u'"
		" WHERE `owner_id` = '%u'"
		" AND `bot_id` = '%u'",
		follow_distance,
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::SaveAllFollowDistances(const uint32 owner_id, const uint32 follow_distance)
{
	if (!owner_id || !follow_distance)
		return false;

	query = StringFormat(
		"UPDATE `bot_data`"
		" SET `follow_distance` = '%u'"
		" WHERE `owner_id` = '%u'",
		follow_distance,
		owner_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::CreateCloneBot(const uint32 owner_id, const uint32 bot_id, const std::string& clone_name, uint32& clone_id)
{
	if (!owner_id || !bot_id || clone_name.empty())
		return false;
	
	query = StringFormat(
		"INSERT INTO `bot_data`"
		" ("
		"`owner_id`,"
		" `spells_id`,"
		" `name`,"
		" `last_name`,"
		" `title`,"
		" `suffix`,"
		" `zone_id`,"
		" `gender`,"
		" `race`,"
		" `class`,"
		" `level`,"
		" `deity`,"
		" `creation_day`,"
		" `last_spawn`,"
		" `time_spawned`,"
		" `size`,"
		" `face`,"
		" `hair_color`,"
		" `hair_style`,"
		" `beard`,"
		" `beard_color`,"
		" `eye_color_1`,"
		" `eye_color_2`,"
		" `drakkin_heritage`,"
		" `drakkin_tattoo`,"
		" `drakkin_details`,"
		" `ac`,"
		" `atk`,"
		" `hp`,"
		" `mana`,"
		" `str`,"
		" `sta`,"
		" `cha`,"
		" `dex`,"
		" `int`,"
		" `agi`,"
		" `wis`,"
		" `fire`,"
		" `cold`,"
		" `magic`,"
		" `poison`,"
		" `disease`,"
		" `corruption`,"
		" `show_helm`,"
		" `follow_distance`"
		")"
		" SELECT"
		" bd.`owner_id`,"
		" bd.`spells_id`,"
		" '%s',"
		" '',"
		" bd.`title`,"
		" bd.`suffix`,"
		" bd.`zone_id`,"
		" bd.`gender`,"
		" bd.`race`,"
		" bd.`class`,"
		" bd.`level`,"
		" bd.`deity`,"
		" UNIX_TIMESTAMP(),"
		" UNIX_TIMESTAMP(),"
		" '0',"
		" bd.`size`,"
		" bd.`face`,"
		" bd.`hair_color`,"
		" bd.`hair_style`,"
		" bd.`beard`,"
		" bd.`beard_color`,"
		" bd.`eye_color_1`,"
		" bd.`eye_color_2`,"
		" bd.`drakkin_heritage`,"
		" bd.`drakkin_tattoo`,"
		" bd.`drakkin_details`,"
		" bd.`ac`,"
		" bd.`atk`,"
		" bd.`hp`,"
		" bd.`mana`,"
		" bd.`str`,"
		" bd.`sta`,"
		" bd.`cha`,"
		" bd.`dex`,"
		" bd.`int`,"
		" bd.`agi`,"
		" bd.`wis`,"
		" bd.`fire`,"
		" bd.`cold`,"
		" bd.`magic`,"
		" bd.`poison`,"
		" bd.`disease`,"
		" bd.`corruption`,"
		" bd.`show_helm`,"
		" bd.`follow_distance`"
		" FROM `bot_data` bd"
		" WHERE"
		" bd.`owner_id` = '%u'"
		" AND"
		" bd.`bot_id` = '%u'",
		clone_name.c_str(),
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	clone_id = results.LastInsertedID();

	return true;
}

bool BotDatabase::CreateCloneBotInventory(const uint32 owner_id, const uint32 bot_id, const uint32 clone_id)
{
	if (!owner_id || !bot_id || !clone_id)
		return false;

	query = StringFormat(
		"INSERT INTO `bot_inventories`"
		" ("
		"bot_id,"
		" `slot_id`,"
		" `item_id`,"
		" `inst_charges`,"
		" `inst_color`,"
		" `inst_no_drop`,"
		" `inst_custom_data`,"
		" `ornament_icon`,"
		" `ornament_id_file`,"
		" `ornament_hero_model`,"
		" `augment_1`,"
		" `augment_2`,"
		" `augment_3`,"
		" `augment_4`,"
		" `augment_5`,"
		" `augment_6`"
		")"
		" SELECT"
		" '%u' bot_id,"
		" bi.`slot_id`,"
		" bi.`item_id`,"
		" bi.`inst_charges`,"
		" bi.`inst_color`,"
		" bi.`inst_no_drop`,"
		" bi.`inst_custom_data`,"
		" bi.`ornament_icon`,"
		" bi.`ornament_id_file`,"
		" bi.`ornament_hero_model`,"
		" bi.`augment_1`,"
		" bi.`augment_2`,"
		" bi.`augment_3`,"
		" bi.`augment_4`,"
		" bi.`augment_5`,"
		" bi.`augment_6`"
		" FROM `bot_inventories` bi"
		" WHERE"
		" bi.`bot_id` = '%u'"
		" AND"
		" '%u' = (SELECT `owner_id` FROM `bot_data` WHERE `bot_id` = '%u')",
		clone_id,
		bot_id,
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteItems(clone_id);
		return false;
	}
	
	return true;
}


/* Bot bot-group functions   */
bool BotDatabase::QueryBotGroupExistence(const std::string& group_name, bool& extant_flag)
{
	if (group_name.empty())
		return false;

	query = StringFormat("SELECT `group_name` FROM `vw_bot_groups` WHERE `group_name` LIKE '%s' LIMIT 1", group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	extant_flag = true;

	return true;
}

bool BotDatabase::LoadBotGroupIDByBotGroupName(const std::string& group_name, uint32& botgroup_id)
{
	if (group_name.empty())
		return false;

	query = StringFormat("SELECT `groups_index` FROM `bot_groups` WHERE `group_name` = '%s' LIMIT 1", group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	botgroup_id = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadBotGroupIDByLeaderID(const uint32 leader_id, uint32& botgroup_id)
{
	if (!leader_id)
		return false;

	query = StringFormat("SELECT `groups_index` FROM `bot_groups` WHERE `group_leader_id` = '%u' LIMIT 1", leader_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	botgroup_id = atoi(row[0]);
	
	return true;
}

bool BotDatabase::LoadBotGroupIDByMemberID(const uint32 member_id, uint32& botgroup_id)
{
	if (!member_id)
		return false;

	query = StringFormat("SELECT `groups_index` FROM `bot_group_members` WHERE `bot_id` = '%u' LIMIT 1", member_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	botgroup_id = atoi(row[0]);
	
	return true;
}

bool BotDatabase::LoadLeaderIDByBotGroupName(const std::string& group_name, uint32& leader_id)
{
	if (group_name.empty())
		return false;

	query = StringFormat("SELECT `group_leader_id` FROM `bot_groups` WHERE `group_name` = '%s' LIMIT 1", group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	leader_id = atoi(row[0]);
	
	return true;
}

bool BotDatabase::LoadLeaderIDByBotGroupID(const uint32 group_id, uint32& leader_id)
{
	if (!group_id)
		return false;

	query = StringFormat("SELECT `group_leader_id` FROM `bot_groups` WHERE `groups_index` = '%u' LIMIT 1", group_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	leader_id = atoi(row[0]);
	
	return true;
}

bool BotDatabase::LoadBotGroupNameByBotGroupID(const uint32 group_id, std::string& botgroup_name)
{
	if (!group_id)
		false;

	query = StringFormat("SELECT `group_name` FROM `bot_groups` WHERE `groups_index` = '%u' LIMIT 1", group_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	botgroup_name = row[0];
	
	return true;
}

bool BotDatabase::LoadBotGroupNameByLeaderID(const uint32 leader_id, std::string& botgroup_name)
{
	if (!leader_id)
		return false;

	query = StringFormat("SELECT `group_name` FROM `bot_groups` WHERE `group_leader_id` = '%u' LIMIT 1", leader_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	botgroup_name = row[0];
	
	return true;
}

bool BotDatabase::CreateBotGroup(const std::string& group_name, const uint32 leader_id)
{
	if (group_name.empty() || !leader_id)
		return false;

	bool extant_flag = false;
	if (!QueryBotGroupExistence(group_name, extant_flag))
		return false;
	if (extant_flag)
		return true;

	query = StringFormat("INSERT INTO `bot_groups` (`group_leader_id`, `group_name`) VALUES ('%u', '%s')", leader_id, group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		DeleteBotGroup(leader_id);
		return false;
	}

	auto botgroup_id = results.LastInsertedID();
	if (!botgroup_id) {
		DeleteBotGroup(leader_id);
		return false;
	}

	query = StringFormat("INSERT INTO `bot_group_members` (`groups_index`, `bot_id`) VALUES ('%u', '%u')", botgroup_id, leader_id);
	results = QueryDatabase(query);
	if (!results.Success()) {
		RemoveMemberFromBotGroup(leader_id);
		return false;
	}

	return true;
}

bool BotDatabase::DeleteBotGroup(const uint32 leader_id)
{
	if (!leader_id)
		return false;

	uint32 botgroup_id = 0;
	if (!LoadBotGroupIDByLeaderID(leader_id, botgroup_id))
		return false;
	if (!botgroup_id)
		return true;

	query = StringFormat("DELETE FROM `bot_group_members` WHERE `groups_index` = '%u'", botgroup_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	query = StringFormat("DELETE FROM `bot_groups` WHERE `groups_index` = '%u'", botgroup_id);
	results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::AddMemberToBotGroup(const uint32 leader_id, const uint32 member_id)
{
	if (!leader_id || !member_id)
		return false;

	uint32 botgroup_id = 0;
	if (!LoadBotGroupIDByLeaderID(leader_id, botgroup_id))
		return false;
	if (!botgroup_id)
		return true;

	query = StringFormat("INSERT INTO `bot_group_members` (`groups_index`, `bot_id`) VALUES ('%u', '%u')", botgroup_id, member_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		RemoveMemberFromBotGroup(member_id);
		return false;
	}

	return true;
}

bool BotDatabase::RemoveMemberFromBotGroup(const uint32 member_id)
{
	if (!member_id)
		return false;

	uint32 botgroup_id = 0;
	if (!LoadBotGroupIDByLeaderID(member_id, botgroup_id))
		return false;
	if (botgroup_id)
		return DeleteBotGroup(member_id);

	query = StringFormat("DELETE FROM `bot_group_members` WHERE `bot_id` = '%u'", member_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadBotGroupIDForLoadBotGroup(const uint32 owner_id, const std::string& group_name, uint32& botgroup_id)
{
	if (!owner_id || group_name.empty())
		return false;

	query = StringFormat("SELECT `groups_index`, `group_name` FROM `vw_bot_groups` WHERE `owner_id` = '%u'", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (!group_name.compare(row[1])) {
			botgroup_id = atoi(row[0]);
			break;
		}
	}

	return true;
}

bool BotDatabase::LoadBotGroup(const std::string& group_name, std::map<uint32, std::list<uint32>>& member_list)
{
	if (group_name.empty())
		return false;

	uint32 botgroup_id = 0;
	if (!LoadBotGroupIDByBotGroupName(group_name, botgroup_id))
		return false;
	if (!botgroup_id)
		return true;

	query = StringFormat("SELECT `bot_id` FROM `bot_group_members` WHERE `groups_index` = '%u' LIMIT 6", botgroup_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row = results.begin(); row != results.end(); ++row)
		member_list[botgroup_id].push_back(atoi(row[0]));
	
	return true;
}

bool BotDatabase::LoadBotGroupsListByOwnerID(const uint32 owner_id, std::list<std::pair<std::string, std::string>>& botgroups_list)
{
	if (!owner_id)
		return false;

	query = StringFormat("SELECT `group_name`, `group_leader_name` FROM `vw_bot_groups` WHERE `owner_id` = '%u'", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row = results.begin(); row != results.end(); ++row)
		botgroups_list.push_back(std::pair<std::string, std::string>(row[0], row[1]));

	return true;
}


/* Bot owner group functions   */
// added owner ID to this function to fix groups with mulitple players grouped with bots.
bool BotDatabase::LoadGroupedBotsByGroupID(const uint32 owner_id, const uint32 group_id, std::list<uint32>& group_list)
{
	if (!group_id || !owner_id)
		return false;

	query = StringFormat(
		"SELECT `charid`"
		" FROM `group_id`"
		" WHERE `groupid` = '%u'"
		" AND `charid` IN ("
		"  SELECT `bot_id`"
		"  FROM `bot_data`"
		"  WHERE `owner_id` = '%u'"
		"  )",
		group_id,
		owner_id
	);

	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row = results.begin(); row != results.end(); ++row)
		group_list.push_back(atoi(row[0]));

	return true;
}


/* Bot heal rotation functions   */
bool BotDatabase::LoadHealRotationIDByBotID(const uint32 bot_id, uint32& hr_index)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `heal_rotation_index` FROM `bot_heal_rotations` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	hr_index = atoi(row[0]);

	return true;
}

bool BotDatabase::LoadHealRotation(Bot* hr_member, std::list<uint32>& member_list, std::list<std::string>& target_list, bool& load_flag, bool& member_fail, bool& target_fail)
{
	if (!hr_member)
		return false;

	uint32 hr_index = 0;
	if (!LoadHealRotationIDByBotID(hr_member->GetBotID(), hr_index))
		return false;
	if (!hr_index)
		return true;

	if (!hr_member->IsHealRotationMember())
		return false;

	query = StringFormat(
		"SELECT "
		" `interval`,"
		" `fast_heals`,"
		" `adaptive_targeting`,"
		" `casting_override`,"
		" `safe_hp_base`,"
		" `safe_hp_cloth`,"
		" `safe_hp_leather`,"
		" `safe_hp_chain`,"
		" `safe_hp_plate`,"
		" `critical_hp_base`,"
		" `critical_hp_cloth`,"
		" `critical_hp_leather`,"
		" `critical_hp_chain`,"
		" `critical_hp_plate`"
		" FROM `bot_heal_rotations`"
		" WHERE `heal_rotation_index` = '%u'"
		" LIMIT 1",
		hr_index
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	(*hr_member->MemberOfHealRotation())->SetIntervalS((uint32)atoi(row[0]));
	(*hr_member->MemberOfHealRotation())->SetFastHeals((bool)atoi(row[1]));
	(*hr_member->MemberOfHealRotation())->SetAdaptiveTargeting((bool)atoi(row[2]));
	(*hr_member->MemberOfHealRotation())->SetCastingOverride((bool)atoi(row[3]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_UNKNOWN, atof(row[4]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_CLOTH, atof(row[5]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_LEATHER, atof(row[6]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_CHAIN, atof(row[7]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_PLATE, atof(row[8]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_UNKNOWN, atof(row[9]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_CLOTH, atof(row[10]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_LEATHER, atof(row[11]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_CHAIN, atof(row[12]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_PLATE, atof(row[13]));

	load_flag = true;

	if (!LoadHealRotationMembers(hr_index, member_list))
		member_fail = true;

	if (!LoadHealRotationTargets(hr_index, target_list))
		target_fail = true;

	return true;
}

bool BotDatabase::LoadHealRotationMembers(const uint32 hr_index, std::list<uint32>& member_list)
{
	if (!hr_index)
		return false;

	query = StringFormat("SELECT `bot_id` FROM `bot_heal_rotation_members` WHERE `heal_rotation_index` = '%u'", hr_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row : results) {
		if (row[0])
			member_list.push_back(atoi(row[0]));
	}

	return true;
}

bool BotDatabase::LoadHealRotationTargets(const uint32 hr_index, std::list<std::string>& target_list)
{
	if (!hr_index)
		return false;

	query = StringFormat("SELECT `target_name` FROM `bot_heal_rotation_targets` WHERE `heal_rotation_index` = '%u'", hr_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row : results) {
		if (row[0])
			target_list.push_back(row[0]);
	}

	return true;
}

bool BotDatabase::SaveHealRotation(Bot* hr_member, bool& member_fail, bool& target_fail)
{
	if (!hr_member)
		return false;

	if (!DeleteHealRotation(hr_member->GetBotID()))
		return false;

	if (!hr_member->IsHealRotationMember())
		return false;

	query = StringFormat(
		"INSERT INTO `bot_heal_rotations` ("
		"`bot_id`,"
		" `interval`,"
		" `fast_heals`,"
		" `adaptive_targeting`,"
		" `casting_override`,"
		" `safe_hp_base`,"
		" `safe_hp_cloth`,"
		" `safe_hp_leather`,"
		" `safe_hp_chain`,"
		" `safe_hp_plate`,"
		" `critical_hp_base`,"
		" `critical_hp_cloth`,"
		" `critical_hp_leather`,"
		" `critical_hp_chain`,"
		" `critical_hp_plate`"
		")"
		" VALUES ("
		"'%u',"
		" '%u',"
		" '%u',"
		" '%u',"
		" '%u',"
		" '%f',"
		" '%f',"
		" '%f',"
		" '%f',"
		" '%f',"
		" '%f',"
		" '%f',"
		" '%f',"
		" '%f',"
		" '%f'"
		")",
		hr_member->GetBotID(),
		((*hr_member->MemberOfHealRotation())->IntervalS()),
		((*hr_member->MemberOfHealRotation())->FastHeals()),
		((*hr_member->MemberOfHealRotation())->AdaptiveTargeting()),
		((*hr_member->MemberOfHealRotation())->CastingOverride()),
		((*hr_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_UNKNOWN)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_CLOTH)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_LEATHER)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_CHAIN)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_PLATE)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_UNKNOWN)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CLOTH)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_LEATHER)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CHAIN)),
		((*hr_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_PLATE))
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	uint32 hr_index = results.LastInsertedID();
	if (!hr_index)
		return false;

	std::list<Bot*>* member_list = (*hr_member->MemberOfHealRotation())->MemberList();

	for (auto member_iter : *member_list) {
		if (!member_iter)
			continue;

		query = StringFormat("INSERT INTO `bot_heal_rotation_members` (`heal_rotation_index`, `bot_id`) VALUES ('%u', '%u')", hr_index, member_iter->GetBotID());
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			member_fail = true;
			break;
		}
	}

	std::list<Mob*>* target_list = (*hr_member->MemberOfHealRotation())->TargetList();

	for (auto target_iter : *target_list) {
		if (!target_iter)
			continue;

		query = StringFormat("INSERT INTO `bot_heal_rotation_targets` (`heal_rotation_index`, `target_name`) VALUES ('%u', '%s')", hr_index, target_iter->GetCleanName());
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			target_fail = true;
			break;
		}
	}

	return true;
}

bool BotDatabase::DeleteHealRotation(const uint32 creator_id)
{
	if (!creator_id)
		return false;

	uint32 hr_index = 0;
	if (!LoadHealRotationIDByBotID(creator_id, hr_index))
		return false;
	if (!hr_index)
		return true;

	query = StringFormat("DELETE FROM `bot_heal_rotation_targets` WHERE `heal_rotation_index` = '%u'", hr_index);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	query = StringFormat("DELETE FROM `bot_heal_rotation_members` WHERE `heal_rotation_index` = '%u'", hr_index);
	results = QueryDatabase(query);
	if (!results.Success())
		return false;

	query = StringFormat("DELETE FROM `bot_heal_rotations` WHERE `heal_rotation_index` = '%u'", hr_index);
	results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::DeleteAllHealRotations(const uint32 owner_id)
{
	if (!owner_id)
		return false;

	query = StringFormat("SELECT `bot_id` FROM `bot_heal_rotations` WHERE `bot_id` IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = '%u')", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row : results) {
		if (!row[0])
			continue;

		DeleteHealRotation(atoi(row[0]));
	}

	return true;
}


/* Bot miscellaneous functions   */
uint8 BotDatabase::GetSpellCastingChance(uint8 spell_type_index, uint8 class_index, uint8 stance_index, uint8 conditional_index) // class_index is 0-based
{
	if (spell_type_index >= MaxSpellTypes)
		return 0;
	if (class_index >= PLAYER_CLASS_COUNT)
		return 0;
	if (stance_index >= MaxStances)
		return 0;
	if (conditional_index >= cntHSND)
		return 0;

	return spell_casting_chances[spell_type_index][class_index][stance_index][conditional_index];
}


/* fail::Bot functions   */
const char* BotDatabase::fail::QueryNameAvailablity() { return "Failed to query name availability"; }
const char* BotDatabase::fail::QueryBotCount() { return "Failed to query bot count"; }
const char* BotDatabase::fail::LoadQuestableSpawnCount() { return "Failed to load questable spawn count"; }
const char* BotDatabase::fail::LoadBotsList() { return "Failed to bots list"; }
const char* BotDatabase::fail::LoadOwnerID() { return "Failed to load owner id"; }
const char* BotDatabase::fail::LoadBotID() { return "Failed to load bot id"; }
const char* BotDatabase::fail::LoadBot() { return "Failed to load bot"; }
const char* BotDatabase::fail::SaveNewBot() { return "Failed to save new bot"; }
const char* BotDatabase::fail::SaveBot() { return "Failed to save bot"; }
const char* BotDatabase::fail::DeleteBot() { return "Failed to delete bot"; }
const char* BotDatabase::fail::LoadBuffs() { return "Failed to load buffs"; }
const char* BotDatabase::fail::SaveBuffs() { return "Failed to save buffs"; }
const char* BotDatabase::fail::DeleteBuffs() { return "Failed to delete buffs"; }
const char* BotDatabase::fail::LoadStance() { return "Failed to load stance"; }
const char* BotDatabase::fail::SaveStance() { return "Failed to save stance"; }
const char* BotDatabase::fail::DeleteStance() { return "Failed to delete stance"; }
const char* BotDatabase::fail::LoadTimers() { return "Failed to load timers"; }
const char* BotDatabase::fail::SaveTimers() { return "Failed to save timers"; }
const char* BotDatabase::fail::DeleteTimers() { return "Failed to delete timers"; }
const char* BotDatabase::fail::LoadGuildMembership() { return "Failed to load guild membership"; }
const char* BotDatabase::fail::SaveGuildMembership() { return "Failed to save guild membership"; }
const char* BotDatabase::fail::DeleteGuildMembership() { return "Failed to delete guild membership"; }

/* fail::Bot inventory functions   */
const char* BotDatabase::fail::QueryInventoryCount() { return "Failed to query inventory count"; }
const char* BotDatabase::fail::LoadItems() { return "Failed to load items"; }
const char* BotDatabase::fail::SaveItems() { return "Failed to save items"; }
const char* BotDatabase::fail::DeleteItems() { return "Failed to delete items"; }
const char* BotDatabase::fail::LoadItemBySlot() { return "Failed to load item by slot"; }
const char* BotDatabase::fail::SaveItemBySlot() { return "Failed to save item by slot"; }
const char* BotDatabase::fail::DeleteItemBySlot() { return "Failed to delete item by slot"; }
const char* BotDatabase::fail::LoadEquipmentColor() { return "Failed to load equipment color"; }
const char* BotDatabase::fail::SaveEquipmentColor() { return "Failed to save equipment color"; }

/* fail::Bot pet functions   */
const char* BotDatabase::fail::LoadPetIndex() { return "Failed to load pet index"; }
const char* BotDatabase::fail::LoadPetSpellID() { return "Failed to load pet spell id"; }
const char* BotDatabase::fail::LoadPetStats() { return "Failed to load pet stats"; }
const char* BotDatabase::fail::SavePetStats() { return "Failed to save pet stats"; }
const char* BotDatabase::fail::DeletePetStats() { return "Failed to delete pet stats"; }
const char* BotDatabase::fail::LoadPetBuffs() { return "Failed to load pet buffs"; }
const char* BotDatabase::fail::SavePetBuffs() { return "Failed to save pet buffs"; }
const char* BotDatabase::fail::DeletePetBuffs() { return "Failed to delete pet buffs"; }
const char* BotDatabase::fail::LoadPetItems() { return "Failed to load pet items"; }
const char* BotDatabase::fail::SavePetItems() { return "Failed to save pet items"; }
const char* BotDatabase::fail::DeletePetItems() { return "Failed to delete pet items"; }

/* fail::Bot command functions   */
const char* BotDatabase::fail::LoadInspectMessage() { return "Failed to load inspect message"; }
const char* BotDatabase::fail::SaveInspectMessage() { return "Failed to save inspect message"; }
const char* BotDatabase::fail::DeleteInspectMessage() { return "Failed to delete inspect message"; }
const char* BotDatabase::fail::SaveAllInspectMessages() { return "Failed to save all inspect messages"; }
const char* BotDatabase::fail::DeleteAllInspectMessages() { return "Failed to delete all inspect messages"; }
const char* BotDatabase::fail::SaveAllArmorColorBySlot() { return "Failed to save all armor color by slot"; }
const char* BotDatabase::fail::SaveAllArmorColors() { return "Failed to save all armor colors"; }
const char* BotDatabase::fail::SaveHelmAppearance() { return "Failed to save helm appearance"; }
const char* BotDatabase::fail::SaveAllHelmAppearances() { return "Failed to save all helm appearances"; }
const char* BotDatabase::fail::ToggleHelmAppearance() { return "Failed to save toggle helm appearance"; }
const char* BotDatabase::fail::ToggleAllHelmAppearances() { return "Failed to save toggle all helm appearance"; }
const char* BotDatabase::fail::SaveFollowDistance() { return "Failed to save follow distance"; }
const char* BotDatabase::fail::SaveAllFollowDistances() { return "Failed to save all follow distances"; }
const char* BotDatabase::fail::CreateCloneBot() { return "Failed to create clone bot"; }
const char* BotDatabase::fail::CreateCloneBotInventory() { return "Failed to create clone bot inventory"; }

/* fail::Bot bot-group functions   */
const char* BotDatabase::fail::QueryBotGroupExistence() { return "Failed to query bot-group existence"; }
const char* BotDatabase::fail::LoadBotGroupIDByBotGroupName() { return "Failed to load bot-group id by bot-group name"; }
const char* BotDatabase::fail::LoadBotGroupIDByLeaderID() { return "Failed to load bot-group id by leader id"; }
const char* BotDatabase::fail::LoadBotGroupIDByMemberID() { return "Failed to load bot-group id by member id"; }
const char* BotDatabase::fail::LoadLeaderIDByBotGroupName() { return "Failed to load leader id by bot-group name"; }
const char* BotDatabase::fail::LoadLeaderIDByBotGroupID() { return "Failed to load leader id by bot-group id"; }
const char* BotDatabase::fail::LoadBotGroupNameByBotGroupID() { return "Failed to load bot-group name by bot-group id"; }
const char* BotDatabase::fail::LoadBotGroupNameByLeaderID() { return "Failed to load bot-group name by leader id"; }
const char* BotDatabase::fail::CreateBotGroup() { return "Failed to create bot-group"; }
const char* BotDatabase::fail::DeleteBotGroup() { return "Failed to delete bot-group"; }
const char* BotDatabase::fail::AddMemberToBotGroup() { return "Failed to add member to bot-group"; }
const char* BotDatabase::fail::RemoveMemberFromBotGroup() { return "Failed to remove member from bot-group"; }
const char* BotDatabase::fail::LoadBotGroupIDForLoadBotGroup() { return "Failed to load bot-group id for load bot-group"; }
const char* BotDatabase::fail::LoadBotGroup() { return "Failed to load bot-group"; }
const char* BotDatabase::fail::LoadBotGroupsListByOwnerID() { return "Failed to load bot-groups list by owner id"; }

/* fail::Bot group functions   */
const char* BotDatabase::fail::LoadGroupedBotsByGroupID() { return "Failed to load grouped bots by group id"; }

/* fail::Bot heal rotation functions   */
const char* BotDatabase::fail::LoadHealRotationIDByBotID() { return "Failed to load heal rotation id by bot id"; }
const char* BotDatabase::fail::LoadHealRotation() { return "Failed to load heal rotation"; }
const char* BotDatabase::fail::LoadHealRotationMembers() { return "Failed to load heal rotation members"; }
const char* BotDatabase::fail::LoadHealRotationTargets() { return "Failed to load heal rotation targets"; }
const char* BotDatabase::fail::SaveHealRotation() { return "Failed to save heal rotation"; }
const char* BotDatabase::fail::DeleteHealRotation() { return "Failed to delete heal rotation"; }
const char* BotDatabase::fail::DeleteAllHealRotations() { return "Failed to delete all heal rotations"; }

/* fail::Bot miscellaneous functions   */

#endif // BOTS
