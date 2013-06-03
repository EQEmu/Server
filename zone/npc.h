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

class NPC;
#include "zonedb.h"
#include "mob.h"
//#include "spawn.h"

#include <list>
#include <deque>

#include "spawn2.h"
#include "../common/loottable.h"
#include "zonedump.h"
#include "QGlobals.h"
#include "../common/rulesys.h"

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
	uint16	type;			// 0 = never, must be one (and only one) of the defined values
	uint16	spellid;		// <= 0 = no spell
	int16	manacost;		// -1 = use spdat, -2 = no cast time
	uint32	time_cancast;	// when we can cast this spell next
	int32	recast_delay;
	int16	priority;
	int16	resist_adjust;
};

class AA_SwarmPetInfo;

class NPC : public Mob
{
public:
	static NPC* SpawnNPC(const char* spawncommand, float in_x, float in_y, float in_z, float in_heading = 0, Client* client = 0);
	static int8 GetAILevel(bool iForceReRead = false);

	NPC(const NPCType* data, Spawn2* respawn, float x, float y, float z, float heading, int iflymode, bool IsCorpse = false);

	virtual ~NPC();

	//abstract virtual function implementations requird by base abstract class
	virtual void Death(Mob* killerMob, int32 damage, uint16 spell_id, SkillType attack_skill);
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false);
	virtual bool Attack(Mob* other, int Hand = 13, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false);
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
	bool			AI_AddNPCSpells(uint32 iDBSpellsID);
	virtual bool	AI_EngagedCastCheck();
	bool			AI_HasSpells() { return HasAISpell; }

	virtual bool	AI_PursueCastCheck();
	virtual bool	AI_IdleCastCheck();
	virtual void	AI_Event_SpellCastFinished(bool iCastSucceeded, uint8 slot);

	void LevelScale();
	void CalcNPCResists();
	void CalcNPCRegen();
	void CalcNPCDamage();


	int32 GetActSpellDamage(uint16 spell_id, int32 value);
	int32 GetActSpellHealing(uint16 spell_id, int32 value);
	inline void SetSpellFocusDMG(int32 NewSpellFocusDMG) {SpellFocusDMG = NewSpellFocusDMG;}
	inline void SetSpellFocusHeal(int32 NewSpellFocusHeal) {SpellFocusHeal = NewSpellFocusHeal;}
	int32 SpellFocusDMG;
	int32 SpellFocusHeal;

	virtual void SetTarget(Mob* mob);
	virtual uint16 GetSkill(SkillType skill_num) const { if (skill_num <= HIGHEST_SKILL) { return skills[skill_num]; } return 0; }

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

	virtual void	RangedAttack(Mob* other);
	virtual void	ThrowingAttack(Mob* other) { }
	int32 GetNumberOfAttacks() const { return attack_count; }

	bool	DatabaseCastAccepted(int spell_id);
	bool	IsFactionListAlly(uint32 other_faction);
	FACTION_VALUE CheckNPCFactionAlly(int32 other_faction);
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther);

	void	GoToBind(uint8 bindnum = 0)	{ GMMove(org_x, org_y, org_z, org_heading); }
	void	Gate();

	void	GetPetState(SpellBuff_Struct *buffs, uint32 *items, char *name);
	void	SetPetState(SpellBuff_Struct *buffs, uint32 *items);
	void	InteractiveChat(uint8 chan_num, uint8 language, const char * message, const char* targetname,Mob* sender);
	void	TakenAction(uint8 action,Mob* actiontaker);
	virtual void SpellProcess();
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);

	void	AddItem(const Item_Struct* item, uint16 charges, bool equipitem = true);
	void	AddItem(uint32 itemid, uint16 charges, bool equipitem = true);
	void	AddLootTable();
	void	AddLootTable(uint32 ldid);
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
	void	DumpLoot(uint32 npcdump_index, ZSDump_NPC_Loot* npclootdump, uint32* NPCLootindex);
	inline uint32	GetLoottableID()	const { return loottable_id; }

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

	float GetSpawnPointX()	const { return org_x; }
	float GetSpawnPointY()	const { return org_y; }
	float GetSpawnPointZ()	const { return org_z; }
	float GetSpawnPointH()	const { return org_heading; }
	float GetGuardPointX()	const { return guard_x; }
	float GetGuardPointY()	const { return guard_y; }
	float GetGuardPointZ()	const { return guard_z; }
	float GetGuardPointH()	const { return guard_heading; }
	EmuAppearance GetGuardPointAnim() const { return guard_anim; }
	void SaveGuardPointAnim(EmuAppearance anim) { guard_anim = anim; }

	void SetFlyMode(uint8 FlyMode){ flymode=FlyMode; }
	uint32 GetFlyMode() const { return flymode; }

	uint8 GetPrimSkill()	const { return prim_melee_type; }
	uint8 GetSecSkill()	const { return sec_melee_type; }
	void SetPrimSkill(uint8 skill_type)	{ prim_melee_type = skill_type; }
	void SetSecSkill(uint8 skill_type)	{ sec_melee_type = skill_type; }

	uint32	MerchantType;
	void	Depop(bool StartSpawnTimer = false);
	void	Stun(int duration);
	void	UnStun();
	uint32	GetSwarmOwner();
	uint32	GetSwarmTarget();
	void	SetSwarmTarget(int target_id = 0);

	void	SignalNPC(int _signal_id);

	inline int32	GetNPCFactionID()	const { return npc_faction_id; }
	inline int32			GetPrimaryFaction()	const { return primary_faction; }
	int32	GetNPCHate(Mob* in_ent) {return hate_list.GetEntHate(in_ent);}
	bool	IsOnHatelist(Mob*p) { return hate_list.IsOnHateList(p);}

	void	SetNPCFactionID(int32 in) { npc_faction_id = in; database.GetFactionIdsForNPC(npc_faction_id, &faction_list, &primary_faction); }

	float	org_x, org_y, org_z, org_heading;

	uint32	GetMaxDMG() const {return max_dmg;}
	uint32	GetMinDMG() const {return min_dmg;}
	float	GetSlowMitigation() const {return slow_mitigation;}
	float	GetAttackSpeed() const {return attack_speed;}
	bool	IsAnimal() const { return(bodytype == BT_Animal); }
	uint16	GetPetSpellID() const {return pet_spell_id;}
	void	SetPetSpellID(uint16 amt) {pet_spell_id = amt;}
	uint32	GetMaxDamage(uint8 tlevel);
	void	SetTaunting(bool tog) {taunting = tog;}
	void	PickPocket(Client* thief);
	void	StartSwarmTimer(uint32 duration) { swarm_timer.Start(duration); }
	void	AddLootDrop(const Item_Struct*dbitem, ItemList* itemlistconst, int16 charges, uint8 minlevel, uint8 maxlevel, bool equipit, bool wearchange = false);
	virtual void DoClassAttacks(Mob *target);
	void	CheckSignal();

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
	void				MoveTo(float mtx, float mty, float mtz, float mth, bool saveguardspot);
	void				GetClosestWaypoint(std::list<wplist> &wp_list, int count, float m_x, float m_y, float m_z);

	uint32				GetEquipment(uint8 material_slot) const;	// returns item id
	int32				GetEquipmentMaterial(uint8 material_slot) const;

	void				NextGuardPosition();
	void				SaveGuardSpot(bool iClearGuardSpot = false);
	inline bool			IsGuarding() const { return(guard_heading != 0); }
	void				SaveGuardSpotCharm();
	void				RestoreGuardSpotCharm();
	void				AI_SetRoambox(float iDist, float iRoamDist, uint32 iDelay = 2500);
	void				AI_SetRoambox(float iDist, float iMaxX, float iMinX, float iMaxY, float iMinY, uint32 iDelay = 2500);

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

	ItemList	itemlist; //kathgar - why is this public? Doing other things or I would check the code

	NPCProximity* proximity;
	Spawn2*	respawn2;
	QGlobalCache *GetQGlobals() { return qGlobals; }
	QGlobalCache *CreateQGlobals() { qGlobals = new QGlobalCache(); return qGlobals; }

	AA_SwarmPetInfo *GetSwarmInfo() { return (swarmInfoPtr); }
	void SetSwarmInfo(AA_SwarmPetInfo *mSwarmInfo) { swarmInfoPtr = mSwarmInfo; }

	int32	GetAccuracyRating() const { return (accuracy_rating); }
	void	SetAccuracyRating(int32 d) { accuracy_rating = d;}
	int32 GetRawAC() const { return AC; }

	void	ModifyNPCStat(const char *identifier, const char *newValue);
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

	//The corpse we make can only be looted by people who got credit for the kill
	const bool HasPrivateCorpse() const { return NPCTypedata->private_corpse; }
	const bool IsUnderwaterOnly() const { return NPCTypedata->underwater; }
	const char* GetRawNPCTypeName() const { return NPCTypedata->name; }

	bool GetDepop() { return p_depop; }

	void NPCSlotTexture(uint8 slot, uint16 texture);	// Sets new material values for slots

	uint32 GetAdventureTemplate() const { return adventure_template_id; }
	void AddSpellToNPCList(int16 iPriority, int16 iSpellID, uint16 iType, int16 iManaCost, int32 iRecastDelay, int16 iResistAdjust);
	void RemoveSpellFromNPCList(int16 spell_id);
	Timer *GetRefaceTimer() const { return reface_timer; }
	const uint32 GetAltCurrencyType() const { return NPCTypedata->alt_currency_type; }

	NPC_Emote_Struct* GetNPCEmote(uint16 emoteid, uint8 event_);
	void DoNPCEmote(uint8 event_, uint16 emoteid);
	bool CanTalk();

	inline void SetSpellScale(float amt)		{ spellscale = amt; }
	inline float GetSpellScale()				{ return spellscale; }

	inline void SetHealScale(float amt)		{ healscale = amt; }
	inline float GetHealScale()					{ return healscale; }

	void AddQuestItem(ItemInst* inst) { questItems.Insert(inst); }

	void ClearQuestLists()
	{
		ClearQuestItems(true);
		ClearQuestDeleteItems(true);
	}

	void ResetQuestDeleteList()
	{
		ClearQuestDeleteItems(true);
	}


	void ClearQuestItems(bool delete_=false)
	{
		LinkedListIterator<ItemInst*> iterator(questItems);
		iterator.Reset();
		while(iterator.MoreElements())
		{
			iterator.RemoveCurrent(delete_);
		}

		questItems.Clear();
	}

	void ClearQuestDeleteItems(bool delete_=false)
	{
		LinkedListIterator<ItemInst*> iterator(questDeletionItems);
		iterator.Reset();
		while(iterator.MoreElements())
		{
			iterator.RemoveCurrent(delete_);
		}

		questDeletionItems.Clear();
	}

	ItemInst* FindQuestItemByID(uint32 itmID, int charges, bool flagItemForDeletion=false)
	{
		LinkedListIterator<ItemInst*> iterator(questItems);
		iterator.Reset();
		int totalCharges = 0;
		while(iterator.MoreElements())
		{
			if ( iterator.GetData()->GetItem()->ID == itmID )
			{
				totalCharges += 1;

				if ( flagItemForDeletion )
					questDeletionItems.Insert(iterator.GetData()->Clone());
				if ( charges > totalCharges )
				{
					iterator.Advance();
					continue;
				}

				return iterator.GetData();
			}
			iterator.Advance();
		}
		return nullptr;
	}

	bool DoesQuestItemExist(uint32 itmID, int charges, bool flagItemForDeletion=false) {
		ItemInst* inst = FindQuestItemByID(itmID,charges,flagItemForDeletion);
		if ( inst != nullptr )
		{
			return true;
		}
		else
			return false;
	}

	void ClearQuestItem(ItemInst* inst, bool delete_=true)
	{
		LinkedListIterator<ItemInst*> iterator(questItems);
		iterator.Reset();

		while(iterator.MoreElements())
		{
			if ( iterator.GetData ()->GetItem()->ID == inst->GetItem()->ID )
			{
				iterator.RemoveCurrent(delete_);
				break;
			}
			iterator.Advance();
		}
	}

	void RemoveQuestDeleteItems()
	{
		LinkedListIterator<ItemInst*> iterator(questDeletionItems);
		iterator.Reset();
		while(iterator.MoreElements())
		{
			ClearQuestItem(iterator.GetData(),true);
			iterator.RemoveCurrent(true);
		}

		questDeletionItems.Clear();
	}

	void PrintOutQuestItems(Client* c);

    uint32 	GetSpawnKillCount();
    int 	GetScore();
    void 	mod_prespawn(Spawn2 *sp);
	int 	mod_npc_damage(int damage, SkillType skillinuse, int hand, const Item_Struct* weapon, Mob* other);
	void	mod_npc_killed_merit(Mob* c);
	void	mod_npc_killed(Mob* oos);

protected:

	const NPCType*	NPCTypedata;
	NPCType*	NPCTypedata_ours;	//special case for npcs with uniquely created data.

	friend class EntityList;
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
	Timer	sendhpupdate_timer;
	Timer	enraged_timer;
	Timer *reface_timer;

	uint32	npc_spells_id;
	uint8	casting_spell_AIindex;
	Timer*	AIautocastspell_timer;
	uint32*	pDontCastBefore_casting_spell;
	std::vector<AISpells_Struct> AIspells;
	bool HasAISpell;
	virtual bool AICastSpell(Mob* tar, uint8 iChance, uint16 iSpellTypes);
	virtual bool AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = 0);


	uint32	max_dmg;
	uint32	min_dmg;
	int32	accuracy_rating;
	int16	attack_count;
	uint32	npc_mana;
	float	spellscale;
	float	healscale;

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
	float guard_x, guard_y, guard_z, guard_heading;
	float guard_x_saved, guard_y_saved, guard_z_saved, guard_heading_saved;
	EmuAppearance guard_anim;
	float roambox_max_x;
	float roambox_max_y;
	float roambox_min_x;
	float roambox_min_y;
	float roambox_distance;
	float roambox_movingto_x;
	float roambox_movingto_y;
	uint32 roambox_delay;

	uint16	skills[HIGHEST_SKILL+1];
	uint32	equipment[MAX_WORN_INVENTORY];	//this is an array of item IDs
	uint16	d_meele_texture1;			//this is an item Material value
	uint16	d_meele_texture2;			//this is an item Material value (offhand)
	uint8	prim_melee_type;			//Sets the Primary Weapon attack message and animation
	uint8	sec_melee_type;				//Sets the Secondary Weapon attack message and animation
	AA_SwarmPetInfo *swarmInfoPtr;

	bool ldon_trapped;
	uint8 ldon_trap_type;
	uint16 ldon_spell_id;
	bool ldon_locked;
	uint16 ldon_locked_skill;
	bool ldon_trap_detected;
	QGlobalCache *qGlobals;
	uint32 adventure_template_id;

	LinkedList<ItemInst*> questItems;
	LinkedList<ItemInst*> questDeletionItems;

	//mercenary stuff
	std::list<MercType> mercTypeList;
	std::list<MercData> mercDataList;

private:
	uint32	loottable_id;
	bool	p_depop;
};

#endif

