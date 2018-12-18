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
#ifndef NPC_H
#define NPC_H

#include "../common/rulesys.h"

#include "mob.h"
#include "qglobals.h"
#include "zonedb.h"
#include "zonedump.h"

#include <deque>
#include <list>


#ifdef _WINDOWS
	#define M_PI	3.141592
#endif

#define LEAVECOMBAT 0
#define ENTERCOMBAT 1
#define	ONDEATH		2
#define	AFTERDEATH	3
#define HAILED		4
#define	KILLEDPC	5
#define	KILLEDNPC	6
#define	ONSPAWN		7
#define	ONDESPAWN	8

typedef struct {
	float min_x;
	float max_x;
	float min_y;
	float max_y;
	float min_z;
	float max_z;
	bool say;
} NPCProximity;

struct AISpells_Struct {
	uint32	type;			// 0 = never, must be one (and only one) of the defined values
	uint16	spellid;		// <= 0 = no spell
	int16	manacost;		// -1 = use spdat, -2 = no cast time
	uint32	time_cancast;	// when we can cast this spell next
	int32	recast_delay;
	int16	priority;
	int16	resist_adjust;
	int8	min_hp; // >0 won't cast if HP is below
	int8	max_hp; // >0 won't cast if HP is above
};

struct AISpellsEffects_Struct {
	uint16	spelleffectid;
	int32	base;
	int32	limit;
	int32	max;
};

struct AISpellsVar_Struct {
	uint32  fail_recast;
	uint32	engaged_no_sp_recast_min;
	uint32	engaged_no_sp_recast_max;
	uint8	engaged_beneficial_self_chance;
	uint8	engaged_beneficial_other_chance;
	uint8	engaged_detrimental_chance;
	uint32  pursue_no_sp_recast_min;
	uint32  pursue_no_sp_recast_max;
	uint8   pursue_detrimental_chance;
	uint32  idle_no_sp_recast_min;
	uint32  idle_no_sp_recast_max;
	uint8	idle_beneficial_chance;
};

class SwarmPet;
class Client;
class Group;
class Raid;
class Spawn2;
class Aura;

namespace EQEmu
{
	struct ItemData;
}

class NPC : public Mob
{
public:
	static NPC* SpawnNPC(const char* spawncommand, const glm::vec4& position, Client* client = nullptr);
	static bool	SpawnZoneController();
	static int8 GetAILevel(bool iForceReRead = false);

	NPC(const NPCType* npc_type_data, Spawn2* respawn, const glm::vec4& position, int iflymode, bool IsCorpse = false);

	virtual ~NPC();

	static NPC *SpawnGridNodeNPC(std::string name, const glm::vec4 &position, uint32 grid_id, uint32 grid_number, uint32 pause);

	//abstract virtual function implementations requird by base abstract class
	virtual bool Death(Mob* killerMob, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill);
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None);
	virtual bool Attack(Mob* other, int Hand = EQEmu::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false,
		bool IsFromSpell = false, ExtraAttackOptions *opts = nullptr);
	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return false; }
	virtual Raid* GetRaid() { return 0; }
	virtual Group* GetGroup() { return 0; }

	virtual bool IsNPC() const { return true; }

	virtual bool Process();
	virtual void	AI_Init();
	virtual void	AI_Start(uint32 iMoveDelay = 0);
	virtual void	AI_Stop();
	void			AI_DoMovement();
	void			AI_SetupNextWaypoint();
	bool			AI_AddNPCSpells(uint32 iDBSpellsID);
	bool			AI_AddNPCSpellsEffects(uint32 iDBSpellsEffectsID);
	virtual bool	AI_EngagedCastCheck();
	bool			AI_HasSpells() { return HasAISpell; }
	bool			AI_HasSpellsEffects() { return HasAISpellEffects; }
	void			ApplyAISpellEffects(StatBonuses* newbon);

	virtual bool	AI_PursueCastCheck();
	virtual bool	AI_IdleCastCheck();
	virtual void	AI_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot);

	void LevelScale();

	virtual void SetTarget(Mob* mob);
	virtual uint16 GetSkill(EQEmu::skills::SkillType skill_num) const { if (skill_num <= EQEmu::skills::HIGHEST_SKILL) { return skills[skill_num]; } return 0; }

	void CalcItemBonuses(StatBonuses *newbon);
	virtual void CalcBonuses();
	virtual int GetCurrentBuffSlots() const { return RuleI(Spells, MaxBuffSlotsNPC); }
	virtual int GetCurrentSongSlots() const { return RuleI(Spells, MaxSongSlotsNPC); }
	virtual int GetCurrentDiscSlots() const { return RuleI(Spells, MaxDiscSlotsNPC); }
	virtual int GetMaxBuffSlots() const { return RuleI(Spells, MaxBuffSlotsNPC); }
	virtual int GetMaxSongSlots() const { return RuleI(Spells, MaxSongSlotsNPC); }
	virtual int GetMaxDiscSlots() const { return RuleI(Spells, MaxDiscSlotsNPC); }
	virtual int GetMaxTotalSlots() const { return RuleI(Spells, MaxTotalSlotsNPC); }
	virtual int GetPetMaxTotalSlots() const { return RuleI(Spells, MaxTotalSlotsPET); }
	virtual void InitializeBuffSlots();
	virtual void UninitializeBuffSlots();

	virtual void	SetAttackTimer();
	virtual void	RangedAttack(Mob* other);
	virtual void	ThrowingAttack(Mob* other) { }
	int32 GetNumberOfAttacks() const { return attack_count; }
	void DoRangedAttackDmg(Mob* other, bool Launch = true, int16 damage_mod = 0, int16 chance_mod = 0, EQEmu::skills::SkillType skill = EQEmu::skills::SkillArchery, float speed = 4.0f, const char *IDFile = nullptr);

	bool	DatabaseCastAccepted(int spell_id);
	bool	IsFactionListAlly(uint32 other_faction);
	FACTION_VALUE CheckNPCFactionAlly(int32 other_faction);
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther);

	void	GoToBind(uint8 bindnum = 0)	{ GMMove(m_SpawnPoint.x, m_SpawnPoint.y, m_SpawnPoint.z, m_SpawnPoint.w); }
	void	Gate(uint8 bindnum = 0);

	void	GetPetState(SpellBuff_Struct *buffs, uint32 *items, char *name);
	void	SetPetState(SpellBuff_Struct *buffs, uint32 *items);
	void	InteractiveChat(uint8 chan_num, uint8 language, const char * message, const char* targetname,Mob* sender);
	void	TakenAction(uint8 action,Mob* actiontaker);
	virtual void SpellProcess();
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);

	void	AddItem(const EQEmu::ItemData* item, uint16 charges, bool equipitem = true);
	void	AddItem(uint32 itemid, uint16 charges, bool equipitem = true, uint32 aug1 = 0, uint32 aug2 = 0, uint32 aug3 = 0, uint32 aug4 = 0, uint32 aug5 = 0, uint32 aug6 = 0);
	void	AddLootTable();
	void	AddLootTable(uint32 ldid);
	void	CheckGlobalLootTables();
	void	DescribeAggro(Client *towho, Mob *mob, bool verbose);
	void	RemoveItem(uint32 item_id, uint16 quantity = 0, uint16 slot = 0);
	void	CheckMinMaxLevel(Mob *them);
	void	ClearItemList();
	ServerLootItem_Struct*	GetItem(int slot_id);
	void	AddCash(uint16 in_copper, uint16 in_silver, uint16 in_gold, uint16 in_platinum);
	void	AddCash();
	void	RemoveCash();
	void	QueryLoot(Client* to);
	uint32	CountLoot();
	inline uint32	GetLoottableID()	const { return loottable_id; }
	virtual void UpdateEquipmentLight();
	inline bool DropsGlobalLoot() const { return !skip_global_loot; }

	inline uint32	GetCopper()		const { return copper; }
	inline uint32	GetSilver()		const { return silver; }
	inline uint32	GetGold()		const { return gold; }
	inline uint32	GetPlatinum()	const { return platinum; }

	inline void	SetCopper(uint32 amt)		{ copper = amt; }
	inline void	SetSilver(uint32 amt)		{ silver = amt; }
	inline void	SetGold(uint32 amt)			{ gold = amt; }
	inline void	SetPlatinum(uint32 amt)		{ platinum = amt; }


	virtual int32 CalcMaxMana();
	void SetGrid(int32 grid_){ grid=grid_; }
	void SetSp2(uint32 sg2){ spawn_group=sg2; }
	void SetWaypointMax(uint16 wp_){ wp_m=wp_; }
	void SetSaveWaypoint(uint16 wp_){ save_wp=wp_; }

	uint16 GetWaypointMax() const { return wp_m; }
	int32 GetGrid() const { return grid; }
	uint32 GetSp2() const { return spawn_group; }
	uint32 GetSpawnPointID() const;

	glm::vec4 const GetSpawnPoint() const { return m_SpawnPoint; }
	glm::vec4 const GetGuardPoint() const { return m_GuardPoint; }
	EmuAppearance GetGuardPointAnim() const { return guard_anim; }
	void SaveGuardPointAnim(EmuAppearance anim) { guard_anim = anim; }

	void SetFlyMode(uint8 FlyMode){ flymode=FlyMode; }
	uint32 GetFlyMode() const { return flymode; }

	uint8 GetPrimSkill()	const { return prim_melee_type; }
	uint8 GetSecSkill()	const { return sec_melee_type; }
	uint8 GetRangedSkill() const { return ranged_type; }
	void SetPrimSkill(uint8 skill_type)	{ prim_melee_type = skill_type; }
	void SetSecSkill(uint8 skill_type)	{ sec_melee_type = skill_type; }
	void SetRangedSkill(uint8 skill_type)	{ ranged_type = skill_type; }

	uint32	MerchantType;
	bool	merchant_open;
	inline void	MerchantOpenShop() { merchant_open = true; }
	inline void	MerchantCloseShop() { merchant_open = false; }
	inline bool	IsMerchantOpen() { return merchant_open; }
	void	Depop(bool StartSpawnTimer = false);
	void	Stun(int duration);
	void	UnStun();
	uint32	GetSwarmOwner();
	uint32	GetSwarmTarget();
	void	SetSwarmTarget(int target_id = 0);
	void	DepopSwarmPets();
	void	PetOnSpawn(NewSpawn_Struct* ns);

	void	SignalNPC(int _signal_id);

	inline int32 GetNPCFactionID() const
	{ return npc_faction_id; }

	inline int32 GetPrimaryFaction() const
	{ return primary_faction; }

	int32 GetNPCHate(Mob *in_ent)
	{ return hate_list.GetEntHateAmount(in_ent); }

	bool IsOnHatelist(Mob *p)
	{ return hate_list.IsEntOnHateList(p); }

	void SetNPCFactionID(int32 in)
	{
		npc_faction_id = in;
		database.GetFactionIdsForNPC(npc_faction_id, &faction_list, &primary_faction);
	}

    glm::vec4 m_SpawnPoint;

	uint32	GetMaxDMG() const {return max_dmg;}
	uint32	GetMinDMG() const {return min_dmg;}
	int GetBaseDamage() const { return base_damage; }
	int GetMinDamage() const { return min_damage; }
	float GetSlowMitigation() const { return slow_mitigation; }
	float	GetAttackSpeed() const {return attack_speed;}
	int		GetAttackDelay() const {return attack_delay;}
	bool	IsAnimal() const { return(bodytype == BT_Animal); }
	uint16	GetPetSpellID() const {return pet_spell_id;}
	void	SetPetSpellID(uint16 amt) {pet_spell_id = amt;}
	uint32	GetMaxDamage(uint8 tlevel);
	void	SetTaunting(bool tog) {taunting = tog;}
	bool	IsTaunting() const { return taunting; }
	void	PickPocket(Client* thief);
	void	Disarm(Client* client, int chance);
	void	StartSwarmTimer(uint32 duration) { swarm_timer.Start(duration); }
	void	AddLootDrop(const EQEmu::ItemData*dbitem, ItemList* itemlistconst, int16 charges, uint8 minlevel, uint8 maxlevel, bool equipit, bool wearchange = false, uint32 aug1 = 0, uint32 aug2 = 0, uint32 aug3 = 0, uint32 aug4 = 0, uint32 aug5 = 0, uint32 aug6 = 0);
	virtual void DoClassAttacks(Mob *target);
	void	CheckSignal();
	inline bool IsNotTargetableWithHotkey() const { return no_target_hotkey; }
	int32 GetNPCHPRegen() const { return hp_regen + itembonuses.HPRegen + spellbonuses.HPRegen; }
	inline const char* GetAmmoIDfile() const { return ammo_idfile; }

	void ModifyStatsOnCharm(bool bRemoved);

	//waypoint crap
	int					GetMaxWp() const { return max_wp; }
	void				DisplayWaypointInfo(Client *to);
	void				CalculateNewWaypoint();
	void				AssignWaypoints(int32 grid);
	void				SetWaypointPause();
	void				UpdateWaypoint(int wp_index);

	// quest wandering commands
	void				StopWandering();
	void				ResumeWandering();
	void				PauseWandering(int pausetime);
	void				MoveTo(const glm::vec4& position, bool saveguardspot);
	void				GetClosestWaypoint(std::list<wplist> &wp_list, int count, const glm::vec3& location);

	uint32				GetEquipment(uint8 material_slot) const;	// returns item id
	int32				GetEquipmentMaterial(uint8 material_slot) const;

	void				NextGuardPosition();
	void				SaveGuardSpot(bool iClearGuardSpot = false);
	inline bool			IsGuarding() const { return(m_GuardPoint.w != 0); }
	void				SaveGuardSpotCharm();

	uint16 GetMeleeTexture1() const;
	uint16 GetMeleeTexture2() const;

	void RestoreGuardSpotCharm();

	void AI_SetRoambox(
		float max_distance,
		float roam_distance_variance,
		uint32 delay = 2500,
		uint32 min_delay = 2500
	);

	void				AI_SetRoambox(float distance, float max_x, float min_x, float max_y, float min_y, uint32 delay = 2500, uint32 min_delay = 2500);

	//mercenary stuff
	void	LoadMercTypes();
	void	LoadMercs();
	std::list<MercType> GetMercTypesList() {return mercTypeList; };
	std::list<MercType> GetMercTypesList( uint32 expansion );
	std::list<MercData> GetMercsList() {return mercDataList; };
	std::list<MercData> GetMercsList( uint32 expansion );
	int		GetNumMercTypes() { return static_cast<int>(mercTypeList.size()); };
	int		GetNumMercTypes( uint32 expansion );
	int		GetNumMercs() { return static_cast<int>(mercDataList.size()); };
	int		GetNumMercs( uint32 expansion );

	inline bool WillAggroNPCs() const { return(npc_aggro); }

	inline void GiveNPCTypeData(NPCType *ours) { NPCTypedata_ours = ours; }
	inline const uint32 GetNPCSpellsID()	const { return npc_spells_id; }
	inline const uint32 GetNPCSpellsEffectsID()	const { return npc_spells_effects_id; }

	float GetProximityMinX();
	float GetProximityMaxX();
	float GetProximityMinY();
	float GetProximityMaxY();
	float GetProximityMinZ();
	float GetProximityMaxZ();

	ItemList	itemlist; //kathgar - why is this public? Doing other things or I would check the code

	NPCProximity* proximity;
	Spawn2*	respawn2;
	QGlobalCache *GetQGlobals() { return qGlobals; }
	QGlobalCache *CreateQGlobals() { qGlobals = new QGlobalCache(); return qGlobals; }

	SwarmPet *GetSwarmInfo() { return (swarmInfoPtr); }
	void SetSwarmInfo(SwarmPet *mSwarmInfo) { swarmInfoPtr = mSwarmInfo; }

	int32	GetAccuracyRating() const { return (accuracy_rating); }
	void	SetAccuracyRating(int32 d) { accuracy_rating = d;}
	int32	GetAvoidanceRating() const { return (avoidance_rating); }
	void	SetAvoidanceRating(int32 d) { avoidance_rating = d;}
	int32 GetRawAC() const { return AC; }

	void	ModifyNPCStat(const char *identifier, const char *new_value);
	virtual void SetLevel(uint8 in_level, bool command = false);

	bool IsLDoNTrapped() const { return (ldon_trapped); }
	void SetLDoNTrapped(bool n) { ldon_trapped = n; }

	uint8 GetLDoNTrapType() const { return (ldon_trap_type); }
	void SetLDoNTrapType(uint8 n) { ldon_trap_type = n; }

	uint16 GetLDoNTrapSpellID() const { return (ldon_spell_id); }
	void SetLDoNTrapSpellID(uint16 n) { ldon_spell_id = n; }

	bool IsLDoNLocked() const { return (ldon_locked); }
	void SetLDoNLocked(bool n) { ldon_locked = n; }

	uint16 GetLDoNLockedSkill() const { return (ldon_locked_skill); }
	void SetLDoNLockedSkill(uint16 n) { ldon_locked_skill = n; }

	bool IsLDoNTrapDetected() const { return (ldon_trap_detected); }
	void SetLDoNTrapDetected(bool n) { ldon_trap_detected = n; }

	const bool GetCombatEvent() const { return combat_event; }
	void SetCombatEvent(bool b) { combat_event = b; }

	/* Only allows players that killed corpse to loot */
	const bool HasPrivateCorpse() const { return NPCTypedata->private_corpse; }

	const bool IsUnderwaterOnly() const { return NPCTypedata->underwater; }
	const char* GetRawNPCTypeName() const { return NPCTypedata->name; }

	void ChangeLastName(const char* in_lastname);
	void ClearLastName();

	bool GetDepop() { return p_depop; }

	void NPCSlotTexture(uint8 slot, uint16 texture);	// Sets new material values for slots

	uint32 GetAdventureTemplate() const { return adventure_template_id; }
	void AddSpellToNPCList(int16 iPriority, int16 iSpellID, uint32 iType, int16 iManaCost, int32 iRecastDelay, int16 iResistAdjust, int8 min_hp, int8 max_hp);
	void AddSpellEffectToNPCList(uint16 iSpellEffectID, int32 base, int32 limit, int32 max);
	void RemoveSpellFromNPCList(int16 spell_id);
	Timer *GetRefaceTimer() const { return reface_timer; }
	const uint32 GetAltCurrencyType() const { return NPCTypedata->alt_currency_type; }

	NPC_Emote_Struct* GetNPCEmote(uint16 emoteid, uint8 event_);
	void DoNPCEmote(uint8 event_, uint16 emoteid);
	bool CanTalk();
	void DoQuestPause(Mob *other);

	inline void SetSpellScale(float amt)		{ spellscale = amt; }
	inline float GetSpellScale()				{ return spellscale; }

	inline void SetHealScale(float amt)		{ healscale = amt; }
	inline float GetHealScale()					{ return healscale; }

	inline void SetSpellFocusDMG(int32 NewSpellFocusDMG) {SpellFocusDMG = NewSpellFocusDMG;}
	inline int32 GetSpellFocusDMG() const { return SpellFocusDMG;}

	inline void SetSpellFocusHeal(int32 NewSpellFocusHeal) {SpellFocusHeal = NewSpellFocusHeal;}
	inline int32 GetSpellFocusHeal() const {return SpellFocusHeal;}

	uint32	GetSpawnKillCount();
	int	GetScore();
	void	mod_prespawn(Spawn2 *sp);
	int	mod_npc_damage(int damage, EQEmu::skills::SkillType skillinuse, int hand, const EQEmu::ItemData* weapon, Mob* other);
	void	mod_npc_killed_merit(Mob* c);
	void	mod_npc_killed(Mob* oos);
	void	AISpellsList(Client *c);
	uint16 GetInnateProcSpellID() const { return innate_proc_spell_id; }

	uint32	GetHeroForgeModel() const { return herosforgemodel; }
	void	SetHeroForgeModel(uint32 model) { herosforgemodel = model; }

	bool IsRaidTarget() const { return raid_target; };
	void ResetHPUpdateTimer() { send_hp_update_timer.Start(); }

	bool IgnoreDespawn() { return ignore_despawn; }

	float GetRoamboxMaxX() const;
	float GetRoamboxMaxY() const;
	float GetRoamboxMinX() const;
	float GetRoamboxMinY() const;
	float GetRoamboxDistance() const;
	float GetRoamboxDestinationX() const;
	float GetRoamboxDestinationY() const;
	float GetRoamboxDestinationZ() const;
	uint32 GetRoamboxDelay() const;
	uint32 GetRoamboxMinDelay() const;

	std::unique_ptr<Timer> AIautocastspell_timer;

protected:

	const NPCType*	NPCTypedata;
	NPCType*	NPCTypedata_ours;	//special case for npcs with uniquely created data.

	friend class EntityList;
	friend class Aura;
	std::list<struct NPCFaction*> faction_list;
	uint32	copper;
	uint32	silver;
	uint32	gold;
	uint32	platinum;
	int32	grid;
	uint32	spawn_group;
	uint16	wp_m;

	int32	npc_faction_id;
	int32	primary_faction;

	Timer	attacked_timer;		//running while we are being attacked (damaged)
	Timer	swarm_timer;
	Timer	classattack_timer;
	Timer	knightattack_timer;
	Timer	assist_timer;		//ask for help from nearby mobs
	Timer	qglobal_purge_timer;

	bool	combat_event;	//true if we are in combat, false otherwise
	Timer	send_hp_update_timer;
	Timer	enraged_timer;
	Timer *reface_timer;

	uint32	npc_spells_id;
	uint8	casting_spell_AIindex;
	
	uint32*	pDontCastBefore_casting_spell;
	std::vector<AISpells_Struct> AIspells;
	bool HasAISpell;
	virtual bool AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes, bool bInnates = false);
	virtual bool AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = 0);
	AISpellsVar_Struct AISpellVar;
	int16 GetFocusEffect(focusType type, uint16 spell_id);
	uint16 innate_proc_spell_id;

	uint32	npc_spells_effects_id;
	std::vector<AISpellsEffects_Struct> AIspellsEffects;
	bool HasAISpellEffects;

	uint32	max_dmg;
	uint32	min_dmg;
	int		base_damage;
	int		min_damage;
	int32	accuracy_rating;
	int32	avoidance_rating;
	int16	attack_count;
	uint32	npc_mana;
	float	spellscale;
	float	healscale;
	int32 SpellFocusDMG;
	int32 SpellFocusHeal;

	// stats to switch back to after charm wears off
	// could probably pick a better name, but these probably aren't taken so ...
	int default_ac;
	int default_min_dmg;
	int default_max_dmg;
	int default_attack_delay;
	int default_accuracy_rating;
	int default_avoidance_rating;
	int default_atk;

	// when charmed, switch to these
	int charm_ac;
	int charm_min_dmg;
	int charm_max_dmg;
	int charm_attack_delay;
	int charm_accuracy_rating;
	int charm_avoidance_rating;
	int charm_atk;

	//pet crap:
	uint16	pet_spell_id;
	bool	taunting;
	Timer	taunt_timer;		//for pet taunting

	bool npc_aggro;

	std::deque<int> signal_q;

	//waypoint crap:
	std::vector<wplist> Waypoints;
	void _ClearWaypints();
	int max_wp;
	int save_wp;
	glm::vec4 m_GuardPoint;
	glm::vec4 m_GuardPointSaved;
	EmuAppearance guard_anim;
	float roambox_max_x;
	float roambox_max_y;
	float roambox_min_x;
	float roambox_min_y;
	float roambox_distance;
	float roambox_destination_x;
	float roambox_destination_y;
	float roambox_destination_z;
	uint32 roambox_delay;
	uint32 roambox_min_delay;

	uint16	skills[EQEmu::skills::HIGHEST_SKILL + 1];

	uint32	equipment[EQEmu::invslot::EQUIPMENT_COUNT];	//this is an array of item IDs

	uint32	herosforgemodel;			//this is the Hero Forge Armor Model (i.e 63 or 84 or 203)
	uint16	d_melee_texture1;
	//this is an item Material value
	uint16	d_melee_texture2;			//this is an item Material value (offhand)
	const char*	ammo_idfile;			//this determines projectile graphic "IT###" (see item field 'idfile')
	uint8	prim_melee_type;			//Sets the Primary Weapon attack message and animation
	uint8	sec_melee_type;				//Sets the Secondary Weapon attack message and animation
	uint8   ranged_type;				//Sets the Ranged Weapon attack message and animation

	SwarmPet *swarmInfoPtr;

	bool ldon_trapped;
	uint8 ldon_trap_type;
	uint16 ldon_spell_id;
	bool ldon_locked;
	uint16 ldon_locked_skill;
	bool ldon_trap_detected;
	QGlobalCache *qGlobals;
	uint32 adventure_template_id;

	//mercenary stuff
	std::list<MercType> mercTypeList;
	std::list<MercData> mercDataList;

	bool raid_target;
	bool ignore_despawn; //NPCs with this set to 1 will ignore the despawn value in spawngroup



private:
	uint32	loottable_id;
	bool	skip_global_loot;
	bool	p_depop;
};

#endif

