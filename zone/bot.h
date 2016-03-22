#ifndef BOT_H
#define BOT_H

#ifdef BOTS

#include "bot_structs.h"
#include "mob.h"
#include "client.h"
#include "pets.h"
#include "groups.h"
#include "corpse.h"
#include "zonedb.h"
#include "string_ids.h"
#include "../common/misc_functions.h"
#include "../common/global_define.h"
#include "guild_mgr.h"
#include "worldserver.h"

#include <sstream>

#define BOT_DEFAULT_FOLLOW_DISTANCE 184

extern WorldServer worldserver;

const int BotAISpellRange = 100; // TODO: Write a method that calcs what the bot's spell range is based on spell, equipment, AA, whatever and replace this
const int MaxSpellTimer = 15;
const int MaxDisciplineTimer = 10;
const int DisciplineReuseStart = MaxSpellTimer + 1;
const int MaxTimer = MaxSpellTimer + MaxDisciplineTimer;
const int MaxStances = 7;
const int MaxSpellTypes = 16;
const int MaxHealRotationMembers = 6;
const int MaxHealRotationTargets = 3;

enum BotStanceType {
	BotStancePassive,
	BotStanceBalanced,
	BotStanceEfficient,
	BotStanceReactive,
	BotStanceAggressive,
	BotStanceBurn,
	BotStanceBurnAE
};

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
	SpellType_CureIndex
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

	enum EqExpansions {
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
	virtual bool Death(Mob* killerMob, int32 damage, uint16 spell_id, SkillUseTypes attack_skill);
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, SkillUseTypes attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, int special = 0);
	virtual bool Attack(Mob* other, int Hand = MainPrimary, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false,
		ExtraAttackOptions *opts = nullptr, int special = 0);
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
	bool IsValidName();
	static bool IsBotNameAvailable(char *botName, std::string* errorMessage);
	bool DeleteBot(std::string* errorMessage);
	void Spawn(Client* botCharacterOwner, std::string* errorMessage);
	virtual void SetLevel(uint8 in_level, bool command = false);
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	virtual bool Process();
	void FinishTrade(Client* client, BotTradeType tradeType);
	virtual bool Save();
	virtual void Depop();
	void CalcBotStats(bool showtext = true);
	uint16 BotGetSpells(int spellslot) { return AIspells[spellslot].spellid; }
	uint16 BotGetSpellType(int spellslot) { return AIspells[spellslot].type; }
	uint16 BotGetSpellPriority(int spellslot) { return AIspells[spellslot].priority; }
	virtual float GetProcChances(float ProcBonus, uint16 hand);
	virtual int GetHandToHandDamage(void);
	virtual bool TryFinishingBlow(Mob *defender, SkillUseTypes skillinuse);
	virtual void DoRiposte(Mob* defender);
	inline virtual int32 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(SkillOffense)) * 9 / 10); }
	inline virtual int32 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	uint32 GetTotalATK();
	uint32 GetATKRating();
	uint16 GetPrimarySkillValue();
	uint16	MaxSkill(SkillUseTypes skillid, uint16 class_, uint16 level) const;
	inline	uint16	MaxSkill(SkillUseTypes skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }
	virtual void DoSpecialAttackDamage(Mob *who, SkillUseTypes skill, int32 max_damage, int32 min_damage = 1, int32 hate_override = -1, int ReuseTime = 10, bool HitChance=false);
	virtual void TryBackstab(Mob *other,int ReuseTime = 10);
	virtual void RogueBackstab(Mob* other, bool min_damage = false, int ReuseTime = 10);
	virtual void RogueAssassinate(Mob* other);
	virtual void DoClassAttacks(Mob *target, bool IsRiposte=false);
	virtual bool TryHeadShot(Mob* defender, SkillUseTypes skillInUse);
	virtual void DoMeleeSkillAttackDmg(Mob* other, uint16 weapon_damage, SkillUseTypes skillinuse, int16 chance_mod=0, int16 focus=0, bool CanRiposte=false, int ReuseTime =0);
	virtual void ApplySpecialAttackMod(SkillUseTypes skill, int32 &dmg, int32 &mindmg);
	bool CanDoSpecialAttack(Mob *other);
	virtual int32 CheckAggroAmount(uint16 spellid);
	virtual void CalcBonuses();
	void CalcItemBonuses(StatBonuses* newbon);
	void AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug = false, bool isTribute = false, int rec_override = 0);
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
	bool DoFinishedSpellAETarget(uint16 spell_id, Mob* spellTarget, uint16 slot, bool &stopLogic);
	bool DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, uint16 slot, bool &stopLogic);
	bool DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, uint16 slot, bool &stopLogic);
	void SendBotArcheryWearChange(uint8 material_slot, uint32 material, uint32 color);
	void Camp(bool databaseSave = true);
	virtual void AddToHateList(Mob* other, uint32 hate = 0, int32 damage = 0, bool iYellForHelp = true, bool bFrenzy = false, bool iBuffTic = false);
	virtual void SetTarget(Mob* mob);
	virtual void Zone();
	std::vector<AISpells_Struct> GetBotSpells() { return AIspells; }
	bool IsArcheryRange(Mob* target);
	void ChangeBotArcherWeapons(bool isArcher);
	void Sit();
	void Stand();
	bool IsSitting();
	bool IsStanding();
	bool IsBotCasterCombatRange(Mob *target);
	bool CalculateNewPosition2(float x, float y, float z, float speed, bool checkZ = true) ;
	bool UseDiscipline(uint32 spell_id, uint32 target);
	uint8 GetNumberNeedingHealedInGroup(uint8 hpr, bool includePets);
	bool GetNeedsCured(Mob *tar);
	bool HasOrMayGetAggro();
	void SetDefaultBotStance();
	void CalcChanceToCast();
	void CreateHealRotation( Mob* target, uint32 timer = 10000 );
	bool AddHealRotationMember( Bot* healer );
	bool RemoveHealRotationMember( Bot* healer );
	bool AddHealRotationTarget( Mob* target );
	//bool AddHealRotationTarget( const char *targetName, int index);
	bool AddHealRotationTarget( Mob* target, int index);
	bool RemoveHealRotationTarget( Mob* target );
	bool RemoveHealRotationTarget( int index);
	void NotifyNextHealRotationMember( bool notifyNow = false );
	void ClearHealRotationLeader() { _healRotationLeader = 0; }
	void ClearHealRotationMembers();
	void ClearHealRotationTargets();
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
	virtual bool AICastSpell(Mob* tar, uint8 iChance, uint16 iSpellTypes);
	virtual bool AI_EngagedCastCheck();
	virtual bool AI_PursueCastCheck();
	virtual bool AI_IdleCastCheck();
	bool AIHealRotation(Mob* tar, bool useFastHeals);

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
	virtual bool CastSpell(uint16 spell_id, uint16 target_id, uint16 slot = USE_ITEM_SPELL_SLOT, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0,
						   uint32 item_slot = 0xFFFFFFFF, int16 *resist_adjust = nullptr, uint32 aa_id = 0);
	virtual bool SpellOnTarget(uint16 spell_id, Mob* spelltar);
	virtual bool IsImmuneToSpell(uint16 spell_id, Mob *caster);
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction);
	virtual bool DoCastSpell(uint16 spell_id, uint16 target_id, uint16 slot = USE_ITEM_SPELL_SLOT, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0, uint32 item_slot = 0xFFFFFFFF, uint32 aa_id = 0);

	// Bot Action Command Methods
	bool MesmerizeTarget(Mob* target);
	bool Bot_Command_Resist(int resisttype, int level);
	bool Bot_Command_DireTarget(int diretype, Mob *target);
	bool Bot_Command_CharmTarget(int charmtype, Mob *target);
	bool Bot_Command_CalmTarget(Mob *target);
	bool Bot_Command_RezzTarget(Mob *target);
	bool Bot_Command_Cure(int curetype, int level);

	// Bot Equipment & Inventory Class Methods
	void BotTradeSwapItem(Client* client, int16 lootSlot, const ItemInst* inst, const ItemInst* inst_swap, uint32 equipableSlots, std::string* errorMessage, bool swap = true);
	void BotTradeAddItem(uint32 id, const ItemInst* inst, int16 charges, uint32 equipableSlots, uint16 lootSlot, std::string* errorMessage, bool addToDb = true);
	void EquipBot(std::string* errorMessage);
	bool CheckLoreConflict(const Item_Struct* item);
	uint32 GetEquipmentColor(uint8 material_slot) const;
	virtual void UpdateEquipmentLight() { m_Light.Type.Equipment = m_inv.FindBrightestLightType(); m_Light.Level.Equipment = m_Light.TypeToLevel(m_Light.Type.Equipment); }

	// Static Class Methods
	static void SaveBotGroup(Group* botGroup, std::string botGroupName, std::string* errorMessage);
	static void DeleteBotGroup(std::string botGroupName, std::string* errorMessage);
	static std::list<BotGroup> LoadBotGroup(std::string botGroupName, std::string* errorMessage);
	static uint32 CanLoadBotGroup(uint32 botOwnerCharacterId, std::string botGroupName, std::string* errorMessage);
	static uint32 GetBotGroupIdByBotGroupName(std::string botGroupName, std::string* errorMessage);
	static uint32 GetBotGroupLeaderIdByBotGroupName(std::string botGroupName);
	static std::list<BotGroupList> GetBotGroupListByBotOwnerCharacterId(uint32 botOwnerCharacterId, std::string* errorMessage);
	static bool DoesBotGroupNameExist(std::string botGroupName);
	//static void DestroyBotRaidObjects(Client* client);	// Can be removed after bot raids are dumped
	static uint32 GetBotIDByBotName(std::string botName);
	static Bot* LoadBot(uint32 botID, std::string* errorMessage);
	static std::list<BotsAvailableList> GetBotList(uint32 botOwnerCharacterID, std::string* errorMessage);
	static void ProcessBotCommands(Client *c, const Seperator *sep);
	static std::list<SpawnedBotsList> ListSpawnedBots(uint32 characterID, std::string* errorMessage);
	static uint32 SpawnedBotCount(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 CreatedBotCount(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 AllowedBotSpawns(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 GetBotOwnerCharacterID(uint32 botID, std::string* errorMessage);
	static void LevelBotWithClient(Client* client, uint8 level, bool sendlvlapp);
	//static bool SetBotOwnerCharacterID(uint32 botID, uint32 botOwnerCharacterID, std::string* errorMessage);
	static std::string ClassIdToString(uint16 classId);
	static std::string RaceIdToString(uint16 raceId);
	static bool IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined);
	static void BotGroupOrderFollow(Group* group, Client* client);
	static void BotGroupOrderGuard(Group* group, Client* client);
	static void BotGroupOrderAttack(Group* group, Mob* target, Client* client);
	static void BotGroupSummon(Group* group, Client* client);
	static Bot* GetBotByBotClientOwnerAndBotName(Client* c, std::string botName);
	static void ProcessBotGroupInvite(Client* c, std::string botName);
	static void ProcessBotGroupDisband(Client* c, std::string botName);
	static void BotOrderCampAll(Client* c);
	static void BotHealRotationsClear( Client* c );
	static void ProcessBotInspectionRequest(Bot* inspectedBot, Client* client);
	static std::list<uint32> GetGroupedBotsByGroupId(uint32 groupId, std::string* errorMessage);
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
	static std::list<BotSpell> GetBotSpellsBySpellType(Bot* botCaster, uint16 spellType);
	static BotSpell GetFirstBotSpellBySpellType(Bot* botCaster, uint16 spellType);
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
	static std::list<Bot*> GetBotsInHealRotation( Bot* leader );

	// Static Bot Group Methods
	static bool AddBotToGroup(Bot* bot, Group* group);
	static bool RemoveBotFromGroup(Bot* bot, Group* group);
	static bool BotGroupCreate(std::string botGroupLeaderName);
	static bool BotGroupCreate(Bot* botGroupLeader);
	static bool	GroupHasClass(Group* group, uint8 classId);
	static bool GroupHasClericClass(Group* group) { return GroupHasClass(group, CLERIC); }
	static bool GroupHasDruidClass(Group* group) { return GroupHasClass(group, DRUID); }
	static bool GroupHasShamanClass(Group* group) { return GroupHasClass(group, SHAMAN); }
	static bool GroupHasEnchanterClass(Group* group) { return GroupHasClass(group, ENCHANTER); }
	static bool GroupHasPriestClass(Group* group) { return GroupHasClass(group, CLERIC | DRUID | SHAMAN); }
	static void BotGroupSay(Mob *speaker, const char *msg, ...);

	// "GET" Class Methods
	uint32 GetBotID() const { return _botID; }
	uint32 GetBotOwnerCharacterID() { return _botOwnerCharacterID; }
	uint32 GetBotSpellID() { return npc_spells_id; }
	Mob* GetBotOwner() { return this->_botOwner; }
	uint32 GetBotArcheryRange();
	ItemInst* GetBotItem(uint32 slotID);
	virtual bool GetSpawnStatus() { return _spawnStatus; }
	uint8 GetPetChooserID() { return _petChooserID; }
	bool IsPetChooser() { return _petChooser; }
	bool IsBotArcher() { return _botArcher; }
	bool IsBotCharmer() { return _botCharmer; }
	virtual bool IsBot() const { return true; }
	bool GetRangerAutoWeaponSelect() { return _rangerAutoWeaponSelect; }
	BotRoleType GetBotRole() { return _botRole; }
	BotStanceType GetBotStance() { return _botStance; }
	uint8 GetChanceToCastBySpellType(uint16 spellType);
	bool IsGroupPrimaryHealer();
	bool IsGroupPrimarySlower();
	bool IsBotCaster() { return (GetClass() == CLERIC || GetClass() == DRUID || GetClass() == SHAMAN || GetClass() == NECROMANCER || GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == ENCHANTER); }
	bool IsBotINTCaster() { return (GetClass() == NECROMANCER || GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == ENCHANTER); }
	bool IsBotWISCaster() { return (GetClass() == CLERIC || GetClass() == DRUID || GetClass() == SHAMAN); }
	bool CanHeal();
	int GetRawACNoShield(int &shield_ac);
	bool GetHasBeenSummoned() { return _hasBeenSummoned; }
	const glm::vec3 GetPreSummonLocation() const { return m_PreSummonLocation; }
	bool GetInHealRotation() { return _isInHealRotation; }
	bool GetHealRotationActive() { return (GetInHealRotation() && _isHealRotationActive); }
	bool GetHealRotationUseFastHeals() { return _healRotationUseFastHeals; }
	bool GetHasHealedThisCycle() { return _hasHealedThisCycle; }
	Mob* GetHealRotationTarget();
	Mob* GetHealRotationTarget(uint8 index);
	Bot* GetHealRotationLeader();
	Bot* GetNextHealRotationMember();
	Bot* GetPrevHealRotationMember();
	uint8 GetNumHealRotationMembers () { return _numHealRotationMembers; }
	uint32 GetHealRotationNextHealTime() { return _healRotationNextHeal; }
	uint32 GetHealRotationTimer () { return _healRotationTimer; }
	bool GetBardUseOutOfCombatSongs() { return _bardUseOutOfCombatSongs;}
	bool GetShowHelm() { return _showhelm; }
	inline virtual int32	GetAC()	const { return AC; }
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
	void SetBotStance(BotStanceType botStance) { _botStance = botStance; }
	void SetSpellRecastTimer(int timer_index, int32 recast_delay);
	void SetDisciplineRecastTimer(int timer_index, int32 recast_delay);
	void SetHasBeenSummoned(bool s);
	void SetPreSummonLocation(const glm::vec3& location) { m_PreSummonLocation = location; }
	void SetInHealRotation( bool inRotation ) { _isInHealRotation = inRotation; }
	void SetHealRotationActive( bool isActive ) { _isHealRotationActive = isActive; }
	void SetHealRotationUseFastHeals( bool useFastHeals ) { _healRotationUseFastHeals = useFastHeals; }
	void SetHasHealedThisCycle( bool hasHealed ) { _hasHealedThisCycle = hasHealed; }
	void SetHealRotationLeader( Bot* leader );
	void SetNextHealRotationMember( Bot* healer );
	void SetPrevHealRotationMember( Bot* healer );
	void SetHealRotationNextHealTime( uint32 nextHealTime ) { _healRotationNextHeal = nextHealTime; }
	void SetHealRotationTimer( uint32 timer ) { _healRotationTimer = timer; }
	void SetNumHealRotationMembers( uint8 numMembers ) { _numHealRotationMembers = numMembers; }
	void SetBardUseOutOfCombatSongs(bool useOutOfCombatSongs) { _bardUseOutOfCombatSongs = useOutOfCombatSongs;}
	void SetShowHelm(bool showhelm) { _showhelm = showhelm; }

	std::string CreateSayLink(Client* botOwner, const char* message, const char* name);

	// Class Destructors
	virtual ~Bot();

protected:
	virtual void PetAIProcess();
	static NPCType FillNPCTypeStruct(uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender, float size, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, int32 hp, int32 mana, int32 mr, int32 cr, int32 dr, int32 fr, int32 pr, int32 corrup, int32 ac, uint32 str, uint32 sta, uint32 dex, uint32 agi, uint32 _int, uint32 wis, uint32 cha, uint32 attack);
	virtual void BotMeditate(bool isSitting);
	virtual void BotRangedAttack(Mob* other);
	virtual bool CheckBotDoubleAttack(bool Triple = false);
	virtual int32 GetBotFocusEffect(BotfocusType bottype, uint16 spell_id);
	virtual int32 CalcBotFocusEffect(BotfocusType bottype, uint16 focus_id, uint16 spell_id, bool best_focus=false);
	virtual int32 CalcBotAAFocus(BotfocusType type, uint32 aa_ID, uint32 points, uint16 spell_id);
	virtual void PerformTradeWithClient(int16 beginSlotID, int16 endSlotID, Client* client);
	virtual bool AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = 0);
	virtual float GetMaxMeleeRangeToTarget(Mob* target);

	static void SetBotGuildMembership(uint32 botId, uint32 guildid, uint8 rank);

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
	Inventory m_inv;
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
	bool _hasBeenSummoned;
	glm::vec3 m_PreSummonLocation;
	uint8 _spellCastingChances[MaxStances][MaxSpellTypes];
	bool _isInHealRotation;
	bool _isHealRotationActive;
	bool _healRotationUseFastHeals;
	bool _hasHealedThisCycle;
	uint32 _healRotationTimer;
	uint32 _healRotationNextHeal;
	//char _healRotationTargets[MaxHealRotationTargets][64];
	uint16 _healRotationTargets[MaxHealRotationTargets];
	uint32 _healRotationLeader;
	uint32 _healRotationMemberNext;
	uint32 _healRotationMemberPrev;
	uint8 _numHealRotationMembers;
	std::map<uint32, BotAA> botAAs;
	InspectMessage_Struct _botInspectMessage;
	bool _bardUseOutOfCombatSongs;
	bool _showhelm;

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
	void GetBotItems(std::string* errorMessage, Inventory &inv);
	void BotRemoveEquipItem(int slot);
	void BotAddEquipItem(int slot, uint32 id);
	uint32 GetBotItemBySlot(uint32 slotID);
	void RemoveBotItemBySlot(uint32 slotID, std::string* errorMessage);
	void SetBotItemInSlot(uint32 slotID, uint32 itemID, const ItemInst* inst, std::string* errorMessage);
	uint32 GetBotItemsCount(std::string* errorMessage);
	uint32 GetTotalPlayTime();
	void SaveBuffs();	// Saves existing buffs to the database to persist zoning and camping
	void LoadBuffs();	// Retrieves saved buffs from the database on spawning
	void LoadPetBuffs(SpellBuff_Struct* petBuffs, uint32 botPetSaveId);
	void SavePetBuffs(SpellBuff_Struct* petBuffs, uint32 botPetSaveId);
	void LoadPetItems(uint32* petItems, uint32 botPetSaveId);
	void SavePetItems(uint32* petItems, uint32 botPetSaveId);
	void LoadPetStats(std::string* petName, uint32* petMana, uint32* petHitPoints, uint32* botPetId, uint32 botPetSaveId);
	uint32 SavePetStats(std::string petName, uint32 petMana, uint32 petHitPoints, uint32 botPetId);
	void LoadPet();	// Load and spawn bot pet if there is one
	void SavePet();	// Save and depop bot pet if there is one
	uint32 GetPetSaveId();
	void DeletePetBuffs(uint32 botPetSaveId);
	void DeletePetItems(uint32 botPetSaveId);
	void DeletePetStats(uint32 botPetSaveId);
	void LoadGuildMembership(uint32* guildId, uint8* guildRank, std::string* guildName);
	void LoadStance();
	void SaveStance();
	void LoadTimers();
	void SaveTimers();
};

#endif // BOTS

#endif // BOT_H
