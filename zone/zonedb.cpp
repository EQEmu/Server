
#include "../common/eqemu_logsys.h"
#include "../common/extprofile.h"
#include "../common/rulesys.h"
#include "../common/strings.h"

#include "client.h"
#include "corpse.h"
#include "groups.h"
#include "merc.h"
#include "zone.h"
#include "zonedb.h"
#include "aura.h"
#include "../common/repositories/blocked_spells_repository.h"
#include "../common/repositories/character_tribute_repository.h"
#include "../common/repositories/character_disciplines_repository.h"
#include "../common/repositories/npc_types_repository.h"
#include "../common/repositories/character_bind_repository.h"
#include "../common/repositories/character_pet_buffs_repository.h"
#include "../common/repositories/character_pet_inventory_repository.h"
#include "../common/repositories/character_pet_info_repository.h"
#include "../common/repositories/character_buffs_repository.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/repositories/spawn2_disabled_repository.h"

#include <ctime>
#include <iostream>
#include <fmt/format.h>

extern Zone* zone;

ZoneDatabase database;
ZoneDatabase content_db;

ZoneDatabase::ZoneDatabase()
: SharedDatabase()
{
	ZDBInitVars();
}

ZoneDatabase::ZoneDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
: SharedDatabase(host, user, passwd, database, port)
{
	ZDBInitVars();
}

void ZoneDatabase::ZDBInitVars() {
	npc_spellseffects_cache = 0;
	npc_spellseffects_loadtried = 0;
	max_faction = 0;
	faction_array = nullptr;
}

ZoneDatabase::~ZoneDatabase() {
	if (npc_spellseffects_cache) {
		for (int x = 0; x <= npc_spellseffects_maxid; x++) {
			safe_delete_array(npc_spellseffects_cache[x]);
		}
		safe_delete_array(npc_spellseffects_cache);
	}
	safe_delete_array(npc_spellseffects_loadtried);

	if (faction_array != nullptr) {
		for (int x = 0; x <= max_faction; x++) {
			if (faction_array[x] != 0)
				safe_delete(faction_array[x]);
		}
		safe_delete_array(faction_array);
	}
}

bool ZoneDatabase::SaveZoneCFG(uint32 zoneid, uint16 instance_version, NewZone_Struct* zd) {
	std::string query = fmt::format(
		"UPDATE zone SET underworld = {:.2f}, minclip = {:.2f}, "
		"maxclip = {:.2f}, fog_minclip = {:.2f}, fog_maxclip = {:.2f}, "
		"fog_blue = {}, fog_red = {}, fog_green = {}, "
		"sky = {}, ztype = {}, zone_exp_multiplier = {:.2f}, "
		"safe_x = {:.2f}, safe_y = {:.2f}, safe_z = {:.2f} "
		"WHERE zoneidnumber = {} AND version = {}",
		zd->underworld,
		zd->minclip,
		zd->maxclip,
		zd->fog_minclip[0],
		zd->fog_maxclip[0],
		zd->fog_blue[0],
		zd->fog_red[0],
		zd->fog_green[0],
		zd->sky,
		zd->ztype,
		zd->zone_exp_multiplier,
		zd->safe_x,
		zd->safe_y,
		zd->safe_z,
		zoneid,
		instance_version
	);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        return false;
	}

	return true;
}

void ZoneDatabase::UpdateRespawnTime(uint32 spawn2_id, uint16 instance_id, uint32 time_left)
{

	timeval tv;
	gettimeofday(&tv, nullptr);
	uint32 current_time = tv.tv_sec;

	/*	If we pass timeleft as 0 that means we clear from respawn time
			otherwise we update with a REPLACE INTO
	*/

	if(time_left == 0) {
        std::string query = StringFormat("DELETE FROM `respawn_times` WHERE `id` = %u AND `instance_id` = %u", spawn2_id, instance_id);
        QueryDatabase(query);
		return;
	}

    std::string query = StringFormat(
		"REPLACE INTO `respawn_times` "
		"(id, "
		"start, "
		"duration, "
		"instance_id) "
		"VALUES "
		"(%u, "
		"%u, "
		"%u, "
		"%u)",
		spawn2_id,
		current_time,
		time_left,
		instance_id
	);
    QueryDatabase(query);

	return;
}

//Gets the respawn time left in the database for the current spawn id
uint32 ZoneDatabase::GetSpawnTimeLeft(uint32 id, uint16 instance_id)
{
	std::string query = StringFormat("SELECT start, duration FROM respawn_times "
                                    "WHERE id = %lu AND instance_id = %lu",
                                    (unsigned long)id, (unsigned long)zone->GetInstanceID());
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return 0;
    }

    if (results.RowCount() != 1)
        return 0;

    auto& row = results.begin();

    timeval tv;
    gettimeofday(&tv, nullptr);
    uint32 resStart = Strings::ToInt(row[0]);
    uint32 resDuration = Strings::ToInt(row[1]);

    //compare our values to current time
    if((resStart + resDuration) <= tv.tv_sec) {
        //our current time was expired
        return 0;
    }

    //we still have time left on this timer
    return ((resStart + resDuration) - tv.tv_sec);

}

void ZoneDatabase::UpdateSpawn2Status(uint32 id, uint8 new_status, uint32 instance_id)
{
	auto spawns = Spawn2DisabledRepository::GetWhere(
		*this,
		fmt::format("spawn2_id = {} and instance_id = {}", id, instance_id)
	);
	if (!spawns.empty()) {
		auto spawn = spawns[0];
		// 1 = enabled 0 = disabled
		spawn.disabled    = new_status ? 0 : 1;
		spawn.instance_id = instance_id;
		Spawn2DisabledRepository::UpdateOne(*this, spawn);
		return;
	}

	auto spawn = Spawn2DisabledRepository::NewEntity();
	spawn.spawn2_id   = id;
	spawn.instance_id = instance_id;
	spawn.disabled    = new_status ? 0 : 1;
	Spawn2DisabledRepository::InsertOne(*this, spawn);
}

bool ZoneDatabase::SetSpecialAttkFlag(uint8 id, const char* flag) {

	std::string query = StringFormat("UPDATE npc_types SET npcspecialattks='%s' WHERE id = %i;", flag, id);
    auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return results.RowsAffected() != 0;
}

// Load child objects for a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::LoadWorldContainer(uint32 parentid, EQ::ItemInstance* container)
{
	if (!container) {
		LogError("Programming error: LoadWorldContainer passed nullptr pointer");
		return;
	}

	std::string query   = StringFormat(
		"SELECT bagidx, itemid, charges, augslot1, augslot2, augslot3, augslot4, augslot5, augslot6 "
		"FROM object_contents WHERE parentid = %i", parentid
	);

	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in DB::LoadWorldContainer: [{}]", results.ErrorMessage().c_str());
		return;
	}

    for (auto& row = results.begin(); row != results.end(); ++row) {
        uint8 index = (uint8)Strings::ToInt(row[0]);
        uint32 item_id = (uint32)Strings::ToInt(row[1]);
        int8 charges = (int8)Strings::ToInt(row[2]);
		uint32 aug[EQ::invaug::SOCKET_COUNT];
        aug[0] = (uint32)Strings::ToInt(row[3]);
        aug[1] = (uint32)Strings::ToInt(row[4]);
        aug[2] = (uint32)Strings::ToInt(row[5]);
        aug[3] = (uint32)Strings::ToInt(row[6]);
        aug[4] = (uint32)Strings::ToInt(row[7]);
		aug[5] = (uint32)Strings::ToInt(row[8]);

        EQ::ItemInstance* inst = database.CreateItem(item_id, charges);
		if (inst && inst->GetItem()->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++)
                if (aug[i])
                    inst->PutAugment(&database, i, aug[i]);
            // Put item inside world container
            container->PutItem(index, *inst);
        }
		safe_delete(inst);
    }

}

// Save child objects for a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::SaveWorldContainer(uint32 zone_id, uint32 parent_id, const EQ::ItemInstance* container)
{
	// Since state is not saved for each world container action, we'll just delete
	// all and save from scratch .. we may come back later to optimize
	if (!container)
		return;

	//Delete all items from container
	DeleteWorldContainer(parent_id,zone_id);

	// Save all 10 items, if they exist
	for (uint8 index = EQ::invbag::SLOT_BEGIN; index <= EQ::invbag::SLOT_END; index++) {

		EQ::ItemInstance* inst = container->GetItem(index);
		if (!inst)
            continue;

        uint32 item_id = inst->GetItem()->ID;
		uint32 augslot[EQ::invaug::SOCKET_COUNT] = { 0, 0, 0, 0, 0, 0 };

		if (inst->IsType(EQ::item::ItemClassCommon)) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
                EQ::ItemInstance *auginst=inst->GetAugment(i);
                augslot[i]=(auginst && auginst->GetItem()) ? auginst->GetItem()->ID : 0;
            }
        }

		std::string query   = StringFormat(
			"REPLACE INTO object_contents "
			"(zoneid, parentid, bagidx, itemid, charges, "
			"augslot1, augslot2, augslot3, augslot4, augslot5, augslot6, droptime) "
			"VALUES (%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, now())",
			zone_id, parent_id, index, item_id, inst->GetCharges(),
			augslot[0], augslot[1], augslot[2], augslot[3], augslot[4], augslot[5]
		);

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			LogError("Error in ZoneDatabase::SaveWorldContainer: [{}]", results.ErrorMessage().c_str());
		}

	}

}

// Remove all child objects inside a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::DeleteWorldContainer(uint32 parent_id, uint32 zone_id)
{
	std::string query = StringFormat(
		"DELETE FROM object_contents WHERE parentid = %i AND zoneid = %i",
		parent_id,
		zone_id
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in ZoneDatabase::DeleteWorldContainer: [{}]", results.ErrorMessage().c_str());
	}

}

Trader_Struct* ZoneDatabase::LoadTraderItem(uint32 char_id)
{
	auto loadti = new Trader_Struct;
	memset(loadti,0,sizeof(Trader_Struct));

	std::string query = StringFormat("SELECT * FROM trader WHERE char_id = %i ORDER BY slot_id LIMIT 80", char_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogTrading("Failed to load trader information!\n");
		return loadti;
	}

	loadti->Code = BazaarTrader_ShowItems;
	for (auto& row = results.begin(); row != results.end(); ++row) {
		if (Strings::ToInt(row[5]) >= 80 || Strings::ToInt(row[4]) < 0) {
			LogTrading("Bad Slot number when trying to load trader information!\n");
			continue;
		}

		loadti->Items[Strings::ToInt(row[5])] = Strings::ToInt(row[1]);
		loadti->ItemCost[Strings::ToInt(row[5])] = Strings::ToInt(row[4]);
	}
	return loadti;
}

TraderCharges_Struct* ZoneDatabase::LoadTraderItemWithCharges(uint32 char_id)
{
	auto loadti = new TraderCharges_Struct;
	memset(loadti,0,sizeof(TraderCharges_Struct));

	std::string query = StringFormat("SELECT * FROM trader WHERE char_id=%i ORDER BY slot_id LIMIT 80", char_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogTrading("Failed to load trader information!\n");
		return loadti;
	}

	for (auto& row = results.begin(); row != results.end(); ++row) {
		if (Strings::ToInt(row[5]) >= 80 || Strings::ToInt(row[5]) < 0) {
			LogTrading("Bad Slot number when trying to load trader information!\n");
			continue;
		}

		loadti->ItemID[Strings::ToInt(row[5])] = Strings::ToInt(row[1]);
		loadti->SerialNumber[Strings::ToInt(row[5])] = Strings::ToInt(row[2]);
		loadti->Charges[Strings::ToInt(row[5])] = Strings::ToInt(row[3]);
		loadti->ItemCost[Strings::ToInt(row[5])] = Strings::ToInt(row[4]);
	}
	return loadti;
}

EQ::ItemInstance* ZoneDatabase::LoadSingleTraderItem(uint32 CharID, int SerialNumber) {
	std::string query = StringFormat("SELECT * FROM trader WHERE char_id = %i AND serialnumber = %i "
                                    "ORDER BY slot_id LIMIT 80", CharID, SerialNumber);
    auto results = QueryDatabase(query);
    if (!results.Success())
        return nullptr;

	if (results.RowCount() == 0) {
    LogTrading("Bad result from query\n"); fflush(stdout);
        return nullptr;
    }

    auto& row = results.begin();

    int ItemID = Strings::ToInt(row[1]);
	int Charges = Strings::ToInt(row[3]);
	int Cost = Strings::ToInt(row[4]);

	const EQ::ItemData *item = database.GetItem(ItemID);

	if(!item) {
		LogTrading("Unable to create item\n");
		fflush(stdout);
		return nullptr;
	}

    if (item->NoDrop == 0)
        return nullptr;

    EQ::ItemInstance* inst = database.CreateItem(item);
	if(!inst) {
		LogTrading("Unable to create item instance\n");
		fflush(stdout);
		return nullptr;
	}

    inst->SetCharges(Charges);
	inst->SetSerialNumber(SerialNumber);
	inst->SetMerchantSlot(SerialNumber);
	inst->SetPrice(Cost);

	if(inst->IsStackable())
		inst->SetMerchantCount(Charges);

	return inst;
}

void ZoneDatabase::SaveTraderItem(uint32 CharID, uint32 ItemID, uint32 SerialNumber, int32 Charges, uint32 ItemCost, uint8 Slot){

	std::string query = StringFormat("REPLACE INTO trader VALUES(%i, %i, %i, %i, %i, %i)",
                                    CharID, ItemID, SerialNumber, Charges, ItemCost, Slot);
    auto results = QueryDatabase(query);
    if (!results.Success())
        LogDebug("[CLIENT] Failed to save trader item: [{}] for char_id: [{}], the error was: [{}]\n", ItemID, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::UpdateTraderItemCharges(int CharID, uint32 SerialNumber, int32 Charges) {
	LogTrading("ZoneDatabase::UpdateTraderItemCharges([{}], [{}], [{}])", CharID, SerialNumber, Charges);

	std::string query = StringFormat("UPDATE trader SET charges = %i WHERE char_id = %i AND serialnumber = %i",
                                    Charges, CharID, SerialNumber);
    auto results = QueryDatabase(query);
    if (!results.Success())
		LogDebug("[CLIENT] Failed to update charges for trader item: [{}] for char_id: [{}], the error was: [{}]\n", SerialNumber, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::UpdateTraderItemPrice(int CharID, uint32 ItemID, uint32 Charges, uint32 NewPrice) {

	LogTrading("ZoneDatabase::UpdateTraderPrice([{}], [{}], [{}], [{}])", CharID, ItemID, Charges, NewPrice);

	const EQ::ItemData *item = database.GetItem(ItemID);

	if(!item)
		return;

	if(NewPrice == 0) {
		LogTrading("Removing Trader items from the DB for CharID [{}], ItemID [{}]", CharID, ItemID);

        std::string query = StringFormat("DELETE FROM trader WHERE char_id = %i AND item_id = %i",CharID, ItemID);
        auto results = QueryDatabase(query);
        if (!results.Success())
			LogDebug("[CLIENT] Failed to remove trader item(s): [{}] for char_id: [{}], the error was: [{}]\n", ItemID, CharID, results.ErrorMessage().c_str());

		return;
	}

    if(!item->Stackable) {
        std::string query = StringFormat("UPDATE trader SET item_cost = %i "
                                        "WHERE char_id = %i AND item_id = %i AND charges=%i",
                                        NewPrice, CharID, ItemID, Charges);
        auto results = QueryDatabase(query);
        if (!results.Success())
            LogDebug("[CLIENT] Failed to update price for trader item: [{}] for char_id: [{}], the error was: [{}]\n", ItemID, CharID, results.ErrorMessage().c_str());

        return;
    }

    std::string query = StringFormat("UPDATE trader SET item_cost = %i "
                                    "WHERE char_id = %i AND item_id = %i",
                                    NewPrice, CharID, ItemID);
    auto results = QueryDatabase(query);
    if (!results.Success())
            LogDebug("[CLIENT] Failed to update price for trader item: [{}] for char_id: [{}], the error was: [{}]\n", ItemID, CharID, results.ErrorMessage().c_str());
}

void ZoneDatabase::DeleteTraderItem(uint32 char_id){

	if(char_id==0) {
        const std::string query = "DELETE FROM trader";
        auto results = QueryDatabase(query);
		if (!results.Success())
			LogDebug("[CLIENT] Failed to delete all trader items data, the error was: [{}]\n", results.ErrorMessage().c_str());

        return;
	}

	std::string query = StringFormat("DELETE FROM trader WHERE char_id = %i", char_id);
	auto results = QueryDatabase(query);
    if (!results.Success())
        LogDebug("[CLIENT] Failed to delete trader item data for char_id: [{}], the error was: [{}]\n", char_id, results.ErrorMessage().c_str());

}
void ZoneDatabase::DeleteTraderItem(uint32 CharID,uint16 SlotID) {

	std::string query = StringFormat("DELETE FROM trader WHERE char_id = %i And slot_id = %i", CharID, SlotID);
	auto results = QueryDatabase(query);
	if (!results.Success())
		LogDebug("[CLIENT] Failed to delete trader item data for char_id: [{}], the error was: [{}]\n",CharID, results.ErrorMessage().c_str());
}

void ZoneDatabase::DeleteBuyLines(uint32 CharID) {

	if(CharID==0) {
        const std::string query = "DELETE FROM buyer";
		auto results = QueryDatabase(query);
        if (!results.Success())
			LogDebug("[CLIENT] Failed to delete all buyer items data, the error was: [{}]\n",results.ErrorMessage().c_str());

        return;
	}

    std::string query = StringFormat("DELETE FROM buyer WHERE charid = %i", CharID);
	auto results = QueryDatabase(query);
	if (!results.Success())
			LogDebug("[CLIENT] Failed to delete buyer item data for charid: [{}], the error was: [{}]\n",CharID,results.ErrorMessage().c_str());

}

void ZoneDatabase::AddBuyLine(uint32 CharID, uint32 BuySlot, uint32 ItemID, const char* ItemName, uint32 Quantity, uint32 Price) {
	std::string query = StringFormat("REPLACE INTO buyer VALUES(%i, %i, %i, \"%s\", %i, %i)",
                                    CharID, BuySlot, ItemID, ItemName, Quantity, Price);
    auto results = QueryDatabase(query);
	if (!results.Success())
		LogDebug("[CLIENT] Failed to save buline item: [{}] for char_id: [{}], the error was: [{}]\n", ItemID, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::RemoveBuyLine(uint32 CharID, uint32 BuySlot) {
	std::string query = StringFormat("DELETE FROM buyer WHERE charid = %i AND buyslot = %i", CharID, BuySlot);
    auto results = QueryDatabase(query);
	if (!results.Success())
		LogDebug("[CLIENT] Failed to delete buyslot [{}] for charid: [{}], the error was: [{}]\n", BuySlot, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::UpdateBuyLine(uint32 CharID, uint32 BuySlot, uint32 Quantity) {
	if(Quantity <= 0) {
		RemoveBuyLine(CharID, BuySlot);
		return;
	}

	std::string query = StringFormat("UPDATE buyer SET quantity = %i WHERE charid = %i AND buyslot = %i", Quantity, CharID, BuySlot);
    auto results = QueryDatabase(query);
	if (!results.Success())
		LogDebug("[CLIENT] Failed to update quantity in buyslot [{}] for charid: [{}], the error was: [{}]\n", BuySlot, CharID, results.ErrorMessage().c_str());

}

#define StructDist(in, f1, f2) (uint32(&in->f2)-uint32(&in->f1))

bool ZoneDatabase::LoadCharacterData(uint32 character_id, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp){
	std::string query = StringFormat(
		"SELECT                     "
		"`name`,                    "
		"last_name,                 "
		"gender,                    "
		"race,                      "
		"class,                     "
		"`level`,                   "
		"deity,                     "
		"birthday,                  "
		"last_login,                "
		"time_played,               "
		"pvp_status,                "
		"level2,                    "
		"anon,                      "
		"gm,                        "
		"intoxication,              "
		"hair_color,                "
		"beard_color,               "
		"eye_color_1,               "
		"eye_color_2,               "
		"hair_style,                "
		"beard,                     "
		"ability_time_seconds,      "
		"ability_number,            "
		"ability_time_minutes,      "
		"ability_time_hours,        "
		"title,                     "
		"suffix,                    "
		"exp,                       "
		"points,                    "
		"mana,                      "
		"cur_hp,                    "
		"str,                       "
		"sta,                       "
		"cha,                       "
		"dex,                       "
		"`int`,                     "
		"agi,                       "
		"wis,                       "
		"face,                      "
		"y,                         "
		"x,                         "
		"z,                         "
		"heading,                   "
		"pvp2,                      "
		"pvp_type,                  "
		"autosplit_enabled,         "
		"zone_change_count,         "
		"drakkin_heritage,          "
		"drakkin_tattoo,            "
		"drakkin_details,           "
		"toxicity,                  "
		"hunger_level,              "
		"thirst_level,              "
		"ability_up,                "
		"zone_id,                   "
		"zone_instance,             "
		"leadership_exp_on,         "
		"ldon_points_guk,           "
		"ldon_points_mir,           "
		"ldon_points_mmc,           "
		"ldon_points_ruj,           "
		"ldon_points_tak,           "
		"ldon_points_available,     "
		"tribute_time_remaining,    "
		"show_helm,                 "
		"career_tribute_points,     "
		"tribute_points,            "
		"tribute_active,            "
		"endurance,                 "
		"group_leadership_exp,      "
		"raid_leadership_exp,       "
		"group_leadership_points,   "
		"raid_leadership_points,    "
		"air_remaining,             "
		"pvp_kills,                 "
		"pvp_deaths,                "
		"pvp_current_points,        "
		"pvp_career_points,         "
		"pvp_best_kill_streak,      "
		"pvp_worst_death_streak,    "
		"pvp_current_kill_streak,   "
		"aa_points_spent,           "
		"aa_exp,                    "
		"aa_points,                 "
		"group_auto_consent,        "
		"raid_auto_consent,         "
		"guild_auto_consent,        "
		"RestTimer,                 "
		"`e_aa_effects`,			"
		"`e_percent_to_aa`,			"
		"`e_expended_aa_spent`,		"
		"`e_last_invsnapshot`		"
		"FROM                       "
		"character_data             "
		"WHERE `id` = %i         ", character_id);
	auto results = database.QueryDatabase(query); int r = 0;
	for (auto& row = results.begin(); row != results.end(); ++row) {
		strcpy(pp->name, row[r]); r++;											 // "`name`,                    "
		strcpy(pp->last_name, row[r]); r++;										 // "last_name,                 "
		pp->gender = Strings::ToInt(row[r]); r++;											 // "gender,                    "
		pp->race = Strings::ToInt(row[r]); r++;											 // "race,                      "
		pp->class_ = Strings::ToInt(row[r]); r++;											 // "class,                     "
		pp->level = Strings::ToInt(row[r]); r++;											 // "`level`,                   "
		pp->deity = Strings::ToInt(row[r]); r++;											 // "deity,                     "
		pp->birthday = Strings::ToInt(row[r]); r++;										 // "birthday,                  "
		pp->lastlogin = Strings::ToInt(row[r]); r++;										 // "last_login,                "
		pp->timePlayedMin = Strings::ToInt(row[r]); r++;									 // "time_played,               "
		pp->pvp = Strings::ToInt(row[r]); r++;											 // "pvp_status,                "
		pp->level2 = Strings::ToInt(row[r]); r++;											 // "level2,                    "
		pp->anon = Strings::ToInt(row[r]); r++;											 // "anon,                      "
		pp->gm = Strings::ToInt(row[r]); r++;												 // "gm,                        "
		pp->intoxication = Strings::ToInt(row[r]); r++;									 // "intoxication,              "
		pp->haircolor = Strings::ToInt(row[r]); r++;										 // "hair_color,                "
		pp->beardcolor = Strings::ToInt(row[r]); r++;										 // "beard_color,               "
		pp->eyecolor1 = Strings::ToInt(row[r]); r++;										 // "eye_color_1,               "
		pp->eyecolor2 = Strings::ToInt(row[r]); r++;										 // "eye_color_2,               "
		pp->hairstyle = Strings::ToInt(row[r]); r++;										 // "hair_style,                "
		pp->beard = Strings::ToInt(row[r]); r++;											 // "beard,                     "
		pp->ability_time_seconds = Strings::ToInt(row[r]); r++;							 // "ability_time_seconds,      "
		pp->ability_number = Strings::ToInt(row[r]); r++;									 // "ability_number,            "
		pp->ability_time_minutes = Strings::ToInt(row[r]); r++;							 // "ability_time_minutes,      "
		pp->ability_time_hours = Strings::ToInt(row[r]); r++;								 // "ability_time_hours,        "
		strcpy(pp->title, row[r]); r++;											 // "title,                     "
		strcpy(pp->suffix, row[r]); r++;										 // "suffix,                    "
		pp->exp = Strings::ToInt(row[r]); r++;											 // "exp,                       "
		pp->points = Strings::ToInt(row[r]); r++;											 // "points,                    "
		pp->mana = Strings::ToInt(row[r]); r++;											 // "mana,                      "
		pp->cur_hp = Strings::ToInt(row[r]); r++;											 // "cur_hp,                    "
		pp->STR = Strings::ToInt(row[r]); r++;											 // "str,                       "
		pp->STA = Strings::ToInt(row[r]); r++;											 // "sta,                       "
		pp->CHA = Strings::ToInt(row[r]); r++;											 // "cha,                       "
		pp->DEX = Strings::ToInt(row[r]); r++;											 // "dex,                       "
		pp->INT = Strings::ToInt(row[r]); r++;											 // "`int`,                     "
		pp->AGI = Strings::ToInt(row[r]); r++;											 // "agi,                       "
		pp->WIS = Strings::ToInt(row[r]); r++;											 // "wis,                       "
		pp->face = Strings::ToInt(row[r]); r++;											 // "face,                      "
		pp->y = Strings::ToFloat(row[r]); r++;												 // "y,                         "
		pp->x = Strings::ToFloat(row[r]); r++;												 // "x,                         "
		pp->z = Strings::ToFloat(row[r]); r++;												 // "z,                         "
		pp->heading = Strings::ToFloat(row[r]); r++;										 // "heading,                   "
		pp->pvp2 = Strings::ToInt(row[r]); r++;											 // "pvp2,                      "
		pp->pvptype = Strings::ToInt(row[r]); r++;										 // "pvp_type,                  "
		pp->autosplit = Strings::ToInt(row[r]); r++;										 // "autosplit_enabled,         "
		pp->zone_change_count = Strings::ToInt(row[r]); r++;								 // "zone_change_count,         "
		pp->drakkin_heritage = Strings::ToInt(row[r]); r++;								 // "drakkin_heritage,          "
		pp->drakkin_tattoo = Strings::ToInt(row[r]); r++;									 // "drakkin_tattoo,            "
		pp->drakkin_details = Strings::ToInt(row[r]); r++;								 // "drakkin_details,           "
		pp->toxicity = Strings::ToInt(row[r]); r++;										 // "toxicity,                  "
		pp->hunger_level = Strings::ToInt(row[r]); r++;									 // "hunger_level,              "
		pp->thirst_level = Strings::ToInt(row[r]); r++;									 // "thirst_level,              "
		pp->ability_up = Strings::ToInt(row[r]); r++;										 // "ability_up,                "
		pp->zone_id = Strings::ToInt(row[r]); r++;										 // "zone_id,                   "
		pp->zoneInstance = Strings::ToInt(row[r]); r++;									 // "zone_instance,             "
		pp->leadAAActive = Strings::ToInt(row[r]); r++;									 // "leadership_exp_on,         "
		pp->ldon_points_guk = Strings::ToInt(row[r]); r++;								 // "ldon_points_guk,           "
		pp->ldon_points_mir = Strings::ToInt(row[r]); r++;								 // "ldon_points_mir,           "
		pp->ldon_points_mmc = Strings::ToInt(row[r]); r++;								 // "ldon_points_mmc,           "
		pp->ldon_points_ruj = Strings::ToInt(row[r]); r++;								 // "ldon_points_ruj,           "
		pp->ldon_points_tak = Strings::ToInt(row[r]); r++;								 // "ldon_points_tak,           "
		pp->ldon_points_available = Strings::ToInt(row[r]); r++;							 // "ldon_points_available,     "
		pp->tribute_time_remaining = Strings::ToUnsignedInt(row[r]); r++;					 // "tribute_time_remaining,    "
		pp->showhelm = Strings::ToInt(row[r]); r++;										 // "show_helm,                 "
		pp->career_tribute_points = Strings::ToInt(row[r]); r++;							 // "career_tribute_points,     "
		pp->tribute_points = Strings::ToInt(row[r]); r++;									 // "tribute_points,            "
		pp->tribute_active = Strings::ToInt(row[r]); r++;									 // "tribute_active,            "
		pp->endurance = Strings::ToInt(row[r]); r++;										 // "endurance,                 "
		pp->group_leadership_exp = Strings::ToInt(row[r]); r++;							 // "group_leadership_exp,      "
		pp->raid_leadership_exp = Strings::ToInt(row[r]); r++;							 // "raid_leadership_exp,       "
		pp->group_leadership_points = Strings::ToInt(row[r]); r++;						 // "group_leadership_points,   "
		pp->raid_leadership_points = Strings::ToInt(row[r]); r++;							 // "raid_leadership_points,    "
		pp->air_remaining = Strings::ToInt(row[r]); r++;									 // "air_remaining,             "
		pp->PVPKills = Strings::ToInt(row[r]); r++;										 // "pvp_kills,                 "
		pp->PVPDeaths = Strings::ToInt(row[r]); r++;										 // "pvp_deaths,                "
		pp->PVPCurrentPoints = Strings::ToInt(row[r]); r++;								 // "pvp_current_points,        "
		pp->PVPCareerPoints = Strings::ToInt(row[r]); r++;								 // "pvp_career_points,         "
		pp->PVPBestKillStreak = Strings::ToInt(row[r]); r++;								 // "pvp_best_kill_streak,      "
		pp->PVPWorstDeathStreak = Strings::ToInt(row[r]); r++;							 // "pvp_worst_death_streak,    "
		pp->PVPCurrentKillStreak = Strings::ToInt(row[r]); r++;							 // "pvp_current_kill_streak,   "
		pp->aapoints_spent = Strings::ToInt(row[r]); r++;									 // "aa_points_spent,           "
		pp->expAA = Strings::ToInt(row[r]); r++;											 // "aa_exp,                    "
		pp->aapoints = Strings::ToInt(row[r]); r++;										 // "aa_points,                 "
		pp->groupAutoconsent = Strings::ToInt(row[r]); r++;								 // "group_auto_consent,        "
		pp->raidAutoconsent = Strings::ToInt(row[r]); r++;								 // "raid_auto_consent,         "
		pp->guildAutoconsent = Strings::ToInt(row[r]); r++;								 // "guild_auto_consent,        "
		pp->RestTimer = Strings::ToInt(row[r]); r++;										 // "RestTimer,                 "
		m_epp->aa_effects = Strings::ToInt(row[r]); r++;									 // "`e_aa_effects`,			"
		m_epp->perAA = Strings::ToInt(row[r]); r++;										 // "`e_percent_to_aa`,			"
		m_epp->expended_aa = Strings::ToInt(row[r]); r++;									 // "`e_expended_aa_spent`,		"
		m_epp->last_invsnapshot_time = Strings::ToUnsignedInt(row[r]); r++;						 // "`e_last_invsnapshot`		"
		m_epp->next_invsnapshot_time = m_epp->last_invsnapshot_time + (RuleI(Character, InvSnapshotMinIntervalM) * 60);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterFactionValues(uint32 character_id, faction_map & val_list) {
	std::string query = StringFormat("SELECT `faction_id`, `current_value` FROM `faction_values` WHERE `char_id` = %i", character_id);
	auto results = database.QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) { val_list[Strings::ToInt(row[0])] = Strings::ToInt(row[1]); }
	return true;
}

bool ZoneDatabase::LoadCharacterMemmedSpells(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT							"
		"slot_id,						"
		"`spell_id`						"
		"FROM							"
		"`character_memmed_spells`		"
		"WHERE `id` = %u ORDER BY `slot_id`", character_id);
	auto results = database.QueryDatabase(query);
	int i = 0;
	/* Initialize Spells */
	for (i = 0; i < EQ::spells::SPELL_GEM_COUNT; i++){
		pp->mem_spells[i] = 0xFFFFFFFF;
	}
	for (auto& row = results.begin(); row != results.end(); ++row) {
		i = Strings::ToInt(row[0]);
		if (i < EQ::spells::SPELL_GEM_COUNT && Strings::ToInt(row[1]) <= SPDAT_RECORDS){
			pp->mem_spells[i] = Strings::ToInt(row[1]);
		}
	}
	return true;
}

bool ZoneDatabase::LoadCharacterSpellBook(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT					"
		"slot_id,				"
		"`spell_id`				"
		"FROM					"
		"`character_spells`		"
		"WHERE `id` = %u ORDER BY `slot_id`", character_id);
	auto results = database.QueryDatabase(query);

	/* Initialize Spells */

	memset(pp->spell_book, 0xFF, (sizeof(uint32) * EQ::spells::SPELLBOOK_SIZE));

	// We have the ability to block loaded spells by max id on a per-client basis..
	// but, we do not have to ability to keep players from using older clients after
	// they have scribed spells on a newer one that exceeds the older one's limit.
	// Load them all so that server actions are valid..but, nix them in translators.

	for (auto& row = results.begin(); row != results.end(); ++row) {
		int idx = Strings::ToInt(row[0]);
		int id = Strings::ToInt(row[1]);

		if (idx < 0 || idx >= EQ::spells::SPELLBOOK_SIZE)
			continue;
		if (id < 3 || id > SPDAT_RECORDS) // 3 ("Summon Corpse") is the first scribable spell in spells_us.txt
			continue;

		pp->spell_book[idx] = id;
	}

	return true;
}

bool ZoneDatabase::LoadCharacterLanguages(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT					"
		"lang_id,				"
		"`value`				"
		"FROM					"
		"`character_languages`	"
		"WHERE `id` = %u ORDER BY `lang_id`", character_id);
	auto results = database.QueryDatabase(query); int i = 0;
	/* Initialize Languages */
	for (i = 0; i < MAX_PP_LANGUAGE; ++i)
		pp->languages[i] = 0;

	for (auto& row = results.begin(); row != results.end(); ++row) {
		i = Strings::ToInt(row[0]);
		if (i < MAX_PP_LANGUAGE){
			pp->languages[i] = Strings::ToInt(row[1]);
		}
	}

	return true;
}

bool ZoneDatabase::LoadCharacterLeadershipAA(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat("SELECT slot, `rank` FROM character_leadership_abilities WHERE `id` = %u", character_id);
	auto results = database.QueryDatabase(query); uint32 slot = 0;
	for (auto& row = results.begin(); row != results.end(); ++row) {
		slot = Strings::ToInt(row[0]);
		pp->leader_abilities.ranks[slot] = Strings::ToInt(row[1]);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterDisciplines(uint32 character_id, PlayerProfile_Struct* pp){

	auto character_disciplines = CharacterDisciplinesRepository::GetWhere(
		database, fmt::format(
			"`id` = {} ORDER BY `slot_id`",
			character_id
		)
	);

	if (character_disciplines.empty()) {
		return false;
	}

	/* Initialize Disciplines */
	memset(pp->disciplines.values, 0, (sizeof(pp->disciplines.values[0]) * MAX_PP_DISCIPLINES));
	for (auto& row : character_disciplines) {
		if (row.slot_id < MAX_PP_DISCIPLINES && IsValidSpell(row.disc_id)) {
			pp->disciplines.values[row.slot_id] = row.disc_id;
		}
	}
	return true;
}

bool ZoneDatabase::LoadCharacterSkills(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT				"
		"skill_id,			"
		"`value`			"
		"FROM				"
		"`character_skills` "
		"WHERE `id` = %u ORDER BY `skill_id`", character_id);
	auto results = database.QueryDatabase(query);
	int i = 0;
	/* Initialize Skill */
	for (i = 0; i < MAX_PP_SKILL; ++i)
		pp->skills[i] = 0;

	for (auto& row = results.begin(); row != results.end(); ++row) {
		i = Strings::ToInt(row[0]);
		if (i < MAX_PP_SKILL)
			pp->skills[i] = Strings::ToInt(row[1]);
	}

	return true;
}

bool ZoneDatabase::LoadCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT                  "
		"platinum,               "
		"gold,                   "
		"silver,                 "
		"copper,                 "
		"platinum_bank,          "
		"gold_bank,              "
		"silver_bank,            "
		"copper_bank,            "
		"platinum_cursor,        "
		"gold_cursor,            "
		"silver_cursor,          "
		"copper_cursor,          "
		"radiant_crystals,       "
		"career_radiant_crystals,"
		"ebon_crystals,          "
		"career_ebon_crystals    "
		"FROM                    "
		"character_currency      "
		"WHERE `id` = %i         ", character_id);
	auto results = database.QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		pp->platinum = Strings::ToInt(row[0]);
		pp->gold = Strings::ToInt(row[1]);
		pp->silver = Strings::ToInt(row[2]);
		pp->copper = Strings::ToInt(row[3]);
		pp->platinum_bank = Strings::ToInt(row[4]);
		pp->gold_bank = Strings::ToInt(row[5]);
		pp->silver_bank = Strings::ToInt(row[6]);
		pp->copper_bank = Strings::ToInt(row[7]);
		pp->platinum_cursor = Strings::ToInt(row[8]);
		pp->gold_cursor = Strings::ToInt(row[9]);
		pp->silver_cursor = Strings::ToInt(row[10]);
		pp->copper_cursor = Strings::ToInt(row[11]);
		pp->currentRadCrystals = Strings::ToInt(row[12]);
		pp->careerRadCrystals = Strings::ToInt(row[13]);
		pp->currentEbonCrystals = Strings::ToInt(row[14]);
		pp->careerEbonCrystals = Strings::ToInt(row[15]);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterMaterialColor(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat("SELECT slot, blue, green, red, use_tint, color FROM `character_material` WHERE `id` = %u LIMIT 9", character_id);
	auto results = database.QueryDatabase(query); int i = 0; int r = 0;
	for (auto& row = results.begin(); row != results.end(); ++row) {
		r = 0;
		i = Strings::ToInt(row[r]); /* Slot */ r++;
		pp->item_tint.Slot[i].Blue = Strings::ToInt(row[r]); r++;
		pp->item_tint.Slot[i].Green = Strings::ToInt(row[r]); r++;
		pp->item_tint.Slot[i].Red = Strings::ToInt(row[r]); r++;
		pp->item_tint.Slot[i].UseTint = Strings::ToInt(row[r]);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterBandolier(uint32 character_id, PlayerProfile_Struct* pp)
{
	std::string query = StringFormat("SELECT `bandolier_id`, `bandolier_slot`, `item_id`, `icon`, `bandolier_name` FROM `character_bandolier` WHERE `id` = %u LIMIT %u",
		character_id, EQ::profile::BANDOLIERS_SIZE);
	auto results = database.QueryDatabase(query); int i = 0; int r = 0; int si = 0;
	for (i = 0; i < EQ::profile::BANDOLIERS_SIZE; i++) {
		pp->bandoliers[i].Name[0] = '\0';
		for (int si = 0; si < EQ::profile::BANDOLIER_ITEM_COUNT; si++) {
			pp->bandoliers[i].Items[si].ID = 0;
			pp->bandoliers[i].Items[si].Icon = 0;
			pp->bandoliers[i].Items[si].Name[0] = '\0';
		}
	}

	for (auto& row = results.begin(); row != results.end(); ++row) {
		r = 0;
		i = Strings::ToInt(row[r]); /* Bandolier ID */ r++;
		si = Strings::ToInt(row[r]); /* Bandolier Slot */ r++;

		const EQ::ItemData* item_data = database.GetItem(Strings::ToInt(row[r]));
		if (item_data) {
			pp->bandoliers[i].Items[si].ID = item_data->ID; r++;
			pp->bandoliers[i].Items[si].Icon = Strings::ToInt(row[r]); r++; // Must use db value in case an Ornamentation is assigned
			strncpy(pp->bandoliers[i].Items[si].Name, item_data->Name, 64);
		}
		else {
			pp->bandoliers[i].Items[si].ID = 0; r++;
			pp->bandoliers[i].Items[si].Icon = 0; r++;
			pp->bandoliers[i].Items[si].Name[0] = '\0';
		}
		strcpy(pp->bandoliers[i].Name, row[r]);  r++;

		si++; // What is this for!?
	}
	return true;
}

void ZoneDatabase::LoadCharacterTribute(Client* c){
	const auto& l = CharacterTributeRepository::GetWhere(database, fmt::format("character_id = {}", c->CharacterID()));

	for (auto& t : c->GetPP().tributes) {
		t.tier    = 0;
		t.tribute = TRIBUTE_NONE;
	}

	auto i = 0;

	for (const auto& e : l) {
		if (e.tribute != TRIBUTE_NONE) {
			c->GetPP().tributes[i].tier    = e.tier;
			c->GetPP().tributes[i].tribute = e.tribute;
			i++;
		}
	}
}

bool ZoneDatabase::LoadCharacterPotions(uint32 character_id, PlayerProfile_Struct *pp)
{
	std::string query =
	    StringFormat("SELECT `potion_id`, `item_id`, `icon` FROM `character_potionbelt` WHERE `id` = %u LIMIT %u",
		character_id, EQ::profile::POTION_BELT_SIZE);
	auto results = database.QueryDatabase(query);
	int i = 0;
	for (i = 0; i < EQ::profile::POTION_BELT_SIZE; i++) {
		pp->potionbelt.Items[i].Icon = 0;
		pp->potionbelt.Items[i].ID = 0;
		pp->potionbelt.Items[i].Name[0] = '\0';
	}

	for (auto& row = results.begin(); row != results.end(); ++row) {
		i = Strings::ToInt(row[0]);
		const EQ::ItemData *item_data = database.GetItem(Strings::ToInt(row[1]));
		if (!item_data)
			continue;
		pp->potionbelt.Items[i].ID = item_data->ID;
		pp->potionbelt.Items[i].Icon = Strings::ToInt(row[2]);
		strncpy(pp->potionbelt.Items[i].Name, item_data->Name, 64);
	}

	return true;
}

bool ZoneDatabase::LoadCharacterBindPoint(uint32 character_id, PlayerProfile_Struct *pp)
{
	std::string query = StringFormat("SELECT `slot`, `zone_id`, `instance_id`, `x`, `y`, `z`, `heading` FROM "
					 "`character_bind` WHERE `id` = %u LIMIT 5",
					 character_id);
	auto results = database.QueryDatabase(query);

	if (!results.RowCount()) // SHIT -- this actually isn't good
		return true;

	for (auto& row = results.begin(); row != results.end(); ++row) {
		int index = Strings::ToInt(row[0]);
		if (index < 0 || index > 4)
			continue;

		pp->binds[index].zone_id = Strings::ToInt(row[1]);
		pp->binds[index].instance_id = Strings::ToInt(row[2]);
		pp->binds[index].x = Strings::ToFloat(row[3]);
		pp->binds[index].y = Strings::ToFloat(row[4]);
		pp->binds[index].z = Strings::ToFloat(row[5]);
		pp->binds[index].heading = Strings::ToFloat(row[6]);
	}

	return true;
}

bool ZoneDatabase::SaveCharacterLanguage(uint32 character_id, uint32 lang_id, uint32 value){
	std::string query = StringFormat("REPLACE INTO `character_languages` (id, lang_id, value) VALUES (%u, %u, %u)", character_id, lang_id, value); QueryDatabase(query);
	LogDebug("ZoneDatabase::SaveCharacterLanguage for character ID: [{}], lang_id:[{}] value:[{}] done", character_id, lang_id, value);
	return true;
}

bool ZoneDatabase::SaveCharacterMaterialColor(uint32 character_id, uint32 slot_id, uint32 color){
	uint8 red = (color & 0x00FF0000) >> 16;
	uint8 green = (color & 0x0000FF00) >> 8;
	uint8 blue = (color & 0x000000FF);

	std::string query = StringFormat("REPLACE INTO `character_material` (id, slot, red, green, blue, color, use_tint) VALUES (%u, %u, %u, %u, %u, %u, 255)", character_id, slot_id, red, green, blue, color); auto results = QueryDatabase(query);
	LogDebug("ZoneDatabase::SaveCharacterMaterialColor for character ID: [{}], slot_id: [{}] color: [{}] done", character_id, slot_id, color);
	return true;
}

bool ZoneDatabase::SaveCharacterSkill(uint32 character_id, uint32 skill_id, uint32 value){
	std::string query = StringFormat("REPLACE INTO `character_skills` (id, skill_id, value) VALUES (%u, %u, %u)", character_id, skill_id, value); auto results = QueryDatabase(query);
	LogDebug("ZoneDatabase::SaveCharacterSkill for character ID: [{}], skill_id:[{}] value:[{}] done", character_id, skill_id, value);
	return true;
}

bool ZoneDatabase::SaveCharacterDisc(uint32 character_id, uint32 slot_id, uint32 disc_id){
	std::string query = StringFormat("REPLACE INTO `character_disciplines` (id, slot_id, disc_id) VALUES (%u, %u, %u)", character_id, slot_id, disc_id);
	auto results = QueryDatabase(query);
	LogDebug("ZoneDatabase::SaveCharacterDisc for character ID: [{}], slot:[{}] disc_id:[{}] done", character_id, slot_id, disc_id);
	return true;
}

void ZoneDatabase::SaveCharacterTribute(Client* c)
{
	std::vector<CharacterTributeRepository::CharacterTribute> tributes = {};
	CharacterTributeRepository::CharacterTribute tribute = {};

	uint32 tribute_count = 0;
	for (auto& t : c->GetPP().tributes) {
		if (t.tribute != TRIBUTE_NONE) {
			tribute_count++;
		}
	}

	tributes.reserve(tribute_count);

	for (auto& t : c->GetPP().tributes) {
		if (t.tribute != TRIBUTE_NONE) {
			tribute.character_id = c->CharacterID();
			tribute.tier         = t.tier;
			tribute.tribute      = t.tribute;

			tributes.emplace_back(tribute);
		}
	}

	CharacterTributeRepository::DeleteWhere(database, fmt::format("character_id = {}", c->CharacterID()));

	if (tribute_count > 0) {
		CharacterTributeRepository::InsertMany(database, tributes);
	}
}

bool ZoneDatabase::SaveCharacterBandolier(uint32 character_id, uint8 bandolier_id, uint8 bandolier_slot, uint32 item_id, uint32 icon, const char* bandolier_name)
{
	char bandolier_name_esc[64];
	DoEscapeString(bandolier_name_esc, bandolier_name, strlen(bandolier_name));
	std::string query = StringFormat("REPLACE INTO `character_bandolier` (id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name) VALUES (%u, %u, %u, %u, %u,'%s')", character_id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name_esc);
	auto results = QueryDatabase(query);
	LogDebug("ZoneDatabase::SaveCharacterBandolier for character ID: [{}], bandolier_id: [{}], bandolier_slot: [{}] item_id: [{}], icon:[{}] band_name:[{}]  done", character_id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name);
	return true;
}

bool ZoneDatabase::SaveCharacterPotionBelt(uint32 character_id, uint8 potion_id, uint32 item_id, uint32 icon)
{
	std::string query = StringFormat("REPLACE INTO `character_potionbelt` (id, potion_id, item_id, icon) VALUES (%u, %u, %u, %u)", character_id, potion_id, item_id, icon);
	auto results = QueryDatabase(query);
	return true;
}

bool ZoneDatabase::SaveCharacterLeadershipAA(uint32 character_id, PlayerProfile_Struct* pp){
	uint8 first_entry = 0; std::string query = "";
	for (int i = 0; i < MAX_LEADERSHIP_AA_ARRAY; i++){
		if (pp->leader_abilities.ranks[i] > 0){
			if (first_entry != 1){
				query = StringFormat("REPLACE INTO `character_leadership_abilities` (id, slot, `rank`) VALUES (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
				first_entry = 1;
			}
			query = query + StringFormat(", (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
		}
	}
	auto results = QueryDatabase(query);
	return true;
}

bool ZoneDatabase::SaveCharacterData(
	Client* c,
	PlayerProfile_Struct* pp,
	ExtendedProfile_Struct* m_epp
) {
	if (!c) {
		return false;
	}

	/* If this is ever zero - the client hasn't fully loaded and potentially crashed during zone */
	if (c->AccountID() <= 0) {
		return false;
	}

	clock_t t = std::clock(); /* Function timer start */
	const auto query = fmt::format(
		"REPLACE INTO `character_data` ("
		" id,                        "
		" account_id,                "
		" `name`,                    "
		" last_name,                 "
		" gender,                    "
		" race,                      "
		" class,                     "
		" `level`,                   "
		" deity,                     "
		" birthday,                  "
		" last_login,                "
		" time_played,               "
		" pvp_status,                "
		" level2,                    "
		" anon,                      "
		" gm,                        "
		" intoxication,              "
		" hair_color,                "
		" beard_color,               "
		" eye_color_1,               "
		" eye_color_2,               "
		" hair_style,                "
		" beard,                     "
		" ability_time_seconds,      "
		" ability_number,            "
		" ability_time_minutes,      "
		" ability_time_hours,        "
		" title,                     "
		" suffix,                    "
		" exp,                       "
		" exp_enabled,               "
		" points,                    "
		" mana,                      "
		" cur_hp,                    "
		" str,                       "
		" sta,                       "
		" cha,                       "
		" dex,                       "
		" `int`,                     "
		" agi,                       "
		" wis,                       "
		" face,                      "
		" y,                         "
		" x,                         "
		" z,                         "
		" heading,                   "
		" pvp2,                      "
		" pvp_type,                  "
		" autosplit_enabled,         "
		" zone_change_count,         "
		" drakkin_heritage,          "
		" drakkin_tattoo,            "
		" drakkin_details,           "
		" toxicity,                  "
		" hunger_level,              "
		" thirst_level,              "
		" ability_up,                "
		" zone_id,                   "
		" zone_instance,             "
		" leadership_exp_on,         "
		" ldon_points_guk,           "
		" ldon_points_mir,           "
		" ldon_points_mmc,           "
		" ldon_points_ruj,           "
		" ldon_points_tak,           "
		" ldon_points_available,     "
		" tribute_time_remaining,    "
		" show_helm,                 "
		" career_tribute_points,     "
		" tribute_points,            "
		" tribute_active,            "
		" endurance,                 "
		" group_leadership_exp,      "
		" raid_leadership_exp,       "
		" group_leadership_points,   "
		" raid_leadership_points,    "
		" air_remaining,             "
		" pvp_kills,                 "
		" pvp_deaths,                "
		" pvp_current_points,        "
		" pvp_career_points,         "
		" pvp_best_kill_streak,      "
		" pvp_worst_death_streak,    "
		" pvp_current_kill_streak,   "
		" aa_points_spent,           "
		" aa_exp,                    "
		" aa_points,                 "
		" group_auto_consent,        "
		" raid_auto_consent,         "
		" guild_auto_consent,        "
		" RestTimer,				 "
		" e_aa_effects,				 "
		" e_percent_to_aa,			 "
		" e_expended_aa_spent,		 "
		" e_last_invsnapshot,		 "
		" mailkey					 "
		")							 "
		"VALUES ("
		"{},"  // id																" id,                        "
		"{},"  // account_id														" account_id,                "
		"'{}',"  // `name`					  pp->name,								" `name`,                    "
		"'{}',"  // last_name					pp->last_name,						" last_name,                 "
		"{},"  // gender					  pp->gender,							" gender,                    "
		"{},"  // race						  pp->race,								" race,                      "
		"{},"  // class						  pp->class_,							" class,                     "
		"{},"  // `level`					  pp->level,							" `level`,                   "
		"{},"  // deity						  pp->deity,							" deity,                     "
		"{},"  // birthday					  pp->birthday,							" birthday,                  "
		"{},"  // last_login				  pp->lastlogin,						" last_login,                "
		"{},"  // time_played				  pp->timePlayedMin,					" time_played,               "
		"{},"  // pvp_status				  pp->pvp,								" pvp_status,                "
		"{},"  // level2					  pp->level2,							" level2,                    "
		"{},"  // anon						  pp->anon,								" anon,                      "
		"{},"  // gm						  pp->gm,								" gm,                        "
		"{},"  // intoxication				  pp->intoxication,						" intoxication,              "
		"{},"  // hair_color				  pp->haircolor,						" hair_color,                "
		"{},"  // beard_color				  pp->beardcolor,						" beard_color,               "
		"{},"  // eye_color_1				  pp->eyecolor1,						" eye_color_1,               "
		"{},"  // eye_color_2				  pp->eyecolor2,						" eye_color_2,               "
		"{},"  // hair_style				  pp->hairstyle,						" hair_style,                "
		"{},"  // beard						  pp->beard,							" beard,                     "
		"{},"  // ability_time_seconds		  pp->ability_time_seconds,				" ability_time_seconds,      "
		"{},"  // ability_number			  pp->ability_number,					" ability_number,            "
		"{},"  // ability_time_minutes		  pp->ability_time_minutes,				" ability_time_minutes,      "
		"{},"  // ability_time_hours		  pp->ability_time_hours,				" ability_time_hours,        "
		"'{}',"  // title					  pp->title,							" title,                     "   "
		"'{}',"  // suffix					  pp->suffix,							" suffix,                    "
		"{},"  // exp						  pp->exp,								" exp,                       "
		"{},"  // exp_enabled				  epp->exp_enabled,							" exp_enabled,                       "
		"{},"  // points					  pp->points,							" points,                    "
		"{},"  // mana						  pp->mana,								" mana,                      "
		"{},"  // cur_hp					  pp->cur_hp,							" cur_hp,                    "
		"{},"  // str						  pp->STR,								" str,                       "
		"{},"  // sta						  pp->STA,								" sta,                       "
		"{},"  // cha						  pp->CHA,								" cha,                       "
		"{},"  // dex						  pp->DEX,								" dex,                       "
		"{},"  // `int`						  pp->INT,								" `int`,                     "
		"{},"  // agi						  pp->AGI,								" agi,                       "
		"{},"  // wis						  pp->WIS,								" wis,                       "
		"{},"  // face						  pp->face,								" face,                      "
		"{:.2f},"  // y							  pp->y,								" y,                         "
		"{:.2f},"  // x							  pp->x,								" x,                         "
		"{:.2f},"  // z							  pp->z,								" z,                         "
		"{:.2f},"  // heading					  pp->heading,							" heading,                   "
		"{},"  // pvp2						  pp->pvp2,								" pvp2,                      "
		"{},"  // pvp_type					  pp->pvptype,							" pvp_type,                  "
		"{},"  // autosplit_enabled			  pp->autosplit,						" autosplit_enabled,         "
		"{},"  // zone_change_count			  pp->zone_change_count,				" zone_change_count,         "
		"{},"  // drakkin_heritage			  pp->drakkin_heritage,					" drakkin_heritage,          "
		"{},"  // drakkin_tattoo			  pp->drakkin_tattoo,					" drakkin_tattoo,            "
		"{},"  // drakkin_details			  pp->drakkin_details,					" drakkin_details,           "
		"{},"  // toxicity					  pp->toxicity,							" toxicity,                  "
		"{},"  // hunger_level				  pp->hunger_level,						" hunger_level,              "
		"{},"  // thirst_level				  pp->thirst_level,						" thirst_level,              "
		"{},"  // ability_up				  pp->ability_up,						" ability_up,                "
		"{},"  // zone_id					  pp->zone_id,							" zone_id,                   "
		"{},"  // zone_instance				  pp->zoneInstance,						" zone_instance,             "
		"{},"  // leadership_exp_on			  pp->leadAAActive,						" leadership_exp_on,         "
		"{},"  // ldon_points_guk			  pp->ldon_points_guk,					" ldon_points_guk,           "
		"{},"  // ldon_points_mir			  pp->ldon_points_mir,					" ldon_points_mir,           "
		"{},"  // ldon_points_mmc			  pp->ldon_points_mmc,					" ldon_points_mmc,           "
		"{},"  // ldon_points_ruj			  pp->ldon_points_ruj,					" ldon_points_ruj,           "
		"{},"  // ldon_points_tak			  pp->ldon_points_tak,					" ldon_points_tak,           "
		"{},"  // ldon_points_available		  pp->ldon_points_available,			" ldon_points_available,     "
		"{},"  // tribute_time_remaining	  pp->tribute_time_remaining,			" tribute_time_remaining,    "
		"{},"  // show_helm					  pp->showhelm,							" show_helm,                 "
		"{},"  // career_tribute_points		  pp->career_tribute_points,			" career_tribute_points,     "
		"{},"  // tribute_points			  pp->tribute_points,					" tribute_points,            "
		"{},"  // tribute_active			  pp->tribute_active,					" tribute_active,            "
		"{},"  // endurance					  pp->endurance,						" endurance,                 "
		"{},"  // group_leadership_exp		  pp->group_leadership_exp,				" group_leadership_exp,      "
		"{},"  // raid_leadership_exp		  pp->raid_leadership_exp,				" raid_leadership_exp,       "
		"{},"  // group_leadership_points	  pp->group_leadership_points,			" group_leadership_points,   "
		"{},"  // raid_leadership_points	  pp->raid_leadership_points,			" raid_leadership_points,    "
		"{},"  // air_remaining				  pp->air_remaining,					" air_remaining,             "
		"{},"  // pvp_kills					  pp->PVPKills,							" pvp_kills,                 "
		"{},"  // pvp_deaths				  pp->PVPDeaths,						" pvp_deaths,                "
		"{},"  // pvp_current_points		  pp->PVPCurrentPoints,					" pvp_current_points,        "
		"{},"  // pvp_career_points			  pp->PVPCareerPoints,					" pvp_career_points,         "
		"{},"  // pvp_best_kill_streak		  pp->PVPBestKillStreak,				" pvp_best_kill_streak,      "
		"{},"  // pvp_worst_death_streak	  pp->PVPWorstDeathStreak,				" pvp_worst_death_streak,    "
		"{},"  // pvp_current_kill_streak	  pp->PVPCurrentKillStreak,				" pvp_current_kill_streak,   "
		"{},"  // aa_points_spent			  pp->aapoints_spent,					" aa_points_spent,           "
		"{},"  // aa_exp					  pp->expAA,							" aa_exp,                    "
		"{},"  // aa_points					  pp->aapoints,							" aa_points,                 "
		"{},"  // group_auto_consent		  pp->groupAutoconsent,					" group_auto_consent,        "
		"{},"  // raid_auto_consent			  pp->raidAutoconsent,					" raid_auto_consent,         "
		"{},"  // guild_auto_consent		  pp->guildAutoconsent,					" guild_auto_consent,        "
		"{},"  // RestTimer					  pp->RestTimer,						" RestTimer)                 "
		"{},"  // e_aa_effects
		"{},"  // e_percent_to_aa
		"{},"  // e_expended_aa_spent
		"{},"  // e_last_invsnapshot
		"'{}'" // mailkey					  mail_key
		")",
		c->CharacterID(),				  // " id,                        "
		c->AccountID(),					  // " account_id,                "
		Strings::Escape(pp->name),		  // " `name`,                    "
		Strings::Escape(pp->last_name),	  // " last_name,                 "
		pp->gender,						  // " gender,                    "
		pp->race,						  // " race,                      "
		pp->class_,						  // " class,                     "
		pp->level,						  // " `level`,                   "
		pp->deity,						  // " deity,                     "
		pp->birthday,					  // " birthday,                  "
		pp->lastlogin,					  // " last_login,                "
		pp->timePlayedMin,				  // " time_played,               "
		pp->pvp,						  // " pvp_status,                "
		pp->level2,						  // " level2,                    "
		pp->anon,						  // " anon,                      "
		pp->gm,							  // " gm,                        "
		pp->intoxication,				  // " intoxication,              "
		pp->haircolor,					  // " hair_color,                "
		pp->beardcolor,					  // " beard_color,               "
		pp->eyecolor1,					  // " eye_color_1,               "
		pp->eyecolor2,					  // " eye_color_2,               "
		pp->hairstyle,					  // " hair_style,                "
		pp->beard,						  // " beard,                     "
		pp->ability_time_seconds,		  // " ability_time_seconds,      "
		pp->ability_number,				  // " ability_number,            "
		pp->ability_time_minutes,		  // " ability_time_minutes,      "
		pp->ability_time_hours,			  // " ability_time_hours,        "
		Strings::Escape(pp->title),		  // " title,                     "
		Strings::Escape(pp->suffix),	  // " suffix,                    "
		pp->exp,						  // " exp,                       "
		c->IsEXPEnabled(),				  // " exp_enabled,               "
		pp->points,						  // " points,                    "
		pp->mana,						  // " mana,                      "
		pp->cur_hp,						  // " cur_hp,                    "
		pp->STR,						  // " str,                       "
		pp->STA,						  // " sta,                       "
		pp->CHA,						  // " cha,                       "
		pp->DEX,						  // " dex,                       "
		pp->INT,						  // " `int`,                     "
		pp->AGI,						  // " agi,                       "
		pp->WIS,						  // " wis,                       "
		pp->face,						  // " face,                      "
		pp->y,							  // " y,                         "
		pp->x,							  // " x,                         "
		pp->z,							  // " z,                         "
		pp->heading,					  // " heading,                   "
		pp->pvp2,						  // " pvp2,                      "
		pp->pvptype,					  // " pvp_type,                  "
		pp->autosplit,					  // " autosplit_enabled,         "
		pp->zone_change_count,			  // " zone_change_count,         "
		pp->drakkin_heritage,			  // " drakkin_heritage,          "
		pp->drakkin_tattoo,				  // " drakkin_tattoo,            "
		pp->drakkin_details,			  // " drakkin_details,           "
		pp->toxicity,					  // " toxicity,                  "
		pp->hunger_level,				  // " hunger_level,              "
		pp->thirst_level,				  // " thirst_level,              "
		pp->ability_up,					  // " ability_up,                "
		pp->zone_id,					  // " zone_id,                   "
		pp->zoneInstance,				  // " zone_instance,             "
		pp->leadAAActive,				  // " leadership_exp_on,         "
		pp->ldon_points_guk,			  // " ldon_points_guk,           "
		pp->ldon_points_mir,			  // " ldon_points_mir,           "
		pp->ldon_points_mmc,			  // " ldon_points_mmc,           "
		pp->ldon_points_ruj,			  // " ldon_points_ruj,           "
		pp->ldon_points_tak,			  // " ldon_points_tak,           "
		pp->ldon_points_available,		  // " ldon_points_available,     "
		pp->tribute_time_remaining,		  // " tribute_time_remaining,    "
		pp->showhelm,					  // " show_helm,                 "
		pp->career_tribute_points,		  // " career_tribute_points,     "
		pp->tribute_points,				  // " tribute_points,            "
		pp->tribute_active,				  // " tribute_active,            "
		pp->endurance,					  // " endurance,                 "
		pp->group_leadership_exp,		  // " group_leadership_exp,      "
		pp->raid_leadership_exp,		  // " raid_leadership_exp,       "
		pp->group_leadership_points,	  // " group_leadership_points,   "
		pp->raid_leadership_points,		  // " raid_leadership_points,    "
		pp->air_remaining,				  // " air_remaining,             "
		pp->PVPKills,					  // " pvp_kills,                 "
		pp->PVPDeaths,					  // " pvp_deaths,                "
		pp->PVPCurrentPoints,			  // " pvp_current_points,        "
		pp->PVPCareerPoints,			  // " pvp_career_points,         "
		pp->PVPBestKillStreak,			  // " pvp_best_kill_streak,      "
		pp->PVPWorstDeathStreak,		  // " pvp_worst_death_streak,    "
		pp->PVPCurrentKillStreak,		  // " pvp_current_kill_streak,   "
		pp->aapoints_spent,				  // " aa_points_spent,           "
		pp->expAA,						  // " aa_exp,                    "
		pp->aapoints,					  // " aa_points,                 "
		pp->groupAutoconsent,			  // " group_auto_consent,        "
		pp->raidAutoconsent,			  // " raid_auto_consent,         "
		pp->guildAutoconsent,			  // " guild_auto_consent,        "
		pp->RestTimer,					  // " RestTimer)                 "
		m_epp->aa_effects,
		m_epp->perAA,
		m_epp->expended_aa,
		m_epp->last_invsnapshot_time,
		c->GetMailKeyFull()
	);
	auto results = database.QueryDatabase(query);
	LogDebug(
		"ZoneDatabase::SaveCharacterData [{}], done Took [{}] seconds",
		c->CharacterID(),
		((float)(std::clock() - t)) / CLOCKS_PER_SEC
	);
	return true;
}

bool ZoneDatabase::SaveCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp){
	if (pp->copper < 0) { pp->copper = 0; }
	if (pp->silver < 0) { pp->silver = 0; }
	if (pp->gold < 0) { pp->gold = 0; }
	if (pp->platinum < 0) { pp->platinum = 0; }
	if (pp->copper_bank < 0) { pp->copper_bank = 0; }
	if (pp->silver_bank < 0) { pp->silver_bank = 0; }
	if (pp->gold_bank < 0) { pp->gold_bank = 0; }
	if (pp->platinum_bank < 0) { pp->platinum_bank = 0; }
	if (pp->platinum_cursor < 0) { pp->platinum_cursor = 0; }
	if (pp->gold_cursor < 0) { pp->gold_cursor = 0; }
	if (pp->silver_cursor < 0) { pp->silver_cursor = 0; }
	if (pp->copper_cursor < 0) { pp->copper_cursor = 0; }
	std::string query = StringFormat(
		"REPLACE INTO `character_currency` (id, platinum, gold, silver, copper,"
		"platinum_bank, gold_bank, silver_bank, copper_bank,"
		"platinum_cursor, gold_cursor, silver_cursor, copper_cursor, "
		"radiant_crystals, career_radiant_crystals, ebon_crystals, career_ebon_crystals)"
		"VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u)",
		character_id,
		pp->platinum,
		pp->gold,
		pp->silver,
		pp->copper,
		pp->platinum_bank,
		pp->gold_bank,
		pp->silver_bank,
		pp->copper_bank,
		pp->platinum_cursor,
		pp->gold_cursor,
		pp->silver_cursor,
		pp->copper_cursor,
		pp->currentRadCrystals,
		pp->careerRadCrystals,
		pp->currentEbonCrystals,
		pp->careerEbonCrystals);
	auto results = database.QueryDatabase(query);
	LogDebug("Saving Currency for character ID: [{}], done", character_id);
	return true;
}

bool ZoneDatabase::SaveCharacterAA(uint32 character_id, uint32 aa_id, uint32 current_level, uint32 charges){
	std::string rquery = StringFormat("REPLACE INTO `character_alternate_abilities` (id, aa_id, aa_value, charges)"
		" VALUES (%u, %u, %u, %u)",
		character_id, aa_id, current_level, charges);
	auto results = QueryDatabase(rquery);
	LogDebug("Saving AA for character ID: [{}], aa_id: [{}] current_level: [{}]", character_id, aa_id, current_level);
	return true;
}

bool ZoneDatabase::SaveCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	if (spell_id > SPDAT_RECORDS){ return false; }
	std::string query = StringFormat("REPLACE INTO `character_memmed_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, slot_id, spell_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::SaveCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	if (spell_id > SPDAT_RECORDS){ return false; }
	std::string query = StringFormat("REPLACE INTO `character_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, slot_id, spell_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	std::string query = StringFormat("DELETE FROM `character_spells` WHERE `slot_id` = %u AND `id` = %u", slot_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterDisc(uint32 character_id, uint32 slot_id){
	std::string query = StringFormat("DELETE FROM `character_disciplines` WHERE `slot_id` = %u AND `id` = %u", slot_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterBandolier(uint32 character_id, uint32 band_id){
	std::string query = StringFormat("DELETE FROM `character_bandolier` WHERE `bandolier_id` = %u AND `id` = %u", band_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterLeadershipAAs(uint32 character_id){
	std::string query = StringFormat("DELETE FROM `character_leadership_abilities` WHERE `id` = %u", character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterAAs(uint32 character_id){
	std::string query = StringFormat("DELETE FROM `character_alternate_abilities` WHERE `id` = %u AND `aa_id` NOT IN(SELECT a.first_rank_id FROM aa_ability a WHERE a.grant_only != 0)", character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterDye(uint32 character_id){
	std::string query = StringFormat("DELETE FROM `character_material` WHERE `id` = %u", character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	std::string query = StringFormat("DELETE FROM `character_memmed_spells` WHERE `slot_id` = %u AND `id` = %u", slot_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::NoRentExpired(const char* name){
	std::string query = StringFormat("SELECT (UNIX_TIMESTAMP(NOW()) - last_login) FROM `character_data` WHERE name = '%s'", name);
	auto results = QueryDatabase(query);
	if (!results.Success())
        return false;

    if (results.RowCount() != 1)
        return false;

	auto& row = results.begin();
	uint32 seconds = Strings::ToInt(row[0]);

	return (seconds>1800);
}

bool ZoneDatabase::SaveCharacterInvSnapshot(uint32 character_id) {
	uint32 time_index = time(nullptr);
	std::string query = StringFormat(
		"INSERT "
		"INTO"
		" `inventory_snapshots` "
		"(`time_index`,"
		" `charid`,"
		" `slotid`,"
		" `itemid`,"
		" `charges`,"
		" `color`,"
		" `augslot1`,"
		" `augslot2`,"
		" `augslot3`,"
		" `augslot4`,"
		" `augslot5`,"
		" `augslot6`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornamenticon`,"
		" `ornamentidfile`,"
		" `ornament_hero_model`"
		") "
		"SELECT"
		" %u,"
		" `charid`,"
		" `slotid`,"
		" `itemid`,"
		" `charges`,"
		" `color`,"
		" `augslot1`,"
		" `augslot2`,"
		" `augslot3`,"
		" `augslot4`,"
		" `augslot5`,"
		" `augslot6`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornamenticon`,"
		" `ornamentidfile`,"
		" `ornament_hero_model` "
		"FROM"
		" `inventory` "
		"WHERE"
		" `charid` = %u",
		time_index,
		character_id
	);
	auto results = database.QueryDatabase(query);
	LogInventory("[{}] ([{}])", character_id, (results.Success() ? "pass" : "fail"));
	return results.Success();
}

int ZoneDatabase::CountCharacterInvSnapshots(uint32 character_id) {
	std::string query = StringFormat(
		"SELECT"
		" COUNT(*) "
		"FROM "
		"("
		"SELECT * FROM"
		" `inventory_snapshots` a "
		"WHERE"
		" `charid` = %u "
		"GROUP BY"
		" `time_index`"
		") b",
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return -1;

	auto& row = results.begin();

	int64 count = Strings::ToBigInt(row[0]);
	if (count > 2147483647)
		return -2;
	if (count < 0)
		return -3;

	return count;
}

void ZoneDatabase::ClearCharacterInvSnapshots(uint32 character_id, bool from_now) {
	uint32 del_time = time(nullptr);
	if (!from_now) { del_time -= RuleI(Character, InvSnapshotHistoryD) * 86400; }

	std::string query = StringFormat(
		"DELETE "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` <= %lu",
		character_id,
		(unsigned long)del_time
	);
	QueryDatabase(query);
}

void ZoneDatabase::ListCharacterInvSnapshots(uint32 character_id, std::list<std::pair<uint32, int>> &is_list) {
	std::string query = StringFormat(
		"SELECT"
		" `time_index`,"
		" COUNT(*) "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"GROUP BY"
		" `time_index` "
		"ORDER BY"
		" `time_index` "
		"DESC",
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		is_list.emplace_back(std::pair<uint32, int>(Strings::ToUnsignedInt(row[0]), Strings::ToInt(row[1])));
}

bool ZoneDatabase::ValidateCharacterInvSnapshotTimestamp(uint32 character_id, uint32 timestamp) {
	if (!character_id || !timestamp)
		return false;

	std::string query = StringFormat(
		"SELECT"
		" * "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` = %u "
		"LIMIT 1",
		character_id,
		timestamp
	);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0)
		return false;

	return true;
}

void ZoneDatabase::ParseCharacterInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &parse_list) {
	std::string query = StringFormat(
		"SELECT"
		" `slotid`,"
		" `itemid` "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` = %u "
		"ORDER BY"
		" `slotid`",
		character_id,
		timestamp
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		parse_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
}

void ZoneDatabase::DivergeCharacterInvSnapshotFromInventory(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list) {
	std::string query = StringFormat(
		"SELECT"
		" slotid,"
		" itemid "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `time_index` = %u "
		"AND"
		" `charid` = %u "
		"AND"
		" `slotid` NOT IN "
		"("
		"SELECT"
		" a.`slotid` "
		"FROM"
		" `inventory_snapshots` a "
		"JOIN"
		" `inventory` b "
		"USING"
		" (`slotid`, `itemid`) "
		"WHERE"
		" a.`time_index` = %u "
		"AND"
		" a.`charid` = %u "
		"AND"
		" b.`charid` = %u"
		")",
		timestamp,
		character_id,
		timestamp,
		character_id,
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		compare_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
}

void ZoneDatabase::DivergeCharacterInventoryFromInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list) {
	std::string query = StringFormat(
		"SELECT"
		" `slotid`,"
		" `itemid` "
		"FROM"
		" `inventory` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `slotid` NOT IN "
		"("
		"SELECT"
		" a.`slotid` "
		"FROM"
		" `inventory` a "
		"JOIN"
		" `inventory_snapshots` b "
		"USING"
		" (`slotid`, `itemid`) "
		"WHERE"
		" b.`time_index` = %u "
		"AND"
		" b.`charid` = %u "
		"AND"
		" a.`charid` = %u"
		")",
		character_id,
		timestamp,
		character_id,
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		compare_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
}

bool ZoneDatabase::RestoreCharacterInvSnapshot(uint32 character_id, uint32 timestamp) {
	// we should know what we're doing by the time we call this function..but,
	// this is to prevent inventory deletions where no timestamp entries exists
	if (!ValidateCharacterInvSnapshotTimestamp(character_id, timestamp)) {
		LogError("called for id: [{}] without valid snapshot entries @ [{}]", character_id, timestamp);
		return false;
	}

	std::string query = StringFormat(
		"DELETE "
		"FROM"
		" `inventory` "
		"WHERE"
		" `charid` = %u",
		character_id
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	query = StringFormat(
		"INSERT "
		"INTO"
		" `inventory` "
		"(`charid`,"
		" `slotid`,"
		" `itemid`,"
		" `charges`,"
		" `color`,"
		" `augslot1`,"
		" `augslot2`,"
		" `augslot3`,"
		" `augslot4`,"
		" `augslot5`,"
		" `augslot6`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornamenticon`,"
		" `ornamentidfile`,"
		" `ornament_hero_model`"
		") "
		"SELECT"
		" `charid`,"
		" `slotid`,"
		" `itemid`,"
		" `charges`,"
		" `color`,"
		" `augslot1`,"
		" `augslot2`,"
		" `augslot3`,"
		" `augslot4`,"
		" `augslot5`,"
		" `augslot6`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornamenticon`,"
		" `ornamentidfile`,"
		" `ornament_hero_model` "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` = %u",
		character_id,
		timestamp
	);
	results = database.QueryDatabase(query);

	LogInventory("[{}] snapshot for [{}] @ [{}]",
		(results.Success() ? "restored" : "failed to restore"), character_id, timestamp);

	return results.Success();
}

const NPCType *ZoneDatabase::LoadNPCTypesData(uint32 npc_type_id, bool bulk_load /*= false*/)
{
	const NPCType *npc = nullptr;

	/* If there is a cached NPC entry, load it */
	auto itr = zone->npctable.find(npc_type_id);
	if (itr != zone->npctable.end()) {
		return itr->second;
	}

	std::string filter = fmt::format("id = {}", npc_type_id);

	if (bulk_load) {
		LogDebug("Performing bulk NPC Types load");

		filter = fmt::format(
			SQL(
				id IN (
					select npcID from spawnentry where spawngroupID IN (
						select spawngroupID from spawn2 where `zone` = '{}' and (`version` = {} OR `version` = -1)
					)
				)
			),
			zone->GetShortName(),
			zone->GetInstanceVersion()
		);
	}

	std::vector<uint32> npc_ids;

	for (NpcTypesRepository::NpcTypes &n : NpcTypesRepository::GetWhere((Database &) content_db, filter)) {
		NPCType *t;
		t = new NPCType;
		memset(t, 0, sizeof *t);

		t->npc_id = n.id;

		strn0cpy(t->name, n.name.c_str(), 50);

		t->level              = n.level;
		t->race               = n.race;
		t->class_             = n.class_;
		t->max_hp             = n.hp;
		t->current_hp         = n.hp;
		t->Mana               = n.mana;
		t->gender             = n.gender;
		t->texture            = n.texture;
		t->helmtexture        = n.helmtexture;
		t->herosforgemodel    = n.herosforgemodel;
		t->size               = n.size;
		t->loottable_id       = n.loottable_id;
		t->merchanttype       = n.merchant_id;
		t->alt_currency_type  = n.alt_currency_id;
		t->adventure_template = n.adventure_template_id;
		t->trap_template      = n.trap_template;
		t->attack_speed       = n.attack_speed;
		t->STR                = n.STR;
		t->STA                = n.STA;
		t->DEX                = n.DEX;
		t->AGI                = n.AGI;
		t->INT                = n._INT;
		t->WIS                = n.WIS;
		t->CHA                = n.CHA;
		t->MR                 = n.MR;
		t->CR                 = n.CR;
		t->DR                 = n.DR;
		t->FR                 = n.FR;
		t->PR                 = n.PR;
		t->Corrup             = n.Corrup;
		t->PhR                = n.PhR;
		t->min_dmg            = n.mindmg;
		t->max_dmg            = n.maxdmg;
		t->attack_count       = n.attack_count;

		if (!n.special_abilities.empty()) {
			strn0cpy(t->special_abilities, n.special_abilities.c_str(), 512);
		}
		else {
			t->special_abilities[0] = '\0';
		}


		t->npc_spells_id         = n.npc_spells_id;
		t->npc_spells_effects_id = n.npc_spells_effects_id;
		t->d_melee_texture1      = n.d_melee_texture1;
		t->d_melee_texture2      = n.d_melee_texture2;
		strn0cpy(t->ammo_idfile, n.ammo_idfile.c_str(), 30);
		t->prim_melee_type = n.prim_melee_type;
		t->sec_melee_type  = n.sec_melee_type;
		t->ranged_type     = n.ranged_type;
		t->runspeed        = n.runspeed;
		t->findable        = n.findable != 0;
		t->is_quest_npc    = n.isquest != 0;
		t->trackable       = n.trackable != 0;
		t->hp_regen        = n.hp_regen_rate;
		t->mana_regen      = n.mana_regen_rate;

		// set default value for aggroradius
		t->aggroradius = (int32) n.aggroradius;
		if (t->aggroradius <= 0) {
			t->aggroradius = 70;
		}

		t->assistradius = (int32) n.assistradius;
		if (t->assistradius <= 0) {
			t->assistradius = t->aggroradius;
		}

		if (n.bodytype > 0) {
			t->bodytype = n.bodytype;
		}
		else {
			t->bodytype = 0;
		}

		// facial features
		t->npc_faction_id   = n.npc_faction_id;
		t->luclinface       = n.face;
		t->hairstyle        = n.luclin_hairstyle;
		t->haircolor        = n.luclin_haircolor;
		t->eyecolor1        = n.luclin_eyecolor;
		t->eyecolor2        = n.luclin_eyecolor2;
		t->beardcolor       = n.luclin_beardcolor;
		t->beard            = n.luclin_beard;
		t->drakkin_heritage = n.drakkin_heritage;
		t->drakkin_tattoo   = n.drakkin_tattoo;
		t->drakkin_details  = n.drakkin_details;

		// armor tint
		uint32 armor_tint_id = n.armortint_id;
		t->armor_tint.Head.Color = (n.armortint_red & 0xFF) << 16;
		t->armor_tint.Head.Color |= (n.armortint_green & 0xFF) << 8;
		t->armor_tint.Head.Color |= (n.armortint_blue & 0xFF);
		t->armor_tint.Head.Color |= (t->armor_tint.Head.Color) ? (0xFF << 24) : 0;

		if (armor_tint_id != 0) {

			std::string armortint_query = StringFormat(
				"SELECT red1h, grn1h, blu1h, "
				"red2c, grn2c, blu2c, "
				"red3a, grn3a, blu3a, "
				"red4b, grn4b, blu4b, "
				"red5g, grn5g, blu5g, "
				"red6l, grn6l, blu6l, "
				"red7f, grn7f, blu7f, "
				"red8x, grn8x, blu8x, "
				"red9x, grn9x, blu9x "
				"FROM npc_types_tint WHERE id = %d",
				armor_tint_id
			);

			auto armortint_results = QueryDatabase(armortint_query);
			if (!armortint_results.Success() || armortint_results.RowCount() == 0) {
				armor_tint_id = 0;
			}
			else {
				auto& armorTint_row = armortint_results.begin();

				for (int index = EQ::textures::textureBegin; index <= EQ::textures::LastTexture; index++) {
					t->armor_tint.Slot[index].Color = Strings::ToInt(armorTint_row[index * 3]) << 16;
					t->armor_tint.Slot[index].Color |= Strings::ToInt(armorTint_row[index * 3 + 1]) << 8;
					t->armor_tint.Slot[index].Color |= Strings::ToInt(armorTint_row[index * 3 + 2]);
					t->armor_tint.Slot[index].Color |= (t->armor_tint.Slot[index].Color)
						? (0xFF << 24) : 0;
				}
			}
		}
		// Try loading npc_types tint fields if armor tint is 0 or query failed to get results
		if (armor_tint_id == 0) {
			for (int index = EQ::textures::armorChest; index < EQ::textures::materialCount; index++) {
				t->armor_tint.Slot[index].Color = t->armor_tint.Slot[0].Color; // odd way to 'zero-out' the array...
			}
		}

		t->see_invis        = n.see_invis;
		t->see_invis_undead = n.see_invis_undead != 0;    // Set see_invis_undead flag

		if (!RuleB(NPC, DisableLastNames) && !n.lastname.empty()) {
			strn0cpy(t->lastname, n.lastname.c_str(), sizeof(t->lastname));
		}

		t->qglobal                = n.qglobal != 0;    // qglobal
		t->AC                     = n.AC;
		t->npc_aggro              = n.npc_aggro != 0;
		t->spawn_limit            = n.spawn_limit;
		t->see_hide               = n.see_hide != 0;
		t->see_improved_hide      = n.see_improved_hide != 0;
		t->ATK                    = n.ATK;
		t->accuracy_rating        = n.Accuracy;
		t->avoidance_rating       = n.Avoidance;
		t->slow_mitigation        = n.slow_mitigation;
		t->maxlevel               = n.maxlevel;
		t->scalerate              = n.scalerate;
		t->private_corpse         = n.private_corpse != 0;
		t->unique_spawn_by_name   = n.unique_spawn_by_name != 0;
		t->underwater             = n.underwater != 0;
		t->emoteid                = n.emoteid;
		t->spellscale             = n.spellscale;
		t->healscale              = n.healscale;
		t->no_target_hotkey       = n.no_target_hotkey != 0;
		t->raid_target            = n.raid_target != 0;
		t->attack_delay           = n.attack_delay * 100; // TODO: fix DB
		t->light                  = (n.light & 0x0F);
		t->armtexture             = n.armtexture;
		t->bracertexture          = n.bracertexture;
		t->handtexture            = n.handtexture;
		t->legtexture             = n.legtexture;
		t->feettexture            = n.feettexture;
		t->ignore_despawn         = n.ignore_despawn != 0;
		t->show_name              = n.show_name != 0;
		t->untargetable           = n.untargetable != 0;
		t->charm_ac               = n.charm_ac;
		t->charm_min_dmg          = n.charm_min_dmg;
		t->charm_max_dmg          = n.charm_max_dmg;
		t->charm_attack_delay     = n.charm_attack_delay * 100; // TODO: fix DB
		t->charm_accuracy_rating  = n.charm_accuracy_rating;
		t->charm_avoidance_rating = n.charm_avoidance_rating;
		t->charm_atk              = n.charm_atk;
		t->skip_global_loot       = n.skip_global_loot != 0;
		t->rare_spawn             = n.rare_spawn != 0;
		t->stuck_behavior         = n.stuck_behavior;
		t->use_model              = n.model;
		t->flymode                = n.flymode;
		t->always_aggro           = n.always_aggro != 0;
		t->exp_mod                = n.exp_mod;
		t->skip_auto_scale        = false; // hardcoded here for now
		t->hp_regen_per_second    = n.hp_regen_per_second;
		t->heroic_strikethrough   = n.heroic_strikethrough;
		t->faction_amount         = n.faction_amount;
		t->keeps_sold_items       = n.keeps_sold_items;

		// If NPC with duplicate NPC id already in table,
		// free item we attempted to add.
		if (zone->npctable.find(t->npc_id) != zone->npctable.end()) {
			std::cerr << "Error loading duplicate NPC " << t->npc_id << std::endl;
			delete t;
			return nullptr;
		}

		zone->npctable[t->npc_id] = t;
		npc = t;

		// If NPC ID is not in npc_ids, add to vector
		if (!std::count(npc_ids.begin(), npc_ids.end(), t->npc_id)) {
			npc_ids.emplace_back(t->npc_id);
		}
	}

	DataBucket::BulkLoadEntities(DataBucketLoadType::NPC, npc_ids);

	return npc;
}

const NPCType* ZoneDatabase::GetMercType(uint32 id, uint16 raceid, uint32 clientlevel)
{
	//need to save based on merc_npc_type & client level
	uint32 merc_type_id = id * 100 + clientlevel;

	// If Merc is already in tree, return it.
	auto itr = zone->merctable.find(merc_type_id);
	if(itr != zone->merctable.end())
		return itr->second;

	//If the id is 0, return nullptr. (sanity check)
	if(id == 0)
        return nullptr;

	// Otherwise, load Merc data on demand
	std::string query = StringFormat("SELECT "
		"m_stats.merc_npc_type_id, "
		"'' AS name, "
		"m_stats.level, "
		"m_types.race_id, "
		"m_subtypes.class_id, "
		"m_stats.hp, "
		"m_stats.mana, "
		"0 AS gender, "
		"m_armorinfo.texture, "
		"m_armorinfo.helmtexture, "
		"m_stats.attack_delay, "
		"m_stats.STR, "
		"m_stats.STA, "
		"m_stats.DEX, "
		"m_stats.AGI, "
		"m_stats._INT, "
		"m_stats.WIS, "
		"m_stats.CHA, "
		"m_stats.MR, "
		"m_stats.CR, "
		"m_stats.DR, "
		"m_stats.FR, "
		"m_stats.PR, "
		"m_stats.Corrup, "
		"m_stats.mindmg, "
		"m_stats.maxdmg, "
		"m_stats.attack_count, "
		"m_stats.special_abilities, "
		"m_weaponinfo.d_melee_texture1, "
		"m_weaponinfo.d_melee_texture2, "
		"m_weaponinfo.prim_melee_type, "
		"m_weaponinfo.sec_melee_type, "
		"m_stats.runspeed, "
		"m_stats.hp_regen_rate, "
		"m_stats.mana_regen_rate, "
		"1 AS bodytype, "
		"m_armorinfo.armortint_id, "
		"m_armorinfo.armortint_red, "
		"m_armorinfo.armortint_green, "
		"m_armorinfo.armortint_blue, "
		"m_stats.AC, "
		"m_stats.ATK, "
		"m_stats.Accuracy, "
		"m_stats.statscale, "
		"m_stats.spellscale, "
		"m_stats.healscale "
		"FROM merc_stats m_stats "
		"INNER JOIN merc_armorinfo m_armorinfo "
		"ON m_stats.merc_npc_type_id = m_armorinfo.merc_npc_type_id "
		"AND m_armorinfo.minlevel <= m_stats.level AND m_armorinfo.maxlevel >= m_stats.level "
		"INNER JOIN merc_weaponinfo m_weaponinfo "
		"ON m_stats.merc_npc_type_id = m_weaponinfo.merc_npc_type_id "
		"AND m_weaponinfo.minlevel <= m_stats.level AND m_weaponinfo.maxlevel >= m_stats.level "
		"INNER JOIN merc_templates m_templates "
		"ON m_templates.merc_npc_type_id = m_stats.merc_npc_type_id "
		"INNER JOIN merc_types m_types "
		"ON m_templates.merc_type_id = m_types.merc_type_id "
		"INNER JOIN merc_subtypes m_subtypes "
		"ON m_templates.merc_subtype_id = m_subtypes.merc_subtype_id "
		"WHERE m_templates.merc_npc_type_id = %d AND m_stats.clientlevel = %d AND m_types.race_id = %d",
		id, clientlevel, raceid); //dual primary keys. one is ID, one is level.

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return nullptr;
	}

	const NPCType *npc = nullptr;

	// Process each row returned.
	for (auto& row = results.begin(); row != results.end(); ++row) {
		NPCType *tmpNPCType;
		tmpNPCType = new NPCType;
		memset(tmpNPCType, 0, sizeof *tmpNPCType);

		tmpNPCType->npc_id = Strings::ToInt(row[0]);

		strn0cpy(tmpNPCType->name, row[1], 50);

		tmpNPCType->level = Strings::ToInt(row[2]);
		tmpNPCType->race = Strings::ToInt(row[3]);
		tmpNPCType->class_ = Strings::ToInt(row[4]);
		tmpNPCType->max_hp = Strings::ToInt(row[5]);
		tmpNPCType->current_hp = tmpNPCType->max_hp;
		tmpNPCType->Mana = Strings::ToInt(row[6]);
		tmpNPCType->gender = Strings::ToInt(row[7]);
		tmpNPCType->texture = Strings::ToInt(row[8]);
		tmpNPCType->helmtexture = Strings::ToInt(row[9]);
		tmpNPCType->attack_delay = Strings::ToInt(row[10]) * 100; // TODO: fix DB
		tmpNPCType->STR = Strings::ToInt(row[11]);
		tmpNPCType->STA = Strings::ToInt(row[12]);
		tmpNPCType->DEX = Strings::ToInt(row[13]);
		tmpNPCType->AGI = Strings::ToInt(row[14]);
		tmpNPCType->INT = Strings::ToInt(row[15]);
		tmpNPCType->WIS = Strings::ToInt(row[16]);
		tmpNPCType->CHA = Strings::ToInt(row[17]);
		tmpNPCType->MR = Strings::ToInt(row[18]);
		tmpNPCType->CR = Strings::ToInt(row[19]);
		tmpNPCType->DR = Strings::ToInt(row[20]);
		tmpNPCType->FR = Strings::ToInt(row[21]);
		tmpNPCType->PR = Strings::ToInt(row[22]);
		tmpNPCType->Corrup = Strings::ToInt(row[23]);
		tmpNPCType->min_dmg = Strings::ToInt(row[24]);
		tmpNPCType->max_dmg = Strings::ToInt(row[25]);
		tmpNPCType->attack_count = Strings::ToInt(row[26]);

		if (row[27] != nullptr)
			strn0cpy(tmpNPCType->special_abilities, row[27], 512);
		else
			tmpNPCType->special_abilities[0] = '\0';

		tmpNPCType->d_melee_texture1 = Strings::ToUnsignedInt(row[28]);
		tmpNPCType->d_melee_texture2 = Strings::ToUnsignedInt(row[29]);
		tmpNPCType->prim_melee_type = Strings::ToInt(row[30]);
		tmpNPCType->sec_melee_type = Strings::ToInt(row[31]);
		tmpNPCType->runspeed = Strings::ToFloat(row[32]);

		tmpNPCType->hp_regen = Strings::ToInt(row[33]);
		tmpNPCType->mana_regen = Strings::ToInt(row[34]);

		tmpNPCType->aggroradius = RuleI(Mercs, AggroRadius);

		if (row[35] && strlen(row[35]))
			tmpNPCType->bodytype = (uint8)Strings::ToInt(row[35]);
		else
			tmpNPCType->bodytype = 1;

		uint32 armor_tint_id = Strings::ToInt(row[36]);
		tmpNPCType->armor_tint.Slot[0].Color = (Strings::ToInt(row[37]) & 0xFF) << 16;
		tmpNPCType->armor_tint.Slot[0].Color |= (Strings::ToInt(row[38]) & 0xFF) << 8;
		tmpNPCType->armor_tint.Slot[0].Color |= (Strings::ToInt(row[39]) & 0xFF);
		tmpNPCType->armor_tint.Slot[0].Color |= (tmpNPCType->armor_tint.Slot[0].Color) ? (0xFF << 24) : 0;

		if (armor_tint_id == 0)
			for (int index = EQ::textures::armorChest; index <= EQ::textures::LastTexture; index++)
				tmpNPCType->armor_tint.Slot[index].Color = tmpNPCType->armor_tint.Slot[0].Color;
		else if (tmpNPCType->armor_tint.Slot[0].Color == 0) {
			std::string armorTint_query = StringFormat("SELECT red1h, grn1h, blu1h, "
								   "red2c, grn2c, blu2c, "
								   "red3a, grn3a, blu3a, "
								   "red4b, grn4b, blu4b, "
								   "red5g, grn5g, blu5g, "
								   "red6l, grn6l, blu6l, "
								   "red7f, grn7f, blu7f, "
								   "red8x, grn8x, blu8x, "
								   "red9x, grn9x, blu9x "
								   "FROM npc_types_tint WHERE id = %d",
								   armor_tint_id);
			auto armorTint_results = QueryDatabase(armorTint_query);
			if (!results.Success() || results.RowCount() == 0)
				armor_tint_id = 0;
			else {
				auto& armorTint_row = results.begin();

				for (int index = EQ::textures::textureBegin; index <= EQ::textures::LastTexture; index++) {
					tmpNPCType->armor_tint.Slot[index].Color = Strings::ToInt(armorTint_row[index * 3]) << 16;
					tmpNPCType->armor_tint.Slot[index].Color |= Strings::ToInt(armorTint_row[index * 3 + 1]) << 8;
					tmpNPCType->armor_tint.Slot[index].Color |= Strings::ToInt(armorTint_row[index * 3 + 2]);
					tmpNPCType->armor_tint.Slot[index].Color |= (tmpNPCType->armor_tint.Slot[index].Color) ? (0xFF << 24) : 0;
				}
			}
		} else
			armor_tint_id = 0;

		tmpNPCType->AC = Strings::ToInt(row[40]);
		tmpNPCType->ATK = Strings::ToInt(row[41]);
		tmpNPCType->accuracy_rating = Strings::ToInt(row[42]);
		tmpNPCType->scalerate = Strings::ToInt(row[43]);
		tmpNPCType->spellscale = Strings::ToInt(row[44]);
		tmpNPCType->healscale = Strings::ToInt(row[45]);
		tmpNPCType->skip_global_loot = true;
		tmpNPCType->skip_auto_scale = true;

		// If Merc with duplicate NPC id already in table,
		// free item we attempted to add.
		if (zone->merctable.find(merc_type_id) != zone->merctable.end()) {
			delete tmpNPCType;
			return nullptr;
		}

		zone->merctable[merc_type_id] = tmpNPCType;
		npc = tmpNPCType;
	}

	return npc;
}

bool ZoneDatabase::LoadMercInfo(Client *client) {

	std::string query = StringFormat("SELECT MercID, Slot, Name, TemplateID, SuspendedTime, "
                                    "IsSuspended, TimerRemaining, Gender, MercSize, StanceID, HP, Mana, "
                                    "Endurance, Face, LuclinHairStyle, LuclinHairColor, "
                                    "LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, "
                                    "DrakkinHeritage, DrakkinTattoo, DrakkinDetails "
                                    "FROM mercs WHERE OwnerCharacterID = '%i' ORDER BY Slot", client->CharacterID());
    auto results = QueryDatabase(query);
    if (!results.Success())
        return false;

	if(results.RowCount() == 0)
		return false;

    for (auto& row = results.begin(); row != results.end(); ++row) {
        uint8 slot = Strings::ToInt(row[1]);

        if(slot >= MAXMERCS)
            continue;

        client->GetMercInfo(slot).mercid = Strings::ToInt(row[0]);
        client->GetMercInfo(slot).slot = slot;
        snprintf(client->GetMercInfo(slot).merc_name, 64, "%s", row[2]);
        client->GetMercInfo(slot).MercTemplateID = Strings::ToInt(row[3]);
        client->GetMercInfo(slot).SuspendedTime = Strings::ToInt(row[4]);
        client->GetMercInfo(slot).IsSuspended = Strings::ToInt(row[5]) == 1 ? true : false;
		client->GetMercInfo(slot).MercTimerRemaining = Strings::ToInt(row[6]);
		client->GetMercInfo(slot).Gender = Strings::ToInt(row[7]);
		client->GetMercInfo(slot).MercSize = Strings::ToFloat(row[8]);
		client->GetMercInfo(slot).State = 5;
		client->GetMercInfo(slot).Stance = Strings::ToInt(row[9]);
		client->GetMercInfo(slot).hp = Strings::ToInt(row[10]);
		client->GetMercInfo(slot).mana = Strings::ToInt(row[11]);
		client->GetMercInfo(slot).endurance = Strings::ToInt(row[12]);
		client->GetMercInfo(slot).face = Strings::ToInt(row[13]);
		client->GetMercInfo(slot).luclinHairStyle = Strings::ToInt(row[14]);
		client->GetMercInfo(slot).luclinHairColor = Strings::ToInt(row[15]);
		client->GetMercInfo(slot).luclinEyeColor = Strings::ToInt(row[16]);
		client->GetMercInfo(slot).luclinEyeColor2 = Strings::ToInt(row[17]);
		client->GetMercInfo(slot).luclinBeardColor = Strings::ToInt(row[18]);
		client->GetMercInfo(slot).luclinBeard = Strings::ToInt(row[19]);
		client->GetMercInfo(slot).drakkinHeritage = Strings::ToInt(row[20]);
		client->GetMercInfo(slot).drakkinTattoo = Strings::ToInt(row[21]);
		client->GetMercInfo(slot).drakkinDetails = Strings::ToInt(row[22]);
    }

	return true;
}

bool ZoneDatabase::LoadCurrentMerc(Client *client) {

	uint8 slot = client->GetMercSlot();

	if(slot > MAXMERCS)
        return false;

    std::string query = StringFormat("SELECT MercID, Name, TemplateID, SuspendedTime, "
                                    "IsSuspended, TimerRemaining, Gender, MercSize, StanceID, HP, "
                                    "Mana, Endurance, Face, LuclinHairStyle, LuclinHairColor, "
                                    "LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, "
                                    "LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails "
                                    "FROM mercs WHERE OwnerCharacterID = '%i' AND Slot = '%u'",
                                    client->CharacterID(), slot);
    auto results = database.QueryDatabase(query);

    if(!results.Success())
		return false;

	if(results.RowCount() == 0)
		return false;


    for (auto& row = results.begin(); row != results.end(); ++row) {
        client->GetMercInfo(slot).mercid = Strings::ToInt(row[0]);
        client->GetMercInfo(slot).slot = slot;
        snprintf(client->GetMercInfo(slot).merc_name, 64, "%s", row[1]);
        client->GetMercInfo(slot).MercTemplateID = Strings::ToInt(row[2]);
        client->GetMercInfo(slot).SuspendedTime = Strings::ToInt(row[3]);
        client->GetMercInfo(slot).IsSuspended = Strings::ToInt(row[4]) == 1? true: false;
        client->GetMercInfo(slot).MercTimerRemaining = Strings::ToInt(row[5]);
		client->GetMercInfo(slot).Gender = Strings::ToInt(row[6]);
		client->GetMercInfo(slot).MercSize = Strings::ToFloat(row[7]);
		client->GetMercInfo(slot).State = Strings::ToInt(row[8]);
		client->GetMercInfo(slot).hp = Strings::ToInt(row[9]);
		client->GetMercInfo(slot).mana = Strings::ToInt(row[10]);
		client->GetMercInfo(slot).endurance = Strings::ToInt(row[11]);
		client->GetMercInfo(slot).face = Strings::ToInt(row[12]);
		client->GetMercInfo(slot).luclinHairStyle = Strings::ToInt(row[13]);
		client->GetMercInfo(slot).luclinHairColor = Strings::ToInt(row[14]);
		client->GetMercInfo(slot).luclinEyeColor = Strings::ToInt(row[15]);
		client->GetMercInfo(slot).luclinEyeColor2 = Strings::ToInt(row[16]);
		client->GetMercInfo(slot).luclinBeardColor = Strings::ToInt(row[17]);
		client->GetMercInfo(slot).luclinBeard = Strings::ToInt(row[18]);
		client->GetMercInfo(slot).drakkinHeritage = Strings::ToInt(row[19]);
		client->GetMercInfo(slot).drakkinTattoo = Strings::ToInt(row[20]);
		client->GetMercInfo(slot).drakkinDetails = Strings::ToInt(row[21]);
	}

	return true;
}

bool ZoneDatabase::SaveMerc(Merc *merc) {
	Client *owner = merc->GetMercOwner();

	if(!owner)
		return false;

	if(merc->GetMercID() == 0)
	{
		// New merc record
		std::string query = StringFormat("INSERT INTO mercs "
		"(OwnerCharacterID, Slot, Name, TemplateID, "
		"SuspendedTime, IsSuspended, TimerRemaining, "
		"Gender, MercSize, StanceID, HP, Mana, Endurance, Face, "
		"LuclinHairStyle, LuclinHairColor, LuclinEyeColor, "
		"LuclinEyeColor2, LuclinBeardColor, LuclinBeard, "
		"DrakkinHeritage, DrakkinTattoo, DrakkinDetails) "
		"VALUES('%u', '%u', '%s', '%u', '%u', '%u', '%u', "
		"'%u', '%u', '%f', '%u', '%u', '%u', '%i', '%i', '%i', "
		"'%i', '%i', '%i', '%i', '%i', '%i', '%i')",
		merc->GetMercCharacterID(), owner->GetNumMercs(),
		merc->GetCleanName(), merc->GetMercTemplateID(),
		owner->GetMercInfo().SuspendedTime, merc->IsSuspended(),
		owner->GetMercInfo().MercTimerRemaining, merc->GetGender(),
		merc->GetSize(), merc->GetStance(), merc->GetHP(),
		merc->GetMana(), merc->GetEndurance(), merc->GetLuclinFace(),
		merc->GetHairStyle(), merc->GetHairColor(), merc->GetEyeColor1(),
		merc->GetEyeColor2(), merc->GetBeardColor(),
		merc->GetBeard(), merc->GetDrakkinHeritage(),
		merc->GetDrakkinTattoo(), merc->GetDrakkinDetails());

		auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			owner->Message(Chat::Red, results.ErrorMessage().c_str());
			return false;
		} else if (results.RowsAffected() != 1) {
			owner->Message(Chat::Red, "Unable to save merc to the database.");
			return false;
		}

		merc->SetMercID(results.LastInsertedID());
		merc->UpdateMercInfo(owner);
		database.SaveMercBuffs(merc);
		return true;
	}

	// Update existing merc record
	std::string query = StringFormat("UPDATE mercs SET OwnerCharacterID = '%u', Slot = '%u', "
	"Name = '%s', TemplateID = '%u', SuspendedTime = '%u', "
	"IsSuspended = '%u', TimerRemaining = '%u', Gender = '%u', MercSize = '%f', "
	"StanceID = '%u', HP = '%u', Mana = '%u', Endurance = '%u', "
	"Face = '%i', LuclinHairStyle = '%i', LuclinHairColor = '%i', "
	"LuclinEyeColor = '%i', LuclinEyeColor2 = '%i', LuclinBeardColor = '%i', "
	"LuclinBeard = '%i', DrakkinHeritage = '%i', DrakkinTattoo = '%i', "
	"DrakkinDetails = '%i' WHERE MercID = '%u'",
	merc->GetMercCharacterID(), owner->GetMercSlot(), merc->GetCleanName(),
	merc->GetMercTemplateID(), owner->GetMercInfo().SuspendedTime,
	merc->IsSuspended(), owner->GetMercInfo().MercTimerRemaining,
	merc->GetGender(), merc->GetSize(), merc->GetStance(), merc->GetHP(),
	merc->GetMana(), merc->GetEndurance(), merc->GetLuclinFace(),
	merc->GetHairStyle(), merc->GetHairColor(), merc->GetEyeColor1(),
	merc->GetEyeColor2(), merc->GetBeardColor(), merc->GetBeard(),
	merc->GetDrakkinHeritage(), merc->GetDrakkinTattoo(), merc->GetDrakkinDetails(),
	merc->GetMercID());

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		owner->Message(Chat::Red, results.ErrorMessage().c_str());
		return false;
	} else if (results.RowsAffected() != 1) {
		owner->Message(Chat::Red, "Unable to save merc to the database.");
		return false;
	}

	merc->UpdateMercInfo(owner);
	database.SaveMercBuffs(merc);

	return true;
}

void ZoneDatabase::SaveMercBuffs(Merc *merc) {

	Buffs_Struct *buffs = merc->GetBuffs();

	// Remove any existing buff saves
	std::string query   = StringFormat("DELETE FROM merc_buffs WHERE MercId = %u", merc->GetMercID());
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error While Deleting Merc Buffs before save: [{}]", results.ErrorMessage().c_str());
		return;
	}

	for (int buffCount = 0; buffCount <= BUFF_COUNT; buffCount++) {
		if (!IsValidSpell(buffs[buffCount].spellid)) {
			continue;
		}

        int IsPersistent = buffs[buffCount].persistant_buff? 1: 0;

        query = StringFormat("INSERT INTO merc_buffs (MercId, SpellId, CasterLevel, DurationFormula, "
                            "TicsRemaining, PoisonCounters, DiseaseCounters, CurseCounters, "
                            "CorruptionCounters, HitCount, MeleeRune, MagicRune, dot_rune, "
                            "caston_x, Persistent, caston_y, caston_z, ExtraDIChance) "
                            "VALUES (%u, %u, %u, %u, %u, %d, %u, %u, %u, %u, %u, %u, %u, %i, %u, %i, %i, %i);",
                            merc->GetMercID(), buffs[buffCount].spellid, buffs[buffCount].casterlevel,
                            spells[buffs[buffCount].spellid].buff_duration_formula, buffs[buffCount].ticsremaining,
                            CalculatePoisonCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            CalculateDiseaseCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            CalculateCurseCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            CalculateCorruptionCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            buffs[buffCount].hit_number, buffs[buffCount].melee_rune, buffs[buffCount].magic_rune,
                            buffs[buffCount].dot_rune, buffs[buffCount].caston_x, IsPersistent, buffs[buffCount].caston_y,
                            buffs[buffCount].caston_z, buffs[buffCount].ExtraDIChance);
        results = database.QueryDatabase(query);
        if(!results.Success()) {
      LogError("Error Saving Merc Buffs: [{}]", results.ErrorMessage().c_str());
            break;
        }
	}
}

void ZoneDatabase::LoadMercBuffs(Merc *merc) {
	Buffs_Struct *buffs = merc->GetBuffs();
	uint32 max_slots = merc->GetMaxBuffSlots();


	bool BuffsLoaded = false;
    std::string query = StringFormat("SELECT SpellId, CasterLevel, DurationFormula, TicsRemaining, "
                                    "PoisonCounters, DiseaseCounters, CurseCounters, CorruptionCounters, "
                                    "HitCount, MeleeRune, MagicRune, dot_rune, caston_x, Persistent, "
                                    "caston_y, caston_z, ExtraDIChance FROM merc_buffs WHERE MercId = %u",
                                    merc->GetMercID());
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		LogError("Error Loading Merc Buffs: [{}]", results.ErrorMessage().c_str());
		return;
	}

    int buffCount = 0;
    for (auto& row = results.begin(); row != results.end(); ++row, ++buffCount) {
        if(buffCount == BUFF_COUNT)
            break;

        buffs[buffCount].spellid = Strings::ToInt(row[0]);
        buffs[buffCount].casterlevel = Strings::ToInt(row[1]);
        buffs[buffCount].ticsremaining = Strings::ToInt(row[3]);

        if(CalculatePoisonCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = Strings::ToInt(row[4]);

        if(CalculateDiseaseCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = Strings::ToInt(row[5]);

        if(CalculateCurseCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = Strings::ToInt(row[6]);

		if(CalculateCorruptionCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = Strings::ToInt(row[7]);

        buffs[buffCount].hit_number = Strings::ToInt(row[8]);
		buffs[buffCount].melee_rune = Strings::ToInt(row[9]);
		buffs[buffCount].magic_rune = Strings::ToInt(row[10]);
		buffs[buffCount].dot_rune = Strings::ToInt(row[11]);
		buffs[buffCount].caston_x = Strings::ToInt(row[12]);
		buffs[buffCount].casterid = 0;

        bool IsPersistent = Strings::ToInt(row[13])? true: false;

        buffs[buffCount].caston_y = Strings::ToInt(row[13]);
        buffs[buffCount].caston_z = Strings::ToInt(row[14]);
        buffs[buffCount].ExtraDIChance = Strings::ToInt(row[15]);

        buffs[buffCount].persistant_buff = IsPersistent;

    }

	query = StringFormat("DELETE FROM merc_buffs WHERE MercId = %u", merc->GetMercID());
    results = database.QueryDatabase(query);
    if(!results.Success())
    LogError("Error Loading Merc Buffs: [{}]", results.ErrorMessage().c_str());

}

bool ZoneDatabase::DeleteMerc(uint32 merc_id) {

	if(merc_id == 0)
		return false;

	// TODO: These queries need to be ran together as a transaction.. ie,
	// if one or more fail then they all will fail to commit to the database.
	// ...Not all mercs will have buffs, so why is it required that both deletes succeed?
	std::string query = StringFormat("DELETE FROM merc_buffs WHERE MercId = '%u'", merc_id);
	auto results = database.QueryDatabase(query);
	if(!results.Success())
	{
		LogError("Error Deleting Merc Buffs: [{}]", results.ErrorMessage().c_str());
	}

	query = StringFormat("DELETE FROM mercs WHERE MercID = '%u'", merc_id);
	results = database.QueryDatabase(query);
	if(!results.Success())
	{
		LogError("Error Deleting Merc: [{}]", results.ErrorMessage().c_str());
		return false;
	}

	return true;
}

void ZoneDatabase::LoadMercEquipment(Merc *merc) {

	std::string query = StringFormat("SELECT item_id FROM merc_inventory "
                                    "WHERE merc_subtype_id = ("
                                    "SELECT merc_subtype_id FROM merc_subtypes "
                                    "WHERE class_id = '%u' AND tier_id = '%u') "
                                    "AND min_level <= %u AND max_level >= %u",
                                    merc->GetClass(), merc->GetTierID(),
                                    merc->GetLevel(), merc->GetLevel());
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		LogError("Error Loading Merc Inventory: [{}]", results.ErrorMessage().c_str());
		return;
	}

    int itemCount = 0;
    for(auto& row = results.begin(); row != results.end(); ++row) {
		if (itemCount == EQ::invslot::EQUIPMENT_COUNT)
            break;

        if(Strings::ToInt(row[0]) == 0)
            continue;

        merc->AddItem(itemCount, Strings::ToInt(row[0]));
        itemCount++;
    }
}

uint8 ZoneDatabase::GetGridType(uint32 grid, uint32 zoneid ) {

	std::string query = StringFormat("SELECT type FROM grid WHERE id = %i AND zoneid = %i", grid, zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        return 0;
	}

    if (results.RowCount() != 1)
        return 0;

    auto& row = results.begin();

	return Strings::ToInt(row[0]);
}

void ZoneDatabase::SaveMerchantTemp(uint32 npcid, uint32 slot, uint32 zone_id, uint32 instance_id, uint32 item, uint32 charges){

	std::string query = StringFormat("REPLACE INTO merchantlist_temp (npcid, slot, zone_id, instance_id, itemid, charges) "
                                    "VALUES(%d, %d, %d, %d, %d, %d)", npcid, slot, zone_id, instance_id, item, charges);
    QueryDatabase(query);
}

void ZoneDatabase::DeleteMerchantTemp(uint32 npcid, uint32 slot, uint32 zone_id, uint32 instance_id) {
	std::string query = StringFormat("DELETE FROM merchantlist_temp WHERE npcid=%d AND slot=%d AND zone_id=%d AND instance_id=%d",
			npcid, slot, zone_id, instance_id);
	QueryDatabase(query);
}

//New functions for timezone
uint32 ZoneDatabase::GetZoneTZ(uint32 zoneid, uint32 version) {

	std::string query = StringFormat("SELECT timezone FROM zone WHERE zoneidnumber = %i "
                                    "AND (version = %i OR version = 0) ORDER BY version DESC",
                                    zoneid, version);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return 0;
    }

    if (results.RowCount() == 0)
        return 0;

    auto& row = results.begin();
    return Strings::ToInt(row[0]);
}

bool ZoneDatabase::SetZoneTZ(uint32 zoneid, uint32 version, uint32 tz) {

	std::string query = StringFormat("UPDATE zone SET timezone = %i "
                                    "WHERE zoneidnumber = %i AND version = %i",
                                    tz, zoneid, version);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

    return results.RowsAffected() == 1;
}

void ZoneDatabase::RefreshGroupFromDB(Client *client){
	if (!client) {
		return;
	}

	Group *group = client->GetGroup();

	if (!group) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = groupActUpdate;

	strcpy(gu->yourname, client->GetName());
	GetGroupLeadershipInfo(group->GetID(), gu->leadersname, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &gu->leader_aas);
	gu->NPCMarkerID = group->GetNPCMarkerID();

	int index = 0;

	auto query = fmt::format(
		"SELECT name FROM group_id WHERE groupid = {}",
		group->GetID()
	);
	auto results = QueryDatabase(query);

	if (results.Success()) {
		for (auto row : results) {
			if (index >= 6) {
				continue;
			}

            if (!strcmp(client->GetName(), row[0])) {
				continue;
			}

			strcpy(gu->membername[index], row[0]);
			index++;
		}
	}

	client->QueuePacket(outapp);
	safe_delete(outapp);

	if (client->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
		group->NotifyMainTank(client, 1);
		group->NotifyPuller(client, 1);
	}

	group->NotifyMainAssist(client, 1);
	group->NotifyMarkNPC(client);
	group->NotifyAssistTarget(client);
	group->NotifyTankTarget(client);
	group->NotifyPullerTarget(client);
	group->SendMarkedNPCsToMember(client);
}

uint8 ZoneDatabase::GroupCount(uint32 groupid) {

	std::string query = StringFormat("SELECT count(charid) FROM group_id WHERE groupid = %d", groupid);
	auto results = QueryDatabase(query);
    if (!results.Success()) {
        return 0;
    }

    if (results.RowCount() == 0)
        return 0;

    auto& row = results.begin();

	return Strings::ToInt(row[0]);
}

uint8 ZoneDatabase::RaidGroupCount(uint32 raidid, uint32 groupid) {

	std::string query = StringFormat("SELECT count(charid) FROM raid_members "
                                    "WHERE raidid = %d AND groupid = %d;", raidid, groupid);
    auto results = QueryDatabase(query);

    if (!results.Success()) {
        return 0;
    }

    if (results.RowCount() == 0)
        return 0;

    auto& row = results.begin();

	return Strings::ToInt(row[0]);
 }

int64 ZoneDatabase::GetBlockedSpellsCount(uint32 zone_id)
{
	return BlockedSpellsRepository::Count(
		*this,
		fmt::format(
			"zoneid = {} {}",
			zone_id,
			ContentFilterCriteria::apply()
		)
	);
}

bool ZoneDatabase::LoadBlockedSpells(int64 blocked_spells_count, ZoneSpellsBlocked* into, uint32 zone_id)
{
	LogInfo("Loading Blocked Spells from database for {} ({}).", zone_store.GetZoneName(zone_id, true), zone_id);

	const auto& l = BlockedSpellsRepository::GetWhere(
		*this,
		fmt::format(
			"zoneid = {} {} ORDER BY id ASC",
			zone_id,
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return true;
	}

	int64 i = 0;

	for (const auto& e : l) {
		if (i >= blocked_spells_count) {
			LogError(
				"Blocked spells count of {} exceeded for {} ({}).",
				blocked_spells_count,
				zone_store.GetZoneName(zone_id, true),
				zone_id
			);
			break;
		}

		memset(&into[i], 0, sizeof(ZoneSpellsBlocked));
		into[i].spellid      = e.spellid;
		into[i].type         = e.type;
		into[i].m_Location   = glm::vec3(e.x, e.y, e.z);
		into[i].m_Difference = glm::vec3(e.x_diff, e.y_diff, e.z_diff);
		strn0cpy(into[i].message, e.message.c_str(), sizeof(into[i].message));
		i++;
	}

	return true;
}

int ZoneDatabase::getZoneShutDownDelay(uint32 zoneID, uint32 version)
{
	auto z = GetZoneVersionWithFallback(zoneID, version);

    return z ? z->shutdowndelay : RuleI(Zone, AutoShutdownDelay);
}

uint32 ZoneDatabase::GetKarma(uint32 acct_id)
{
    std::string query = StringFormat("SELECT `karma` FROM `account` WHERE `id` = '%i' LIMIT 1", acct_id);
    auto results = QueryDatabase(query);
	if (!results.Success())
		return 0;

	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToInt(row[0]);
	}

	return 0;
}

void ZoneDatabase::UpdateKarma(uint32 acct_id, uint32 amount)
{
	std::string query = StringFormat("UPDATE account SET karma = %i WHERE id = %i", amount, acct_id);
    QueryDatabase(query);
}

void ZoneDatabase::ListAllInstances(Client* client, uint32 character_id)
{
	if (!client) {
		return;
	}

	std::string query = fmt::format(
		"SELECT instance_list.id, zone, version, start_time, duration, never_expires "
		"FROM instance_list JOIN instance_list_player "
		"ON instance_list.id = instance_list_player.id "
		"WHERE instance_list_player.charid = {}",
		character_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	auto character_name = database.GetCharNameByID(character_id);
	bool is_same_client = client->CharacterID() == character_id;
	if (character_name.empty()) {
		client->Message(
			Chat::White,
			fmt::format(
				"Character ID '{}' does not exist.",
				character_id
			).c_str()
		);
		return;
	}

	if (!results.RowCount()) {
		client->Message(
			Chat::White,
			fmt::format(
				"{} not in any Instances.",
				(
					is_same_client ?
					"You are" :
					fmt::format(
						"{} ({}) is",
						character_name,
						character_id
					)
				)
			).c_str()
		);
		return;
	}

	client->Message(
		Chat::White,
		fmt::format(
			"{} in the following Instances.",
			(
				is_same_client ?
				"You are" :
				fmt::format(
					"{} ({}) is",
					character_name,
					character_id
				)
			)
		).c_str()
	);

	uint32 instance_count = 0;
	for (auto row : results) {
		auto instance_id = Strings::ToUnsignedInt(row[0]);
		auto zone_id = Strings::ToUnsignedInt(row[1]);
		auto version = Strings::ToUnsignedInt(row[2]);
		auto start_time = Strings::ToUnsignedInt(row[3]);
		auto duration = Strings::ToUnsignedInt(row[4]);
		auto never_expires = Strings::ToInt(row[5]) ? true : false;
		std::string remaining_time_string = "Never";
		timeval time_value;
		gettimeofday(&time_value, nullptr);
		auto current_time = time_value.tv_sec;
		auto remaining_time = ((start_time + duration) - current_time);
		if (!never_expires) {
			if (remaining_time > 0) {
				remaining_time_string = Strings::SecondsToTime(remaining_time);
			} else {
				remaining_time_string = "Already Expired";
			}
		}

		client->Message(
			Chat::White,
			fmt::format("Instance {} | Zone: {} ({}){}",
				instance_id,
				ZoneLongName(zone_id),
				zone_id,
				(
					version ?
					fmt::format(
						" Version: {}",
						version
					) :
					""
				)
			).c_str()
		);

		client->Message(
			Chat::White,
			fmt::format(
				"Instance {} | Expires: {}",
				instance_id,
				remaining_time_string,
				remaining_time
			).c_str()
		);

		instance_count++;
	}

	client->Message(
		Chat::White,
		fmt::format(
			"{} in {} Instance{}.",
			(
				is_same_client ?
				"You are" :
				fmt::format(
					"{} ({}) is",
					character_name,
					character_id
				)
			),
			instance_count,
			instance_count != 1 ? "s" : ""
		).c_str()
	);
}

void ZoneDatabase::QGlobalPurge()
{
	const std::string query = "DELETE FROM quest_globals WHERE expdate < UNIX_TIMESTAMP()";
	database.QueryDatabase(query);
}

void ZoneDatabase::LoadAltCurrencyValues(uint32 char_id, std::map<uint32, uint32> &currency) {

	std::string query = StringFormat("SELECT currency_id, amount "
                                    "FROM character_alt_currency "
                                    "WHERE char_id = '%u'", char_id);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    for (auto& row = results.begin(); row != results.end(); ++row)
        currency[Strings::ToInt(row[0])] = Strings::ToInt(row[1]);

}

void ZoneDatabase::UpdateAltCurrencyValue(uint32 char_id, uint32 currency_id, uint32 value) {

	std::string query = StringFormat("REPLACE INTO character_alt_currency (char_id, currency_id, amount) "
                                    "VALUES('%u', '%u', '%u')", char_id, currency_id, value);
	database.QueryDatabase(query);

}

void ZoneDatabase::SaveBuffs(Client *client) {

	// delete the character buffs
	CharacterBuffsRepository::DeleteWhere(database, fmt::format("character_id = {}", client->CharacterID()));

	// get the character buffs
	uint32 buff_count = client->GetMaxBuffSlots();
	Buffs_Struct *buffs = client->GetBuffs();

	// character buffs struct
	auto b = CharacterBuffsRepository::NewEntity();

	// vector of character buffs
	std::vector<CharacterBuffsRepository::CharacterBuffs> character_buffs = {};

	// count the number of buffs that are valid
	int character_buff_count = 0;
	for (int index = 0; index < buff_count; index++) {
		if (!IsValidSpell(buffs[index].spellid)) {
			continue;
		}
		character_buff_count++;
	}

	// allocate memory for the character buffs
	character_buffs.reserve(character_buff_count);

	for (int index = 0; index < buff_count; index++) {
		if (!IsValidSpell(buffs[index].spellid)) {
			continue;
		}

		// fill in the buff struct
		b.character_id   = client->CharacterID();
		b.slot_id        = index;
		b.spell_id       = buffs[index].spellid;
		b.caster_level   = buffs[index].casterlevel;
		b.caster_name    = buffs[index].caster_name;
		b.ticsremaining  = buffs[index].ticsremaining;
		b.counters       = buffs[index].counters;
		b.numhits        = buffs[index].hit_number;
		b.melee_rune     = buffs[index].melee_rune;
		b.magic_rune     = buffs[index].magic_rune;
		b.persistent     = buffs[index].persistant_buff;
		b.dot_rune       = buffs[index].dot_rune;
		b.caston_x       = buffs[index].caston_x;
		b.caston_y       = buffs[index].caston_y;
		b.caston_z       = buffs[index].caston_z;
		b.ExtraDIChance  = buffs[index].ExtraDIChance;
		b.instrument_mod = buffs[index].instrument_mod;

		// add the buff to the vector
		character_buffs.emplace_back(b);
	}

	// insert the buffs into the database
	if (!character_buffs.empty()) {
		CharacterBuffsRepository::InsertMany(database, character_buffs);
	}
}

void ZoneDatabase::LoadBuffs(Client *client)
{

	Buffs_Struct *buffs = client->GetBuffs();
	uint32 max_slots = client->GetMaxBuffSlots();

	for (int index = 0; index < max_slots; ++index)
		buffs[index].spellid = SPELL_UNKNOWN;

	std::string query = StringFormat("SELECT spell_id, slot_id, caster_level, caster_name, ticsremaining, "
					 "counters, numhits, melee_rune, magic_rune, persistent, dot_rune, "
					 "caston_x, caston_y, caston_z, ExtraDIChance, instrument_mod "
					 "FROM `character_buffs` WHERE `character_id` = '%u'",
					 client->CharacterID());
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto& row = results.begin(); row != results.end(); ++row) {
		uint32 slot_id = Strings::ToUnsignedInt(row[1]);
		if (slot_id >= client->GetMaxBuffSlots())
			continue;

		uint32 spell_id = Strings::ToUnsignedInt(row[0]);
		if (!IsValidSpell(spell_id))
			continue;

		Client *caster = entity_list.GetClientByName(row[3]);
		uint32 caster_level = Strings::ToInt(row[2]);
		int32 ticsremaining = Strings::ToInt(row[4]);
		uint32 counters = Strings::ToUnsignedInt(row[5]);
		uint32 hit_number = Strings::ToUnsignedInt(row[6]);
		uint32 melee_rune = Strings::ToUnsignedInt(row[7]);
		uint32 magic_rune = Strings::ToUnsignedInt(row[8]);
		uint8 persistent = Strings::ToUnsignedInt(row[9]);
		uint32 dot_rune = Strings::ToUnsignedInt(row[10]);
		int32 caston_x = Strings::ToUnsignedInt(row[11]);
		int32 caston_y = Strings::ToUnsignedInt(row[12]);
		int32 caston_z = Strings::ToUnsignedInt(row[13]);
		int32 ExtraDIChance = Strings::ToUnsignedInt(row[14]);
		uint32 instrument_mod = Strings::ToUnsignedInt(row[15]);

		buffs[slot_id].spellid = spell_id;
		buffs[slot_id].casterlevel = caster_level;

		if (caster) {
			buffs[slot_id].casterid = caster->GetID();
			strcpy(buffs[slot_id].caster_name, caster->GetName());
			buffs[slot_id].client = true;
		} else {
			buffs[slot_id].casterid = 0;
			strcpy(buffs[slot_id].caster_name, "");
			buffs[slot_id].client = false;
		}

		buffs[slot_id].ticsremaining = ticsremaining;
		buffs[slot_id].counters = counters;
		buffs[slot_id].hit_number = hit_number;
		buffs[slot_id].melee_rune = melee_rune;
		buffs[slot_id].magic_rune = magic_rune;
		buffs[slot_id].persistant_buff = persistent ? true : false;
		buffs[slot_id].dot_rune = dot_rune;
		buffs[slot_id].caston_x = caston_x;
		buffs[slot_id].caston_y = caston_y;
		buffs[slot_id].caston_z = caston_z;
		buffs[slot_id].ExtraDIChance = ExtraDIChance;
		buffs[slot_id].RootBreakChance = 0;
		buffs[slot_id].virus_spread_time = 0;
		buffs[slot_id].UpdateClient = false;
		buffs[slot_id].instrument_mod = instrument_mod;
	}

	// We load up to the most our client supports
	max_slots = EQ::spells::StaticLookup(client->ClientVersion())->LongBuffs;
	for (int index = 0; index < max_slots; ++index) {
		if (!IsValidSpell(buffs[index].spellid))
			continue;

		for (int effectIndex = 0; effectIndex < EFFECT_COUNT; ++effectIndex) {

			if (spells[buffs[index].spellid].effect_id[effectIndex] == SE_Charm) {
				buffs[index].spellid = SPELL_UNKNOWN;
				break;
			}

			if (spells[buffs[index].spellid].effect_id[effectIndex] == SE_Illusion) {
				if (buffs[index].persistant_buff)
					break;

				buffs[index].spellid = SPELL_UNKNOWN;
				break;
			}
		}
	}
}

void ZoneDatabase::SaveAuras(Client *c)
{
	auto query = StringFormat("DELETE FROM `character_auras` WHERE `id` = %u", c->CharacterID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	const auto &auras = c->GetAuraMgr();
	for (int i = 0; i < auras.count; ++i) {
		auto aura = auras.auras[i].aura;
		if (aura && aura->AuraZones()) {
			query = StringFormat("INSERT INTO `character_auras` (id, slot, spell_id) VALUES(%u, %d, %d)",
					     c->CharacterID(), i, aura->GetAuraID());
			auto results = database.QueryDatabase(query);
			if (!results.Success())
				return;
		}
	}
}

void ZoneDatabase::LoadAuras(Client *c)
{
	auto query = StringFormat("SELECT `spell_id` FROM `character_auras` WHERE `id` = %u ORDER BY `slot`", c->CharacterID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	for (auto& row = results.begin(); row != results.end(); ++row)
		c->MakeAura(Strings::ToInt(row[0]));
}

void ZoneDatabase::SavePetInfo(Client *client)
{
	PetInfo *petinfo = nullptr;

	// Pet Info
	std::vector<CharacterPetInfoRepository::CharacterPetInfo> pet_infos = {};
	CharacterPetInfoRepository::CharacterPetInfo pet_info = {};

	// Pet buffs
	std::vector<CharacterPetBuffsRepository::CharacterPetBuffs> pet_buffs = {};
	CharacterPetBuffsRepository::CharacterPetBuffs pet_buff = {};

	// Pet inventory
	std::vector<CharacterPetInventoryRepository::CharacterPetInventory> inventory = {};
	CharacterPetInventoryRepository::CharacterPetInventory item = {};

	// Loop through pet types
	for (int pet = 0; pet < 2; pet++) {
		petinfo = client->GetPetInfo(pet);
		if (!petinfo) {
			continue;
		}

		// build pet info into struct
		pet_info.char_id  = client->CharacterID();
		pet_info.pet      = pet;
		pet_info.petname  = petinfo->Name;
		pet_info.petpower = petinfo->petpower;
		pet_info.spell_id = petinfo->SpellID;
		pet_info.hp       = petinfo->HP;
		pet_info.mana     = petinfo->Mana;
		pet_info.size     = petinfo->size;
		pet_info.taunting = (petinfo->taunting) ? 1 : 0;

		// add pet info to vector
		pet_infos.push_back(pet_info);

		// build pet buffs into struct
		int pet_buff_count = 0;
		// Guard against setting the maximum pet slots above the client allowed maximum.
		int max_slots = RuleI(Spells, MaxTotalSlotsPET) > PET_BUFF_COUNT ? PET_BUFF_COUNT : RuleI(Spells, MaxTotalSlotsPET);

		// count pet buffs
		for (int index = 0; index < max_slots; index++) {
			if (!IsValidSpell(petinfo->Buffs[index].spellid)) {
				continue;
			}
			pet_buff_count++;
		}

		// reserve space for pet buffs
		pet_buffs.reserve(pet_buff_count);

		// loop through pet buffs
		for (int index = 0; index < max_slots; index++) {
			if (!IsValidSpell(petinfo->Buffs[index].spellid)) {
				continue;
			}

			pet_buff.char_id        = client->CharacterID();
			pet_buff.pet            = pet;
			pet_buff.slot           = index;
			pet_buff.spell_id       = petinfo->Buffs[index].spellid;
			pet_buff.caster_level   = petinfo->Buffs[index].level;
			pet_buff.ticsremaining  = petinfo->Buffs[index].duration;
			pet_buff.counters       = petinfo->Buffs[index].counters;
			pet_buff.instrument_mod = petinfo->Buffs[index].bard_modifier;

			// add pet buffs to vector
			pet_buffs.push_back(pet_buff);
		}

		// build pet inventory into struct
		int pet_inventory_count = 0;
		for (int index = EQ::invslot::EQUIPMENT_BEGIN; index <= EQ::invslot::EQUIPMENT_END; index++) {
			if (!petinfo->Items[index]) {
				continue;
			}
			pet_inventory_count++;
		}

		// reserve space for pet inventory
		inventory.reserve(pet_inventory_count);

		// loop through pet inventory
		for (int index = EQ::invslot::EQUIPMENT_BEGIN; index <= EQ::invslot::EQUIPMENT_END; index++) {
			if (!petinfo->Items[index]) {
				continue;
			}

			item.char_id = client->CharacterID();
			item.pet     = pet;
			item.slot    = index;
			item.item_id = petinfo->Items[index];

			// add pet inventory to vector
			inventory.push_back(item);
		}
	}

	// Delete existing pet info
	CharacterPetInfoRepository::DeleteWhere(database, fmt::format("char_id = {}", client->CharacterID()));

	// insert pet info into database
	if (!pet_infos.empty()) {
		CharacterPetInfoRepository::InsertMany(database, pet_infos);
	}

	// Delete existing pet buffs
	CharacterPetBuffsRepository::DeleteWhere(database, fmt::format("char_id = {}", client->CharacterID()));

	// insert pet buffs into database
	if (!pet_buffs.empty()) {
		CharacterPetBuffsRepository::InsertMany(database, pet_buffs);
	}

	// Delete existing pet inventory
	CharacterPetInventoryRepository::DeleteWhere(database, fmt::format("char_id = {}", client->CharacterID()));

	// insert pet inventory into database
	if (!inventory.empty()) {
		CharacterPetInventoryRepository::InsertMany(database, inventory);
	}
}

void ZoneDatabase::RemoveTempFactions(Client *client) {

	std::string query = StringFormat("DELETE FROM faction_values "
                                    "WHERE temp = 1 AND char_id = %u",
                                    client->CharacterID());
	QueryDatabase(query);
}

void ZoneDatabase::UpdateItemRecast(uint32 character_id, uint32 recast_type, uint32 timestamp)
{
	const auto query = fmt::format(
		"REPLACE INTO character_item_recast (id, recast_type, timestamp) VALUES ({}, {}, {})",
		character_id,
		recast_type,
		timestamp
	);
	QueryDatabase(query);
}

void ZoneDatabase::DeleteItemRecast(uint32 character_id, uint32 recast_type)
{
	const auto query = fmt::format(
		"DELETE FROM character_item_recast WHERE id = {} AND recast_type = {}",
		character_id,
		recast_type
	);
	QueryDatabase(query);
}

void ZoneDatabase::LoadPetInfo(Client *client)
{

	// Load current pet and suspended pet
	PetInfo *petinfo = client->GetPetInfo(0);
	PetInfo *suspended = client->GetPetInfo(1);

	memset(petinfo, 0, sizeof(PetInfo));
	memset(suspended, 0, sizeof(PetInfo));

	std::string query = StringFormat("SELECT `pet`, `petname`, `petpower`, `spell_id`, "
					 "`hp`, `mana`, `size` , `taunting` FROM `character_pet_info` "
					 "WHERE `char_id` = %u",
					 client->CharacterID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	PetInfo *pi;
	for (auto& row = results.begin(); row != results.end(); ++row) {
		uint16 pet = Strings::ToInt(row[0]);

		if (pet == 0)
			pi = petinfo;
		else if (pet == 1)
			pi = suspended;
		else
			continue;

		strncpy(pi->Name, row[1], 64);
		pi->petpower = Strings::ToInt(row[2]);
		pi->SpellID = Strings::ToInt(row[3]);
		pi->HP = Strings::ToUnsignedInt(row[4]);
		pi->Mana = Strings::ToUnsignedInt(row[5]);
		pi->size = Strings::ToFloat(row[6]);
		pi->taunting = (bool) Strings::ToInt(row[7]);
	}

	query = StringFormat("SELECT `pet`, `slot`, `spell_id`, `caster_level`, `castername`, "
			     "`ticsremaining`, `counters`, `instrument_mod` FROM `character_pet_buffs` "
			     "WHERE `char_id` = %u",
			     client->CharacterID());
	results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto& row = results.begin(); row != results.end(); ++row) {
		uint16 pet = Strings::ToInt(row[0]);
		if (pet == 0)
			pi = petinfo;
		else if (pet == 1)
			pi = suspended;
		else
			continue;

		uint32 slot_id = Strings::ToUnsignedInt(row[1]);
		if (slot_id >= RuleI(Spells, MaxTotalSlotsPET))
			continue;

		uint32 spell_id = Strings::ToUnsignedInt(row[2]);
		if (!IsValidSpell(spell_id))
			continue;

		uint32 caster_level = Strings::ToInt(row[3]);
		int caster_id = 0;
		// The castername field is currently unused
		int32 ticsremaining = Strings::ToInt(row[5]);
		uint32 counters = Strings::ToUnsignedInt(row[6]);
		uint8 bard_mod = Strings::ToUnsignedInt(row[7]);

		pi->Buffs[slot_id].spellid = spell_id;
		pi->Buffs[slot_id].level = caster_level;
		pi->Buffs[slot_id].player_id = caster_id;
		pi->Buffs[slot_id].effect_type = 2; // Always 2 in buffs struct for real buffs

		pi->Buffs[slot_id].duration = ticsremaining;
		pi->Buffs[slot_id].counters = counters;
		pi->Buffs[slot_id].bard_modifier = bard_mod;
	}

	query = StringFormat("SELECT `pet`, `slot`, `item_id` "
			     "FROM `character_pet_inventory` "
			     "WHERE `char_id`=%u",
			     client->CharacterID());
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto& row = results.begin(); row != results.end(); ++row) {
		uint16 pet = Strings::ToInt(row[0]);
		if (pet == 0)
			pi = petinfo;
		else if (pet == 1)
			pi = suspended;
		else
			continue;

		int slot = Strings::ToInt(row[1]);
		if (slot < EQ::invslot::EQUIPMENT_BEGIN || slot > EQ::invslot::EQUIPMENT_END)
			continue;

		pi->Items[slot] = Strings::ToUnsignedInt(row[2]);
	}
}

bool ZoneDatabase::GetFactionData(FactionMods* fm, uint32 class_mod, uint32 race_mod, uint32 deity_mod, int32 faction_id) {
	if (faction_id <= 0 || faction_id > (int32) max_faction)
		return false;

	if (faction_array[faction_id] == 0){
		return false;
	}

	fm->base = faction_array[faction_id]->base;
	fm->min = faction_array[faction_id]->min; // The lowest your personal earned faction can go - before race/class/deity adjustments.
	fm->max = faction_array[faction_id]->max; // The highest your personal earned faction can go - before race/class/deity adjustments.

	if(class_mod > 0) {
		char str[32];
		sprintf(str, "c%u", class_mod);

		std::map<std::string, int16>::const_iterator iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->class_mod = iter->second;
		} else {
			fm->class_mod = 0;
		}
	} else {
		fm->class_mod = 0;
	}

	if(race_mod > 0) {
		char str[32];
		sprintf(str, "r%u", race_mod);

		auto iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->race_mod = iter->second;
		} else {
			fm->race_mod = 0;
		}
	} else {
		fm->race_mod = 0;
	}

	if(deity_mod > 0) {
		char str[32];
		sprintf(str, "d%u", deity_mod);

		auto iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->deity_mod = iter->second;
		} else {
			fm->deity_mod = 0;
		}
	} else {
		fm->deity_mod = 0;
	}

	return true;
}

//o--------------------------------------------------------------
//| Name: GetFactionName; Dec. 16
//o--------------------------------------------------------------
//| Notes: Retrieves the name of the specified faction .Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::GetFactionName(int32 faction_id, char* name, uint32 buflen) {
	if ((faction_id <= 0) || faction_id > int32(max_faction) ||(faction_array[faction_id] == 0))
		return false;
	if (faction_array[faction_id]->name[0] != 0) {
		strn0cpy(name, faction_array[faction_id]->name, buflen);
		return true;
	}
	return false;

}

std::string ZoneDatabase::GetFactionName(int32 faction_id)
{
	std::string faction_name;
	if (
		faction_id <= 0 ||
		 faction_id > static_cast<int>(max_faction) ||
		 !faction_array[faction_id]
	) {
		return faction_name;
	}

	faction_name = faction_array[faction_id]->name;

	return faction_name;
}

//o--------------------------------------------------------------
//| Name: GetNPCFactionList; Dec. 16, 2001
//o--------------------------------------------------------------
//| Purpose: Gets a list of faction_id's and values bound to the npc_id. Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::GetNPCFactionList(uint32 npcfaction_id, int32* faction_id, int32* value, uint8* temp, int32* primary_faction) {
	if (npcfaction_id <= 0) {
		if (primary_faction)
			*primary_faction = npcfaction_id;
		return true;
	}
	const NPCFactionList* nfl = GetNPCFactionEntry(npcfaction_id);
	if (!nfl)
		return false;
	if (primary_faction)
		*primary_faction = nfl->primaryfaction;
	for (int i=0; i<MAX_NPC_FACTIONS; i++) {
		faction_id[i] = nfl->factionid[i];
		value[i] = nfl->factionvalue[i];
		temp[i] = nfl->factiontemp[i];
	}
	return true;
}

//o--------------------------------------------------------------
//| Name: SetCharacterFactionLevel; Dec. 20, 2001
//o--------------------------------------------------------------
//| Purpose: Update characters faction level with specified faction_id to specified value. Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list)
{

	std::string query;

	if(temp == 2)
		temp = 0;

	if(temp == 3)
		temp = 1;

	query = StringFormat("INSERT INTO `faction_values` "
						"(`char_id`, `faction_id`, `current_value`, `temp`) "
						"VALUES (%i, %i, %i, %i) "
						"ON DUPLICATE KEY UPDATE `current_value`=%i,`temp`=%i",
						char_id, faction_id, value, temp, value, temp);
    auto results = QueryDatabase(query);

	if (!results.Success())
		return false;
	else
		val_list[faction_id] = value;

	return true;
}

bool ZoneDatabase::LoadFactionData()
{
	std::string query("SELECT MAX(`id`) FROM `faction_list`");

	auto faction_max_results = QueryDatabase(query);
	if (!faction_max_results.Success() || faction_max_results.RowCount() == 0) {
		return false;
	}

    auto& fmr_row = faction_max_results.begin();

	max_faction = Strings::ToUnsignedInt(fmr_row[0]);
	faction_array = new Faction *[max_faction + 1];

	memset(faction_array, 0, (sizeof(Faction*) * (max_faction + 1)));

	std::vector<std::string> faction_ids;

	// load factions
    query = "SELECT `id`, `name`, `base` FROM `faction_list`";

    auto faction_results = QueryDatabase(query);
    if (!faction_results.Success()) {
        return false;
    }

	for (auto fr_row : faction_results) {

		uint32 index = Strings::ToUnsignedInt(fr_row[0]);
		if (index > max_faction) {
			Log(Logs::General, Logs::Error, "Faction '%u' is out-of-bounds for faction array size!", index);
			continue;
		}

		// this should never hit since `id` is keyed..but, it alleviates any risk of lost pointers
		if (faction_array[index] != nullptr) {
			Log(Logs::General, Logs::Error, "Faction '%u' has already been assigned! (Duplicate Entry)", index);
			continue;
		}

		faction_array[index] = new Faction;
		strn0cpy(faction_array[index]->name, fr_row[1], 50);
		faction_array[index]->base = Strings::ToInt(fr_row[2]);
		faction_array[index]->min = MIN_PERSONAL_FACTION;
		faction_array[index]->max = MAX_PERSONAL_FACTION;

		faction_ids.push_back(fr_row[0]);
	}

	LogInfo("Loaded [{}] faction(s)", Strings::Commify(std::to_string(faction_ids.size())));

	const std::string faction_id_criteria(Strings::Implode(",", faction_ids));

	// load faction mins/maxes
	query = fmt::format("SELECT `client_faction_id`, `min`, `max` FROM `faction_base_data` WHERE `client_faction_id` IN ({})", faction_id_criteria);

	auto base_results = QueryDatabase(query);
	if (base_results.Success()) {

		for (auto br_row : base_results) {

			uint32 index = Strings::ToUnsignedInt(br_row[0]);
			if (index > max_faction) {
				LogError("Faction [{}] is out-of-bounds for faction array size in Base adjustment!", index);
				continue;
			}

			if (faction_array[index] == nullptr) {
				LogError("Faction [{}] does not exist for Base adjustment!", index);
				continue;
			}

			faction_array[index]->min = Strings::ToInt(br_row[1]);
			faction_array[index]->max = Strings::ToInt(br_row[2]);
		}

		LogInfo("Loaded [{}] faction base(s)", Strings::Commify(std::to_string(base_results.RowCount())));
	}
	else {
		LogInfo("Unable to load Faction Base data...");
	}

	// load race, class and deity modifiers
	query = fmt::format("SELECT `faction_id`, `mod`, `mod_name` FROM `faction_list_mod` WHERE `faction_id` IN ({})", faction_id_criteria);

	auto modifier_results = QueryDatabase(query);
	if (modifier_results.Success()) {

		for (auto mr_row : modifier_results) {

			uint32 index = Strings::ToUnsignedInt(mr_row[0]);
			if (index > max_faction) {
				Log(Logs::General, Logs::Error, "Faction '%u' is out-of-bounds for faction array size in Modifier adjustment!", index);
				continue;
			}

			if (faction_array[index] == nullptr) {
				Log(Logs::General, Logs::Error, "Faction '%u' does not exist for Modifier adjustment!", index);
				continue;
			}

			faction_array[index]->mods[mr_row[2]] = Strings::ToInt(mr_row[1]);
		}

		LogInfo("Loaded [{}] faction modifier(s)", Strings::Commify(std::to_string(modifier_results.RowCount())));
	}
	else {
		LogError("Unable to load Faction Modifier data");
	}

	return true;
}

bool ZoneDatabase::GetFactionIdsForNPC(uint32 nfl_id, std::list<struct NPCFaction*> *faction_list, int32* primary_faction) {
	if (nfl_id <= 0) {
		std::list<struct NPCFaction*>::iterator cur,end;
		cur = faction_list->begin();
		end = faction_list->end();
		for(; cur != end; ++cur) {
			struct NPCFaction* tmp = *cur;
			safe_delete(tmp);
		}

		faction_list->clear();
		if (primary_faction)
			*primary_faction = nfl_id;
		return true;
	}
	const NPCFactionList* nfl = GetNPCFactionEntry(nfl_id);
	if (!nfl)
		return false;
	if (primary_faction)
		*primary_faction = nfl->primaryfaction;

	std::list<struct NPCFaction*>::iterator cur,end;
	cur = faction_list->begin();
	end = faction_list->end();
	for(; cur != end; ++cur) {
		struct NPCFaction* tmp = *cur;
		safe_delete(tmp);
	}
	faction_list->clear();
	for (int i=0; i<MAX_NPC_FACTIONS; i++) {
		struct NPCFaction *pFac;
		if (nfl->factionid[i]) {
			pFac = new struct NPCFaction;
			pFac->factionID = nfl->factionid[i];
			pFac->value_mod = nfl->factionvalue[i];
			pFac->npc_value = nfl->factionnpcvalue[i];
			pFac->temp = nfl->factiontemp[i];
			faction_list->push_back(pFac);
		}
	}
	return true;
}

/*  Corpse Queries */

uint32 ZoneDatabase::SendCharacterCorpseToGraveyard(uint32 dbid, uint32 zone_id, uint16 instance_id, const glm::vec4& position) {

	double xcorpse = (position.x + zone->random.Real(-20,20));
	double ycorpse = (position.y + zone->random.Real(-20,20));

	std::string query = StringFormat("UPDATE `character_corpses` "
                                    "SET `zone_id` = %u, `instance_id` = 0, "
                                    "`x` = %1.1f, `y` = %1.1f, `z` = %1.1f, `heading` = %1.1f, "
                                    "`was_at_graveyard` = 1 "
                                    "WHERE `id` = %d",
                                    zone_id, xcorpse, ycorpse, position.z, position.w, dbid);
	QueryDatabase(query);
	return dbid;
}

void ZoneDatabase::SendCharacterCorpseToNonInstance(uint32 corpse_db_id)
{
	if (corpse_db_id != 0)
	{
		auto query = fmt::format(SQL(
			UPDATE character_corpses SET instance_id = 0 WHERE id = {};
		), corpse_db_id);

		QueryDatabase(query);
	}
}

uint32 ZoneDatabase::GetCharacterCorpseDecayTimer(uint32 corpse_db_id){
	std::string query = StringFormat("SELECT(UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) FROM `character_corpses` WHERE `id` = %d AND NOT `time_of_death` = 0", corpse_db_id);
	auto results = QueryDatabase(query);
	auto& row = results.begin();
	if (results.Success() && results.RowsAffected() != 0)
		return Strings::ToUnsignedInt(row[0]);

	return 0;
}

uint32 ZoneDatabase::UpdateCharacterCorpse(uint32 db_id, uint32 char_id, const char* char_name, uint32 zone_id, uint16 instance_id, const CharacterCorpseEntry& corpse, const glm::vec4& position, uint32 guild_id, bool is_rezzed) {
	std::string query = StringFormat("UPDATE `character_corpses` "
                                    "SET `charname` = '%s', `zone_id` = %u, `instance_id` = %u, `charid` = %d, "
                                    "`x` = %1.1f,`y` =	%1.1f,`z` = %1.1f, `heading` = %1.1f, `guild_consent_id` = %u, "
                                    "`is_locked` = %d, `exp` = %u, `size` = %f, `level` = %u, "
                                    "`race` = %u, `gender` = %u, `class` = %u, `deity` = %u, "
                                    "`texture` = %u, `helm_texture` = %u, `copper` = %u, "
                                    "`silver` = %u, `gold` = %u, `platinum` = %u, `hair_color`  = %u, "
                                    "`beard_color` = %u, `eye_color_1` = %u, `eye_color_2` = %u, "
                                    "`hair_style`  = %u, `face` = %u, `beard` = %u, `drakkin_heritage` = %u, "
                                    "`drakkin_tattoo`  = %u, `drakkin_details` = %u, `wc_1` = %u, "
                                    "`wc_2` = %u, `wc_3` = %u, `wc_4` = %u, `wc_5` = %u, `wc_6` = %u, "
                                    "`wc_7` = %u, `wc_8` = %u, `wc_9` = %u "
                                    "WHERE `id` = %u",
                                    Strings::Escape(char_name).c_str(), zone_id, instance_id, char_id,
                                    position.x, position.y, position.z, position.w, guild_id,
                                    corpse.locked, corpse.exp, corpse.size, corpse.level, corpse.race,
                                    corpse.gender, corpse.class_, corpse.deity, corpse.texture,
                                    corpse.helmtexture, corpse.copper, corpse.silver, corpse.gold,
                                    corpse.plat, corpse.haircolor, corpse.beardcolor, corpse.eyecolor1,
                                    corpse.eyecolor2, corpse.hairstyle, corpse.face, corpse.beard,
                                    corpse.drakkin_heritage, corpse.drakkin_tattoo, corpse.drakkin_details,
                                    corpse.item_tint.Head.Color, corpse.item_tint.Chest.Color, corpse.item_tint.Arms.Color,
                                    corpse.item_tint.Wrist.Color, corpse.item_tint.Hands.Color, corpse.item_tint.Legs.Color,
                                    corpse.item_tint.Feet.Color, corpse.item_tint.Primary.Color, corpse.item_tint.Secondary.Color,
                                    db_id);
	auto results = QueryDatabase(query);

	return db_id;
}

uint32 ZoneDatabase::UpdateCharacterCorpseConsent(uint32 charid, uint32 guildid)
{
	std::string query = fmt::format("UPDATE `character_corpses` SET `guild_consent_id` = '{}' WHERE charid = '{}'", guildid, charid);
	auto results = QueryDatabase(query);
	return results.RowsAffected();
}

void ZoneDatabase::MarkCorpseAsRezzed(uint32 db_id) {
	std::string query = StringFormat("UPDATE `character_corpses` SET `is_rezzed` = 1 WHERE `id` = %i", db_id);
	auto results = QueryDatabase(query);
}

uint32 ZoneDatabase::SaveCharacterCorpse(uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, const CharacterCorpseEntry& corpse, const glm::vec4& position, uint32 guildid) {
	/* Dump Basic Corpse Data */
	std::string query = StringFormat(
		"INSERT INTO `character_corpses` "
		"SET `charname` = '%s',  "
		"`zone_id` =	%u,  "
		"`instance_id` =	%u,  "
		"`charid` = %d, "
		"`x` =	%1.1f,  "
		"`y` = %1.1f,  "
		"`z` = %1.1f,  "
		"`heading` = %1.1f, "
		"`guild_consent_id` = %u,  "
		"`time_of_death` = NOW(),  "
		"`is_buried` =	0,  "
		"`is_locked` = %d, "
		"`exp` = %u,  "
		"`size` = %f,  "
		"`level` = %u,  "
		"`race` = %u,  "
		"`gender` = %u, "
		"`class` = %u,  "
		"`deity` = %u,  "
		"`texture` = %u,  "
		"`helm_texture` = %u, "
		"`copper` = %u,  "
		"`silver` = %u, "
		"`gold` = %u, "
		"`platinum` = %u, "
		"`hair_color`  = %u, "
		"`beard_color` = %u, "
		"`eye_color_1` = %u, "
		"`eye_color_2` = %u, "
		"`hair_style`  = %u, "
		"`face` = %u, "
		"`beard` = %u, "
		"`drakkin_heritage` = %u, "
		"`drakkin_tattoo` = %u, "
		"`drakkin_details` = %u, "
		"`wc_1` = %u, "
		"`wc_2` = %u, "
		"`wc_3` = %u, "
		"`wc_4` = %u, "
		"`wc_5` = %u, "
		"`wc_6` = %u, "
		"`wc_7` = %u, "
		"`wc_8` = %u, "
		"`wc_9`	= %u ",
		Strings::Escape(charname).c_str(),
		zoneid,
		instanceid,
		charid,
		position.x,
		position.y,
		position.z,
		position.w,
		guildid,
		corpse.locked,
		corpse.exp,
		corpse.size,
		corpse.level,
		corpse.race,
		corpse.gender,
		corpse.class_,
		corpse.deity,
		corpse.texture,
		corpse.helmtexture,
		corpse.copper,
		corpse.silver,
		corpse.gold,
		corpse.plat,
		corpse.haircolor,
		corpse.beardcolor,
		corpse.eyecolor1,
		corpse.eyecolor2,
		corpse.hairstyle,
		corpse.face,
		corpse.beard,
		corpse.drakkin_heritage,
		corpse.drakkin_tattoo,
		corpse.drakkin_details,
		corpse.item_tint.Head.Color,
		corpse.item_tint.Chest.Color,
		corpse.item_tint.Arms.Color,
		corpse.item_tint.Wrist.Color,
		corpse.item_tint.Hands.Color,
		corpse.item_tint.Legs.Color,
		corpse.item_tint.Feet.Color,
		corpse.item_tint.Primary.Color,
		corpse.item_tint.Secondary.Color
	);
	auto results = QueryDatabase(query);
	uint32 last_insert_id = results.LastInsertedID();

	std::string corpse_items_query;
	/* Dump Items from Inventory */
	uint8 first_entry = 0;
	for(auto &item : corpse.items)
	{
		if (first_entry != 1){
			corpse_items_query = StringFormat("REPLACE INTO `character_corpse_items` \n"
				" (corpse_id, equip_slot, item_id, charges, aug_1, aug_2, aug_3, aug_4, aug_5, aug_6, attuned, custom_data, ornamenticon, ornamentidfile, ornament_hero_model) \n"
				" VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, '%s', %u, %u, %u) \n",
				last_insert_id,
				item.equip_slot,
				item.item_id,
				item.charges,
				item.aug_1,
				item.aug_2,
				item.aug_3,
				item.aug_4,
				item.aug_5,
				item.aug_6,
				item.attuned,
				item.custom_data.c_str(),
				item.ornamenticon,
				item.ornamentidfile,
				item.ornament_hero_model
			);
			first_entry = 1;
		}
		else{
			corpse_items_query = corpse_items_query + StringFormat(", (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, '%s', %u, %u, %u) \n",
				last_insert_id,
				item.equip_slot,
				item.item_id,
				item.charges,
				item.aug_1,
				item.aug_2,
				item.aug_3,
				item.aug_4,
				item.aug_5,
				item.aug_6,
				item.attuned,
				item.custom_data.c_str(),
				item.ornamenticon,
				item.ornamentidfile,
				item.ornament_hero_model
			);
		}
	}

	if (!corpse_items_query.empty())
		QueryDatabase(corpse_items_query);

	return last_insert_id;
}

uint32 ZoneDatabase::GetCharacterBuriedCorpseCount(uint32 char_id) {
	std::string query = StringFormat("SELECT COUNT(*) FROM `character_corpses` WHERE `charid` = '%u' AND `is_buried` = 1", char_id);
	auto results = QueryDatabase(query);

	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToInt(row[0]);
	}
	return 0;
}

uint32 ZoneDatabase::GetCharacterCorpseCount(uint32 char_id) {
	std::string query = StringFormat("SELECT COUNT(*) FROM `character_corpses` WHERE `charid` = '%u'", char_id);
	auto results = QueryDatabase(query);

	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToInt(row[0]);
	}
	return 0;
}

uint32 ZoneDatabase::GetCharacterCorpseID(uint32 char_id, uint8 corpse) {
	std::string query = StringFormat("SELECT `id` FROM `character_corpses` WHERE `charid` = '%u' limit %d, 1", char_id, corpse);

	auto results = QueryDatabase(query);
	auto& row = results.begin();

	if (row != results.end())
		return Strings::ToUnsignedInt(row[0]);
	else
		return 0;
}

uint32 ZoneDatabase::GetCharacterCorpseItemAt(uint32 corpse_id, uint16 slotid) {
	Corpse* tmp = LoadCharacterCorpse(corpse_id);
	uint32 itemid = 0;

	if (tmp) {
		itemid = tmp->GetWornItem(slotid);
		tmp->DepopPlayerCorpse();
	}
	return itemid;
}

bool ZoneDatabase::LoadCharacterCorpseData(uint32 corpse_id, CharacterCorpseEntry& corpse){
	std::string query = StringFormat(
		"SELECT           \n"
		"is_locked,       \n"
		"exp,             \n"
		"size,            \n"
		"`level`,         \n"
		"race,            \n"
		"gender,          \n"
		"class,           \n"
		"deity,           \n"
		"texture,         \n"
		"helm_texture,    \n"
		"copper,          \n"
		"silver,          \n"
		"gold,            \n"
		"platinum,        \n"
		"hair_color,      \n"
		"beard_color,     \n"
		"eye_color_1,     \n"
		"eye_color_2,     \n"
		"hair_style,      \n"
		"face,            \n"
		"beard,           \n"
		"drakkin_heritage,\n"
		"drakkin_tattoo,  \n"
		"drakkin_details, \n"
		"wc_1,            \n"
		"wc_2,            \n"
		"wc_3,            \n"
		"wc_4,            \n"
		"wc_5,            \n"
		"wc_6,            \n"
		"wc_7,            \n"
		"wc_8,            \n"
		"wc_9             \n"
		"FROM             \n"
		"character_corpses\n"
		"WHERE `id` = %u  LIMIT 1\n",
		corpse_id
	);
	auto results = QueryDatabase(query);
	uint16 i = 0;
	for (auto& row = results.begin(); row != results.end(); ++row) {
		corpse.locked = Strings::ToInt(row[i++]);						// is_locked,
		corpse.exp = Strings::ToUnsignedInt(row[i++]);							// exp,
		corpse.size = Strings::ToInt(row[i++]);							// size,
		corpse.level = Strings::ToInt(row[i++]);						// `level`,
		corpse.race = Strings::ToInt(row[i++]);							// race,
		corpse.gender = Strings::ToInt(row[i++]);						// gender,
		corpse.class_ = Strings::ToInt(row[i++]);						// class,
		corpse.deity = Strings::ToInt(row[i++]);						// deity,
		corpse.texture = Strings::ToInt(row[i++]);						// texture,
		corpse.helmtexture = Strings::ToInt(row[i++]);					// helm_texture,
		corpse.copper = Strings::ToUnsignedInt(row[i++]);						// copper,
		corpse.silver = Strings::ToUnsignedInt(row[i++]);						// silver,
		corpse.gold = Strings::ToUnsignedInt(row[i++]);						// gold,
		corpse.plat = Strings::ToUnsignedInt(row[i++]);						// platinum,
		corpse.haircolor = Strings::ToInt(row[i++]);					// hair_color,
		corpse.beardcolor = Strings::ToInt(row[i++]);					// beard_color,
		corpse.eyecolor1 = Strings::ToInt(row[i++]);					// eye_color_1,
		corpse.eyecolor2 = Strings::ToInt(row[i++]);					// eye_color_2,
		corpse.hairstyle = Strings::ToInt(row[i++]);					// hair_style,
		corpse.face = Strings::ToInt(row[i++]);							// face,
		corpse.beard = Strings::ToInt(row[i++]);						// beard,
		corpse.drakkin_heritage = Strings::ToUnsignedInt(row[i++]);			// drakkin_heritage,
		corpse.drakkin_tattoo = Strings::ToUnsignedInt(row[i++]);				// drakkin_tattoo,
		corpse.drakkin_details = Strings::ToUnsignedInt(row[i++]);				// drakkin_details,
		corpse.item_tint.Head.Color = Strings::ToUnsignedInt(row[i++]);		// wc_1,
		corpse.item_tint.Chest.Color = Strings::ToUnsignedInt(row[i++]);		// wc_2,
		corpse.item_tint.Arms.Color = Strings::ToUnsignedInt(row[i++]);		// wc_3,
		corpse.item_tint.Wrist.Color = Strings::ToUnsignedInt(row[i++]);		// wc_4,
		corpse.item_tint.Hands.Color = Strings::ToUnsignedInt(row[i++]);		// wc_5,
		corpse.item_tint.Legs.Color = Strings::ToUnsignedInt(row[i++]);		// wc_6,
		corpse.item_tint.Feet.Color = Strings::ToUnsignedInt(row[i++]);		// wc_7,
		corpse.item_tint.Primary.Color = Strings::ToUnsignedInt(row[i++]);		// wc_8,
		corpse.item_tint.Secondary.Color = Strings::ToUnsignedInt(row[i++]);	// wc_9
	}
	query = StringFormat(
		"SELECT                       \n"
		"equip_slot,                  \n"
		"item_id,                     \n"
		"charges,                     \n"
		"aug_1,                       \n"
		"aug_2,                       \n"
		"aug_3,                       \n"
		"aug_4,                       \n"
		"aug_5,                       \n"
		"aug_6,                       \n"
		"attuned,                     \n"
		"custom_data,                 \n"
		"ornamenticon,                \n"
		"ornamentidfile,              \n"
		"ornament_hero_model          \n"
		"FROM                         \n"
		"character_corpse_items       \n"
		"WHERE `corpse_id` = %u\n"
		,
		corpse_id
	);
	results = QueryDatabase(query);

	i = 0;
	uint16 r = 0;
	for (auto& row = results.begin(); row != results.end(); ++row) {
		CharacterCorpseItemEntry item;
		item.equip_slot = Strings::ToInt(row[r++]);		// equip_slot,
		item.item_id = Strings::ToUnsignedInt(row[r++]); 		// item_id,
		item.charges = Strings::ToInt(row[r++]); 		// charges,
		item.aug_1 = Strings::ToInt(row[r++]); 			// aug_1,
		item.aug_2 = Strings::ToInt(row[r++]); 			// aug_2,
		item.aug_3 = Strings::ToInt(row[r++]); 			// aug_3,
		item.aug_4 = Strings::ToInt(row[r++]); 			// aug_4,
		item.aug_5 = Strings::ToInt(row[r++]); 			// aug_5,
		item.aug_6 = Strings::ToInt(row[r++]); 			// aug_6,
		item.attuned = Strings::ToInt(row[r++]) > 0 ? true : false; // attuned,

		if (row[r]) {
			item.custom_data = row[r++];
		}
		else {
			r++;
		}

		item.ornamenticon = Strings::ToUnsignedInt(row[r++]);
		item.ornamentidfile = Strings::ToUnsignedInt(row[r++]);
		item.ornament_hero_model = Strings::ToUnsignedInt(row[r++]);

		corpse.items.emplace_back(std::move(item));
		r = 0;
		i++;
	}

	return true;
}

Corpse* ZoneDatabase::SummonBuriedCharacterCorpses(uint32 char_id, uint32 dest_zone_id, uint16 dest_instance_id, const glm::vec4& position) {
	Corpse* corpse = nullptr;
	std::string query = StringFormat("SELECT `id`, `charname`, `time_of_death`, `is_rezzed`, `guild_consent_id` "
                                    "FROM `character_corpses` "
                                    "WHERE `charid` = '%u' AND `is_buried` = 1 "
                                    "ORDER BY `time_of_death` LIMIT 1",
                                    char_id);
	auto results = QueryDatabase(query);

	for (auto& row = results.begin(); row != results.end(); ++row) {
		corpse = Corpse::LoadCharacterCorpseEntity(
			Strings::ToUnsignedInt(row[0]), 			 // uint32 in_dbid
			char_id, 				 // uint32 in_charid
			row[1], 				 // char* in_charname
			position,
			row[2], 				 // char* time_of_death
			Strings::ToInt(row[3]) == 1, 		 // bool rezzed
			false,					 // bool was_at_graveyard
			Strings::ToUnsignedInt(row[4])            // uint32 guild_consent_id
		);
		if (!corpse)
            continue;

		entity_list.AddCorpse(corpse);
		corpse->SetDecayTimer(RuleI(Character, CorpseDecayTimeMS));
		corpse->Spawn();
		if (!UnburyCharacterCorpse(corpse->GetCorpseDBID(), dest_zone_id, dest_instance_id, position))
			LogError("Unable to unbury a summoned player corpse for character id [{}]", char_id);
	}

	return corpse;
}

bool ZoneDatabase::SummonAllCharacterCorpses(uint32 char_id, uint32 dest_zone_id, uint16 dest_instance_id, const glm::vec4& position) {
	Corpse* corpse = nullptr;
	int CorpseCount = 0;

	std::string query = StringFormat(
		"UPDATE character_corpses SET zone_id = %i, instance_id = %i, x = %f, y = %f, z = %f, heading = %f, is_buried = 0, was_at_graveyard = 0 WHERE charid = %i",
		dest_zone_id, dest_instance_id, position.x, position.y, position.z, position.w, char_id
	);
	auto results = QueryDatabase(query);

	query = StringFormat(
		"SELECT `id`, `charname`, `time_of_death`, `is_rezzed`, `guild_consent_id` FROM `character_corpses` WHERE `charid` = '%u'"
		"ORDER BY time_of_death",
		char_id);
	results = QueryDatabase(query);

	for (auto& row = results.begin(); row != results.end(); ++row) {
		corpse = Corpse::LoadCharacterCorpseEntity(
			Strings::ToUnsignedInt(row[0]),
			char_id,
			row[1],
			position,
			row[2],
			Strings::ToInt(row[3]) == 1,
			false,
			Strings::ToUnsignedInt(row[4]));

		if (corpse) {
			entity_list.AddCorpse(corpse);
			corpse->SetDecayTimer(RuleI(Character, CorpseDecayTimeMS));
			corpse->Spawn();
			++CorpseCount;
		}
		else{
			LogError("Unable to construct a player corpse for character id [{}]", char_id);
		}
	}

	return (CorpseCount > 0);
}

int ZoneDatabase::CountCharacterCorpses(uint32 char_id) {
	std::string query = fmt::format(
		SQL(
			SELECT
			COUNT(*)
			FROM
			character_corpses
			WHERE
			charid = '{}'
		),
		char_id
	);
	auto results = QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToInt(row[0]);
	}
	return 0;
}

int ZoneDatabase::CountCharacterCorpsesByZoneID(uint32 char_id, uint32 zone_id) {
	std::string query = fmt::format(
		SQL(
			SELECT
			COUNT(*)
			FROM
			character_corpses
			WHERE
			charid = '{}'
			AND
			zone_id = '{}'
		),
		char_id,
		zone_id
	);
	auto results = QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToInt(row[0]);
	}
	return 0;
}

bool ZoneDatabase::UnburyCharacterCorpse(uint32 db_id, uint32 new_zone_id, uint16 new_instance_id, const glm::vec4& position) {
	std::string query = StringFormat("UPDATE `character_corpses` "
                                    "SET `is_buried` = 0, `zone_id` = %u, `instance_id` = %u, "
                                    "`x` = %f, `y` = %f, `z` = %f, `heading` = %f, "
                                    "`time_of_death` = Now(), `was_at_graveyard` = 0 "
                                    "WHERE `id` = %u",
                                    new_zone_id, new_instance_id,
                                    position.x, position.y, position.z, position.w, db_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0)
		return true;

	return false;
}

Corpse* ZoneDatabase::LoadCharacterCorpse(uint32 player_corpse_id) {
	Corpse* NewCorpse = 0;
	std::string query = StringFormat(
		"SELECT `id`, `charid`, `charname`, `x`, `y`, `z`, `heading`, `time_of_death`, `is_rezzed`, `was_at_graveyard`, `guild_consent_id` FROM `character_corpses` WHERE `id` = '%u' LIMIT 1",
		player_corpse_id
	);
	auto results = QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
        auto position = glm::vec4(Strings::ToFloat(row[3]), Strings::ToFloat(row[4]), Strings::ToFloat(row[5]), Strings::ToFloat(row[6]));
		NewCorpse = Corpse::LoadCharacterCorpseEntity(
				Strings::ToUnsignedInt(row[0]), 		 // id					  uint32 in_dbid
				Strings::ToUnsignedInt(row[1]),		 // charid				  uint32 in_charid
				row[2], 			 //	char_name
				position,
				row[7],				 // time_of_death		  char* time_of_death
				Strings::ToInt(row[8]) == 1, 	 // is_rezzed			  bool rezzed
				Strings::ToInt(row[9]),		 // was_at_graveyard	  bool was_at_graveyard
				Strings::ToUnsignedInt(row[10])       // guild_consent_id      uint32 guild_consent_id
			);
		entity_list.AddCorpse(NewCorpse);
	}
	return NewCorpse;
}

bool ZoneDatabase::LoadCharacterCorpses(uint32 zone_id, uint16 instance_id) {
	std::string query;
	if (!RuleB(Zone, EnableShadowrest)){
		query = StringFormat("SELECT id, charid, charname, x, y, z, heading, time_of_death, is_rezzed, was_at_graveyard, guild_consent_id FROM character_corpses WHERE zone_id='%u' AND instance_id='%u'", zone_id, instance_id);
	}
	else{
		query = StringFormat("SELECT id, charid, charname, x, y, z, heading, time_of_death, is_rezzed, 0 as was_at_graveyard, guild_consent_id FROM character_corpses WHERE zone_id='%u' AND instance_id='%u' AND is_buried=0", zone_id, instance_id);
	}

	auto results = QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
        auto position = glm::vec4(Strings::ToFloat(row[3]), Strings::ToFloat(row[4]), Strings::ToFloat(row[5]), Strings::ToFloat(row[6]));
		entity_list.AddCorpse(
			 Corpse::LoadCharacterCorpseEntity(
				Strings::ToUnsignedInt(row[0]), 		  // id					  uint32 in_dbid
				Strings::ToUnsignedInt(row[1]), 		  // charid				  uint32 in_charid
				row[2], 			  //					  char_name
				position,
				row[7], 			  // time_of_death		  char* time_of_death
				Strings::ToInt(row[8]) == 1, 	  // is_rezzed			  bool rezzed
				Strings::ToInt(row[9]),
				Strings::ToUnsignedInt(row[10]))       // guild_consent_id     uint32 guild_consent_id
		);
	}

	LogInfo("Loaded [{}] player corpse(s)", Strings::Commify(results.RowCount()));

	return true;
}

uint32 ZoneDatabase::GetFirstCorpseID(uint32 char_id) {
	std::string query = StringFormat("SELECT `id` FROM `character_corpses` WHERE `charid` = '%u' AND `is_buried` = 0 ORDER BY `time_of_death` LIMIT 1", char_id);
	auto results = QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToInt(row[0]);
	}
	return 0;
}

bool ZoneDatabase::DeleteItemOffCharacterCorpse(uint32 db_id, uint32 equip_slot, uint32 item_id){
	std::string query = StringFormat("DELETE FROM `character_corpse_items` WHERE `corpse_id` = %u AND equip_slot = %u AND item_id = %u", db_id, equip_slot, item_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0){
		return true;
	}
	return false;
}

bool ZoneDatabase::BuryCharacterCorpse(uint32 db_id) {
	std::string query = StringFormat("UPDATE `character_corpses` SET `is_buried` = 1 WHERE `id` = %u", db_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0){
		return true;
	}
	return false;
}

bool ZoneDatabase::BuryAllCharacterCorpses(uint32 char_id) {
	std::string query = StringFormat("SELECT `id` FROM `character_corpses` WHERE `charid` = %u", char_id);
	auto results = QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		BuryCharacterCorpse(Strings::ToInt(row[0]));
		return true;
	}
	return false;
}

bool ZoneDatabase::DeleteCharacterCorpse(uint32 db_id) {
	std::string query = StringFormat("DELETE FROM `character_corpses` WHERE `id` = %d", db_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0)
		return true;

	return false;
}

uint32 ZoneDatabase::LoadSaylinkID(const char* saylink_text, bool auto_insert)
{
	if (!saylink_text || saylink_text[0] == '\0')
		return 0;

	std::string query = StringFormat("SELECT `id` FROM `saylink` WHERE `phrase` = '%s' LIMIT 1", saylink_text);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return 0;
	if (!results.RowCount()) {
		if (auto_insert)
			return SaveSaylinkID(saylink_text);
		else
			return 0;
	}

	auto& row = results.begin();
	return Strings::ToInt(row[0]);
}

uint32 ZoneDatabase::SaveSaylinkID(const char* saylink_text)
{
	if (!saylink_text || saylink_text[0] == '\0')
		return 0;

	std::string query = StringFormat("INSERT INTO `saylink` (`phrase`) VALUES ('%s')", saylink_text);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return 0;

	return results.LastInsertedID();
}

double ZoneDatabase::GetAAEXPModifier(uint32 character_id, uint32 zone_id, int16 instance_version) const {
	const std::string query = fmt::format(
		SQL(
			SELECT
			`aa_modifier`
			FROM
			`character_exp_modifiers`
			WHERE
			`character_id` = {}
			AND
			(`zone_id` = {} OR `zone_id` = 0) AND
			(`instance_version` = {} OR `instance_version` = -1)
			ORDER BY `zone_id`, `instance_version` DESC
			LIMIT 1
		),
		character_id,
		zone_id,
		instance_version
	);

	auto results = database.QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToFloat(row[0]);
	}

	return 1.0f;
}

double ZoneDatabase::GetEXPModifier(uint32 character_id, uint32 zone_id, int16 instance_version) const {
	const std::string query = fmt::format(
		SQL(
			SELECT
			`exp_modifier`
			FROM
			`character_exp_modifiers`
			WHERE
			`character_id` = {}
			AND
			(`zone_id` = {} OR `zone_id` = 0) AND
			(`instance_version` = {} OR `instance_version` = -1)
			ORDER BY `zone_id`, `instance_version` DESC
			LIMIT 1
		),
		character_id,
		zone_id,
		instance_version
	);

	auto results = database.QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		return Strings::ToFloat(row[0]);
	}

	return 1.0f;
}

void ZoneDatabase::SetAAEXPModifier(uint32 character_id, uint32 zone_id, double aa_modifier, int16 instance_version) {
	float exp_modifier = GetEXPModifier(character_id, zone_id, instance_version);
	std::string query = fmt::format(
		SQL(
			REPLACE INTO
			`character_exp_modifiers`
			VALUES
			({}, {}, {}, {}, {})
		),
		character_id,
		zone_id,
		instance_version,
		aa_modifier,
		exp_modifier
	);
	database.QueryDatabase(query);
}

void ZoneDatabase::SetEXPModifier(uint32 character_id, uint32 zone_id, double exp_modifier, int16 instance_version) {
	float aa_modifier = GetAAEXPModifier(character_id, zone_id, instance_version);
	std::string query = fmt::format(
		SQL(
			REPLACE INTO
			`character_exp_modifiers`
			VALUES
			({}, {}, {}, {}, {})
		),
		character_id,
		zone_id,
		instance_version,
		aa_modifier,
		exp_modifier
	);
	database.QueryDatabase(query);
}

void ZoneDatabase::UpdateGMStatus(uint32 accID, int newStatus)
{
	if (accID) {
		std::string query = fmt::format(
			SQL(
				UPDATE
				`account`
				SET `status` = {}
				WHERE
				`id` = {}
			),
			newStatus,
			accID
		);
		database.QueryDatabase(query);
	}
}

void ZoneDatabase::SaveCharacterBinds(Client *c)
{
	// bulk save character binds
	std::vector<CharacterBindRepository::CharacterBind> binds = {};
	CharacterBindRepository::CharacterBind bind = {};

	// count character binds
	int bind_count = 0;
	for (auto & b : c->GetPP().binds) {
		if (b.zone_id) {
			bind_count++;
		}
	}

	// allocate memory for binds
	binds.reserve(bind_count);

	// copy binds to vector
	int i = 0;
	for (auto &b: c->GetPP().binds) {
		if (b.zone_id) {
			// copy bind data
			bind.id          = c->CharacterID();
			bind.zone_id     = b.zone_id;
			bind.instance_id = b.instance_id;
			bind.x           = b.x;
			bind.y           = b.y;
			bind.z           = b.z;
			bind.heading     = b.heading;
			bind.slot        = i;

			// add bind to vector
			binds.emplace_back(bind);

			i++;
		}
	}

	// save binds
	if (bind_count > 0) {
		// delete old binds
		CharacterBindRepository::DeleteWhere(database, fmt::format("id = {}", c->CharacterID()));
		// save new binds
		CharacterBindRepository::InsertMany(database, binds);
	}
}
