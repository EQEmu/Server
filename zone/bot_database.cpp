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

#include "../common/repositories/bot_buffs_repository.h"
#include "../common/repositories/bot_create_combinations_repository.h"
#include "../common/repositories/bot_data_repository.h"
#include "../common/repositories/bot_heal_rotations_repository.h"
#include "../common/repositories/bot_heal_rotation_members_repository.h"
#include "../common/repositories/bot_heal_rotation_targets_repository.h"
#include "../common/repositories/bot_inspect_messages_repository.h"
#include "../common/repositories/bot_inventories_repository.h"
#include "../common/repositories/bot_owner_options_repository.h"
#include "../common/repositories/bot_pets_repository.h"
#include "../common/repositories/bot_pet_buffs_repository.h"
#include "../common/repositories/bot_pet_inventories_repository.h"
#include "../common/repositories/bot_spell_casting_chances_repository.h"
#include "../common/repositories/bot_stances_repository.h"
#include "../common/repositories/bot_timers_repository.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/repositories/group_id_repository.h"

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
	const auto& l = BotSpellCastingChancesRepository::All(database);

	if (l.empty()) {
		return false;
	}

	std::vector<uint8> conditions = { };

	for (const auto& e : l) {
		if (
			e.spell_type_index >= Bot::SPELL_TYPE_COUNT ||
			!IsPlayerClass(e.class_id) ||
			e.stance_index >= EQ::constants::STANCE_TYPE_COUNT
		) {
			continue;
		}

		const uint8 class_index = (e.class_id - 1);

		conditions = {
			e.nHSND_value,
			e.pH_value,
			e.pS_value,
			e.pHS_value,
			e.pN_value,
			e.pHN_value,
			e.pSN_value,
			e.pHSN_value,
			e.pD_value,
			e.pHD_value,
			e.pSD_value,
			e.pHSD_value,
			e.pND_value,
			e.pHND_value,
			e.pSND_value,
			e.pHSN_value
		};

		for (uint8 index = 0; index < conditions.size(); index++) {
			uint8 value = conditions[index];

			if (!value) {
				continue;
			}

			if (value > 100) {
				value = 100;
			}

			Bot::spell_casting_chances[e.spell_type_index][class_index][e.stance_index][index] = value;
		}
	}

	return true;
}

bool BotDatabase::QueryNameAvailablity(const std::string& bot_name, bool& available_flag)
{
	if (
		bot_name.empty() ||
		bot_name.size() > 60 ||
		database.IsNameUsed(bot_name)
	) {
		return false;
	}

	available_flag = true;

	return true;
}

bool BotDatabase::QueryBotCount(const uint32 owner_id, int class_id, uint32& bot_count, uint32& bot_class_count)
{
	if (!owner_id) {
		return false;
	}

	bot_count = BotDataRepository::Count(
		database,
		fmt::format(
			"`owner_id` = {}",
			owner_id
		)
	);

	if (IsPlayerClass(class_id)) {
		bot_class_count = BotDataRepository::Count(
			database,
			fmt::format(
				"`owner_id` = {} AND `class` = {}",
				owner_id,
				class_id
			)
		);
	}

	return true;
}

bool BotDatabase::LoadBotsList(const uint32 owner_id, std::list<BotsAvailableList>& bots_list, bool by_account)
{
	if (!owner_id) {
		return false;
	}

	BotsAvailableList ble;

	if (by_account) {
		const std::string& owner_name = database.GetCharNameByID(owner_id);

		const auto& l = BotDataRepository::GetWhere(
			database,
			fmt::format(
				SQL(
					`owner_id` IN
					(
						SELECT `id` FROM `character_data` WHERE `account_id` =
						(
							SELECT `account_id` FROM `character_data` WHERE `id` = {}
						)
					)
				),
				owner_id
			)
		);

		if (l.empty()) {
			return true;
		}

		for (const auto& e : l) {
			ble.bot_id   = e.bot_id;
			ble.class_   = e.class_;
			ble.level    = e.level;
			ble.race     = e.race;
			ble.gender   = e.gender;
			ble.owner_id = e.owner_id;

			strn0cpy(ble.bot_name, e.name.c_str(), sizeof(ble.bot_name));
			strn0cpy(ble.owner_name, owner_name.c_str(), sizeof(ble.owner_name));

			bots_list.emplace_back(ble);
		}
	} else {
		const auto& l = BotDataRepository::GetWhere(
			database,
			fmt::format(
				"`owner_id` = {}",
				owner_id
			)
		);

		if (l.empty()) {
			return true;
		}

		for (const auto& e : l) {
			ble.bot_id   = e.bot_id;
			ble.class_   = e.class_;
			ble.level    = e.level;
			ble.race     = e.race;
			ble.gender   = e.gender;
			ble.owner_id = e.owner_id;

			strn0cpy(ble.bot_name, e.name.c_str(), sizeof(ble.bot_name));
			strn0cpy(ble.owner_name, "You", sizeof(ble.owner_name));

			bots_list.emplace_back(ble);
		}
	}

	return true;
}

uint32 BotDatabase::GetOwnerID(const uint32 bot_id)
{
	if (!bot_id) {
		return 0;
	}

	const auto& l = BotDataRepository::FindOne(database, bot_id);

	return l.bot_id ? l.owner_id : 0;
}

bool BotDatabase::LoadBotID(const std::string& bot_name, uint32& bot_id, uint8& bot_class_id)
{
	if (bot_name.empty()) {
		return false;
	}

	const auto& l = BotDataRepository::GetWhere(
		database,
		fmt::format(
			"`name` = '{}' LIMIT 1",
			Strings::Escape(bot_name)
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	bot_id       = e.bot_id;
	bot_class_id = e.class_;

	return true;
}

bool BotDatabase::LoadBot(const uint32 bot_id, Bot*& loaded_bot)
{
	if (!bot_id || loaded_bot) {
		return false;
	}

	const auto& e = BotDataRepository::FindOne(database, bot_id);
	if (!e.bot_id) {
		return false;
	}

	auto d = Bot::CreateDefaultNPCTypeStructForBot(
		e.name,
		e.last_name,
		e.level,
		e.race,
		e.class_,
		e.gender
	);

	auto t = Bot::FillNPCTypeStruct(
		e.spells_id,
		e.name,
		e.last_name,
		e.level,
		e.race,
		e.class_,
		e.gender,
		e.size,
		e.face,
		e.hair_style,
		e.hair_color,
		e.eye_color_1,
		e.eye_color_2,
		e.beard,
		e.beard_color,
		e.drakkin_heritage,
		e.drakkin_tattoo,
		e.drakkin_details,
		e.hp,
		e.mana,
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
		e.owner_id,
		e.spells_id,
		e.time_spawned,
		e.zone_id,
		t,
		e.expansion_bitmask
	);

	if (loaded_bot) {
		loaded_bot->SetSurname(e.last_name);
		loaded_bot->SetTitle(e.title);
		loaded_bot->SetSuffix(e.suffix);

		loaded_bot->SetShowHelm(e.show_helm);

		auto bfd = EQ::Clamp(e.follow_distance, static_cast<uint32>(1), BOT_FOLLOW_DISTANCE_DEFAULT_MAX);

		loaded_bot->SetFollowDistance(bfd);

		loaded_bot->SetStopMeleeLevel(e.stop_melee_level);

		loaded_bot->SetBotEnforceSpellSetting(e.enforce_spell_settings);

		loaded_bot->SetBotArcherySetting(e.archery_setting);

		loaded_bot->SetBotCasterRange(e.caster_range);
	}

	return true;
}

bool BotDatabase::SaveNewBot(Bot* b, uint32& bot_id)
{
	if (!b) {
		return false;
	}

	auto e = BotDataRepository::NewEntity();

	e.owner_id               = b->GetBotOwnerCharacterID();
	e.spells_id              = b->GetBotSpellID();
	e.name                   = b->GetCleanName();
	e.last_name              = b->GetLastName();
	e.title                  = b->GetTitle();
	e.suffix                 = b->GetSuffix();
	e.zone_id                = b->GetLastZoneID();
	e.gender                 = b->GetGender();
	e.race                   = b->GetBaseRace();
	e.class_                 = b->GetClass();
	e.level                  = b->GetLevel();
	e.creation_day           = std::time(nullptr);
	e.last_spawn             = std::time(nullptr);
	e.size                   = b->GetSize();
	e.face                   = b->GetLuclinFace();
	e.hair_color             = b->GetHairColor();
	e.hair_style             = b->GetHairStyle();
	e.beard                  = b->GetBeard();
	e.beard_color            = b->GetBeardColor();
	e.eye_color_1            = b->GetEyeColor1();
	e.eye_color_2            = b->GetEyeColor2();
	e.drakkin_heritage       = b->GetDrakkinHeritage();
	e.drakkin_tattoo         = b->GetDrakkinTattoo();
	e.drakkin_details        = b->GetDrakkinDetails();
	e.ac                     = b->GetBaseAC();
	e.atk                    = b->GetBaseATK();
	e.hp                     = b->GetHP();
	e.mana                   = b->GetMana();
	e.str                    = b->GetBaseSTR();
	e.sta                    = b->GetBaseSTA();
	e.cha                    = b->GetBaseCHA();
	e.dex                    = b->GetBaseDEX();
	e.int_                   = b->GetBaseINT();
	e.agi                    = b->GetBaseAGI();
	e.wis                    = b->GetBaseWIS();
	e.fire                   = b->GetBaseFR();
	e.cold                   = b->GetBaseCR();
	e.magic                  = b->GetBaseMR();
	e.poison                 = b->GetBasePR();
	e.disease                = b->GetBaseDR();
	e.corruption             = b->GetBaseCorrup();
	e.show_helm              = b->GetShowHelm() ? 1 : 0;
	e.follow_distance        = b->GetFollowDistance();
	e.stop_melee_level       = b->GetStopMeleeLevel();
	e.expansion_bitmask      = b->GetExpansionBitmask();
	e.enforce_spell_settings = b->GetBotEnforceSpellSetting();
	e.archery_setting        = b->IsBotArcher() ? 1 : 0;
	e.caster_range           = b->GetBotCasterRange();

	e = BotDataRepository::InsertOne(database, e);

	if (!e.bot_id) {
		return false;
	}

	bot_id = e.bot_id;

	return true;
}

bool BotDatabase::SaveBot(Bot* b)
{
	if (!b) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, b->GetBotID());
	if (!e.bot_id) {
		return false;
	}

	e.owner_id               = b->GetBotOwnerCharacterID();
	e.spells_id              = b->GetBotSpellID();
	e.name                   = b->GetCleanName();
	e.last_name              = b->GetLastName();
	e.title                  = b->GetTitle();
	e.suffix                 = b->GetSuffix();
	e.zone_id                = b->GetLastZoneID();
	e.gender                 = b->GetBaseGender();
	e.race                   = b->GetBaseRace();
	e.class_                 = b->GetClass();
	e.level                  = b->GetLevel();
	e.last_spawn             = std::time(nullptr);
	e.time_spawned           = b->GetTotalPlayTime();
	e.size                   = b->GetSize();
	e.face                   = b->GetLuclinFace();
	e.hair_color             = b->GetHairColor();
	e.hair_style             = b->GetHairStyle();
	e.beard                  = b->GetBeard();
	e.beard_color            = b->GetBeardColor();
	e.eye_color_1            = b->GetEyeColor1();
	e.eye_color_2            = b->GetEyeColor2();
	e.drakkin_heritage       = b->GetDrakkinHeritage();
	e.drakkin_tattoo         = b->GetDrakkinTattoo();
	e.drakkin_details        = b->GetDrakkinDetails();
	e.ac                     = b->GetBaseAC();
	e.atk                    = b->GetBaseATK();
	e.hp                     = b->GetHP();
	e.mana                   = b->GetMana();
	e.str                    = b->GetBaseSTR();
	e.sta                    = b->GetBaseSTA();
	e.cha                    = b->GetBaseCHA();
	e.dex                    = b->GetBaseDEX();
	e.int_                   = b->GetBaseINT();
	e.agi                    = b->GetBaseAGI();
	e.wis                    = b->GetBaseWIS();
	e.fire                   = b->GetBaseFR();
	e.cold                   = b->GetBaseCR();
	e.magic                  = b->GetBaseMR();
	e.poison                 = b->GetBasePR();
	e.disease                = b->GetBaseDR();
	e.corruption             = b->GetBaseCorrup();
	e.show_helm              = b->GetShowHelm() ? 1 : 0;
	e.follow_distance        = b->GetFollowDistance();
	e.stop_melee_level       = b->GetStopMeleeLevel();
	e.expansion_bitmask      = b->GetExpansionBitmask();
	e.enforce_spell_settings = b->GetBotEnforceSpellSetting();
	e.archery_setting        = b->IsBotArcher() ? 1 : 0;

	return BotDataRepository::UpdateOne(database, e);
}

bool BotDatabase::DeleteBot(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	BotDataRepository::DeleteOne(database, bot_id);

	return true;
}

bool BotDatabase::LoadBuffs(Bot* b)
{
	if (!b) {
		return false;
	}

	const auto& l = BotBuffsRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {}",
			b->GetBotID()
		)
	);

	if (l.empty()) {
		return true;
	}

	auto buffs = b->GetBuffs();

	if (!buffs) {
		return false;
	}

	uint32 max_slots = b->GetMaxBuffSlots();
	for (int index = 0; index < max_slots; index++) {
		buffs[index].spellid = SPELL_UNKNOWN;
	}

	uint32 buff_count = 0;

	for (const auto& e : l) {
		if (buff_count >= BUFF_COUNT) {
			continue;
		}

		buffs[buff_count].spellid       = e.spell_id;
		buffs[buff_count].casterlevel   = e.caster_level;
		buffs[buff_count].ticsremaining = e.tics_remaining;
		buffs[buff_count].counters      = 0;

		if (CalculatePoisonCounters(buffs[buff_count].spellid) > 0) {
			buffs[buff_count].counters = e.poison_counters;
		} else if (CalculateDiseaseCounters(buffs[buff_count].spellid) > 0) {
			buffs[buff_count].counters = e.disease_counters;
		} else if (CalculateCurseCounters(buffs[buff_count].spellid) > 0) {
			buffs[buff_count].counters = e.curse_counters;
		} else if (CalculateCorruptionCounters(buffs[buff_count].spellid) > 0) {
			buffs[buff_count].counters = e.corruption_counters;
		}

		buffs[buff_count].hit_number      = e.numhits;
		buffs[buff_count].melee_rune      = e.melee_rune;
		buffs[buff_count].magic_rune      = e.magic_rune;
		buffs[buff_count].dot_rune        = e.dot_rune;
		buffs[buff_count].persistant_buff = e.persistent;
		buffs[buff_count].caston_x        = e.caston_x;
		buffs[buff_count].caston_y        = e.caston_y;
		buffs[buff_count].caston_z        = e.caston_z;
		buffs[buff_count].ExtraDIChance   = e.extra_di_chance;
		buffs[buff_count].instrument_mod  = e.instrument_mod;
		buffs[buff_count].casterid        = 0;

		++buff_count;
	}

	return true;
}

bool BotDatabase::SaveBuffs(Bot* b)
{
	if (!b) {
		return false;
	}

	if (!DeleteBuffs(b->GetBotID())) {
		return false;
	}

	auto buffs = b->GetBuffs();

	if (!buffs) {
		return false;
	}

	auto e = BotBuffsRepository::NewEntity();

	e.bot_id = b->GetBotID();

	std::vector<BotBuffsRepository::BotBuffs> v = { };

	for (int buff_index = 0; buff_index < BUFF_COUNT; ++buff_index) {
		if (!IsValidSpell(buffs[buff_index].spellid)) {
			continue;
		}

		e.spell_id            = buffs[buff_index].spellid;
		e.caster_level        = buffs[buff_index].casterlevel;
		e.duration_formula    = spells[buffs[buff_index].spellid].buff_duration_formula;
		e.tics_remaining      = buffs[buff_index].ticsremaining;
		e.poison_counters     = CalculatePoisonCounters(buffs[buff_index].spellid) > 0 ? buffs[buff_index].counters : 0;
		e.disease_counters    = CalculateDiseaseCounters(buffs[buff_index].spellid) > 0 ? buffs[buff_index].counters : 0;
		e.curse_counters      = CalculateCurseCounters(buffs[buff_index].spellid) > 0 ? buffs[buff_index].counters : 0;
		e.corruption_counters = CalculateCorruptionCounters(buffs[buff_index].spellid) > 0 ? buffs[buff_index].counters : 0;
		e.numhits             = buffs[buff_index].hit_number;
		e.melee_rune          = buffs[buff_index].melee_rune;
		e.magic_rune          = buffs[buff_index].magic_rune;
		e.dot_rune            = buffs[buff_index].dot_rune;
		e.persistent          = buffs[buff_index].persistant_buff ? 1 : 0;
		e.caston_x            = buffs[buff_index].caston_x;
		e.caston_y            = buffs[buff_index].caston_y;
		e.caston_z            = buffs[buff_index].caston_z;
		e.extra_di_chance     = buffs[buff_index].ExtraDIChance;

		v.emplace_back(e);
	}

	if (!v.empty()) {
		const int inserted = BotBuffsRepository::InsertMany(database, v);

		if (!inserted) {
			DeleteBuffs(b->GetBotID());
			return false;
		}
	}

	return true;
}

bool BotDatabase::DeleteBuffs(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	BotBuffsRepository::DeleteWhere(
		database,
		fmt::format(
			"`bot_id` = {}",
			bot_id
		)
	);

	return true;
}

bool BotDatabase::LoadStance(const uint32 bot_id, int& bot_stance)
{
	if (!bot_id) {
		return false;
	}

	const auto& l = BotStancesRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {} LIMIT 1",
			bot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	bot_stance = e.stance_id;

	return true;
}

bool BotDatabase::LoadStance(Bot* b, bool& stance_flag)
{
	if (!b) {
		return false;
	}

	b->SetDefaultBotStance();

	const auto& l = BotStancesRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {} LIMIT 1",
			b->GetBotID()
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	b->SetBotStance(static_cast<EQ::constants::StanceType>(e.stance_id));

	stance_flag = true;

	return true;
}

bool BotDatabase::SaveStance(const uint32 bot_id, const int bot_stance)
{
	if (!bot_id) {
		return false;
	}

	return BotStancesRepository::ReplaceOne(
		database,
		BotStancesRepository::BotStances{
			.bot_id = bot_id,
			.stance_id = static_cast<uint8_t>(bot_stance)
		}
	);
}

bool BotDatabase::SaveStance(Bot* b)
{
	if (!b) {
		return false;
	}

	return BotStancesRepository::ReplaceOne(
		database,
		BotStancesRepository::BotStances{
			.bot_id = b->GetBotID(),
			.stance_id = static_cast<uint8_t>(b->GetBotStance())
		}
	);
}

bool BotDatabase::DeleteStance(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	BotStancesRepository::DeleteOne(database, bot_id);

	return true;
}

bool BotDatabase::LoadTimers(Bot* b)
{
	if (!b) {
		return false;
	}

	const auto& l = BotTimersRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {}",
			b->GetBotID()
		)
	);

	std::vector<BotTimer_Struct> v;

	BotTimer_Struct t{ };

	for (const auto& e : l) {
		if (t.timer_value < (Timer::GetCurrentTime() + t.recast_time)) {
			t.timer_id    = e.timer_id;
			t.timer_value = e.timer_value;
			t.recast_time = e.recast_time;
			t.is_spell    = e.is_spell;
			t.is_disc     = e.is_disc;
			t.spell_id    = e.spell_id;
			t.is_item     = e.is_item;
			t.item_id     = e.item_id;

			v.push_back(t);
		}
	}

	if (!v.empty()) {
		b->SetBotTimers(v);
	}

	return true;
}

bool BotDatabase::SaveTimers(Bot* b)
{
	if (!b) {
		return false;
	}

	if (!DeleteTimers(b->GetBotID())) {
		return false;
	}

	std::vector<BotTimer_Struct> v = b->GetBotTimers();

	if (v.empty()) {
		return true;
	}

	std::vector<BotTimersRepository::BotTimers> l;

	if (!v.empty()) {
		for (auto & bot_timer : v) {
			if (bot_timer.timer_value <= Timer::GetCurrentTime()) {
				continue;
			}

			auto e = BotTimersRepository::BotTimers{
				.bot_id = b->GetBotID(),
				.timer_id = bot_timer.timer_id,
				.timer_value = bot_timer.timer_value,
				.recast_time = bot_timer.recast_time,
				.is_spell = bot_timer.is_spell,
				.is_disc = bot_timer.is_disc,
				.spell_id = bot_timer.spell_id,
				.is_item = bot_timer.is_item,
				.item_id = bot_timer.item_id
			};

			l.push_back(e);
		}

		if (l.empty()) {
			return true;
		}

		BotTimersRepository::DeleteWhere(
			database,
			fmt::format(
				"`bot_id` = {}",
				b->GetBotID()
			)
		);

		const int inserted = BotTimersRepository::InsertMany(database, l);

		if (!inserted) {
			DeleteTimers(b->GetBotID());
			return false;
		}
	}

	return true;
}

bool BotDatabase::DeleteTimers(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	BotTimersRepository::DeleteWhere(
		database,
		fmt::format(
			"`bot_id` = {}",
			bot_id
		)
	);

	return true;
}

bool BotDatabase::QueryInventoryCount(const uint32 bot_id, uint32& item_count)
{
	if (!bot_id) {
		return false;
	}

	item_count = BotInventoriesRepository::Count(
		database,
		fmt::format(
			"`bot_id` = {}",
			bot_id
		)
	);

	return true;
}

bool BotDatabase::LoadItems(const uint32 bot_id, EQ::InventoryProfile& inventory_inst)
{
	if (!bot_id) {
		return false;
	}

	const auto& l = BotInventoriesRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {} ORDER BY `slot_id`",
			bot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		if (!EQ::ValueWithin(e.slot_id, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) {
			continue;
		}

		auto inst = database.CreateItem(
			e.item_id,
			e.inst_charges,
			e.augment_1,
			e.augment_2,
			e.augment_3,
			e.augment_4,
			e.augment_5,
			e.augment_6
		);

		if (!inst) {
			LogError(
				"Warning: bot_id [{}] has an invalid item_id [{}] in slot_id [{}]",
				bot_id,
				e.item_id,
				e.slot_id
			);

			continue;
		}

		if (e.inst_charges == INT16_MAX) {
			inst->SetCharges(-1);
		} else if (
			e.inst_charges == 0 &&
			inst->IsStackable()
		) { // Stackable items need a minimum charge of 1 remain moveable.
			inst->SetCharges(1);
		} else {
			inst->SetCharges(e.inst_charges);
		}

		if (e.inst_color) {
			inst->SetColor(e.inst_color);
		}

		if (inst->GetItem()->Attuneable) {
			if (e.inst_no_drop) {
				inst->SetAttuned(true);
			} else if (EQ::ValueWithin(e.slot_id, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) {
				inst->SetAttuned(true);
			}
		}

		if (!e.inst_custom_data.empty()) {
			inst->SetCustomDataString(e.inst_custom_data);
		}

		inst->SetOrnamentIcon(e.ornament_icon);
		inst->SetOrnamentationIDFile(e.ornament_id_file);
		inst->SetOrnamentHeroModel(e.ornament_hero_model);

		if (inventory_inst.PutItem(e.slot_id, *inst) == INVALID_INDEX) {
			LogError(
				"Warning: Invalid slot_id for item in inventory: bot_id [{}] item_id [{}] slot_id [{}]",
				bot_id,
				e.item_id,
				e.slot_id
			);
		}

		safe_delete(inst);
	}

	return true;
}

bool BotDatabase::DeleteItems(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	BotInventoriesRepository::DeleteOne(database, bot_id);

	return true;
}

bool BotDatabase::LoadItemBySlot(const uint32 bot_id, const uint32 slot_id, uint32& item_id)
{
	if (!bot_id || slot_id > EQ::invslot::EQUIPMENT_END) {
		return false;
	}

	const auto& l = BotInventoriesRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {} AND `slot_id` = {} LIMIT 1",
			bot_id,
			slot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	item_id = e.item_id;

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
			"`bot_id` = {}",
			bot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		m.emplace(std::pair<uint16, uint32>(e.slot_id, e.item_id));
	}

	return true;
}

bool BotDatabase::SaveItemBySlot(Bot* b, const uint32 slot_id, const EQ::ItemInstance* inst)
{
	if (
		!b ||
		!b->GetBotID() ||
		slot_id > EQ::invslot::EQUIPMENT_END
	) {
		return false;
	}

	if (!inst || !inst->GetID()) {
		return true;
	}

	DeleteItemBySlot(b->GetBotID(), slot_id);

	uint32 augment_id[EQ::invaug::SOCKET_COUNT] = { 0, 0, 0, 0, 0, 0 };

	for (uint16 slot_id = EQ::invaug::SOCKET_BEGIN; slot_id <= EQ::invaug::SOCKET_END; ++slot_id) {
		augment_id[slot_id] = inst->GetAugmentItemID(slot_id);
	}

	uint16 item_charges = 0;

	if (inst->GetCharges() >= 0) {
		item_charges = inst->GetCharges();
	} else {
		item_charges = INT16_MAX;
	}

	auto e = BotInventoriesRepository::NewEntity();

	e.bot_id              = b->GetBotID();
	e.slot_id             = slot_id;
	e.item_id             = inst->GetID();
	e.inst_charges        = item_charges;
	e.inst_color          = inst->GetColor();
	e.inst_no_drop        = inst->IsAttuned() ? 1 : 0;
	e.inst_custom_data    = inst->GetCustomDataString();
	e.ornament_icon       = inst->GetOrnamentationIcon();
	e.ornament_id_file    = inst->GetOrnamentationIDFile();
	e.ornament_hero_model = inst->GetOrnamentHeroModel();
	e.augment_1           = augment_id[0];
	e.augment_2           = augment_id[1];
	e.augment_3           = augment_id[2];
	e.augment_4           = augment_id[3];
	e.augment_5           = augment_id[4];
	e.augment_6           = augment_id[5];

	return BotInventoriesRepository::InsertOne(database, e).inventories_index;
}

bool BotDatabase::DeleteItemBySlot(const uint32 bot_id, const uint32 slot_id)
{
	if (!bot_id || slot_id > EQ::invslot::EQUIPMENT_END) {
		return false;
	}

	BotInventoriesRepository::DeleteWhere(
		database,
		fmt::format(
			"`bot_id` = {} AND `slot_id` = {}",
			bot_id,
			slot_id
		)
	);

	return true;
}

bool BotDatabase::SaveEquipmentColor(const uint32 bot_id, const int16 slot_id, const uint32 color)
{
	if (!bot_id) {
		return false;
	}

	const bool all_flag = (slot_id == -2);

	if (!EQ::ValueWithin(slot_id, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END) && !all_flag) {
		return false;
	}

	std::string where_clause;
	if (all_flag) {
		where_clause = fmt::format(
			"IN ({}, {}, {}, {}, {}, {}, {})",
			EQ::invslot::slotHead,
			EQ::invslot::slotArms,
			EQ::invslot::slotWrist1,
			EQ::invslot::slotHands,
			EQ::invslot::slotChest,
			EQ::invslot::slotLegs,
			EQ::invslot::slotFeet
		);
	} else {
		where_clause = fmt::format(
			"= {}",
			slot_id
		);
	}

	return BotInventoriesRepository::UpdateItemColors(database, bot_id, color, where_clause);
}

bool BotDatabase::LoadPetIndex(const uint32 bot_id, uint32& pet_index)
{
	if (!bot_id) {
		return false;
	}

	const auto& l = BotPetsRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {} LIMIT 1",
			bot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	pet_index = e.pets_index;

	return true;
}

bool BotDatabase::LoadPetSpellID(const uint32 bot_id, uint32& pet_spell_id)
{
	if (!bot_id) {
		return false;
	}

	const auto& l = BotPetsRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {} LIMIT 1",
			bot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	pet_spell_id = e.spell_id;

	return true;
}

bool BotDatabase::LoadPetStats(const uint32 bot_id, std::string& pet_name, uint32& pet_mana, uint32& pet_hp, uint32& pet_spell_id)
{
	if (!bot_id) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	const auto& l = BotPetsRepository::GetWhere(
		database,
		fmt::format(
			"`pets_index` = {} LIMIT 1",
			saved_pet_index
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	pet_spell_id = e.spell_id;
	pet_name     = e.name;
	pet_mana     = e.mana;
	pet_hp       = e.hp;

	return true;
}

bool BotDatabase::SavePetStats(const uint32 bot_id, const std::string& pet_name, const uint32 pet_mana, const uint32 pet_hp, const uint32 pet_spell_id)
{
	if (!bot_id || pet_name.empty() || !pet_spell_id || pet_spell_id > SPDAT_RECORDS) {
		return false;
	}

	if (
		!DeletePetItems(bot_id) ||
		!DeletePetBuffs(bot_id) ||
		!DeletePetStats(bot_id)
	) {
		return false;
	}

	return BotPetsRepository::InsertOne(
		database,
		BotPetsRepository::BotPets{
			.spell_id = pet_spell_id,
			.bot_id = bot_id,
			.name = pet_name,
			.mana = static_cast<int32_t>(pet_mana),
			.hp = static_cast<int32_t>(pet_hp)
		}
	).pets_index;
}

bool BotDatabase::DeletePetStats(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	return BotPetsRepository::DeleteOne(database, saved_pet_index) == 1;
}

bool BotDatabase::LoadPetBuffs(const uint32 bot_id, SpellBuff_Struct* pet_buffs)
{
	if (!bot_id) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	const auto& l = BotPetBuffsRepository::GetWhere(
		database,
		fmt::format(
			"`pets_index` = {}",
			saved_pet_index
		)
	);

	if (l.empty()) {
		return true;
	}

	uint16 buff_index = 0;

	for (const auto& e : l) {
		if (buff_index >= PET_BUFF_COUNT) {
			break;
		}

		pet_buffs[buff_index].spellid  = e.spell_id;
		pet_buffs[buff_index].level    = e.caster_level;
		pet_buffs[buff_index].duration = e.duration;

		if (CalculatePoisonCounters(pet_buffs[buff_index].spellid) > 0) {
			pet_buffs[buff_index].counters = CalculatePoisonCounters(pet_buffs[buff_index].spellid);
		} else if (CalculateDiseaseCounters(pet_buffs[buff_index].spellid) > 0) {
			pet_buffs[buff_index].counters = CalculateDiseaseCounters(pet_buffs[buff_index].spellid);
		} else if (CalculateCurseCounters(pet_buffs[buff_index].spellid) > 0) {
			pet_buffs[buff_index].counters = CalculateCurseCounters(pet_buffs[buff_index].spellid);
		} else if (CalculateCorruptionCounters(pet_buffs[buff_index].spellid) > 0) {
			pet_buffs[buff_index].counters = CalculateCorruptionCounters(pet_buffs[buff_index].spellid);
		}

		++buff_index;
	}

	return true;
}

bool BotDatabase::SavePetBuffs(const uint32 bot_id, const SpellBuff_Struct* pet_buffs, bool delete_flag)
{
	if (
		!bot_id ||
		!pet_buffs ||
		(delete_flag && !DeletePetBuffs(bot_id))
	) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	auto e = BotPetBuffsRepository::NewEntity();

	e.pets_index = saved_pet_index;

	std::vector<BotPetBuffsRepository::BotPetBuffs> v;

	for (uint16 buff_index = 0; buff_index < PET_BUFF_COUNT; ++buff_index) {
		if (!IsValidSpell(pet_buffs[buff_index].spellid)) {
			continue;
		}

		e.spell_id     = pet_buffs[buff_index].spellid;
		e.caster_level = pet_buffs[buff_index].level;
		e.duration     = pet_buffs[buff_index].duration;

		v.emplace_back(e);
	}

	if (!v.empty()) {
		BotPetBuffsRepository::InsertMany(database, v);
	}

	return true;
}

bool BotDatabase::DeletePetBuffs(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	BotPetBuffsRepository::DeleteOne(database, saved_pet_index);

	return true;
}

bool BotDatabase::LoadPetItems(const uint32 bot_id, uint32* pet_items)
{
	if (!bot_id || !pet_items) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	const auto& l = BotPetInventoriesRepository::GetWhere(
		database,
		fmt::format(
			"`pets_index` = {}",
			saved_pet_index
		)
	);

	if (l.empty()) {
		return true;
	}

	int16 slot_id = EQ::invslot::EQUIPMENT_BEGIN;

	for (const auto& e : l) {
		if (!EQ::ValueWithin(slot_id, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) {
			break;
		}

		pet_items[slot_id] = e.item_id;
		++slot_id;
	}

	return true;
}

bool BotDatabase::SavePetItems(const uint32 bot_id, const uint32* pet_items, bool delete_flag)
{
	// Only use 'delete_flag' if not invoked after a botdb.SavePetStats() call

	if (
		!bot_id ||
		!pet_items ||
			(delete_flag && !DeletePetItems(bot_id))
	) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	auto e = BotPetInventoriesRepository::NewEntity();

	e.pets_index = saved_pet_index;

	std::vector<BotPetInventoriesRepository::BotPetInventories> v;

	for (int slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		if (!pet_items[slot_id]) {
			continue;
		}

		e.item_id = pet_items[slot_id];

		v.emplace_back(e);
	}

	if (!v.empty()) {
		BotPetInventoriesRepository::InsertMany(database, v);
	}

	return true;
}

bool BotDatabase::DeletePetItems(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	uint32 saved_pet_index = 0;

	if (!LoadPetIndex(bot_id, saved_pet_index)) {
		return false;
	}

	if (!saved_pet_index) {
		return true;
	}

	BotPetInventoriesRepository::DeleteOne(database, saved_pet_index);

	return true;
}

bool BotDatabase::LoadInspectMessage(const uint32 bot_id, InspectMessage_Struct& inspect_message)
{
	if (!bot_id) {
		return false;
	}

	const auto& e = BotInspectMessagesRepository::FindOne(database, bot_id);

	if (!e.bot_id) {
		return false;
	}

	if (e.inspect_message.empty()) {
		return true;
	}

	std::string bot_message = e.inspect_message;

	if (bot_message.size() > UINT8_MAX) {
		bot_message = bot_message.substr(0, UINT8_MAX);
	}

	strn0cpy(inspect_message.text, bot_message.c_str(), sizeof(inspect_message.text));

	return true;
}

bool BotDatabase::SaveInspectMessage(const uint32 bot_id, const InspectMessage_Struct& inspect_message)
{
	if (!bot_id || !DeleteInspectMessage(bot_id)) {
		return false;
	}

	std::string bot_message = inspect_message.text;

	if (bot_message.empty()) {
		return true;
	}

	if (bot_message.size() > UINT8_MAX) {
		bot_message = bot_message.substr(0, UINT8_MAX);
	}
	return true;
}

bool BotDatabase::DeleteInspectMessage(const uint32 bot_id)
{
	if (!bot_id) {
		return false;
	}

	BotInspectMessagesRepository::DeleteOne(database, bot_id);

	return true;
}

bool BotDatabase::SaveAllInspectMessages(const uint32 owner_id, const InspectMessage_Struct& inspect_message)
{
	if (!owner_id || !DeleteAllInspectMessages(owner_id)) {
		return false;
	}

	std::string bot_message = inspect_message.text;

	if (bot_message.empty()) {
		return true;
	}

	if (bot_message.size() > UINT8_MAX) {
		bot_message = bot_message.substr(0, UINT8_MAX);
	}

	return BotInspectMessagesRepository::SaveAllInspectMessages(database, owner_id, bot_message);
}

bool BotDatabase::DeleteAllInspectMessages(const uint32 owner_id)
{
	if (!owner_id) {
		return false;
	}

	BotInspectMessagesRepository::DeleteAllInspectMessages(database, owner_id);

	return true;
}

bool BotDatabase::SaveAllArmorColorBySlot(const uint32 owner_id, const int16 slot_id, const uint32 rgb_value)
{
	if (!owner_id) {
		return false;
	}

	BotInventoriesRepository::SaveAllArmorColorsBySlot(database, owner_id, slot_id, rgb_value);

	return true;
}

bool BotDatabase::SaveAllArmorColors(const uint32 owner_id, const uint32 rgb_value)
{
	if (!owner_id) {
		return false;
	}

	return BotInventoriesRepository::SaveAllArmorColors(database, owner_id, rgb_value);
}

bool BotDatabase::SaveHelmAppearance(const uint32 bot_id, const bool show_flag)
{
	if (!bot_id) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	e.show_helm = show_flag ? 1 : 0;

	return BotDataRepository::UpdateOne(database, e);
}

bool BotDatabase::SaveAllHelmAppearances(const uint32 owner_id, const bool show_flag)
{
	if (!owner_id) {
		return false;
	}

	return BotDataRepository::SaveAllHelmAppearances(database, owner_id, show_flag);
}

bool BotDatabase::ToggleAllHelmAppearances(const uint32 owner_id)
{
	if (!owner_id) {
		return false;
	}

	return BotDataRepository::ToggleAllHelmAppearances(database, owner_id);
}

bool BotDatabase::SaveFollowDistance(const uint32 bot_id, const uint32 follow_distance)
{
	if (!bot_id || !follow_distance) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	e.follow_distance = follow_distance;

	return BotDataRepository::UpdateOne(database, e);
}

bool BotDatabase::SaveAllFollowDistances(const uint32 owner_id, const uint32 follow_distance)
{
	if (!owner_id || !follow_distance) {
		return false;
	}

	return BotDataRepository::SaveAllFollowDistances(database, owner_id, follow_distance);
}

bool BotDatabase::CreateCloneBot(const uint32 bot_id, const std::string& clone_name, uint32& clone_id)
{
	if (!bot_id || clone_name.empty()) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	e.bot_id = 0;
	e.name   = clone_name;

	e = BotDataRepository::InsertOne(database, e);

	if (!e.bot_id) {
		return false;
	}

	clone_id = e.bot_id;

	return true;
}

bool BotDatabase::CreateCloneBotInventory(const uint32 bot_id, const uint32 clone_id)
{
	if (!bot_id || !clone_id) {
		return false;
	}

	auto l = BotInventoriesRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {}",
			bot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	for (auto& e : l) {
		e.inventories_index = 0;
		e.bot_id            = clone_id;
	}

	return BotInventoriesRepository::InsertMany(database, l);
}

bool BotDatabase::SaveStopMeleeLevel(const uint32 bot_id, const uint8 sml_value)
{
	if (!bot_id) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	e.stop_melee_level = sml_value;

	return BotDataRepository::UpdateOne(database, e);
}

bool BotDatabase::LoadOwnerOptions(Client* c)
{
	if (!c || !c->CharacterID()) {
		return false;
	}

	const auto& l = BotOwnerOptionsRepository::GetWhere(
		database,
		fmt::format(
			"`owner_id` = {}",
			c->CharacterID()
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		c->SetBotOption(static_cast<Client::BotOwnerOption>(e.option_type), e.option_value);
	}

	return true;
}

bool BotDatabase::SaveOwnerOption(const uint32 owner_id, size_t type, const bool flag)
{
	if (!owner_id) {
		return false;
	}

	std::vector<Client::BotOwnerOption> l = {
		Client::booDeathMarquee,
		Client::booStatsUpdate,
		Client::booSpawnMessageClassSpecific,
		Client::booUnused,
		Client::booAutoDefend,
		Client::booBuffCounter,
		Client::booMonkWuMessage
	};

	if (
		std::find(
			l.begin(),
			l.end(),
			static_cast<Client::BotOwnerOption>(type)
		) != l.end()
	) {
		return BotOwnerOptionsRepository::ReplaceOne(
			database,
			BotOwnerOptionsRepository::BotOwnerOptions{
				.owner_id = owner_id,
				.option_type = static_cast<uint16_t>(type),
				.option_value = static_cast<uint16_t>(flag ? 1 : 0)
			}
		);
	}

	return false;
}

bool BotDatabase::SaveOwnerOption(const uint32 owner_id, const std::pair<size_t, size_t> type, const std::pair<bool, bool> flag)
{
	if (!owner_id) {
		return false;
	}

	std::vector<Client::BotOwnerOption> l = {
		Client::booSpawnMessageSay,
		Client::booSpawnMessageTell
	};

	auto e = BotOwnerOptionsRepository::NewEntity();

	std::vector<BotOwnerOptionsRepository::BotOwnerOptions> v;

	if (
		std::find(
			l.begin(),
			l.end(),
			static_cast<Client::BotOwnerOption>(type.first)
		) != l.end() &&
		std::find(
			l.begin(),
			l.end(),
			static_cast<Client::BotOwnerOption>(type.second)
		) != l.end()
	) {
		e.owner_id     = owner_id;
		e.option_type  = static_cast<uint16_t>(type.first);
		e.option_value = static_cast<uint16_t>(flag.first ? 1 : 0);

		v.emplace_back(e);

		e.option_type  = static_cast<uint16_t>(type.second);
		e.option_value = static_cast<uint16_t>(flag.second ? 1 : 0);

		v.emplace_back(e);

		return BotOwnerOptionsRepository::ReplaceMany(database, v);
	}

	return false;
}

bool BotDatabase::LoadGroupedBotsByGroupID(const uint32 owner_id, const uint32 group_id, std::list<uint32>& group_list)
{
	if (!group_id || !owner_id) {
		return false;
	}

	const auto& l = GroupIdRepository::GetWhere(
		database,
		fmt::format(
			"`group_id` = {} AND `bot_id` != 0 AND `name` IN (SELECT `name` FROM `bot_data` WHERE `owner_id` = {})",
			group_id,
			owner_id
		)
	);

	for (const auto& e : l) {
		group_list.emplace_back(e.bot_id);
	}

	return true;
}

bool BotDatabase::LoadHealRotationIDByBotID(const uint32 bot_id, uint32& hr_index)
{
	if (!bot_id) {
		return false;
	}

	const auto& l = BotHealRotationsRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` = {} LIMIT 1",
			bot_id
		)
	);

	if (l.empty()) {
		return true;
	}

	auto e = l.front();

	hr_index = e.heal_rotation_index;

	return true;
}

bool BotDatabase::LoadHealRotation(Bot* hr_member, std::list<uint32>& member_list, std::list<std::string>& target_list, bool& load_flag, bool& member_fail, bool& target_fail)
{
	if (!hr_member) {
		return false;
	}

	uint32 hr_index = 0;

	if (!LoadHealRotationIDByBotID(hr_member->GetBotID(), hr_index)) {
		return false;
	}

	if (!hr_index) {
		return true;
	}

	if (!hr_member->IsHealRotationMember()) {
		return false;
	}

	const auto& e = BotHealRotationsRepository::FindOne(database, hr_index);

	if (!e.heal_rotation_index) {
		return false;
	}

	auto m = (*hr_member->MemberOfHealRotation());

	m->SetIntervalS(e.interval_);
	m->SetFastHeals(e.fast_heals);
	m->SetAdaptiveTargeting(e.adaptive_targeting);
	m->SetCastingOverride(e.casting_override);

	m->SetArmorTypeSafeHPRatio(ARMOR_TYPE_UNKNOWN, e.safe_hp_base);
	m->SetArmorTypeSafeHPRatio(ARMOR_TYPE_CLOTH, e.safe_hp_cloth);
	m->SetArmorTypeSafeHPRatio(ARMOR_TYPE_LEATHER, e.safe_hp_leather);
	m->SetArmorTypeSafeHPRatio(ARMOR_TYPE_CHAIN, e.safe_hp_chain);
	m->SetArmorTypeSafeHPRatio(ARMOR_TYPE_PLATE, e.safe_hp_plate);

	m->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_UNKNOWN, e.critical_hp_base);
	m->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_CLOTH, e.critical_hp_cloth);
	m->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_LEATHER, e.critical_hp_leather);
	m->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_CHAIN, e.critical_hp_chain);
	m->SetArmorTypeCriticalHPRatio(ARMOR_TYPE_PLATE, e.critical_hp_plate);

	load_flag = true;

	if (!LoadHealRotationMembers(hr_index, member_list)) {
		member_fail = true;
	}

	if (!LoadHealRotationTargets(hr_index, target_list)) {
		target_fail = true;
	}

	return true;
}

bool BotDatabase::LoadHealRotationMembers(const uint32 hr_index, std::list<uint32>& member_list)
{
	if (!hr_index) {
		return false;
	}

	const auto& l = BotHealRotationMembersRepository::GetWhere(
		database,
		fmt::format(
			"`heal_rotation_index` = {}",
			hr_index
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		member_list.push_back(e.bot_id);
	}

	return true;
}

bool BotDatabase::LoadHealRotationTargets(const uint32 hr_index, std::list<std::string>& target_list)
{
	if (!hr_index) {
		return false;
	}

	const auto& l = BotHealRotationTargetsRepository::GetWhere(
		database,
		fmt::format(
			"`heal_rotation_index` = {}",
			hr_index
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		target_list.push_back(e.target_name);
	}

	return true;
}

bool BotDatabase::SaveHealRotation(Bot* hr_member, bool& member_fail, bool& target_fail)
{
	if (
		!hr_member ||
		!DeleteHealRotation(hr_member->GetBotID()) ||
		!hr_member->IsHealRotationMember()
	) {
		return false;
	}

	auto m = (*hr_member->MemberOfHealRotation());

	auto e = BotHealRotationsRepository::NewEntity();

	e.bot_id             = hr_member->GetBotID();
	e.interval_          = m->IntervalS();
	e.fast_heals         = m->FastHeals();
	e.adaptive_targeting = m->AdaptiveTargeting();
	e.casting_override   = m->CastingOverride();

	e.safe_hp_base    = m->ArmorTypeSafeHPRatio(ARMOR_TYPE_UNKNOWN);
	e.safe_hp_cloth   = m->ArmorTypeSafeHPRatio(ARMOR_TYPE_CLOTH);
	e.safe_hp_leather = m->ArmorTypeSafeHPRatio(ARMOR_TYPE_LEATHER);
	e.safe_hp_chain   = m->ArmorTypeSafeHPRatio(ARMOR_TYPE_CHAIN);
	e.safe_hp_plate   = m->ArmorTypeSafeHPRatio(ARMOR_TYPE_PLATE);

	e.critical_hp_base    = m->ArmorTypeCriticalHPRatio(ARMOR_TYPE_UNKNOWN);
	e.critical_hp_cloth   = m->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CLOTH);
	e.critical_hp_leather = m->ArmorTypeCriticalHPRatio(ARMOR_TYPE_LEATHER);
	e.critical_hp_chain   = m->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CHAIN);
	e.critical_hp_plate   = m->ArmorTypeCriticalHPRatio(ARMOR_TYPE_PLATE);

	e = BotHealRotationsRepository::InsertOne(database, e);

	if (!e.heal_rotation_index) {
		return false;
	}

	std::list<Bot*>* ml = m->MemberList();

	auto re = BotHealRotationMembersRepository::NewEntity();

	re.heal_rotation_index = e.heal_rotation_index;

	std::vector<BotHealRotationMembersRepository::BotHealRotationMembers> rv;

	for (auto m : *ml) {
		if (!m) {
			continue;
		}

		re.bot_id = m->GetBotID();

		rv.emplace_back(re);
	}

	const int inserted_members = BotHealRotationMembersRepository::InsertMany(database, rv);

	if (!inserted_members) {
		member_fail = true;
	}

	std::list<Mob*>* tl = m->TargetList();

	auto te = BotHealRotationTargetsRepository::NewEntity();

	te.heal_rotation_index = e.heal_rotation_index;

	std::vector<BotHealRotationTargetsRepository::BotHealRotationTargets> tv;

	for (auto m : *tl) {
		if (!m) {
			continue;
		}

		te.target_name = m->GetCleanName();

		tv.emplace_back(te);
	}

	const int inserted_targets = BotHealRotationTargetsRepository::InsertMany(database, tv);

	if (!inserted_targets) {
		target_fail = true;
	}

	return true;
}

bool BotDatabase::DeleteHealRotation(const uint32 creator_id)
{
	if (!creator_id) {
		return false;
	}

	uint32 hr_index = 0;

	if (!LoadHealRotationIDByBotID(creator_id, hr_index)) {
		return false;
	}

	if (!hr_index) {
		return true;
	}

	BotHealRotationTargetsRepository::DeleteWhere(
		database,
		fmt::format(
			"`heal_rotation_index` = {}",
			hr_index
		)
	);

	BotHealRotationMembersRepository::DeleteWhere(
		database,
		fmt::format(
			"`heal_rotation_index` = {}",
			hr_index
		)
	);

	BotHealRotationsRepository::DeleteWhere(
		database,
		fmt::format(
			"`heal_rotation_index` = {}",
			hr_index
		)
	);

	return true;
}

bool BotDatabase::DeleteAllHealRotations(const uint32 owner_id)
{
	if (!owner_id) {
		return false;
	}

	const auto& l = BotHealRotationsRepository::GetWhere(
		database,
		fmt::format(
			"`bot_id` IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = {})",
			owner_id
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		DeleteHealRotation(e.bot_id);
	}

	return true;
}


/* Bot miscellaneous functions   */
uint8 BotDatabase::GetSpellCastingChance(uint8 spell_type_index, uint8 class_index, uint8 stance_index, uint8 conditional_index) // class_index is 0-based
{
	if (
		spell_type_index >= Bot::SPELL_TYPE_COUNT ||
		class_index >= Class::PLAYER_CLASS_COUNT ||
		stance_index >= EQ::constants::STANCE_TYPE_COUNT ||
		conditional_index >= cntHSND
	) {
		return 0;
	}

	return Bot::spell_casting_chances[spell_type_index][class_index][stance_index][conditional_index];
}

uint32 BotDatabase::GetRaceClassBitmask(uint32 bot_race)
{
	const auto& e = BotCreateCombinationsRepository::FindOne(database, bot_race);

	return e.race ? e.classes : 0;
}

bool BotDatabase::SaveExpansionBitmask(const uint32 bot_id, const int expansion_bitmask)
{
	if (!bot_id) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	if (!e.bot_id) {
		return false;
	}

	e.expansion_bitmask = expansion_bitmask;

	return BotDataRepository::UpdateOne(database, e);
}

bool BotDatabase::SaveEnforceSpellSetting(const uint32 bot_id, const bool enforce_spell_setting)
{
	if (!bot_id) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	if (!e.bot_id) {
		return false;
	}

	e.enforce_spell_settings = enforce_spell_setting ? 1 : 0;

	return BotDataRepository::UpdateOne(database, e);
}

bool BotDatabase::SaveBotArcherSetting(const uint32 bot_id, const bool bot_archer_setting)
{
	if (!bot_id) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	if (!e.bot_id) {
		return false;
	}

	e.archery_setting = bot_archer_setting ? 1 : 0;

	return BotDataRepository::UpdateOne(database, e);
}

bool BotDatabase::SaveBotCasterRange(const uint32 bot_id, const uint32 bot_caster_range_value)
{
	if (!bot_id) {
		return false;
	}

	auto e = BotDataRepository::FindOne(database, bot_id);

	if (!e.bot_id) {
		return false;
	}

	e.caster_range = bot_caster_range_value;

	return BotDataRepository::UpdateOne(database, e);
}

const uint8 BotDatabase::GetBotClassByID(const uint32 bot_id)
{
	const auto& e = BotDataRepository::FindOne(database, bot_id);

	return e.bot_id ? e.class_ : Class::None;
}

const uint8 BotDatabase::GetBotGenderByID(const uint32 bot_id)
{
	const auto& e = BotDataRepository::FindOne(database, bot_id);

	return e.bot_id ? e.gender : Gender::Neuter;
}

std::vector<uint32> BotDatabase::GetBotIDsByCharacterID(const uint32 character_id, uint8 class_id)
{
	std::vector<uint32> v;

	const auto& l = BotDataRepository::GetWhere(
		database,
		fmt::format(
			"`owner_id` = {}{}",
			character_id,
			(
				class_id ?
				fmt::format(
					" AND `class` = {}",
					class_id
				) :
				""
			)
		)
	);

	for (const auto& e : l) {
		v.push_back(e.bot_id);
	}

	return v;
}

const uint8 BotDatabase::GetBotLevelByID(const uint32 bot_id)
{
	const auto& e = BotDataRepository::FindOne(database, bot_id);

	return e.bot_id ? e.level : 0;
}

const std::string BotDatabase::GetBotNameByID(const uint32 bot_id)
{
	const auto& e = BotDataRepository::FindOne(database, bot_id);

	return e.bot_id ? e.name : std::string();
}

const uint16 BotDatabase::GetBotRaceByID(const uint32 bot_id)
{
	const auto& e = BotDataRepository::FindOne(database, bot_id);

	return e.bot_id ? e.race : Race::Doug;
}

const int BotDatabase::GetBotExtraHasteByID(const uint32 bot_id)
{
	const auto& e = BotDataRepository::FindOne(database, bot_id);

	return e.bot_id ? e.extra_haste : 0;
}
