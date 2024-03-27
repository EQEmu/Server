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

#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/rulesys.h"
#include "../common/seperator.h"
#include "../common/spdat.h"
#include "../common/strings.h"
#include "../common/emu_versions.h"
#include "../common/features.h"
#include "../common/item_instance.h"
#include "../common/linked_list.h"
#include "../common/servertalk.h"
#include "../common/say_link.h"
#include "../common/data_verification.h"

#include "../common/repositories/npc_types_repository.h"
#include "../common/repositories/spawngroup_repository.h"
#include "../common/repositories/spawn2_repository.h"
#include "../common/repositories/spawnentry_repository.h"

#include "client.h"
#include "entity.h"
#include "npc.h"
#include "string_ids.h"
#include "spawn2.h"
#include "zone.h"
#include "quest_parser_collection.h"
#include "water_map.h"
#include "npc_scale_manager.h"

#include "bot.h"
#include "../common/skill_caps.h"

#include <stdio.h>
#include <string>
#include <utility>

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include <stdlib.h>
#include <pthread.h>
#endif

extern Zone* zone;
extern volatile bool is_zone_loaded;
extern EntityList entity_list;

NPC::NPC(const NPCType *npc_type_data, Spawn2 *in_respawn, const glm::vec4 &position, GravityBehavior iflymode, bool IsCorpse)
	: Mob(
		  npc_type_data->name,
		  npc_type_data->lastname,
		  npc_type_data->max_hp,
		  npc_type_data->max_hp,
		  npc_type_data->gender,
		  npc_type_data->race,
		  npc_type_data->class_,
		  (bodyType) npc_type_data->bodytype,
		  npc_type_data->deity,
		  npc_type_data->level,
		  npc_type_data->npc_id,
		  npc_type_data->size,
		  npc_type_data->runspeed,
		  position,
		  npc_type_data->light, // innate_light
		  npc_type_data->texture,
		  npc_type_data->helmtexture,
		  npc_type_data->AC,
		  npc_type_data->ATK,
		  npc_type_data->STR,
		  npc_type_data->STA,
		  npc_type_data->DEX,
		  npc_type_data->AGI,
		  npc_type_data->INT,
		  npc_type_data->WIS,
		  npc_type_data->CHA,
		  npc_type_data->haircolor,
		  npc_type_data->beardcolor,
		  npc_type_data->eyecolor1,
		  npc_type_data->eyecolor2,
		  npc_type_data->hairstyle,
		  npc_type_data->luclinface,
		  npc_type_data->beard,
		  npc_type_data->drakkin_heritage,
		  npc_type_data->drakkin_tattoo,
		  npc_type_data->drakkin_details,
		  npc_type_data->armor_tint,
		  0,
		  npc_type_data->see_invis,            // pass see_invis/see_ivu flags to mob constructor
		  npc_type_data->see_invis_undead,
		  npc_type_data->see_hide,
		  npc_type_data->see_improved_hide,
		  npc_type_data->hp_regen,
		  npc_type_data->mana_regen,
		  npc_type_data->qglobal,
		  npc_type_data->maxlevel,
		  npc_type_data->scalerate,
		  npc_type_data->armtexture,
		  npc_type_data->bracertexture,
		  npc_type_data->handtexture,
		  npc_type_data->legtexture,
		  npc_type_data->feettexture,
		  npc_type_data->use_model,
		  npc_type_data->always_aggro,
		  npc_type_data->heroic_strikethrough,
		  npc_type_data->keeps_sold_items,
		  npc_type_data->hp_regen_per_second
	  ),
	  attacked_timer(CombatEventTimer_expire),
	  swarm_timer(100),
	  classattack_timer(1000),
	  monkattack_timer(1000),
	  knightattack_timer(1000),
	  assist_timer(AIassistcheck_delay),
	  qglobal_purge_timer(30000),
	  send_hp_update_timer(2000),
	  enraged_timer(1000),
	  taunt_timer(TauntReuseTime * 1000),
	  m_SpawnPoint(position),
	  m_GuardPoint(-1, -1, -1, 0),
	  m_GuardPointSaved(0, 0, 0, 0)
{
	//What is the point of this, since the names get mangled..
	Mob *mob = entity_list.GetMob(name);
	if (mob != nullptr) {
		entity_list.RemoveEntity(mob->GetID());
	}

	int moblevel = GetLevel();

	NPCTypedata      = npc_type_data;
	NPCTypedata_ours = nullptr;
	respawn2         = in_respawn;

	swarm_timer.Disable();

	if (size <= 0.0f) {
		size = GetRaceGenderDefaultHeight(race, gender);
	}

	// lava dragon is a fixed size model and should always use its default
	// otherwise pathing issues
	if (race == Race::LavaDragon) {
		size = 5;
	}
	if (race == Race::Wurm) {
		size = 15;
	}

	SetTaunting(false);
	proximity            = nullptr;
	m_loot_copper        = 0;
	m_loot_silver        = 0;
	m_loot_gold          = 0;
	m_loot_platinum      = 0;
	max_dmg              = npc_type_data->max_dmg;
	min_dmg              = npc_type_data->min_dmg;
	attack_count         = npc_type_data->attack_count;
	grid                 = 0;
	wp_m                 = 0;
	max_wp               = 0;
	save_wp              = 0;
	spawn_group_id       = 0;
	swarmInfoPtr         = nullptr;
	spellscale           = npc_type_data->spellscale;
	healscale            = npc_type_data->healscale;
	pAggroRange          = npc_type_data->aggroradius;
	pAssistRange         = npc_type_data->assistradius;
	findable             = npc_type_data->findable;
	trackable            = npc_type_data->trackable;
	MR                   = npc_type_data->MR;
	CR                   = npc_type_data->CR;
	DR                   = npc_type_data->DR;
	FR                   = npc_type_data->FR;
	PR                   = npc_type_data->PR;
	Corrup               = npc_type_data->Corrup;
	PhR                  = npc_type_data->PhR;
	STR                  = npc_type_data->STR;
	STA                  = npc_type_data->STA;
	AGI                  = npc_type_data->AGI;
	DEX                  = npc_type_data->DEX;
	INT                  = npc_type_data->INT;
	WIS                  = npc_type_data->WIS;
	CHA                  = npc_type_data->CHA;
	npc_mana             = npc_type_data->Mana;
	m_is_underwater_only = npc_type_data->underwater;
	m_is_quest_npc       = npc_type_data->is_quest_npc;

	//quick fix of ordering if they screwed it up in the DB
	if (max_dmg < min_dmg) {
		int tmp = min_dmg;
		min_dmg = max_dmg;
		max_dmg = tmp;
	}

	// Max Level and Stat Scaling if maxlevel is set
	if (maxlevel > level) {
		LevelScale();
	}

	base_damage          = round((max_dmg - min_dmg) / 1.9);
	min_damage           = min_dmg - round(base_damage / 10.0);
	accuracy_rating      = npc_type_data->accuracy_rating;
	avoidance_rating     = npc_type_data->avoidance_rating;
	ATK                  = npc_type_data->ATK;
	heroic_strikethrough = npc_type_data->heroic_strikethrough;
	keeps_sold_items     = npc_type_data->keeps_sold_items;

	// used for when switch back to charm
	default_ac               = npc_type_data->AC;
	default_min_dmg          = min_dmg;
	default_max_dmg          = max_dmg;
	default_attack_delay     = npc_type_data->attack_delay;
	default_accuracy_rating  = npc_type_data->accuracy_rating;
	default_avoidance_rating = npc_type_data->avoidance_rating;
	default_atk              = npc_type_data->ATK;
	strn0cpy(default_special_abilities, npc_type_data->special_abilities, 512);

	// used for when getting charmed, if 0, doesn't swap
	charm_ac               = npc_type_data->charm_ac;
	charm_min_dmg          = npc_type_data->charm_min_dmg;
	charm_max_dmg          = npc_type_data->charm_max_dmg;
	charm_attack_delay     = npc_type_data->charm_attack_delay;
	charm_accuracy_rating  = npc_type_data->charm_accuracy_rating;
	charm_avoidance_rating = npc_type_data->charm_avoidance_rating;
	charm_atk              = npc_type_data->charm_atk;

	CalcMaxMana();
	RestoreMana();

	MerchantType          = npc_type_data->merchanttype;
	merchant_open         = (
		GetClass() == Class::Merchant ||
		GetClass() == Class::DiscordMerchant ||
		GetClass() == Class::AdventureMerchant ||
		GetClass() == Class::NorrathsKeepersMerchant ||
		GetClass() == Class::DarkReignMerchant ||
		GetClass() == Class::AlternateCurrencyMerchant
	);
	adventure_template_id = npc_type_data->adventure_template;
	flymode               = iflymode;

	// If server has set a flymode in db honor it over all else.
	// If server has not set a flymde in db, and this is a boat - force floating.
	if (npc_type_data->flymode >= 0) {
		flymode = static_cast<GravityBehavior>(npc_type_data->flymode);
	}
	else if (GetIsBoat()) {
		flymode = GravityBehavior::Floating;
	}

	guard_anim            = eaStanding;

	m_roambox.max_x     = -2;
	m_roambox.max_y     = -2;
	m_roambox.min_x     = -2;
	m_roambox.min_y     = -2;
	m_roambox.distance  = 0;
	m_roambox.dest_x    = -2;
	m_roambox.dest_y    = -2;
	m_roambox.dest_z    = 0;
	m_roambox.delay     = 1000;
	m_roambox.min_delay = 1000;

	p_depop            = false;
	m_loottable_id     = npc_type_data->loottable_id;
	m_skip_global_loot = npc_type_data->skip_global_loot;
	m_skip_auto_scale  = npc_type_data->skip_auto_scale;
	rare_spawn         = npc_type_data->rare_spawn;
	no_target_hotkey   = npc_type_data->no_target_hotkey;
	primary_faction    = 0;
	faction_amount     = npc_type_data->faction_amount;

	SetNPCFactionID(npc_type_data->npc_faction_id);

	npc_spells_id        = 0;
	HasAISpell           = false;
	HasAISpellEffects    = false;
	innate_proc_spell_id = 0;
	m_record_loot_stats  = false;

	if (GetClass() == Class::MercenaryLiaison && RuleB(Mercs, AllowMercs)) {
		LoadMercenaryTypes();
		LoadMercenaries();
	}

	SpellFocusDMG  = 0;
	SpellFocusHeal = 0;

	pet_spell_id = 0;

	delaytimer      = false;
	combat_event    = false;
	attack_speed    = npc_type_data->attack_speed;
	attack_delay    = npc_type_data->attack_delay;
	slow_mitigation = npc_type_data->slow_mitigation;

	EntityList::RemoveNumbers(name);
	entity_list.MakeNameUnique(name);

	npc_aggro = npc_type_data->npc_aggro;

	AISpellVar.fail_recast                     = static_cast<uint32>(RuleI(Spells, AI_SpellCastFinishedFailRecast));
	AISpellVar.engaged_no_sp_recast_min        = static_cast<uint32>(RuleI(Spells, AI_EngagedNoSpellMinRecast));
	AISpellVar.engaged_no_sp_recast_max        = static_cast<uint32>(RuleI(Spells, AI_EngagedNoSpellMaxRecast));
	AISpellVar.engaged_beneficial_self_chance  = static_cast<uint8> (RuleI(Spells, AI_EngagedBeneficialSelfChance));
	AISpellVar.engaged_beneficial_other_chance = static_cast<uint8> (RuleI(Spells, AI_EngagedBeneficialOtherChance));
	AISpellVar.engaged_detrimental_chance      = static_cast<uint8> (RuleI(Spells, AI_EngagedDetrimentalChance));
	AISpellVar.pursue_no_sp_recast_min         = static_cast<uint32>(RuleI(Spells, AI_PursueNoSpellMinRecast));
	AISpellVar.pursue_no_sp_recast_max         = static_cast<uint32>(RuleI(Spells, AI_PursueNoSpellMaxRecast));
	AISpellVar.pursue_detrimental_chance       = static_cast<uint8> (RuleI(Spells, AI_PursueDetrimentalChance));
	AISpellVar.idle_no_sp_recast_min           = static_cast<uint32>(RuleI(Spells, AI_IdleNoSpellMinRecast));
	AISpellVar.idle_no_sp_recast_max           = static_cast<uint32>(RuleI(Spells, AI_IdleNoSpellMaxRecast));
	AISpellVar.idle_beneficial_chance          = static_cast<uint8> (RuleI(Spells, AI_IdleBeneficialChance));

	// It's possible for IsBot() to not be set yet during Bot loading, so have to use an alternative to catch Bots
	if (!EQ::ValueWithin(npc_type_data->npc_spells_id, EQ::constants::BotSpellIDs::Warrior, EQ::constants::BotSpellIDs::Berserker)) {
		AI_Init();
		AI_Start();
	} else {
		CastToBot()->AI_Bot_Init();
		CastToBot()->AI_Bot_Start();
	}

	d_melee_texture1 = npc_type_data->d_melee_texture1;
	d_melee_texture2 = npc_type_data->d_melee_texture2;
	herosforgemodel  = npc_type_data->herosforgemodel;

	ammo_idfile = npc_type_data->ammo_idfile;
	memset(equipment, 0, sizeof(equipment));
	prim_melee_type = npc_type_data->prim_melee_type;
	sec_melee_type  = npc_type_data->sec_melee_type;
	ranged_type     = npc_type_data->ranged_type;

	// If Melee Textures are not set, set attack type to Hand to Hand as default
	if (!d_melee_texture1) {
		prim_melee_type = 28;
	}
	if (!d_melee_texture2) {
		sec_melee_type = 28;
	}

	//give NPCs skill values...
	int r;
	for (r = 0; r <= EQ::skills::HIGHEST_SKILL; r++) {
		skills[r] = skill_caps.GetSkillCap(GetClass(), (EQ::skills::SkillType)r, moblevel).cap;
	}
	// some overrides -- really we need to be able to set skills for mobs in the DB
	// There are some known low level SHM/BST pets that do not follow this, which supports
	// the theory of needing to be able to set skills for each mob separately
	if (IsBot()) {
		if (GetClass() != Class::Paladin && GetClass() != Class::ShadowKnight) {
			knightattack_timer.Disable();
		}
		else if (GetClass() != Class::Monk || GetLevel() < 10) {
			monkattack_timer.Disable();
		}
	}
	else {
		if (moblevel > 50) {
			skills[EQ::skills::SkillDoubleAttack] = 250;
			skills[EQ::skills::SkillDualWield] = 250;
		}
		else if (moblevel > 3) {
			skills[EQ::skills::SkillDoubleAttack] = moblevel * 5;
			skills[EQ::skills::SkillDualWield] = skills[EQ::skills::SkillDoubleAttack];
		}
		else {
			skills[EQ::skills::SkillDoubleAttack] = moblevel * 5;
		}
	}

	SetLDoNTrapped(false);
	SetLDoNTrapType(0);
	SetLDoNTrapSpellID(0);
	SetLDoNLocked(false);
	SetLDoNLockedSkill(0);
	SetLDoNTrapDetected(false);

	if (npc_type_data->trap_template > 0) {
		std::map<uint32, std::list<LDoNTrapTemplate *> >::iterator trap_ent_iter;
		std::list<LDoNTrapTemplate *> trap_list;

		trap_ent_iter = zone->ldon_trap_entry_list.find(npc_type_data->trap_template);
		if (trap_ent_iter != zone->ldon_trap_entry_list.end()) {
			trap_list = trap_ent_iter->second;
			if (trap_list.size() > 0) {
				auto trap_list_iter = trap_list.begin();
				std::advance(trap_list_iter, zone->random.Int(0, trap_list.size() - 1));
				LDoNTrapTemplate *trap_template = (*trap_list_iter);
				if (trap_template) {
					if (trap_template->spell_id > 0) {
						SetLDoNTrapped(true);
						SetLDoNTrapSpellID(trap_template->spell_id);
					} else {
						SetLDoNTrapped(false);
						SetLDoNTrapSpellID(0);
					}

					SetLDoNTrapType(static_cast<uint8>(trap_template->type));

					if (trap_template->locked > 0) {
						SetLDoNLocked(true);
						SetLDoNLockedSkill(trap_template->skill);
					} else {
						SetLDoNLocked(false);
						SetLDoNLockedSkill(0);
					}

					SetLDoNTrapDetected(false);
				}
			}
		}
	}

	reface_timer = new Timer(15000);
	reface_timer->Disable();

	qGlobals = nullptr;

	SetEmoteID(npc_type_data->emoteid);
	InitializeBuffSlots();
	CalcBonuses();

	raid_target    = npc_type_data->raid_target;
	ignore_despawn = npc_type_data->ignore_despawn;
	m_targetable   = !npc_type_data->untargetable;

	npc_scale_manager->ScaleNPC(this);

	RestoreMana();

	if (GetBodyType() == BT_Animal && !RuleB(NPC, AnimalsOpenDoors)) {
		m_can_open_doors = false;
	}

	GetInv().SetInventoryVersion(EQ::versions::MobVersion::RoF2);
	GetInv().SetGMInventory(false);
}

float NPC::GetRoamboxMaxX() const
{
	return m_roambox.max_x;
}

float NPC::GetRoamboxMaxY() const
{
	return m_roambox.max_y;
}

float NPC::GetRoamboxMinX() const
{
	return m_roambox.min_x;
}

float NPC::GetRoamboxMinY() const
{
	return m_roambox.min_y;
}

float NPC::GetRoamboxDistance() const
{
	return m_roambox.distance;
}

float NPC::GetRoamboxDestinationX() const
{
	return m_roambox.dest_x;
}

float NPC::GetRoamboxDestinationY() const
{
	return m_roambox.dest_y;
}

float NPC::GetRoamboxDestinationZ() const
{
	return m_roambox.dest_z;
}

uint32 NPC::GetRoamboxDelay() const
{
	return m_roambox.delay;
}

uint32 NPC::GetRoamboxMinDelay() const
{
	return m_roambox.min_delay;
}

NPC::~NPC()
{
	AI_Stop();

	if (proximity) {
		entity_list.RemoveProximity(GetID());
		safe_delete(proximity);
	}

	safe_delete(NPCTypedata_ours);

	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		safe_delete(item);
	}

	m_loot_items.clear();
	faction_list.clear();

	safe_delete(reface_timer);
	safe_delete(swarmInfoPtr);
	safe_delete(qGlobals);
	UninitializeBuffSlots();
}

void NPC::SetTarget(Mob* mob) {
	if(mob == GetTarget())		//dont bother if they are allready our target
		return;

	if (GetPetTargetLockID()) {
		TryDepopTargetLockedPets(mob);
	}

	if (mob) {
		SetAttackTimer();
	} else {
		ranged_timer.Disable();
		//attack_timer.Disable();
		attack_dw_timer.Disable();
	}

	// either normal pet and owner is client or charmed pet and owner is client
	Mob *owner = nullptr;
	if (IsPet() && IsPetOwnerClient()) {
		owner = GetOwner();
	} else if (IsCharmed()) {
		owner = GetOwner();
		if (owner && !owner->IsClient())
			owner = nullptr;
	}

	if (owner) {
		auto client = owner->CastToClient();
		if (client->ClientVersionBit() & EQ::versions::maskUFAndLater) {
			auto app = new EQApplicationPacket(OP_PetHoTT, sizeof(ClientTarget_Struct));
			auto ct = (ClientTarget_Struct *)app->pBuffer;
			ct->new_target = mob ? mob->GetID() : 0;
			client->FastQueuePacket(&app);
		}
	}
	Mob::SetTarget(mob);
}

bool NPC::Process()
{
	if (p_depop)
	{
		Mob* owner = entity_list.GetMob(ownerid);
		if (owner != 0)
		{
			//if(GetBodyType() != BT_SwarmPet)
			// owner->SetPetID(0);
			ownerid = 0;
			petid = 0;
		}
		return false;
	}

	if (IsStunned() && stunned_timer.Check()) {
		Mob::UnStun();
		spun_timer.Disable();
	}

	SpellProcess();

	if (swarm_timer.Check()) {
		DepopSwarmPets();
	}

	if (mob_close_scan_timer.Check()) {
		entity_list.ScanCloseMobs(close_mobs, this, IsMoving());
	}

	const uint16 npc_mob_close_scan_timer_moving = 6000;
	const uint16 npc_mob_close_scan_timer_idle   = 60000;

	if (mob_check_moving_timer.Check()) {
		if (moving) {
			if (mob_close_scan_timer.GetRemainingTime() > npc_mob_close_scan_timer_moving) {
				LogAIScanCloseDetail("NPC [{}] Restarting with moving timer", GetCleanName());
				mob_close_scan_timer.Disable();
				mob_close_scan_timer.Start(npc_mob_close_scan_timer_moving);
				mob_close_scan_timer.Trigger();
			}
		}
		else if (mob_close_scan_timer.GetDuration() == npc_mob_close_scan_timer_moving) {
			LogAIScanCloseDetail("NPC [{}] Restarting with idle timer", GetCleanName());
			mob_close_scan_timer.Disable();
			mob_close_scan_timer.Start(npc_mob_close_scan_timer_idle);
		}
	}

	if (hp_regen_per_second > 0 && hp_regen_per_second_timer.Check()) {
		if (GetHP() < GetMaxHP()) {
			SetHP(GetHP() + hp_regen_per_second);
		}
	}

	if (tic_timer.Check()) {
		if (parse->HasQuestSub(GetNPCTypeID(), EVENT_TICK)) {
			parse->EventNPC(EVENT_TICK, this, nullptr, "", 0);
		}

		BuffProcess();

		if (currently_fleeing) {
			ProcessFlee();
		}

		int64 npc_sitting_regen_bonus = 0;
		int64 pet_regen_bonus = 0;
		int64 npc_regen = 0;
		int64 npc_hp_regen = GetNPCHPRegen();

		if (GetAppearance() == eaSitting) {
			npc_sitting_regen_bonus += 3;
		}

		int64 ooc_regen_calc = 0;
		if (ooc_regen > 0) { //should pull from Mob class
			ooc_regen_calc += GetMaxHP() * ooc_regen / 100;
		}

		/**
		 * Use max value between two values
		 */
		npc_regen = std::max(npc_hp_regen, ooc_regen_calc);

		if ((GetHP() < GetMaxHP()) && !IsPet()) {
			if (!IsEngaged()) {
				SetHP(GetHP() + npc_regen + npc_sitting_regen_bonus);
			}
			else {
				SetHP(GetHP() + npc_hp_regen);
			}
		}
		else if (GetHP() < GetMaxHP() && GetOwnerID() != 0) {
			if (!IsEngaged()) {
				if (ooc_regen > 0) {
					pet_regen_bonus = std::max(ooc_regen_calc, npc_hp_regen);
				}
				else {
					pet_regen_bonus = npc_hp_regen + (GetLevel() / 5);
				}

				SetHP(GetHP() + npc_sitting_regen_bonus + pet_regen_bonus);
			}
			else {
				SetHP(GetHP() + npc_hp_regen);
			}

		}
		else {
			SetHP(GetHP() + npc_hp_regen + npc_sitting_regen_bonus);
		}

		if (GetMana() < GetMaxMana()) {
			if (RuleB(NPC, UseMeditateBasedManaRegen)) {
				int64 npc_idle_mana_regen_bonus = 2;
				uint16 meditate_skill = GetSkill(EQ::skills::SkillMeditate);
				if (!IsEngaged() && meditate_skill > 0) {
					uint8 clevel = GetLevel();
					npc_idle_mana_regen_bonus =
						(((meditate_skill / 10) +
						(clevel - (clevel / 4))) / 4) + 4;
				}
				SetMana(GetMana() + mana_regen + npc_idle_mana_regen_bonus);
			}
			else {
				SetMana(GetMana() + mana_regen + npc_sitting_regen_bonus);
			}
		}

		SendHPUpdate();

		if (zone->adv_data && !p_depop) {
			ServerZoneAdventureDataReply_Struct *ds = (ServerZoneAdventureDataReply_Struct *) zone->adv_data;
			if (ds->type == Adventure_Rescue && ds->data_id == GetNPCTypeID()) {
				Mob *o = GetOwner();
				if (o && o->IsClient()) {
					float x_diff = ds->dest_x - GetX();
					float y_diff = ds->dest_y - GetY();
					float z_diff = ds->dest_z - GetZ();
					float dist   = ((x_diff * x_diff) + (y_diff * y_diff) + (z_diff * z_diff));
					if (dist < RuleR(Adventure, DistanceForRescueComplete)) {
						zone->DoAdventureCountIncrease();
						Say(
							"You don't know what this means to me. Thank you so much for finding and saving me from"
							" this wretched place. I'll find my way from here."
						);
						Depop();
					}
				}
			}
		}
	}

	/**
	 * Send HP updates when engaged
	 */
	if (send_hp_update_timer.Check(false) && IsEngaged()) {
		SendHPUpdate();
	}

	if (viral_timer.Check()) {
		VirusEffectProcess();
	}

	if(spellbonuses.GravityEffect == 1) {
		if(gravity_timer.Check())
			DoGravityEffect();
	}

	if(reface_timer->Check() && !IsEngaged() && IsPositionEqualWithinCertainZ(m_Position, m_GuardPoint, 5.0f)) {
		RotateTo(m_GuardPoint.w);
		reface_timer->Disable();
	}

	// needs to be done before mez and stun
	if (ForcedMovement)
		ProcessForcedMovement();

	if (IsMezzed())
		return true;

	if(IsStunned()) {
		if(spun_timer.Check())
			Spin();
		return true;
	}

	if (enraged_timer.Check()){
		ProcessEnrage();

		/* Don't keep running the check every second if we don't have enrage */
		if (!GetSpecialAbility(SPECATK_ENRAGE)) {
			enraged_timer.Disable();
		}
	}

	//Handle assists...
	if (assist_cap_timer.Check()) {
		if (NPCAssistCap() > 0)
			DelAssistCap();
		else
			assist_cap_timer.Disable();
	}

	if (assist_timer.Check() && IsEngaged() && !Charmed() && !HasAssistAggro() &&
	    NPCAssistCap() < RuleI(Combat, NPCAssistCap)) {
		// Some cases like flash of light used for aggro haven't set target
		if (!GetTarget()) {
			SetTarget(hate_list.GetMobWithMostHateOnList(this));
		}
		AIYellForHelp(this, GetTarget());
		if (NPCAssistCap() > 0 && !assist_cap_timer.Enabled())
			assist_cap_timer.Start(RuleI(Combat, NPCAssistCapTimer));
	}

	if(qGlobals)
	{
		if(qglobal_purge_timer.Check())
		{
			qGlobals->PurgeExpiredGlobals();
		}
	}

	AI_Process();

	return true;
}

uint32 NPC::CountLoot() {
	return(m_loot_items.size());
}

void NPC::UpdateEquipmentLight()
{
	m_Light.Type[EQ::lightsource::LightEquipment] = 0;
	m_Light.Level[EQ::lightsource::LightEquipment] = 0;

	for (int index = EQ::invslot::EQUIPMENT_BEGIN; index <= EQ::invslot::EQUIPMENT_END; ++index) {
		if (index == EQ::invslot::slotAmmo) { continue; }

		auto item = database.GetItem(equipment[index]);
		if (item == nullptr) { continue; }

		if (EQ::lightsource::IsLevelGreater(item->Light, m_Light.Type[EQ::lightsource::LightEquipment])) {
			m_Light.Type[EQ::lightsource::LightEquipment] = item->Light;
			m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]);
		}
	}

	uint8 general_light_type = 0;
	for (auto iter = m_loot_items.begin(); iter != m_loot_items.end(); ++iter) {
		auto item = database.GetItem((*iter)->item_id);
		if (item == nullptr) { continue; }

		if (!item->IsClassCommon()) { continue; }
		if (item->Light < 9 || item->Light > 13) { continue; }

		if (EQ::lightsource::TypeToLevel(item->Light))
			general_light_type = item->Light;
	}

	if (EQ::lightsource::IsLevelGreater(general_light_type, m_Light.Type[EQ::lightsource::LightEquipment]))
		m_Light.Type[EQ::lightsource::LightEquipment] = general_light_type;

	m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]);
}

void NPC::Depop(bool start_spawn_timer) {
	if (emoteid) {
		DoNPCEmote(EQ::constants::EmoteEventTypes::OnDespawn, emoteid);
	}

	if (IsNPC()) {
		if (parse->HasQuestSub(GetNPCTypeID(), EVENT_DESPAWN)) {
			parse->EventNPC(EVENT_DESPAWN, this, nullptr, "", 0);
		}

		if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_DESPAWN_ZONE)) {
			DispatchZoneControllerEvent(EVENT_DESPAWN_ZONE, this, "", 0, nullptr);
		}
	} else if (IsBot()) {
		if (parse->BotHasQuestSub(EVENT_DESPAWN)) {
			parse->EventBot(EVENT_DESPAWN, CastToBot(), nullptr, "", 0);
		}

		if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_DESPAWN_ZONE)) {
			DispatchZoneControllerEvent(EVENT_DESPAWN_ZONE, this, "", 0, nullptr);
		}
	}

	p_depop = true;
	if (respawn2) {
		if (start_spawn_timer) {
			respawn2->DeathReset();
		} else {
			respawn2->Depop();
		}
	}
}

bool NPC::SpawnZoneController()
{

	if (!RuleB(Zone, UseZoneController))
		return false;

	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	strncpy(npc_type->name, "zone_controller", 60);
	npc_type->current_hp       = 2000000000;
	npc_type->max_hp           = 2000000000;
	npc_type->hp_regen         = 100000000;
	npc_type->race             = 240;
	npc_type->size             = .1;
	npc_type->gender           = 2;
	npc_type->class_           = 1;
	npc_type->deity            = 1;
	npc_type->level            = 200;
	npc_type->npc_id           = ZONE_CONTROLLER_NPC_ID;
	npc_type->loottable_id     = 0;
	npc_type->texture          = 3;
	npc_type->runspeed         = 0;
	npc_type->d_melee_texture1 = 0;
	npc_type->d_melee_texture2 = 0;
	npc_type->merchanttype     = 0;
	npc_type->bodytype         = 11;
	npc_type->skip_global_loot = true;

	if (RuleB(Zone, EnableZoneControllerGlobals)) {
		npc_type->qglobal = true;
	}

	npc_type->prim_melee_type = 28;
	npc_type->sec_melee_type  = 28;

	npc_type->findable  = 0;
	npc_type->trackable = 0;

	strcpy(npc_type->special_abilities, "12,1^13,1^14,1^15,1^16,1^17,1^19,1^22,1^24,1^25,1^28,1^31,1^35,1^39,1^42,1");

	glm::vec4 point;
	point.x = 3000;
	point.y = 1000;
	point.z = 500;

	auto npc = new NPC(npc_type, nullptr, point, GravityBehavior::Flying);
	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc);

	return true;
}

void NPC::SpawnGridNodeNPC(const glm::vec4 &position, int32 grid_id, int32 grid_number, int32 zoffset) {
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	std::string str_zoffset = Strings::NumberToWords(zoffset);
	std::string str_number = Strings::NumberToWords(grid_number);

	strcpy(npc_type->name, str_number.c_str());
	if (zoffset != 0) {
		strcat(npc_type->name, "(Stacked)");
	}

	npc_type->current_hp = 4000000;
	npc_type->max_hp = 4000000;
	npc_type->race = 2254;
	npc_type->gender = Gender::Neuter;
	npc_type->class_ = 9;
	npc_type->deity = 1;
	npc_type->level = 200;
	npc_type->npc_id = 0;
	npc_type->loottable_id = 0;
	npc_type->texture = 1;
	npc_type->light = 1;
	npc_type->size = 1;
	npc_type->runspeed = 0;
	npc_type->merchanttype = 1;
	npc_type->bodytype = 1;
	npc_type->show_name = true;
	npc_type->findable = true;
	strn0cpy(npc_type->special_abilities, "24,1^35,1", 512);

	auto node_position = glm::vec4(position.x, position.y, position.z, position.w);
	auto npc = new NPC(npc_type, nullptr, node_position, GravityBehavior::Flying);
	npc->name[strlen(npc->name) - 3] = (char) NULL;
	npc->GiveNPCTypeData(npc_type);
	npc->SetEntityVariable("grid_id", itoa(grid_id));
	entity_list.AddNPC(npc);
}

NPC * NPC::SpawnZonePointNodeNPC(std::string name, const glm::vec4 &position)
{
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	char node_name[64];
	strn0cpy(node_name, name.c_str(), 64);

	strcpy(npc_type->name, entity_list.MakeNameUnique(node_name));

	npc_type->current_hp       = 4000000;
	npc_type->max_hp           = 4000000;
	npc_type->race             = 2254;
	npc_type->gender           = 2;
	npc_type->class_           = 9;
	npc_type->deity            = 1;
	npc_type->level            = 200;
	npc_type->npc_id           = 0;
	npc_type->loottable_id     = 0;
	npc_type->texture          = 1;
	npc_type->light            = 1;
	npc_type->size             = 5;
	npc_type->runspeed         = 0;
	npc_type->merchanttype     = 1;
	npc_type->bodytype         = 1;
	npc_type->show_name        = true;
	npc_type->findable         = true;

	strcpy(npc_type->special_abilities, "12,1^13,1^14,1^15,1^16,1^17,1^19,1^22,1^24,1^25,1^28,1^31,1^35,1^39,1^42,1");

	auto node_position = glm::vec4(position.x, position.y, position.z, position.w);
	auto npc           = new NPC(npc_type, nullptr, node_position, GravityBehavior::Flying);

	npc->name[strlen(npc->name)-3] = (char) NULL;

	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc);

	return npc;
}

NPC * NPC::SpawnNodeNPC(std::string name, std::string last_name, const glm::vec4 &position) {
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	strncpy(npc_type->name, name.c_str(), 60);

	npc_type->current_hp       = 4000000;
	npc_type->max_hp           = 4000000;
	npc_type->race             = 2254;
	npc_type->gender           = 2;
	npc_type->class_           = 9;
	npc_type->deity            = 1;
	npc_type->level            = 200;
	npc_type->npc_id           = 0;
	npc_type->loottable_id     = 0;
	npc_type->texture          = 1;
	npc_type->light            = 1;
	npc_type->size             = 3;
	npc_type->d_melee_texture1 = 1;
	npc_type->d_melee_texture2 = 1;
	npc_type->merchanttype     = 1;
	npc_type->bodytype         = 1;
	npc_type->show_name        = true;
	npc_type->findable         = true;
	npc_type->runspeed         = 1.25;

	strcpy(npc_type->special_abilities, "12,1^13,1^14,1^15,1^16,1^17,1^19,1^22,1^24,1^25,1^28,1^31,1^35,1^39,1^42,1");

	auto node_position = glm::vec4(position.x, position.y, position.z, position.w);
	auto npc           = new NPC(npc_type, nullptr, node_position, GravityBehavior::Flying);

	npc->name[strlen(npc->name) - 3] = (char) NULL;

	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc, true, true);

	return npc;
}

NPC* NPC::SpawnNPC(const char* spawncommand, const glm::vec4& position, Client* client) {
	if (spawncommand == 0 || spawncommand[0] == 0) {
		return 0;
	}
	else {
		Seperator sep(spawncommand);
		//Lets see if someone didn't fill out the whole #spawn function properly
		if (!sep.IsNumber(1)) {
			sprintf(sep.arg[1], "1");
		}

		if (!sep.IsNumber(2)) {
			sprintf(sep.arg[2], "1");
		}

		if (!sep.IsNumber(3)) {
			sprintf(sep.arg[3], "0");
		}

		if (Strings::ToInt(sep.arg[4]) > 2100000000 || Strings::ToInt(sep.arg[4]) <= 0) {
			sprintf(sep.arg[4], " ");
		}

		if (!strcmp(sep.arg[5], "-")) {
			sprintf(sep.arg[5], " ");
		}

		if (!sep.IsNumber(5)) {
			sprintf(sep.arg[5], " ");
		}

		if (!sep.IsNumber(6)) {
			sprintf(sep.arg[6], "1");
		}

		if (!sep.IsNumber(8)) {
			sprintf(sep.arg[8], "0");
		}

		if (!sep.IsNumber(9)) {
			sprintf(sep.arg[9], "0");
		}

		if (!sep.IsNumber(7)) {
			sprintf(sep.arg[7], "0");
		}

		if (!strcmp(sep.arg[4], "-")) {
			sprintf(sep.arg[4], " ");
		}

		if (!sep.IsNumber(10)) {    // bodytype
			sprintf(sep.arg[10], "0");
		}

		//Calc MaxHP if client neglected to enter it...
		if (sep.arg[4] && !sep.IsNumber(4)) {
			sprintf(sep.arg[4], "1");
		}

		// Autoselect NPC Gender
		if (sep.arg[5][0] == 0) {
			sprintf(sep.arg[5], "%i", (int) Mob::GetDefaultGender(Strings::ToInt(sep.arg[1])));
		}

		//Time to create the NPC!!
		auto npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));

		strncpy(npc_type->name, sep.arg[0], 60);

		npc_type->current_hp       = Strings::ToInt(sep.arg[4]);
		npc_type->max_hp           = Strings::ToInt(sep.arg[4]);
		npc_type->race             = Strings::ToInt(sep.arg[1]);
		npc_type->gender           = Strings::ToInt(sep.arg[5]);
		npc_type->class_           = Strings::ToInt(sep.arg[6]);
		npc_type->deity            = 1;
		npc_type->level            = Strings::ToInt(sep.arg[2]);
		npc_type->npc_id           = 0;
		npc_type->loottable_id     = 0;
		npc_type->texture          = Strings::ToInt(sep.arg[3]);
		npc_type->light            = 0;
		npc_type->runspeed         = 1.25f;
		npc_type->d_melee_texture1 = Strings::ToUnsignedInt(sep.arg[7]);
		npc_type->d_melee_texture2 = Strings::ToUnsignedInt(sep.arg[8]);
		npc_type->merchanttype     = Strings::ToInt(sep.arg[9]);
		npc_type->bodytype         = Strings::ToInt(sep.arg[10]);

		npc_type->STR = 0;
		npc_type->STA = 0;
		npc_type->DEX = 0;
		npc_type->AGI = 0;
		npc_type->INT = 0;
		npc_type->WIS = 0;
		npc_type->CHA = 0;

		npc_type->attack_delay = 3000;

		npc_type->prim_melee_type = static_cast<uint8>(EQ::skills::SkillHandtoHand);
		npc_type->sec_melee_type  = static_cast<uint8>(EQ::skills::SkillHandtoHand);

		auto npc = new NPC(npc_type, nullptr, position, GravityBehavior::Water);
		npc->GiveNPCTypeData(npc_type);

		entity_list.AddNPC(npc);

		if (client) {
			// Notify client of spawn data
			client->Message(Chat::White, fmt::format("Name | {}", npc->name).c_str());
			client->Message(Chat::White, fmt::format("Level | {}", npc->level).c_str());
			client->Message(Chat::White, fmt::format("Health | {}", npc->max_hp).c_str());
			client->Message(Chat::White, fmt::format("Race | {} ({})", GetRaceIDName(npc->race), npc->race).c_str());
			client->Message(Chat::White, fmt::format("Class | {} ({})", GetClassIDName(npc->class_), npc->class_).c_str());
			client->Message(Chat::White, fmt::format("Gender | {} ({})", GetGenderName(npc->gender), npc->gender).c_str());
			client->Message(Chat::White, fmt::format("Texture | {}", npc->texture).c_str());

			if (npc->d_melee_texture1 || npc->d_melee_texture2) {
				client->Message(
					Chat::White,
					fmt::format(
						"Weapon Item Number | Primary: {} Secondary: {}",
						npc->d_melee_texture1,
						npc->d_melee_texture2
					).c_str()
				);
			}

			if (npc->MerchantType) {
				client->Message(Chat::White, fmt::format("Merchant ID | {}", npc->MerchantType).c_str());
			}

			if (npc->bodytype) {
				client->Message(Chat::White, fmt::format("Body Type | {} ({})", EQ::constants::GetBodyTypeName(npc->bodytype), npc->bodytype).c_str());
			}

			client->Message(Chat::White, "New NPC spawned!");
		}

		return npc;
	}
}

uint32 ZoneDatabase::CreateNewNPCCommand(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 extra
)
{
	auto e = NpcTypesRepository::NewEntity();

	e.id              = NpcTypesRepository::GetOpenIDInZoneRange(*this, c->GetZoneID());
	e.name            = Strings::RemoveNumbers(n->GetName());
	e.level           = n->GetLevel();
	e.race            = n->GetRace();
	e.class_          = n->GetClass();
	e.hp              = n->GetMaxHP();
	e.gender          = n->GetGender();
	e.texture         = n->GetTexture();
	e.helmtexture     = n->GetHelmTexture();
	e.size            = n->GetSize();
	e.loottable_id    = n->GetLoottableID();
	e.merchant_id     = n->MerchantType;
	e.runspeed        = n->GetRunspeed();
	e.prim_melee_type = static_cast<uint8_t>(EQ::skills::SkillHandtoHand);
	e.sec_melee_type  = static_cast<uint8_t>(EQ::skills::SkillHandtoHand);

	e = NpcTypesRepository::InsertOne(*this, e);

	if (!e.id) {
		return false;
	}

	auto sg = SpawngroupRepository::NewEntity();

	sg.name = fmt::format(
		"{}-{}",
		zone,
		n->GetName()
	);

	sg = SpawngroupRepository::InsertOne(*this, sg);

	if (!sg.id) {
		return false;
	}

	n->SetSpawnGroupId(sg.id);
	n->SetNPCTypeID(e.id);

	auto s2 = Spawn2Repository::NewEntity();

	s2.zone         = zone;
	s2.version      = instance_version;
	s2.x            = n->GetX();
	s2.y            = n->GetY();
	s2.z            = n->GetZ();
	s2.respawntime  = 1200;
	s2.heading      = n->GetHeading();
	s2.spawngroupID = sg.id;

	s2 = Spawn2Repository::InsertOne(*this, s2);

	if (!s2.id) {
		return false;
	}

	auto se = SpawnentryRepository::NewEntity();

	se.spawngroupID = sg.id;
	se.npcID        = e.id;
	se.chance       = 100;

	se = SpawnentryRepository::InsertOne(*this, se);

	if (!se.spawngroupID) {
		return false;
	}

	return true;
}

uint32 ZoneDatabase::AddNewNPCSpawnGroupCommand(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 in_respawn_time
)
{
	auto sg = SpawngroupRepository::InsertOne(
		*this,
		SpawngroupRepository::Spawngroup{
			.name = fmt::format(
				"{}_{}_{}",
				zone,
				Strings::Escape(n->GetName()),
				Timer::GetCurrentTime()
			)
		}
	);

	if (!sg.id) {
		return 0;
	}

	uint32 respawn_time = 1200;
	uint32 spawn_id     = 0;

	if (in_respawn_time) {
		respawn_time = in_respawn_time;
	} else if (n->respawn2 && n->respawn2->RespawnTimer()) {
		respawn_time = n->respawn2->RespawnTimer();
	}

	auto s2 = Spawn2Repository::NewEntity();

	s2.zone         = zone;
	s2.version      = instance_version;
	s2.x            = n->GetX();
	s2.y            = n->GetY();
	s2.z            = n->GetZ();
	s2.heading      = n->GetHeading();
	s2.respawntime  = respawn_time;
	s2.spawngroupID = sg.id;

	s2 = Spawn2Repository::InsertOne(*this, s2);

	if (!s2.id) {
		return 0;
	}

	auto se = SpawnentryRepository::NewEntity();

	se.spawngroupID = sg.id;
	se.npcID        = n->GetNPCTypeID();
	se.chance       = 100;

	se = SpawnentryRepository::InsertOne(*this, se);

	if (!se.spawngroupID) {
		return 0;
	}

	return s2.id;
}

uint32 ZoneDatabase::UpdateNPCTypeAppearance(Client* c, NPC* n)
{
	auto e = NpcTypesRepository::FindOne(*this, n->GetNPCTypeID());

	e.name         = Strings::RemoveNumbers(n->GetName());
	e.level        = n->GetLevel();
	e.race         = n->GetRace();
	e.class_       = n->GetClass();
	e.hp           = n->GetMaxHP();
	e.gender       = n->GetGender();
	e.texture      = n->GetTexture();
	e.helmtexture  = n->GetHelmTexture();
	e.size         = n->GetSize();
	e.loottable_id = n->GetLoottableID();
	e.merchant_id  = n->MerchantType;
	e.face         = n->GetLuclinFace();

	const int updated = NpcTypesRepository::UpdateOne(*this, e);

	return updated;
}

uint32 ZoneDatabase::DeleteSpawnLeaveInNPCTypeTable(const std::string& zone, Client* c, NPC* n)
{
	const auto& l = Spawn2Repository::GetWhere(
		*this,
		fmt::format(
			"`zone` = '{}' AND `spawngroupID` = {}",
			zone,
			n->GetSpawnGroupId()
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto e = l.front();

	if (!Spawn2Repository::DeleteOne(*this, e.id)) {
		return 0;
	}

	if (!SpawngroupRepository::DeleteOne(*this, e.spawngroupID)) {
		return 0;
	}

	if (!SpawnentryRepository::DeleteOne(*this, e.spawngroupID)) {
		return 0;
	}

	return 1;
}

uint32 ZoneDatabase::DeleteSpawnRemoveFromNPCTypeTable(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n
)
{
	const auto& l = Spawn2Repository::GetWhere(
		*this,
		fmt::format(
			"`zone` = '{}' AND (`version` = {} OR `version` = -1) AND `spawngroupID` = {}",
			zone,
			instance_version,
			n->GetSpawnGroupId()
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto e = l.front();

	if (!Spawn2Repository::DeleteOne(*this, e.id)) {
		return 0;
	}

	if (!SpawngroupRepository::DeleteOne(*this, e.spawngroupID)) {
		return 0;
	}

	if (!SpawnentryRepository::DeleteOne(*this, e.spawngroupID)) {
		return 0;
	}

	if (!NpcTypesRepository::DeleteOne(*this, n->GetNPCTypeID())) {
		return 0;
	}

	return 1;
}

uint32 ZoneDatabase::AddSpawnFromSpawnGroup(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 spawngroup_id
)
{
	auto e = Spawn2Repository::NewEntity();

	e.zone         = zone;
	e.version      = instance_version;
	e.x            = c->GetX();
	e.y            = c->GetY();
	e.z            = c->GetZ();
	e.heading      = c->GetHeading();
	e.respawntime  = 120;
	e.spawngroupID = spawngroup_id;

	e = Spawn2Repository::InsertOne(*this, e);

	if (!e.id) {
		return 0;
	}

	return 1;
}

uint32 ZoneDatabase::AddNPCTypes(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 spawngroup_id
)
{
	auto e = NpcTypesRepository::NewEntity();

	e.name            = Strings::RemoveNumbers(n->GetName());
	e.level           = n->GetLevel();
	e.race            = n->GetRace();
	e.class_          = n->GetClass();
	e.hp              = n->GetMaxHP();
	e.gender          = n->GetGender();
	e.texture         = n->GetTexture();
	e.helmtexture     = n->GetHelmTexture();
	e.size            = n->GetSize();
	e.loottable_id    = n->GetLoottableID();
	e.merchant_id     = n->MerchantType;
	e.face            = n->GetLuclinFace();
	e.runspeed        = n->GetRunspeed();
	e.prim_melee_type = static_cast<uint8_t>(EQ::skills::SkillHandtoHand);
	e.sec_melee_type  = static_cast<uint8_t>(EQ::skills::SkillHandtoHand);

	e = NpcTypesRepository::InsertOne(*this, e);

	if (!e.id) {
		return 0;
	}

	if (c) {
		c->Message(
			Chat::White,
			fmt::format(
				"NPC Created | ID: {} Name: {}",
				e.id,
				e.name
			).c_str()
		);
	}

	return 1;
}

uint32 ZoneDatabase::NPCSpawnDB(
	uint8 command,
	const std::string& zone,
	uint32 instance_version,
	Client *c,
	NPC* n,
	uint32 extra
)
{
	switch (command) {
		case NPCSpawnTypes::CreateNewSpawn: {
			return CreateNewNPCCommand(zone, instance_version, c, n, extra);
		}
		case NPCSpawnTypes::AddNewSpawngroup: {
			return AddNewNPCSpawnGroupCommand(zone, instance_version, c, n, extra);
		}
		case NPCSpawnTypes::UpdateAppearance: {
			return UpdateNPCTypeAppearance(c, n);
		}
		case NPCSpawnTypes::RemoveSpawn: {
			return DeleteSpawnLeaveInNPCTypeTable(zone, c, n);
		}
		case NPCSpawnTypes::DeleteSpawn: {
			return DeleteSpawnRemoveFromNPCTypeTable(zone, instance_version, c, n);
		}
		case NPCSpawnTypes::AddSpawnFromSpawngroup: {
			return AddSpawnFromSpawnGroup(zone, instance_version, c, n, extra);
		}
		case NPCSpawnTypes::CreateNewNPC: {
			return AddNPCTypes(zone, instance_version, c, n, extra);
		}
	}

	return false;
}

uint32 NPC::GetEquipmentMaterial(uint8 material_slot) const
{
	const uint32 texture_profile_material = GetTextureProfileMaterial(material_slot);

	Log(Logs::Detail, Logs::MobAppearance, "[%s] material_slot: %u",
		clean_name,
		material_slot
	);

	if (texture_profile_material) {
		return texture_profile_material;
	}

	if (material_slot >= EQ::textures::materialCount) {
		return 0;
	}

	int16 invslot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
	if (invslot == INVALID_INDEX) {
		return 0;
	}

	if (equipment[invslot] == 0) {
		switch (material_slot) {
			case EQ::textures::armorHead:
				return helmtexture;
			case EQ::textures::armorChest:
				return texture;
			case EQ::textures::armorArms:
				return armtexture;
			case EQ::textures::armorWrist:
				return bracertexture;
			case EQ::textures::armorHands:
				return handtexture;
			case EQ::textures::armorLegs:
				return legtexture;
			case EQ::textures::armorFeet:
				return feettexture;
			case EQ::textures::weaponPrimary:
				return d_melee_texture1;
			case EQ::textures::weaponSecondary:
				return d_melee_texture2;
			default:
				//they have nothing in the slot, and its not a special slot... they get nothing.
				return 0;
		}
	}

	//they have some loot item in this slot, pass it up to the default handler
	return Mob::GetEquipmentMaterial(material_slot);
}

uint32 NPC::GetMaxDamage(uint8 tlevel)
{
	uint64 dmg = 0;
	if (tlevel < 40)
		dmg = tlevel*2+2;
	else if (tlevel < 50)
		dmg = level*25/10+2;
	else if (tlevel < 60)
		dmg = (tlevel*3+2)+((tlevel-50)*30);
	else
		dmg = (tlevel*3+2)+((tlevel-50)*35);
	return dmg;
}

void NPC::PickPocket(Client* thief)
{
	thief->CheckIncreaseSkill(EQ::skills::SkillPickPockets, nullptr, 5);

	//make sure were allowed to target them:
	int over_level = GetLevel();
	if(over_level > (thief->GetLevel() + THIEF_PICKPOCKET_OVER)) {
		thief->Message(Chat::Red, "You are too inexperienced to pick pocket this target");
		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		//should we check aggro
		return;
	}

	if(zone->random.Roll(5)) {
		if (zone->CanDoCombat())
			AddToHateList(thief, 50);
		Say("Stop thief!");
		thief->Message(Chat::Red, "You are noticed trying to steal!");
		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		return;
	}

	int steal_skill = thief->GetSkill(EQ::skills::SkillPickPockets);
	int steal_chance = steal_skill * 100 / (5 * over_level + 5);

	// Determine whether to steal money or an item.
	uint32 money[6] = {0, ((steal_skill >= 125) ? (GetPlatinum()) : (0)), ((steal_skill >= 60) ? (GetGold()) : (0)), GetSilver(),
					   GetCopper(), 0 };
	bool has_coin = ((money[PickPocketPlatinum] | money[PickPocketGold] | money[PickPocketSilver] | money[PickPocketCopper]) != 0);
	bool steal_item = (steal_skill >= steal_chance && (zone->random.Roll(50) || !has_coin));

	// still needs to have FindFreeSlot vs PutItemInInventory issue worked out
	while (steal_item) {
		std::vector<std::pair<const EQ::ItemData*, uint16>> loot_selection; // <const ItemData*, charges>
		for (auto item_iter : m_loot_items) {
			if (!item_iter || !item_iter->item_id)
				continue;

			auto item_test = database.GetItem(item_iter->item_id);
			if (item_test->Magic || !item_test->NoDrop || item_test->IsClassBag() || thief->CheckLoreConflict(item_test) || item_iter->equip_slot != EQ::invslot::SLOT_INVALID)
				continue;

			loot_selection.emplace_back(std::make_pair(item_test, ((item_test->Stackable) ? (1) : (item_iter->charges))));
		}
		if (loot_selection.empty()) {
			steal_item = false;
			break;
		}

		int random = zone->random.Int(0, (loot_selection.size() - 1));
		uint16 slot_id = thief->GetInv().FindFreeSlot(false, true, (loot_selection[random].first->Size), (loot_selection[random].first->ItemType == EQ::item::ItemTypeArrow));
		if (slot_id == INVALID_INDEX) {
			steal_item = false;
			break;
		}

		auto item_inst = database.CreateItem(loot_selection[random].first, loot_selection[random].second);
		if (item_inst == nullptr) {
			steal_item = false;
			break;
		}

		// Successful item pickpocket
		if (item_inst->IsStackable() && RuleB(Character, UseStackablePickPocketing)) {
			if (!thief->TryStacking(item_inst, ItemPacketTrade, false, false)) {
				thief->PutItemInInventory(slot_id, *item_inst);
				thief->SendItemPacket(slot_id, item_inst, ItemPacketTrade);
			}
		}
		else {
			thief->PutItemInInventory(slot_id, *item_inst);
			thief->SendItemPacket(slot_id, item_inst, ItemPacketTrade);
		}
		RemoveItem(item_inst->GetID());
		thief->SendPickPocketResponse(this, 0, PickPocketItem, item_inst->GetItem());
		safe_delete(item_inst);

		return;
	}

	while (!steal_item && has_coin) {
		uint32 coin_amount = zone->random.Int(1, (steal_skill / 25) + 1);

		int coin_type = PickPocketPlatinum;
		while (coin_type <= PickPocketCopper) {
			if (money[coin_type]) {
				if (coin_amount > money[coin_type])
					coin_amount = money[coin_type];
				break;
			}
			++coin_type;
		}
		if (coin_type > PickPocketCopper)
			break;

		memset(money, 0, (sizeof(int) * 6));
		money[coin_type] = coin_amount;

		if (zone->random.Roll(steal_chance)) { // Successful coin pickpocket
			switch (coin_type) {
			case PickPocketPlatinum:
				SetPlatinum(GetPlatinum() - coin_amount);
				break;
			case PickPocketGold:
				SetGold(GetGold() - coin_amount);
				break;
			case PickPocketSilver:
				SetSilver(GetSilver() - coin_amount);
				break;
			case PickPocketCopper:
				SetCopper(GetCopper() - coin_amount);
				break;
			default: // has_coin..but, doesn't have coin?
				thief->SendPickPocketResponse(this, 0, PickPocketFailed);
				return;
			}

			thief->AddMoneyToPP(money[PickPocketCopper], money[PickPocketSilver], money[PickPocketGold], money[PickPocketPlatinum], false);
			thief->SendPickPocketResponse(this, coin_amount, coin_type);
			return;
		}

		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		return;
	}

	thief->Message(Chat::White, "This target's pockets are empty");
	thief->SendPickPocketResponse(this, 0, PickPocketFailed);
}

void NPC::Disarm(Client* client, int chance) {
	// disarm primary if available, otherwise disarm secondary
	const EQ::ItemData* weapon = NULL;
	uint8 eslot = 0xFF;
	if (equipment[EQ::invslot::slotPrimary] != 0)
		eslot = EQ::invslot::slotPrimary;
	else if (equipment[EQ::invslot::slotSecondary] != 0)
		eslot = EQ::invslot::slotSecondary;
	if (eslot != 0xFF) {
		if (zone->random.Int(0, 1000) <= chance) {
			weapon = database.GetItem(equipment[eslot]);
			if (weapon) {
				if (!weapon->Magic && weapon->NoDrop != 0) {
					int16               charges = -1;
					LootItems::iterator cur, end;
					cur = m_loot_items.begin();
					end = m_loot_items.end();
					// Get charges for the item in the loot table
					for (; cur != end; cur++) {
						LootItem * citem = *cur;
						if (citem->item_id == weapon->ID) {
							charges = citem->charges;
							break;
						}
					}
					EQ::ItemInstance *inst = NULL;
					inst = database.CreateItem(weapon->ID, charges);
					// Remove item from loot table
					RemoveItem(weapon->ID);
					CalcBonuses();
					if (inst) {
						// create a ground item
						Object* object = new Object(inst, GetX(), GetY(), GetZ(), 0.0f, 300000);
						entity_list.AddObject(object, true);
						object->StartDecay();
						safe_delete(inst);
					}
				}
			}
			// Update Appearance
			equipment[eslot] = 0;
			int matslot = eslot == EQ::invslot::slotPrimary ? EQ::textures::weaponPrimary : EQ::textures::weaponSecondary;
			if (matslot != -1)
				SendWearChange(matslot);
			if ((CastToMob()->GetBodyType() == BT_Humanoid || CastToMob()->GetBodyType() == BT_Summoned) && eslot == EQ::invslot::slotPrimary)
				Say("Ahh! My weapon!");
			client->MessageString(Chat::Skills, DISARM_SUCCESS, GetCleanName());
			if (chance != 1000)
				client->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 4);
			return;
		}
		client->MessageString(Chat::Skills, DISARM_FAILED);
		if (chance != 1000)
			client->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 2);
		return;
	}
	client->MessageString(Chat::Skills, DISARM_FAILED);
}

void Mob::NPCSpecialAttacks(const char* parse, int permtag, bool reset, bool remove) {
	if(reset)
	{
		ClearSpecialAbilities();
	}

	const char* orig_parse = parse;
	while (*parse)
	{
		switch(*parse)
		{
			case 'E':
				SetSpecialAbility(SPECATK_ENRAGE, remove ? 0 : 1);
				break;
			case 'F':
				SetSpecialAbility(SPECATK_FLURRY, remove ? 0 : 1);
				break;
			case 'R':
				SetSpecialAbility(SPECATK_RAMPAGE, remove ? 0 : 1);
				break;
			case 'r':
				SetSpecialAbility(SPECATK_AREA_RAMPAGE, remove ? 0 : 1);
				break;
			case 'S':
				if(remove) {
					SetSpecialAbility(SPECATK_SUMMON, 0);
					StopSpecialAbilityTimer(SPECATK_SUMMON);
				} else {
					SetSpecialAbility(SPECATK_SUMMON, 1);
				}
			break;
			case 'T':
				SetSpecialAbility(SPECATK_TRIPLE, remove ? 0 : 1);
				break;
			case 'Q':
				//quad requires triple to work properly
				if(remove) {
					SetSpecialAbility(SPECATK_QUAD, 0);
				} else {
					SetSpecialAbility(SPECATK_TRIPLE, 1);
					SetSpecialAbility(SPECATK_QUAD, 1);
					}
				break;
			case 'b':
				SetSpecialAbility(SPECATK_BANE, remove ? 0 : 1);
				break;
			case 'm':
				SetSpecialAbility(SPECATK_MAGICAL, remove ? 0 : 1);
				break;
			case 'U':
				SetSpecialAbility(UNSLOWABLE, remove ? 0 : 1);
				break;
			case 'M':
				SetSpecialAbility(UNMEZABLE, remove ? 0 : 1);
				break;
			case 'C':
				SetSpecialAbility(UNCHARMABLE, remove ? 0 : 1);
				break;
			case 'N':
				SetSpecialAbility(UNSTUNABLE, remove ? 0 : 1);
				break;
			case 'I':
				SetSpecialAbility(UNSNAREABLE, remove ? 0 : 1);
				break;
			case 'D':
				SetSpecialAbility(UNFEARABLE, remove ? 0 : 1);
				break;
			case 'K':
				SetSpecialAbility(UNDISPELLABLE, remove ? 0 : 1);
				break;
			case 'A':
				SetSpecialAbility(IMMUNE_MELEE, remove ? 0 : 1);
				break;
			case 'B':
				SetSpecialAbility(IMMUNE_MAGIC, remove ? 0 : 1);
				break;
			case 'f':
				SetSpecialAbility(IMMUNE_FLEEING, remove ? 0 : 1);
				break;
			case 'O':
				SetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE, remove ? 0 : 1);
				break;
			case 'W':
				SetSpecialAbility(IMMUNE_MELEE_NONMAGICAL, remove ? 0 : 1);
				break;
			case 'H':
				SetSpecialAbility(IMMUNE_AGGRO, remove ? 0 : 1);
				break;
			case 'G':
				SetSpecialAbility(IMMUNE_AGGRO_ON, remove ? 0 : 1);
				break;
			case 'g':
				SetSpecialAbility(IMMUNE_CASTING_FROM_RANGE, remove ? 0 : 1);
				break;
			case 'd':
				SetSpecialAbility(IMMUNE_FEIGN_DEATH, remove ? 0 : 1);
				break;
			case 'Y':
				SetSpecialAbility(SPECATK_RANGED_ATK, remove ? 0 : 1);
				break;
			case 'L':
				SetSpecialAbility(SPECATK_INNATE_DW, remove ? 0 : 1);
				break;
			case 't':
				SetSpecialAbility(NPC_TUNNELVISION, remove ? 0 : 1);
				break;
			case 'n':
				SetSpecialAbility(NPC_NO_BUFFHEAL_FRIENDS, remove ? 0 : 1);
				break;
			case 'p':
				SetSpecialAbility(IMMUNE_PACIFY, remove ? 0 : 1);
				break;
			case 'J':
				SetSpecialAbility(LEASH, remove ? 0 : 1);
				break;
			case 'j':
				SetSpecialAbility(TETHER, remove ? 0 : 1);
				break;
			case 'o':
				SetSpecialAbility(DESTRUCTIBLE_OBJECT, remove ? 0 : 1);
				SetDestructibleObject(remove ? true : false);
				break;
			case 'Z':
				SetSpecialAbility(NO_HARM_FROM_CLIENT, remove ? 0 : 1);
				break;
			case 'i':
				SetSpecialAbility(IMMUNE_TAUNT, remove ? 0 : 1);
				break;
			case 'e':
				SetSpecialAbility(ALWAYS_FLEE, remove ? 0 : 1);
				break;
			case 'h':
				SetSpecialAbility(FLEE_PERCENT, remove ? 0 : 1);
				break;

			default:
				break;
		}
		parse++;
	}

	if(permtag == 1 && GetNPCTypeID() > 0)
	{
		if(content_db.SetSpecialAttkFlag(GetNPCTypeID(), orig_parse))
		{
			LogInfo("NPCTypeID: [{}] flagged to [{}] for Special Attacks.\n",GetNPCTypeID(),orig_parse);
		}
	}
}

bool Mob::HasNPCSpecialAtk(const char* parse) {

	bool HasAllAttacks = true;

	while (*parse && HasAllAttacks == true)
	{
		switch(*parse)
		{
			case 'E':
				if (!GetSpecialAbility(SPECATK_ENRAGE))
					HasAllAttacks = false;
				break;
			case 'F':
				if (!GetSpecialAbility(SPECATK_FLURRY))
					HasAllAttacks = false;
				break;
			case 'R':
				if (!GetSpecialAbility(SPECATK_RAMPAGE))
					HasAllAttacks = false;
				break;
			case 'r':
				if (!GetSpecialAbility(SPECATK_AREA_RAMPAGE))
					HasAllAttacks = false;
				break;
			case 'S':
				if (!GetSpecialAbility(SPECATK_SUMMON))
					HasAllAttacks = false;
				break;
			case 'T':
				if (!GetSpecialAbility(SPECATK_TRIPLE))
					HasAllAttacks = false;
				break;
			case 'Q':
				if (!GetSpecialAbility(SPECATK_QUAD))
					HasAllAttacks = false;
				break;
			case 'b':
				if (!GetSpecialAbility(SPECATK_BANE))
					HasAllAttacks = false;
				break;
			case 'm':
				if (!GetSpecialAbility(SPECATK_MAGICAL))
					HasAllAttacks = false;
				break;
			case 'U':
				if (!GetSpecialAbility(UNSLOWABLE))
					HasAllAttacks = false;
				break;
			case 'M':
				if (!GetSpecialAbility(UNMEZABLE))
					HasAllAttacks = false;
				break;
			case 'C':
				if (!GetSpecialAbility(UNCHARMABLE))
					HasAllAttacks = false;
				break;
			case 'N':
				if (!GetSpecialAbility(UNSTUNABLE))
					HasAllAttacks = false;
				break;
			case 'I':
				if (!GetSpecialAbility(UNSNAREABLE))
					HasAllAttacks = false;
				break;
			case 'D':
				if (!GetSpecialAbility(UNFEARABLE))
					HasAllAttacks = false;
				break;
			case 'A':
				if (!GetSpecialAbility(IMMUNE_MELEE))
					HasAllAttacks = false;
				break;
			case 'B':
				if (!GetSpecialAbility(IMMUNE_MAGIC))
					HasAllAttacks = false;
				break;
			case 'f':
				if (!GetSpecialAbility(IMMUNE_FLEEING))
					HasAllAttacks = false;
				break;
			case 'O':
				if (!GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE))
					HasAllAttacks = false;
				break;
			case 'W':
				if (!GetSpecialAbility(IMMUNE_MELEE_NONMAGICAL))
					HasAllAttacks = false;
				break;
			case 'H':
				if (!GetSpecialAbility(IMMUNE_AGGRO))
					HasAllAttacks = false;
				break;
			case 'G':
				if (!GetSpecialAbility(IMMUNE_AGGRO_ON))
					HasAllAttacks = false;
				break;
			case 'g':
				if (!GetSpecialAbility(IMMUNE_CASTING_FROM_RANGE))
					HasAllAttacks = false;
				break;
			case 'd':
				if (!GetSpecialAbility(IMMUNE_FEIGN_DEATH))
					HasAllAttacks = false;
				break;
			case 'Y':
				if (!GetSpecialAbility(SPECATK_RANGED_ATK))
					HasAllAttacks = false;
				break;
			case 'L':
				if (!GetSpecialAbility(SPECATK_INNATE_DW))
					HasAllAttacks = false;
				break;
			case 't':
				if (!GetSpecialAbility(NPC_TUNNELVISION))
					HasAllAttacks = false;
				break;
			case 'n':
				if (!GetSpecialAbility(NPC_NO_BUFFHEAL_FRIENDS))
					HasAllAttacks = false;
				break;
			case 'p':
				if(!GetSpecialAbility(IMMUNE_PACIFY))
					HasAllAttacks = false;
				break;
			case 'J':
				if(!GetSpecialAbility(LEASH))
					HasAllAttacks = false;
				break;
			case 'j':
				if(!GetSpecialAbility(TETHER))
					HasAllAttacks = false;
				break;
			case 'o':
				if(!GetSpecialAbility(DESTRUCTIBLE_OBJECT))
				{
					HasAllAttacks = false;
					SetDestructibleObject(false);
				}
				break;
			case 'Z':
				if(!GetSpecialAbility(NO_HARM_FROM_CLIENT)){
					HasAllAttacks = false;
				}
				break;
			case 'e':
				if(!GetSpecialAbility(ALWAYS_FLEE))
					HasAllAttacks = false;
				break;
			case 'h':
				if(!GetSpecialAbility(FLEE_PERCENT))
					HasAllAttacks = false;
				break;
			default:
				HasAllAttacks = false;
				break;
		}
		parse++;
	}

	return HasAllAttacks;
}

void NPC::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho)
{
	Mob::FillSpawnStruct(ns, ForWho);
	PetOnSpawn(ns);
	ns->spawn.is_npc = 1;
	UpdateActiveLight();
	ns->spawn.light = GetActiveLightType();
	ns->spawn.show_name = NPCTypedata->show_name;
}

void NPC::PetOnSpawn(NewSpawn_Struct* ns)
{
	//Basic settings to make sure swarm pets work properly.
	Mob *swarm_owner = nullptr;
	if (GetSwarmOwner()) {
		swarm_owner = entity_list.GetMobID(GetSwarmOwner());
	}

	if (swarm_owner) {
		if (swarm_owner->IsClient()) {
			SetPetOwnerClient(true); //Simple flag to determine if pet belongs to a client
			SetAllowBeneficial(true);//Allow temp pets to receive buffs and heals if owner is client.
			//This will allow CLIENT swarm pets NOT to be targeted with F8.
			ns->spawn.targetable_with_hotkey = false;
			no_target_hotkey = true;
		} else {
			//NPC cast swarm pets should still be targetable with F8.
			ns->spawn.targetable_with_hotkey = true;
			no_target_hotkey = false;
		}

		SetTempPet(true); //Simple mob flag for checking if temp pet
		swarm_owner->SetTempPetsActive(true); //Necessary fail safe flag set if mob ever had a swarm pet to ensure they are removed.
		swarm_owner->SetTempPetCount(swarm_owner->GetTempPetCount() + 1);

		//Not recommended if using above (However, this will work better on older clients).
		if (RuleB(Pets, UnTargetableSwarmPet)) {
			ns->spawn.bodytype = BT_NoTarget;
		}

		if (
			!IsCharmed() &&
			swarm_owner->IsClient() &&
			RuleB(Pets, ClientPetsUseOwnerNameInLastName)
		) {
			const auto& tmp_lastname = fmt::format("{}'s Swarm", swarm_owner->GetName());
			if (tmp_lastname.size() < sizeof(ns->spawn.lastName)) {
				strn0cpy(ns->spawn.lastName, tmp_lastname.c_str(), sizeof(ns->spawn.lastName));
			}
		}

		if (swarm_owner->IsBot()) {
			SetPetOwnerBot(true);
		} else if (swarm_owner->IsNPC()) {
			SetPetOwnerNPC(true);
		}
	} else if (GetOwnerID()) {
		ns->spawn.is_pet = 1;

		if (!IsCharmed()) {
			const auto c = entity_list.GetClientByID(GetOwnerID());
			if (c) {
				SetPetOwnerClient(true);
				if (RuleB(Pets, ClientPetsUseOwnerNameInLastName)) {
					std::string tmp_lastname;				
					switch(ns->spawn.race) {
						case Race::AirElemental2:
						case Race::AirElemental:
						case Race::EarthElemental2:
						case Race::EarthElemental:
						case Race::WaterElemental2:
						case Race::WaterElemental:
						case Race::FireElemental2:
						case Race::FireElemental:
						case Race::Elemental:
							tmp_lastname = fmt::format("{}'s Elemental Minion", c->GetName());
							break;
						case Race::Skeleton:
						case Race::Skeleton2:
						case Race::Skeleton3:
							tmp_lastname = fmt::format("{}'s Skeletal Minion", c->GetName());
							break;
						case Race::Spectre:
						case Race::Spectre2:
							tmp_lastname = fmt::format("{}'s Reaper", c->GetName());
							break;
						default:
							tmp_lastname = fmt::format("{}'s Minion", c->GetName());
					}

					if (tmp_lastname.size() < sizeof(ns->spawn.lastName)) {
						strn0cpy(ns->spawn.lastName, tmp_lastname.c_str(), sizeof(ns->spawn.lastName));
					}
				}
			} else {
				Mob* owner = entity_list.GetMob(GetOwnerID());
				if (owner) {
					if (owner->IsBot()) {
						SetPetOwnerBot(true);
					} else if (owner->IsNPC()) {
						SetPetOwnerNPC(true);
					}
				}
			}
		}
	} else {
		ns->spawn.is_pet = 0;
	}
}

void NPC::SetLevel(uint8 in_level, bool command)
{
	if(in_level > level)
		SendLevelAppearance();
	level = in_level;
	SendAppearancePacket(AppearanceType::WhoLevel, in_level);
}

void NPC::ModifyNPCStat(const std::string& stat, const std::string& value)
{
	auto stat_lower = Strings::ToLower(stat);

	auto variable_key = fmt::format(
		"modify_stat_{}",
		stat_lower
	);

	SetEntityVariable(variable_key, value);

	LogNPCScaling("NPC::ModifyNPCStat: Key [{}] Value [{}] ", variable_key, value);

	if (stat_lower == "ac") {
		AC = Strings::ToInt(value);
		CalcAC();
		return;
	}
	else if (stat_lower == "str") {
		STR = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "sta") {
		STA = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "agi") {
		AGI = Strings::ToInt(value);
		CalcAC();
		return;
	}
	else if (stat_lower == "dex") {
		DEX = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "wis") {
		WIS = Strings::ToInt(value);
		CalcMaxMana();
		return;
	}
	else if (stat_lower == "int" || stat_lower == "_int") {
		INT = Strings::ToInt(value);
		CalcMaxMana();
		return;
	}
	else if (stat_lower == "cha") {
		CHA = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "max_hp") {
		base_hp = Strings::ToBigInt(value);

		CalcMaxHP();
		if (current_hp > max_hp) {
			current_hp = max_hp;
		}

		return;
	}
	else if (stat_lower == "max_mana") {
		npc_mana = Strings::ToUnsignedBigInt(value);
		CalcMaxMana();
		if (current_mana > max_mana) {
			current_mana = max_mana;
		}
		return;
	}
	else if (stat_lower == "mr") {
		MR = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "fr") {
		FR = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "cr") {
		CR = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "cor") {
		Corrup = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "pr") {
		PR = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "dr") {
		DR = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "phr") {
		PhR = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "runspeed") {
		runspeed       = Strings::ToFloat(value);
		base_runspeed  = (int) (runspeed * 40.0f);
		base_walkspeed = base_runspeed * 100 / 265;
		walkspeed      = ((float) base_walkspeed) * 0.025f;
		base_fearspeed = base_runspeed * 100 / 127;
		fearspeed      = ((float) base_fearspeed) * 0.025f;
		CalcBonuses();
		return;
	}
	else if (stat_lower == "special_attacks") {
		NPCSpecialAttacks(value.c_str(), 0, true);
		return;
	}
	else if (stat_lower == "special_abilities") {
		ProcessSpecialAbilities(value);
		return;
	}
	else if (stat_lower == "attack_speed") {
		attack_speed = Strings::ToFloat(value);
		CalcBonuses();
		return;
	}
	else if (stat_lower == "attack_delay") {
		/* TODO: fix DB */
		attack_delay = Strings::ToInt(value) * 100;
		CalcBonuses();
		return;
	}
	else if (stat_lower == "atk") {
		ATK = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "accuracy") {
		accuracy_rating = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "avoidance") {
		avoidance_rating = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "trackable") {
		trackable = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "min_hit") {
		min_dmg = Strings::ToInt(value);
		// Clamp max_dmg to be >= min_dmg
		max_dmg = std::max(min_dmg, max_dmg);
		base_damage = round((max_dmg - min_dmg) / 1.9);
		min_damage  = min_dmg - round(base_damage / 10.0);
		return;
	}
	else if (stat_lower == "max_hit") {
		max_dmg = Strings::ToInt(value);
		// Clamp min_dmg to be <= max_dmg
		min_dmg = std::min(min_dmg, max_dmg);
		base_damage = round((max_dmg - min_dmg) / 1.9);
		min_damage  = min_dmg - round(base_damage / 10.0);
		return;
	}
	else if (stat_lower == "attack_count") {
		attack_count = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "see_invis") {
		see_invis = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "see_invis_undead") {
		see_invis_undead = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "see_hide") {
		see_hide = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "see_improved_hide") {
		see_improved_hide = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "hp_regen") {
		hp_regen = Strings::ToBigInt(value);
		return;
	}
	else if (stat_lower == "hp_regen_per_second") {
		hp_regen_per_second = Strings::ToBigInt(value);
		return;
	}
	else if (stat_lower == "mana_regen") {
		mana_regen = Strings::ToBigInt(value);
		return;
	}
	else if (stat_lower == "level") {
		SetLevel(Strings::ToInt(value));
		return;
	}
	else if (stat_lower == "aggro") {
		pAggroRange = Strings::ToFloat(value);
		return;
	}
	else if (stat_lower == "assist") {
		pAssistRange = Strings::ToFloat(value);
		return;
	}
	else if (stat_lower == "slow_mitigation") {
		slow_mitigation = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "loottable_id") {
		m_loottable_id = Strings::ToFloat(value);
		return;
	}
	else if (stat_lower == "healscale") {
		healscale = Strings::ToFloat(value);
		return;
	}
	else if (stat_lower == "spellscale") {
		spellscale = Strings::ToFloat(value);
		return;
	}
	else if (stat_lower == "npc_spells_id") {
		AI_AddNPCSpells(Strings::ToInt(value));
		return;
	}
	else if (stat_lower == "npc_spells_effects_id") {
		AI_AddNPCSpellsEffects(Strings::ToInt(value));
		CalcBonuses();
		return;
	}
	else if (stat_lower == "heroic_strikethrough") {
		heroic_strikethrough = Strings::ToInt(value);
		return;
	}
	else if (stat_lower == "keeps_sold_items") {
		SetKeepsSoldItems(Strings::ToBool(value));
		return;
	}
}

float NPC::GetNPCStat(const std::string& stat)
{

	if (auto stat_lower = Strings::ToLower(stat); stat_lower == "ac") {
		return AC;
	}
	else if (stat_lower == "str") {
		return STR;
	}
	else if (stat_lower == "sta") {
		return STA;
	}
	else if (stat_lower == "agi") {
		return AGI;
	}
	else if (stat_lower == "dex") {
		return DEX;
	}
	else if (stat_lower == "wis") {
		return WIS;
	}
	else if (stat_lower == "int" || stat_lower == "_int") {
		return INT;
	}
	else if (stat_lower == "cha") {
		return CHA;
	}
	else if (stat_lower == "max_hp") {
		return base_hp;
	}
	else if (stat_lower == "max_mana") {
		return npc_mana;
	}
	else if (stat_lower == "mr") {
		return MR;
	}
	else if (stat_lower == "fr") {
		return FR;
	}
	else if (stat_lower == "cr") {
		return CR;
	}
	else if (stat_lower == "cor") {
		return Corrup;
	}
	else if (stat_lower == "phr") {
		return PhR;
	}
	else if (stat_lower == "pr") {
		return PR;
	}
	else if (stat_lower == "dr") {
		return DR;
	}
	else if (stat_lower == "runspeed") {
		return runspeed;
	}
	else if (stat_lower == "attack_speed") {
		return attack_speed;
	}
	else if (stat_lower == "attack_delay") {
		return attack_delay;
	}
	else if (stat_lower == "atk") {
		return ATK;
	}
	else if (stat_lower == "accuracy") {
		return accuracy_rating;
	}
	else if (stat_lower == "avoidance") {
		return avoidance_rating;
	}
	else if (stat_lower == "trackable") {
		return trackable;
	}
	else if (stat_lower == "min_hit") {
		return min_dmg;
	}
	else if (stat_lower == "max_hit") {
		return max_dmg;
	}
	else if (stat_lower == "attack_count") {
		return attack_count;
	}
	else if (stat_lower == "see_invis") {
		return see_invis;
	}
	else if (stat_lower == "see_invis_undead") {
		return see_invis_undead;
	}
	else if (stat_lower == "see_hide") {
		return see_hide;
	}
	else if (stat_lower == "see_improved_hide") {
		return see_improved_hide;
	}
	else if (stat_lower == "hp_regen") {
		return hp_regen;
	}
	else if (stat_lower == "hp_regen_per_second") {
		return hp_regen_per_second;
	}
	else if (stat_lower == "mana_regen") {
		return mana_regen;
	}
	else if (stat_lower == "level") {
		return GetOrigLevel();
	}
	else if (stat_lower == "aggro") {
		return pAggroRange;
	}
	else if (stat_lower == "assist") {
		return pAssistRange;
	}
	else if (stat_lower == "slow_mitigation") {
		return slow_mitigation;
	}
	else if (stat_lower == "loottable_id") {
		return m_loottable_id;
	}
	else if (stat_lower == "healscale") {
		return healscale;
	}
	else if (stat_lower == "spellscale") {
		return spellscale;
	}
	else if (stat_lower == "npc_spells_id") {
		return npc_spells_id;
	}
	else if (stat_lower == "npc_spells_effects_id") {
		return npc_spells_effects_id;
	}
	else if (stat_lower == "heroic_strikethrough") {
		return heroic_strikethrough;
	}
	else if (stat_lower == "keeps_sold_items") {
		return keeps_sold_items;
	}
	//default values
	else if (stat_lower == "default_ac") {
		return default_ac;
	}
	else if (stat_lower == "default_min_hit") {
		return default_min_dmg;
	}
	else if (stat_lower == "default_max_hit") {
		return default_max_dmg;
	}
	else if (stat_lower == "default_attack_delay") {
		return default_attack_delay;
	}
	else if (stat_lower == "default_accuracy") {
		return default_accuracy_rating;
	}
	else if (stat_lower == "default_avoidance") {
		return default_avoidance_rating;
	}
	else if (stat_lower == "default_atk") {
		return default_atk;
	}

	return 0.0f;
}

void NPC::LevelScale() {

	uint8 random_level = (zone->random.Int(level, maxlevel));

	float scaling = (((random_level / (float)level) - 1) * (scalerate / 100.0f));

	if (RuleB(NPC, NewLevelScaling)) {
		if (scalerate == 0 || maxlevel <= 25) {
			// Don't add HP to dynamically scaled NPCs since this will be calculated later
			if (max_hp > 0 || m_skip_auto_scale)
			{
				// pre-pop seems to scale by 20 HP increments while newer by 100
				// We also don't want 100 increments on newer noobie zones, check level
				if (zone->GetZoneID() < 200 || level < 48) {
					max_hp += (random_level - level) * 20;
					base_hp += (random_level - level) * 20;
				} else {
					max_hp += (random_level - level) * 100;
					base_hp += (random_level - level) * 100;
				}

				current_hp = max_hp;
			}

			// Don't add max_dmg to dynamically scaled NPCs since this will be calculated later
			if (max_dmg > 0 || m_skip_auto_scale)
			{
				max_dmg += (random_level - level) * 2;
			}
		} else {
			uint8 scale_adjust = 1;

			base_hp += (int64)(base_hp * scaling);
			max_hp += (int64)(max_hp * scaling);
			current_hp = max_hp;

			if (max_dmg) {
				max_dmg += (int64)(max_dmg * scaling / scale_adjust);
				min_dmg += (int64)(min_dmg * scaling / scale_adjust);
			}

			STR += (int)(STR * scaling / scale_adjust);
			STA += (int)(STA * scaling / scale_adjust);
			AGI += (int)(AGI * scaling / scale_adjust);
			DEX += (int)(DEX * scaling / scale_adjust);
			INT += (int)(INT * scaling / scale_adjust);
			WIS += (int)(WIS * scaling / scale_adjust);
			CHA += (int)(CHA * scaling / scale_adjust);
			if (MR)
				MR += (int)(MR * scaling / scale_adjust);
			if (CR)
				CR += (int)(CR * scaling / scale_adjust);
			if (DR)
				DR += (int)(DR * scaling / scale_adjust);
			if (FR)
				FR += (int)(FR * scaling / scale_adjust);
			if (PR)
				PR += (int)(PR * scaling / scale_adjust);
		}
	} else {
		// Compensate for scale rates at low levels so they don't add too much
		uint8 scale_adjust = 1;
		if(level > 0 && level <= 5)
			scale_adjust = 10;
		if(level > 5 && level <= 10)
			scale_adjust = 5;
		if(level > 10 && level <= 15)
			scale_adjust = 3;
		if(level > 15 && level <= 25)
			scale_adjust = 2;

		AC += (int)(AC * scaling);
		ATK += (int)(ATK * scaling);
		base_hp += (int64)(base_hp * scaling);
		max_hp += (int64)(max_hp * scaling);
		current_hp = max_hp;
		STR += (int)(STR * scaling / scale_adjust);
		STA += (int)(STA * scaling / scale_adjust);
		AGI += (int)(AGI * scaling / scale_adjust);
		DEX += (int)(DEX * scaling / scale_adjust);
		INT += (int)(INT * scaling / scale_adjust);
		WIS += (int)(WIS * scaling / scale_adjust);
		CHA += (int)(CHA * scaling / scale_adjust);
		if (MR)
			MR += (int)(MR * scaling / scale_adjust);
		if (CR)
			CR += (int)(CR * scaling / scale_adjust);
		if (DR)
			DR += (int)(DR * scaling / scale_adjust);
		if (FR)
			FR += (int)(FR * scaling / scale_adjust);
		if (PR)
			PR += (int)(PR * scaling / scale_adjust);

		if (max_dmg)
		{
			max_dmg += (int64)(max_dmg * scaling / scale_adjust);
			min_dmg += (int64)(min_dmg * scaling / scale_adjust);
		}

	}
	level = random_level;

}

uint32 NPC::GetSpawnPointID() const
{
	if (respawn2) {
		return respawn2->GetID();
	}
	return 0;
}

void NPC::NPCSlotTexture(uint8 slot, uint32 texture)
{
	if (slot == EQ::textures::TextureSlot::weaponPrimary) {
		d_melee_texture1 = texture;
	} else if (slot == EQ::textures::TextureSlot::weaponSecondary) {
		d_melee_texture2 = texture;
	} else {
		// Reserved for texturing individual armor slots
	}
}

uint32 NPC::GetSwarmOwner()
{
	if(GetSwarmInfo() != nullptr)
	{
		return GetSwarmInfo()->owner_id;
	}
	return 0;
}

uint32 NPC::GetSwarmTarget()
{
	if(GetSwarmInfo() != nullptr)
	{
		return GetSwarmInfo()->target;
	}
	return 0;
}

void NPC::SetSwarmTarget(int target_id)
{
	if(GetSwarmInfo() != nullptr)
	{
		GetSwarmInfo()->target = target_id;
	}
}

int64 NPC::CalcMaxMana()
{
	if (npc_mana == 0) {
		if (IsIntelligenceCasterClass()) {
			max_mana = (((GetINT() / 2) + 1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
		} else if (IsWisdomCasterClass()) {
			max_mana = (((GetWIS() / 2) + 1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
		} else {
			max_mana = 0;
		}

		if (max_mana < 0) {
			max_mana = 0;
		}

		return max_mana;
	} else {
		if (IsIntelligenceCasterClass()) {
			max_mana = npc_mana + spellbonuses.Mana + itembonuses.Mana;
		} else if (IsWisdomCasterClass()) {
			max_mana = npc_mana + spellbonuses.Mana + itembonuses.Mana;
		} else {
			max_mana = 0;
		}

		if (max_mana < 0) {
			max_mana = 0;
		}

		return max_mana;
	}
}

void NPC::SignalNPC(int _signal_id)
{
	signal_q.push_back(_signal_id);
}

void NPC::SendPayload(int payload_id, std::string payload_value)
{
	if (parse->HasQuestSub(GetNPCTypeID(), EVENT_PAYLOAD)) {
		const auto& export_string = fmt::format("{} {}", payload_id, payload_value);

		parse->EventNPC(EVENT_PAYLOAD, this, nullptr, export_string, 0);
	}
}

NPC_Emote_Struct* NPC::GetNPCEmote(uint32 emote_id, uint8 event_) {
	std::vector<NPC_Emote_Struct*> emotes;

	for (auto& e : zone->npc_emote_list) {
		if (e->emoteid == emote_id && e->event_ == event_) {
			emotes.emplace_back(e);
		}
	}

	if (emotes.empty()) {
		return nullptr;
	} else if (emotes.size() == 1) {
		return emotes.front();
	}

	const int index = zone->random.Roll0(emotes.size());

	return emotes[index];
}

void NPC::DoNPCEmote(uint8 event_, uint32 emote_id, Mob* t)
{
	if (!emote_id) {
		return;
	}

	const auto& e = GetNPCEmote(emote_id, event_);
	if (!e) {
		return;
	}

	std::string processed = e->text;

	// Mob Variables
	Strings::FindReplace(processed, "$mname", GetCleanName());
	Strings::FindReplace(processed, "$mracep", GetRacePlural());
	Strings::FindReplace(processed, "$mrace", GetPlayerRaceName(GetRace()));
	Strings::FindReplace(processed, "$mclass", GetClassIDName(GetClass()));
	Strings::FindReplace(processed, "$mclassp", GetClassPlural());

	// Target Variables
	Strings::FindReplace(processed, "$name", t ? t->GetCleanName() : "foe");
	Strings::FindReplace(processed, "$class", t ? GetClassIDName(t->GetClass()) : "class");
	Strings::FindReplace(processed, "$classp", t ? t->GetClassPlural() : "classes");
	Strings::FindReplace(processed, "$race", t ? GetPlayerRaceName(t->GetRace()) : "race");
	Strings::FindReplace(processed, "$racep", t ? t->GetRacePlural() : "races");

	if (emoteid == e->emoteid) {
		if (event_ == EQ::constants::EmoteEventTypes::Hailed && t) {
			DoQuestPause(t);
		}

		if (e->type == EQ::constants::EmoteTypes::Say) {
			Say(processed.c_str());
		} else if (e->type == EQ::constants::EmoteTypes::Emote) {
			Emote(processed.c_str());
		} else if (e->type == EQ::constants::EmoteTypes::Shout) {
			Shout(processed.c_str());
		} else if (e->type == EQ::constants::EmoteTypes::Proximity) {
			entity_list.MessageCloseString(
				this,
				true,
				200,
				Chat::NPCQuestSay,
				GENERIC_STRING,
				processed.c_str()
			);
		}
	}
}

bool NPC::CanTalk()
{
	//Races that should be able to talk. (Races up to Titanium)

	uint16 TalkRace[473] =
	{1,2,3,4,5,6,7,8,9,10,11,12,0,0,15,16,0,18,19,20,0,0,23,0,25,0,0,0,0,0,0,
	32,0,0,0,0,0,0,39,40,0,0,0,44,0,0,0,0,49,0,51,0,53,54,55,56,57,58,0,0,0,
	62,0,64,65,66,67,0,0,70,71,0,0,0,0,0,77,78,79,0,81,82,0,0,0,86,0,0,0,90,
	0,92,93,94,95,0,0,98,99,0,101,0,103,0,0,0,0,0,0,110,111,112,0,0,0,0,0,0,
	0,0,0,0,123,0,0,126,0,128,0,130,131,0,0,0,0,136,137,0,139,140,0,0,0,144,
	0,0,0,0,0,150,151,152,153,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,183,184,0,0,187,188,189,0,0,0,0,0,195,196,0,198,0,0,0,202,0,
	0,205,0,0,208,0,0,0,0,0,0,0,0,217,0,219,0,0,0,0,0,0,226,0,0,229,230,0,0,
	0,0,235,236,0,238,239,240,241,242,243,244,0,246,247,0,0,0,251,0,0,254,255,
	256,257,0,0,0,0,0,0,0,0,266,267,0,0,270,271,0,0,0,0,0,277,278,0,0,0,0,283,
	284,0,286,0,288,289,290,0,0,0,0,295,296,297,298,299,300,0,0,0,304,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,320,0,322,323,324,325,0,0,0,0,330,331,332,333,334,335,
	336,337,338,339,340,341,342,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,359,360,361,362,
	0,364,365,366,0,368,369,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,385,386,0,0,0,0,0,392,
	393,394,395,396,397,398,0,400,402,0,0,0,0,406,0,408,0,0,411,0,413,0,0,0,417,
	0,0,420,0,0,0,0,425,0,0,0,0,0,0,0,433,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,458,0,0,0,0,0,0,0,0,467,0,0,470,0,0,473};

	if (TalkRace[GetRace() - 1] > 0)
		return true;

	return false;
}

//this is called with 'this' as the mob being looked at, and
//iOther the mob who is doing the looking. It should figure out
//what iOther thinks about 'this'
FACTION_VALUE NPC::GetReverseFactionCon(Mob* iOther) {
	iOther = iOther->GetOwnerOrSelf();
	int primaryFaction= iOther->GetPrimaryFaction();

	//I am pretty sure that this special faction call is backwards
	//and should be iOther->GetSpecialFactionCon(this)
	if (primaryFaction < 0)
		return GetSpecialFactionCon(iOther);

	if (primaryFaction == 0)
		return FACTION_INDIFFERENTLY;

	//if we are a pet, use our owner's faction stuff
	Mob *own = GetOwner();
	if (own != nullptr)
		return own->GetReverseFactionCon(iOther);

	//make sure iOther is an npc
	//also, if we dont have a faction, then they arnt gunna think anything of us either
	if(!iOther->IsNPC() || GetPrimaryFaction() == 0)
		return(FACTION_INDIFFERENTLY);

	//if we get here, iOther is an NPC too

	//otherwise, employ the npc faction stuff
	//so we need to look at iOther's faction table to see
	//what iOther thinks about our primary faction
	return(iOther->CastToNPC()->CheckNPCFactionAlly(GetPrimaryFaction()));
}

//Look through our faction list and return a faction con based
//on the npc_value for the other person's primary faction in our list.
FACTION_VALUE NPC::CheckNPCFactionAlly(int32 other_faction)
{
	for (const auto& e : faction_list) {
		if (e.faction_id == other_faction) {
			if (e.npc_value > 0) {
				return FACTION_ALLY;
			} else if (e.npc_value < 0) {
				return FACTION_SCOWLS;
			} else {
				return FACTION_INDIFFERENTLY;
			}
		}
	}

	// I believe that the assumption is, barring no entry in npc_faction_entries
	// that two npcs on like faction con ally to each other.  This catches cases
	// where an npc is on a faction but has no hits (hence no entry in
	// npc_faction_entries).

	if (GetPrimaryFaction() == other_faction) {
		return FACTION_ALLY;
	} else {
		return FACTION_INDIFFERENTLY;
	}
}

bool NPC::IsFactionListAlly(uint32 other_faction)
{
	return CheckNPCFactionAlly(other_faction) == FACTION_ALLY;
}

int NPC::GetScore()
{
    int lv = std::min(70, (int)GetLevel());
    int basedmg = (lv*2)*(1+(lv / 100)) - (lv / 2);
    int minx = 0;
    int64 basehp = 0;
    int hpcontrib = 0;
    int dmgcontrib = 0;
    int spccontrib = 0;
    int64 hp = GetMaxHP();
    int mindmg = min_dmg;
    int maxdmg = max_dmg;
    int final;

    if(lv < 46)
    {
		minx = static_cast<int> (ceil( ((lv - (lv / 10.0)) - 1.0) ));
		basehp = (lv * 10) + (lv * lv);
	}
	else
	{
		minx = static_cast<int> (ceil( ((lv - (lv / 10.0)) - 1.0) - (( lv - 45.0 ) / 2.0) ));
        basehp = (lv * 10) + ((lv * lv) * 4);
    }

    if(hp > basehp)
    {
        hpcontrib = static_cast<int> (((hp / static_cast<float> (basehp)) * 1.5));
        if(hpcontrib > 5) { hpcontrib = 5; }

        if(maxdmg > basedmg)
        {
            dmgcontrib = static_cast<int> (ceil( ((maxdmg / basedmg) * 1.5) ));
        }

        if(HasNPCSpecialAtk("E")) { spccontrib++; }    //Enrage
        if(HasNPCSpecialAtk("F")) { spccontrib++; }    //Flurry
        if(HasNPCSpecialAtk("R")) { spccontrib++; }    //Rampage
        if(HasNPCSpecialAtk("r")) { spccontrib++; }    //Area Rampage
        if(HasNPCSpecialAtk("S")) { spccontrib++; }    //Summon
        if(HasNPCSpecialAtk("T")) { spccontrib += 2; } //Triple
        if(HasNPCSpecialAtk("Q")) { spccontrib += 3; } //Quad
        if(HasNPCSpecialAtk("U")) { spccontrib += 5; } //Unslowable
        if(HasNPCSpecialAtk("L")) { spccontrib++; }    //Innate Dual Wield
    }

    if(npc_spells_id > 12)
	{
        if(lv < 16)
            spccontrib++;
        else
            spccontrib += static_cast<int> (floor(lv/15.0));
    }

    final = minx + hpcontrib + dmgcontrib + spccontrib;
    final = std::max(1, final);
    final = std::min(100, final);
    return(final);
}

uint32 NPC::GetSpawnKillCount()
{
	uint32 sid = GetSpawnPointID();

	if(sid > 0)
	{
		return(zone->GetSpawnKillCount(sid));
	}

	return(0);
}

void NPC::DoQuestPause(Mob *other) {
	if(IsMoving() && !IsOnHatelist(other)) {
		PauseWandering(RuleI(NPC, SayPauseTimeInSec));
		if (other && !other->sneaking)
			FaceTarget(other);
	} else if(!IsMoving()) {
		if (other && !other->sneaking && GetAppearance() != eaSitting && GetAppearance() != eaDead)
			FaceTarget(other);
	}

}

void NPC::ChangeLastName(std::string last_name)
{
	auto outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
	auto gmn = (GMLastName_Struct*) outapp->pBuffer;

	strn0cpy(gmn->name, GetName(), sizeof(gmn->name));
	strn0cpy(gmn->gmname, GetName(), sizeof(gmn->gmname));
	strn0cpy(gmn->lastname, last_name.c_str(), sizeof(gmn->lastname));

	gmn->unknown[0] = 1;
	gmn->unknown[1] = 1;
	gmn->unknown[2] = 1;
	gmn->unknown[3] = 1;

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);
}

void NPC::ClearLastName()
{
	std::string empty;
	ChangeLastName(empty);
}

void NPC::DepopSwarmPets()
{
	if (GetSwarmInfo()) {
		if (GetSwarmInfo()->duration->Check(false)){
			Mob* owner = entity_list.GetMobID(GetSwarmInfo()->owner_id);
			if (owner) {
				owner->SetTempPetCount(owner->GetTempPetCount() - 1);
			}
			Depop();
			return;
		}
	}
}

void NPC::ModifyStatsOnCharm(bool is_charm_removed)
{
	if (is_charm_removed) {
		if (charm_ac) {
			AC = default_ac;
		}
		if (charm_attack_delay) {
			attack_delay = default_attack_delay;
		}
		if (charm_accuracy_rating) {
			accuracy_rating = default_accuracy_rating;
		}
		if (charm_avoidance_rating) {
			avoidance_rating = default_avoidance_rating;
		}
		if (charm_atk) {
			ATK = default_atk;
		}
		if (charm_min_dmg || charm_max_dmg) {
			base_damage = round((default_max_dmg - default_min_dmg) / 1.9);
			min_damage  = default_min_dmg - round(base_damage / 10.0);
		}
		if (RuleB(Spells, CharmDisablesSpecialAbilities)) {
			ProcessSpecialAbilities(default_special_abilities);
		}

		SetAttackTimer();
		CalcAC();

		return;
	}

	if (charm_ac) {
		AC = charm_ac;
	}
	if (charm_attack_delay) {
		attack_delay = charm_attack_delay;
	}
	if (charm_accuracy_rating) {
		accuracy_rating = charm_accuracy_rating;
	}
	if (charm_avoidance_rating) {
		avoidance_rating = charm_avoidance_rating;
	}
	if (charm_atk) {
		ATK = charm_atk;
	}
	if (charm_min_dmg || charm_max_dmg) {
		base_damage = round((charm_max_dmg - charm_min_dmg) / 1.9);
		min_damage  = charm_min_dmg - round(base_damage / 10.0);
	}
	if (RuleB(Spells, CharmDisablesSpecialAbilities)) {
		ClearSpecialAbilities();
	}

	// the rest of the stats aren't cached, so lets just do these two instead of full CalcBonuses()
	SetAttackTimer();
	CalcAC();
}

uint16 NPC::GetMeleeTexture1() const
{
	return d_melee_texture1;
}

uint16 NPC::GetMeleeTexture2() const
{
	return d_melee_texture2;
}

float NPC::GetProximityMinX()
{
	return proximity->min_x;
}

float NPC::GetProximityMaxX()
{
	return proximity->max_x;
}

float NPC::GetProximityMinY()
{
	return proximity->min_y;
}

float NPC::GetProximityMaxY()
{
	return proximity->max_y;
}

float NPC::GetProximityMinZ()
{
	return proximity->min_z;
}

float NPC::GetProximityMaxZ()
{
	return proximity->max_z;
}

bool NPC::IsProximitySet()
{
	if (proximity && proximity->proximity_set) {
		return proximity->proximity_set;
	}

	return false;
}

/**
 * @param box_size
 * @param move_distance
 * @param move_delay
 */
void NPC::SetSimpleRoamBox(float box_size, float move_distance, int move_delay)
{
	AI_SetRoambox(
		(move_distance != 0 ? move_distance : box_size / 2),
		GetX() + box_size,
		GetX() - box_size,
		GetY() + box_size,
		GetY() - box_size,
		move_delay
	);
}

/**
 * @param caster
 * @param chance
 * @param cast_range
 * @param spell_types
 * @return
 */
bool NPC::AICheckCloseBeneficialSpells(
	NPC *caster,
	uint8 chance,
	float cast_range,
	uint32 spell_types
)
{
	if((spell_types & SPELL_TYPES_DETRIMENTAL) != 0) {
		LogError("Detrimental spells requested from AICheckCloseBeneficialSpells!");
		return false;
	}

	if (!caster) {
		return false;
	}

	if (!caster->AI_HasSpells()) {
		return false;
	}

	if (caster->GetSpecialAbility(NPC_NO_BUFFHEAL_FRIENDS)) {
		return false;
	}

	if (chance < 100) {
		uint8 tmp = zone->random.Int(0, 99);
		if (tmp >= chance) {
			return false;
		}
	}

	/**
	 * Indifferent
	 */
	if (caster->GetPrimaryFaction() == 0) {
		return false;
	}

	/**
	 * Check through close range mobs
	 */
	for (auto & close_mob : entity_list.GetCloseMobList(caster, cast_range)) {
		Mob *mob = close_mob.second;
		if (!mob) {
			continue;
		}

		if (mob->IsClient()) {
			continue;
		}

		float distance = Distance(mob->GetPosition(), caster->GetPosition());
		if (distance > cast_range) {
			continue;
		}

		if (!mob->CheckLosFN(caster)) {
			continue;
		}

		if (mob->GetReverseFactionCon(caster) >= FACTION_KINDLY) {
			continue;
		}

		LogAICastBeneficialClose(
			"NPC [{}] Distance [{}] Cast Range [{}] Caster [{}]",
			mob->GetCleanName(),
			distance,
			cast_range,
			caster->GetCleanName()
		);

		if ((spell_types & SpellType_Buff) && !RuleB(NPC, BuffFriends)) {
			if (mob != caster) {
				spell_types = SpellType_Heal;
			}
		}

		if (caster->AICastSpell(mob, 100, spell_types)) {
			return true;
		}
	}

	return false;
}

/**
 * @param sender
 * @param attacker
 */
void NPC::AIYellForHelp(Mob *sender, Mob *attacker)
{
	LogAIYellForHelp("Mob[{}] Target[{}]",
		(sender == nullptr ? "NULL MOB" : GetCleanName()),
		(attacker == nullptr ? "NULL TARGET" : attacker->GetCleanName())
		);

	if (!sender || !attacker) {
		return;
	}

	/**
	 * If we dont have a faction set, we're gonna be indiff to everybody
	 */
	if (sender->GetPrimaryFaction() == 0) {
		LogAIYellForHelp("No Primary Faction");
		return;
	}

	if (sender->HasAssistAggro()) {
		LogAIYellForHelp("I have assist aggro");
		return;
	}

	LogAIYellForHelp(
		"NPC [{}] ID [{}] is starting to scan",
		GetCleanName(),
		GetID()
	);

	for (auto &close_mob : entity_list.GetCloseMobList(sender)) {
		Mob   *mob     = close_mob.second;
		if (!mob) {
			continue;
		}

		float distance = DistanceSquared(m_Position, mob->GetPosition());

		if (mob->IsClient()) {
			continue;
		}

		float assist_range = (mob->GetAssistRange() * mob->GetAssistRange());

		// Implement optional sneak-pull
		if (RuleB(Combat, EnableSneakPull) && attacker->sneaking) {
			assist_range = RuleI(Combat, SneakPullAssistRange);
			if (attacker->IsClient()) {
				float clientx = attacker->GetX();
				float clienty = attacker->GetY();
				if (attacker->CastToClient()->BehindMob(mob, clientx, clienty)) {
					assist_range = 0;
				}
			}
		}

		if (distance > assist_range) {
			continue;
		}

		LogAIYellForHelpDetail(
			"NPC [{}] ID [{}] is scanning - checking against NPC [{}] range [{}] dist [{}] in_range [{}]",
			GetCleanName(),
			GetID(),
			mob->GetCleanName(),
			assist_range,
			distance,
			(distance < assist_range)
		);

		if (mob->CheckAggro(attacker)) {
			continue;
		}

		if (sender->NPCAssistCap() >= RuleI(Combat, NPCAssistCap)) {
			break;
		}

		if (
			mob != sender
			&& mob != attacker
			&& mob->GetPrimaryFaction() != 0
			&& !mob->IsEngaged()
			&& ((!mob->IsPet()) || (mob->IsPet() && mob->GetOwner() && !mob->GetOwner()->IsClient()))
			) {

			/**
			 * if they are in range, make sure we are not green...
			 * then jump in if they are our friend
			 */
			if (mob->GetLevel() >= 50 || mob->AlwaysAggro() || attacker->GetLevelCon(mob->GetLevel()) != CON_GRAY) {
				if (mob->GetPrimaryFaction() == sender->CastToNPC()->GetPrimaryFaction()) {
					const auto f = zone->GetNPCFaction(mob->CastToNPC()->GetNPCFactionID());
					if (f) {
						if (f->ignore_primary_assist) {
							continue; //Same faction and ignore primary assist
						}
					}
				}

				if (sender->GetReverseFactionCon(mob) <= FACTION_AMIABLY) {
					//attacking someone on same faction, or a friend
					//Father Nitwit: make sure we can see them.
					if (mob->CheckLosFN(sender)) {
						mob->AddToHateList(attacker, 25, 0, false);
						sender->AddAssistCap();

						LogAIYellForHelpDetail(
							"NPC [{}] is assisting [{}] against target [{}]",
							mob->GetCleanName(),
							GetCleanName(),
							attacker->GetCleanName()
						);
					}
				}
			}
		}
	}

}

void NPC::RecalculateSkills()
{
  	int r;
	for (r = 0; r <= EQ::skills::HIGHEST_SKILL; r++) {
		skills[r] = skill_caps.GetSkillCap(GetClass(), (EQ::skills::SkillType)r, level).cap;
	}

	// some overrides -- really we need to be able to set skills for mobs in the DB
	// There are some known low level SHM/BST pets that do not follow this, which supports
	// the theory of needing to be able to set skills for each mob separately
	if (!IsBot()) {
		if (level > 50) {
			skills[EQ::skills::SkillDoubleAttack] = 250;
			skills[EQ::skills::SkillDualWield] = 250;
		}
		else if (level > 3) {
			skills[EQ::skills::SkillDoubleAttack] = level * 5;
			skills[EQ::skills::SkillDualWield] = skills[EQ::skills::SkillDoubleAttack];
		}
		else {
			skills[EQ::skills::SkillDoubleAttack] = level * 5;
		}
	}
}

void NPC::ReloadSpells() {
	AI_AddNPCSpells(GetNPCSpellsID());
	AI_AddNPCSpellsEffects(GetNPCSpellsEffectsID());
}

void NPC::ScaleNPC(uint8 npc_level, bool always_scale, bool override_special_abilities) {
	if (GetLevel() != npc_level) {
		SetLevel(npc_level);
		RecalculateSkills();
		ReloadSpells();
	}

	npc_scale_manager->ResetNPCScaling(this);
	npc_scale_manager->ScaleNPC(this, always_scale, override_special_abilities);
}

bool NPC::IsGuard()
{
	switch (GetRace()) {
	case Race::FreeportGuard:
		if (GetTexture() == 1 || GetTexture() == 2)
			return true;
		break;
	case Race::IksarCitizen:
		if (GetTexture() == 1)
			return true;
		break;
	case Race::Felguard:
	case Race::Fayguard:
	case Race::VahShirGuard:
	case Race::QeynosCitizen:
	case Race::RivervaleCitizen:
	case Race::EruditeCitizen:
	case Race::HalasCitizen:
	case Race::NeriakCitizen:
	case Race::GrobbCitizen:
	case OGGOK_CITIZEN:
	case Race::KaladimCitizen:
		return true;
	default:
		break;
	}
	if (GetPrimaryFaction() == DB_FACTION_GEM_CHOPPERS || GetPrimaryFaction() == DB_FACTION_HERETICS || GetPrimaryFaction() == DB_FACTION_KING_AKANON) { //these 3 factions of guards use player races instead of their own races so we must define them by faction.
		return true;
	}
	return false;
}

std::vector<int> NPC::GetLootList() {
	std::vector<int> npc_items;
	for (auto current_item  = m_loot_items.begin(); current_item != m_loot_items.end(); ++current_item) {
		LootItem * loot_item = *current_item;
		if (!loot_item) {
			LogError("NPC::GetLootList() - ItemList error, null item");
			continue;
		}

		if (std::find(npc_items.begin(), npc_items.end(), loot_item->item_id) != npc_items.end()) {
			continue;
		}

		npc_items.push_back(loot_item->item_id);
	}
	return npc_items;
}

bool NPC::IsRecordLootStats() const
{
	return m_record_loot_stats;
}

void NPC::SetRecordLootStats(bool record_loot_stats)
{
	NPC::m_record_loot_stats = record_loot_stats;
}

const std::vector<uint32> &NPC::GetRolledItems() const
{
	return m_rolled_items;
}

int NPC::GetRolledItemCount(uint32 item_id)
{
	int rolled_count = 0;
	for (auto &e: m_rolled_items) {
		if (item_id == e) {
			rolled_count++;
		}
	}

	return rolled_count;
}

void NPC::SendPositionToClients()
{
	auto      p  = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	auto      *s = (PlayerPositionUpdateServer_Struct *) p->pBuffer;
	for (auto &c: entity_list.GetClientList()) {
		MakeSpawnUpdate(s);
		c.second->QueuePacket(p, false);
	}
	safe_delete(p);
}

void NPC::HandleRoambox()
{
	bool has_arrived = GetCWP() == EQ::WaypointStatus::RoamBoxPauseInProgress && !IsMoving();
	if (has_arrived) {
		int roambox_move_delay = EQ::ClampLower(GetRoamboxDelay(), GetRoamboxMinDelay());
		int move_delay_max     = (roambox_move_delay > 0 ? roambox_move_delay : (int) GetRoamboxMinDelay() * 4);
		int random_timer       = RandomTimer(
			GetRoamboxMinDelay(),
			move_delay_max
		);

		LogNPCRoamBoxDetail(
			"({}) random_timer [{}] roambox_move_delay [{}] move_min [{}] move_max [{}]",
			GetCleanName(),
			random_timer,
			roambox_move_delay,
			(int) GetRoamboxMinDelay(),
			move_delay_max
		);

		time_until_can_move = Timer::GetCurrentTime() + random_timer;
		SetCurrentWP(0);
		return;
	}

	bool ready_to_set_new_destination = !IsMoving() && time_until_can_move < Timer::GetCurrentTime();
	if (ready_to_set_new_destination) {
		// make several attempts to find a valid next move in the box
		bool can_path = false;
		for (int i = 0; i < 10; i++) {
			auto move_x      = static_cast<float>(zone->random.Real(-m_roambox.distance, m_roambox.distance));
			auto move_y      = static_cast<float>(zone->random.Real(-m_roambox.distance, m_roambox.distance));
			auto requested_x = EQ::Clamp((GetX() + move_x), m_roambox.min_x, m_roambox.max_x);
			auto requested_y = EQ::Clamp((GetY() + move_y), m_roambox.min_y, m_roambox.max_y);
			auto requested_z = GetGroundZ(requested_x, requested_y);

			if (std::abs(requested_z - GetZ()) > 100) {
				LogNPCRoamBox("[{}] | Failed to find reasonable ground [{}]", GetCleanName(), i);
				continue;
			}

			std::vector<float> heights = {0, 250, -250};
			for (auto &h: heights) {
				if (CanPathTo(requested_x, requested_y, requested_z + h)) {
					LogNPCRoamBox("[{}] Found line of sight to path attempt [{}] at height [{}]", GetCleanName(), i, h);
					can_path = true;
					break;
				}
			}

			if (!can_path) {
				LogNPCRoamBox("[{}] | Failed line of sight to path attempt [{}]", GetCleanName(), i);
				continue;
			}

			m_roambox.dest_x = requested_x;
			m_roambox.dest_y = requested_y;

			/**
			 * If our roambox was configured with large distances, chances of hitting the min or max end of
			 * the clamp is high, this causes NPC's to gather on the border of a box, to reduce clustering
			 * either lower the roambox distance or the code will do a simple random between min - max when it
			 * hits the min or max of the clamp
			 */
			if (m_roambox.dest_x == m_roambox.min_x || m_roambox.dest_x == m_roambox.max_x) {
				m_roambox.dest_x = static_cast<float>(zone->random.Real(m_roambox.min_x, m_roambox.max_x));
			}

			if (m_roambox.dest_y == m_roambox.min_y || m_roambox.dest_y == m_roambox.max_y) {
				m_roambox.dest_y = static_cast<float>(zone->random.Real(m_roambox.min_y, m_roambox.max_y));
			}

			// If mob was not spawned in water, let's not randomly roam them into water
			// if the roam box was sloppily configured
			if (!GetWasSpawnedInWater()) {
				m_roambox.dest_z = GetGroundZ(m_roambox.dest_x, m_roambox.dest_y);
				if (zone->HasMap() && zone->HasWaterMap()) {
					auto position = glm::vec3(
						m_roambox.dest_x,
						m_roambox.dest_y,
						m_roambox.dest_z
					);

					// If someone brought us into water when we naturally wouldn't path there, return to spawn
					if (zone->watermap->InLiquid(position) && zone->watermap->InLiquid(m_Position)) {
						m_roambox.dest_x = m_SpawnPoint.x;
						m_roambox.dest_y = m_SpawnPoint.y;
					}

					if (zone->watermap->InLiquid(position)) {
						LogNPCRoamBoxDetail("[{}] | My destination is in water and I don't belong there!", GetCleanName());

						return;
					}
				}
			}
			else { // Mob was in water, make sure new spot is in water also
				m_roambox.dest_z = m_Position.z;
				auto position = glm::vec3(
					m_roambox.dest_x,
					m_roambox.dest_y,
					m_Position.z + 15
				);
				if (zone->HasWaterMap() && !zone->watermap->InLiquid(position)) {
					m_roambox.dest_x = m_SpawnPoint.x;
					m_roambox.dest_y = m_SpawnPoint.y;
					m_roambox.dest_z = m_SpawnPoint.z;
				}
			}

			LogNPCRoamBox(
				"[{}] | Pathing to [{}] [{}] [{}]",
				GetCleanName(),
				m_roambox.dest_x,
				m_roambox.dest_y,
				m_roambox.dest_z
			);

			LogNPCRoamBox(
				"NPC ({}) distance [{}] X (min/max) [{} / {}] Y (min/max) [{} / {}] | Dest x/y/z [{} / {} / {}]",
				GetCleanName(),
				m_roambox.distance,
				m_roambox.min_x,
				m_roambox.max_x,
				m_roambox.min_y,
				m_roambox.max_y,
				m_roambox.dest_x,
				m_roambox.dest_y,
				m_roambox.dest_z
			);

			if (can_path) {
				SetCurrentWP(EQ::WaypointStatus::RoamBoxPauseInProgress);
				NavigateTo(m_roambox.dest_x, m_roambox.dest_y, m_roambox.dest_z);
				return;
			}
		}

		// failed to find path, reset timer
		int roambox_move_delay = EQ::ClampLower(GetRoamboxDelay(), GetRoamboxMinDelay());
		int move_delay_max     = (roambox_move_delay > 0 ? roambox_move_delay : (int) GetRoamboxMinDelay() * 4);
		int random_timer       = RandomTimer(
			GetRoamboxMinDelay(),
			move_delay_max
		);
		time_until_can_move = Timer::GetCurrentTime() + random_timer;
	}

	return;
}

void NPC::SetTaunting(bool is_taunting) {
	taunting = is_taunting;

	if (IsPet() && IsPetOwnerClient()) {
		GetOwner()->CastToClient()->SetPetCommandState(PET_BUTTON_TAUNT, is_taunting);
	}
}

bool NPC::CanPathTo(float x, float y, float z)
{
	PathfinderOptions opts;
	opts.smooth_path = true;
	opts.step_size   = RuleR(Pathing, NavmeshStepSize);
	opts.offset      = GetZOffset();
	opts.flags       = PathingNotDisabled ^ PathingZoneLine;

	bool partial = false;
	bool stuck   = false;
	auto route   = zone->pathing->FindPath(
		glm::vec3(GetX(), GetY(), GetZ()),
		glm::vec3(x, y, z),
		partial,
		stuck,
		opts
	);

	return !route.empty();
}

void NPC::DescribeSpecialAbilities(Client* c)
{
	if (!c) {
		return;
	}

	// These abilities are simple on/off flags
	static const std::vector<uint32> toggleable_special_abilities = {
		SPECATK_TRIPLE,
		SPECATK_QUAD,
		SPECATK_INNATE_DW,
		SPECATK_BANE,
		SPECATK_MAGICAL,
		UNSLOWABLE,
		UNMEZABLE,
		UNCHARMABLE,
		UNSTUNABLE,
		UNSNAREABLE,
		UNFEARABLE,
		UNDISPELLABLE,
		IMMUNE_MELEE,
		IMMUNE_MAGIC,
		IMMUNE_FLEEING,
		IMMUNE_MELEE_EXCEPT_BANE,
		IMMUNE_MELEE_NONMAGICAL,
		IMMUNE_AGGRO,
		IMMUNE_AGGRO_ON,
		IMMUNE_CASTING_FROM_RANGE,
		IMMUNE_FEIGN_DEATH,
		IMMUNE_TAUNT,
		NPC_NO_BUFFHEAL_FRIENDS,
		IMMUNE_PACIFY,
		DESTRUCTIBLE_OBJECT,
		NO_HARM_FROM_CLIENT,
		ALWAYS_FLEE,
		ALLOW_BENEFICIAL,
		DISABLE_MELEE,
		ALLOW_TO_TANK,
		IGNORE_ROOT_AGGRO_RULES,
		PROX_AGGRO,
		IMMUNE_RANGED_ATTACKS,
		IMMUNE_DAMAGE_CLIENT,
		IMMUNE_DAMAGE_NPC,
		IMMUNE_AGGRO_CLIENT,
		IMMUNE_AGGRO_NPC,
		IMMUNE_FADING_MEMORIES,
		IMMUNE_OPEN,
		IMMUNE_ASSASSINATE,
		IMMUNE_HEADSHOT,
		IMMUNE_AGGRO_BOT,
		IMMUNE_DAMAGE_BOT
	};

	// These abilities have parameters that need to be parsed out individually
	static const std::map<uint32, std::vector<std::string>> parameter_special_abilities = {
		{ SPECATK_SUMMON, { "Cooldown in Milliseconds", "Health Percentage" } },
		{
			SPECATK_ENRAGE,
			{
				"Health Percentage",
				"Duration in Milliseconds",
				"Cooldown in Milliseconds"
			}
		},
		{
			SPECATK_RAMPAGE,
			{
				"Chance",
				"Targets",
				"Flat Damage Bonus",
				"Ignore Armor Percentage",
				"Ignore Flat Armor Amount",
				"Critical Chance",
				"Flat Critical Chance Bonus"
			}
		},
		{
			SPECATK_AREA_RAMPAGE,
			{
				"Targets",
				"Normal Attack Damage Percentage",
				"Flat Damage Bonus",
				"Ignore Armor Percentage",
				"Ignore Flat Armor Amount",
				"Critical Chance",
				"Flat Critical Chance Bonus"
			}
		},
		{
			SPECATK_FLURRY,
			{
				"Attacks",
				"Normal Attack Damage Percentage",
				"Flat Damage Bonus",
				"Ignore Armor Percentage",
				"Ignore Flat Armor Amount",
				"Critical Chance",
				"Flat Critical Chance Bonus"
			}
		},
		{
			SPECATK_RANGED_ATK,
			{
				"Attacks",
				"Maximum Range",
				"Chance",
				"Damage Percentage",
				"Minimum Range"
			}
		},
		{ NPC_TUNNELVISION, { "Aggro Modifier on Non-Tanks" } },
		{ LEASH, { "Range" } },
		{ TETHER, { "Range" } },
		{ FLEE_PERCENT, { "Health Percentage", "Chance" } },
		{
			NPC_CHASE_DISTANCE,
			{
				"Maximum Distance",
				"Minimum Distance",
				"Ignore Line of Sight"
			}
		},
		{ CASTING_RESIST_DIFF, { "Resist Difficulty Value" } },
		{
			COUNTER_AVOID_DAMAGE,
			{
				"Reduction Percentage for Block, Dodge, Parry, and Riposte",
				"Reduction Percentage for Riposte",
				"Reduction Percentage for Block",
				"Reduction Percentage for Parry",
				"Reduction Percentage for Dodge",
			}
		},
		{
			MODIFY_AVOID_DAMAGE,
			{
				"Addition Percentage for Block, Dodge, Parry, and Riposte",
				"Addition Percentage for Riposte",
				"Addition Percentage for Block",
				"Addition Percentage for Parry",
				"Addition Percentage for Dodge",
			}
		},
	};

	std::vector<std::string> messages = { };

	for (const auto& e : toggleable_special_abilities) {
		if (GetSpecialAbility(e)) {
			messages.emplace_back(
				fmt::format(
					"{} ({})",
					EQ::constants::GetSpecialAbilityName(e),
					e
				)
			);
		}
	}

	int slot_id;

	for (const auto& e : parameter_special_abilities) {
		if (GetSpecialAbility(e.first)) {
			slot_id = 0;

			for (const auto& a : e.second) {
				messages.emplace_back(
					fmt::format(
						"{} ({}) | {}: {}",
						EQ::constants::GetSpecialAbilityName(e.first),
						e.first,
						a,
						GetSpecialAbilityParam(e.first, slot_id)
					)
				);

				slot_id++;
			}
		}
	}

	if (messages.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has no special abilities.",
				c->GetTargetDescription(this)
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} has the following special abilit{}:",
			c->GetTargetDescription(this),
			messages.size() != 1 ? "ies" : "y"
		).c_str()
	);

	std::sort(
		messages.begin(),
		messages.end(),
		[](const std::string& a, const std::string& b) {
			return a < b;
		}
	);

	for (const auto& e : messages) {
		c->Message(Chat::White, e.c_str());
	}
}
