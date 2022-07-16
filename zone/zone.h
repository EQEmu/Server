/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#ifndef ZONE_H
#define ZONE_H

#include "../common/eqtime.h"
#include "../common/linked_list.h"
#include "../common/rulesys.h"
#include "../common/types.h"
#include "../common/random.h"
#include "../common/string_util.h"
#include "zonedb.h"
#include "zone_store.h"
#include "../common/repositories/grid_repository.h"
#include "../common/repositories/grid_entries_repository.h"
#include "../common/repositories/zone_points_repository.h"
#include "qglobals.h"
#include "spawn2.h"
#include "spawngroup.h"
#include "aa_ability.h"
#include "pathfinder_interface.h"
#include "global_loot_manager.h"
#include "queryserv.h"
#include "../common/discord/discord.h"

class DynamicZone;

struct ZonePoint {
	float  x;
	float  y;
	float  z;
	float  heading;
	uint16 number;
	float  target_x;
	float  target_y;
	float  target_z;
	float  target_heading;
	uint16 target_zone_id;
	int32  target_zone_instance;
	uint32 client_version_mask;
	bool   is_virtual;
	int    height;
	int    width;
};

struct ZoneClientAuth_Struct {
	uint32 ip;            // client's IP address
	uint32 wid;        // client's WorldID#
	uint32 accid;
	int16  admin;
	uint32 charid;
	uint32 lsid;
	bool   tellsoff;
	char   charname[64];
	char   lskey[30];
	bool   stale;
};

struct ZoneEXPModInfo {
	float ExpMod;
	float AAExpMod;
};

struct item_tick_struct {
	uint32      itemid;
	uint32      chance;
	uint32      level;
	int16       bagslot;
	std::string qglobal;
};

class Client;
class Expedition;
class Map;
class Mob;
class WaterMap;
extern EntityList entity_list;
struct NPCType;
struct ServerZoneIncomingClient_Struct;
class MobMovementManager;

class Zone {
public:
	static bool Bootup(uint32 iZoneID, uint32 iInstanceID, bool is_static = false);
	static void Shutdown(bool quiet = false);

	Zone(uint32 in_zoneid, uint32 in_instanceid, const char *in_short_name);
	~Zone();

	AA::Ability *GetAlternateAdvancementAbility(int id);
	AA::Ability *GetAlternateAdvancementAbilityByRank(int rank_id);
	AA::Rank *GetAlternateAdvancementRank(int rank_id);
	bool is_zone_time_localized;
	bool process_mobs_while_empty;
	bool AggroLimitReached() { return (aggroedmobs > 10) ? true : false; }
	bool AllowMercs() const { return (allow_mercs); }
	bool CanBind() const { return (can_bind); }
	bool CanCastOutdoor() const { return (can_castoutdoor); } //qadar
	bool CanDoCombat() const { return (can_combat); }
	bool CanLevitate() const { return (can_levitate); } // Magoth78
	bool Depop(bool StartSpawnTimer = false);
	bool did_adventure_actions;
	bool GetAuth(
		uint32 iIP,
		const char *iCharName,
		uint32 *oWID = 0,
		uint32 *oAccID = 0,
		uint32 *oCharID = 0,
		int16 *oStatus = 0,
		char *oLSKey = 0,
		bool *oTellsOff = 0
	);
	bool HasGraveyard();
	bool HasWeather();
	bool Init(bool is_static);
	bool IsCity() const { return (is_city); }
	bool IsHotzone() const { return (is_hotzone); }
	bool IsLoaded();
	bool IsPVPZone() { return pvpzone; }
	bool IsSpellBlocked(uint32 spell_id, const glm::vec3 &location);
	bool IsUCSServerAvailable() { return m_ucss_available; }
	bool IsZone(uint32 zone_id, uint16 instance_id) const;
	bool LoadGroundSpawns();
	bool LoadZoneCFG(const char *filename, uint16 instance_version);
	bool LoadZoneObjects();
	bool Process();
	bool SaveZoneCFG();

	int GetNpcPositionUpdateDistance() const;
	void SetNpcPositionUpdateDistance(int in_npc_position_update_distance);

	char *adv_data;

	const char *GetSpellBlockedMessage(uint32 spell_id, const glm::vec3 &location);

	EQ::Random random;
	EQTime     zone_time;

	ZonePoint *
	GetClosestZonePoint(const glm::vec3 &location, const char *to_name, Client *client, float max_distance = 40000.0f);

	inline bool BuffTimersSuspended() const { return newzone_data.SuspendBuffs != 0; };
	inline bool HasMap() { return zonemap != nullptr; }
	inline bool HasWaterMap() { return watermap != nullptr; }
	inline bool InstantGrids() { return (!initgrids_timer.Enabled()); }
	inline bool IsStaticZone() { return staticzone; }
	inline const bool IsInstancePersistent() const { return pers_instance; }
	inline const char *GetFileName() { return file_name; }
	inline const char *GetLongName() { return long_name; }
	inline const char *GetShortName() { return short_name; }
	inline const uint8 GetZoneType() const { return zone_type; }
	inline const uint16 GetInstanceVersion() const { return instanceversion; }
	inline const uint32 &GetMaxClients() { return pMaxClients; }
	inline const uint32 &graveyard_id() { return pgraveyard_id; }
	inline const uint32 &graveyard_zoneid() { return pgraveyard_zoneid; }
	inline const uint32 GetInstanceID() const { return instanceid; }
	inline const uint32 GetZoneID() const { return zoneid; }
	inline glm::vec4 GetSafePoint() { return m_SafePoint; }
	inline glm::vec4 GetGraveyardPoint() { return m_Graveyard; }
	inline std::vector<int> GetGlobalLootTables(NPC *mob) const { return m_global_loot.GetGlobalLootTables(mob); }
	inline Timer *GetInstanceTimer() { return Instance_Timer; }
	inline void AddGlobalLootEntry(GlobalLootEntry &in) { return m_global_loot.AddEntry(in); }
	inline void SetZoneHasCurrentTime(bool time) { zone_has_current_time = time; }
	inline void ShowNPCGlobalLoot(Client *to, NPC *who) { m_global_loot.ShowNPCGlobalLoot(to, who); }
	inline void ShowZoneGlobalLoot(Client *to) { m_global_loot.ShowZoneGlobalLoot(to); }
	int GetZoneTotalBlockedSpells() { return zone_total_blocked_spells; }
	void DumpMerchantList(uint32 npcid);
	int SaveTempItem(uint32 merchantid, uint32 npcid, uint32 item, int32 charges, bool sold = false);
	int32 MobsAggroCount() { return aggroedmobs; }
	DynamicZone *GetDynamicZone();

	IPathfinder                                   *pathing;
	LinkedList<NPC_Emote_Struct *>                NPCEmoteList;
	LinkedList<Spawn2 *>                          spawn2_list;
	LinkedList<ZonePoint *>                       zone_point_list;
	std::vector<ZonePointsRepository::ZonePoints> virtual_zone_point_list;

	Map                   *zonemap;
	MercTemplate *GetMercTemplate(uint32 template_id);
	NewZone_Struct        newzone_data;
	QGlobalCache *CreateQGlobals()
	{
		qGlobals = new QGlobalCache();
		return qGlobals;
	}
	QGlobalCache *GetQGlobals() { return qGlobals; }
	SpawnConditionManager spawn_conditions;
	SpawnGroupList        spawn_group_list;

	std::list<AltCurrencyDefinition_Struct>          AlternateCurrencies;
	std::list<InternalVeteranReward>                 VeteranRewards;
	std::map<uint32, LDoNTrapTemplate *>             ldon_trap_list;
	std::map<uint32, MercTemplate>                   merc_templates;
	std::map<uint32, NPCType *>                      merctable;
	std::map<uint32, NPCType *>                      npctable;
	std::map<uint32, std::list<LDoNTrapTemplate *> > ldon_trap_entry_list;
	std::map<uint32, std::list<MerchantList> >       merchanttable;
	std::map<uint32, std::list<MercSpellEntry> >     merc_spells_list;
	std::map<uint32, std::list<MercStanceInfo> >     merc_stance_list;
	std::map<uint32, std::list<TempMerchantList> >   tmpmerchanttable;
	std::map<uint32, std::string>                    adventure_entry_list_flavor;
	std::map<uint32, ZoneEXPModInfo>                 level_exp_mod;

	std::pair<AA::Ability *, AA::Rank *> GetAlternateAdvancementAbilityAndRank(int id, int points_spent);

	std::unordered_map<int, item_tick_struct>             tick_items;
	std::unordered_map<int, std::unique_ptr<AA::Ability>> aa_abilities;
	std::unordered_map<int, std::unique_ptr<AA::Rank>>    aa_ranks;

	std::vector<GridRepository::Grid>             zone_grids;
	std::vector<GridEntriesRepository::GridEntry> zone_grid_entries;

	std::unordered_map<uint32, std::unique_ptr<DynamicZone>> dynamic_zone_cache;
	std::unordered_map<uint32, std::unique_ptr<Expedition>>  expedition_cache;

	time_t weather_timer;
	Timer  spawn2_timer;
	Timer  hot_reload_timer;

	uint8  weather_intensity;
	uint8  zone_weather;
	uint8  loglevelvar;
	uint8  lootvar;
	uint8  merchantvar;
	uint8  tradevar;
	uint32 numzonepoints;
	uint32 CountAuth();
	uint32 CountSpawn2();
	uint32 GetSpawnKillCount(uint32 in_spawnid);
	uint32 GetTempMerchantQuantity(uint32 NPCID, uint32 Slot);

	uint32 GetCurrencyID(uint32 item_id);
	uint32 GetCurrencyItemID(uint32 currency_id);

	std::string GetZoneDescription();
	void SendReloadMessage(std::string reload_type);

	void AddAggroMob() { aggroedmobs++; }
	void AddAuth(ServerZoneIncomingClient_Struct *szic);
	void ChangeWeather();
	void ClearBlockedSpells();
	void ClearNPCTypeCache(int id);
	void CalculateNpcUpdateDistanceSpread();
	void DelAggroMob() { aggroedmobs--; }
	void DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID);
	void Despawn(uint32 spawngroupID);
	void DoAdventureActions();
	void DoAdventureAssassinationCountIncrease();
	void DoAdventureCountIncrease();
	void GetMerchantDataForZoneLoad();
	void GetTimeSync();
	void LoadAdventureFlavor();
	void LoadAlternateAdvancement();
	void LoadAlternateCurrencies();
	void LoadZoneBlockedSpells();
	void LoadLDoNTrapEntries();
	void LoadLDoNTraps();
	void LoadLevelEXPMods();
	void LoadGrids();
	void LoadMercSpells();
	void LoadMercTemplates();
	void LoadNewMerchantData(uint32 merchantid);
	void LoadNPCEmotes(LinkedList<NPC_Emote_Struct *> *NPCEmoteList);
	void LoadTempMerchantData();
	void LoadTickItems();
	void LoadVeteranRewards();
	void LoadZoneDoors();
	void ReloadStaticData();
	void ReloadWorld(uint8 global_repop);
	void RemoveAuth(const char *iCharName, const char *iLSKey);
	void RemoveAuth(uint32 lsid);
	void Repop();
	void RequestUCSServerStatus();
	void ResetAuth();
	void SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute);
	void SetGraveyard(uint32 zoneid, const glm::vec4 &graveyardPosition);
	void SetInstanceTimer(uint32 new_duration);
	void SetStaticZone(bool sz) { staticzone = sz; }
	void SetTime(uint8 hour, uint8 minute, bool update_world = true);
	void SetUCSServerAvailable(bool ucss_available, uint32 update_timestamp);
	void SpawnConditionChanged(const SpawnCondition &c, int16 old_value);
	void StartShutdownTimer(uint32 set_time = (RuleI(Zone, AutoShutdownDelay)));
	void UpdateQGlobal(uint32 qid, QGlobal newGlobal);
	void weatherSend(Client *client = nullptr);
	void ClearSpawnTimers();

	bool IsQuestHotReloadQueued() const;
	void SetQuestHotReloadQueued(bool in_quest_hot_reload_queued);

	WaterMap *watermap;
	ZonePoint *GetClosestZonePoint(const glm::vec3 &location, uint32 to, Client *client, float max_distance = 40000.0f);
	ZonePoint *GetClosestZonePointWithoutZone(float x, float y, float z, Client *client, float max_distance = 40000.0f);

	Timer GetInitgridsTimer();
	uint32 GetInstanceTimeRemaining() const;
	void SetInstanceTimeRemaining(uint32 instance_time_remaining);

	/**
	 * GMSay Callback for LogSys
	 *
	 * @param log_category
	 * @param message
	 */
	static void GMSayHookCallBackProcess(uint16 log_category, std::string message)
	{
		/**
		 * Cut messages down to 4000 max to prevent client crash
		 */
		if (!message.empty()) {
			message = message.substr(0, 4000);
		}

		/**
		 * Replace Occurrences of % or MessageStatus will crash
		 */
		find_replace(message, std::string("%"), std::string("."));

		if (message.find('\n') != std::string::npos) {
			auto message_split = SplitString(message, '\n');
			entity_list.MessageStatus(
				0,
				AccountStatus::QuestTroupe,
				LogSys.GetGMSayColorFromCategory(log_category),
				message_split[0].c_str()
			);

			for (size_t iter = 1; iter < message_split.size(); ++iter) {
				entity_list.MessageStatus(
					0,
					AccountStatus::QuestTroupe,
					LogSys.GetGMSayColorFromCategory(log_category),
					fmt::format(
						"--- {}",
						message_split[iter]
					).c_str()
				);
			}
		}
		else {
			entity_list.MessageStatus(
				0,
				AccountStatus::QuestTroupe,
				LogSys.GetGMSayColorFromCategory(log_category),
				message.c_str()
			);
		}
	}

	static void SendDiscordMessage(int webhook_id, const std::string& message);
	static void SendDiscordMessage(const std::string& webhook_name, const std::string& message);
	static void DiscordWebhookMessageHandler(uint16 log_category, int webhook_id, const std::string &message)
	{
		std::string message_prefix;
		if (!LogSys.origination_info.zone_short_name.empty()) {
			message_prefix = fmt::format(
				"[**{}**] **Zone** [**{}**] ",
				Logs::LogCategoryName[log_category],
				LogSys.origination_info.zone_short_name
			);
		}

		SendDiscordMessage(webhook_id, message_prefix + Discord::FormatDiscordMessage(log_category, message));
	};

	double GetMaxMovementUpdateRange() const { return max_movement_update_range; }

	/**
	 * Modding hooks
	 */
	void mod_init();
	void mod_repop();
	void SetIsHotzone(bool is_hotzone);

private:
	bool      allow_mercs;
	bool      can_bind;
	bool      can_castoutdoor;
	bool      can_combat;
	bool      can_levitate;
	bool      is_city;
	bool      is_hotzone;
	bool      pers_instance;
	bool      pvpzone;
	bool      m_ucss_available;
	bool      staticzone;
	bool      zone_has_current_time;
	bool      quest_hot_reload_queued;
	double    max_movement_update_range;
	char      *long_name;
	char      *map_name;
	char      *short_name;
	char      file_name[16];
	glm::vec4 m_SafePoint;
	glm::vec4 m_Graveyard;
	int       default_ruleset;
	int       zone_total_blocked_spells;
	int       npc_position_update_distance;
	int32     aggroedmobs;
	uint8     zone_type;
	uint16    instanceversion;
	uint32    instanceid;
	uint32    instance_time_remaining;
	uint32    pgraveyard_id, pgraveyard_zoneid;
	uint32    pMaxClients;
	uint32    zoneid;
	uint32    m_last_ucss_update;

	GlobalLootManager                   m_global_loot;
	LinkedList<ZoneClientAuth_Struct *> client_auth_list;
	MobMovementManager                  *mMovementManager;
	QGlobalCache                        *qGlobals;
	Timer                               *Instance_Shutdown_Timer;
	Timer                               *Instance_Timer;
	Timer                               *Instance_Warning_timer;
	Timer                               *Weather_Timer;
	Timer                               autoshutdown_timer;
	Timer                               clientauth_timer;
	Timer                               initgrids_timer;
	Timer                               qglobal_purge_timer;
	ZoneSpellsBlocked                   *blocked_spells;

};

#endif
