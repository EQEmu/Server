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

#include "../common/data_verification.h"
#include "../common/global_define.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include "../common/eqemu_logsys.h"

#include "../common/repositories/bot_data_repository.h"
#include "../common/repositories/bot_inventories_repository.h"

#include "zonedb.h"
#include "bot.h"
#include "client.h"

#include <fmt/format.h>


bool BotDatabase::LoadBotCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &bot_command_settings)
{
	bot_command_settings.clear();

	query = "SELECT `bot_command`, `access`, `aliases` FROM `bot_command_settings`";
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		bot_command_settings[row[0]].first = Strings::ToInt(row[1]);
		if (row[2][0] == 0)
			continue;

		auto aliases = Strings::Split(row[2], '|');
		for (auto iter : aliases) {
			if (!iter.empty())
				bot_command_settings[row[0]].second.push_back(iter);
		}
	}

	return true;
}

bool BotDatabase::UpdateInjectedBotCommandSettings(const std::vector<std::pair<std::string, uint8>> &injected)
{
	if (injected.size()) {

		query = fmt::format(
			"REPLACE INTO `bot_command_settings`(`bot_command`, `access`) VALUES {}",
			Strings::ImplodePair(
				",",
				std::pair<char, char>('(', ')'),
				join_pair(",", std::pair<char, char>('\'', '\''), injected)
			)
		);

		if (!database.QueryDatabase(query).Success()) {
			return false;
		}

		LogInfo(
			"[{}] New Bot Command{} Added",
			injected.size(),
			(injected.size() == 1 ? "" : "s")
		);
	}

	return true;
}

bool BotDatabase::UpdateOrphanedBotCommandSettings(const std::vector<std::string> &orphaned)
{
	if (orphaned.size()) {

		query = fmt::format(
			"DELETE FROM `bot_command_settings` WHERE `bot_command` IN ({})",
			Strings::ImplodePair(",", std::pair<char, char>('\'', '\''), orphaned)
		);

		if (!database.QueryDatabase(query).Success()) {
			return false;
		}

		LogInfo(
			"[{}] Orphaned Bot Command{} Deleted",
			orphaned.size(),
			(orphaned.size() == 1 ? "" : "s")
		);
	}

	return true;
}

bool BotDatabase::LoadBotSpellCastingChances()
{
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

	auto results = database.QueryDatabase(query);
	if (!results.Success() || !results.RowCount())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		uint8 spell_type_index = Strings::ToInt(row[0]);
		if (spell_type_index >= Bot::SPELL_TYPE_COUNT)
			continue;
		uint8 class_index = Strings::ToInt(row[1]);
		if (class_index < WARRIOR || class_index > BERSERKER)
			continue;
		--class_index;
		uint8 stance_index = Strings::ToInt(row[2]);
		if (stance_index >= EQ::constants::STANCE_TYPE_COUNT)
			continue;

		for (uint8 conditional_index = nHSND; conditional_index < cntHSND; ++conditional_index) {
			uint8 value = Strings::ToInt(row[3 + conditional_index]);
			if (!value)
				continue;
			if (value > 100)
				value = 100;

			Bot::spell_casting_chances[spell_type_index][class_index][stance_index][conditional_index] = value;
		}
	}

	return true;
}


/* Bot functions   */
bool BotDatabase::QueryNameAvailablity(const std::string& bot_name, bool& available_flag)
{
	if (bot_name.empty() || bot_name.size() > 60 || !database.CheckUsedName(bot_name)) {
		return false;
	}

	query = fmt::format(
		"SELECT b.bot_id FROM bot_data b "
		"WHERE b.`name` LIKE '{}' "
		"LIMIT 1",
		bot_name
	);

	auto results = database.QueryDatabase(query);

	if (!results.RowCount()) {
		query = fmt::format(
			"SELECT c.id FROM character_data c "
			"WHERE c.`name` LIKE '{}' "
			"LIMIT 1",
			bot_name
		);
		results = database.QueryDatabase(query);
	}

	if (!results.Success()) {
		return false;
	}

	if (results.RowCount()) {
		return true;
	}

	available_flag = true;

	return true;
}

bool BotDatabase::QueryBotCount(const uint32 owner_id, int class_id, uint32& bot_count, uint32& bot_class_count)
{
	if (!owner_id) {
		return false;
	}

	query = fmt::format(
		"SELECT COUNT(`bot_id`) FROM `bot_data` WHERE `owner_id` = {}",
		owner_id
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (!results.RowCount()) {
		return true;
	}

	auto row = results.begin();
	bot_count = Strings::ToUnsignedInt(row[0]);

	if (EQ::ValueWithin(class_id, WARRIOR, BERSERKER)) {
		query = fmt::format(
			"SELECT COUNT(`bot_id`) FROM `bot_data` WHERE `owner_id` = {} AND `class` = {}",
			owner_id,
			class_id
		);
		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		if (!results.RowCount()) {
			return true;
		}

		auto row = results.begin();
		bot_class_count = Strings::ToUnsignedInt(row[0]);
	}

	return true;
}

bool BotDatabase::LoadBotsList(const uint32 owner_id, std::list<BotsAvailableList>& bots_list, bool ByAccount)
{
	if (!owner_id)
		return false;

	if (ByAccount == true)
		 query = StringFormat("SELECT bot_id, bd.`name`, bd.class, bd.`level`, bd.race, bd.gender, cd.`name` as owner, bd.owner_id, cd.account_id, cd.id"
			 " FROM bot_data as bd inner join character_data as cd on bd.owner_id = cd.id"
			 " WHERE cd.account_id = (select account_id from bot_data bd inner join character_data as cd on bd.owner_id = cd.id where bd.owner_id = '%u' LIMIT 1)"
			 " ORDER BY bd.owner_id", owner_id);
	else
		 query = StringFormat("SELECT `bot_id`, `name`, `class`, `level`, `race`, `gender`, 'You' as owner, owner_id FROM `bot_data` WHERE `owner_id` = '%u'", owner_id);

	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row = results.begin(); row != results.end(); ++row) {
		BotsAvailableList bot_entry;

		bot_entry.ID = Strings::ToInt(row[0]);

		memset(&bot_entry.Name, 0, sizeof(bot_entry.Name));
		std::string bot_name = row[1];
		if (bot_name.size() > 63)
			bot_name = bot_name.substr(0, 63);
		if (!bot_name.empty())
			strcpy(bot_entry.Name, bot_name.c_str());
		memset(&bot_entry.Owner, 0, sizeof(bot_entry.Owner));
		std::string bot_owner = row[6];
		if (bot_owner.size() > 63)
			 bot_owner = bot_owner.substr(0, 63);
		if (!bot_owner.empty())
			 strcpy(bot_entry.Owner, bot_owner.c_str());
		bot_entry.Class = Strings::ToInt(row[2]);
		bot_entry.Level = Strings::ToInt(row[3]);
		bot_entry.Race = Strings::ToInt(row[4]);
		bot_entry.Gender = Strings::ToInt(row[5]);
		bot_entry.Owner_ID = Strings::ToInt(row[7]);
		bots_list.push_back(bot_entry);
	}

	return true;
}

bool BotDatabase::LoadOwnerID(const std::string& bot_name, uint32& owner_id)
{
	if (bot_name.empty())
		return false;

	query = StringFormat("SELECT `owner_id` FROM `bot_data` WHERE `name` = '%s' LIMIT 1", bot_name.c_str());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	owner_id = Strings::ToInt(row[0]);

	return true;
}

bool BotDatabase::LoadOwnerID(const uint32 bot_id, uint32& owner_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `owner_id` FROM `bot_data` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	owner_id = Strings::ToInt(row[0]);

	return true;
}

uint32 BotDatabase::GetOwnerID(const uint32 bot_id)
{
	if (!bot_id) {
		return 0;
	}

	const auto& l = BotDataRepository::FindOne(database, bot_id);
	if (!l.bot_id) {
		return 0;
	}

	return l.owner_id;
}

bool BotDatabase::LoadBotID(const uint32 owner_id, const std::string& bot_name, uint32& bot_id)
{
	if (!owner_id || bot_name.empty()) {
		return false;
	}

	query = fmt::format(
		"SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = {} AND `name` = '{}' LIMIT 1",
		owner_id,
		bot_name
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (!results.RowCount()) {
		return true;
	}

	auto row = results.begin();
	bot_id = Strings::ToUnsignedInt(row[0]);

	return true;
}

bool BotDatabase::LoadBotID(const uint32 owner_id, const std::string& bot_name, uint32& bot_id, uint8& bot_class_id)
{
	if (!owner_id || bot_name.empty()) {
		return false;
	}

	query = fmt::format(
		"SELECT `bot_id`, `class` FROM `bot_data` WHERE `owner_id` = {} AND `name` = '{}' LIMIT 1",
		owner_id,
		bot_name
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (!results.RowCount()) {
		return true;
	}

	auto row = results.begin();
	bot_id = Strings::ToUnsignedInt(row[0]);
	bot_class_id = static_cast<uint8>(Strings::ToUnsignedInt(row[1]));

	return true;
}

bool BotDatabase::LoadBot(const uint32 bot_id, Bot*& loaded_bot)
{
	if (!bot_id || loaded_bot) {
		return false;
	}

	const auto& l = BotDataRepository::FindOne(database, bot_id);
	if (!l.bot_id) {
		return false;
	}

	auto d = Bot::CreateDefaultNPCTypeStructForBot(
		l.name,
		l.last_name,
		l.level,
		l.race,
		l.class_,
		l.gender
	);

	auto t = Bot::FillNPCTypeStruct(
		l.spells_id,
		l.name,
		l.last_name,
		l.level,
		l.race,
		l.class_,
		l.gender,
		l.size,
		l.face,
		l.hair_style,
		l.hair_color,
		l.eye_color_1,
		l.eye_color_2,
		l.beard,
		l.beard_color,
		l.drakkin_heritage,
		l.drakkin_tattoo,
		l.drakkin_details,
		l.hp,
		l.mana,
		d->MR,
		d->CR,
		d->DR,
		d->FR,
		d->PR,
		d->Corrup,
		d->AC,
		d->STR,
		d->STA,
		d->DEX,
		d->AGI,
		d->INT,
		d->WIS,
		d->CHA,
		d->ATK
	);

	safe_delete(d);

	loaded_bot = new Bot(
		bot_id,
		l.owner_id,
		l.spells_id,
		l.time_spawned,
		l.zone_id,
		t
	);

	if (loaded_bot) {
		loaded_bot->SetSurname(l.last_name);
		loaded_bot->SetTitle(l.title);
		loaded_bot->SetSuffix(l.suffix);

		loaded_bot->SetShowHelm((l.show_helm ? true : false));

		auto bfd = EQ::Clamp(l.follow_distance, static_cast<uint32>(1), BOT_FOLLOW_DISTANCE_DEFAULT_MAX);
		loaded_bot->SetFollowDistance(bfd);

		loaded_bot->SetStopMeleeLevel(l.stop_melee_level);

		loaded_bot->SetExpansionBitmask(l.expansion_bitmask, false);

		loaded_bot->SetBotEnforceSpellSetting((l.enforce_spell_settings ? true : false));

		loaded_bot->SetBotArcherySetting((l.archery_setting ? true : false));

		loaded_bot->SetBotCasterRange(l.caster_range);
	}

	return true;
}

bool BotDatabase::SaveNewBot(Bot* bot_inst, uint32& bot_id)
{
	if (!bot_inst) {
		return false;
	}

	auto e = BotDataRepository::NewEntity();

	e.owner_id               = bot_inst->GetBotOwnerCharacterID();
	e.spells_id              = bot_inst->GetBotSpellID();
	e.name                   = bot_inst->GetCleanName();
	e.last_name              = bot_inst->GetLastName();
	e.title                  = bot_inst->GetTitle();
	e.suffix                 = bot_inst->GetSuffix();
	e.zone_id                = bot_inst->GetLastZoneID();
	e.gender                 = bot_inst->GetGender();
	e.race                   = bot_inst->GetBaseRace();
	e.class_                 = bot_inst->GetClass();
	e.level                  = bot_inst->GetLevel();
	e.creation_day           = std::time(nullptr);
	e.last_spawn             = std::time(nullptr);
	e.size                   = bot_inst->GetSize();
	e.face                   = bot_inst->GetLuclinFace();
	e.hair_color             = bot_inst->GetHairColor();
	e.hair_style             = bot_inst->GetHairStyle();
	e.beard                  = bot_inst->GetBeard();
	e.beard_color            = bot_inst->GetBeardColor();
	e.eye_color_1            = bot_inst->GetEyeColor1();
	e.eye_color_2            = bot_inst->GetEyeColor2();
	e.drakkin_heritage       = bot_inst->GetDrakkinHeritage();
	e.drakkin_tattoo         = bot_inst->GetDrakkinTattoo();
	e.drakkin_details        = bot_inst->GetDrakkinDetails();
	e.ac                     = bot_inst->GetBaseAC();
	e.atk                    = bot_inst->GetBaseATK();
	e.hp                     = bot_inst->GetHP();
	e.mana                   = bot_inst->GetMana();
	e.str                    = bot_inst->GetBaseSTR();
	e.sta                    = bot_inst->GetBaseSTA();
	e.cha                    = bot_inst->GetBaseCHA();
	e.dex                    = bot_inst->GetBaseDEX();
	e.int_                   = bot_inst->GetBaseINT();
	e.agi                    = bot_inst->GetBaseAGI();
	e.wis                    = bot_inst->GetBaseWIS();
	e.fire                   = bot_inst->GetBaseFR();
	e.cold                   = bot_inst->GetBaseCR();
	e.magic                  = bot_inst->GetBaseMR();
	e.poison                 = bot_inst->GetBasePR();
	e.disease                = bot_inst->GetBaseDR();
	e.corruption             = bot_inst->GetBaseCorrup();
	e.show_helm              = bot_inst->GetShowHelm() ? 1 : 0;
	e.follow_distance        = bot_inst->GetFollowDistance();
	e.stop_melee_level       = bot_inst->GetStopMeleeLevel();
	e.expansion_bitmask      = bot_inst->GetExpansionBitmask();
	e.enforce_spell_settings = bot_inst->GetBotEnforceSpellSetting();
	e.archery_setting        = bot_inst->IsBotArcher() ? 1 : 0;
	e.caster_range           = bot_inst->GetBotCasterRange();

	auto b = BotDataRepository::InsertOne(database, e);
	if (!b.bot_id) {
		return false;
	}

	bot_id = b.bot_id;

	return true;
}

bool BotDatabase::SaveBot(Bot* bot_inst)
{
	if (!bot_inst) {
		return false;
	}

	auto l = BotDataRepository::FindOne(database, bot_inst->GetBotID());
	if (!l.bot_id) {
		return false;
	}

	l.owner_id               = bot_inst->GetBotOwnerCharacterID();
	l.spells_id              = bot_inst->GetBotSpellID();
	l.name                   = bot_inst->GetCleanName();
	l.last_name              = bot_inst->GetLastName();
	l.title                  = bot_inst->GetTitle();
	l.suffix                 = bot_inst->GetSuffix();
	l.zone_id                = bot_inst->GetLastZoneID();
	l.gender                 = bot_inst->GetBaseGender();
	l.race                   = bot_inst->GetBaseRace();
	l.class_                 = bot_inst->GetClass();
	l.level                  = bot_inst->GetLevel();
	l.last_spawn             = std::time(nullptr);
	l.time_spawned           = bot_inst->GetTotalPlayTime();
	l.size                   = bot_inst->GetSize();
	l.face                   = bot_inst->GetLuclinFace();
	l.hair_color             = bot_inst->GetHairColor();
	l.hair_style             = bot_inst->GetHairStyle();
	l.beard                  = bot_inst->GetBeard();
	l.beard_color            = bot_inst->GetBeardColor();
	l.eye_color_1            = bot_inst->GetEyeColor1();
	l.eye_color_2            = bot_inst->GetEyeColor2();
	l.drakkin_heritage       = bot_inst->GetDrakkinHeritage();
	l.drakkin_tattoo         = bot_inst->GetDrakkinTattoo();
	l.drakkin_details        = bot_inst->GetDrakkinDetails();
	l.ac                     = bot_inst->GetBaseAC();
	l.atk                    = bot_inst->GetBaseATK();
	l.hp                     = bot_inst->GetHP();
	l.mana                   = bot_inst->GetMana();
	l.str                    = bot_inst->GetBaseSTR();
	l.sta                    = bot_inst->GetBaseSTA();
	l.cha                    = bot_inst->GetBaseCHA();
	l.dex                    = bot_inst->GetBaseDEX();
	l.int_                   = bot_inst->GetBaseINT();
	l.agi                    = bot_inst->GetBaseAGI();
	l.wis                    = bot_inst->GetBaseWIS();
	l.fire                   = bot_inst->GetBaseFR();
	l.cold                   = bot_inst->GetBaseCR();
	l.magic                  = bot_inst->GetBaseMR();
	l.poison                 = bot_inst->GetBasePR();
	l.disease                = bot_inst->GetBaseDR();
	l.corruption             = bot_inst->GetBaseCorrup();
	l.show_helm              = bot_inst->GetShowHelm() ? 1 : 0;
	l.follow_distance        = bot_inst->GetFollowDistance();
	l.stop_melee_level       = bot_inst->GetStopMeleeLevel();
	l.expansion_bitmask      = bot_inst->GetExpansionBitmask();
	l.enforce_spell_settings = bot_inst->GetBotEnforceSpellSetting();
	l.archery_setting        = bot_inst->IsBotArcher() ? 1 : 0;

	const auto updated = BotDataRepository::UpdateOne(database, l);
	if (!updated) {
		return false;
	}

	return true;
}

bool BotDatabase::DeleteBot(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	const auto deleted = BotDataRepository::DeleteOne(database, bot_id);
	if (!deleted) {
		return false;
	}

	return true;
}

bool BotDatabase::LoadBuffs(Bot* bot_inst)
{
	if (!bot_inst) {
		return false;
	}

	query = fmt::format(
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
		" WHERE `bot_id` = {}",
		bot_inst->GetBotID()
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	if (!results.RowCount()) {
		return true;
	}

	Buffs_Struct* bot_buffs = bot_inst->GetBuffs();

	if (!bot_buffs) {
		return false;
	}

	uint32 max_slots = bot_inst->GetMaxBuffSlots();
	for (int index = 0; index < max_slots; index++) {
		bot_buffs[index].spellid = SPELL_UNKNOWN;
	}

	int buff_count = 0;
	for (auto row = results.begin(); row != results.end() && buff_count < BUFF_COUNT; ++row) {
		bot_buffs[buff_count].spellid = Strings::ToUnsignedInt(row[0]);
		bot_buffs[buff_count].casterlevel = Strings::ToUnsignedInt(row[1]);
		//row[2] (duration_formula) can probably be removed
		bot_buffs[buff_count].ticsremaining = Strings::ToInt(row[3]);

		bot_buffs[buff_count].counters = 0;
		if (CalculatePoisonCounters(bot_buffs[buff_count].spellid) > 0) {
			bot_buffs[buff_count].counters = Strings::ToUnsignedInt(row[4]);
		} else if (CalculateDiseaseCounters(bot_buffs[buff_count].spellid) > 0) {
			bot_buffs[buff_count].counters = Strings::ToUnsignedInt(row[5]);
		} else if (CalculateCurseCounters(bot_buffs[buff_count].spellid) > 0) {
			bot_buffs[buff_count].counters = Strings::ToUnsignedInt(row[6]);
		} else if (CalculateCorruptionCounters(bot_buffs[buff_count].spellid) > 0) {
			bot_buffs[buff_count].counters = Strings::ToUnsignedInt(row[7]);
		}

		bot_buffs[buff_count].hit_number = Strings::ToUnsignedInt(row[8]);
		bot_buffs[buff_count].melee_rune = Strings::ToUnsignedInt(row[9]);
		bot_buffs[buff_count].magic_rune = Strings::ToUnsignedInt(row[10]);
		bot_buffs[buff_count].dot_rune = Strings::ToUnsignedInt(row[11]);
		bot_buffs[buff_count].persistant_buff = (Strings::ToBool(row[12])) != 0;
		bot_buffs[buff_count].caston_x = Strings::ToInt(row[13]);
		bot_buffs[buff_count].caston_y = Strings::ToInt(row[14]);
		bot_buffs[buff_count].caston_z = Strings::ToInt(row[15]);
		bot_buffs[buff_count].ExtraDIChance = Strings::ToInt(row[16]);
		bot_buffs[buff_count].instrument_mod = Strings::ToUnsignedInt(row[17]);
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
		if (!IsValidSpell(bot_buffs[buff_index].spellid)) {
			continue;
		}

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
			spells[bot_buffs[buff_index].spellid].buff_duration_formula,
			bot_buffs[buff_index].ticsremaining,
			((CalculatePoisonCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			((CalculateDiseaseCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			((CalculateCurseCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			((CalculateCorruptionCounters(bot_buffs[buff_index].spellid) > 0) ? (bot_buffs[buff_index].counters) : (0)),
			bot_buffs[buff_index].hit_number,
			bot_buffs[buff_index].melee_rune,
			bot_buffs[buff_index].magic_rune,
			bot_buffs[buff_index].dot_rune,
			((bot_buffs[buff_index].persistant_buff) ? (1) : (0)),
			bot_buffs[buff_index].caston_x,
			bot_buffs[buff_index].caston_y,
			bot_buffs[buff_index].caston_z,
			bot_buffs[buff_index].ExtraDIChance
		);
		auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadStance(const uint32 bot_id, int& bot_stance)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `stance_id` FROM `bot_stances` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	bot_stance = Strings::ToInt(row[0]);

	return true;
}

bool BotDatabase::LoadStance(Bot* bot_inst, bool& stance_flag)
{
	if (!bot_inst)
		return false;

	bot_inst->SetDefaultBotStance();

	query = StringFormat("SELECT `stance_id` FROM `bot_stances` WHERE `bot_id` = '%u' LIMIT 1", bot_inst->GetBotID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	bot_inst->SetBotStance((EQ::constants::StanceType)Strings::ToInt(row[0]));
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
		timer_id = Strings::ToInt(row[0]) - 1;
		timer_value = Strings::ToInt(row[1]);
		max_value = Strings::ToInt(row[2]);

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

		query = fmt::format(
				"REPLACE INTO `bot_timers` (`bot_id`, `timer_id`, `timer_value`) VALUES ('{}', '{}', '{}')",
				bot_inst->GetBotID(), (timer_index + 1), bot_timers[timer_index]
		);
		auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	item_count = Strings::ToInt(row[0]);

	return true;
}

bool BotDatabase::LoadItems(const uint32 bot_id, EQ::InventoryProfile& inventory_inst)
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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row = results.begin(); row != results.end(); ++row) {
		int16 slot_id = Strings::ToInt(row[0]);
		if (slot_id < EQ::invslot::EQUIPMENT_BEGIN || slot_id > EQ::invslot::EQUIPMENT_END)
			continue;

		uint32 item_id = Strings::ToInt(row[1]);
		uint16 item_charges = (uint16)Strings::ToInt(row[2]);

		EQ::ItemInstance* item_inst = database.CreateItem(
			item_id,
			item_charges,
			(uint32)Strings::ToUnsignedInt(row[9]),
			(uint32)Strings::ToUnsignedInt(row[10]),
			(uint32)Strings::ToUnsignedInt(row[11]),
			(uint32)Strings::ToUnsignedInt(row[12]),
			(uint32)Strings::ToUnsignedInt(row[13]),
			(uint32)Strings::ToUnsignedInt(row[14])
		);
		if (!item_inst) {
			LogError("Warning: bot_id [{}] has an invalid item_id [{}] in inventory slot [{}]", bot_id, item_id, slot_id);
			continue;
		}

		if (item_charges == 0x7FFF)
			item_inst->SetCharges(-1);
		else if (item_charges == 0 && item_inst->IsStackable()) // Stackable items need a minimum charge of 1 remain moveable.
			item_inst->SetCharges(1);
		else
			item_inst->SetCharges(item_charges);

		uint32 item_color = Strings::ToUnsignedInt(row[3]);
		if (item_color > 0)
			item_inst->SetColor(item_color);

		if (item_inst->GetItem()->Attuneable) {
			if (Strings::ToInt(row[4]))
				item_inst->SetAttuned(true);
			else if (slot_id >= EQ::invslot::EQUIPMENT_BEGIN && slot_id <= EQ::invslot::EQUIPMENT_END)
				item_inst->SetAttuned(true);
		}

		if (row[5]) {
			std::string data_str(row[5]);
			item_inst->SetCustomDataString(data_str);
		}

		item_inst->SetOrnamentIcon((uint32)Strings::ToUnsignedInt(row[6]));
		item_inst->SetOrnamentationIDFile((uint32)Strings::ToUnsignedInt(row[7]));
		item_inst->SetOrnamentHeroModel((uint32)Strings::ToUnsignedInt(row[8]));

		if (inventory_inst.PutItem(slot_id, *item_inst) == INVALID_INDEX)
			LogError("Warning: Invalid slot_id for item in inventory: bot_id = [{}], item_id = [{}], slot_id = [{}]", bot_id, item_id, slot_id);

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
	auto results = database.QueryDatabase(query);
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
	if (!bot_id || slot_id > EQ::invslot::EQUIPMENT_END)
		return false;

	query = StringFormat("SELECT `item_id` FROM `bot_inventories` WHERE `bot_id` = '%i' AND `slot_id` = '%i' LIMIT 1", bot_id, slot_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	item_id = Strings::ToInt(row[0]);

	return true;
}

bool BotDatabase::LoadItemSlots(const uint32 bot_id, std::map<uint16, uint32>& m)
{
	if (!bot_id) {
		return false;
	}

	const auto& l = BotInventoriesRepository::GetWhere(
		database,
		fmt::format(
			"bot_id = {}",
			bot_id
		)
	);

	if (!l.empty()) {
		for (const auto& e : l) {
			m.emplace(std::pair<uint16, uint32>(e.slot_id, e.item_id));
		}
	}

	return true;
}

bool BotDatabase::SaveItemBySlot(Bot* bot_inst, const uint32 slot_id, const EQ::ItemInstance* item_inst)
{
	if (!bot_inst || !bot_inst->GetBotID() || slot_id > EQ::invslot::EQUIPMENT_END)
		return false;

	if (!DeleteItemBySlot(bot_inst->GetBotID(), slot_id))
		return false;

	if (!item_inst || !item_inst->GetID())
		return true;
	uint32 augment_id[EQ::invaug::SOCKET_COUNT] = { 0, 0, 0, 0, 0, 0 };
	for (int augment_iter = EQ::invaug::SOCKET_BEGIN; augment_iter <= EQ::invaug::SOCKET_END; ++augment_iter)
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
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		DeleteItemBySlot(bot_inst->GetBotID(), slot_id);
		return false;
	}

	return true;
}

bool BotDatabase::DeleteItemBySlot(const uint32 bot_id, const uint32 slot_id)
{
	if (!bot_id || slot_id > EQ::invslot::EQUIPMENT_END)
		return false;

	query = StringFormat("DELETE FROM `bot_inventories` WHERE `bot_id` = '%u' AND `slot_id` = '%u'", bot_id, slot_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadEquipmentColor(const uint32 bot_id, const uint8 material_slot_id, uint32& rgb)
{
	if (!bot_id)
		return false;

	int16 slot_id = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot_id);
	if (slot_id == INVALID_INDEX)
		return false;

	query = StringFormat("SELECT `inst_color` FROM `bot_inventories` WHERE `bot_id` = '%u' AND `slot_id` = '%u' LIMIT 1", bot_id, slot_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	rgb = Strings::ToUnsignedInt(row[0]);

	return true;
}

bool BotDatabase::SaveEquipmentColor(const uint32 bot_id, const int16 slot_id, const uint32 rgb)
{
	if (!bot_id)
		return false;

	bool all_flag = (slot_id == -2);
	if ((slot_id < EQ::invslot::EQUIPMENT_BEGIN || slot_id > EQ::invslot::EQUIPMENT_END) && !all_flag)
		return false;

	std::string where_clause;
	if (all_flag)
		where_clause = StringFormat(" AND `slot_id` IN ('%u', '%u', '%u', '%u', '%u', '%u', '%u')", EQ::invslot::slotHead, EQ::invslot::slotArms, EQ::invslot::slotWrist1, EQ::invslot::slotHands, EQ::invslot::slotChest, EQ::invslot::slotLegs, EQ::invslot::slotFeet);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	pet_index = Strings::ToInt(row[0]);

	return true;
}

bool BotDatabase::LoadPetSpellID(const uint32 bot_id, uint32& pet_spell_id)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `spell_id` FROM `bot_pets` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	pet_spell_id = Strings::ToInt(row[0]);

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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	pet_spell_id = Strings::ToInt(row[0]);
	pet_name = row[1];
	pet_mana = Strings::ToInt(row[2]);
	pet_hp = Strings::ToInt(row[3]);

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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	int buff_index = 0;
	for (auto row = results.begin(); row != results.end() && buff_index < PET_BUFF_COUNT; ++row) {
		pet_buffs[buff_index].spellid = Strings::ToInt(row[0]);
		pet_buffs[buff_index].level = Strings::ToInt(row[1]);
		pet_buffs[buff_index].duration = Strings::ToInt(row[2]);

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
		if (!IsValidSpell(pet_buffs[buff_index].spellid)) {
			continue;
		}

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
		auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	int item_index = EQ::invslot::EQUIPMENT_BEGIN;
	for (auto row = results.begin(); row != results.end() && (item_index >= EQ::invslot::EQUIPMENT_BEGIN && item_index <= EQ::invslot::EQUIPMENT_END); ++row) {
		pet_items[item_index] = Strings::ToInt(row[0]);
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

	for (int item_index = EQ::invslot::EQUIPMENT_BEGIN; item_index <= EQ::invslot::EQUIPMENT_END; ++item_index) {
		if (!pet_items[item_index])
			continue;

		query = StringFormat("INSERT INTO `bot_pet_inventories` (`pets_index`, `item_id`) VALUES ('%u', '%u')", saved_pet_index, pet_items[item_index]);
		auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
		EQ::invslot::slotHead, EQ::invslot::slotChest, EQ::invslot::slotArms, EQ::invslot::slotWrist1, EQ::invslot::slotWrist2, EQ::invslot::slotHands, EQ::invslot::slotLegs, EQ::invslot::slotFeet,
		slot_id
	);
	auto results = database.QueryDatabase(query);
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
		EQ::invslot::slotHead, EQ::invslot::slotChest, EQ::invslot::slotArms, EQ::invslot::slotWrist1, EQ::invslot::slotWrist2, EQ::invslot::slotHands, EQ::invslot::slotLegs, EQ::invslot::slotFeet
	);
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
		" `follow_distance`,"
		" `stop_melee_level`"
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
		" bd.`follow_distance`,"
		" bd.`stop_melee_level`"
		" FROM `bot_data` bd"
		" WHERE"
		" bd.`owner_id` = '%u'"
		" AND"
		" bd.`bot_id` = '%u'",
		clone_name.c_str(),
		owner_id,
		bot_id
	);
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		DeleteItems(clone_id);
		return false;
	}

	return true;
}

bool BotDatabase::SaveStopMeleeLevel(const uint32 owner_id, const uint32 bot_id, const uint8 sml_value)
{
	if (!owner_id || !bot_id)
		return false;

	query = StringFormat(
		"UPDATE `bot_data`"
		" SET `stop_melee_level` = '%u'"
		" WHERE `owner_id` = '%u'"
		" AND `bot_id` = '%u'",
		sml_value,
		owner_id,
		bot_id
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::LoadOwnerOptions(Client *owner)
{
	if (!owner || !owner->CharacterID()) {
		return false;
	}

	query = fmt::format("SELECT `option_type`, `option_value` FROM `bot_owner_options` WHERE `owner_id` = '{}'", owner->CharacterID());

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row : results) {

		owner->SetBotOption(static_cast<Client::BotOwnerOption>(Strings::ToUnsignedInt(row[0])), (Strings::ToUnsignedInt(row[1]) != 0));
	}

	return true;
}

bool BotDatabase::SaveOwnerOption(const uint32 owner_id, size_t type, const bool flag)
{
	if (!owner_id) {
		return false;
	}

	switch (static_cast<Client::BotOwnerOption>(type)) {
	case Client::booDeathMarquee:
	case Client::booStatsUpdate:
	case Client::booSpawnMessageClassSpecific:
	case Client::booAltCombat:
	case Client::booAutoDefend:
	case Client::booBuffCounter:
	case Client::booMonkWuMessage:
	{
		query = fmt::format(
			"REPLACE INTO `bot_owner_options`(`owner_id`, `option_type`, `option_value`) VALUES ('{}', '{}', '{}')",
			owner_id,
			type,
			(flag == true ? 1 : 0)
		);

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		return true;
	}
	default:
		return false;
	}
}

bool BotDatabase::SaveOwnerOption(const uint32 owner_id, const std::pair<size_t, size_t> type, const std::pair<bool, bool> flag)
{
	if (!owner_id) {
		return false;
	}

	switch (static_cast<Client::BotOwnerOption>(type.first)) {
	case Client::booSpawnMessageSay:
	case Client::booSpawnMessageTell:
	{
		switch (static_cast<Client::BotOwnerOption>(type.second)) {
		case Client::booSpawnMessageSay:
		case Client::booSpawnMessageTell:
		{
			query = fmt::format(
				"REPLACE INTO `bot_owner_options`(`owner_id`, `option_type`, `option_value`) VALUES ('{}', '{}', '{}'), ('{}', '{}', '{}')",
				owner_id,
				type.first,
				(flag.first == true ? 1 : 0),
				owner_id,
				type.second,
				(flag.second == true ? 1 : 0)
			);

			auto results = database.QueryDatabase(query);
			if (!results.Success()) {
				return false;
			}

			return true;
		}
		default:
			return false;
		}
	}
	default:
		return false;
	}
}

/* Bot owner group functions   */
// added owner ID to this function to fix groups with mulitple players grouped with bots.
bool BotDatabase::LoadGroupedBotsByGroupID(const uint32 owner_id, const uint32 group_id, std::list<uint32>& group_list)
{
	if (!group_id || !owner_id) {
		return false;
	}

	query = fmt::format(
		"SELECT `charid` FROM `group_id` WHERE `groupid` = {} AND `name` IN "
		"(SELECT `name` FROM `bot_data` WHERE `owner_id` = {})",
		group_id,
		owner_id
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (!results.RowCount()) {
		return true;
	}

	for (auto row : results) {
		group_list.push_back(Strings::ToUnsignedInt(row[0]));
	}

	return true;
}


/* Bot heal rotation functions   */
bool BotDatabase::LoadHealRotationIDByBotID(const uint32 bot_id, uint32& hr_index)
{
	if (!bot_id)
		return false;

	query = StringFormat("SELECT `heal_rotation_index` FROM `bot_heal_rotations` WHERE `bot_id` = '%u' LIMIT 1", bot_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	hr_index = Strings::ToInt(row[0]);

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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	auto row = results.begin();
	(*hr_member->MemberOfHealRotation())->SetIntervalS((uint32)Strings::ToInt(row[0]));
	(*hr_member->MemberOfHealRotation())->SetFastHeals((bool)Strings::ToInt(row[1]));
	(*hr_member->MemberOfHealRotation())->SetAdaptiveTargeting((bool)Strings::ToInt(row[2]));
	(*hr_member->MemberOfHealRotation())->SetCastingOverride((bool)Strings::ToInt(row[3]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_UNKNOWN, Strings::ToFloat(row[4]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_CLOTH, Strings::ToFloat(row[5]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_LEATHER, Strings::ToFloat(row[6]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_CHAIN, Strings::ToFloat(row[7]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(ARMOR_TYPE_PLATE, Strings::ToFloat(row[8]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_UNKNOWN, Strings::ToFloat(row[9]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_CLOTH, Strings::ToFloat(row[10]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_LEATHER, Strings::ToFloat(row[11]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_CHAIN, Strings::ToFloat(row[12]));
	(*hr_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_PLATE, Strings::ToFloat(row[13]));

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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;
	if (!results.RowCount())
		return true;

	for (auto row : results) {
		if (row[0])
			member_list.push_back(Strings::ToInt(row[0]));
	}

	return true;
}

bool BotDatabase::LoadHealRotationTargets(const uint32 hr_index, std::list<std::string>& target_list)
{
	if (!hr_index)
		return false;

	query = StringFormat("SELECT `target_name` FROM `bot_heal_rotation_targets` WHERE `heal_rotation_index` = '%u'", hr_index);
	auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
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
		auto results = database.QueryDatabase(query);
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
		auto results = database.QueryDatabase(query);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	query = StringFormat("DELETE FROM `bot_heal_rotation_members` WHERE `heal_rotation_index` = '%u'", hr_index);
	results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	query = StringFormat("DELETE FROM `bot_heal_rotations` WHERE `heal_rotation_index` = '%u'", hr_index);
	results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool BotDatabase::DeleteAllHealRotations(const uint32 owner_id)
{
	if (!owner_id)
		return false;

	query = StringFormat("SELECT `bot_id` FROM `bot_heal_rotations` WHERE `bot_id` IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = '%u')", owner_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row : results) {
		if (!row[0])
			continue;

		DeleteHealRotation(Strings::ToInt(row[0]));
	}

	return true;
}


/* Bot miscellaneous functions   */
uint8 BotDatabase::GetSpellCastingChance(uint8 spell_type_index, uint8 class_index, uint8 stance_index, uint8 conditional_index) // class_index is 0-based
{
	if (spell_type_index >= Bot::SPELL_TYPE_COUNT)
		return 0;
	if (class_index >= PLAYER_CLASS_COUNT)
		return 0;
	if (stance_index >= EQ::constants::STANCE_TYPE_COUNT)
		return 0;
	if (conditional_index >= cntHSND)
		return 0;

	return Bot::spell_casting_chances[spell_type_index][class_index][stance_index][conditional_index];
}

uint16 BotDatabase::GetRaceClassBitmask(uint16 bot_race)
{
	std::string query = fmt::format(
		"SELECT `classes` FROM `bot_create_combinations` WHERE `race` = {}",
		bot_race
	);
	auto results = database.QueryDatabase(query);
	uint16 classes = 0;
	if (results.RowCount() == 1) {
		auto row = results.begin();
		classes = Strings::ToInt(row[0]);
	}
	return classes;
}

bool BotDatabase::SaveExpansionBitmask(const uint32 bot_id, const int expansion_bitmask)
{
	if (!bot_id) {
		return false;
	}

	query = fmt::format(
		"UPDATE `bot_data` "
		"SET `expansion_bitmask` = {} "
		"WHERE `bot_id` = {}",
		expansion_bitmask,
		bot_id
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

bool BotDatabase::SaveEnforceSpellSetting(const uint32 bot_id, const bool enforce_spell_setting)
{
	if (!bot_id) {
		return false;
	}

	query = fmt::format(
		"UPDATE `bot_data`"
		"SET `enforce_spell_settings` = {} "
		"WHERE `bot_id` = {}",
		(enforce_spell_setting ? 1 : 0),
		bot_id
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

bool BotDatabase::SaveBotArcherSetting(const uint32 bot_id, const bool bot_archer_setting)
{
	if (!bot_id) {
		return false;
	}
	query = fmt::format(
		"UPDATE `bot_data`"
		"SET `archery_setting` = {} "
		"WHERE `bot_id` = {}",
		(bot_archer_setting ? 1 : 0),
		bot_id
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}
	return true;
}

std::string BotDatabase::GetBotNameByID(const uint32 bot_id)
{

	if (!bot_id) {
		return nullptr;
	}
	query = fmt::format(
		"SELECT `name` FROM `bot_data` WHERE `bot_id` = {}",
		bot_id
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		return nullptr;
	}

	if (results.RowCount() == 1) {
		auto row = results.begin();
		return row[0];
	}

	return nullptr;
}

bool BotDatabase::SaveBotCasterRange(const uint32 owner_id, const uint32 bot_id, const uint32 bot_caster_range_value)
{
	if (!owner_id || !bot_id) {
		return false;
	}

	query = fmt::format(
			"UPDATE `bot_data`"
			" SET `caster_range` = '{}'"
			" WHERE `owner_id` = '{}'"
			" AND `bot_id` = '{}'",
			bot_caster_range_value,
			owner_id,
			bot_id
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	return true;
}

/* fail::Bot functions   */
const char* BotDatabase::fail::LoadBotsList() { return "Failed to bots list"; }
const char* BotDatabase::fail::LoadOwnerID() { return "Failed to load owner ID"; }
const char* BotDatabase::fail::LoadBotID() { return "Failed to load bot ID"; }
const char* BotDatabase::fail::LoadBot() { return "Failed to load bot"; }
const char* BotDatabase::fail::SaveNewBot() { return "Failed to save new bot"; }
const char* BotDatabase::fail::SaveBot() { return "Failed to save bot"; }
const char* BotDatabase::fail::DeleteBot() { return "Failed to delete bot"; }
const char* BotDatabase::fail::LoadBuffs() { return "Failed to load buffs"; }
const char* BotDatabase::fail::SaveBuffs() { return "Failed to save buffs"; }
const char* BotDatabase::fail::DeleteBuffs() { return "Failed to delete buffs"; }
const char* BotDatabase::fail::DeleteStance() { return "Failed to delete stance"; }
const char* BotDatabase::fail::LoadTimers() { return "Failed to load timers"; }
const char* BotDatabase::fail::SaveTimers() { return "Failed to save timers"; }
const char* BotDatabase::fail::DeleteTimers() { return "Failed to delete timers"; }

/* fail::Bot inventory functions   */
const char* BotDatabase::fail::QueryInventoryCount() { return "Failed to query inventory count"; }
const char* BotDatabase::fail::LoadItems() { return "Failed to load items"; }
const char* BotDatabase::fail::SaveItems() { return "Failed to save items"; }
const char* BotDatabase::fail::DeleteItems() { return "Failed to delete items"; }
const char* BotDatabase::fail::SaveItemBySlot() { return "Failed to save item by slot"; }
const char* BotDatabase::fail::DeleteItemBySlot() { return "Failed to delete item by slot"; }
const char* BotDatabase::fail::LoadEquipmentColor() { return "Failed to load equipment color"; }
const char* BotDatabase::fail::SaveEquipmentColor() { return "Failed to save equipment color"; }

/* fail::Bot pet functions   */
const char* BotDatabase::fail::LoadPetIndex() { return "Failed to load pet index"; }
const char* BotDatabase::fail::LoadPetSpellID() { return "Failed to load pet spell ID"; }
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
const char* BotDatabase::fail::SaveStopMeleeLevel() { return "Failed to save stop melee level"; }
const char* BotDatabase::fail::SaveBotCasterRange() { return "Failed to save caster range"; }

/* fail::Bot heal rotation functions   */
const char* BotDatabase::fail::LoadHealRotationIDByBotID() { return "Failed to load heal rotation ID by bot ID"; }
const char* BotDatabase::fail::LoadHealRotation() { return "Failed to load heal rotation"; }
const char* BotDatabase::fail::LoadHealRotationMembers() { return "Failed to load heal rotation members"; }
const char* BotDatabase::fail::LoadHealRotationTargets() { return "Failed to load heal rotation targets"; }
const char* BotDatabase::fail::SaveHealRotation() { return "Failed to save heal rotation"; }
const char* BotDatabase::fail::DeleteHealRotation() { return "Failed to delete heal rotation"; }
const char* BotDatabase::fail::DeleteAllHealRotations() { return "Failed to delete all heal rotations"; }

/* fail::Bot miscellaneous functions   */
const char* BotDatabase::fail::GetBotNameByID() { return "Failed to get bot name by bot ID"; }
const char* BotDatabase::fail::LoadGroupedBotsByGroupID() { return "Failed to load grouped bots by group ID."; }
