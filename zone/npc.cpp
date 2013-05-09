/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include <iostream>
#include <string>
#include <cctype>
using namespace std;
#include <math.h>
#include "../common/moremath.h"
#include <stdio.h>
#include "../common/packet_dump_file.h"
#include "zone.h"
#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp  _stricmp
#else
#include <stdlib.h>
#include <pthread.h>
#endif

#include "npc.h"
#include "map.h"
#include "entity.h"
#include "masterentity.h"
#include "../common/spdat.h"
#include "../common/bodytypes.h"
#include "spawngroup.h"
#include "../common/MiscFunctions.h"
#include "../common/rulesys.h"
#include "StringIDs.h"

//#define SPELLQUEUE //Use only if you want to be spammed by spell testing


extern Zone* zone;
extern volatile bool ZoneLoaded;
extern EntityList entity_list;

#include "QuestParserCollection.h"

NPC::NPC(const NPCType* d, Spawn2* in_respawn, float x, float y, float z, float heading, int iflymode, bool IsCorpse)
: Mob(d->name,
	  d->lastname,
	  d->max_hp,
	  d->max_hp,
	  d->gender,
	  d->race,
	  d->class_,
      (bodyType)d->bodytype,
	  d->deity,
	  d->level,
	  d->npc_id,
	  d->size,
	  d->runspeed,
	  heading,
	  x,
	  y,
	  z,
	  d->light,
	  d->texture,
	  d->helmtexture,
	  d->AC,
	  d->ATK,
	  d->STR,
	  d->STA,
	  d->DEX,
	  d->AGI,
	  d->INT,
	  d->WIS,
	  d->CHA,
	  d->haircolor,
	  d->beardcolor,
	  d->eyecolor1,
	  d->eyecolor2,
	  d->hairstyle,
	  d->luclinface,
	  d->beard,
	  d->drakkin_heritage,
	  d->drakkin_tattoo,
	  d->drakkin_details,
	  (uint32*)d->armor_tint,
	  0,
	  d->see_invis,			// pass see_invis/see_ivu flags to mob constructor
	  d->see_invis_undead,
	  d->see_hide,
	  d->see_improved_hide,
	  d->hp_regen,
	  d->mana_regen,
	  d->qglobal,
	  d->maxlevel,
	  d->scalerate ),
	attacked_timer(CombatEventTimer_expire),
	swarm_timer(100),
	classattack_timer(1000),
	knightattack_timer(1000),
	assist_timer(AIassistcheck_delay),
	sendhpupdate_timer(1000),
	enraged_timer(1000),
	taunt_timer(TauntReuseTime * 1000),
	qglobal_purge_timer(30000)
{
	//What is the point of this, since the names get mangled..
	Mob* mob = entity_list.GetMob(name);
	if(mob != 0)
		entity_list.RemoveEntity(mob->GetID());

	int moblevel=GetLevel();
	
	NPCTypedata = d;
	NPCTypedata_ours = nullptr;
	respawn2 = in_respawn;
	swarm_timer.Disable();
	
	taunting = false;
	proximity = nullptr;
	copper = 0;
	silver = 0;
	gold = 0;
	platinum = 0;
	max_dmg = d->max_dmg;
	min_dmg = d->min_dmg;
    attack_count = d->attack_count;
	grid = 0;
	wp_m = 0;
	max_wp=0;
	save_wp = 0;
	spawn_group = 0;
	swarmInfoPtr = nullptr;
	spellscale = d->spellscale;
	healscale = d->healscale;

	logging_enabled = NPC_DEFAULT_LOGGING_ENABLED;
	
	pAggroRange = d->aggroradius;
	pAssistRange = GetAggroRange();
	findable = d->findable;
	trackable = d->trackable;

    MR = d->MR;
    CR = d->CR;
    DR = d->DR;
    FR = d->FR;
    PR = d->PR;
	Corrup = d->Corrup;

	STR = d->STR;
	STA = d->STA;
	AGI = d->AGI;
	DEX = d->DEX;
	INT = d->INT;
	WIS = d->WIS;
	CHA = d->CHA;
    npc_mana = d->Mana;

	//quick fix of ordering if they screwed it up in the DB
	if(max_dmg < min_dmg) {
		int tmp = min_dmg;
		min_dmg = max_dmg;
		max_dmg = tmp;
	}

	// Max Level and Stat Scaling if maxlevel is set
	if(maxlevel > level)
	{
		LevelScale();
	}

	// Set Resists if they are 0 in the DB
	CalcNPCResists();

	// Set Mana and HP Regen Rates if they are 0 in the DB
	CalcNPCRegen();
		
	// Set Min and Max Damage if they are 0 in the DB
	if(max_dmg == 0){
		CalcNPCDamage();
	}	
	
	accuracy_rating = d->accuracy_rating;
	ATK = d->ATK;

    CalcMaxMana();
    SetMana(GetMaxMana());

	MerchantType = d->merchanttype;
	adventure_template_id = d->adventure_template;
	org_x = x;
	org_y = y;
	org_z = z;
	flymode = iflymode;
	guard_x = -1;	//just some value we might be able to recongize as "unset"
	guard_y = -1;
	guard_z = -1;
	guard_heading = 0;
    guard_anim = eaStanding;
	roambox_distance = 0;
	roambox_max_x = -2;
	roambox_max_y = -2;
	roambox_min_x = -2;
	roambox_min_y = -2;
	roambox_movingto_x = -2;
	roambox_movingto_y = -2;
	roambox_delay = 1000;
	org_heading = heading;	
	p_depop = false;
	loottable_id = d->loottable_id;	

	primary_faction = 0;
	SetNPCFactionID(d->npc_faction_id);

	npc_spells_id = 0;
	HasAISpell = false;

	if(GetClass() == MERCERNARY_MASTER && RuleB(Mercs, AllowMercs))
	{
		LoadMercTypes();
		LoadMercs();
	}

	SpellFocusDMG = 0;
	SpellFocusHeal = 0; 
	
	pet_spell_id = 0;
	
	delaytimer = false;
	combat_event = false;
	attack_speed = d->attack_speed;
	slow_mitigation = d->slow_mitigation;

	EntityList::RemoveNumbers(name);
    entity_list.MakeNameUnique(name);

	npc_aggro = d->npc_aggro;

	if(!IsMerc())
		AI_Start();

	d_meele_texture1 = d->d_meele_texture1;
	d_meele_texture2 = d->d_meele_texture2;
	memset(equipment, 0, sizeof(equipment));
	prim_melee_type = d->prim_melee_type;
	sec_melee_type = d->sec_melee_type;

	// If Melee Textures are not set, set attack type to Hand to Hand as default
	if(!d_meele_texture1)
		prim_melee_type = 28;
	if(!d_meele_texture2)
		sec_melee_type = 28;
	
	//give NPCs skill values...
	int r;
	for(r = 0; r <= HIGHEST_SKILL; r++) {
		skills[r] = database.GetSkillCap(GetClass(),(SkillType)r,moblevel);
	}

	if(d->trap_template > 0)
	{
		std::map<uint32,std::list<LDoNTrapTemplate*> >::iterator trap_ent_iter; 
		std::list<LDoNTrapTemplate*> trap_list;

		trap_ent_iter = zone->ldon_trap_entry_list.find(d->trap_template);
		if(trap_ent_iter != zone->ldon_trap_entry_list.end())
		{
			trap_list = trap_ent_iter->second;
			if(trap_list.size() > 0)
			{
				uint16 count = MakeRandomInt(0, (trap_list.size()-1));
				std::list<LDoNTrapTemplate*>::iterator trap_list_iter = trap_list.begin();
				for(int x = 0; x < count; ++x)
				{
					trap_list_iter++;
				}
				LDoNTrapTemplate* tt = (*trap_list_iter);
				if(tt)
				{
					if((uint8)tt->spell_id > 0)
					{
						ldon_trapped = true;
						ldon_spell_id = tt->spell_id;
					}
					else
					{
						ldon_trapped = false;
						ldon_spell_id = 0;
					}

					ldon_trap_type = (uint8)tt->type;
					if(tt->locked > 0)
					{
						ldon_locked = true;
						ldon_locked_skill = tt->skill;
					}
					else
					{
						ldon_locked = false;
						ldon_locked_skill = 0;
					}
					ldon_trap_detected = 0;
				}
			}
			else
			{
				ldon_trapped = false;
				ldon_trap_type = 0;
				ldon_spell_id = 0;
				ldon_locked = false;
				ldon_locked_skill = 0;
				ldon_trap_detected = 0;
			}
		}
		else
		{
			ldon_trapped = false;
			ldon_trap_type = 0;
			ldon_spell_id = 0;
			ldon_locked = false;
			ldon_locked_skill = 0;
			ldon_trap_detected = 0;
		}
	}
	else
	{
		ldon_trapped = false;
		ldon_trap_type = 0;
		ldon_spell_id = 0;
		ldon_locked = false;
		ldon_locked_skill = 0;
		ldon_trap_detected = 0;
	}
    reface_timer = new Timer(15000);
    reface_timer->Disable();
	qGlobals = nullptr;
	guard_x_saved = 0;
	guard_y_saved = 0;
	guard_z_saved = 0;
	guard_heading_saved = 0;
	SetEmoteID(d->emoteid);
	InitializeBuffSlots();
	CalcBonuses();
}
	  
NPC::~NPC()
{
	ClearQuestLists();
	entity_list.RemoveNPC(GetID());
	AI_Stop();

	if(proximity != nullptr) {
		entity_list.RemoveProximity(GetID());
		safe_delete(proximity);
	}

	//clear our spawn limit record if we had one.
	entity_list.LimitRemoveNPC(this);
	
	safe_delete(NPCTypedata_ours);
	
	{
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; cur++) {
		ServerLootItem_Struct* item = *cur;
		safe_delete(item);
	}
	itemlist.clear();
	}
	
	{
	list<struct NPCFaction*>::iterator cur,end;
	cur = faction_list.begin();
	end = faction_list.end();
	for(; cur != end; cur++) {
		struct NPCFaction* fac = *cur;
		safe_delete(fac);
	}
	faction_list.clear();
	}

    safe_delete(reface_timer);
	safe_delete(swarmInfoPtr);
	safe_delete(qGlobals);
	UninitializeBuffSlots();
}

void NPC::SetTarget(Mob* mob) {
	if(mob == GetTarget())		//dont bother if they are allready our target
		return;
	
	//our target is already set, do not turn from the course, unless our current target is dead.
	if(GetSwarmInfo() && GetTarget() && (GetTarget()->GetHP() > 0)) {
		Mob *targ = entity_list.GetMob(GetSwarmInfo()->target);
		if(targ != mob){
			return;
		}
	}

	if (mob) {
		SetAttackTimer();
	} else {
		ranged_timer.Disable();
		//attack_timer.Disable();
		attack_dw_timer.Disable();
	}
	Mob::SetTarget(mob);
}

ServerLootItem_Struct* NPC::GetItem(int slot_id) {
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; cur++) {
		ServerLootItem_Struct* item = *cur;
		if (item->equipSlot == slot_id) {
			return item;
		}
	}
	return(nullptr);
}
	  
void NPC::RemoveItem(uint32 item_id, uint16 quantity, uint16 slot) {
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; cur++) {
		ServerLootItem_Struct* item = *cur;
		if (item->item_id == item_id && slot <= 0 && quantity <= 0) {
			itemlist.erase(cur);
			return;
		}
		else if (item->item_id == item_id && item->equipSlot == slot  && quantity >= 1) {
			//cout<<"NPC::RemoveItem"<<" equipSlot:"<<iterator.GetData()->equipSlot<<" quantity:"<< quantity<<endl;
			if (item->charges <= quantity)
				itemlist.erase(cur);
			else
				item->charges -= quantity;
			return;
		}
	}
}

void NPC::CheckMinMaxLevel(Mob *them)
{
	if(them == nullptr || !them->IsClient())
		return;

	uint16 themlevel = them->GetLevel();
	uint8 material;

	list<ServerLootItem_Struct*>::iterator cur = itemlist.begin();
	while(cur != itemlist.end())
	{
		if(!(*cur))
			return;

		if(themlevel < (*cur)->minlevel || themlevel > (*cur)->maxlevel)
		{
			material = Inventory::CalcMaterialFromSlot((*cur)->equipSlot);
			if(material != 0xFF)
				SendWearChange(material);

			cur = itemlist.erase(cur);
			continue;
		}
		cur++;
	}
	
}

void NPC::ClearItemList() {
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; cur++) {
		ServerLootItem_Struct* item = *cur;
		safe_delete(item);
	}
	itemlist.clear();
}
	  
void NPC::QueryLoot(Client* to) {
	int x = 0;
	to->Message(0, "Coin: %ip %ig %is %ic", platinum, gold, silver, copper);

	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; cur++) {
		const Item_Struct* item = database.GetItem((*cur)->item_id);
		if (item)
			if (to->GetClientVersion() >= EQClientRoF)
			{
				to->Message(0, "minlvl: %i maxlvl: %i %i: %c%06X0000000000000000000000000000000000000000000000000%s%c",(*cur)->minlevel, (*cur)->maxlevel, (int) item->ID,0x12, item->ID, item->Name, 0x12);
			}
			else if (to->GetClientVersion() >= EQClientSoF)
			{
				to->Message(0, "minlvl: %i maxlvl: %i %i: %c%06X00000000000000000000000000000000000000000000%s%c",(*cur)->minlevel, (*cur)->maxlevel, (int) item->ID,0x12, item->ID, item->Name, 0x12);
			}
			else
			{
				to->Message(0, "minlvl: %i maxlvl: %i %i: %c%06X000000000000000000000000000000000000000%s%c",(*cur)->minlevel, (*cur)->maxlevel, (int) item->ID,0x12, item->ID, item->Name, 0x12);
			}
		else
		    LogFile->write(EQEMuLog::Error, "Database error, invalid item");
		x++;
	}
	to->Message(0, "%i items on %s.", x, GetName());
}

void NPC::AddCash(uint16 in_copper, uint16 in_silver, uint16 in_gold, uint16 in_platinum) {
	if(in_copper >= 0)
        copper = in_copper;
    else
        copper = 0;

    if(in_silver >= 0)
        silver = in_silver;
    else
        silver = 0;

    if(in_gold >= 0)
        gold = in_gold;
    else
        gold = 0;

    if(in_platinum >= 0)
        platinum = in_platinum;
    else
        platinum = 0;
}
	  
void NPC::AddCash() {
	copper = MakeRandomInt(1, 100);
	silver = MakeRandomInt(1, 50);
	gold = MakeRandomInt(1, 10);
	platinum = MakeRandomInt(1, 5);
}
	  
void NPC::RemoveCash() {
	copper = 0;
	silver = 0;
	gold = 0;
	platinum = 0;
}

bool NPC::Process()
{
	_ZP(NPC_Process);
	
    adverrorinfo = 1;
	if (IsStunned() && stunned_timer.Check())
    {
        this->stunned = false;
        this->stunned_timer.Disable();
		this->spun_timer.Disable();
    }

    if (p_depop)
    {
        Mob* owner = entity_list.GetMob(this->ownerid);
        if (owner != 0)
        {
        	//if(GetBodyType() != BT_SwarmPet)
	        //    owner->SetPetID(0);
			this->ownerid = 0;
            this->petid = 0;
        }
        return false;
    }

    adverrorinfo = 2;
    
    SpellProcess();
    
    if(tic_timer.Check()) 
	{	
		BuffProcess();
		
		if(curfp)
			ProcessFlee();
		
		uint32 bonus = 0;
		
		if(GetAppearance() == eaSitting)
			bonus+=3;
		
		int32 OOCRegen = 0;
		if(oocregen > 0){ //should pull from Mob class
			OOCRegen += GetMaxHP() * oocregen / 100;
			}
		//Lieka Edit:  Fixing NPC regen.  NPCs should regen to full during a set duration, not based on their HPs.  Increase NPC's HPs by % of total HPs / tick.
		if((GetHP() < GetMaxHP()) && !IsPet()) {
			if(!IsEngaged()) {//NPC out of combat
				if(hp_regen > OOCRegen)
					SetHP(GetHP() + hp_regen);
				else
					SetHP(GetHP() + OOCRegen);
			} else
				SetHP(GetHP()+hp_regen);
		} else if(GetHP() < GetMaxHP() && GetOwnerID() !=0) {
			if(!IsEngaged()) //pet
				SetHP(GetHP()+hp_regen+bonus+(GetLevel()/5));
			else
				SetHP(GetHP()+hp_regen+bonus);
		} else 
			SetHP(GetHP()+hp_regen);

		if(GetMana() < GetMaxMana()) {
			SetMana(GetMana()+mana_regen+bonus);
		}


		if(zone->adv_data && !p_depop)
		{
			ServerZoneAdventureDataReply_Struct* ds = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
			if(ds->type == Adventure_Rescue && ds->data_id == GetNPCTypeID())
			{
				Mob *o = GetOwner();
				if(o && o->IsClient())
				{
					float x_diff = ds->dest_x - GetX();
					float y_diff = ds->dest_y - GetY();
					float z_diff = ds->dest_z - GetZ();
					float dist = ((x_diff * x_diff) + (y_diff * y_diff) + (z_diff * z_diff));
					if(dist < RuleR(Adventure, DistanceForRescueComplete))
					{
						zone->DoAdventureCountIncrease();
						Say("You don't know what this means to me. Thank you so much for finding and saving me from"
							" this wretched place. I'll find my way from here.");
						Depop();
					}
				}
			}
		}
    }

	if (sendhpupdate_timer.Check() && (IsTargeted() || (IsPet() && GetOwner() && GetOwner()->IsClient()))) {
		if(!IsFullHP || cur_hp<max_hp){
			SendHPUpdate();
		}
	}
	
	if(HasVirus()) {
		if(viral_timer.Check()) {
			viral_timer_counter++;
			for(int i = 0; i < MAX_SPELL_TRIGGER*2; i+=2) {
				if(viral_spells[i])	{
					if(viral_timer_counter % spells[viral_spells[i]].viral_timer == 0) {
						SpreadVirus(viral_spells[i], viral_spells[i+1]);
					}
				}
			}
		}
		if(viral_timer_counter > 999)
			viral_timer_counter = 0;
	}
	
	if(spellbonuses.GravityEffect == 1) {
		if(gravity_timer.Check())
			DoGravityEffect();
	}

    if(reface_timer->Check() && !IsEngaged() && (guard_x == GetX() && guard_y == GetY() && guard_z == GetZ())) {
        SetHeading(guard_heading);
        SendPosition();
        reface_timer->Disable();
    }
	
    if (IsMezzed())
	    return true;
	
	if(IsStunned()) {
		if(spun_timer.Check()) 
			Spin();
		return true;
	}
	
	if (enraged_timer.Check()){
		ProcessEnrage();
	}
	
	//Handle assists...
	if(assist_timer.Check() && IsEngaged() && !Charmed()) {
		entity_list.AIYellForHelp(this, GetTarget());
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
	return(itemlist.size());
}

void NPC::DumpLoot(uint32 npcdump_index, ZSDump_NPC_Loot* npclootdump, uint32* NPCLootindex) {
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; cur++) {
		ServerLootItem_Struct* item = *cur;
		npclootdump[*NPCLootindex].npc_dump_index = npcdump_index;
		npclootdump[*NPCLootindex].itemid = item->item_id;
		npclootdump[*NPCLootindex].charges = item->charges;
		npclootdump[*NPCLootindex].equipSlot = item->equipSlot;
		npclootdump[*NPCLootindex].minlevel = item->minlevel;
		npclootdump[*NPCLootindex].maxlevel = item->maxlevel;
		(*NPCLootindex)++;
	}
	ClearItemList();
}

void NPC::Depop(bool StartSpawnTimer) {
	uint16 emoteid = this->GetEmoteID();
	if(emoteid != 0)
		this->DoNPCEmote(ONDESPAWN,emoteid);
	p_depop = true;
	if (StartSpawnTimer) {
		if (respawn2 != 0) {
			respawn2->DeathReset();
		}
	}
}

bool NPC::DatabaseCastAccepted(int spell_id) {
	for (int i=0; i < 12; i++) {
		switch(spells[spell_id].effectid[i]) {
		case SE_Stamina: {
			if(IsEngaged() && GetHPRatio() < 100)
				return true;
			else
				return false;
			break;
						 }
		case SE_CurrentHPOnce:
		case SE_CurrentHP: {
			if(this->GetHPRatio() < 100 && spells[spell_id].buffduration == 0)
				return true;
			else
				return false;
			break;
						   }
			
		case SE_HealOverTime: {
			if(this->GetHPRatio() < 100)
				return true;
			else
				return false;
			break;
							  }
		case SE_DamageShield: {
			return true;
							  }
		case SE_NecPet:
		case SE_SummonPet: {
			if(GetPet()){
#ifdef SPELLQUEUE
				printf("%s: Attempted to make a second pet, denied.\n",GetName());
#endif
				return false;
			}
			break;
						   }
		case SE_LocateCorpse:
		case SE_SummonCorpse: {
			return false; //Pfft, npcs don't need to summon corpses/locate corpses!
			break;
						}
		default:
			if(spells[spell_id].goodEffect == 1 && !(spells[spell_id].buffduration == 0 && this->GetHPRatio() == 100) && !IsEngaged())
				return true;
			return false;
		}
	}
	return false;
}

NPC* NPC::SpawnNPC(const char* spawncommand, float in_x, float in_y, float in_z, float in_heading, Client* client) {
	if(spawncommand == 0 || spawncommand[0] == 0) {
		return 0;
	}
	else {
		Seperator sep(spawncommand);
		//Lets see if someone didn't fill out the whole #spawn function properly 
		if (!sep.IsNumber(1))
			sprintf(sep.arg[1],"1"); 
		if (!sep.IsNumber(2))
			sprintf(sep.arg[2],"1"); 
		if (!sep.IsNumber(3))
			sprintf(sep.arg[3],"0");
		if (atoi(sep.arg[4]) > 2100000000 || atoi(sep.arg[4]) <= 0)
			sprintf(sep.arg[4]," ");
		if (!strcmp(sep.arg[5],"-"))
			sprintf(sep.arg[5]," "); 
		if (!sep.IsNumber(5))
			sprintf(sep.arg[5]," "); 
		if (!sep.IsNumber(6))
			sprintf(sep.arg[6],"1");
		if (!sep.IsNumber(8))
			sprintf(sep.arg[8],"0");
		if (!sep.IsNumber(9))
			sprintf(sep.arg[9], "0");
		if (!sep.IsNumber(7))
			sprintf(sep.arg[7],"0");
		if (!strcmp(sep.arg[4],"-"))
			sprintf(sep.arg[4]," "); 
		if (!sep.IsNumber(10))	// bodytype
			sprintf(sep.arg[10], "0");
		//Calc MaxHP if client neglected to enter it...
		if (!sep.IsNumber(4)) {
			//Stolen from Client::GetMaxHP...
			uint8 multiplier = 0;
			int tmplevel = atoi(sep.arg[2]);
			switch(atoi(sep.arg[5]))
			{
			case WARRIOR:
				if (tmplevel < 20)
					multiplier = 22;
				else if (tmplevel < 30)
					multiplier = 23;
				else if (tmplevel < 40)
					multiplier = 25;
				else if (tmplevel < 53)
					multiplier = 27;
				else if (tmplevel < 57)
					multiplier = 28;
				else 
					multiplier = 30;
				break;
				
			case DRUID:
			case CLERIC:
			case SHAMAN:
				multiplier = 15;
				break;
				
			case PALADIN:
			case SHADOWKNIGHT:
				if (tmplevel < 35)
					multiplier = 21;
				else if (tmplevel < 45)
					multiplier = 22;
				else if (tmplevel < 51)
					multiplier = 23;
				else if (tmplevel < 56)
					multiplier = 24;
				else if (tmplevel < 60)
					multiplier = 25;
				else
					multiplier = 26;
				break;
				
			case MONK:
			case BARD:
			case ROGUE:
			//case BEASTLORD:
				if (tmplevel < 51)
					multiplier = 18;
				else if (tmplevel < 58)
					multiplier = 19;
				else
					multiplier = 20;				
				break;
				
			case RANGER:
				if (tmplevel < 58)
					multiplier = 20;
				else
					multiplier = 21;			
				break;
				
			case MAGICIAN:
			case WIZARD:
			case NECROMANCER:
			case ENCHANTER:
				multiplier = 12;
				break;
				
			default:
				if (tmplevel < 35)
					multiplier = 21;
				else if (tmplevel < 45)
					multiplier = 22;
				else if (tmplevel < 51)
					multiplier = 23;
				else if (tmplevel < 56)
					multiplier = 24;
				else if (tmplevel < 60)
					multiplier = 25;
				else
					multiplier = 26;
				break;
			}
			sprintf(sep.arg[4],"%i",5+multiplier*atoi(sep.arg[2])+multiplier*atoi(sep.arg[2])*75/300);
		}
		
		// Autoselect NPC Gender
		if (sep.arg[5][0] == 0) {
			sprintf(sep.arg[5], "%i", (int) Mob::GetDefaultGender(atoi(sep.arg[1])));
		}
		
		//Time to create the NPC!! 
		NPCType* npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));
		
		strncpy(npc_type->name, sep.arg[0], 60);
		npc_type->cur_hp = atoi(sep.arg[4]); 
		npc_type->max_hp = atoi(sep.arg[4]); 
		npc_type->race = atoi(sep.arg[1]);
		npc_type->gender = atoi(sep.arg[5]); 
		npc_type->class_ = atoi(sep.arg[6]); 
		npc_type->deity = 1;
		npc_type->level = atoi(sep.arg[2]);
		npc_type->npc_id = 0;
		npc_type->loottable_id = 0;
		npc_type->texture = atoi(sep.arg[3]);
		npc_type->light = 0;
		npc_type->runspeed = 1.25;
		npc_type->d_meele_texture1 = atoi(sep.arg[7]);
		npc_type->d_meele_texture2 = atoi(sep.arg[8]);
		npc_type->merchanttype = atoi(sep.arg[9]);	
		npc_type->bodytype = atoi(sep.arg[10]);
		
		npc_type->STR = 150;
		npc_type->STA = 150;
		npc_type->DEX = 150;
		npc_type->AGI = 150;
		npc_type->INT = 150;
		npc_type->WIS = 150;
		npc_type->CHA = 150;

		npc_type->prim_melee_type = 28;
		npc_type->sec_melee_type = 28;
		
		NPC* npc = new NPC(npc_type, 0, in_x, in_y, in_z, in_heading/8, FlyMode3);
		npc->GiveNPCTypeData(npc_type);
		
		entity_list.AddNPC(npc);

		if (client) {
			// Notify client of spawn data 
			client->Message(0, "New spawn:");
			client->Message(0, "Name: %s", npc->name);
			client->Message(0, "Race: %u", npc->race);
			client->Message(0, "Level: %u", npc->level);
			client->Message(0, "Material: %u", npc->texture);
			client->Message(0, "Current/Max HP: %i", npc->max_hp);
			client->Message(0, "Gender: %u", npc->gender);
			client->Message(0, "Class: %u", npc->class_);
			client->Message(0, "Weapon Item Number: %u/%u", npc->d_meele_texture1, npc->d_meele_texture2);
			client->Message(0, "MerchantID: %u", npc->MerchantType);
			client->Message(0, "Bodytype: %u", npc->bodytype);
		}

		return npc;
	}
}

uint32 ZoneDatabase::NPCSpawnDB(uint8 command, const char* zone, uint32 zone_version, Client *c, NPC* spawn, uint32 extra) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
    MYSQL_ROW row;
	uint32 tmp = 0;
	uint32 tmp2 = 0;
    uint32 last_insert_id = 0;
	switch (command) {
		case 0: { // Create a new NPC and add all spawn related data
			uint32 npc_type_id = 0;
			uint32 spawngroupid;
			if (extra && c && c->GetZoneID())
			{
				// Set an npc_type ID within the standard range for the current zone if possible (zone_id * 1000)
				int starting_npc_id = c->GetZoneID() * 1000;				
				if (RunQuery(query, MakeAnyLenString(&query, "SELECT MAX(id) FROM npc_types WHERE id >= %i AND id < %i", starting_npc_id, (starting_npc_id + 1000)), errbuf, &result)) {
					row = mysql_fetch_row(result);
					if(row)
					{
						if (row[0])
						{
							npc_type_id = atoi(row[0]) + 1;
							// Prevent the npc_type id from exceeding the range for this zone
							if (npc_type_id >= (starting_npc_id + 1000))
							{
								npc_type_id = 0;
							}
						}
						else
						{
							// row[0] is nullptr - No npc_type IDs set in this range yet
							npc_type_id = starting_npc_id;
						}
					}
					
					safe_delete_array(query);
					mysql_free_result(result);
				}
			}
			char tmpstr[64];
			EntityList::RemoveNumbers(strn0cpy(tmpstr, spawn->GetName(), sizeof(tmpstr)));
			if (npc_type_id)
			{
				if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO npc_types (id, name, level, race, class, hp, gender, texture, helmtexture, size, loottable_id, merchant_id, face, runspeed, prim_melee_type, sec_melee_type) values(%i,\"%s\",%i,%i,%i,%i,%i,%i,%i,%f,%i,%i,%i,%f,%i,%i)", npc_type_id, tmpstr, spawn->GetLevel(), spawn->GetRace(), spawn->GetClass(), spawn->GetMaxHP(), spawn->GetGender(), spawn->GetTexture(), spawn->GetHelmTexture(), spawn->GetSize(), spawn->GetLoottableID(), spawn->MerchantType, 0, spawn->GetRunspeed(), 28, 28), errbuf, 0, 0, &npc_type_id)) {
		    			LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
					safe_delete(query);
					return false;
				}
			}
			else
			{
				if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO npc_types (name, level, race, class, hp, gender, texture, helmtexture, size, loottable_id, merchant_id, face, runspeed, prim_melee_type, sec_melee_type) values(\"%s\",%i,%i,%i,%i,%i,%i,%i,%f,%i,%i,%i,%f,%i,%i)", tmpstr, spawn->GetLevel(), spawn->GetRace(), spawn->GetClass(), spawn->GetMaxHP(), spawn->GetGender(), spawn->GetTexture(), spawn->GetHelmTexture(), spawn->GetSize(), spawn->GetLoottableID(), spawn->MerchantType, 0, spawn->GetRunspeed(), 28, 28), errbuf, 0, 0, &npc_type_id)) {
		    			LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
					safe_delete(query);
					return false;
				}
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			snprintf(tmpstr, sizeof(tmpstr), "%s-%s", zone, spawn->GetName());
			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO spawngroup (id, name) values(%i, '%s')", tmp, tmpstr), errbuf, 0, 0, &spawngroupid)) {
		    		LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO spawn2 (zone, version, x, y, z, respawntime, heading, spawngroupID) values('%s', %u, %f, %f, %f, %i, %f, %i)", zone, zone_version, spawn->GetX(), spawn->GetY(), spawn->GetZ(), 1200, spawn->GetHeading(), spawngroupid), errbuf, 0, 0, &tmp)) {
		    		LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO spawnentry (spawngroupID, npcID, chance) values(%i, %i, %i)", spawngroupid, npc_type_id, 100), errbuf, 0)) {
		    		LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			return true;
			break;
		}
		case 1:{ // Add new spawn group and spawn point for an existing NPC Type ID
			tmp2 = spawn->GetNPCTypeID();
			char tmpstr[64];
			snprintf(tmpstr, sizeof(tmpstr), "%s%s%i", zone, spawn->GetName(),Timer::GetCurrentTime());
			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO spawngroup (name) values('%s')", tmpstr), errbuf, 0, 0, &last_insert_id)) {
		    		LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);

			uint32 respawntime = 0;
			uint32 spawnid = 0;
			if (extra)
				respawntime = extra;
			else if(spawn->respawn2 && spawn->respawn2->RespawnTimer() != 0)
				respawntime = spawn->respawn2->RespawnTimer();
			else
				respawntime = 1200;
			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO spawn2 (zone, version, x, y, z, respawntime, heading, spawngroupID) values('%s', %u, %f, %f, %f, %i, %f, %i)", zone, zone_version, spawn->GetX(), spawn->GetY(), spawn->GetZ(), respawntime, spawn->GetHeading(), last_insert_id), errbuf, 0, 0, &spawnid)) {
		    		LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);

			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO spawnentry (spawngroupID, npcID, chance) values(%i, %i, %i)", last_insert_id, tmp2, 100), errbuf, 0)) {
		    		LogFile->write(EQEMuLog::Error, "NPCSpawnDB Error: %s %s", query, errbuf);
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			return spawnid;
			break;
		}
		case 2: { // Update npc_type appearance and other data on targeted spawn
			if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE npc_types SET name=\"%s\", level=%i, race=%i, class=%i, hp=%i, gender=%i, texture=%i, helmtexture=%i, size=%i, loottable_id=%i, merchant_id=%i, face=%i, WHERE id=%i", spawn->GetName(), spawn->GetLevel(), spawn->GetRace(), spawn->GetClass(), spawn->GetMaxHP(), spawn->GetGender(), spawn->GetTexture(), spawn->GetHelmTexture(), spawn->GetSize(), spawn->GetLoottableID(), spawn->MerchantType, spawn->GetNPCTypeID()), errbuf, 0)) {
				if(c) c->LogSQL(query);
				safe_delete_array(query);
				return true;
			}
			else {
				safe_delete_array(query);
				return false;
			}
			break;
		}
		case 3: { // delete spawn from spawning, but leave in npc_types table
			if (!RunQuery(query, MakeAnyLenString(&query, "SELECT id,spawngroupID from spawn2 where zone='%s' AND spawngroupID=%i", zone, spawn->GetSp2()), errbuf, &result)) {
				safe_delete_array(query);
				return 0;
			}
			safe_delete_array(query);

			row = mysql_fetch_row(result);
			if (row == nullptr) return false;
			if (row[0]) tmp = atoi(row[0]);
			if (row[1]) tmp2 = atoi(row[1]);

			if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM spawn2 WHERE id='%i'", tmp), errbuf,0)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM spawngroup WHERE id='%i'", tmp2), errbuf,0)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM spawnentry WHERE spawngroupID='%i'", tmp2), errbuf,0)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			return true;


			break;
		}
		case 4: { //delete spawn from DB (including npc_type)
			if (!RunQuery(query, MakeAnyLenString(&query, "SELECT id,spawngroupID from spawn2 where zone='%s' AND version=%u AND spawngroupID=%i", zone, zone_version, spawn->GetSp2()), errbuf, &result)) {
				safe_delete_array(query);
				return(0);
			}
			safe_delete_array(query);

			row = mysql_fetch_row(result);
			if (row == nullptr) return false;
			if (row[0]) tmp = atoi(row[0]);
			if (row[1]) tmp2 = atoi(row[1]);
			mysql_free_result(result);

			if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM spawn2 WHERE id='%i'", tmp), errbuf,0)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM spawngroup WHERE id='%i'", tmp2), errbuf,0)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM spawnentry WHERE spawngroupID='%i'", tmp2), errbuf,0)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM npc_types WHERE id='%i'", spawn->GetNPCTypeID()), errbuf,0)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			return true;
			break;
		}
		case 5: { // add a spawn from spawngroup
			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO spawn2 (zone, version, x, y, z, respawntime, heading, spawngroupID) values('%s', %u, %f, %f, %f, %i, %f, %i)", zone, zone_version, c->GetX(), c->GetY(), c->GetZ(), 120, c->GetHeading(), extra), errbuf, 0, 0, &tmp)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);

			return true;
			break;
			}
		case 6: { // add npc_type
			uint32 npc_type_id;
			char tmpstr[64];
			EntityList::RemoveNumbers(strn0cpy(tmpstr, spawn->GetName(), sizeof(tmpstr)));
			if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO npc_types (name, level, race, class, hp, gender, texture, helmtexture, size, loottable_id, merchant_id, face, runspeed, prim_melee_type, sec_melee_type) values(\"%s\",%i,%i,%i,%i,%i,%i,%i,%f,%i,%i,%i,%f,%i,%i)", tmpstr, spawn->GetLevel(), spawn->GetRace(), spawn->GetClass(), spawn->GetMaxHP(), spawn->GetGender(), spawn->GetTexture(), spawn->GetHelmTexture(), spawn->GetSize(), spawn->GetLoottableID(), spawn->MerchantType, 0, spawn->GetRunspeed(), 28, 28), errbuf, 0, 0, &npc_type_id)) {
				safe_delete(query);
				return false;
			}
			if(c) c->LogSQL(query);
			safe_delete_array(query);
			if(c) c->Message(0, "%s npc_type ID %i created successfully!", tmpstr, npc_type_id);
			return true;
			break;
		}
	}
	return false;
}

int32 NPC::GetEquipmentMaterial(uint8 material_slot) const
{
	if (material_slot >= MAX_MATERIALS)
		return 0;

	int inv_slot = Inventory::CalcSlotFromMaterial(material_slot);
	if (inv_slot == -1)
		return 0;
	if(equipment[inv_slot] == 0) {
		switch(material_slot) {
		case MATERIAL_HEAD:
			return helmtexture;
		case MATERIAL_CHEST:
			return texture;
		case MATERIAL_PRIMARY:
			return d_meele_texture1;
		case MATERIAL_SECONDARY:
			return d_meele_texture2;
		default:
			//they have nothing in the slot, and its not a special slot... they get nothing.
			return(0);
		}
	}
	
	//they have some loot item in this slot, pass it up to the default handler
	return(Mob::GetEquipmentMaterial(material_slot));
}

uint32 NPC::GetMaxDamage(uint8 tlevel)
{
	uint32 dmg = 0;
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

void NPC::PickPocket(Client* thief) {
	
	thief->CheckIncreaseSkill(PICK_POCKETS, nullptr, 5);
	
	//make sure were allowed to targte them:
	int olevel = GetLevel();
	if(olevel > (thief->GetLevel() + THIEF_PICKPOCKET_OVER)) {
		thief->Message(13, "You are too inexperienced to pick pocket this target");
		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		//should we check aggro
		return;
	}
	
	if(MakeRandomInt(0, 100) > 95){
		AddToHateList(thief, 50);
		Say("Stop thief!");
		thief->Message(13, "You are noticed trying to steal!");
		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		return;
	}
	
	int steal_skill = thief->GetSkill(PICK_POCKETS);
	int stealchance = steal_skill*100/(5*olevel+5);
	ItemInst* inst = 0;
	int x = 0;
	int slot[50];
	int steal_items[50];
	int charges[50];
	int money[4];
	money[0] = GetPlatinum();
	money[1] = GetGold();
	money[2] = GetSilver();
	money[3] = GetCopper();
	if (steal_skill < 125)
		money[0] = 0;
	if (steal_skill < 60)
		money[1] = 0;
	memset(slot,0,50);
	memset(steal_items,0,50);
	memset(charges,0,50);
	//Determine wheter to steal money or an item.
	bool no_coin = ((money[0] + money[1] + money[2] + money[3]) == 0);
	bool steal_item = (MakeRandomInt(0, 99) < 50 || no_coin);
	if (steal_item)
	{
		ItemList::iterator cur,end;
		cur = itemlist.begin();
		end = itemlist.end();
		for(; cur != end && x < 49; cur++) {
			ServerLootItem_Struct* citem = *cur;
			const Item_Struct* item = database.GetItem(citem->item_id);
			if (item)
			{
				inst = database.CreateItem(item, citem->charges);
				bool is_arrow = (item->ItemType == ItemTypeArrow) ? true : false;
				int slot_id = thief->GetInv().FindFreeSlot(false, true, inst->GetItem()->Size, is_arrow);
				if (/*!Equipped(item->ID) &&*/
					 !item->Magic && item->NoDrop != 0 && !inst->IsType(ItemClassContainer) && slot_id != SLOT_INVALID 
					/*&& steal_skill > item->StealSkill*/ )
				{
					slot[x] = slot_id;
					steal_items[x] = item->ID;
					if (inst->IsStackable())
						charges[x] = 1;
					else
						charges[x] = citem->charges;
					x++;
				}
			}
		}
		if (x > 0)
		{
			int random = MakeRandomInt(0, x-1);
			inst = database.CreateItem(steal_items[random], charges[random]);
			if (inst)
			{
				const Item_Struct* item = inst->GetItem();
				if (item)
				{
					if (/*item->StealSkill || */steal_skill >= stealchance)
					{
						thief->PutItemInInventory(slot[random], *inst);
						thief->SendItemPacket(slot[random], inst, ItemPacketTrade);
						RemoveItem(item->ID);
						thief->SendPickPocketResponse(this, 0, PickPocketItem, item);
					}
					else
						steal_item = false;
				}
				else
					steal_item = false;
			}
			else
				steal_item = false;
		}
		else if (!no_coin)
		{
			steal_item = false;
		}
		else
		{
			thief->Message(0, "This target's pockets are empty");
			thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		}
	}
	if (!steal_item) //Steal money
	{
		uint32 amt = MakeRandomInt(1, (steal_skill/25)+1);
		int steal_type = 0;
		if (!money[0])
		{
			steal_type = 1;
			if (!money[1])
			{
				steal_type = 2;
				if (!money[2])
				{
					steal_type = 3;
				}
			}
		}

		if (MakeRandomInt(0, 100) <= stealchance)
		{
			switch (steal_type)
			{
			case 0:{
					if (amt > GetPlatinum())
						amt = GetPlatinum();
					SetPlatinum(GetPlatinum()-amt);
					thief->AddMoneyToPP(0,0,0,amt,false);
					thief->SendPickPocketResponse(this, amt, PickPocketPlatinum);
					break;
				   }
			case 1:{
					if (amt > GetGold())
						amt = GetGold();
					SetGold(GetGold()-amt);
					thief->AddMoneyToPP(0,0,amt,0,false);
					thief->SendPickPocketResponse(this, amt, PickPocketGold);
					break;
				   }
			case 2:{
					if (amt > GetSilver())
						amt = GetSilver();
					SetSilver(GetSilver()-amt);
					thief->AddMoneyToPP(0,amt,0,0,false);
					thief->SendPickPocketResponse(this, amt, PickPocketSilver);
					break;
				   }
			case 3:{
					if (amt > GetCopper())
						amt = GetCopper();
					SetCopper(GetCopper()-amt);
					thief->AddMoneyToPP(amt,0,0,0,false);
					thief->SendPickPocketResponse(this, amt, PickPocketCopper);
					break;
				   }
			}
		}
		else
		{
			thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		}
	}
	safe_delete(inst);
}

void Mob::NPCSpecialAttacks(const char* parse, int permtag, bool reset, bool remove) {
    if(reset)
	{
		for(int i = 0; i < SPECATK_MAXNUM; i++)
		{
			SpecAttacks[i] = false;
			safe_delete(SpecAttackTimers[i]);
		}
    }

	const char* orig_parse = parse;
    while (*parse)
    {
		switch(*parse)
		{
			case 'E':
				SpecAttacks[SPECATK_ENRAGE] = (remove ? false : true);
				break;
			case 'F':
				SpecAttacks[SPECATK_FLURRY] = (remove ? false : true);
				break;
			case 'R':
				SpecAttacks[SPECATK_RAMPAGE] = (remove ? false : true);
				break;
			case 'r':
				SpecAttacks[SPECATK_AREA_RAMPAGE] = (remove ? false : true);
				break;
			case 'S':
				if(remove) {
					SpecAttacks[SPECATK_SUMMON] = false;
					safe_delete(SpecAttackTimers[SPECATK_SUMMON]);
				} else {
					SpecAttacks[SPECATK_SUMMON] = true;
					safe_delete(SpecAttackTimers[SPECATK_SUMMON]);
					SpecAttackTimers[SPECATK_SUMMON] = new Timer(6000);
					SpecAttackTimers[SPECATK_SUMMON]->Start();
				}
			break;
			case 'T':
				SpecAttacks[SPECATK_TRIPLE] = (remove ? false : true);
				break;
			case 'Q':
				//quad requires triple to work properly
				if(remove) {
					SpecAttacks[SPECATK_QUAD] = false;
				} else {
					SpecAttacks[SPECATK_TRIPLE] = true;
					SpecAttacks[SPECATK_QUAD] = true;
				}
				break;
			case 'b':
				SpecAttacks[SPECATK_BANE] = (remove ? false : true);
				break;
			case 'm':
				SpecAttacks[SPECATK_MAGICAL] = (remove ? false : true);
				break;
			case 'U':
				SpecAttacks[UNSLOWABLE] = (remove ? false : true);
				break;
			case 'M':
				SpecAttacks[UNMEZABLE] = (remove ? false : true);
				break;
			case 'C':
				SpecAttacks[UNCHARMABLE] = (remove ? false : true);
				break;
			case 'N':
				SpecAttacks[UNSTUNABLE] = (remove ? false : true);
				break;
			case 'I':
				SpecAttacks[UNSNAREABLE] = (remove ? false : true);
				break;
			case 'D':
				SpecAttacks[UNFEARABLE] = (remove ? false : true);
				break;
			case 'K':
				SpecAttacks[UNDISPELLABLE] = (remove ? false : true);
				break;
			case 'A':
				SpecAttacks[IMMUNE_MELEE] = (remove ? false : true);
				break;
			case 'B':
				SpecAttacks[IMMUNE_MAGIC] = (remove ? false : true);
				break;
			case 'f':
				SpecAttacks[IMMUNE_FLEEING] = (remove ? false : true);
				break;
			case 'O':
				SpecAttacks[IMMUNE_MELEE_EXCEPT_BANE] = (remove ? false : true);
				break;
			case 'W':
				SpecAttacks[IMMUNE_MELEE_NONMAGICAL] = (remove ? false : true);
				break;
			case 'H':
				SpecAttacks[IMMUNE_AGGRO] = (remove ? false : true);
				break;
			case 'G':
				SpecAttacks[IMMUNE_AGGRO_ON] = (remove ? false : true);
				break;
			case 'g':
				SpecAttacks[IMMUNE_CASTING_FROM_RANGE] = (remove ? false : true);
				break;
			case 'd':
				SpecAttacks[IMMUNE_FEIGN_DEATH] = (remove ? false : true);
				break;
			case 'Y':
				SpecAttacks[SPECATK_RANGED_ATK] = (remove ? false : true);
				break;
			case 'L':
				SpecAttacks[SPECATK_INNATE_DW] = (remove ? false : true);
				break;
			case 't':
				SpecAttacks[NPC_TUNNELVISION] = (remove ? false : true);
				break;
			case 'n':
				SpecAttacks[NPC_NO_BUFFHEAL_FRIENDS] = (remove ? false : true);
				break;
			case 'p':
				SpecAttacks[IMMUNE_PACIFY] = (remove ? false : true);
				break;
			case 'J':
				SpecAttacks[LEASH] = (remove ? false : true);
				break;
			case 'j':
				SpecAttacks[TETHER] = (remove ? false : true);
				break;
			case 'o':
				SpecAttacks[DESTRUCTIBLE_OBJECT] = (remove ? false : true);
				SetDestructibleObject(true);
				break;
			case 'Z':
				SpecAttacks[NO_HARM_FROM_CLIENT] = (remove ? false : true);
				break;
			case 'i':
				SpecAttacks[IMMUNE_TAUNT] = (remove ? false : true);
				break;

			default:
				break;
		}
		parse++;
    }

	if(permtag == 1 && this->GetNPCTypeID() > 0)
	{
		if(database.SetSpecialAttkFlag(this->GetNPCTypeID(), orig_parse))
		{
			LogFile->write(EQEMuLog::Normal, "NPCTypeID: %i flagged to '%s' for Special Attacks.\n",this->GetNPCTypeID(),orig_parse);
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
				if (!SpecAttacks[SPECATK_ENRAGE])
					HasAllAttacks = false;
				break;
			case 'F':
				if (!SpecAttacks[SPECATK_FLURRY])
					HasAllAttacks = false;
				break;
			case 'R':
				if (!SpecAttacks[SPECATK_RAMPAGE])
					HasAllAttacks = false;
				break;
			case 'r':
				if (!SpecAttacks[SPECATK_AREA_RAMPAGE])
					HasAllAttacks = false;
				break;
			case 'S':
				if (!SpecAttacks[SPECATK_SUMMON])
					HasAllAttacks = false;
				break;
			case 'T':
				if (!SpecAttacks[SPECATK_TRIPLE])
					HasAllAttacks = false;
				break;
			case 'Q':
				if (!SpecAttacks[SPECATK_QUAD])
					HasAllAttacks = false;
				break;
			case 'b':
				if (!SpecAttacks[SPECATK_BANE])
					HasAllAttacks = false;
				break;
			case 'm':
				if (!SpecAttacks[SPECATK_MAGICAL])
					HasAllAttacks = false;
				break;
			case 'U':
				if (!SpecAttacks[UNSLOWABLE])
					HasAllAttacks = false;
				break;
			case 'M':
				if (!SpecAttacks[UNMEZABLE])
					HasAllAttacks = false;
				break;
			case 'C':
				if (!SpecAttacks[UNCHARMABLE])
					HasAllAttacks = false;
				break;
			case 'N':
				if (!SpecAttacks[UNSTUNABLE])
					HasAllAttacks = false;
				break;
			case 'I':
				if (!SpecAttacks[UNSNAREABLE])
					HasAllAttacks = false;
				break;
			case 'D':
				if (!SpecAttacks[UNFEARABLE])
					HasAllAttacks = false;
				break;
			case 'A':
				if (!SpecAttacks[IMMUNE_MELEE])
					HasAllAttacks = false;
				break;
			case 'B':
				if (!SpecAttacks[IMMUNE_MAGIC])
					HasAllAttacks = false;
				break;
			case 'f':
				if (!SpecAttacks[IMMUNE_FLEEING])
					HasAllAttacks = false;
				break;
			case 'O':
				if (!SpecAttacks[IMMUNE_MELEE_EXCEPT_BANE])
					HasAllAttacks = false;
				break;
			case 'W':
				if (!SpecAttacks[IMMUNE_MELEE_NONMAGICAL])
					HasAllAttacks = false;
				break;
			case 'H':
				if (!SpecAttacks[IMMUNE_AGGRO])
					HasAllAttacks = false;
				break;
			case 'G':
				if (!SpecAttacks[IMMUNE_AGGRO_ON])
					HasAllAttacks = false;
				break;
			case 'g':
				if (!SpecAttacks[IMMUNE_CASTING_FROM_RANGE])
					HasAllAttacks = false;
				break;
			case 'd':
				if (!SpecAttacks[IMMUNE_FEIGN_DEATH])
					HasAllAttacks = false;
				break;
			case 'Y':
				if (!SpecAttacks[SPECATK_RANGED_ATK])
					HasAllAttacks = false;
				break;
			case 'L':
				if (!SpecAttacks[SPECATK_INNATE_DW])
					HasAllAttacks = false;
				break;
			case 't':
				if (!SpecAttacks[NPC_TUNNELVISION])
					HasAllAttacks = false;
				break;
			case 'n':
				if (!SpecAttacks[NPC_NO_BUFFHEAL_FRIENDS])
					HasAllAttacks = false;
				break;
			case 'p':
				if(!SpecAttacks[IMMUNE_PACIFY])
					HasAllAttacks = false;
				break;
			case 'J':
				if(!SpecAttacks[LEASH])
					HasAllAttacks = false;
				break;
			case 'j':
				if(!SpecAttacks[TETHER])
					HasAllAttacks = false;
				break;
			case 'o':
				if(!SpecAttacks[DESTRUCTIBLE_OBJECT])
				{
					HasAllAttacks = false;
					SetDestructibleObject(false);
				}
				break;
			case 'Z':
				if(!SpecAttacks[NO_HARM_FROM_CLIENT]){
					HasAllAttacks = false;
				}
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
	if (RuleB(Pets, UnTargetableSwarmPet)) {
		if(GetOwnerID() || GetSwarmOwner()) {
			ns->spawn.is_pet = 1;
			if (!IsCharmed() && GetOwnerID()) {
				Client *c = entity_list.GetClientByID(GetOwnerID());
				if(c)
					sprintf(ns->spawn.lastName, "%s's Pet", c->GetName());
			}
			else if (GetSwarmOwner()) {
				ns->spawn.bodytype = 11;
				if(!IsCharmed())
				{
					Client *c = entity_list.GetClientByID(GetSwarmOwner());
					if(c)
						sprintf(ns->spawn.lastName, "%s's Pet", c->GetName());
				}
			}
		}
	} else {
		if(GetOwnerID()) {
			ns->spawn.is_pet = 1;
			if (!IsCharmed() && GetOwnerID()) {
				Client *c = entity_list.GetClientByID(GetOwnerID());
				if(c)
					sprintf(ns->spawn.lastName, "%s's Pet", c->GetName());
			}
		} else
			ns->spawn.is_pet = 0;
	}

	ns->spawn.is_npc = 1;
}

void NPC::SetLevel(uint8 in_level, bool command)
{
	if(in_level > level)
		SendLevelAppearance();
	level = in_level;
	SendAppearancePacket(AT_WhoLevel, in_level);
}

void NPC::ModifyNPCStat(const char *identifier, const char *newValue)
{
	std::string id = identifier;
	std::string val = newValue;
	for(int i = 0; i < id.length(); ++i)
	{
		id[i] = std::tolower(id[i]);
	}

	if(id == "ac")
	{
		AC = atoi(val.c_str());
		return;
	}

	if(id == "str")
	{
		STR = atoi(val.c_str());
		return;
	}

	if(id == "sta")
	{
		STA = atoi(val.c_str());
		return;
	}

	if(id == "agi")
	{
		AGI = atoi(val.c_str());
		return;
	}

	if(id == "dex")
	{
		DEX = atoi(val.c_str());
		return;
	}

	if(id == "wis")
	{
		WIS = atoi(val.c_str());
		CalcMaxMana();
		return;
	}

	if(id == "int" || id == "_int")
	{
		INT = atoi(val.c_str());
		CalcMaxMana();
		return;
	}

	if(id == "cha")
	{
		CHA = atoi(val.c_str());
		return;
	}

	if(id == "max_hp")
	{
		base_hp = atoi(val.c_str());
		CalcMaxHP();
		if(cur_hp > max_hp)
			cur_hp = max_hp;
		return;
	}

    if(id == "max_mana")
	{
		npc_mana = atoi(val.c_str());
		CalcMaxMana();
		if(cur_mana > max_mana)
			cur_mana = max_mana;
		return;
	}

	if(id == "mr")
	{
		MR = atoi(val.c_str());
		return;
	}

	if(id == "fr")
	{
		FR = atoi(val.c_str());
		return;
	}

	if(id == "cr")
	{
		CR = atoi(val.c_str());
		return;
	}

	if(id == "pr")
	{
		PR = atoi(val.c_str());
		return;
	}

	if(id == "dr")
	{
		DR = atoi(val.c_str());
		return;
	}

	if(id == "runspeed")
	{
		runspeed = (float)atof(val.c_str());
		CalcBonuses();
		return;
	}

	if(id == "special_attacks")
	{
		//Added reset flag.
		NPCSpecialAttacks(val.c_str(), 0, 1);
		return;
	}

	if(id == "attack_speed")
	{
		attack_speed = (float)atof(val.c_str());
		CalcBonuses();
		return;
	}

	if(id == "atk")
	{
		ATK = atoi(val.c_str());
		return;
	}

	if(id == "accuracy")
	{
		accuracy_rating = atoi(val.c_str());
		return;
	}

	if(id == "trackable")
	{
		trackable = atoi(val.c_str());
		return;
	}

	if(id == "min_hit")
	{
		min_dmg = atoi(val.c_str());
		return;
	}

	if(id == "max_hit")
	{
		max_dmg = atoi(val.c_str());
		return;
	}

    if(id == "attack_count")
	{
		attack_count = atoi(val.c_str());
		return;
	}

	if(id == "see_invis")
	{
		see_invis = atoi(val.c_str());
		return;
	}

	if(id == "see_invis_undead")
	{
		see_invis_undead = atoi(val.c_str());
		return;
	}

	if(id == "see_hide")
	{
		see_hide = atoi(val.c_str());
		return;
	}

	if(id == "see_improved_hide")
	{
		see_improved_hide = atoi(val.c_str());
		return;
	}

	if(id == "hp_regen")
	{
		hp_regen = atoi(val.c_str());
		return;
	}

	if(id == "mana_regen")
	{
		mana_regen = atoi(val.c_str());
		return;
	}

	if(id == "level")
	{
		SetLevel(atoi(val.c_str()));
		return;
	}

	if(id == "aggro")
	{
		pAggroRange = atof(val.c_str());
		return;
	}

	if(id == "assist")
	{
		pAssistRange = atof(val.c_str());
		return;
	}

	if(id == "slow_mitigation")
	{
		slow_mitigation = atof(val.c_str());
		return;
	}
	if(id == "loottable_id")
	{
		loottable_id = atof(val.c_str());
		return;
	}
	if(id == "healscale")
	{
		healscale = atof(val.c_str());
		return;
	}
	if(id == "spellscale")
	{
		spellscale = atof(val.c_str());
		return;
	}
}

void NPC::LevelScale() {

	uint8 random_level = (MakeRandomInt(level, maxlevel));

	float scaling = (((random_level / (float)level) - 1) * (scalerate / 100.0f));
	
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

	base_hp += (int)(base_hp * scaling);
	max_hp += (int)(max_hp * scaling);
	cur_hp = max_hp;
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
		max_dmg += (int)(max_dmg * scaling / scale_adjust);
		min_dmg += (int)(min_dmg * scaling / scale_adjust);
	}

	level = random_level;

	return;
}

void NPC::CalcNPCResists() {

    if (!MR)
        MR = (GetLevel() * 11)/10;
    if (!CR)
        CR = (GetLevel() * 11)/10;
    if (!DR)
        DR = (GetLevel() * 11)/10;
    if (!FR)
        FR = (GetLevel() * 11)/10;
    if (!PR)
        PR = (GetLevel() * 11)/10;
	if (!Corrup)
        Corrup = 15;
	return;
}

void NPC::CalcNPCRegen() {

    // Fix for lazy db-updaters (regen values left at 0)
    if (GetCasterClass() != 'N' && mana_regen == 0)
        mana_regen = (GetLevel() / 10) + 4;
	else if(mana_regen < 0)
		mana_regen = 0;
	else
		mana_regen = mana_regen;
	
	// Gives low end monsters no regen if set to 0 in database. Should make low end monsters killable
	// Might want to lower this to /5 rather than 10.
	if(hp_regen == 0)
	{
		if(GetLevel() <= 6)  
            hp_regen = 1;  
       else if(GetLevel() > 6 && GetLevel() <= 10)  
            hp_regen = 2;  
       else if(GetLevel() > 10 && GetLevel() <= 15)  
            hp_regen = 3;  
       else if(GetLevel() > 15 && GetLevel() <= 20)  
            hp_regen = 5;  
       else if(GetLevel() > 20 && GetLevel() <= 30)  
            hp_regen = 7;  
       else if(GetLevel() > 30 && GetLevel() <= 35)  
            hp_regen = 9;  
       else if(GetLevel() > 35 && GetLevel() <= 40)  
            hp_regen = 12;  
       else if(GetLevel() > 40 && GetLevel() <= 45)  
            hp_regen = 18;  
       else if(GetLevel() > 45 && GetLevel() <= 50)  
            hp_regen = 21;  
       else
            hp_regen = 30;
	} else if(hp_regen < 0) {
		hp_regen = 0;
	} else
		hp_regen = hp_regen;

	return;
}

void NPC::CalcNPCDamage() {

	int AC_adjust=12;

	if (GetLevel() >= 66) {
		if (min_dmg==0)
			min_dmg = 220;
		if (max_dmg==0)
			max_dmg = ((((99000)*(GetLevel()-64))/400)*AC_adjust/10);
	}
	else if (GetLevel() >= 60 && GetLevel() <= 65){
	    if(min_dmg==0)
			min_dmg = (GetLevel()+(GetLevel()/3));
	    if(max_dmg==0)
	    	max_dmg = (GetLevel()*3)*AC_adjust/10;
	}
	else if (GetLevel() >= 51 && GetLevel() <= 59){
	    if(min_dmg==0)
	    	min_dmg = (GetLevel()+(GetLevel()/3));
	    if(max_dmg==0)
	    	max_dmg = (GetLevel()*3)*AC_adjust/10;
	}
	else if (GetLevel() >= 40 && GetLevel() <= 50) {
		if (min_dmg==0)
			min_dmg = GetLevel();
		if(max_dmg==0)
			max_dmg = (GetLevel()*3)*AC_adjust/10;
	}
	else if (GetLevel() >= 28 && GetLevel() <= 39) {
	    if (min_dmg==0)
			min_dmg = GetLevel() / 2;
	    if (max_dmg==0)
			max_dmg = ((GetLevel()*2)+2)*AC_adjust/10;
	}
	else if (GetLevel() <= 27) {
	    if (min_dmg==0)
			min_dmg=1;
	    if (max_dmg==0)
			max_dmg = (GetLevel()*2)*AC_adjust/10;
	}
	
	int clfact = GetClassLevelFactor();
	min_dmg = (min_dmg * clfact) / 220;
	max_dmg = (max_dmg * clfact) / 220;	

	return;
}


uint32 NPC::GetSpawnPointID() const
{
	if(respawn2)
	{
		return respawn2->GetID();
	}
	return 0;
}

void NPC::NPCSlotTexture(uint8 slot, uint16 texture)
{
	if (slot == 7) {
		d_meele_texture1 = texture;
	}
	else if (slot == 8) {
		d_meele_texture2 = texture;
	}
	else if (slot < 6) {
		// Reserved for texturing individual armor slots
	}
	return;
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
	return;
}

int32 NPC::CalcMaxMana() {
	if(npc_mana == 0) {
        switch (GetCasterClass()) {
		    case 'I':
			    max_mana = (((GetINT()/2)+1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
			    break;
		    case 'W':
			    max_mana = (((GetWIS()/2)+1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
			    break;
		    case 'N':
		    default:
			    max_mana = 0;
			    break;
	    }
	    if (max_mana < 0) {
		    max_mana = 0;
	    }
	
	    return max_mana;
    } else {
        switch (GetCasterClass()) {
		    case 'I':
			    max_mana = npc_mana + spellbonuses.Mana + itembonuses.Mana;
			    break;
		    case 'W':
			    max_mana = npc_mana + spellbonuses.Mana + itembonuses.Mana;
			    break;
		    case 'N':
		    default:
			    max_mana = 0;
			    break;
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

NPC_Emote_Struct* NPC::GetNPCEmote(uint16 emoteid, uint8 event_) {
	LinkedListIterator<NPC_Emote_Struct*> iterator(zone->NPCEmoteList);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		NPC_Emote_Struct* nes = iterator.GetData();
		if (emoteid == nes->emoteid && event_ == nes->event_) {
			return (nes);
		}
		iterator.Advance();
	}
	return (nullptr);
}
	
void NPC::DoNPCEmote(uint8 event_, uint16 emoteid)
{
	if(this == nullptr || emoteid == 0)
	{
		return;
	}

	NPC_Emote_Struct* nes = GetNPCEmote(emoteid,event_);
	if(nes == nullptr)
	{
		return;
	}

	if(emoteid == nes->emoteid)
	{
		if(nes->type == 1)
			this->Emote("%s",nes->text);
		else if(nes->type == 2)
			this->Shout("%s",nes->text);
		else if(nes->type == 3)
			entity_list.MessageClose_StringID(this, true, 200, 10, GENERIC_STRING, nes->text);
		else
			this->Say("%s",nes->text);
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

	int talk_check = TalkRace[GetRace() - 1];

	if (TalkRace[GetRace() - 1] > 0)
		return true;
	
	return false;
}

void NPC::PrintOutQuestItems(Client* c){
		c->Message(4,"Quest Items currently awaiting completion on %s",GetName());

		LinkedListIterator<ItemInst*> iterator(questItems);
		iterator.Reset();

		while(iterator.MoreElements())
		{
			c->Message(5,"ItemName: %s (%d) | Charges: %i",iterator.GetData()->GetItem()->Name,iterator.GetData()->GetItem()->ID,iterator.GetData()->GetCharges());
			iterator.Advance();
		}

		c->Message(4,"End of quest items list.");
}

//this is called with 'this' as the mob being looked at, and
//iOther the mob who is doing the looking. It should figure out
//what iOther thinks about 'this'
FACTION_VALUE NPC::GetReverseFactionCon(Mob* iOther) {	
	_ZP(NPC_GetReverseFactionCon);

	iOther = iOther->GetOwnerOrSelf();
	int primaryFaction= iOther->GetPrimaryFaction();
	
	//I am pretty sure that this special faction call is backwards
	//and should be iOther->GetSpecialFactionCon(this)
	if (primaryFaction < 0)
		return GetSpecialFactionCon(iOther);
	
	if (primaryFaction == 0)
		return FACTION_INDIFFERENT;
	
	//if we are a pet, use our owner's faction stuff
	Mob *own = GetOwner();
	if (own != nullptr)
		return own->GetReverseFactionCon(iOther);
	
	//make sure iOther is an npc
	//also, if we dont have a faction, then they arnt gunna think anything of us either
	if(!iOther->IsNPC() || GetPrimaryFaction() == 0)
		return(FACTION_INDIFFERENT);
	
	//if we get here, iOther is an NPC too
	
	//otherwise, employ the npc faction stuff
	//so we need to look at iOther's faction table to see
	//what iOther thinks about our primary faction
	return(iOther->CastToNPC()->CheckNPCFactionAlly(GetPrimaryFaction()));
}

//Look through our faction list and return a faction con based 
//on the npc_value for the other person's primary faction in our list.
FACTION_VALUE NPC::CheckNPCFactionAlly(int32 other_faction) {
	list<struct NPCFaction*>::iterator cur,end;
	cur = faction_list.begin();
	end = faction_list.end();
	for(; cur != end; cur++) {
		struct NPCFaction* fac = *cur;
		if ((int32)fac->factionID == other_faction) {
			if (fac->npc_value > 0)
				return FACTION_ALLY;
			else if (fac->npc_value < 0)
				return FACTION_SCOWLS;
			else
				return FACTION_INDIFFERENT;
		}
	}
	return FACTION_INDIFFERENT;
}

bool NPC::IsFactionListAlly(uint32 other_faction) {
	return(CheckNPCFactionAlly(other_faction) == FACTION_ALLY);
}

int NPC::GetScore()
{
    int lv = min(70, (int)GetLevel());
    int basedmg = (lv*2)*(1+(lv / 100)) - (lv / 2);
    int minx = 0;
    int basehp = 0;
    int hpcontrib = 0;
    int dmgcontrib = 0;
    int spccontrib = 0;
    int hp = GetMaxHP();
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
    final = max(1, final);
    final = min(100, final);
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
