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

#ifndef BOT_H
#define BOT_H

#include "bot_structs.h"
#include "mob.h"
#include "client.h"
#include "pets.h"
#include "heal_rotation.h"
#include "groups.h"
#include "corpse.h"
#include "zonedb.h"
#include "../common/zone_store.h"
#include "string_ids.h"
#include "../common/misc_functions.h"
#include "../common/global_define.h"
#include "guild_mgr.h"
#include "worldserver.h"

#include <sstream>

constexpr uint32 BOT_FOLLOW_DISTANCE_DEFAULT = 184; // as DSq value (~13.565 units)
constexpr uint32 BOT_FOLLOW_DISTANCE_DEFAULT_MAX = 2500; // as DSq value (50 units)
constexpr uint32 BOT_FOLLOW_DISTANCE_WALK = 1000; // as DSq value (~31.623 units)

constexpr uint32 BOT_KEEP_ALIVE_INTERVAL = 5000; // 5 seconds

//constexpr uint32 BOT_COMBAT_JITTER_INTERVAL_MIN = 5000; // 5 seconds
//constexpr uint32 BOT_COMBAT_JITTER_INTERVAL_MAX = 20000; // 20 seconds

extern WorldServer worldserver;

constexpr int BotAISpellRange = 100; // TODO: Write a method that calcs what the bot's spell range is based on spell, equipment, AA, whatever and replace this
constexpr int MaxSpellTimer = 15;
constexpr int MaxDisciplineTimer = 10;
constexpr int DisciplineReuseStart = MaxSpellTimer + 1;
constexpr int MaxTimer = MaxSpellTimer + MaxDisciplineTimer;



// nHSND	negative Healer/Slower/Nuker/Doter
// pH		positive Healer
// pS		positive Slower
// pHS		positive Healer/Slower
// pN		positive Nuker
// pHN		positive Healer/Nuker
// pSN		positive Slower/Nuker
// pHSN		positive Healer/Slower/Nuker
// pD		positive Doter
// pHD		positive Healer/Doter
// pSD		positive Slower/Doter
// pHSD		positive Healer/Slower/Doter
// pND		positive Nuker/Doter
// pHND		positive Healer/Nuker/Doter
// pSND		positive Slower/Nuker/Doter
// pHSND	positive Healer/Slower/Nuker/Doter
// cntHSND	count Healer/Slower/Nuker/Doter
enum BotCastingChanceConditional : uint8
{
	nHSND = 0,
	pH,
	pS,
	pHS,
	pN,
	pHN,
	pSN,
	pHSN,
	pD,
	pHD,
	pSD,
	pHSD,
	pND,
	pHND,
	pSND,
	pHSND,
	cntHSND = 16
};


class Bot : public NPC {
	friend class Mob;
public:
	// Class enums
	enum BotTradeType {	// types of trades a bot can do
		BotTradeClientNormal,
		BotTradeClientNoDropNoTrade
	};

	enum BotRoleType {
		BotRoleMainAssist,
		BotRoleGroupHealer,
		BotRoleRaidHealer
	};

	enum SpellTypeIndex : uint32 {
		spellTypeIndexNuke,
		spellTypeIndexHeal,
		spellTypeIndexRoot,
		spellTypeIndexBuff,
		spellTypeIndexEscape,
		spellTypeIndexPet,
		spellTypeIndexLifetap,
		spellTypeIndexSnare,
		spellTypeIndexDot,
		spellTypeIndexDispel,
		spellTypeIndexInCombatBuff,
		spellTypeIndexMez,
		spellTypeIndexCharm,
		spellTypeIndexSlow,
		spellTypeIndexDebuff,
		spellTypeIndexCure,
		spellTypeIndexResurrect,
		spellTypeIndexHateRedux,
		spellTypeIndexInCombatBuffSong,
		spellTypeIndexOutOfCombatBuffSong,
		spellTypeIndexPreCombatBuff,
		spellTypeIndexPreCombatBuffSong
	};

	static const uint32 SPELL_TYPE_FIRST = spellTypeIndexNuke;
	static const uint32 SPELL_TYPE_LAST = spellTypeIndexPreCombatBuffSong;
	static const uint32 SPELL_TYPE_COUNT = SPELL_TYPE_LAST + 1;

	// Class Constructors
	Bot(NPCType *npcTypeData, Client* botOwner);
	Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, double totalPlayTime, uint32 lastZoneId, NPCType *npcTypeData);

	//abstract virtual override function implementations requird by base abstract class
	bool Death(Mob* killerMob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill) override;
	void Damage(Mob* from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1,
		bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None) override;

	bool HasRaid() override { return (GetRaid() ? true : false); }
	bool HasGroup() override { return (GetGroup() ? true : false); }
	Raid* GetRaid() override { return entity_list.GetRaidByMob(this); }
	Group* GetGroup() override { return entity_list.GetGroupByMob(this); }

	// Common, but informal "interfaces" with Client object
	uint32 CharacterID() { return GetBotID(); } // Just returns the Bot Id
	inline bool IsInAGuild() const { return (_guildId != GUILD_NONE && _guildId != 0); }
	inline bool IsInGuild(uint32 in_gid) const { return (in_gid == _guildId && IsInAGuild()); }
	inline uint32 GuildID() const { return _guildId; }
	inline uint8	GuildRank()	const { return _guildRank; }

	// Class Methods
	bool IsValidRaceClassCombo();
	static bool IsValidRaceClassCombo(uint16 r, uint8 c);
	bool IsValidName();
	static bool IsValidName(std::string& name);
	bool Spawn(Client* botCharacterOwner);
	void SetLevel(uint8 in_level, bool command = false) override;
	void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) override;
	bool Process() override;
	void FinishTrade(Client* client, BotTradeType trade_type);
	bool Save() override;
	void Depop();
	void CalcBotStats(bool showtext = true);
	uint16 BotGetSpells(int spellslot) { return AIBot_spells[spellslot].spellid; }
	uint32 BotGetSpellType(int spellslot) { return AIBot_spells[spellslot].type; }
	uint16 BotGetSpellPriority(int spellslot) { return AIBot_spells[spellslot].priority; }
	float GetProcChances(float ProcBonus, uint16 hand) override;
	int GetHandToHandDamage(void) override;
	bool TryFinishingBlow(Mob *defender, int64 &damage) override;
	void DoRiposte(Mob* defender) override;
	inline int32 GetATK() const override { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(EQ::skills::SkillOffense)) * 9 / 10); }
	inline int32 GetATKBonus() const override { return itembonuses.ATK + spellbonuses.ATK; }
	uint32 GetTotalATK();
	uint32 GetATKRating();
	uint16 GetPrimarySkillValue();
	uint16	MaxSkill(EQ::skills::SkillType skillid, uint16 class_, uint16 level) const;
	inline	uint16	MaxSkill(EQ::skills::SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }
	int GetBaseSkillDamage(EQ::skills::SkillType skill, Mob *target = nullptr) override;
	void DoSpecialAttackDamage(Mob *who, EQ::skills::SkillType skill, int32 max_damage, int32 min_damage = 1, int32 hate_override = -1, int ReuseTime = 10, bool HitChance = false);
	void TryBackstab(Mob *other,int ReuseTime = 10) override;
	void RogueBackstab(Mob* other, bool min_damage = false, int ReuseTime = 10) override;
	void RogueAssassinate(Mob* other) override;
	void DoClassAttacks(Mob *target, bool IsRiposte=false);
	bool CanDoSpecialAttack(Mob *other);
	void CalcBonuses() override;
	void CalcItemBonuses(StatBonuses* newbon);
	void AddItemBonuses(const EQ::ItemInstance *inst, StatBonuses* newbon, bool isAug = false, bool isTribute = false, int rec_override = 0);
	int CalcRecommendedLevelBonus(uint8 level, uint8 reclevel, int basestat);
	void MakePet(uint16 spell_id, const char* pettype, const char *petname = nullptr) override;
	FACTION_VALUE GetReverseFactionCon(Mob* iOther) override;
	inline bool IsPet() override { return false; }
	bool IsNPC() const override { return false; }
	Mob* GetOwner() override;
	Mob* GetOwnerOrSelf() override;
	inline bool HasOwner() override { return (GetBotOwner() ? true : false); }
	int64 CalcMaxMana() override;
	void SetAttackTimer() override;
	uint64 GetClassHPFactor();
	int64 CalcMaxHP() override;
	bool DoFinishedSpellAETarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool &stopLogic);
	bool DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool &stopLogic);
	bool DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool &stopLogic);
	void SendBotArcheryWearChange(uint8 material_slot, uint32 material, uint32 color);
	void Camp(bool save_to_database = true);
	void SetTarget(Mob* mob) override;
	void Zone();
	bool IsArcheryRange(Mob* target);
	void ChangeBotArcherWeapons(bool isArcher);
	void Sit();
	void Stand();
	bool IsSitting();
	bool IsStanding();
	int GetWalkspeed() const override { return (int)((float)_GetWalkSpeed() * 1.785714285f); } // 1.25 / 0.7 = 1.7857142857142857142857142857143
	int GetRunspeed() const override { return (int)((float)_GetRunSpeed() * 1.785714285f); }
	void WalkTo(float x, float y, float z) override;
	void RunTo(float x, float y, float z) override;
	void StopMoving() override;
	void StopMoving(float new_heading) override;
	//bool GetCombatJitterFlag() { return m_combat_jitter_flag; }
	bool GetGuardFlag() { return m_guard_flag; }
	void SetGuardFlag(bool flag = true) { m_guard_flag = flag; }
	bool GetHoldFlag() { return m_hold_flag; }
	void SetHoldFlag(bool flag = true) { m_hold_flag = flag; }
	bool GetAttackFlag() { return m_attack_flag; }
	void SetAttackFlag(bool flag = true) { m_attack_flag = flag; }
	bool GetAttackingFlag() { return m_attacking_flag; }
	bool GetPullFlag() { return m_pull_flag; }
	void SetPullFlag(bool flag = true) { m_pull_flag = flag; }
	bool GetPullingFlag() { return m_pulling_flag; }
	bool GetReturningFlag() { return m_returning_flag; }
	bool UseDiscipline(uint32 spell_id, uint32 target);
	uint8 GetNumberNeedingHealedInGroup(uint8 hpr, bool includePets);
	bool GetNeedsCured(Mob *tar);
	bool GetNeedsHateRedux(Mob *tar);
	bool HasOrMayGetAggro();
	void SetDefaultBotStance();
	void SetSurname(std::string bot_surname);
	void SetTitle(std::string bot_title);
	void SetSuffix(std::string bot_suffix);
	std::string GetSurname() { return _surname; }
	std::string GetTitle() { return _title; }
	std::string GetSuffix() { return _suffix; }
	inline virtual int32	GetMaxStat();
	inline virtual int32	GetMaxResist();
	inline virtual int32	GetMaxSTR();
	inline virtual int32	GetMaxSTA();
	inline virtual int32	GetMaxDEX();
	inline virtual int32	GetMaxAGI();
	inline virtual int32	GetMaxINT();
	inline virtual int32	GetMaxWIS();
	inline virtual int32	GetMaxCHA();
	inline virtual int32	GetMaxMR();
	inline virtual int32	GetMaxPR();
	inline virtual int32	GetMaxDR();
	inline virtual int32	GetMaxCR();
	inline virtual int32	GetMaxFR();
	inline virtual int32	GetMaxCorrup();
	int32	CalcATK();
	int32	CalcSTR();
	int32	CalcSTA();
	int32	CalcDEX();
	int32	CalcAGI();
	int32	CalcINT();
	int32	CalcWIS();
	int32	CalcCHA();
	int32	CalcMR();
	int32	CalcFR();
	int32	CalcDR();
	int32	CalcPR();
	int32	CalcCR();
	int32	CalcCorrup();
	int64	CalcHPRegenCap();
	int64	CalcManaRegenCap();
	int32	LevelRegen();
	int64	CalcHPRegen();
	int64	CalcManaRegen();
	uint32	CalcCurrentWeight();
	int		GroupLeadershipAAHealthEnhancement();
	int		GroupLeadershipAAManaEnhancement();
	int	GroupLeadershipAAHealthRegeneration();
	int		GroupLeadershipAAOffenseEnhancement();
	void CalcRestState();
	int64	CalcMaxEndurance();	//This calculates the maximum endurance we can have
	int64	CalcBaseEndurance();	//Calculates Base End
	int64	CalcEnduranceRegen();	//Calculates endurance regen used in DoEnduranceRegen()
	int64	GetEndurance()	const {return cur_end;}	//This gets our current endurance
	int64	GetMaxEndurance() const {return max_end;}	//This gets our endurance from the last CalcMaxEndurance() call
	int64	CalcEnduranceRegenCap();
	inline uint8 GetEndurancePercent() { return (uint8)((float)cur_end / (float)max_end * 100.0f); }
	void SetEndurance(int32 newEnd);	//This sets the current endurance to the new value
	void DoEnduranceRegen();	//This Regenerates endurance
	void DoEnduranceUpkeep();	//does the endurance upkeep

	bool AI_AddBotSpells(uint32 bot_spell_id);
	void AddSpellToBotList(
		int16 iPriority,
		uint16 iSpellID,
		uint32 iType,
		int16 iManaCost,
		int32 iRecastDelay,
		int16 iResistAdjust,
		uint8 min_level,
		uint8 max_level,
		int8 min_hp,
		int8 max_hp,
		std::string bucket_name,
		std::string bucket_value,
		uint8 bucket_comparison
	);

	void AddSpellToBotEnforceList(
		int16 iPriority,
		uint16 iSpellID,
		uint32 iType,
		int16 iManaCost,
		int32 iRecastDelay,
		int16 iResistAdjust,
		uint8 min_level,
		uint8 max_level,
		int8 min_hp,
		int8 max_hp,
		std::string bucket_name,
		std::string bucket_value,
		uint8 bucket_comparison
	);

	void AI_Bot_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot);
	// AI Methods
	bool AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes);
	bool AI_EngagedCastCheck() override;
	bool AI_PursueCastCheck() override;
	bool AI_IdleCastCheck() override;
	bool AIHealRotation(Mob* tar, bool useFastHeals);
	bool GetPauseAI() { return _pauseAI; }
	void SetPauseAI(bool pause_flag) { _pauseAI = pause_flag; }
	uint8 GetStopMeleeLevel() { return _stopMeleeLevel; }
	void SetStopMeleeLevel(uint8 level);
	void SetGuardMode();
	void SetHoldMode();

	// Bot AI Methods
	void AI_Bot_Init();
	void AI_Bot_Start(uint32 iMoveDelay = 0);

	// Mob AI Virtual Override Methods
	void AI_Process() override;
	void AI_Stop() override;

	// Mob Spell Virtual Override Methods
	void SpellProcess() override;
	int32 GetActSpellDuration(uint16 spell_id, int32 duration) override;
	float GetAOERange(uint16 spell_id) override;
	virtual bool SpellEffect(Mob* caster, uint16 spell_id, float partial = 100);
	void DoBuffTic(const Buffs_Struct &buff, int slot, Mob* caster = nullptr) override;
	virtual bool CastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot = EQ::spells::CastingSlot::Item, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0,
						uint32 item_slot = 0xFFFFFFFF, int16 *resist_adjust = nullptr, uint32 aa_id = 0);
	virtual bool SpellOnTarget(uint16 spell_id, Mob* spelltar);
	bool IsImmuneToSpell(uint16 spell_id, Mob *caster) override;
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, EQ::spells::CastingSlot slot);
	virtual bool DoCastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot = EQ::spells::CastingSlot::Item, int32 casttime = -1, int32 mana_cost = -1,
						uint32* oSpellWillFinish = 0, uint32 item_slot = 0xFFFFFFFF, uint32 aa_id = 0);
	inline int64 GetFocusEffect(focusType type, uint16 spell_id, Mob *caster = nullptr, bool from_buff_tic = false) override
		{ return Mob::GetFocusEffect(type, spell_id, caster, from_buff_tic); }
	inline bool Attack(Mob* other, int Hand = EQ::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false,
		bool IsFromSpell = false, ExtraAttackOptions *opts = nullptr) override
			{ return Mob::Attack(other, Hand, FromRiposte, IsStrikethrough, IsFromSpell, opts); }

	bool GetBotOwnerDataBuckets();
	bool GetBotDataBuckets();
	bool CheckDataBucket(std::string bucket_name, std::string bucket_value, uint8 bucket_comparison);

	// Bot Equipment & Inventory Class Methods
	void BotTradeAddItem(const EQ::ItemInstance* inst, uint16 slot_id, std::string* error_message, bool save_to_database = true);
	void EquipBot(std::string* error_message);
	bool CheckLoreConflict(const EQ::ItemData* item);
	void UpdateEquipmentLight() override { m_Light.Type[EQ::lightsource::LightEquipment] = m_inv.FindBrightestLightType(); m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]); }
	inline EQ::InventoryProfile& GetInv() { return m_inv; }

	// Static Class Methods
	//static void DestroyBotRaidObjects(Client* client);	// Can be removed after bot raids are dumped
	static Bot* LoadBot(uint32 botID);
	static uint32 SpawnedBotCount(const uint32 owner_id, uint8 class_id = NO_CLASS);
	static void LevelBotWithClient(Client* client, uint8 level, bool sendlvlapp);
	//static bool SetBotOwnerCharacterID(uint32 botID, uint32 botOwnerCharacterID, std::string* error_message);
	static bool IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined);
	static Bot* GetBotByBotClientOwnerAndBotName(Client* c, std::string botName);
	static void ProcessBotGroupInvite(Client* c, std::string botName);
	static void ProcessBotGroupDisband(Client* c, std::string botName);
	static void BotOrderCampAll(Client* c, uint8 class_id = NO_CLASS);
	static void ProcessBotInspectionRequest(Bot* inspectedBot, Client* client);
	static void LoadAndSpawnAllZonedBots(Client* bot_owner);
	static bool GroupHasBot(Group* group);
	static Bot* GetFirstBotInGroup(Group* group);
	static void ProcessClientZoneChange(Client* botOwner);
	static void ProcessBotOwnerRefDelete(Mob* botOwner);	// Removes a Client* reference when the Client object is destroyed
	static void ProcessGuildInvite(Client* guildOfficer, Bot* botToGuild);	// Processes a client's request to guild a bot
	static bool ProcessGuildRemoval(Client* guildOfficer, std::string botName);	// Processes a client's request to deguild a bot
	static int32 GetSpellRecastTimer(Bot *caster, int timer_index);
	static bool CheckSpellRecastTimers(Bot *caster, int SpellIndex);
	static int32 GetDisciplineRecastTimer(Bot *caster, int timer_index);
	static bool CheckDisciplineRecastTimers(Bot *caster, int timer_index);
	static uint32 GetDisciplineRemainingTime(Bot *caster, int timer_index);

	static std::list<BotSpell> GetBotSpellsForSpellEffect(Bot* botCaster, int spellEffect);
	static std::list<BotSpell> GetBotSpellsForSpellEffectAndTargetType(Bot* botCaster, int spellEffect, SpellTargetType targetType);
	static std::list<BotSpell> GetBotSpellsBySpellType(Bot* botCaster, uint32 spellType);
	static std::list<BotSpell_wPriority> GetPrioritizedBotSpellsBySpellType(Bot* botCaster, uint32 spellType);

	static BotSpell GetFirstBotSpellBySpellType(Bot* botCaster, uint32 spellType);
	static BotSpell GetBestBotSpellForFastHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForHealOverTime(Bot* botCaster);
	static BotSpell GetBestBotSpellForPercentageHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForRegularSingleTargetHeal(Bot* botCaster);
	static BotSpell GetFirstBotSpellForSingleTargetHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForGroupHealOverTime(Bot* botCaster);
	static BotSpell GetBestBotSpellForGroupCompleteHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForGroupHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForMagicBasedSlow(Bot* botCaster);
	static BotSpell GetBestBotSpellForDiseaseBasedSlow(Bot* botCaster);

	static Mob* GetFirstIncomingMobToMez(Bot* botCaster, BotSpell botSpell);
	static BotSpell GetBestBotSpellForMez(Bot* botCaster);
	static BotSpell GetBestBotMagicianPetSpell(Bot* botCaster);
	static std::string GetBotMagicianPetType(Bot* botCaster);
	static BotSpell GetBestBotSpellForNukeByTargetType(Bot* botCaster, SpellTargetType targetType);
	static BotSpell GetBestBotSpellForStunByTargetType(Bot* botCaster, SpellTargetType targetType);
	static BotSpell GetBestBotWizardNukeSpellByTargetResists(Bot* botCaster, Mob* target);
	static BotSpell GetDebuffBotSpell(Bot* botCaster, Mob* target);
	static BotSpell GetBestBotSpellForCure(Bot* botCaster, Mob* target);
	static BotSpell GetBestBotSpellForResistDebuff(Bot* botCaster, Mob* target);

	static NPCType *CreateDefaultNPCTypeStructForBot(
		std::string botName,
		std::string botLastName,
		uint8 botLevel,
		uint16 botRace,
		uint8 botClass,
		uint8 gender
	);

	// Static Bot Group Methods
	static bool AddBotToGroup(Bot* bot, Group* group);
	static bool RemoveBotFromGroup(Bot* bot, Group* group);
	static void BotGroupSay(Mob *speaker, const char *msg, ...);

	// "GET" Class Methods
	uint32 GetBotID() const { return _botID; }
	uint32 GetBotOwnerCharacterID() { return _botOwnerCharacterID; }
	uint32 GetBotSpellID() { return npc_spells_id; }
	Mob* GetBotOwner() { return this->_botOwner; }
	uint32 GetBotArcheryRange();
	EQ::ItemInstance* GetBotItem(uint16 slot_id);
	bool GetSpawnStatus() { return _spawnStatus; }
	uint8 GetPetChooserID() { return _petChooserID; }
	bool IsPetChooser() { return _petChooser; }
	bool IsBotArcher() { return m_bot_archery_setting; }
	bool IsBotCharmer() { return _botCharmer; }
	bool IsBot() const override { return true; }
	bool GetRangerAutoWeaponSelect() { return _rangerAutoWeaponSelect; }
	BotRoleType GetBotRole() { return _botRole; }
	EQ::constants::StanceType GetBotStance() { return _botStance; }
	uint8 GetChanceToCastBySpellType(uint32 spellType);
	bool GetBotEnforceSpellSetting() { return m_enforce_spell_settings; }

	bool IsGroupHealer() { return m_CastingRoles.GroupHealer; }
	bool IsGroupSlower() { return m_CastingRoles.GroupSlower; }
	bool IsGroupNuker() { return m_CastingRoles.GroupNuker; }
	bool IsGroupDoter() { return m_CastingRoles.GroupDoter; }
	static void UpdateGroupCastingRoles(const Group* group, bool disband = false);

	//bool IsRaidHealer() { return m_CastingRoles.RaidHealer; }
	//bool IsRaidSlower() { return m_CastingRoles.RaidSlower; }
	//bool IsRaidNuker() { return m_CastingRoles.RaidNuker; }
	//bool IsRaidDoter() { return m_CastingRoles.RaidDoter; }
	//static void UpdateRaidCastingRoles(const Raid* raid, bool disband = false);

	bool IsBotCaster() { return IsCasterClass(GetClass()); }
	bool IsBotHybrid() { return IsHybridClass(GetClass()); }
	bool IsBotINTCaster() { return IsINTCasterClass(GetClass()); }
	bool IsBotWISCaster() { return IsWISCasterClass(GetClass()); }
	bool IsBotSpellFighter() { return IsSpellFighterClass(GetClass()); }
	bool IsBotFighter() { return IsFighterClass(GetClass()); }
	bool IsBotNonSpellFighter() { return IsNonSpellFighterClass(GetClass()); }
	uint8 GetBotClass() { return GetClass(); }
	bool CanHeal();
	int GetRawACNoShield(int &shield_ac);

	// new heal rotation code
	bool CreateHealRotation(uint32 cycle_duration_ms = 5000, bool fast_heals = false, bool adaptive_targeting = false, bool casting_override = false);
	bool DestroyHealRotation();
	bool JoinHealRotationMemberPool(std::shared_ptr<HealRotation>* heal_rotation);
	bool LeaveHealRotationMemberPool();

	bool IsHealRotationMember() { return (m_member_of_heal_rotation.use_count() && m_member_of_heal_rotation.get()); }
	bool UseHealRotationFastHeals();
	bool UseHealRotationAdaptiveTargeting();

	bool IsHealRotationActive();
	bool IsHealRotationReady();
	bool IsHealRotationCaster();
	bool HealRotationPokeTarget();
	Mob* HealRotationTarget();
	bool AdvanceHealRotation(bool use_interval = true);

	bool IsMyHealRotationSet();
	bool AmICastingForHealRotation();
	void SetMyCastingForHealRotation(bool flag = true);

	std::shared_ptr<HealRotation>* MemberOfHealRotation() { return &m_member_of_heal_rotation; }

	bool GetAltOutOfCombatBehavior() { return _altoutofcombatbehavior;}
	bool GetShowHelm() { return _showhelm; }
	inline int32	GetSTR()	const override { return STR; }
	inline int32	GetSTA()	const override { return STA; }
	inline int32	GetDEX()	const override { return DEX; }
	inline int32	GetAGI()	const override { return AGI; }
	inline int32	GetINT()	const override { return INT; }
	inline int32	GetWIS()	const override { return WIS; }
	inline int32	GetCHA()	const override { return CHA; }
	inline int32	GetMR()	const override { return MR; }
	inline int32	GetFR()	const override { return FR; }
	inline int32	GetDR()	const override { return DR; }
	inline int32	GetPR()	const override { return PR; }
	inline int32	GetCR()	const override { return CR; }
	inline int32	GetCorrup()	const override { return Corrup; }
	//Heroic
	inline int32	GetHeroicSTR()	const override { return itembonuses.HeroicSTR; }
	inline int32	GetHeroicSTA()	const override { return itembonuses.HeroicSTA; }
	inline int32	GetHeroicDEX()	const override { return itembonuses.HeroicDEX; }
	inline int32	GetHeroicAGI()	const override { return itembonuses.HeroicAGI; }
	inline int32	GetHeroicINT()	const override { return itembonuses.HeroicINT; }
	inline int32	GetHeroicWIS()	const override { return itembonuses.HeroicWIS; }
	inline int32	GetHeroicCHA()	const override { return itembonuses.HeroicCHA; }
	inline int32	GetHeroicMR()	const override { return itembonuses.HeroicMR; }
	inline int32	GetHeroicFR()	const override { return itembonuses.HeroicFR; }
	inline int32	GetHeroicDR()	const override { return itembonuses.HeroicDR; }
	inline int32	GetHeroicPR()	const override { return itembonuses.HeroicPR; }
	inline int32	GetHeroicCR()	const override { return itembonuses.HeroicCR; }
	inline virtual int32	GetHeroicCorrup()	const { return itembonuses.HeroicCorrup; }
	// Mod2
	inline virtual int32	GetShielding()		const { return itembonuses.MeleeMitigation; }
	inline virtual int32	GetSpellShield()	const { return itembonuses.SpellShield; }
	inline virtual int32	GetDoTShield()		const { return itembonuses.DoTShielding; }
	inline virtual int32	GetStunResist()		const { return itembonuses.StunResist; }
	inline virtual int32	GetStrikeThrough()	const { return itembonuses.StrikeThrough; }
	inline virtual int32	GetAvoidance()		const { return itembonuses.AvoidMeleeChance; }
	inline virtual int32	GetAccuracy()		const { return itembonuses.HitChance; }
	inline virtual int32	GetCombatEffects()	const { return itembonuses.ProcChance; }
	inline virtual int32	GetDS()				const { return itembonuses.DamageShield; }
	// Mod3
	inline virtual int32	GetHealAmt()		const { return itembonuses.HealAmt; }
	inline virtual int32	GetSpellDmg()		const { return itembonuses.SpellDmg; }
	inline virtual int32	GetClair()			const { return itembonuses.Clairvoyance; }
	inline virtual int32	GetDSMit()			const { return itembonuses.DSMitigation; }

	inline virtual int32	GetSingMod()		const { return itembonuses.singingMod; }
	inline virtual int32	GetBrassMod()		const { return itembonuses.brassMod; }
	inline virtual int32	GetPercMod()		const { return itembonuses.percussionMod; }
	inline virtual int32	GetStringMod()		const { return itembonuses.stringedMod; }
	inline virtual int32	GetWindMod()		const { return itembonuses.windMod; }

	inline virtual int32	GetDelayDeath()		const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath; }

	inline InspectMessage_Struct& GetInspectMessage() { return _botInspectMessage; }
	inline const InspectMessage_Struct& GetInspectMessage() const { return _botInspectMessage; }

	// "Quest API" Methods
	bool HasBotSpellEntry(uint16 spellid);
	void ApplySpell(int spell_id, int duration = 0, ApplySpellType apply_type = ApplySpellType::Solo, bool allow_pets = false, bool is_raid_group_only = true);
	void BreakInvis();
	void Escape();
	void Fling(float value, float target_x, float target_y, float target_z, bool ignore_los = false, bool clip_through_walls = false, bool calculate_speed = false);
	std::vector<Mob*> GetApplySpellList(ApplySpellType apply_type, bool allow_pets, bool is_raid_group_only);
	int32 GetItemIDAt(int16 slot_id);
	int32 GetAugmentIDAt(int16 slot_id, uint8 augslot);
	int32 GetRawItemAC();
	void SendSpellAnim(uint16 targetid, uint16 spell_id);
	void SetSpellDuration(int spell_id, int duration = 0, ApplySpellType apply_type = ApplySpellType::Solo, bool allow_pets = false, bool is_raid_group_only = true);

	// "SET" Class Methods
	void SetBotSpellID(uint32 newSpellID);
	void SetSpawnStatus(bool spawnStatus) { _spawnStatus = spawnStatus; }
	void SetPetChooserID(uint8 id) { _petChooserID = id; }
	void SetBotArcherySetting(bool bot_archer_setting, bool save = false);
	void SetBotCharmer(bool c) { _botCharmer = c; }
	void SetPetChooser(bool p) { _petChooser = p; }
	void SetBotOwner(Mob* botOwner) { this->_botOwner = botOwner; }
	// void SetBotOwnerCharacterID(uint32 botOwnerCharacterID) { _botOwnerCharacterID = botOwnerCharacterID; }
	void SetRangerAutoWeaponSelect(bool enable) { GetClass() == RANGER ? _rangerAutoWeaponSelect = enable : _rangerAutoWeaponSelect = false; }
	void SetBotRole(BotRoleType botRole) { _botRole = botRole; }
	void SetBotStance(EQ::constants::StanceType botStance) {
		if (botStance >= EQ::constants::stancePassive && botStance <= EQ::constants::stanceBurnAE)
			_botStance = botStance;
		else
			_botStance = EQ::constants::stancePassive;
	}
	void SetSpellRecastTimer(int timer_index, int32 recast_delay);
	void SetDisciplineRecastTimer(int timer_index, int32 recast_delay);
	void SetAltOutOfCombatBehavior(bool behavior_flag) { _altoutofcombatbehavior = behavior_flag;}
	void SetShowHelm(bool showhelm) { _showhelm = showhelm; }
	void SetBeardColor(uint8 value) { beardcolor = value; }
	void SetBeard(uint8 value) { beard = value; }
	void SetEyeColor1(uint8 value) { eyecolor1 = value; }
	void SetEyeColor2(uint8 value) { eyecolor2 = value; }
	void SetLuclinFace(uint8 value) { luclinface = value; }
	void SetHairColor(uint8 value) { haircolor = value; }
	void SetHairStyle(uint8 value) { hairstyle = value; }
	void SetDrakkinDetails(uint32 value) { drakkin_details = value; }
	void SetDrakkinHeritage(uint32 value) { drakkin_heritage = value; }
	void SetDrakkinTattoo(uint32 value) { drakkin_tattoo = value; }
	bool DyeArmor(int16 slot_id, uint32 rgb, bool all_flag = false, bool save_flag = true);

	int GetExpansionBitmask();
	void SetExpansionBitmask(int expansion_bitmask, bool save = true);

	void ListBotSpells(uint8 min_level);

	std::string GetHPString(int8 min_hp, int8 max_hp);

	bool AddBotSpellSetting(uint16 spell_id, BotSpellSetting* bs);
	bool DeleteBotSpellSetting(uint16 spell_id);
	BotSpellSetting* GetBotSpellSetting(uint16 spell_id);
	void ListBotSpellSettings();
	void LoadBotSpellSettings();
	bool UpdateBotSpellSetting(uint16 spell_id, BotSpellSetting* bs);
	void SetBotEnforceSpellSetting(bool enforcespellsettings, bool save = false);
	bool GetBotEnforceSpellSetting() const { return m_enforce_spell_settings; }

	static void SpawnBotGroupByName(Client* c, std::string botgroup_name, uint32 leader_id);

	std::string CreateSayLink(Client* botOwner, const char* message, const char* name);

	// Class Destructors
	~Bot() override;

	// Publicized protected functions
	void BotRangedAttack(Mob* other);

	// Publicized private functions
	static NPCType *FillNPCTypeStruct(
		uint32 botSpellsID,
		std::string botName,
		std::string botLastName,
		uint8 botLevel,
		uint16 botRace,
		uint8 botClass,
		uint8 gender,
		float size,
		uint32 face,
		uint32 hairStyle,
		uint32 hairColor,
		uint32 eyeColor,
		uint32 eyeColor2,
		uint32 beard,
		uint32 beardColor,
		uint32 drakkinHeritage,
		uint32 drakkinTattoo,
		uint32 drakkinDetails,
		int32 hp,
		int32 mana,
		int32 mr,
		int32 cr,
		int32 dr,
		int32 fr,
		int32 pr,
		int32 corrup,
		int32 ac,
		uint32 str,
		uint32 sta,
		uint32 dex,
		uint32 agi,
		uint32 _int,
		uint32 wis,
		uint32 cha,
		uint32 attack
	);
	void BotRemoveEquipItem(uint16 slot_id);
	void RemoveBotItemBySlot(uint16 slot_id, std::string* error_message);
	void AddBotItem(
		uint16 slot_id,
		uint32 item_id,
		int16 charges = -1,
		bool attuned = false,
		uint32 augment_one = 0,
		uint32 augment_two = 0,
		uint32 augment_three = 0,
		uint32 augment_four = 0,
		uint32 augment_five = 0,
		uint32 augment_six = 0
	);
	uint32 CountBotItem(uint32 item_id);
	std::map<uint16, uint32> GetBotItemSlots();
	uint32 GetBotItemBySlot(uint16 slot_id);
	bool HasBotItem(uint32 item_id);
	void RemoveBotItem(uint32 item_id);
	uint32 GetTotalPlayTime();

	// New accessors for BotDatabase access
	bool DeleteBot();
	uint32* GetTimers() { return timers; }
	uint32 GetLastZoneID() { return _lastZoneId; }
	int32 GetBaseAC() { return _baseAC; }
	int32 GetBaseATK() { return _baseATK; }
	int32 GetBaseSTR() { return _baseSTR; }
	int32 GetBaseSTA() { return _baseSTA; }
	int32 GetBaseCHA() { return _baseCHA; }
	int32 GetBaseDEX() { return _baseDEX; }
	int32 GetBaseINT() { return _baseINT; }
	int32 GetBaseAGI() { return _baseAGI; }
	int32 GetBaseWIS() { return _baseWIS; }
	int32 GetBaseFR() { return _baseFR; }
	int32 GetBaseCR() { return _baseCR; }
	int32 GetBaseMR() { return _baseMR; }
	int32 GetBasePR() { return _basePR; }
	int32 GetBaseDR() { return _baseDR; }
	int32 GetBaseCorrup() { return _baseCorrup; }

	void Signal(int signal_id);
	void SendPayload(int payload_id, std::string payload_value = std::string());
	void OwnerMessage(std::string message);

protected:
	void PetAIProcess();
	void BotMeditate(bool isSitting);
	bool CheckBotDoubleAttack(bool Triple = false);
	void PerformTradeWithClient(int16 begin_slot_id, int16 end_slot_id, Client* client);
	bool AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = 0) override;

	BotCastingRoles& GetCastingRoles() { return m_CastingRoles; }
	void SetGroupHealer(bool flag = true) { m_CastingRoles.GroupHealer = flag; }
	void SetGroupSlower(bool flag = true) { m_CastingRoles.GroupSlower = flag; }
	void SetGroupNuker(bool flag = true) { m_CastingRoles.GroupNuker = flag; }
	void SetGroupDoter(bool flag = true) { m_CastingRoles.GroupDoter = flag; }
	//void SetRaidHealer(bool flag = true) { m_CastingRoles.RaidHealer = flag; }
	//void SetRaidSlower(bool flag = true) { m_CastingRoles.RaidSlower = flag; }
	//void SetRaidNuker(bool flag = true) { m_CastingRoles.RaidNuker = flag; }
	//void SetRaidDoter(bool flag = true) { m_CastingRoles.RaidDoter = flag; }
	std::deque<int> bot_signal_q;

	std::vector<BotSpells_Struct> AIBot_spells;
	std::vector<BotSpells_Struct> AIBot_spells_enforced;

private:
	// Class Members
	uint32 _botID;
	uint32 _botOwnerCharacterID;
	bool _spawnStatus;
	Mob* _botOwner;
	bool _botOrderAttack;
	bool m_bot_archery_setting;
	bool _botCharmer;
	bool _petChooser;
	uint8 _petChooserID;
	bool berserk;
	EQ::InventoryProfile m_inv;
	double _lastTotalPlayTime;
	time_t _startTotalPlayTime;
	Mob* _previousTarget;
	uint32 _guildId;
	uint8 _guildRank;
	std::string _guildName;
	std::string _surname;
	std::string _title;
	std::string _suffix;
	uint32 _lastZoneId;
	bool _rangerAutoWeaponSelect;
	BotRoleType _botRole;
	EQ::constants::StanceType _botStance;
	EQ::constants::StanceType _baseBotStance;
	unsigned int RestRegenHP;
	unsigned int RestRegenMana;
	unsigned int RestRegenEndurance;
	Timer rest_timer;
	Timer ping_timer;
	int32	base_end;
	int32	cur_end;
	int32	max_end;
	int32	end_regen;
	uint32 timers[MaxTimer];

	Timer m_evade_timer; // can be moved to pTimers at some point
	Timer m_alt_combat_hate_timer;
	Timer m_auto_defend_timer;
	//Timer m_combat_jitter_timer;
	//bool m_combat_jitter_flag;
	bool m_guard_flag;
	bool m_hold_flag;
	bool m_attack_flag;
	bool m_attacking_flag;
	bool m_pull_flag;
	bool m_pulling_flag;
	bool m_returning_flag;
	eStandingPetOrder m_previous_pet_order;

	BotCastingRoles m_CastingRoles;
	std::map<std::string,std::string> bot_data_buckets;
	std::map<std::string,std::string> bot_owner_data_buckets;

	std::map<uint16, BotSpellSetting> bot_spell_settings;

	std::shared_ptr<HealRotation> m_member_of_heal_rotation;

	std::map<uint32, BotAA> botAAs;
	InspectMessage_Struct _botInspectMessage;
	bool _altoutofcombatbehavior;
	bool _showhelm;
	bool _pauseAI;
	uint8 _stopMeleeLevel;
	int m_expansion_bitmask;
	bool m_enforce_spell_settings;

	// Private "base stats" Members
	int32 _baseMR;
	int32 _baseCR;
	int32 _baseDR;
	int32 _baseFR;
	int32 _basePR;
	int32 _baseCorrup;
	int32 _baseAC;
	int32 _baseSTR;
	int32 _baseSTA;
	int32 _baseDEX;
	int32 _baseAGI;
	int32 _baseINT;
	int32 _baseWIS;
	int32 _baseCHA;
	int32 _baseATK;
	uint16 _baseRace;	// Necessary to preserve the race otherwise bots get their race updated in the db when they get an illusion.
	uint8 _baseGender;	// Bots gender. Necessary to preserve the original value otherwise it can be changed by illusions.

	// Class Methods
	void LoadAAs();
	int32 acmod();
	void GenerateBaseStats();
	void GenerateAppearance();
	void GenerateArmorClass();
	int32 GenerateBaseHitPoints();
	int32 GenerateBaseManaPoints();
	void GenerateSpecialAttacks();
	void SetBotID(uint32 botID);
	//void SetCombatJitterFlag(bool flag = true) { m_combat_jitter_flag = flag; }
	void SetAttackingFlag(bool flag = true) { m_attacking_flag = flag; }
	void SetPullingFlag(bool flag = true) { m_pulling_flag = flag; }
	void SetReturningFlag(bool flag = true) { m_returning_flag = flag; }

	// Private "Inventory" Methods
	void GetBotItems(EQ::InventoryProfile &inv, std::string* error_message);
	void BotAddEquipItem(uint16 slot_id, uint32 item_id);

	// Private "Pet" Methods
	bool LoadPet();	// Load and spawn bot pet if there is one
	bool SavePet();	// Save and depop bot pet if there is one
	bool DeletePet();

	public:
	static uint8 spell_casting_chances[SPELL_TYPE_COUNT][PLAYER_CLASS_COUNT][EQ::constants::STANCE_TYPE_COUNT][cntHSND];
};

bool IsSpellInBotList(DBbotspells_Struct* spell_list, uint16 iSpellID);

#endif // BOT_H
