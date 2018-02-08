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

#ifdef BOTS

#include "bot_structs.h"
#include "mob.h"
#include "client.h"
#include "pets.h"
#include "heal_rotation.h"
#include "groups.h"
#include "corpse.h"
#include "zonedb.h"
#include "bot_database.h"
#include "string_ids.h"
#include "../common/misc_functions.h"
#include "../common/global_define.h"
#include "guild_mgr.h"
#include "worldserver.h"

#include <sstream>

#define BOT_FOLLOW_DISTANCE_DEFAULT 184 // as DSq value (~13.565 units)
#define BOT_FOLLOW_DISTANCE_DEFAULT_MAX 2500 // as DSq value (50 units)
#define BOT_FOLLOW_DISTANCE_WALK 1000 // as DSq value (~31.623 units)

#define BOT_LEASH_DISTANCE 250000 // as DSq value (500 units)

extern WorldServer worldserver;

const int BotAISpellRange = 100; // TODO: Write a method that calcs what the bot's spell range is based on spell, equipment, AA, whatever and replace this
const int MaxSpellTimer = 15;
const int MaxDisciplineTimer = 10;
const int DisciplineReuseStart = MaxSpellTimer + 1;
const int MaxTimer = MaxSpellTimer + MaxDisciplineTimer;

enum BotStanceType {
	BotStancePassive,
	BotStanceBalanced,
	BotStanceEfficient,
	BotStanceReactive,
	BotStanceAggressive,
	BotStanceBurn,
	BotStanceBurnAE,
	BotStanceUnknown,
	MaxStances = BotStanceUnknown
};

#define BOT_STANCE_COUNT 8
#define VALIDBOTSTANCE(x) ((x >= (int)BotStancePassive && x <= (int)BotStanceBurnAE) ? ((BotStanceType)x) : (BotStanceUnknown))

static const std::string bot_stance_name[BOT_STANCE_COUNT] = {
	"Passive",		// 0
	"Balanced",		// 1
	"Efficient",	// 2
	"Reactive",		// 3
	"Aggressive",	// 4
	"Burn",			// 5
	"BurnAE",		// 6
	"Unknown"		// 7
};

static const char* GetBotStanceName(int stance_id) { return bot_stance_name[VALIDBOTSTANCE(stance_id)].c_str(); }

#define VALIDBOTEQUIPSLOT(x) ((x >= EQEmu::legacy::EQUIPMENT_BEGIN && x <= EQEmu::legacy::EQUIPMENT_END) ? (x) : ((x == EQEmu::inventory::slotPowerSource) ? (22) : (23)))

static std::string bot_equip_slot_name[EQEmu::legacy::EQUIPMENT_SIZE + 2] =
{
	"Charm",			// MainCharm
	"Left Ear",			// MainEar1
	"Head",				// MainHead
	"Face",				// MainFace
	"Right Ear",		// MainEar2
	"Neck",				// MainNeck 
	"Shoulders",		// MainShoulders
	"Arms",				// MainArms
	"Back",				// MainBack
	"Left Wrist",		// MainWrist1
	"Right Wrist",		// MainWrist2
	"Range",			// MainRange
	"Hands",			// MainHands
	"Primary Hand",		// MainPrimary
	"Secondary Hand",	// MainSecondary
	"Left Finger",		// MainFinger1
	"Right Finger",		// MainFinger2
	"Chest",			// MainChest
	"Legs",				// MainLegs
	"Feet",				// MainFeet
	"Waist",			// MainWaist
	"Ammo",				// MainAmmo
	"Power Source",		// 22 (MainPowerSource = 9999)
	"Unknown"
};

static const char* GetBotEquipSlotName(int slot_id) { return bot_equip_slot_name[VALIDBOTEQUIPSLOT(slot_id)].c_str(); }

enum SpellTypeIndex {
	SpellType_NukeIndex,
	SpellType_HealIndex,
	SpellType_RootIndex,
	SpellType_BuffIndex,
	SpellType_EscapeIndex,
	SpellType_PetIndex,
	SpellType_LifetapIndex,
	SpellType_SnareIndex,
	SpellType_DOTIndex,
	SpellType_DispelIndex,
	SpellType_InCombatBuffIndex,
	SpellType_MezIndex,
	SpellType_CharmIndex,
	SpellType_SlowIndex,
	SpellType_DebuffIndex,
	SpellType_CureIndex,
	SpellType_ResurrectIndex,
	SpellType_HateReduxIndex,
	SpellType_InCombatBuffSongIndex,
	SpellType_OutOfCombatBuffSongIndex,
	SpellType_PreCombatBuffIndex,
	SpellType_PreCombatBuffSongIndex,
	MaxSpellTypes
};

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
	enum BotfocusType {	//focus types
		BotfocusSpellHaste = 1,
		BotfocusSpellDuration,
		BotfocusRange,
		BotfocusReagentCost,
		BotfocusManaCost,
		BotfocusImprovedHeal,
		BotfocusImprovedDamage,
		BotfocusImprovedDOT,		//i dont know about this...
		BotfocusFcDamagePctCrit,
		BotfocusImprovedUndeadDamage,
		BotfocusPetPower,
		BotfocusResistRate,
		BotfocusSpellHateMod,
		BotfocusTriggerOnCast,
		BotfocusSpellVulnerability,
		BotfocusTwincast,
		BotfocusSympatheticProc,
		BotfocusFcDamageAmt,
		BotfocusFcDamageAmtCrit,
		BotfocusSpellDurByTic,
		BotfocusSwarmPetDuration,
		BotfocusReduceRecastTime,
		BotfocusBlockNextSpell,
		BotfocusFcHealPctIncoming,
		BotfocusFcDamageAmtIncoming,
		BotfocusFcHealAmtIncoming,
		BotfocusFcBaseEffects,
		BotfocusIncreaseNumHits,
		BotfocusFcHealPctCritIncoming,
		BotfocusFcHealAmt,
		BotfocusFcHealAmtCrit,
	};

	enum BotTradeType {	// types of trades a bot can do
		BotTradeClientNormal,
		BotTradeClientNoDropNoTrade
	};

	enum BotRoleType {
		BotRoleMainAssist,
		BotRoleGroupHealer,
		BotRoleRaidHealer
	};

	enum EqExpansions { // expansions are off..EQ should be '0'
		ExpansionNone,
		ExpansionEQ,
		ExpansionRoK,
		ExpansionSoV,
		ExpansionSoL,
		ExpansionPoP,
		ExpansionLoY,
		ExpansionLDoN,
		ExpansionGoD,
		ExpansionOoW,
		ExpansionDoN,
		ExpansionDoDH,
		ExpansionPoR,
		ExpansionTSS,
		ExpansionSoF,
		ExpansionSoD,
		ExpansionUF,
		ExpansionHoT,
		ExpansionVoA,
		ExpansionRoF
	};

	// Class Constructors
	Bot(NPCType npcTypeData, Client* botOwner);
	Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, double totalPlayTime, uint32 lastZoneId, NPCType npcTypeData);

	//abstract virtual function implementations requird by base abstract class
	virtual bool Death(Mob* killerMob, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill);
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None);
	virtual bool Attack(Mob* other, int Hand = EQEmu::inventory::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false,
		ExtraAttackOptions *opts = nullptr);
	virtual bool HasRaid() { return (GetRaid() ? true : false); }
	virtual bool HasGroup() { return (GetGroup() ? true : false); }
	virtual Raid* GetRaid() { return entity_list.GetRaidByMob(this); }
	virtual Group* GetGroup() { return entity_list.GetGroupByMob(this); }

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
	void Spawn(Client* botCharacterOwner);
	virtual void SetLevel(uint8 in_level, bool command = false);
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	virtual bool Process();
	void FinishTrade(Client* client, BotTradeType tradeType);
	virtual bool Save();
	virtual void Depop();
	void CalcBotStats(bool showtext = true);
	uint16 BotGetSpells(int spellslot) { return AIspells[spellslot].spellid; }
	uint32 BotGetSpellType(int spellslot) { return AIspells[spellslot].type; }
	uint16 BotGetSpellPriority(int spellslot) { return AIspells[spellslot].priority; }
	virtual float GetProcChances(float ProcBonus, uint16 hand);
	virtual int GetHandToHandDamage(void);
	virtual bool TryFinishingBlow(Mob *defender, int &damage);
	virtual void DoRiposte(Mob* defender);
	inline virtual int32 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(EQEmu::skills::SkillOffense)) * 9 / 10); }
	inline virtual int32 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	uint32 GetTotalATK();
	uint32 GetATKRating();
	uint16 GetPrimarySkillValue();
	uint16	MaxSkill(EQEmu::skills::SkillType skillid, uint16 class_, uint16 level) const;
	inline	uint16	MaxSkill(EQEmu::skills::SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }
	virtual int GetBaseSkillDamage(EQEmu::skills::SkillType skill, Mob *target = nullptr);
	virtual void DoSpecialAttackDamage(Mob *who, EQEmu::skills::SkillType skill, int32 max_damage, int32 min_damage = 1, int32 hate_override = -1, int ReuseTime = 10, bool HitChance = false);
	virtual void TryBackstab(Mob *other,int ReuseTime = 10);
	virtual void RogueBackstab(Mob* other, bool min_damage = false, int ReuseTime = 10);
	virtual void RogueAssassinate(Mob* other);
	virtual void DoClassAttacks(Mob *target, bool IsRiposte=false);
	virtual void ApplySpecialAttackMod(EQEmu::skills::SkillType skill, int32 &dmg, int32 &mindmg);
	bool CanDoSpecialAttack(Mob *other);
	virtual int32 CheckAggroAmount(uint16 spellid);
	virtual void CalcBonuses();
	void CalcItemBonuses(StatBonuses* newbon);
	void AddItemBonuses(const EQEmu::ItemInstance *inst, StatBonuses* newbon, bool isAug = false, bool isTribute = false, int rec_override = 0);
	int CalcRecommendedLevelBonus(uint8 level, uint8 reclevel, int basestat);
	virtual void MakePet(uint16 spell_id, const char* pettype, const char *petname = nullptr);
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther);
	inline virtual bool IsPet() { return false; }
	virtual bool IsNPC() const { return false; }
	virtual Mob* GetOwner();
	virtual Mob* GetOwnerOrSelf();
	inline virtual bool HasOwner() { return (GetBotOwner() ? true : false); }
	virtual int32 CheckHealAggroAmount(uint16 spellid, Mob *target, uint32 heal_possible = 0);
	virtual int32 CalcMaxMana();
	virtual void SetAttackTimer();
	uint32 GetClassHPFactor();
	virtual int32 CalcMaxHP();
	bool DoFinishedSpellAETarget(uint16 spell_id, Mob* spellTarget, EQEmu::CastingSlot slot, bool &stopLogic);
	bool DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, EQEmu::CastingSlot slot, bool &stopLogic);
	bool DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, EQEmu::CastingSlot slot, bool &stopLogic);
	void SendBotArcheryWearChange(uint8 material_slot, uint32 material, uint32 color);
	void Camp(bool databaseSave = true);
	virtual void AddToHateList(Mob* other, uint32 hate = 0, int32 damage = 0, bool iYellForHelp = true, bool bFrenzy = false, bool iBuffTic = false, bool pet_command = false);
	virtual void SetTarget(Mob* mob);
	virtual void Zone();
	std::vector<AISpells_Struct> GetBotSpells() { return AIspells; }
	bool IsArcheryRange(Mob* target);
	void ChangeBotArcherWeapons(bool isArcher);
	void Sit();
	void Stand();
	bool IsSitting();
	bool IsStanding();
	int GetBotWalkspeed() const { return (int)((float)_GetWalkSpeed() * 1.786f); } // 1.25 / 0.7 = 1.7857142857142857142857142857143
	int GetBotRunspeed() const { return (int)((float)_GetRunSpeed() * 1.786f); }
	int GetBotFearSpeed() const { return (int)((float)_GetFearSpeed() * 1.786f); }
	bool UseDiscipline(uint32 spell_id, uint32 target);
	uint8 GetNumberNeedingHealedInGroup(uint8 hpr, bool includePets);
	bool GetNeedsCured(Mob *tar);
	bool GetNeedsHateRedux(Mob *tar);
	bool HasOrMayGetAggro();
	void SetDefaultBotStance();

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
	int32	CalcHPRegenCap();
	int32	CalcManaRegenCap();
	int32	LevelRegen();
	int32	CalcHPRegen();
	int32	CalcManaRegen();
	uint32	CalcCurrentWeight();
	int		GroupLeadershipAAHealthEnhancement();
	int		GroupLeadershipAAManaEnhancement();
	int	GroupLeadershipAAHealthRegeneration();
	int		GroupLeadershipAAOffenseEnhancement();
	void CalcRestState();
	int32	CalcMaxEndurance();	//This calculates the maximum endurance we can have
	int32	CalcBaseEndurance();	//Calculates Base End
	int32	CalcEnduranceRegen();	//Calculates endurance regen used in DoEnduranceRegen()
	int32	GetEndurance()	const {return cur_end;}	//This gets our current endurance
	int32	GetMaxEndurance() const {return max_end;}	//This gets our endurance from the last CalcMaxEndurance() call
	int32	CalcEnduranceRegenCap();
	inline uint8 GetEndurancePercent() { return (uint8)((float)cur_end / (float)max_end * 100.0f); }
	void SetEndurance(int32 newEnd);	//This sets the current endurance to the new value
	void DoEnduranceRegen();	//This Regenerates endurance
	void DoEnduranceUpkeep();	//does the endurance upkeep

	// AI Methods
	virtual bool AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes);
	virtual bool AI_EngagedCastCheck();
	virtual bool AI_PursueCastCheck();
	virtual bool AI_IdleCastCheck();
	bool AIHealRotation(Mob* tar, bool useFastHeals);
	bool GetPauseAI() { return _pauseAI; }
	void SetPauseAI(bool pause_flag) { _pauseAI = pause_flag; }
	void SetGuardMode();
	
	// Mob AI Virtual Override Methods
	virtual void AI_Process();
	virtual void AI_Stop();

	// Mob Spell Virtual Override Methods
	virtual void SpellProcess();
	virtual int32 GetActSpellDamage(uint16 spell_id, int32 value, Mob* target = nullptr);
	virtual int32 GetActSpellHealing(uint16 spell_id, int32 value, Mob* target = nullptr);
	virtual int32 GetActSpellCasttime(uint16 spell_id, int32 casttime);
	virtual int32 GetActSpellCost(uint16 spell_id, int32 cost);
	virtual float GetActSpellRange(uint16 spell_id, float range);
	virtual int32 GetActSpellDuration(uint16 spell_id, int32 duration);
	virtual float GetAOERange(uint16 spell_id);
	virtual bool SpellEffect(Mob* caster, uint16 spell_id, float partial = 100);
	virtual void DoBuffTic(const Buffs_Struct &buff, int slot, Mob* caster = nullptr);
	virtual bool CastSpell(uint16 spell_id, uint16 target_id, EQEmu::CastingSlot slot = EQEmu::CastingSlot::Item, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0,
						   uint32 item_slot = 0xFFFFFFFF, int16 *resist_adjust = nullptr, uint32 aa_id = 0);
	virtual bool SpellOnTarget(uint16 spell_id, Mob* spelltar);
	virtual bool IsImmuneToSpell(uint16 spell_id, Mob *caster);
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, EQEmu::CastingSlot slot);
	virtual bool DoCastSpell(uint16 spell_id, uint16 target_id, EQEmu::CastingSlot slot = EQEmu::CastingSlot::Item, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0, uint32 item_slot = 0xFFFFFFFF, uint32 aa_id = 0);

	// Bot Equipment & Inventory Class Methods
	void BotTradeSwapItem(Client* client, int16 lootSlot, const EQEmu::ItemInstance* inst, const EQEmu::ItemInstance* inst_swap, uint32 equipableSlots, std::string* errorMessage, bool swap = true);
	void BotTradeAddItem(uint32 id, const EQEmu::ItemInstance* inst, int16 charges, uint32 equipableSlots, uint16 lootSlot, std::string* errorMessage, bool addToDb = true);
	void EquipBot(std::string* errorMessage);
	bool CheckLoreConflict(const EQEmu::ItemData* item);
	virtual void UpdateEquipmentLight() { m_Light.Type[EQEmu::lightsource::LightEquipment] = m_inv.FindBrightestLightType(); m_Light.Level[EQEmu::lightsource::LightEquipment] = EQEmu::lightsource::TypeToLevel(m_Light.Type[EQEmu::lightsource::LightEquipment]); }

	// Static Class Methods	
	//static void DestroyBotRaidObjects(Client* client);	// Can be removed after bot raids are dumped
	static Bot* LoadBot(uint32 botID);
	static uint32 SpawnedBotCount(uint32 botOwnerCharacterID);
	static void LevelBotWithClient(Client* client, uint8 level, bool sendlvlapp);
	//static bool SetBotOwnerCharacterID(uint32 botID, uint32 botOwnerCharacterID, std::string* errorMessage);
	static std::string ClassIdToString(uint16 classId);
	static std::string RaceIdToString(uint16 raceId);
	static bool IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined);
	static Bot* GetBotByBotClientOwnerAndBotName(Client* c, std::string botName);
	static void ProcessBotGroupInvite(Client* c, std::string botName);
	static void ProcessBotGroupDisband(Client* c, std::string botName);
	static void BotOrderCampAll(Client* c);
	static void ProcessBotInspectionRequest(Bot* inspectedBot, Client* client);
	static void LoadAndSpawnAllZonedBots(Client* botOwner);
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
	
	static NPCType CreateDefaultNPCTypeStructForBot(std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender);

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
	EQEmu::ItemInstance* GetBotItem(uint32 slotID);
	virtual bool GetSpawnStatus() { return _spawnStatus; }
	uint8 GetPetChooserID() { return _petChooserID; }
	bool IsPetChooser() { return _petChooser; }
	bool IsBotArcher() { return _botArcher; }
	bool IsBotCharmer() { return _botCharmer; }
	virtual bool IsBot() const { return true; }
	bool GetRangerAutoWeaponSelect() { return _rangerAutoWeaponSelect; }
	BotRoleType GetBotRole() { return _botRole; }
	BotStanceType GetBotStance() { return _botStance; }
	uint8 GetChanceToCastBySpellType(uint32 spellType);

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
	bool IsBotINTCaster() { return IsINTCasterClass(GetClass()); }
	bool IsBotWISCaster() { return IsWISCasterClass(GetClass()); }
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
	inline virtual int32	GetSTR()	const { return STR; }
	inline virtual int32	GetSTA()	const { return STA; }
	inline virtual int32	GetDEX()	const { return DEX; }
	inline virtual int32	GetAGI()	const { return AGI; }
	inline virtual int32	GetINT()	const { return INT; }
	inline virtual int32	GetWIS()	const { return WIS; }
	inline virtual int32	GetCHA()	const { return CHA; }
	inline virtual int32	GetMR()	const { return MR; }
	inline virtual int32	GetFR()	const { return FR; }
	inline virtual int32	GetDR()	const { return DR; }
	inline virtual int32	GetPR()	const { return PR; }
	inline virtual int32	GetCR()	const { return CR; }
	inline virtual int32	GetCorrup()	const { return Corrup; }
	//Heroic
	inline virtual int32	GetHeroicSTR()	const { return itembonuses.HeroicSTR; }
	inline virtual int32	GetHeroicSTA()	const { return itembonuses.HeroicSTA; }
	inline virtual int32	GetHeroicDEX()	const { return itembonuses.HeroicDEX; }
	inline virtual int32	GetHeroicAGI()	const { return itembonuses.HeroicAGI; }
	inline virtual int32	GetHeroicINT()	const { return itembonuses.HeroicINT; }
	inline virtual int32	GetHeroicWIS()	const { return itembonuses.HeroicWIS; }
	inline virtual int32	GetHeroicCHA()	const { return itembonuses.HeroicCHA; }
	inline virtual int32	GetHeroicMR()	const { return itembonuses.HeroicMR; }
	inline virtual int32	GetHeroicFR()	const { return itembonuses.HeroicFR; }
	inline virtual int32	GetHeroicDR()	const { return itembonuses.HeroicDR; }
	inline virtual int32	GetHeroicPR()	const { return itembonuses.HeroicPR; }
	inline virtual int32	GetHeroicCR()	const { return itembonuses.HeroicCR; }
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

	// "SET" Class Methods
	void SetBotSpellID(uint32 newSpellID);
	virtual void SetSpawnStatus(bool spawnStatus) { _spawnStatus = spawnStatus; }
	void SetPetChooserID(uint8 id) { _petChooserID = id; }
	void SetBotArcher(bool a) { _botArcher = a; }
	void SetBotCharmer(bool c) { _botCharmer = c; }
	void SetPetChooser(bool p) { _petChooser = p; }
	void SetBotOwner(Mob* botOwner) { this->_botOwner = botOwner; }
	// void SetBotOwnerCharacterID(uint32 botOwnerCharacterID) { _botOwnerCharacterID = botOwnerCharacterID; }
	void SetRangerAutoWeaponSelect(bool enable) { GetClass() == RANGER ? _rangerAutoWeaponSelect = enable : _rangerAutoWeaponSelect = false; }
	void SetBotRole(BotRoleType botRole) { _botRole = botRole; }
	void SetBotStance(BotStanceType botStance) { _botStance = ((botStance != BotStanceUnknown) ? (botStance) : (BotStancePassive)); }
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

	std::string CreateSayLink(Client* botOwner, const char* message, const char* name);

	// Class Destructors
	virtual ~Bot();

	// Publicized protected functions
	virtual void BotRangedAttack(Mob* other);

	// Publicized private functions
	static NPCType FillNPCTypeStruct(uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender, float size, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, int32 hp, int32 mana, int32 mr, int32 cr, int32 dr, int32 fr, int32 pr, int32 corrup, int32 ac, uint32 str, uint32 sta, uint32 dex, uint32 agi, uint32 _int, uint32 wis, uint32 cha, uint32 attack);
	void BotRemoveEquipItem(int16 slot);
	void RemoveBotItemBySlot(uint32 slotID, std::string* errorMessage);
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
	
protected:
	virtual void PetAIProcess();
	virtual void BotMeditate(bool isSitting);
	virtual bool CheckBotDoubleAttack(bool Triple = false);
	virtual int32 GetBotFocusEffect(BotfocusType bottype, uint16 spell_id);
	virtual int32 CalcBotFocusEffect(BotfocusType bottype, uint16 focus_id, uint16 spell_id, bool best_focus=false);
	virtual int32 CalcBotAAFocus(BotfocusType type, uint32 aa_ID, uint32 points, uint16 spell_id);
	virtual void PerformTradeWithClient(int16 beginSlotID, int16 endSlotID, Client* client);
	virtual bool AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = 0);

	BotCastingRoles& GetCastingRoles() { return m_CastingRoles; }
	void SetGroupHealer(bool flag = true) { m_CastingRoles.GroupHealer = flag; }
	void SetGroupSlower(bool flag = true) { m_CastingRoles.GroupSlower = flag; }
	void SetGroupNuker(bool flag = true) { m_CastingRoles.GroupNuker = flag; }
	void SetGroupDoter(bool flag = true) { m_CastingRoles.GroupDoter = flag; }
	//void SetRaidHealer(bool flag = true) { m_CastingRoles.RaidHealer = flag; }
	//void SetRaidSlower(bool flag = true) { m_CastingRoles.RaidSlower = flag; }
	//void SetRaidNuker(bool flag = true) { m_CastingRoles.RaidNuker = flag; }
	//void SetRaidDoter(bool flag = true) { m_CastingRoles.RaidDoter = flag; }

private:
	// Class Members
	uint32 _botID;
	uint32 _botOwnerCharacterID;
	//uint32 _botSpellID;
	bool _spawnStatus;
	Mob* _botOwner;
	bool _botOrderAttack;
	bool _botArcher;
	bool _botCharmer;
	bool _petChooser;
	uint8 _petChooserID;
	bool berserk;
	EQEmu::InventoryProfile m_inv;
	double _lastTotalPlayTime;
	time_t _startTotalPlayTime;
	Mob* _previousTarget;
	uint32 _guildId;
	uint8 _guildRank;
	std::string _guildName;
	uint32 _lastZoneId;
	bool _rangerAutoWeaponSelect;
	BotRoleType _botRole;
	BotStanceType _botStance;
	BotStanceType _baseBotStance;
	unsigned int RestRegenHP;
	unsigned int RestRegenMana;
	unsigned int RestRegenEndurance;
	Timer rest_timer;
	int32	base_end;
	int32	cur_end;
	int32	max_end;
	int32	end_regen;
	uint32 timers[MaxTimer];
	
	Timer evade_timer; // can be moved to pTimers at some point

	BotCastingRoles m_CastingRoles;

	std::shared_ptr<HealRotation> m_member_of_heal_rotation;

	std::map<uint32, BotAA> botAAs;
	InspectMessage_Struct _botInspectMessage;
	bool _altoutofcombatbehavior;
	bool _showhelm;
	bool _pauseAI;

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

	// Private "Inventory" Methods
	void GetBotItems(EQEmu::InventoryProfile &inv, std::string* errorMessage);
	void BotAddEquipItem(int slot, uint32 id);
	uint32 GetBotItemBySlot(uint32 slotID);

	// Private "Pet" Methods
	bool LoadPet();	// Load and spawn bot pet if there is one
	bool SavePet();	// Save and depop bot pet if there is one
	bool DeletePet();
};

#endif // BOTS

#endif // BOT_H
