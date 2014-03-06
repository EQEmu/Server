/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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
#include "../common/debug.h"
#include "../common/spdat.h"
#include "masterentity.h"
#include "../common/packet_dump.h"
#include "../common/moremath.h"
#include "../common/Item.h"
#include "worldserver.h"
#include "../common/skills.h"
#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/rulesys.h"
#include <math.h>
#include <assert.h>
#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif

#include "StringIDs.h"
#include "QuestParserCollection.h"

extern Zone* zone;
extern volatile bool ZoneLoaded;
extern WorldServer worldserver;


// the spell can still fail here, if the buff can't stack
// in this case false will be returned, true otherwise
bool Mob::SpellEffect(Mob* caster, uint16 spell_id, float partial)
{
	int caster_level, buffslot, effect, effect_value, i;
	ItemInst *SummonedItem=nullptr;
#ifdef SPELL_EFFECT_SPAM
#define _EDLEN	200
	char effect_desc[_EDLEN];
#endif

	if(!IsValidSpell(spell_id))
		return false;

	const SPDat_Spell_Struct &spell = spells[spell_id];

	bool c_override = false;
	if(caster && caster->IsClient() && GetCastedSpellInvSlot() > 0)
	{
		const ItemInst* inst = caster->CastToClient()->GetInv().GetItem(GetCastedSpellInvSlot());
		if(inst)
		{
			if(inst->GetItem()->Click.Level > 0)
			{
				caster_level = inst->GetItem()->Click.Level;
				c_override = true;
			}
			else
			{
				caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
			}
		}
		else
			caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
	}
	else
		caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);

	if(c_override)
	{
		int durat = CalcBuffDuration(caster, this, spell_id, caster_level);
		if(durat > 0)
		{
			buffslot = AddBuff(caster, spell_id, durat, caster_level);
			if(buffslot == -1)	// stacking failure
				return false;
		}
		else
		{
			buffslot = -2;
		}
	}
	else
	{
		if(IsBuffSpell(spell_id)){
			if(IsEffectInSpell(spell_id, SE_BindSight))
			{
				if(caster)
				{
					buffslot = caster->AddBuff(caster, spell_id);
				}
				else
					buffslot = -1;
			}
			else
			{
				buffslot = AddBuff(caster, spell_id);
			}
			if(buffslot == -1)	// stacking failure
				return false;
		} else {
			buffslot = -2;	//represents not a buff I guess
		}
	}

#ifdef SPELL_EFFECT_SPAM
		Message(0, "You are affected by spell '%s' (id %d)", spell.name, spell_id);
		if(buffslot >= 0)
		{
			Message(0, "Buff slot:  %d  Duration:  %d tics", buffslot, buffs[buffslot].ticsremaining);
		}
#endif

	if(buffslot >= 0)
	{
		buffs[buffslot].melee_rune = 0;
		buffs[buffslot].magic_rune = 0;
		buffs[buffslot].numhits = 0;

		if(IsClient() && CastToClient()->GetClientVersionBit() & BIT_UnderfootAndLater)
		{
			EQApplicationPacket *outapp = MakeBuffsPacket(false);
			CastToClient()->FastQueuePacket(&outapp);
		}
	}

	if(IsNPC())
	{
		std::vector<void*> args;
		args.push_back(&buffslot);
		int i = parse->EventSpell(EVENT_SPELL_EFFECT_NPC, CastToNPC(), nullptr, spell_id, caster ? caster->GetID() : 0, &args);
		if(i != 0){
			CalcBonuses();
			return true;
		}
	}
	else if(IsClient())
	{
		std::vector<void*> args;
		args.push_back(&buffslot);
		int i = parse->EventSpell(EVENT_SPELL_EFFECT_CLIENT, nullptr, CastToClient(), spell_id, caster ? caster->GetID() : 0, &args);
		if(i != 0){
			CalcBonuses();
			return true;
		}
	}

	if(spells[spell_id].viral_targets > 0) {
		if(!viral_timer.Enabled())
			viral_timer.Start(1000);

		has_virus = true;
		for(int i = 0; i < MAX_SPELL_TRIGGER*2; i+=2)
		{
			if(!viral_spells[i])
			{
				viral_spells[i] = spell_id;
				viral_spells[i+1] = caster->GetID();
				break;
			}
		}
	}

	if(spells[spell_id].numhits > 0 && buffslot >= 0){

		int numhit = spells[spell_id].numhits;

		if (caster && caster->IsClient()){
			numhit += numhit*caster->CastToClient()->GetFocusEffect(focusFcLimitUse, spell_id)/100;
			numhit += caster->CastToClient()->GetFocusEffect(focusIncreaseNumHits, spell_id);
		}
		
		buffs[buffslot].numhits = numhit;
	}

	// iterate through the effects in the spell
	for (i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spell_id, i))
			continue;

		effect = spell.effectid[i];
		effect_value = CalcSpellEffectValue(spell_id, i, caster_level, caster ? caster : this);

		if(spell_id == SPELL_LAY_ON_HANDS && caster && caster->GetAA(aaImprovedLayOnHands))
			effect_value = GetMaxHP();

#ifdef SPELL_EFFECT_SPAM
		effect_desc[0] = 0;
#endif

		switch(effect)
		{
			case SE_CurrentHP:	// nukes, heals; also regen/dot if a buff
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints: %+i", effect_value);
#endif
				// SE_CurrentHP is calculated at first tick if its a dot/buff
				if (buffslot >= 0)
					break;

				// for offensive spells check if we have a spell rune on
				int32 dmg = effect_value;
				if(dmg < 0)
				{
					if (!PassCastRestriction(false, spells[spell_id].base2[i], true))
						break;

					// take partial damage into account
					dmg = (int32) (dmg * partial / 100);

					//handles AAs and what not...
					if(caster)
						dmg = caster->GetActSpellDamage(spell_id, dmg, this);
					
					dmg = -dmg;
					Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
				}
				else if(dmg > 0) {
					//healing spell...

					if (!PassCastRestriction(false, spells[spell_id].base2[i], false))
						break;

					if(caster)
						dmg = caster->GetActSpellHealing(spell_id, dmg, this);

					HealDamage(dmg, caster);
				}

#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints: %+i  actual: %+i", effect_value, dmg);
#endif
				break;
			}

			case SE_CurrentHPOnce:	// used in buffs usually, see Courage
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints Once: %+i", effect_value);
#endif

				int32 dmg = effect_value;
				if (spell_id == 2751 && caster) //Manaburn
				{
					dmg = caster->GetMana()*-3;
					caster->SetMana(0);
				} else if (spell_id == 2755 && caster) //Lifeburn
				{
					dmg = caster->GetHP()*-15/10;
					caster->SetHP(1);
					if(caster->IsClient()){
						caster->CastToClient()->SetFeigned(true);
						caster->SendAppearancePacket(AT_Anim, 115);
					}
				}

				//do any AAs apply to these spells?
				if(dmg < 0) {
					dmg = -dmg;
					Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
				} else {
					HealDamage(dmg, caster);
				}
				break;
			}

			case SE_PercentalHeal:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Percental Heal: %+i (%d%% max)", spell.max[i], effect_value);
#endif
				//im not 100% sure about this implementation.
				//the spell value forumula dosent work for these... at least spell 3232 anyways
				int32 val = spell.max[i];

				if(caster)
					val = caster->GetActSpellHealing(spell_id, val, this);

				int32 mhp = GetMaxHP();
				int32 cap = mhp * spell.base[i] / 100;

				if(cap < val)
					val = cap;

				if(val > 0)
					HealDamage(val, caster);

				break;
			}

			case SE_CompleteHeal:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Complete Heal");
#endif
				//make sure they are not allready affected by this...
				//I think that is the point of making this a buff.
				//this is in the wrong spot, it should be in the immune
				//section so the buff timer does not get refreshed!

				int i;
				bool inuse = false;
				uint32 buff_count = GetMaxTotalSlots();
				for(i = 0; i < buff_count; i++) {
					if(buffs[i].spellid == spell_id && i != buffslot) {
						Message(0, "You must wait before you can be affected by this spell again.");
						inuse = true;
						break;
					}
				}
				if(inuse)
					break;

				Heal();
				break;
			}

			case SE_CurrentMana:
			{
				// Bards don't get mana from effects, good or bad.
				if(GetClass() == BARD)
					break;
				if(IsManaTapSpell(spell_id)) {
					if(GetCasterClass() != 'N') {
#ifdef SPELL_EFFECT_SPAM
						snprintf(effect_desc, _EDLEN, "Current Mana: %+i", effect_value);
#endif
						SetMana(GetMana() + effect_value);
						caster->SetMana(caster->GetMana() + abs(effect_value));
						
						if (effect_value < 0)
							TryTriggerOnValueAmount(false, true);
#ifdef SPELL_EFFECT_SPAM
						caster->Message(0, "You have gained %+i mana!", effect_value);
#endif
					}
				}
				else {
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Mana: %+i", effect_value);
#endif
				if (buffslot >= 0)
					break;

				SetMana(GetMana() + effect_value);
				if (effect_value < 0)
					TryTriggerOnValueAmount(false, true);
				}

				break;
			}

			case SE_CurrentManaOnce:
			{
				// Bards don't get mana from effects, good or bad.
				if(GetClass() == BARD)
					break;
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Mana Once: %+i", effect_value);
#endif
				SetMana(GetMana() + effect_value);
				break;
			}

			case SE_Translocate:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Translocate: %s %d %d %d heading %d",
					spell.teleport_zone, spell.base[1], spell.base[0],
					spell.base[2], spell.base[3]
				);
#endif
				if(IsClient())
				{

					if(caster)
						CastToClient()->SendOPTranslocateConfirm(caster, spell_id);

				}
				break;
			}

			case SE_Succor:
			{
				float x, y, z, heading;
				const char *target_zone;

				x = spell.base[1];
				y = spell.base[0];
				z = spell.base[2];
				heading = spell.base[3];

				if(!strcmp(spell.teleport_zone, "same"))
				{
					target_zone = 0;
				}
				else
				{
					target_zone = spell.teleport_zone;
					if(IsNPC() && target_zone != zone->GetShortName()){
						if(!GetOwner()){
							CastToNPC()->Depop();
							break;
						}else{
							if(!GetOwner()->IsClient())
								CastToNPC()->Depop();
								break;
						}
					}
				}

				if(IsClient())
				{
					// Below are the spellid's for known evac/succor spells that send player
					// to the current zone's safe points.

					// Succor = 1567
					// Lesser Succor = 2183
					// Evacuate = 1628
					// Lesser Evacuate = 2184
					// Decession = 2558
					// Greater Decession = 3244
					// Egress = 1566

					if(!target_zone) {
#ifdef SPELL_EFFECT_SPAM
						LogFile->write(EQEMuLog::Debug, "Succor/Evacuation Spell In Same Zone.");
#endif
						if(IsClient())
							CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading, 0, EvacToSafeCoords);
						else
							GMMove(x, y, z, heading);
					}
					else {
#ifdef SPELL_EFFECT_SPAM
						LogFile->write(EQEMuLog::Debug, "Succor/Evacuation Spell To Another Zone.");
#endif
						if(IsClient())
							CastToClient()->MovePC(target_zone, x, y, z, heading);
					}
				}

				break;
			}
			case SE_YetAnotherGate: //Shin: Used on Teleport Bind.
			case SE_Teleport:	// gates, rings, circles, etc
			case SE_Teleport2:
			{
				float x, y, z, heading;
				const char *target_zone;

				x = spell.base[1];
				y = spell.base[0];
				z = spell.base[2];
				heading = spell.base[3];

				if(!strcmp(spell.teleport_zone, "same"))
				{
					target_zone = 0;
				}
				else
				{
					target_zone = spell.teleport_zone;

					if(IsNPC() && target_zone != zone->GetShortName()){
						if(!GetOwner()){
							CastToNPC()->Depop();
							break;
						}else{
							if(!GetOwner()->IsClient())
								CastToNPC()->Depop();
								break;
						}
					}
				}

				if (effect == SE_YetAnotherGate && caster->IsClient())
				{ //Shin: Teleport Bind uses caster's bind point
					x = caster->CastToClient()->GetBindX();
					y = caster->CastToClient()->GetBindY();
					z = caster->CastToClient()->GetBindZ();
					heading = caster->CastToClient()->GetBindHeading();
					//target_zone = caster->CastToClient()->GetBindZoneId(); target_zone doesn't work due to const char
					CastToClient()->MovePC(caster->CastToClient()->GetBindZoneID(), 0, x, y, z, heading);
					break;
				}

#ifdef SPELL_EFFECT_SPAM
				const char *efstr = "Teleport";
				if(effect == SE_Teleport)
					efstr = "Teleport v1";
				else if(effect == SE_Teleport2)
					efstr = "Teleport v2";
				else if(effect == SE_Succor)
					efstr = "Succor";

				snprintf(effect_desc, _EDLEN,
					"%s: %0.2f, %0.2f, %0.2f heading %0.2f in %s",
					efstr, x, y, z, heading, target_zone ? target_zone : "same zone"
				);
#endif
				if(IsClient())
				{
					if(!target_zone)
						CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading);
					else
						CastToClient()->MovePC(target_zone, x, y, z, heading);
				}
				else{
					if(!target_zone)
						GMMove(x, y, z, heading);
				}
				break;
			}

			case SE_Invisibility:
			case SE_Invisibility2:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Invisibility");
#endif
				SetInvisible(spell.base[i]);
				break;
			}

			case SE_InvisVsAnimals:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Invisibility to Animals");
#endif
				invisible_animals = true;
				break;
			}

			case SE_InvisVsUndead2:
			case SE_InvisVsUndead:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Invisibility to Undead");
#endif
				invisible_undead = true;
				break;
			}
			case SE_SeeInvis:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "See Invisible");
#endif
				see_invis = spell.base[i];
				break;
			}

			case SE_FleshToBone:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Flesh To Bone");
#endif
				if(IsClient()){
					ItemInst* transI = CastToClient()->GetInv().GetItem(SLOT_CURSOR);
					if(transI && transI->IsType(ItemClassCommon) && transI->IsStackable()){
						uint32 fcharges = transI->GetCharges();
							//Does it sound like meat... maybe should check if it looks like meat too...
							if(strstr(transI->GetItem()->Name, "meat") ||
								strstr(transI->GetItem()->Name, "Meat") ||
								strstr(transI->GetItem()->Name, "flesh") ||
								strstr(transI->GetItem()->Name, "Flesh") ||
								strstr(transI->GetItem()->Name, "parts") ||
								strstr(transI->GetItem()->Name, "Parts")){
								CastToClient()->DeleteItemInInventory(SLOT_CURSOR, fcharges, true);
								CastToClient()->SummonItem(13073, fcharges);
							}
							else{
								Message(13, "You can only transmute flesh to bone.");
							}
						}
					else{
						Message(13, "You can only transmute flesh to bone.");
					}
				}
				break;
			}

			case SE_GroupFearImmunity:{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Group Fear Immunity");
#endif
				//Added client messages to give some indication this effect is active.
				uint32 group_id_caster = 0;
				uint32 time = spell.base[i]*10;
				if(caster->IsClient())
				{
					if(caster->IsGrouped())
					{
						group_id_caster = GetGroup()->GetID();
					}
					else if(caster->IsRaidGrouped())
					{
						group_id_caster = (GetRaid()->GetGroup(CastToClient()) == 0xFFFF) ? 0 : (GetRaid()->GetGroup(CastToClient()) + 1);
					}
				}
				if(group_id_caster){
					Group *g = entity_list.GetGroupByID(group_id_caster);
					uint32 time = spell.base[i]*10;
					if(g){
						for(int gi=0; gi < 6; gi++){
							if(g->members[gi] && g->members[gi]->IsClient())
							{
								g->members[gi]->CastToClient()->EnableAAEffect(aaEffectWarcry , time);
								if (g->members[gi]->GetID() != caster->GetID())
									g->members[gi]->Message(13, "You hear the war cry.");
								else
									Message(13, "You let loose a fierce war cry.");
							}
						}
					}
				}

				else{
					CastToClient()->EnableAAEffect(aaEffectWarcry , time);
					Message(13, "You let loose a fierce war cry.");
				}

				break;
			}

			case SE_AddFaction:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Faction Mod: %+i", effect_value);
#endif
				// EverHood
				if(caster && GetPrimaryFaction()>0) {
					caster->AddFactionBonus(GetPrimaryFaction(),effect_value);
				}
				break;
			}

			case SE_Stun:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Stun: %d msec", effect_value);
#endif
				//Typically we check for immunities else where but since stun immunities are different and only
				//Block the stun part and not the whole spell, we do it here, also do the message here so we wont get the message on a resist
				int max_level = spell.max[i];
				//max_level of 0 means we assume a default of 55.
				if (max_level == 0)
					max_level = RuleI(Spells, BaseImmunityLevel);
				// NPCs get to ignore max_level for their spells.
				// Ignore if spell is beneficial (ex. Harvest)
				if (IsDetrimentalSpell(spell.id) && (GetSpecialAbility(UNSTUNABLE) ||
						((GetLevel() > max_level) && caster && (!caster->IsNPC() ||
						(caster->IsNPC() && !RuleB(Spells, NPCIgnoreBaseImmunity))))))
				{
					caster->Message_StringID(MT_SpellFailure, IMMUNE_STUN);
				} else {
					int stun_resist = itembonuses.StunResist+spellbonuses.StunResist;
					if (IsClient())
						stun_resist += aabonuses.StunResist;

					if (stun_resist <= 0 || MakeRandomInt(0,99) >= stun_resist) {
						mlog(COMBAT__HITS, "Stunned. We had %d percent resist chance.", stun_resist);
						
						if (caster->IsClient())
							effect_value += effect_value*caster->CastToClient()->GetFocusEffect(focusFcStunTimeMod, spell_id)/100;

						Stun(effect_value);
					} else {
						if (IsClient())
							Message_StringID(MT_Stun, SHAKE_OFF_STUN);

						mlog(COMBAT__HITS, "Stun Resisted. We had %d percent resist chance.", stun_resist);
					}
				}
				break;
			}

			case SE_Charm:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Charm: %+i (up to lvl %d)", effect_value, spell.max[i]);
#endif

				if (!caster)	// can't be someone's pet unless we know who that someone is
					break;

				if(IsNPC())
				{
					CastToNPC()->SaveGuardSpotCharm();
				}
				InterruptSpell();
				entity_list.RemoveDebuffs(this);
				entity_list.RemoveFromTargets(this);
				WipeHateList();

				if (IsClient() && caster->IsClient()) {
					caster->Message(0, "Unable to cast charm on a fellow player.");
					BuffFadeByEffect(SE_Charm);
					break;
				} else if(IsCorpse()) {
					caster->Message(0, "Unable to cast charm on a corpse.");
					BuffFadeByEffect(SE_Charm);
					break;
				} else if(caster->GetPet() != nullptr && caster->IsClient()) {
					caster->Message(0, "You cannot charm something when you already have a pet.");
					BuffFadeByEffect(SE_Charm);
					break;
				} else if(GetOwner()) {
					caster->Message(0, "You cannot charm someone else's pet!");
					BuffFadeByEffect(SE_Charm);
					break;
				}

				Mob *my_pet = GetPet();
				if(my_pet)
				{
					my_pet->Kill();
				}

				caster->SetPet(this);
				SetOwnerID(caster->GetID());
				SetPetOrder(SPO_Follow);

				if(caster->IsClient()){
					EQApplicationPacket *app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
					Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
					ps->owner_id = caster->GetID();
					ps->pet_id = this->GetID();
					ps->command = 1;
					entity_list.QueueClients(this, app);
					safe_delete(app);
					SendPetBuffsToClient();
					SendAppearancePacket(AT_Pet, caster->GetID(), true, true);
				}

				if (IsClient())
				{
					AI_Start();
					SendAppearancePacket(14, 100, true, true);
				} else if(IsNPC()) {
					CastToNPC()->SetPetSpellID(0);	//not a pet spell.
				}

				bool bBreak = false;

				// define spells with fixed duration
				// charm spells with -1 in field 209 are all of fixed duration, so lets use that instead of spell_ids
				if(spells[spell_id].powerful_flag == -1)
					bBreak = true;

				if (!bBreak)
				{
					int resistMod = partial + (GetCHA()/25);
					resistMod = resistMod > 100 ? 100 : resistMod;
					buffs[buffslot].ticsremaining = resistMod * buffs[buffslot].ticsremaining / 100;
				}

				if(IsClient())
				{
					if(buffs[buffslot].ticsremaining > RuleI(Character, MaxCharmDurationForPlayerCharacter))
						buffs[buffslot].ticsremaining = RuleI(Character, MaxCharmDurationForPlayerCharacter);
				}

				break;
			}


			case SE_SenseDead:
			case SE_SenseSummoned:
			case SE_SenseAnimals:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Sense Target: %+i", effect_value);
#endif
				if(IsClient())
				{
					CastToClient()->SetSenseExemption(true);

					if(CastToClient()->GetClientVersionBit() & BIT_SoDAndLater)
					{
						bodyType bt = BT_Undead;

						int MessageID = SENSE_UNDEAD;

						if(effect == SE_SenseSummoned)
						{
							bt = BT_Summoned;
							MessageID = SENSE_SUMMONED;
						}
						else if(effect == SE_SenseAnimals)
						{
							bt = BT_Animal;
							MessageID = SENSE_ANIMAL;
						}

						Mob *ClosestMob = entity_list.GetClosestMobByBodyType(this, bt);

						if(ClosestMob)
						{
							Message_StringID(MT_Spells, MessageID);
							SetHeading(CalculateHeadingToTarget(ClosestMob->GetX(), ClosestMob->GetY()));
							SetTarget(ClosestMob);
							CastToClient()->SendTargetCommand(ClosestMob->GetID());
							SendPosUpdate(2);
						}
						else
							Message_StringID(clientMessageError, SENSE_NOTHING);
					}
				}
				break;
			}

			case SE_Fear:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Fear: %+i", effect_value);
#endif
				//use resistance value for duration...
				buffs[buffslot].ticsremaining = ((buffs[buffslot].ticsremaining * partial) / 100);

				if(IsClient())
				{
					if(buffs[buffslot].ticsremaining > RuleI(Character, MaxFearDurationForPlayerCharacter))
						buffs[buffslot].ticsremaining = RuleI(Character, MaxFearDurationForPlayerCharacter);
				}

				if(RuleB(Combat, EnableFearPathing)){
					if(IsClient())
					{
						AI_Start();
						animation = GetRunspeed() * 21; //set our animation to match our speed about
					}

					CalculateNewFearpoint();
					if(curfp)
					{
						break;
					}
				}
				else
				{
					Stun(buffs[buffslot].ticsremaining * 6000 - (6000 - tic_timer.GetRemainingTime()));
				}
				break;
			}

			case SE_BindAffinity:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Bind Affinity");
#endif
				if (IsClient())
				{
					if(CastToClient()->GetGM() || RuleB(Character, BindAnywhere))
					{
						EQApplicationPacket *action_packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
						Action_Struct* action = (Action_Struct*) action_packet->pBuffer;
						EQApplicationPacket *message_packet = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
						CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;

						action->target = GetID();
						action->source = caster ? caster->GetID() : GetID();
						action->level = 65;
						action->instrument_mod = 10;
						action->sequence = (GetHeading() * 12345 / 2);
						action->type = 231;
						action->spell = spell_id;
						action->buff_unknown = 4;

						cd->target = action->target;
						cd->source = action->source;
						cd->type = action->type;
						cd->spellid = action->spell;
						cd->sequence = action->sequence;

						CastToClient()->QueuePacket(action_packet);
						if(caster->IsClient() && caster != this)
							caster->CastToClient()->QueuePacket(action_packet);

						CastToClient()->QueuePacket(message_packet);
						if(caster->IsClient() && caster != this)
							caster->CastToClient()->QueuePacket(message_packet);

						CastToClient()->SetBindPoint();
						Save();
						safe_delete(action_packet);
						safe_delete(message_packet);
					}
					else
					{
						if(!zone->CanBind())
						{
							Message_StringID(MT_SpellFailure, CANNOT_BIND);
							break;
						}
						if(!zone->IsCity())
						{
							if(caster != this)
							{
								Message_StringID(MT_SpellFailure, CANNOT_BIND);
								break;
							}
							else
							{
								EQApplicationPacket *action_packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
								Action_Struct* action = (Action_Struct*) action_packet->pBuffer;
								EQApplicationPacket *message_packet = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
								CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;

								action->target = GetID();
								action->source = caster ? caster->GetID() : GetID();
								action->level = 65;
								action->instrument_mod = 10;
								action->sequence = (GetHeading() * 12345 / 2);
								action->type = 231;
								action->spell = spell_id;
								action->buff_unknown = 4;

								cd->target = action->target;
								cd->source = action->source;
								cd->type = action->type;
								cd->spellid = action->spell;
								cd->sequence = action->sequence;

								CastToClient()->QueuePacket(action_packet);
								if(caster->IsClient() && caster != this)
									caster->CastToClient()->QueuePacket(action_packet);

								CastToClient()->QueuePacket(message_packet);
								if(caster->IsClient() && caster != this)
									caster->CastToClient()->QueuePacket(message_packet);

								CastToClient()->SetBindPoint();
								Save();
								safe_delete(action_packet);
								safe_delete(message_packet);
							}
						}
						else
						{
							EQApplicationPacket *action_packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
							Action_Struct* action = (Action_Struct*) action_packet->pBuffer;
							EQApplicationPacket *message_packet = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
							CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;

							action->target = GetID();
							action->source = caster ? caster->GetID() : GetID();
							action->level = 65;
							action->instrument_mod = 10;
							action->sequence = (GetHeading() * 12345 / 2);
							action->type = 231;
							action->spell = spell_id;
							action->buff_unknown = 4;

							cd->target = action->target;
							cd->source = action->source;
							cd->type = action->type;
							cd->spellid = action->spell;
							cd->sequence = action->sequence;

							CastToClient()->QueuePacket(action_packet);
							if(caster->IsClient() && caster != this)
								caster->CastToClient()->QueuePacket(action_packet);

							CastToClient()->QueuePacket(message_packet);
							if(caster->IsClient() && caster != this)
								caster->CastToClient()->QueuePacket(message_packet);

							CastToClient()->SetBindPoint();
							Save();
							safe_delete(action_packet);
							safe_delete(message_packet);
						}
					}
				}
				break;
			}

			case SE_Gate:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Gate");
#endif
				if(!spellbonuses.AntiGate)
					Gate();
				break;
			}

			case SE_CancelMagic:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Cancel Magic: %d", effect_value);
#endif
				if(GetSpecialAbility(UNDISPELLABLE)){
					caster->Message_StringID(MT_SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}

				uint32 buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if(	buffs[slot].spellid != SPELL_UNKNOWN &&
						spells[buffs[slot].spellid].dispel_flag == 0 &&
						!IsDiscipline(buffs[slot].spellid))
					{
						if (TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
							BuffFadeBySlot(slot);
							slot = buff_count;
						}
					}
				}
				break;
			}

			case SE_DispelDetrimental:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Dispel Detrimental: %d", effect_value);
#endif
				if(GetSpecialAbility(UNDISPELLABLE)){
					caster->Message_StringID(MT_SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}

				uint32 buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (buffs[slot].spellid != SPELL_UNKNOWN &&
						IsDetrimentalSpell(buffs[slot].spellid) &&
						spells[buffs[slot].spellid].dispel_flag == 0)
					{
						if (TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
							BuffFadeBySlot(slot);
							slot = buff_count;
						}
					}
				}
				break;
			}

			case SE_DispelBeneficial:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Dispel Beneficial: %d", effect_value);
#endif
				if(GetSpecialAbility(UNDISPELLABLE)){
					caster->Message_StringID(MT_SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}

				uint32 buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (buffs[slot].spellid != SPELL_UNKNOWN &&
						IsBeneficialSpell(buffs[slot].spellid) &&
						spells[buffs[slot].spellid].dispel_flag == 0)
					{
						if (TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
							BuffFadeBySlot(slot);
							slot = buff_count;
						}
					}
				}
				break;
			}

			case SE_Purify:
			{
				//Attempt to remove all Deterimental buffs.
				uint32 buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (buffs[slot].spellid != SPELL_UNKNOWN &&
						IsDetrimentalSpell(buffs[slot].spellid))
					{
						if (TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
							BuffFadeBySlot(slot);
						}
					}
				}
				break;
			}

			case SE_Mez:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Mesmerize");
#endif
				Mesmerize();
				break;
			}

			case SE_SummonItem:
			{
				const Item_Struct *item = database.GetItem(spell.base[i]);
#ifdef SPELL_EFFECT_SPAM
				const char *itemname = item ? item->Name : "*Unknown Item*";
				snprintf(effect_desc, _EDLEN, "Summon Item: %s (id %d)", itemname, spell.base[i]);
#endif
				if (!item) {
					Message(13, "Unable to summon item %d. Item not found.", spell.base[i]);
				} else if (IsClient()) {
					Client *c = CastToClient();
					if (c->CheckLoreConflict(item)) {
						c->DuplicateLoreMessage(spell.base[i]);
					} else {
						int charges;
						if (item->Stackable)
							charges = (spell.formula[i] > item->StackSize) ? item->StackSize : spell.formula[i];
						else if (item->MaxCharges) // mod rods etc
							charges = item->MaxCharges;
						else
							charges = 1;

						if (charges < 1)
							charges = 1;

						if (SummonedItem) {
							c->PushItemOnCursor(*SummonedItem);
							c->SendItemPacket(SLOT_CURSOR, SummonedItem, ItemPacketSummonItem);
							safe_delete(SummonedItem);
						}
						SummonedItem = database.CreateItem(spell.base[i], charges);
					}
				}

				break;
			}
			case SE_SummonItemIntoBag:
			{
				const Item_Struct *item = database.GetItem(spell.base[i]);
#ifdef SPELL_EFFECT_SPAM
				const char *itemname = item ? item->Name : "*Unknown Item*";
				snprintf(effect_desc, _EDLEN, "Summon Item In Bag: %s (id %d)", itemname, spell.base[i]);
#endif
				uint8 slot;

				if (!SummonedItem || !SummonedItem->IsType(ItemClassContainer)) {
					if (caster)
						caster->Message(13, "SE_SummonItemIntoBag but no bag has been summoned!");
				} else if ((slot = SummonedItem->FirstOpenSlot()) == 0xff) {
					if (caster)
						caster->Message(13, "SE_SummonItemIntoBag but no room in summoned bag!");
				} else if (IsClient()) {
					if (CastToClient()->CheckLoreConflict(item)) {
						CastToClient()->DuplicateLoreMessage(spell.base[i]);
					} else {
						int charges;

						if (item->Stackable)
							charges = (spell.formula[i] > item->StackSize) ? item->StackSize : spell.formula[i];
						else if (item->MaxCharges) // mod rods, not sure if there are actual examples of this for IntoBag
							charges = item->MaxCharges;
						else
							charges = 1;

						if (charges < 1)
							charges = 1;

						ItemInst *SubItem = database.CreateItem(spell.base[i], charges);
						if (SubItem != nullptr) {
							SummonedItem->PutItem(slot, *SubItem);
							safe_delete(SubItem);
						}
					}
				}

				break;
			}

			case SE_SummonBSTPet:
			case SE_NecPet:
			case SE_SummonPet:
			case SE_Familiar:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Summon %s: %s", (effect==SE_Familiar)?"Familiar":"Pet", spell.teleport_zone);
#endif
				if(GetPet())
				{
					Message_StringID(MT_Shout, ONLY_ONE_PET);
				}
				else
				{
					MakePet(spell_id, spell.teleport_zone);
				}
				break;
			}

			case SE_DivineAura:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Invulnerability");
#endif
				if(spell_id==4789) // Touch of the Divine - Divine Save
					buffs[buffslot].ticsremaining = spells[spell_id].buffduration; // Prevent focus/aa buff extension

				SetInvul(true);
				break;
			}

			case SE_ShadowStep:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Shadow Step: %d", effect_value);
#endif
				if(IsNPC())	// see Song of Highsun - sends mob home
				{
					Gate();
				}
				// shadow step is handled by client already, nothing required
				break;
			}

			case SE_Blind:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Blind: %+i", effect_value);
#endif
				if (spells[spell_id].base[i] == 1)
					BuffFadeByEffect(SE_Blind);
				// handled by client
				// TODO: blind flag?
				break;
			}

			case SE_Rune:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Melee Absorb Rune: %+i", effect_value);
#endif
				effect_value = ApplySpellEffectiveness(caster, spell_id, effect_value);
				buffs[buffslot].melee_rune = effect_value;
				SetHasRune(true);
				break;
			}

			case SE_AbsorbMagicAtt:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Spell Absorb Rune: %+i", effect_value);
#endif
				if(effect_value > 0) {
				buffs[buffslot].magic_rune = effect_value;
					SetHasSpellRune(true);
				}
				break;
			}

			case SE_MitigateMeleeDamage:
			{
				buffs[buffslot].melee_rune = GetPartialMeleeRuneAmount(spell_id);
				SetHasPartialMeleeRune(true);
				break;
			}

			case SE_MeleeThresholdGuard:
			{
				buffs[buffslot].melee_rune = spells[spell_id].max[i];
				break;
			}

			case SE_SpellThresholdGuard:
			{
				buffs[buffslot].magic_rune = spells[spell_id].max[i];
				break;
			}

			case SE_MitigateSpellDamage:
			{
				buffs[buffslot].magic_rune = GetPartialMagicRuneAmount(spell_id);
				SetHasPartialSpellRune(true);
				break;
			}

			case SE_MitigateDotDamage:
			{
				buffs[buffslot].dot_rune = spells[spell_id].max[i];
				break;
			}

			case SE_TriggerMeleeThreshold:
			{
				buffs[buffslot].melee_rune = spells[spell_id].base2[i];
				break;
			}
			
			case SE_TriggerSpellThreshold:
			{
				buffs[buffslot].magic_rune = spells[spell_id].base2[i];
				break;
			}

			case SE_DistanceRemoval:
			{
				buffs[buffslot].caston_x = int(GetX());	
				buffs[buffslot].caston_y = int(GetY());	
				buffs[buffslot].caston_z = int(GetZ());	
				break;
			}
			
			case SE_Levitate:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Levitate");
#endif
				//this sends the levitate packet to everybody else
				//who does not otherwise receive the buff packet.
				SendAppearancePacket(AT_Levitate, 2, true, true);
				break;
			}

			case SE_DeathSave: {

				int16 mod = 0;
				
				if(caster) {
					mod =	caster->aabonuses.UnfailingDivinity +
							caster->itembonuses.UnfailingDivinity +
							caster->spellbonuses.UnfailingDivinity;
				}
 				
				buffs[buffslot].ExtraDIChance = mod;
  				break;
 			}

			case SE_Illusion:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Illusion: race %d", effect_value);
#endif
				// Gender Illusions
				if(spell.base[i] == -1) {
					// Specific Gender Illusions
					if(spell_id == 1732 || spell_id == 1731) {
						int specific_gender = -1;
						// Male
						if(spell_id == 1732)
							specific_gender = 0;
						// Female
						else if (spell_id == 1731)
							specific_gender = 1;
						if(specific_gender > -1) {
							if(caster && caster->GetTarget()) {
								SendIllusionPacket
								(
									caster->GetTarget()->GetBaseRace(),
									specific_gender,
									caster->GetTarget()->GetTexture()
								);
							}
						}
					}
					// Change Gender Illusions
					else {
						if(caster && caster->GetTarget()) {
							int opposite_gender = 0;
							if(caster->GetTarget()->GetGender() == 0)
								opposite_gender = 1;

							SendIllusionPacket
							(
								caster->GetTarget()->GetRace(),
								opposite_gender,
								caster->GetTarget()->GetTexture()
							);
						}
					}
				}
				// Racial Illusions
				else {
					SendIllusionPacket
					(
						spell.base[i],
						Mob::GetDefaultGender(spell.base[i], GetGender()),
						spell.base2[i]
					);
					if(spell.base[i] == OGRE){
						SendAppearancePacket(AT_Size, 9);
					}
					else if(spell.base[i] == TROLL){
						SendAppearancePacket(AT_Size, 8);
					}
					else if(spell.base[i] == VAHSHIR || spell.base[i] == BARBARIAN){
						SendAppearancePacket(AT_Size, 7);
					}
					else if(spell.base[i] == HALF_ELF || spell.base[i] == WOOD_ELF || spell.base[i] == DARK_ELF || spell.base[i] == FROGLOK){
						SendAppearancePacket(AT_Size, 5);
					}
					else if(spell.base[i] == DWARF){
						SendAppearancePacket(AT_Size, 4);
					}
					else if(spell.base[i] == HALFLING || spell.base[i] == GNOME){
						SendAppearancePacket(AT_Size, 3);
					}
					else if(spell.base[i] == WOLF) {
						SendAppearancePacket(AT_Size, 2);
					}
					else{
						SendAppearancePacket(AT_Size, 6);
					}
				}
				for(int x = 0; x < 7; x++){
					SendWearChange(x);
				}
				if(caster && caster->GetAA(aaPermanentIllusion))
					buffs[buffslot].persistant_buff = 1;
				else
					buffs[buffslot].persistant_buff = 0;
				break;
			}

			case SE_IllusionCopy:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Illusion Copy");
#endif
				if(caster && caster->GetTarget()){
						SendIllusionPacket
						(
							caster->GetTarget()->GetRace(),
							caster->GetTarget()->GetGender(),
							caster->GetTarget()->GetTexture()
						);
						caster->SendAppearancePacket(AT_Size, caster->GetTarget()->GetSize());
						for(int x = 0; x < 7; x++){
							caster->SendWearChange(x);
						}
				}
			}

			case SE_WipeHateList:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Memory Blur: %d", effect_value);
#endif
				int wipechance = spells[spell_id].base[i];
				int bonus = 0;
				
				if (caster){
				bonus = caster->spellbonuses.IncreaseChanceMemwipe + 
						caster->itembonuses.IncreaseChanceMemwipe + 
						caster->aabonuses.IncreaseChanceMemwipe;
				}

				wipechance += wipechance*bonus/100;
				
				if(MakeRandomInt(0, 99) < wipechance)
				{
					if(IsAIControlled())
					{
						WipeHateList();
					}
					Message(13, "Your mind fogs. Who are my friends? Who are my enemies?... it was all so clear a moment ago...");
				}
				break;
			}

			case SE_SpinTarget:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Spin: %d", effect_value);
#endif
				// the spinning is handled by the client
				int max_level = spells[spell_id].max[i];
				if(max_level == 0)
					max_level = RuleI(Spells, BaseImmunityLevel); // Default max is 55 level limit

				// NPCs ignore level limits in their spells
				if(GetSpecialAbility(UNSTUNABLE) ||
					((GetLevel() > max_level)
					&& caster && (!caster->IsNPC() || (caster->IsNPC() && !RuleB(Spells, NPCIgnoreBaseImmunity)))))
				{
					caster->Message_StringID(MT_Shout, IMMUNE_STUN);
				}
				else
				{
					// the spinning is handled by the client
					// Stun duration is based on the effect_value, not the buff duration(alot don't have buffs)
					Stun(effect_value);
					if(!IsClient()) {
						Spin();
						spun_timer.Start(100); // spins alittle every 100 ms
					}
				}
				break;
			}

			case SE_EyeOfZomm:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Eye of Zomm");
#endif
				if(caster && caster->IsClient()) {
					char eye_name[64];
					snprintf(eye_name, sizeof(eye_name), "Eye_of_%s", caster->GetCleanName());
					int duration = CalcBuffDuration(caster, this, spell_id) * 6;
					caster->TemporaryPets(spell_id, nullptr, eye_name, duration);
				}
				break;
			}

			case SE_ReclaimPet:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Reclaim Pet");
#endif
				if
				(
					IsNPC() &&
					GetOwnerID() &&		// I'm a pet
					caster &&					// there's a caster
					caster->GetID() == GetOwnerID()	&& // and it's my master
					GetPetType() != petCharmed
				)
				{
					int lvlmod = 4;
					if(caster->IsClient() && caster->CastToClient()->GetAA(aaImprovedReclaimEnergy))
						lvlmod = 8;	//this is an unconfirmed number, I made it up
					if(caster->IsClient() && caster->CastToClient()->GetAA(aaImprovedReclaimEnergy2))
						lvlmod = 8;	//this is an unconfirmed number, I made it up
					caster->SetMana(caster->GetMana()+(GetLevel()*lvlmod));

					if(caster->IsClient())
						caster->CastToClient()->SetPet(0);
					SetOwnerID(0);	// this will kill the pet
				}
				break;
			}

			case SE_BindSight:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Bind Sight");
#endif
				if(caster && caster->IsClient())
				{
					caster->CastToClient()->SetBindSightTarget(this);
				}
				break;
			}

			case SE_FeignDeath:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Feign Death");
#endif
				//todo, look up spell ID in DB
				if(spell_id == 2488) //Dook- Lifeburn fix
					break;

				if(IsClient())
					CastToClient()->SetFeigned(true);
				break;
			}

			case SE_Sentinel:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Sentinel");
#endif
				if(caster)
				{
					if(caster == this)
					{
						Message_StringID(MT_Spells,
							SENTINEL_TRIG_YOU);
					}
					else
					{
						caster->Message_StringID(MT_Spells,
							SENTINEL_TRIG_OTHER, GetCleanName());
					}
				}
				break;
			}

			case SE_LocateCorpse:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Locate Corpse");
#endif
				// This is handled by the client prior to SoD.
				//
				if(IsClient() && (CastToClient()->GetClientVersionBit() & BIT_SoDAndLater))
					CastToClient()->LocateCorpse();

				break;
			}

			case SE_Revive:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Revive");	// heh the corpse won't see this
#endif
				if (IsCorpse() && CastToCorpse()->IsPlayerCorpse()) {

					if(caster)
						mlog(SPELLS__REZ, " corpse being rezzed using spell %i by %s",
							spell_id, caster->GetName());

					CastToCorpse()->CastRezz(spell_id, caster);
				}
				break;
			}

			case SE_ModelSize:
			case SE_ChangeHeight:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Model Size: %d%%", effect_value);
#endif
				ChangeSize(GetSize() * (effect_value / 100.0));
				break;
			}

			case SE_Root:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Root: %+i", effect_value);
#endif
				rooted = true;

				if (caster){
					buffs[buffslot].RootBreakChance = caster->aabonuses.RootBreakChance + 
													caster->itembonuses.RootBreakChance +
													caster->spellbonuses.RootBreakChance;
				}

				break;
			}

			case SE_SummonHorse:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Summon Mount: %s", spell.teleport_zone);
#endif
				if(IsClient())	// NPCs can't ride
				{
					CastToClient()->SummonHorse(spell_id);
				}
				break;
			}

			case SE_SummonCorpse:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Summon Corpse: %d", effect_value);
#endif
				// can only summon corpses of clients
				if(!IsNPC()) {
					Client* TargetClient = 0;
					if(this->GetTarget())
						TargetClient = this->GetTarget()->CastToClient();
					else
						TargetClient = this->CastToClient();

					// We now have a valid target for this spell. Either the caster himself or a targetted player. Lets see if the target is in the group.
					Group* group = entity_list.GetGroupByClient(TargetClient);
					if(group) {
						if(!group->IsGroupMember(TargetClient)) {
							Message(13, "Your target must be a group member for this spell.");
							break;
						}
					}
					else {
						Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
						if(r)
						{
							uint32 gid = 0xFFFFFFFF;
							gid = r->GetGroup(caster->GetName());
							if(gid < 11)
							{
								if(r->GetGroup(TargetClient->GetName()) != gid) {
									Message(13, "Your target must be a group member for this spell.");
									break;
								}
							}
						} else {
							if(TargetClient != this->CastToClient()) {
								Message(13, "Your target must be a group member for this spell.");
								break;
							}
						}
					}

					// Now we should either be casting this on self or its being cast on a valid group member
					if(TargetClient) {
						Corpse *corpse = entity_list.GetCorpseByOwner(TargetClient);
						if(corpse) {
							if(TargetClient == this->CastToClient())
								Message_StringID(4, SUMMONING_CORPSE, TargetClient->CastToMob()->GetCleanName());
							else
								Message_StringID(4, SUMMONING_CORPSE_OTHER, TargetClient->CastToMob()->GetCleanName());

							corpse->Summon(CastToClient(), true, true);
						}
						else {
							// No corpse found in the zone
							Message_StringID(4, CORPSE_CANT_SENSE);
						}
					}
					else {
						Message_StringID(4, TARGET_NOT_FOUND);
						LogFile->write(EQEMuLog::Error, "%s attempted to cast spell id %u with spell effect SE_SummonCorpse, but could not cast target into a Client object.", GetCleanName(), spell_id);
					}
				}

				break;
			}
			case SE_AddMeleeProc:
			case SE_WeaponProc:
			{
				uint16 procid = GetProcID(spell_id, i);
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Weapon Proc: %s (id %d)", spells[effect_value].name, procid);
#endif

				if(spells[spell_id].base2[i] == 0)
					AddProcToWeapon(procid, false, 100, spell_id);
				else
					AddProcToWeapon(procid, false, spells[spell_id].base2[i]+100, spell_id);
				break;
			}

			case SE_SkillProc2:
			case SE_SkillProc:
			{
				uint16 procid = GetProcID(spell_id, i);
#ifdef SPELL_EFFECT_SPAM
			snprintf(effect_desc, _EDLEN, "Weapon Proc: %s (id %d)", spells[effect_value].name, procid);
#endif
				if(spells[spell_id].base2[i] == 0)
					AddSkillProc(procid, 100, spell_id);
				else
					AddSkillProc(procid, spells[spell_id].base2[i]+100, spell_id);
				break;
			}

			case SE_NegateAttacks:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Melee Negate Attack Rune: %+i", effect_value);
#endif
				if(buffslot >= 0)
					buffs[buffslot].numhits = effect_value;
				break;
			}
			case SE_AppraiseLDonChest:
			{
				if(IsNPC())
				{
					int check = spell.max[0];
					int target = spell.targettype;
					if(target == ST_LDoNChest_Cursed)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNSenseTraps(CastToNPC(), check, LDoNTypeCursed);
						}
					}
					else if(target == ST_Target)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNSenseTraps(CastToNPC(), check, LDoNTypeMagical);
						}
					}
				}
				break;
			}

			case SE_DisarmLDoNTrap:
			{
				if(IsNPC())
				{
					int check = spell.max[0];
					int target = spell.targettype;
					if(target == ST_LDoNChest_Cursed)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNDisarm(CastToNPC(), check, LDoNTypeCursed);
						}
					}
					else if(target == ST_Target)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNDisarm(CastToNPC(), check, LDoNTypeMagical);
						}
					}
				}
				break;
			}

			case SE_UnlockLDoNChest:
			{
				if(IsNPC())
				{
					int check = spell.max[0];
					int target = spell.targettype;
					if(target == ST_LDoNChest_Cursed)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNPickLock(CastToNPC(), check, LDoNTypeCursed);
						}
					}
					else if(target == ST_Target)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNPickLock(CastToNPC(), check, LDoNTypeMagical);
						}
					}
				}
				break;
			}

			case SE_Lull:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Lull");
#endif
				// TODO: check vs. CHA when harmony effect failed, if caster is to be added to hatelist
				break;
			}

			case SE_PoisonCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Poison Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = 0 - effect_value;
					uint32 buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (buffs[j].spellid >= (uint16)SPDAT_RECORDS)
							continue;
						if (CalculatePoisonCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= buffs[j].counters) {
							if (caster)
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						} else {
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SE_DiseaseCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Disease Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = 0 - effect_value;
					uint32 buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (buffs[j].spellid >= (uint16)SPDAT_RECORDS)
							continue;
						if (CalculateDiseaseCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= buffs[j].counters)
						{
							if (caster)
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						}
						else
						{
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SE_CurseCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Curse Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = 0 - effect_value;
					uint32 buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (buffs[j].spellid >= (uint16)SPDAT_RECORDS)
							continue;
						if (CalculateCurseCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= buffs[j].counters)
						{
							if (caster)
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						}
						else
						{
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SE_CorruptionCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Corruption Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = -effect_value;
					uint32 buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (buffs[j].spellid >= (uint16)SPDAT_RECORDS)
							continue;
						if (CalculateCorruptionCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= buffs[j].counters) {
							if (caster)
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						} else {
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SE_Destroy:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Destroy");
#endif
				if(IsNPC()) {
					if(GetLevel() <= 52)
						CastToNPC()->Depop();
					else
						Message(13, "Your target is too high level to be affected by this spell.");
				}
				break;
			}

			case SE_TossUp:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Toss Up: %d", effect_value);
#endif
				double toss_amt = (double)spells[spell_id].base[i];
				if(toss_amt < 0)
					toss_amt = -toss_amt;

				if(IsNPC())
				{
					Stun(toss_amt);
				}
				toss_amt = sqrt(toss_amt)-2.0;

				if(toss_amt < 0.0)
					toss_amt = 0.0;

				if(toss_amt > 20.0)
					toss_amt = 20.0;

				if(IsClient())
				{
					CastToClient()->SetKnockBackExemption(true);
				}

				double look_heading = GetHeading();
				look_heading /= 256;
				look_heading *= 360;
				look_heading += 180;
				if(look_heading > 360)
					look_heading -= 360;

				//x and y are crossed mkay
				double new_x = spells[spell_id].pushback * sin(double(look_heading * 3.141592 / 180.0));
				double new_y = spells[spell_id].pushback * cos(double(look_heading * 3.141592 / 180.0));

				EQApplicationPacket* outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
				PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

				spu->spawn_id	= GetID();
				spu->x_pos		= FloatToEQ19(GetX());
				spu->y_pos		= FloatToEQ19(GetY());
				spu->z_pos		= FloatToEQ19(GetZ());
				spu->delta_x	= NewFloatToEQ13(new_x);
				spu->delta_y	= NewFloatToEQ13(new_y);
				spu->delta_z	= NewFloatToEQ13(toss_amt);
				spu->heading	= FloatToEQ19(GetHeading());
				spu->padding0002	=0;
				spu->padding0006	=7;
				spu->padding0014	=0x7f;
				spu->padding0018	=0x5df27;
				spu->animation = 0;
				spu->delta_heading = NewFloatToEQ13(0);
				outapp_push->priority = 5;
				entity_list.QueueClients(this, outapp_push, true);
				if(IsClient())
					CastToClient()->FastQueuePacket(&outapp_push);

				break;
			}

			case SE_StopRain:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Stop Rain");
#endif
				zone->zone_weather = 0;
				zone->weather_intensity = 0;
				zone->weatherSend();
				break;
			}

			case SE_Sacrifice:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Sacrifice");
#endif
				if(!IsClient() || !caster->IsClient()){
					break;
				}
				CastToClient()->SacrificeConfirm(caster->CastToClient());
				break;
			}

			case SE_SummonPC:
			{
			if(IsClient()){
					CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), caster->GetX(), caster->GetY(), caster->GetZ(), caster->GetHeading(), 2, SummonPC);
					Message(15, "You have been summoned!");
					entity_list.ClearAggro(this);
				}
				else
					caster->Message(13, "This spell can only be cast on players.");

				break;
			}

			case SE_Silence:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Silence");
#endif
				Silence(true);
				break;
			}

			case SE_Amnesia:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Amnesia");
#endif
				Amnesia(true);
				break;
			}

			case SE_CallPet:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Call Pet");
#endif
				// this is cast on self, not on the pet
				if(GetPet() && GetPet()->IsNPC())
				{
					GetPet()->CastToNPC()->GMMove(GetX(), GetY(), GetZ(), GetHeading());
				}
				break;
			}

			case SE_StackingCommand_Block:
			case SE_StackingCommand_Overwrite:
			{
				// these are special effects used by the buff stuff
				break;
			}


			case SE_TemporaryPets: //Dook- swarms and wards:
			{
				// EverHood - this makes necro epic 1.5/2.0 proc work properly
				if((spell_id != 6882) && (spell_id != 6884)) // Chaotic Jester/Steadfast Servant
				{
					char pet_name[64];
					snprintf(pet_name, sizeof(pet_name), "%s`s pet", caster->GetCleanName());
					caster->TemporaryPets(spell_id, this, pet_name);
				}
				else
					caster->TemporaryPets(spell_id, this, nullptr);
				break;
			}

			case SE_FadingMemories:		//Dook- escape etc
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Fading Memories");
#endif
				if(caster && caster->IsClient())
					caster->CastToClient()->Escape();
				else
				{
					entity_list.RemoveFromTargets(caster);
					SetInvisible(1);
				}
				break;
			}

			case SE_RangedProc:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Ranged Proc: %+i", effect_value);
#endif
				uint16 procid = GetProcID(spell_id, i);

				if(spells[spell_id].base2[i] == 0)
					AddRangedProc(procid, 100, spell_id);
				else
					AddRangedProc(procid, spells[spell_id].base2[i]+100, spell_id);
				break;
			}

			case SE_Rampage:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Rampage");
#endif
				if(caster)
					entity_list.AEAttack(caster, 30, 13, 0, true); // on live wars dont get a duration ramp, its a one shot deal

				break;
			}

			case SE_AEMelee:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Duration Rampage");
#endif
				if (caster && caster->IsClient()) { // will tidy this up later so that NPCs can duration ramp from spells too
					CastToClient()->DurationRampage(effect_value*12);
				}
				break;
			}

			case SE_AETaunt://Dook- slapped it in the spell effect so client does the animations
			{			// and incase there are similar spells we havent found yet
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "AE Taunt");
#endif
				if(caster && caster->IsClient())
					entity_list.AETaunt(caster->CastToClient());
				break;
			}

			case SE_SkillAttack:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Skill Attack");
#endif
				/*
				Weapon Damage = spells[spell_id].base[i]
				Chance to Hit Bonus = spells[spell_id].base2[i]
				???? = spells[spell_id].max[i] - MOST of the effects have this value.
				*Max is lower value then Weapon base, possibly min hit vs Weapon Damage range ie. MakeRandInt(max,base)
				*/
				int16 focus = 0;

				if(caster->IsClient())
					focus = caster->CastToClient()->GetFocusEffect(focusFcBaseEffects, spell_id);

				switch(spells[spell_id].skill)
				{
					case SkillThrowing:
						caster->DoThrowingAttackDmg(this, nullptr, nullptr, spells[spell_id].base[i],spells[spell_id].base2[i], focus);
					break;

					case SkillArchery:
						caster->DoArcheryAttackDmg(this, nullptr, nullptr, spells[spell_id].base[i],spells[spell_id].base2[i],focus);
					break;

					default:
						caster->DoMeleeSkillAttackDmg(this, spells[spell_id].base[i], spells[spell_id].skill, spells[spell_id].base2[i], focus);
					break;
				}
				break;
			}

			case SE_WakeTheDead:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Wake The Dead");
#endif
				//meh dupe issue with npc casting this
				if(caster->IsClient()){
					//this spell doesn't appear to actually contain the information on duration inside of it oddly
					int dur = 60;
					if(spell_id == 3269)
						dur += 15;
					else if(spell_id == 3270)
						dur += 30;

					caster->WakeTheDead(spell_id, caster->GetTarget(), dur);
				}
				break;
			}

			case SE_Doppelganger:
			{
				if(caster && caster->IsClient()) {
					char pet_name[64];
					snprintf(pet_name, sizeof(pet_name), "%s`s doppelganger", caster->GetCleanName());
					int pet_count = spells[spell_id].base[i];
					int pet_duration = spells[spell_id].max[i];
					caster->CastToClient()->Doppelganger(spell_id, this, pet_name, pet_count, pet_duration);
				}
				break;
			}

			case SE_DefensiveProc:
			{
				uint16 procid = GetProcID(spell_id, i);
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Defensive Proc: %s (id %d)", spells[effect_value].name, procid);
#endif
				if(spells[spell_id].base2[i] == 0)
					AddDefensiveProc(procid, 100,spell_id);
				else
					AddDefensiveProc(procid, spells[spell_id].base2[i]+100,spell_id);
				break;

				break;
			}

			case SE_BardAEDot:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Bard AE Dot: %+i", effect_value);
#endif
				// SE_CurrentHP is calculated at first tick if its a dot/buff
				if (buffslot >= 0)
					break;

				// for offensive spells check if we have a spell rune on
				int32 dmg = effect_value;
				if(dmg < 0)
				{
					// take partial damage into account
					dmg = (int32) (dmg * partial / 100);

					//handles AAs and what not...
					//need a bard version of this prolly...
					//if(caster)
					//	dmg = caster->GetActSpellDamage(spell_id, dmg);

					dmg = -dmg;
					Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
				} else if(dmg > 0) {
					//healing spell...
					if(caster)
						dmg = caster->GetActSpellHealing(spell_id, dmg, this);
					HealDamage(dmg, caster);
				}
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints: %+i  actual: %+i", effect_value, dmg);
#endif
				break;
			}

			case SE_CurrentEndurance: {
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Endurance: %+i", effect_value);
#endif
				if(IsClient()) {
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() + effect_value);
					if (effect_value < 0)
						TryTriggerOnValueAmount(false, false, true);
				}
				break;
			}

			case SE_CurrentEnduranceOnce:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Endurance Once: %+i", effect_value);
#endif

				if(IsClient()) {
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() + effect_value);
					if (effect_value < 0)
						TryTriggerOnValueAmount(false, false, true);
				}
				break;
			}

			case SE_BalanceHP: {
				if(!caster)
					break;

				if(!caster->IsClient())
					break;

				Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
				if(r)
				{
					uint32 gid = 0xFFFFFFFF;
					gid = r->GetGroup(caster->GetName());
					if(gid < 11)
					{
						r->BalanceHP(spell.base[i], gid, spell.range, caster);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g)
					break;

				g->BalanceHP(spell.base[i], spell.range, caster);
				break;
			}

			case SE_BalanceMana: {
				if(!caster)
					break;

				if(!caster->IsClient())
					break;

				Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
				if(r)
				{
					uint32 gid = 0xFFFFFFFF;
					gid = r->GetGroup(caster->GetName());
					if(gid < 11)
					{
						r->BalanceMana(spell.base[i], gid, spell.range, caster);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g)
					break;

				g->BalanceMana(spell.base[i], spell.range, caster);
				break;
			}

			case SE_SummonAndResAllCorpses:
			{
				if(IsClient())
					CastToClient()->SummonAndRezzAllCorpses();

				break;
			}

			case SE_GateToHomeCity:
			{
				if(IsClient())
					CastToClient()->GoToBind(4);
				break;
			}

			case SE_SuspendMinion:
			case SE_SuspendPet:
			{
				if(IsClient())
					CastToClient()->SuspendMinion();

				break;
			}

			case SE_FcTimerRefresh:
			{
				if(IsClient()) {
					for(unsigned int i =0 ; i < MAX_PP_MEMSPELL; ++i) {
						if(IsValidSpell(CastToClient()->m_pp.mem_spells[i])) {
							if (CalcFocusEffect(focusFcTimerRefresh, spell_id, CastToClient()->m_pp.mem_spells[i])){
								CastToClient()->m_pp.spellSlotRefresh[i] = 1;
								CastToClient()->GetPTimers().Clear(&database, (pTimerSpellStart + CastToClient()->m_pp.mem_spells[i]));
							}
						}
					}
					SetMana(GetMana());
				}
				break;
			}

			case SE_HealGroupFromMana: {
				if(!caster)
					break;

				if(!caster->IsClient())
					break;

				uint32 max_mana = spell.base[i];
				int ratio = spell.base2[i];
				uint32 heal_amt = 0;

				if (caster->GetMana() <= max_mana){
					heal_amt = ratio*caster->GetMana()/10;
					caster->SetMana(0);
				}

				else {
					heal_amt = ratio*max_mana/10;
					caster->SetMana(caster->GetMana() - max_mana);
				}

				Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
				if(r)
				{
					uint32 gid = 0xFFFFFFFF;
					gid = r->GetGroup(caster->GetName());
					if(gid < 11)
					{
						r->HealGroup(heal_amt,caster, gid, spell.range);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g){
					caster->HealDamage(heal_amt);
					break;
				}

				g->HealGroup(heal_amt, caster, spell.range);
				break;
			}

			case SE_ManaDrainWithDmg:
			{
				int mana_damage = 0;
				int32 mana_to_use = GetMana() - spell.base[i];
				if(mana_to_use > -1) {
					SetMana(GetMana() - spell.base[i]);	
					TryTriggerOnValueAmount(false, true);
					// we take full dmg(-10 to make the damage the right sign)
					mana_damage = spell.base[i] / -10 * spell.base2[i];
					Damage(caster, mana_damage, spell_id, spell.skill, false, i, true);
				}
				else {
					mana_damage = GetMana() / -10 * spell.base2[i];
					SetMana(0);
					Damage(caster, mana_damage, spell_id, spell.skill, false, i, true);
				}
				break;
			}

			case SE_EndDrainWithDmg:
			{
				if(IsClient()) {
					int end_damage = 0;
					int32 end_to_use = CastToClient()->GetEndurance() - spell.base[i];
					if(end_to_use > -1) {
						CastToClient()->SetEndurance(CastToClient()->GetEndurance() - spell.base[i]);
						TryTriggerOnValueAmount(false, false, true);
						// we take full dmg(-10 to make the damage the right sign)
						end_damage = spell.base[i] / -10 * spell.base2[i];
						Damage(caster, end_damage, spell_id, spell.skill, false, i, true);
					}
					else {
						end_damage = CastToClient()->GetEndurance() / -10 * spell.base2[i];
						CastToClient()->SetEndurance(0);
						Damage(caster, end_damage, spell_id, spell.skill, false, i, true);
					}
				}
				break;
			}

			case SE_SetBodyType:
			{
				SetBodyType((bodyType)spell.base[i], false);
				break;
			}

			case SE_Leap:
			{
				// These effects remove lev and only work a certain distance away.
				BuffFadeByEffect(SE_Levitate);
				if (caster && caster->GetTarget()) {
					float my_x = caster->GetX();
					float my_y = caster->GetY();
					float my_z = caster->GetZ();
					float target_x = GetX();
					float target_y = GetY();
					float target_z = GetZ();
					if ((CalculateDistance(my_x, my_y, my_z) > 10) &&
						(CalculateDistance(my_x, my_y, my_z) < 75) &&
						(caster->CheckLosFN(caster->GetTarget())))
					{
						float value, x_vector, y_vector, hypot;

						value = (float)spell.base[i]; // distance away from target

						x_vector = target_x - my_x;
						y_vector = target_y - my_y;
						hypot = sqrt(x_vector*x_vector + y_vector*y_vector);

						x_vector /= hypot;
						y_vector /= hypot;

						my_x = target_x - (x_vector * value);
						my_y = target_y - (y_vector * value);

						float new_ground = GetGroundZ(my_x, my_y);

						if(caster->IsClient())
							caster->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), my_x, my_y, new_ground, GetHeading()*2);
						else
							caster->GMMove(my_x, my_y, new_ground, GetHeading());
					}
				}
				break;
			}
			case SE_VoiceGraft:
			{
				if(caster && caster->GetPet())
					caster->spellbonuses.VoiceGraft = caster->GetPetID();

				break;
			}

			case SE_ManaBurn:
			{
				uint32 max_mana = spell.base[i];
				int ratio = spell.base2[i];
				int32 dmg = 0;

				if (caster){
					if (caster->GetMana() <= max_mana){
							dmg = ratio*caster->GetMana()/10;
							caster->SetMana(0);
					}

					else {
						dmg = ratio*max_mana/10;
						caster->SetMana(caster->GetMana() - max_mana);
						TryTriggerOnValueAmount(false, true);
					}

					if(IsDetrimentalSpell(spell_id)) {
						dmg = -dmg;
						Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
					} else {
						HealDamage(dmg, caster);
					}
				}
			}

			case SE_Taunt:
			{
				if (IsNPC())
					caster->Taunt(this->CastToNPC(), false, spell.base[i]);
			}

			// Handled Elsewhere
			case SE_ImmuneFleeing:
			case SE_NegateSpellEffect:
			case SE_Knockdown: // handled by client
			case SE_ShadowStepDirectional: // handled by client
			case SE_SpellOnDeath:
			case SE_BlockNextSpellFocus:
			case SE_ReduceReuseTimer:
			case SE_SwarmPetDuration:
			case SE_LimitHPPercent:
			case SE_LimitManaPercent:
			case SE_LimitEndPercent:
			case SE_ExtraAttackChance:
			case SE_ProcChance:
			case SE_StunResist:
			case SE_MinDamageModifier:
			case SE_DamageModifier:
			case SE_HitChance:
			case SE_MeleeSkillCheck:
			case SE_HundredHands:
			case SE_ResistFearChance:
			case SE_ResistSpellChance:
			case SE_AllInstrumentMod:
			case SE_MeleeLifetap:
			case SE_DoubleAttackChance:
			case SE_TripleAttackChance:
			case SE_DualWieldChance:
			case SE_ParryChance:
			case SE_DodgeChance:
			case SE_RiposteChance:
			case SE_AvoidMeleeChance:
			case SE_CrippBlowChance:
			case SE_CriticalHitChance:
			case SE_MeleeMitigation:
			case SE_Reflect:
			case SE_Screech:
			case SE_Amplification:
			case SE_MagicWeapon:
			case SE_Hunger:
			case SE_MagnifyVision:
			case SE_Lycanthropy:
			case SE_NegateIfCombat:
			case SE_CastingLevel:
			case SE_CastingLevel2:
			case SE_RaiseStatCap:
			case SE_ResistAll:
			case SE_ResistMagic:
			case SE_ResistDisease:
			case SE_ResistPoison:
			case SE_ResistCold:
			case SE_ResistFire:
			case SE_AllStats:
			case SE_CHA:
			case SE_WIS:
			case SE_INT:
			case SE_STA:
			case SE_AGI:
			case SE_DEX:
			case SE_STR:
			case SE_ATK:
			case SE_ArmorClass:
			case SE_EndurancePool:
			case SE_Stamina:
			case SE_UltraVision:
			case SE_InfraVision:
			case SE_ManaPool:
			case SE_TotalHP:
			case SE_ChangeFrenzyRad:
			case SE_Harmony:
			case SE_ChangeAggro:
			case SE_Hate2:
			case SE_Identify:
			case SE_InstantHate:
			case SE_ReduceHate:
			case SE_SpellDamageShield:
			case SE_ReverseDS:
			case SE_DamageShield:
			case SE_TrueNorth:
			case SE_WaterBreathing:
			case SE_MovementSpeed:
			case SE_HealOverTime:
			case SE_PercentXPIncrease:
			case SE_DivineSave:
			case SE_Accuracy:
			case SE_Flurry:
			case SE_AttackSpeed:
			case SE_AttackSpeed2:
			case SE_AttackSpeed3:
			case SE_AttackSpeed4:
			case SE_ImprovedDamage:
			case SE_ImprovedHeal:
			case SE_IncreaseSpellHaste:
			case SE_IncreaseSpellDuration:
			case SE_IncreaseRange:
			case SE_SpellHateMod:
			case SE_ReduceReagentCost:
			case SE_ReduceManaCost:
			case SE_LimitMaxLevel:
			case SE_LimitResist:
			case SE_LimitTarget:
			case SE_LimitEffect:
			case SE_LimitSpellType:
			case SE_LimitSpell:
			case SE_LimitMinDur:
			case SE_LimitInstant:
			case SE_LimitMinLevel:
			case SE_LimitCastTimeMin:
			case SE_LimitManaMin:
			case SE_LimitCombatSkills:
			case SE_SpellDurationIncByTic:
			case SE_TriggerOnCast:
			case SE_HealRate:
			case SE_SkillDamageTaken:
			case SE_FcSpellVulnerability:
			case SE_SpellTrigger:
			case SE_ApplyEffect:
			case SE_FcTwincast:
			case SE_DelayDeath:
			case SE_InterruptCasting:
			case SE_ImprovedSpellEffect:
			case SE_BossSpellTrigger:
			case SE_CastOnWearoff:
			case SE_EffectOnFade:
			case SE_MaxHPChange:
			case SE_SympatheticProc:
			case SE_FcDamageAmt:
			case SE_CriticalSpellChance:
			case SE_SpellCritChance:
			case SE_SpellCritDmgIncrease:
			case SE_DotCritDmgIncrease:
			case SE_CriticalHealChance:
			case SE_CriticalHealOverTime:
			case SE_CriticalDoTChance:
			case SE_SpellOnKill:
			case SE_SpellOnKill2:
			case SE_CriticalDamageMob:
			case SE_LimitSpellGroup:
			case SE_ResistCorruption:
			case SE_ReduceSkillTimer:
			case SE_HPToMana:
			case SE_ManaAbsorbPercentDamage:
			case SE_SkillDamageAmount:
			case SE_SkillDamageAmount2:
			case SE_GravityEffect:
			case SE_IncreaseBlockChance:
			case SE_AntiGate:
			case SE_Fearless:
			case SE_FcDamageAmtCrit:
			case SE_FcHealAmtCrit:
			case SE_CastOnCurer:
			case SE_CastOnCure:
			case SE_CastonNumHitFade:
			case SE_LimitToSkill:
			case SE_SpellProcChance:
			case SE_CharmBreakChance:
			case SE_BardSongRange:
			case SE_ACv2:
			case SE_ManaRegen_v2:
			case SE_FcDamagePctCrit:
			case  SE_FcHealAmt:
			case SE_FcHealPctIncoming:
			case SE_CriticalHealDecay:
			case SE_CriticalRegenDecay:
			case SE_FcDamageAmtIncoming:
			case SE_LimitCastingSkill:
			case SE_MitigateDamageShield:
			case SE_FcBaseEffects:
			case SE_LimitClass:
			case SE_BlockBehind:
			case SE_ShieldBlock:
			case SE_PetCriticalHit:
			case SE_SlayUndead:
			case SE_GiveDoubleAttack:
			case SE_StrikeThrough2:
			case SE_SecondaryDmgInc:
			case SE_ArcheryDamageModifier:
			case SE_ConsumeProjectile:
			case SE_ForageAdditionalItems:
			case SE_Salvage:
			case SE_FrontalBackstabChance:
			case SE_FrontalBackstabMinDmg:
			case SE_TripleBackstab:
			case SE_DoubleSpecialAttack:
			case SE_IncreaseRunSpeedCap:
			case SE_BaseMovementSpeed:
			case SE_FrontalStunResist:
			case SE_ImprovedBindWound:
			case SE_MaxBindWound:
			case SE_CombatStability:
			case SE_AddSingingMod:
			case SE_SongModCap:
			case SE_PetAvoidance:
			case SE_GiveDoubleRiposte:
			case SE_Ambidexterity:
			case SE_PetMaxHP:
			case SE_PetFlurry:
			case SE_MasteryofPast:
			case SE_GivePetGroupTarget:
			case SE_RootBreakChance:
			case SE_UnfailingDivinity:
			case SE_ChannelChanceSpells:
			case SE_ChannelChanceItems:
			case SE_FcHealPctCritIncoming:
			case SE_FcIncreaseNumHits:
			case SE_CastonFocusEffect:
			case SE_FcHealAmtIncoming:
			case SE_LimitManaMax:
			case SE_DoubleRangedAttack:
			case SE_ShieldEquipHateMod:
			case SE_ShieldEquipDmgMod:
			case SE_TriggerOnReqTarget:
			case SE_LimitRace:
			case SE_FcLimitUse:
			case SE_FcMute:	
			case SE_LimitUseType:
			case SE_FcStunTimeMod:
			case SE_StunBashChance:
			case SE_IncreaseChanceMemwipe:
			case SE_CriticalMend:
			case SE_LimitCastTimeMax:
			case SE_TriggerOnReqCaster:
			case SE_FrenziedDevastation:
			{
				break;
			}

			default:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Unknown Effect ID %d", effect);
#else
				Message(0, "Unknown spell effect %d in spell %s (id %d)", effect, spell.name, spell_id);
#endif
			}
		}
#ifdef SPELL_EFFECT_SPAM
		Message(0, ". . . Effect #%i: %s", i + 1, (effect_desc && effect_desc[0]) ? effect_desc : "Unknown");
#endif
	}

	CalcBonuses();

	if (SummonedItem) {
		Client *c=CastToClient();
		c->PushItemOnCursor(*SummonedItem);
		c->SendItemPacket(SLOT_CURSOR, SummonedItem, ItemPacketSummonItem);
		safe_delete(SummonedItem);
	}

	return true;
}

int Mob::CalcSpellEffectValue(uint16 spell_id, int effect_id, int caster_level, Mob *caster, int ticsremaining)
{
	int formula, base, max, effect_value;

	if
	(
		!IsValidSpell(spell_id) ||
		effect_id < 0 ||
		effect_id >= EFFECT_COUNT
	)
		return 0;

	formula = spells[spell_id].formula[effect_id];
	base = spells[spell_id].base[effect_id];
	max = spells[spell_id].max[effect_id];

	if(IsBlankSpellEffect(spell_id, effect_id))
		return 0;

	effect_value = CalcSpellEffectValue_formula(formula, base, max, caster_level, spell_id, ticsremaining);

	if(caster && IsBardSong(spell_id) &&
	(spells[spell_id].effectid[effect_id] != SE_AttackSpeed) &&
	(spells[spell_id].effectid[effect_id] != SE_AttackSpeed2) &&
	(spells[spell_id].effectid[effect_id] != SE_AttackSpeed3) &&
	(spells[spell_id].effectid[effect_id] != SE_Lull) &&
	(spells[spell_id].effectid[effect_id] != SE_ChangeFrenzyRad) &&
	(spells[spell_id].effectid[effect_id] != SE_Harmony) &&
	(spells[spell_id].effectid[effect_id] != SE_CurrentMana)&&
	(spells[spell_id].effectid[effect_id] != SE_ManaRegen_v2))
	{

		int oval = effect_value;
		int mod = caster->GetInstrumentMod(spell_id);
		mod = ApplySpellEffectiveness(caster, spell_id, mod, true);
		effect_value = effect_value * mod / 10;
		mlog(SPELLS__BARDS, "Effect value %d altered with bard modifier of %d to yeild %d", oval, mod, effect_value);
	}

	effect_value = mod_effect_value(effect_value, spell_id, spells[spell_id].effectid[effect_id], caster);

	return(effect_value);
}

// generic formula calculations
int Mob::CalcSpellEffectValue_formula(int formula, int base, int max, int caster_level, uint16 spell_id, int ticsremaining)
{
/*
neotokyo: i need those formulas checked!!!!

0 = base
1 - 99 = base + level * formulaID
100 = base
101 = base + level / 2
102 = base + level
103 = base + level * 2
104 = base + level * 3
105 = base + level * 4
106 ? base + level * 5
107 ? min + level / 2
108 = min + level / 3
109 = min + level / 4
110 = min + level / 5
119 ? min + level / 8
121 ? min + level / 4
122 = splurt
123 ?
203 = stacking issues ? max
205 = stacking issues ? 105


0x77 = min + level / 8
*/

	int result = 0, updownsign = 1, ubase = base;
	if(ubase < 0)
		ubase = 0 - ubase;

	// this updown thing might look messed up but if you look at the
	// spells it actually looks like some have a positive base and max where
	// the max is actually less than the base, hence they grow downward
/*
This seems to mainly catch spells where both base and max are negative.
Strangely, damage spells have a negative base and positive max, but
snare has both of them negative, yet their range should work the same:
(meaning they both start at a negative value and the value gets lower)
*/
	if (max < base && max != 0)
	{
		// values are calculated down
		updownsign = -1;
	}
	else
	{
		// values are calculated up
		updownsign = 1;
	}

	mlog(SPELLS__EFFECT_VALUES, "CSEV: spell %d, formula %d, base %d, max %d, lvl %d. Up/Down %d",
		spell_id, formula, base, max, caster_level, updownsign);

	switch(formula)
	{
		case 60:	//used in stun spells..?
		case 70:
			result = ubase/100; break;
		case 0:
		case 100:	// confirmed 2/6/04
			result = ubase; break;
		case 101:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level / 2)); break;
		case 102:	// confirmed 2/6/04
			result = updownsign * (ubase + caster_level); break;
		case 103:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 2)); break;
		case 104:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 3)); break;
		case 105:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 4)); break;

		case 107:
			//Used on Reckless Strength, I think it should decay over time
			result = updownsign * (ubase + (caster_level / 2)); break;
		case 108:
			result = updownsign * (ubase + (caster_level / 3)); break;
		case 109:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level / 4)); break;

		case 110:	// confirmed 2/6/04
			//is there a reason we dont use updownsign here???
			result = ubase + (caster_level / 5); break;

		case 111:
            result = updownsign * (ubase + 6 * (caster_level - GetMinLevel(spell_id))); break;
		case 112:
            result = updownsign * (ubase + 8 * (caster_level - GetMinLevel(spell_id))); break;
		case 113:
            result = updownsign * (ubase + 10 * (caster_level - GetMinLevel(spell_id))); break;
		case 114:
            result = updownsign * (ubase + 15 * (caster_level - GetMinLevel(spell_id))); break;

		//these formula were updated according to lucy 10/16/04
		case 115:	// this is only in symbol of transal
			result = ubase + 6 * (caster_level - GetMinLevel(spell_id)); break;
		case 116:	// this is only in symbol of ryltan
            result = ubase + 8 * (caster_level - GetMinLevel(spell_id)); break;
		case 117:	// this is only in symbol of pinzarn
            result = ubase + 12 * (caster_level - GetMinLevel(spell_id)); break;
		case 118:	// used in naltron and a few others
            result = ubase + 20 * (caster_level - GetMinLevel(spell_id)); break;

		case 119:	// confirmed 2/6/04
			result = ubase + (caster_level / 8); break;
		case 121:	// corrected 2/6/04
			result = ubase + (caster_level / 3); break;
		case 122:
		{
			// May need to account for duration focus effects
			int ticdif = spells[spell_id].buffduration - (ticsremaining - 1);
			if(ticdif < 0)
				ticdif = 0;

			result = updownsign * (ubase - (12 * ticdif));
			break;
		}
		case 123:	// added 2/6/04
			result = MakeRandomInt(ubase, abs(max));
			break;

		//these are used in stacking effects... formula unknown
		case 201:
		case 203:
			result = max;
			break;
		default:
		{
			if (formula < 100)
				result = ubase + (caster_level * formula);
			else if((formula > 1000) && (formula < 1999))
			{
				// These work like splurt, accept instead of being hard coded to 12, it is formula - 1000.
				// Formula 1999 seems to have a slightly different effect, so is not included here
				int ticdif = spells[spell_id].buffduration - (ticsremaining - 1);
				if(ticdif < 0)
					ticdif = 0;

				result = updownsign * (ubase - ((formula - 1000) * ticdif));
			}
			else if((formula >= 2000) && (formula <= 2650))
			{
				// Source: http://crucible.samanna.net/viewtopic.php?f=38&t=6259
				result = ubase * (caster_level * (formula - 2000) + 1);
			}
			else
				LogFile->write(EQEMuLog::Debug, "Unknown spell effect value forumula %d", formula);
		}
	}

	int oresult = result;

	// now check result against the allowed maximum
	if (max != 0)
	{
		if (updownsign == 1)
		{
			if (result > max)
				result = max;
		}
		else
		{
			if (result < max)
				result = max;
		}
	}

	// if base is less than zero, then the result need to be negative too
	if (base < 0 && result > 0)
		result *= -1;

	mlog(SPELLS__EFFECT_VALUES, "Result: %d (orig %d), cap %d %s", result, oresult, max, (base < 0 && result > 0)?"Inverted due to negative base":"");

	return result;
}


void Mob::BuffProcess()
{
	uint32 buff_count = GetMaxTotalSlots();

	for (int buffs_i = 0; buffs_i < buff_count; ++buffs_i)
	{
		if (buffs[buffs_i].spellid != SPELL_UNKNOWN)
		{
			DoBuffTic(buffs[buffs_i].spellid, buffs_i, buffs[buffs_i].ticsremaining, buffs[buffs_i].casterlevel, entity_list.GetMob(buffs[buffs_i].casterid));
			// If the Mob died during DoBuffTic, then the buff we are currently processing will have been removed
			if(buffs[buffs_i].spellid == SPELL_UNKNOWN)
				continue;

			if(spells[buffs[buffs_i].spellid].buffdurationformula != DF_Permanent)
			{
				if(!zone->BuffTimersSuspended() || !IsSuspendableSpell(buffs[buffs_i].spellid))
				{
					--buffs[buffs_i].ticsremaining;

					if (buffs[buffs_i].ticsremaining == 0) {
						if (!IsShortDurationBuff(buffs[buffs_i].spellid) ||
							IsFearSpell(buffs[buffs_i].spellid) ||
							IsCharmSpell(buffs[buffs_i].spellid) ||
							IsMezSpell(buffs[buffs_i].spellid) ||
							IsBlindSpell(buffs[buffs_i].spellid))
						{
							mlog(SPELLS__BUFFS, "Buff %d in slot %d has expired. Fading.", buffs[buffs_i].spellid, buffs_i);
							BuffFadeBySlot(buffs_i);
						}
					}
					else if (buffs[buffs_i].ticsremaining < 0)
					{
						mlog(SPELLS__BUFFS, "Buff %d in slot %d has expired. Fading.", buffs[buffs_i].spellid, buffs_i);
						BuffFadeBySlot(buffs_i);
					}
					else
					{
						mlog(SPELLS__BUFFS, "Buff %d in slot %d has %d tics remaining.", buffs[buffs_i].spellid, buffs_i, buffs[buffs_i].ticsremaining);
					}
				}
				else if(IsClient() && !(CastToClient()->GetClientVersionBit() & BIT_SoFAndLater))
				{
					buffs[buffs_i].UpdateClient = true;
				}
			}

			if(IsClient())
			{
				if(buffs[buffs_i].UpdateClient == true)
				{
					CastToClient()->SendBuffDurationPacket(buffs[buffs_i].spellid, buffs[buffs_i].ticsremaining, buffs[buffs_i].casterlevel);
					buffs[buffs_i].UpdateClient = false;
				}
			}
		}
	}
}

void Mob::DoBuffTic(uint16 spell_id, int slot, uint32 ticsremaining, uint8 caster_level, Mob* caster) {
	int effect, effect_value;

	if(!IsValidSpell(spell_id))
		return;

	const SPDat_Spell_Struct &spell = spells[spell_id];

	if (spell_id == SPELL_UNKNOWN)
		return;

	if(IsNPC())
	{
		std::vector<void*> args;
		args.push_back(&ticsremaining);
		args.push_back(&caster_level);
		args.push_back(&slot);
		int i = parse->EventSpell(EVENT_SPELL_BUFF_TIC_NPC, CastToNPC(), nullptr, spell_id, caster ? caster->GetID() : 0, &args);
		if(i != 0) {
			return;
		}
	}
	else
	{
		std::vector<void*> args;
		args.push_back(&ticsremaining);
		args.push_back(&caster_level);
		args.push_back(&slot);
		int i = parse->EventSpell(EVENT_SPELL_BUFF_TIC_CLIENT, nullptr, CastToClient(), spell_id, caster ? caster->GetID() : 0, &args);
		if(i != 0) {
			return;
		}
	}

	// Check for non buff spell effects to fade
	// AE melee effects
	if(IsClient())
		CastToClient()->CheckAAEffect(aaEffectRampage);

	for (int i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spell_id, i))
			continue;

		effect = spell.effectid[i];
		//I copied the calculation into each case which needed it instead of
		//doing it every time up here, since most buff effects dont need it

		switch(effect)
		{
			case SE_CurrentHP:
			{
				effect_value = CalcSpellEffectValue(spell_id, i, caster_level, caster, ticsremaining);
				//Handle client cast DOTs here.
				if (caster && caster->IsClient() && IsDetrimentalSpell(spell_id) && effect_value < 0) {

					effect_value = caster->CastToClient()->GetActDoTDamage(spell_id, effect_value, this);

					if (!caster->CastToClient()->GetFeigned())
						AddToHateList(caster, -effect_value);
				}

				if(effect_value < 0)
				{
					if(caster)
					{
						if(!caster->IsClient()){

							if (!IsClient()) //Allow NPC's to generate hate if casted on other NPC's.
								AddToHateList(caster, -effect_value);
						}

						if(caster->IsNPC())
							effect_value = caster->CastToNPC()->GetActSpellDamage(spell_id, effect_value, this);
					}

					effect_value = -effect_value;
					Damage(caster, effect_value, spell_id, spell.skill, false, i, true);
				} else if(effect_value > 0) {
					// Regen spell...
					// handled with bonuses
				}
				break;
			}
			case SE_HealOverTime:
			{
				effect_value = CalcSpellEffectValue(spell_id, i, caster_level);
				if(caster)
					effect_value = caster->GetActSpellHealing(spell_id, effect_value);

				HealDamage(effect_value, caster, spell_id);
				//healing aggro would go here; removed for now
				break;
			}

			case SE_CurrentEndurance: {
				// Handled with bonuses
				break;
			}

			case SE_BardAEDot:
			{
				effect_value = CalcSpellEffectValue(spell_id, i, caster_level, caster);

				if (invulnerable || /*effect_value > 0 ||*/ DivineAura())
					break;

				if(effect_value < 0) {
					effect_value = -effect_value;
					if(caster){
						if(caster->IsClient() && !caster->CastToClient()->GetFeigned()){
							AddToHateList(caster, effect_value);
						}
						else if(!caster->IsClient())
							AddToHateList(caster, effect_value);
					}
					Damage(caster, effect_value, spell_id, spell.skill, false, i, true);
				} else if(effect_value > 0) {
					//healing spell...
					HealDamage(effect_value, caster);
					//healing aggro would go here; removed for now
				}
				break;
			}

			case SE_Hate2:{
				effect_value = CalcSpellEffectValue(spell_id, i, caster_level);
				if(caster){
					if(effect_value > 0){
						if(caster){
							if(caster->IsClient() && !caster->CastToClient()->GetFeigned()){
								AddToHateList(caster, effect_value);
							}
							else if(!caster->IsClient())
								AddToHateList(caster, effect_value);
						}
					}else{
						int32 newhate = GetHateAmount(caster) + effect_value;
						if (newhate < 1) {
							SetHate(caster,1);
						} else {
							SetHate(caster,newhate);
						}
					}
				}
				break;
			}

			case SE_WipeHateList:
			{

				int wipechance = spells[spell_id].base[i];
				int bonus = 0;
				
				if (caster){
					bonus =	caster->spellbonuses.IncreaseChanceMemwipe + 
							caster->itembonuses.IncreaseChanceMemwipe + 
							caster->aabonuses.IncreaseChanceMemwipe;
				}
				
				wipechance += wipechance*bonus/100;
				
				if(MakeRandomInt(0, 99) < wipechance)
				{
					if(IsAIControlled())
					{
						WipeHateList();
					}
					Message(13, "Your mind fogs. Who are my friends? Who are my enemies?... it was all so clear a moment ago...");
				}
				break;
			}

			case SE_Charm: {
				if (!caster || !PassCharismaCheck(caster, this, spell_id)) {
					BuffFadeByEffect(SE_Charm);
				}

				break;
			}

			case SE_Root: {
				
				/* Root formula derived from extensive personal live parses - Kayen
				ROOT has a 40% chance to do a resist check to break.
				Resist check has NO LOWER bounds.
				If multiple roots on target. Root in first slot will be checked first to break from nukes.
				If multiple roots on target and broken by spell. Roots are removed ONE at a time in order of buff slot.
				*/

				if (MakeRandomInt(0, 99) < RuleI(Spells, RootBreakCheckChance)){
				
					float resist_check = ResistSpell(spells[spell_id].resisttype, spell_id, caster);

					if(resist_check == 100) 
						break;
					else
						if(!TryFadeEffect(slot))
							BuffFadeBySlot(slot);
				}

				break;
			}

			case SE_Hunger: {
				// this procedure gets called 7 times for every once that the stamina update occurs so we add 1/7 of the subtraction.
				// It's far from perfect, but works without any unnecessary buff checks to bog down the server.
				if(IsClient()) {
					CastToClient()->m_pp.hunger_level += 5;
					CastToClient()->m_pp.thirst_level += 5;
				}
				break;
			}
			case SE_Invisibility:
			case SE_InvisVsAnimals:
			case SE_InvisVsUndead:
			{
				if(ticsremaining > 3)
				{
					if(!IsBardSong(spell_id))
					{
						double break_chance = 2.0;
						if(caster)
						{
							break_chance -= (2 * (((double)caster->GetSkill(SkillDivination) + ((double)caster->GetLevel() * 3.0)) / 650.0));
						}
						else
						{
							break_chance -= (2 * (((double)GetSkill(SkillDivination) + ((double)GetLevel() * 3.0)) / 650.0));
						}

						if(MakeRandomFloat(0.0, 100.0) < break_chance)
						{
							BuffModifyDurationBySpellID(spell_id, 3);
						}
					}
				}
			}
			case SE_Invisibility2:
			case SE_InvisVsUndead2:
			{
				if(ticsremaining <= 3 && ticsremaining > 1)
				{
					Message_StringID(MT_Spells, INVIS_BEGIN_BREAK);
				}
				break;
			}
			case SE_InterruptCasting:
			{
				if(IsCasting())
				{
					if(MakeRandomInt(0, 100) <= spells[spell_id].base[i])
					{
						InterruptSpell();
					}
				}
				break;
			}
			// These effects always trigger when they fade.
			case SE_ImprovedSpellEffect:
			case SE_BossSpellTrigger:
			case SE_CastOnWearoff:
			{
				if (ticsremaining == 1)
				{
					SpellOnTarget(spells[spell_id].base[i], this);
				}
				break;
			}
			case SE_LocateCorpse:
			{
				// This is handled by the client prior to SoD.

				if(IsClient() && (CastToClient()->GetClientVersionBit() & BIT_SoDAndLater))
					CastToClient()->LocateCorpse();
			}
			case SE_TotalHP:
			{
				if (spell.formula[i] > 1000 && spell.formula[i] < 1999)
				{
					// These formulas can affect Max HP each tick
					// Maybe there is a more efficient way to recalculate this for just Max HP each tic...
					//CalcBonuses();
					CalcSpellBonuses(&spellbonuses);
					CalcMaxHP();
				}
				break;
			}

			case SE_DistanceRemoval:
			{
				if (spellbonuses.DistanceRemoval){

					int distance =	((int(GetX()) - buffs[slot].caston_x) * (int(GetX()) - buffs[slot].caston_x)) + 
									((int(GetY()) - buffs[slot].caston_y) * (int(GetY()) - buffs[slot].caston_y)) +
									((int(GetZ()) - buffs[slot].caston_z) * (int(GetZ()) - buffs[slot].caston_z));

					if (distance > (spells[spell_id].base[i] * spells[spell_id].base[i])){

						if(!TryFadeEffect(slot))
							BuffFadeBySlot(slot , true);
					}
					break;
				}
			}

			default:
			{
				// do we need to do anyting here?
			}
		}
	}
}

// removes the buff in the buff slot 'slot'
void Mob::BuffFadeBySlot(int slot, bool iRecalcBonuses)
{
	if(slot < 0 || slot > GetMaxTotalSlots())
		return;

	if(!IsValidSpell(buffs[slot].spellid))
		return;

	if (IsClient() && !CastToClient()->IsDead())
		CastToClient()->MakeBuffFadePacket(buffs[slot].spellid, slot);

	mlog(SPELLS__BUFFS, "Fading buff %d from slot %d", buffs[slot].spellid, slot);

	if(spells[buffs[slot].spellid].viral_targets > 0) {
		bool last_virus = true;
		for(int i = 0; i < MAX_SPELL_TRIGGER*2; i+=2)
		{
			if(viral_spells[i] && viral_spells[i] != buffs[slot].spellid)
			{
				// If we have a virus that doesn't match this one then don't stop the viral timer
				last_virus = false;
			}
		}
		// This is the last virus on us so lets stop timer
		if(last_virus) {
			viral_timer.Disable();
			has_virus = false;
		}
	}

	if(IsClient()) {
		std::vector<void*> args;
		args.push_back(&buffs[slot].casterid);

		parse->EventSpell(EVENT_SPELL_FADE, nullptr, CastToClient(), buffs[slot].spellid, slot, &args);
	} else if(IsNPC()) {
		std::vector<void*> args;
		args.push_back(&buffs[slot].casterid);

		parse->EventSpell(EVENT_SPELL_FADE, CastToNPC(), nullptr, buffs[slot].spellid, slot, &args);
	}

	for (int i=0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(buffs[slot].spellid, i))
			continue;

		switch (spells[buffs[slot].spellid].effectid[i])
		{
			case SE_AddMeleeProc:
			case SE_WeaponProc:
			{
				uint16 procid = GetProcID(buffs[slot].spellid, i);
				RemoveProcFromWeapon(procid, false);
				break;
			}

			case SE_SkillProc2:
			case SE_SkillProc:
			{
				uint16 procid = GetProcID(buffs[slot].spellid, i);
				RemoveSkillProc(procid);
				break;
			}

			case SE_DefensiveProc:
			{
				uint16 procid = GetProcID(buffs[slot].spellid, i);
				RemoveDefensiveProc(procid);
				break;
			}

			case SE_RangedProc:
			{
				uint16 procid = GetProcID(buffs[slot].spellid, i);
				RemoveRangedProc(procid);
				break;
			}

			case SE_SummonHorse:
			{
				if(IsClient())
				{
					/*Mob* horse = entity_list.GetMob(this->CastToClient()->GetHorseId());
					if (horse) horse->Depop();
					CastToClient()->SetHasMount(false);*/
					CastToClient()->SetHorseId(0);
				}
				break;
			}

			case SE_IllusionCopy:
			case SE_Illusion:
			{
				SendIllusionPacket(0, GetBaseGender());
				if(GetRace() == OGRE){
					SendAppearancePacket(AT_Size, 9);
				}
				else if(GetRace() == TROLL){
					SendAppearancePacket(AT_Size, 8);
				}
				else if(GetRace() == VAHSHIR || GetRace() == FROGLOK || GetRace() == BARBARIAN){
					SendAppearancePacket(AT_Size, 7);
				}
				else if(GetRace() == HALF_ELF || GetRace() == WOOD_ELF || GetRace() == DARK_ELF){
					SendAppearancePacket(AT_Size, 5);
				}
				else if(GetRace() == DWARF){
					SendAppearancePacket(AT_Size, 4);
				}
				else if(GetRace() == HALFLING || GetRace() == GNOME){
					SendAppearancePacket(AT_Size, 3);
				}
				else{
					SendAppearancePacket(AT_Size, 6);
				}
				for(int x = 0; x < 7; x++){
					SendWearChange(x);
				}
				break;
			}

			case SE_Levitate:
			{
				if (!AffectedBySpellExcludingSlot(slot, SE_Levitate))
					SendAppearancePacket(AT_Levitate, 0);
				break;
			}

			case SE_Invisibility2:
			case SE_Invisibility:
			{
				SetInvisible(0);
				break;
			}

			case SE_InvisVsUndead2:
			case SE_InvisVsUndead:
			{
				invisible_undead = false;	// Mongrel: No longer IVU
				break;
			}

			case SE_InvisVsAnimals:
			{
				invisible_animals = false;
				break;
			}

			case SE_SeeInvis:
			{
				see_invis = 0;
				break;
			}

			case SE_Silence:
			{
				Silence(false);
				break;
			}

			case SE_Amnesia:
			{
				Amnesia(false);
				break;
			}

			case SE_DivineAura:
			{
				SetInvul(false);
				break;
			}

			case SE_Rune:
			{
				buffs[slot].melee_rune = 0;
				break;
			}

			case SE_AbsorbMagicAtt:
			{
				buffs[slot].magic_rune = 0;
				break;
			}

			case SE_Familiar:
			{
				Mob *mypet = GetPet();
				if (mypet){
					if(mypet->IsNPC())
						mypet->CastToNPC()->Depop();
					SetPetID(0);
				}
				break;
			}

			case SE_Mez:
			{
				SendAppearancePacket(AT_Anim, ANIM_STAND);	// unfreeze
				this->mezzed = false;
				break;
			}

			case SE_Charm:
			{
				if(IsNPC())
				{
					CastToNPC()->RestoreGuardSpotCharm();
					SendAppearancePacket(AT_Pet, 0, true, true);
				}

				Mob* tempmob = GetOwner();
				SetOwnerID(0);
				if(tempmob)
				{
					tempmob->SetPet(0);
				}
				if (IsAIControlled())
				{
					// clear the hate list of the mobs
					entity_list.ReplaceWithTarget(this, tempmob);
					WipeHateList();
					if(tempmob)
						AddToHateList(tempmob, 1, 0);
					SendAppearancePacket(AT_Anim, ANIM_STAND);
				}
				if(tempmob && tempmob->IsClient())
				{
					EQApplicationPacket *app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
					Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
					ps->owner_id = tempmob->GetID();
					ps->pet_id = this->GetID();
					ps->command = 0;
					entity_list.QueueClients(this, app);
					safe_delete(app);
				}
				if(IsClient())
				{
					InterruptSpell();
					if (this->CastToClient()->IsLD())
						AI_Start(CLIENT_LD_TIMEOUT);
					else
					{
						bool feared = FindType(SE_Fear);
						if(!feared)
							AI_Stop();
					}
				}
				break;
			}

			case SE_Root:
			{
				buffs[slot].RootBreakChance = 0;
				rooted = false;
				break;
			}

			case SE_Fear:
			{
				if(RuleB(Combat, EnableFearPathing)){
					if(IsClient())
					{
						bool charmed = FindType(SE_Charm);
						if(!charmed)
							AI_Stop();
					}

					if(curfp) {
						curfp = false;
						break;
					}
				}
				else
				{
					UnStun();
				}
				break;
			}

			case SE_ImmuneFleeing:
			{
				if(RuleB(Combat, EnableFearPathing)){
					if(flee_mode) {
						curfp = true;
						CheckFlee();
						break;
					}
				}
			}

			case SE_BindSight:
			{
				if(IsClient())
				{
					CastToClient()->SetBindSightTarget(nullptr);
				}
				break;
			}

			case SE_SetBodyType:
			{
				SetBodyType(GetOrigBodyType(), false);
				break;
			}

			case SE_MovementSpeed:
			{
				if(IsClient())
				{
					Client *my_c = CastToClient();
					uint32 cur_time = Timer::GetCurrentTime();
					if((cur_time - my_c->m_TimeSinceLastPositionCheck) > 1000)
					{
						float speed = (my_c->m_DistanceSinceLastPositionCheck * 100) / (float)(cur_time - my_c->m_TimeSinceLastPositionCheck);
						float runs = my_c->GetRunspeed();
						if(speed > (runs * RuleR(Zone, MQWarpDetectionDistanceFactor)))
						{
							if(!my_c->GetGMSpeed() && (runs >= my_c->GetBaseRunspeed() || (speed > (my_c->GetBaseRunspeed() * RuleR(Zone, MQWarpDetectionDistanceFactor)))))
							{
								printf("%s %i moving too fast! moved: %.2f in %ims, speed %.2f\n", __FILE__, __LINE__,
									my_c->m_DistanceSinceLastPositionCheck, (cur_time - my_c->m_TimeSinceLastPositionCheck), speed);
								if(my_c->IsShadowStepExempted())
								{
									if(my_c->m_DistanceSinceLastPositionCheck > 800)
									{
										my_c->CheatDetected(MQWarpShadowStep, my_c->GetX(), my_c->GetY(), my_c->GetZ());
									}
								}
								else if(my_c->IsKnockBackExempted())
								{
									//still potential to trigger this if you're knocked back off a
									//HUGE fall that takes > 2.5 seconds
									if(speed > 30.0f)
									{
										my_c->CheatDetected(MQWarpKnockBack, my_c->GetX(), my_c->GetY(), my_c->GetZ());
									}
								}
								else if(!my_c->IsPortExempted())
								{
									if(!my_c->IsMQExemptedArea(zone->GetZoneID(), my_c->GetX(), my_c->GetY(), my_c->GetZ()))
									{
										if(speed > (runs * 2 * RuleR(Zone, MQWarpDetectionDistanceFactor)))
										{
											my_c->m_TimeSinceLastPositionCheck = cur_time;
											my_c->m_DistanceSinceLastPositionCheck = 0.0f;
											my_c->CheatDetected(MQWarp, my_c->GetX(), my_c->GetY(), my_c->GetZ());
											//my_c->Death(my_c, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
										}
										else
										{
											my_c->CheatDetected(MQWarpLight, my_c->GetX(), my_c->GetY(), my_c->GetZ());
										}
									}
								}
							}
						}
					}
					my_c->m_TimeSinceLastPositionCheck = cur_time;
					my_c->m_DistanceSinceLastPositionCheck = 0.0f;
				}
			}
		}
	}

	// notify caster (or their master) of buff that it's worn off
	Mob *p = entity_list.GetMob(buffs[slot].casterid);
	if (p && p != this && !IsBardSong(buffs[slot].spellid))
	{
		Mob *notify = p;
		if(p->IsPet())
			notify = p->GetOwner();
		if(p) {
			notify->Message_StringID(MT_WornOff, SPELL_WORN_OFF_OF,
				spells[buffs[slot].spellid].name, GetCleanName());
		}
	}

	if (HasNumhits()){

		uint32 buff_max = GetMaxTotalSlots();
		bool found_numhits = false;
		
		for(uint32 d = 0; d < buff_max; d++) {
	
			if(IsValidSpell(buffs[d].spellid) && (buffs[d].numhits > 0)) {
				Numhits(true);
				found_numhits = true;
			}
		}

		if (!found_numhits)
			Numhits(false);
	}

	buffs[slot].spellid = SPELL_UNKNOWN;
	if(IsPet() && GetOwner() && GetOwner()->IsClient()) {
		SendPetBuffsToClient();
	}
	if((IsClient() && !CastToClient()->GetPVP()) || (IsPet() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
				(IsMerc() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()))
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();

		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, BIT_SoDAndLater);
		if(GetTarget() == this) {
			CastToClient()->QueuePacket(outapp);
		}

		safe_delete(outapp);
	}

	if(IsClient() && CastToClient()->GetClientVersionBit() & BIT_UnderfootAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket(false);
		CastToClient()->FastQueuePacket(&outapp);
	}

	if (iRecalcBonuses)
		CalcBonuses();
}

/* No longer used.
int16 Client::CalcAAFocusEffect(focusType type, uint16 focus_spell, uint16 spell_id)
{
	uint32 slots = 0;
	uint32 aa_AA = 0;
	uint32 aa_value = 0;

	int32 value = 0;
	// Iterate through all of the client's AAs
	for (int i = 0; i < MAX_PP_AA_ARRAY; i++)
	{
		aa_AA = this->aa[i]->AA;
		aa_value = this->aa[i]->value;
		if (aa_AA > 0 || aa_value > 0)
		{
			slots = zone->GetTotalAALevels(aa_AA);
			if (slots > 0)
			for(int j = 1;j <= slots; j++)
			{
				switch (aa_effects[aa_AA][j].skill_id)
				{
					case SE_TriggerOnCast:
						// If focus_spell matches the spell listed in the DB, load these restrictions
						if(type == focusTriggerOnCast && focus_spell == aa_effects[aa_AA][j].base1)
							value = CalcAAFocus(type, aa_AA, spell_id);
					break;
				}
			}
		}
	}
	return value;
}
*/


int16 Client::CalcAAFocus(focusType type, uint32 aa_ID, uint16 spell_id)
{
	const SPDat_Spell_Struct &spell = spells[spell_id];

	int16 value = 0;
	int lvlModifier = 100;
	int spell_level = 0;
	int lvldiff = 0;
	uint32 effect = 0;
	int32 base1 = 0;
	int32 base2 = 0;
	uint32 slot = 0;

	bool LimitFailure = false;
	bool LimitInclude[MaxLimitInclude] = { false }; 
	int FocusCount = 0;

	std::map<uint32, std::map<uint32, AA_Ability> >::const_iterator find_iter = aa_effects.find(aa_ID);
	if(find_iter == aa_effects.end())
	{
		return 0;
	}

	for (std::map<uint32, AA_Ability>::const_iterator iter = aa_effects[aa_ID].begin(); iter != aa_effects[aa_ID].end(); ++iter)
	{
		effect = iter->second.skill_id;
		base1 = iter->second.base1;
		base2 = iter->second.base2;
		slot = iter->second.slot;

		/*
		AA Foci's can contain multiple focus effects within the same AA.
		To handle this we will not automatically return zero if a limit is found.
		Instead if limit is found and multiple focus effects, we will reset the limit check
		when the next valid focus effect is found.
		*/

		if (IsFocusEffect(0, 0, true,effect) || (effect == SE_TriggerOnCast)){
			FocusCount++;
			//If limit found on prior check next, else end loop.
			if (FocusCount > 1){

				for(int e = 0; e < MaxLimitInclude; e+=2) {
					if (LimitInclude[e] && !LimitInclude[e+1])
						LimitFailure = true;
				}

				if (LimitFailure){
					value = 0;
					LimitFailure = false;
					
					for(int e = 0; e < MaxLimitInclude; e++) {
						LimitInclude[e] = false; //Reset array
					}
				}

				else{
					break;
				}
			}
		}


		switch (effect)
		{
			case SE_Blank:
				break;

			//Handle Focus Limits

			case SE_LimitResist:
				if(base1 < 0){
					if(spell.resisttype == -base1) //Exclude
						LimitFailure = true;
				}
				else {
				LimitInclude[0] = true;
				if (spell.resisttype == base1) //Include
					LimitInclude[1] = true;
				}
				break;

			case SE_LimitInstant:
				if(spell.buffduration)
					LimitFailure = true;
				break;
			
			case SE_LimitMaxLevel:
				spell_level = spell.classes[(GetClass()%16) - 1];
				lvldiff = spell_level - base1;
				//every level over cap reduces the effect by base2 percent unless from a clicky when ItemCastsUseFocus is true
				if(lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) || RuleB(Character, ItemCastsUseFocus) == false))	{
					if(base2 > 0){
						lvlModifier -= base2*lvldiff;
						if(lvlModifier < 1)
							LimitFailure = true;
					}
					else 
						LimitFailure = true;
				}
				break;

			case SE_LimitMinLevel:
				if((spell.classes[(GetClass()%16) - 1]) < base1)
					LimitFailure = true;
				break;

			case SE_LimitCastTimeMin:
				if (spell.cast_time < base1)
					LimitFailure = true;
				break;

			case SE_LimitCastTimeMax:
				if (spell.cast_time > base1)
					LimitFailure = true;
				break;

			case SE_LimitSpell:
				if(base1 < 0) {	//Exclude
					if (spell_id == -base1)
						LimitFailure = true;
				} 
				else {
					LimitInclude[2] = true;
					if (spell_id == base1) //Include
						LimitInclude[3] = true;
				}
				break;

			case SE_LimitMinDur:
				if (base1 > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
					LimitFailure = true;
					break;

			case SE_LimitEffect:
				if(base1 < 0){
					if(IsEffectInSpell(spell_id,-base1)) //Exclude
						LimitFailure = true;
					}
				else{
					LimitInclude[4] = true;
					if(IsEffectInSpell(spell_id,base1)) //Include
						LimitInclude[5] = true;
				}
				break;

			case SE_LimitSpellType:
				switch(base1)
				{
					case 0:
						if (!IsDetrimentalSpell(spell_id))
							LimitFailure = true;
						break;
					case 1:
						if (!IsBeneficialSpell(spell_id))
							LimitFailure = true;
						break;
				}
				break;

			case SE_LimitManaMin:
				if(spell.mana < base1)
					LimitFailure = true;
				break;

			case SE_LimitManaMax:
				if(spell.mana > base1)
					LimitFailure = true;
				break;

			case SE_LimitTarget:
				if (base1 < 0) {
					if (-base1 == spell.targettype) //Exclude
						LimitFailure = true;
				}
				else {
					LimitInclude[6] = true;
					if (base1 == spell.targettype) //Include
						LimitInclude[7] = true;
				}
				break;

			case SE_LimitCombatSkills:
				if (base1 == 0 && IsCombatSkill(spell_id)) //Exclude Discs
					LimitFailure = true;
				else if (base1 == 1 && !IsCombatSkill(spell_id)) //Exclude Spells
					LimitFailure = true;
			break;

			case SE_LimitSpellGroup:
				if(base1 < 0) {
					if (-base1 == spell.spellgroup) //Exclude
						LimitFailure = true;
				}
				else {
					LimitInclude[8] = true;
					if (base1 == spell.spellgroup) //Include
						LimitInclude[9] = true;
				}
				break;

			case SE_LimitCastingSkill:
			if(base1 < 0) {
				if(-base1 == spell.skill)
					LimitFailure = true;
			}
			else {
				LimitInclude[10] = true;
				if(base1 == spell.skill)
					LimitInclude[11] = true;
			}
			break;

			case SE_LimitClass:
			//Do not use this limit more then once per spell. If multiple class, treat value like items would.
			if (!PassLimitClass(base1, GetClass()))
				LimitFailure = true;
				break;

			case SE_LimitRace:
				if (base1 != GetRace())
					LimitFailure = true;
				break;

			case SE_LimitUseMin:
				if (base1 > spell.numhits)
					LimitFailure = true;
				break;

			case SE_LimitUseType:
				if (base1 != spell.numhitstype)
					LimitFailure = true;
				break;

			//Handle Focus Effects
			case SE_ImprovedDamage:
				if (type == focusImprovedDamage && base1 > value)
					value = base1;
				break;

			case SE_ImprovedHeal:
				if (type == focusImprovedHeal && base1 > value)
					value = base1;
				break;

			case SE_ReduceManaCost:
				if (type == focusManaCost)
					value = base1;
			break;

			case SE_IncreaseSpellHaste:
				if (type == focusSpellHaste && base1 > value)
					value = base1;
				break;

			case SE_IncreaseSpellDuration:
				if (type == focusSpellDuration && base1 > value)
					value = base1;
				break;

			case SE_SpellDurationIncByTic:
				if (type == focusSpellDurByTic && base1 > value)
					value = base1;
				break;

			case SE_SwarmPetDuration:
				if (type == focusSwarmPetDuration && base1 > value)
						value = base1;
				break;

			case SE_IncreaseRange:
				if (type == focusRange && base1 > value)
					value = base1;
				break;

			case SE_ReduceReagentCost:
				if (type == focusReagentCost && base1 > value)
					value = base1;
				break;

			case SE_PetPowerIncrease:
				if (type == focusPetPower && base1 > value)
					value = base1;
				break;

			case SE_SpellResistReduction:
				if (type == focusResistRate && base1 > value)
					value = base1;
				break;

			case SE_SpellHateMod:
				if (type == focusSpellHateMod ) {
					if(value != 0) {
						if(value > 0){
							if(base1 > value)
								value = base1;
						}
						else{
							if(base1 < value)
								value = base1;
						}
					}
					else
						value = base1;
				}
				break;

			case SE_ReduceReuseTimer:
				if(type == focusReduceRecastTime)
					value = base1 / 1000;
				break;

			case SE_TriggerOnCast:
				if(type == focusTriggerOnCast){
					if(MakeRandomInt(0, 100) <= base1){
						value = base2;
					}

					else{
						value = 0;
						LimitFailure = true;
					}
				break;
				}

			case SE_FcSpellVulnerability:
				if(type == focusSpellVulnerability)
					value = base1;
				break;

			case SE_BlockNextSpellFocus:
				if(type == focusBlockNextSpell){
					if(MakeRandomInt(1, 100) <= base1)
						value = 1;
				}
				break;

			case SE_FcTwincast:
				if(type == focusTwincast)
					value = base1;
				break;

			case SE_SympatheticProc:
				//No AA support at this time.
				break;

			case SE_FcDamageAmt:
				if(type == focusFcDamageAmt)
					value = base1;
				break;

			case SE_FcDamageAmtCrit:
				if(type == focusFcDamageAmtCrit)
					value = base1;
				break;

			case SE_FcDamageAmtIncoming:
				if(type == focusFcDamageAmtIncoming)
					value = base1;
				break;

			case SE_FcHealAmtIncoming:
				if(type == focusFcHealAmtIncoming)
					value = base1;
				break;

			case SE_FcHealPctCritIncoming:
				if (type == focusFcHealPctCritIncoming)
					value = base1;
				break;

			case SE_FcHealAmtCrit:
				if(type == focusFcHealAmtCrit)
					value = base1;
				break;

			case  SE_FcHealAmt:
				if(type == focusFcHealAmt)
					value = base1;
				break;

			case SE_FcHealPctIncoming:
				if(type == focusFcHealPctIncoming)
					value = base1;
				break;

			case SE_FcBaseEffects:
				if (type == focusFcBaseEffects)
					value = base1;
				break;

			case SE_FcDamagePctCrit:
				if(type == focusFcDamagePctCrit)
					value = base1;
				break;

			case SE_FcIncreaseNumHits:
				if(type == focusIncreaseNumHits)
					value = base1;
				break;
		
			case SE_FcLimitUse:
				if(type == focusFcLimitUse)
					value = base1;
				break;
			
			case SE_FcMute:
				if(type == focusFcMute)
					value = base1;
				break;

			case SE_FcStunTimeMod:
				if(type == focusFcStunTimeMod)
					value = base1;
				break;

		}
	}

	for(int e = 0; e < MaxLimitInclude; e+=2) {
		if (LimitInclude[e] && !LimitInclude[e+1])
			return 0;
	}

	if (LimitFailure)
		return 0;
	
	return(value*lvlModifier/100);
}

//given an item/spell's focus ID and the spell being cast, determine the focus ammount, if any
//assumes that spell_id is not a bard spell and that both ids are valid spell ids
int16 Mob::CalcFocusEffect(focusType type, uint16 focus_id, uint16 spell_id, bool best_focus) {

	if(!IsValidSpell(focus_id) || !IsValidSpell(spell_id))
		return 0;

	
	const SPDat_Spell_Struct &focus_spell = spells[focus_id];
	const SPDat_Spell_Struct &spell = spells[spell_id];

	int16 value = 0;
	int lvlModifier = 100;
	int spell_level = 0;
	int lvldiff = 0;
	uint32 Caston_spell_id = 0;

	bool LimitInclude[MaxLimitInclude] = { false }; 
	/* Certain limits require only one of several Include conditions to be true. Ie. Add damage to fire OR ice spells.
	0/1   SE_LimitResist
	2/3   SE_LimitSpell
	4/5   SE_LimitEffect
	6/7   SE_LimitTarget
	8/9   SE_LimitSpellGroup:
	10/11 SE_LimitCastingSkill:
	Remember: Update MaxLimitInclude in spdat.h if adding new limits that require Includes
	*/ 
	
	for (int i = 0; i < EFFECT_COUNT; i++) {

		switch (focus_spell.effectid[i]) {
		
		case SE_Blank:
			break;
			
		case SE_LimitResist:
			if(focus_spell.base[i] < 0){
				if (spell.resisttype == -focus_spell.base[i]) //Exclude
					return 0;
			}
			else {
				LimitInclude[0] = true;
				if (spell.resisttype == focus_spell.base[i]) //Include
					LimitInclude[1] = true;
			}
			break;
		
		case SE_LimitInstant:
			if(spell.buffduration)
				return 0;
			break;

		case SE_LimitMaxLevel:
			if (IsNPC())
				break;
			spell_level = spell.classes[(GetClass()%16) - 1];
			lvldiff = spell_level - focus_spell.base[i];
			//every level over cap reduces the effect by focus_spell.base2[i] percent unless from a clicky when ItemCastsUseFocus is true
			if(lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) || RuleB(Character, ItemCastsUseFocus) == false)){
				if(focus_spell.base2[i] > 0){
					lvlModifier -= focus_spell.base2[i]*lvldiff;
					if(lvlModifier < 1)
						return 0;
				}
				else
					return 0;
			}
			break;
		
		case SE_LimitMinLevel:
			if (IsNPC())
				break;
			if (spell.classes[(GetClass()%16) - 1] < focus_spell.base[i])
				return(0);
			break;

		case SE_LimitCastTimeMin:
			if (spells[spell_id].cast_time < (uint16)focus_spell.base[i])
				return(0);
			break;

		case SE_LimitCastTimeMax:
			if (spells[spell_id].cast_time > (uint16)focus_spell.base[i])
				return(0);
			break;
			
		case SE_LimitSpell:
			if(focus_spell.base[i] < 0) {	//Exclude
				if (spell_id == -focus_spell.base[i])
					return(0);
			} 
			else {
				LimitInclude[2] = true;
				if (spell_id == focus_spell.base[i]) //Include
					LimitInclude[3] = true;
			}
			break;

		case SE_LimitMinDur:
				if (focus_spell.base[i] > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
					return(0);
			break;

		case SE_LimitEffect:
			if(focus_spell.base[i] < 0){
				if(IsEffectInSpell(spell_id,-focus_spell.base[i])) //Exclude
					return 0;
				}
			else{
				LimitInclude[4] = true;
				if(IsEffectInSpell(spell_id,focus_spell.base[i])) //Include
					LimitInclude[5] = true;
			}
			break;


		case SE_LimitSpellType:
			switch( focus_spell.base[i]){
				case 0:
					if (!IsDetrimentalSpell(spell_id))
						return 0;
					break;
				case 1:
					if (!IsBeneficialSpell(spell_id))
						return 0;
					break;
				default:
					LogFile->write(EQEMuLog::Normal, "CalcFocusEffect: unknown limit spelltype %d", focus_spell.base[i]);
			}
			break;

		case SE_LimitManaMin:
			if(spell.mana < focus_spell.base[i])
				return 0;
			break;

		case SE_LimitManaMax:
			if(spell.mana > focus_spell.base[i])
				return 0;
			break;

		case SE_LimitTarget:
			if (focus_spell.base[i] < 0) {
				if (-focus_spell.base[i] == spell.targettype) //Exclude
					return 0;
			}
			else {
				LimitInclude[6] = true;
				if (focus_spell.base[i] == spell.targettype) //Include
					LimitInclude[7] = true;
			}
			break;

		case SE_LimitCombatSkills:
			if (focus_spell.base[i] == 0 && IsCombatSkill(spell_id)) //Exclude Disc
				return 0;				
			else if (focus_spell.base[i] == 1 && !IsCombatSkill(spell_id)) //Include Spells
				return 0;
			break;

		case SE_LimitSpellGroup:
			if(focus_spell.base[i] < 0) {
				if (-focus_spell.base[i] == spell.spellgroup) //Exclude
					return 0;
			}
			else {
				LimitInclude[8] = true;
				if (focus_spell.base[i] == spell.spellgroup) //Include
					LimitInclude[9] = true;
			}
			break;

		case SE_LimitCastingSkill:
			if(focus_spell.base[i] < 0) {
				if(-focus_spell.base[i] == spell.skill)
					return 0;
			}
			else {
				LimitInclude[10] = true;
				if(focus_spell.base[i] == spell.skill)
					LimitInclude[11] = true;
			}
			break;
		
		case SE_LimitClass:
			//Do not use this limit more then once per spell. If multiple class, treat value like items would.
			if (!PassLimitClass(focus_spell.base[i], GetClass()))
				return 0;
			break;

		case SE_LimitRace:
			if (focus_spell.base[i] != GetRace())
				return 0;
			break;

		case SE_LimitUseMin:
			if (focus_spell.base[i] > spell.numhits)
				return 0;
			break;

		case SE_LimitUseType:
			if (focus_spell.base[i] != spell.numhitstype)
				return 0;
			break;

		case SE_CastonFocusEffect:
			if (focus_spell.base[i] > 0)
				Caston_spell_id = focus_spell.base[i];
			break;


		//handle effects
		case SE_ImprovedDamage:
			if (type == focusImprovedDamage) {
				// This is used to determine which focus should be used for the random calculation
				if(best_focus) {
					// If the spell contains a value in the base2 field then that is the max value
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					}
					// If the spell does not contain a base2 value, then its a straight non random value
					else {
						value = focus_spell.base[i];
					}
				}
				// Actual focus calculation starts here
				else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				}
				else {
					value = MakeRandomInt(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_ImprovedHeal:
			if (type == focusImprovedHeal) {
				if(best_focus) {
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					}
					else {
						value = focus_spell.base[i];
					}
				}
				else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				}
				else {
					value = MakeRandomInt(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_ReduceManaCost:
			if (type == focusManaCost) {
				if(best_focus) {
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					}
					else {
						value = focus_spell.base[i];
					}
				}
				else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				}
				else {
					value = MakeRandomInt(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_IncreaseSpellHaste:
			if (type == focusSpellHaste && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;
		
		case SE_IncreaseSpellDuration:
			if (type == focusSpellDuration && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;

		case SE_SpellDurationIncByTic:
			if (type == focusSpellDurByTic && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;

		case SE_SwarmPetDuration:
			if (type == focusSwarmPetDuration && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;

		case SE_IncreaseRange:
			if (type == focusRange && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;

		case SE_ReduceReagentCost:
			if (type == focusReagentCost && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;

		case SE_PetPowerIncrease:
			if (type == focusPetPower && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;

		case SE_SpellResistReduction:
			if (type == focusResistRate && focus_spell.base[i] > value)
				value = focus_spell.base[i];
			break;

		case SE_SpellHateMod:
			if (type == focusSpellHateMod){
				if(value != 0){
					if(value > 0){
						if(focus_spell.base[i] > value)
							value = focus_spell.base[i];
					}
					else{
						if(focus_spell.base[i] < value)
							value = focus_spell.base[i];
					}
				}
				else
					value = focus_spell.base[i];
			}
			break;

		case SE_ReduceReuseTimer:
			if(type == focusReduceRecastTime)
				value = focus_spell.base[i] / 1000;
			break;

		case SE_TriggerOnCast:
			if(type == focusTriggerOnCast){
				if(MakeRandomInt(1, 100) <= focus_spell.base[i])
					value = focus_spell.base2[i];
				else
					value = 0;
			}
			break;

		case SE_BlockNextSpellFocus:
			if(type == focusBlockNextSpell){
				if(MakeRandomInt(1, 100) <= focus_spell.base[i])
					value = 1;
			}
			break;

		case SE_SympatheticProc:
			if(type == focusSympatheticProc) {
				float ProcChance, ProcBonus;
				int16 ProcRateMod = focus_spell.base[i]; //Baseline is 100 for most Sympathetic foci
				int32 cast_time = GetActSpellCasttime(spell_id, spells[spell_id].cast_time);
				GetSympatheticProcChances(ProcBonus, ProcChance, cast_time, ProcRateMod);

				if(MakeRandomFloat(0, 1) <= ProcChance)
					value = focus_id;

				else
					value = 0;
			}
			break;

		case SE_FcSpellVulnerability:
			if(type == focusSpellVulnerability)
				value = focus_spell.base[i];
			break;

		case SE_FcTwincast:
			if(type == focusTwincast)
				value = focus_spell.base[i];
			break;

		case SE_FcDamageAmt:
			if(type == focusFcDamageAmt)
				value = focus_spell.base[i];
			break;

		case SE_FcDamageAmtCrit:
			if(type == focusFcDamageAmtCrit)
				value = focus_spell.base[i];
			break;

		case SE_FcDamageAmtIncoming:
			if(type == focusFcDamageAmtIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcHealAmtIncoming:
			if(type == focusFcHealAmtIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcDamagePctCrit:
			if(type == focusFcDamagePctCrit)
				value = focus_spell.base[i];
			break;

		case SE_FcHealPctCritIncoming:
			if (type == focusFcHealPctCritIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcHealAmtCrit:
			if(type == focusFcHealAmtCrit)
				value = focus_spell.base[i];
			break;

		case  SE_FcHealAmt:
			if(type == focusFcHealAmt)
				value = focus_spell.base[i];
			break;

		case SE_FcHealPctIncoming:
			if(type == focusFcHealPctIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcBaseEffects:
			if (type == focusFcBaseEffects)
				value = focus_spell.base[i];
			break;

		case SE_FcIncreaseNumHits:
			if(type == focusIncreaseNumHits)
				value = focus_spell.base[i];
			break;

		case SE_FcLimitUse:
			if(type == focusFcLimitUse)
				value = focus_spell.base[i];
			break;

		case SE_FcMute:
			if(type == focusFcMute)
				value = focus_spell.base[i];
			break;

		case SE_FcStunTimeMod:
			if(type == focusFcStunTimeMod)
				value = focus_spell.base[i];
			break;

		case SE_FcTimerRefresh:
			if(type == focusFcTimerRefresh)
				value = focus_spell.base[i];
			break;

#if EQDEBUG >= 6
		//this spits up a lot of garbage when calculating spell focuses
		//since they have all kinds of extra effects on them.
		default:
			LogFile->write(EQEMuLog::Normal, "CalcFocusEffect: unknown effectid %d", focus_spell.effectid[i]);
#endif
		}
		
	}

	for(int e = 0; e < MaxLimitInclude; e+=2) {
		if (LimitInclude[e] && !LimitInclude[e+1])
			return 0;
	}
	
	if (Caston_spell_id){
		if(IsValidSpell(Caston_spell_id) && (Caston_spell_id != spell_id))
			SpellFinished(Caston_spell_id, this, 10, 0, -1, spells[Caston_spell_id].ResistDiff);
	}

	return(value*lvlModifier/100);
}

int16 Client::GetSympatheticFocusEffect(focusType type, uint16 spell_id) {

	if (IsBardSong(spell_id))
		return 0;

	uint16 proc_spellid = 0;
	uint8 SizeProcList = 0;
	uint8 MAX_SYMPATHETIC = 10;

	std::vector<int> SympatheticProcList;

	//item focus
	if (itembonuses.FocusEffects[type]){

		const Item_Struct* TempItem = 0;

		for(int x=0; x<=21; x++)
		{
			if (SizeProcList > MAX_SYMPATHETIC)
				continue;

			TempItem = nullptr;
			ItemInst* ins = GetInv().GetItem(x);
			if (!ins)
				continue;
			TempItem = ins->GetItem();
			if (TempItem && TempItem->Focus.Effect > 0 && TempItem->Focus.Effect != SPELL_UNKNOWN) {

					proc_spellid = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);

					if (proc_spellid > 0)
					{
						SympatheticProcList.push_back(proc_spellid);
						SizeProcList = SympatheticProcList.size();
					}
			}

			for(int y = 0; y < MAX_AUGMENT_SLOTS; ++y)
			{
				if (SizeProcList > MAX_SYMPATHETIC)
					continue;

				ItemInst *aug = nullptr;
				aug = ins->GetAugment(y);
				if(aug)
				{
					const Item_Struct* TempItemAug = aug->GetItem();
					if (TempItemAug && TempItemAug->Focus.Effect > 0 && TempItemAug->Focus.Effect != SPELL_UNKNOWN) {

						proc_spellid = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_id);

						if (proc_spellid > 0)
						{
							SympatheticProcList.push_back(proc_spellid);
							SizeProcList = SympatheticProcList.size();
						}
					}
				}
			}
		}
	}

	//Spell Focus
	if (spellbonuses.FocusEffects[type]){
		int buff_slot = 0;
		uint16 focusspellid = 0;
		uint32 buff_max = GetMaxTotalSlots();
		for (buff_slot = 0; buff_slot < buff_max; buff_slot++) {

			if (SizeProcList > MAX_SYMPATHETIC)
				continue;

			focusspellid = buffs[buff_slot].spellid;
			if (focusspellid == 0 || focusspellid >= SPDAT_RECORDS)
				continue;

				proc_spellid = CalcFocusEffect(type, focusspellid, spell_id);

				if (proc_spellid > 0)
				{
					SympatheticProcList.push_back(proc_spellid);
					SizeProcList = SympatheticProcList.size();
				}
		}
	}

	if (SizeProcList > 0)
	{
		uint8 random = MakeRandomInt(0, SizeProcList-1);
		int FinalSympatheticProc = SympatheticProcList[random];
		SympatheticProcList.clear();
		return FinalSympatheticProc;
	}

	return 0;
}

int16 Client::GetFocusEffect(focusType type, uint16 spell_id) {

	if (IsBardSong(spell_id) && type != focusFcBaseEffects)
		return 0;

	int16 realTotal = 0;
	int16 realTotal2 = 0;
	int16 realTotal3 = 0;
	bool rand_effectiveness = false;

	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if((type == focusManaCost || type == focusImprovedHeal || type == focusImprovedDamage)
		&& RuleB(Spells, LiveLikeFocusEffects))
	{
		rand_effectiveness = true;
	}

	//Check if item focus effect exists for the client.
	if (itembonuses.FocusEffects[type]){

		const Item_Struct* TempItem = 0;
		const Item_Struct* UsedItem = 0;
		uint16 UsedFocusID = 0;
		int16 Total = 0;
		int16 focus_max = 0;
		int16 focus_max_real = 0;

		//item focus
		for(int x=0; x<=21; x++)
		{
			TempItem = nullptr;
			ItemInst* ins = GetInv().GetItem(x);
			if (!ins)
				continue;
			TempItem = ins->GetItem();
			if (TempItem && TempItem->Focus.Effect > 0 && TempItem->Focus.Effect != SPELL_UNKNOWN) {
				if(rand_effectiveness) {
					focus_max = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id, true);
					if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (focus_max < 0 && focus_max < focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
				else {
					Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
					if (Total > 0 && realTotal >= 0 && Total > realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (Total < 0 && Total < realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
			}

			for(int y = 0; y < MAX_AUGMENT_SLOTS; ++y)
			{
				ItemInst *aug = nullptr;
				aug = ins->GetAugment(y);
				if(aug)
				{
					const Item_Struct* TempItemAug = aug->GetItem();
					if (TempItemAug && TempItemAug->Focus.Effect > 0 && TempItemAug->Focus.Effect != SPELL_UNKNOWN) {
						if(rand_effectiveness) {
							focus_max = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_id, true);
							if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
								focus_max_real = focus_max;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							} else if (focus_max < 0 && focus_max < focus_max_real) {
								focus_max_real = focus_max;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							}
						}
						else {
							Total = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_id);
							if (Total > 0 && realTotal >= 0 && Total > realTotal) {
								realTotal = Total;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							} else if (Total < 0 && Total < realTotal) {
								realTotal = Total;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							}
						}
					}
				}
			}
		}

		//Tribute Focus
		for(int x = TRIBUTE_SLOT_START; x < (TRIBUTE_SLOT_START + MAX_PLAYER_TRIBUTES); ++x)
		{
			TempItem = nullptr;
			ItemInst* ins = GetInv().GetItem(x);
			if (!ins)
				continue;
			TempItem = ins->GetItem();
			if (TempItem && TempItem->Focus.Effect > 0 && TempItem->Focus.Effect != SPELL_UNKNOWN) {
				if(rand_effectiveness) {
					focus_max = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id, true);
					if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (focus_max < 0 && focus_max < focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
				else {
					Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
					if (Total > 0 && realTotal >= 0 && Total > realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
					else if (Total < 0 && Total < realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
			}
		}

		if(UsedItem && rand_effectiveness && focus_max_real != 0)
			realTotal = CalcFocusEffect(type, UsedFocusID, spell_id);

		if (realTotal != 0 && UsedItem)
			Message_StringID(MT_Spells, BEGINS_TO_GLOW, UsedItem->Name);
	}

	//Check if spell focus effect exists for the client.
	if (spellbonuses.FocusEffects[type]){

		//Spell Focus
		int16 Total2 = 0;
		int16 focus_max2 = 0;
		int16 focus_max_real2 = 0;

		int buff_tracker = -1;
		int buff_slot = 0;
		uint16 focusspellid = 0;
		uint16 focusspell_tracker = 0;
		uint32 buff_max = GetMaxTotalSlots();
		for (buff_slot = 0; buff_slot < buff_max; buff_slot++) {
			focusspellid = buffs[buff_slot].spellid;
			if (focusspellid == 0 || focusspellid >= SPDAT_RECORDS)
				continue;

			if(rand_effectiveness) {
				focus_max2 = CalcFocusEffect(type, focusspellid, spell_id, true);
				if (focus_max2 > 0 && focus_max_real2 >= 0 && focus_max2 > focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (focus_max2 < 0 && focus_max2 < focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
			else {
				Total2 = CalcFocusEffect(type, focusspellid, spell_id);
				if (Total2 > 0 && realTotal2 >= 0 && Total2 > realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (Total2 < 0 && Total2 < realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
		}

		if(focusspell_tracker && rand_effectiveness && focus_max_real2 != 0)
			realTotal2 = CalcFocusEffect(type, focusspell_tracker, spell_id);

		// For effects like gift of mana that only fire once, save the spellid into an array that consists of all available buff slots.
		if(buff_tracker >= 0 && buffs[buff_tracker].numhits > 0) {
			m_spellHitsLeft[buff_tracker] = focusspell_tracker;
		}
	}


	// AA Focus
	if (aabonuses.FocusEffects[type]){

		int16 Total3 = 0;
		uint32 slots = 0;
		uint32 aa_AA = 0;
		uint32 aa_value = 0;

		for (int i = 0; i < MAX_PP_AA_ARRAY; i++)
		{
			aa_AA = this->aa[i]->AA;
			aa_value = this->aa[i]->value;
			if (aa_AA < 1 || aa_value < 1)
				continue;

			Total3 = CalcAAFocus(type, aa_AA, spell_id);
			if (Total3 > 0 && realTotal3 >= 0 && Total3 > realTotal3) {
				realTotal3 = Total3;
			}
			else if (Total3 < 0 && Total3 < realTotal3) {
				realTotal3 = Total3;
			}
		}
	}

	if(type == focusReagentCost && IsSummonPetSpell(spell_id) && GetAA(aaElementalPact))
		return 100;

	if(type == focusReagentCost && (IsEffectInSpell(spell_id, SE_SummonItem) || IsSacrificeSpell(spell_id)))
		return 0;
	//Summon Spells that require reagents are typically imbue type spells, enchant metal, sacrifice and shouldn't be affected
	//by reagent conservation for obvious reasons.

	return realTotal + realTotal2 + realTotal3;
}

void Mob::CheckNumHitsRemaining(uint8 type, uint32 buff_slot, uint16 spell_id)
{
	/*
	Field 175 = numhits type
	1:  [Incoming Hit Attempts] (323=SE_DefensiveProc, 172=SE_AvoidMeleeChance, 1=SE_ArmorClass, 40=SE_DivineAura)
	2:  [Outgoing Hit Attempts]  (185=SE_DamageModifer, 184=SE_HitChance)
	3:  [Incoming Spells]  (180=SE_ResistSpellChance, 296=SE_FcSpellVulnerability) //Note: Determinetal spells only unless proven otherwise
	4:  NONE
	5:  [Outgoing Hit Successes] (220=SE_SkillDamageAmount, 178=SE_MeleeLifetap, 121=SE_ReverseDS, ?373=SE_CastOnWearoff)
	6:  [Incoming Hit Successes] (59=SE_DamageShield, 197=SE_SkillDamageTaken, 162=define SE_MitigateMeleeDamage)
	7:  [Matching Spells] *When focus is triggered (focus effects)
	8:  [Incoming Hits or Spells] (329=SE_ManaAbsorbPercentDamage)
	9:  [Reflected Spells] If successful
	10: [Defensive Procs] Only count down buff of the proc that executed
	11: [Melee Procs] Only count down the buff of the proc that executed
	*/

	if (!HasNumhits())
		return;

	bool bDepleted = false;
	uint32 buff_max = GetMaxTotalSlots();

	//Spell specific procs [Type 7,10,11]
	if (IsValidSpell(spell_id)){
		
		for(uint32 d = 0; d < buff_max; d++) {
		
			if((buffs[d].spellid == spell_id) && (buffs[d].numhits > 0) && (spells[buffs[d].spellid].numhitstype == type)){

				if(--buffs[d].numhits == 0) {
					CastOnNumHitFade(buffs[d].spellid);
					if(!TryFadeEffect(d))
						BuffFadeBySlot(d, true);
				}
			}
		}
	}

	else if (type == 7){
		if (buff_slot > 0){

			if(--buffs[buff_slot].numhits == 0) {
				CastOnNumHitFade(buffs[buff_slot].spellid);
				if(!TryFadeEffect(buff_slot))
					BuffFadeBySlot(buff_slot , true);
				}
			}

		else {
	
			for(int d = 0; d < buff_max; d++) {
			
				if(!m_spellHitsLeft[d])
					continue;

				if ((IsValidSpell(buffs[d].spellid)) && (m_spellHitsLeft[d] == buffs[d].spellid)) {
					if(--buffs[d].numhits == 0) {
						CastOnNumHitFade(buffs[d].spellid);
						m_spellHitsLeft[d] = 0;
						if(!TryFadeEffect(d))
							BuffFadeBySlot(d, true);
					}
				}
			}
		}
	}


	else{

		for(uint32 d = 0; d < buff_max; d++) {
		
			if((IsValidSpell(buffs[d].spellid)) && (buffs[d].numhits > 0) && (spells[buffs[d].spellid].numhitstype == type)){
			
				if(--buffs[d].numhits == 0) {
					CastOnNumHitFade(buffs[d].spellid);
					if(!TryFadeEffect(d)){
						BuffFadeBySlot(d, true);
					}
				}
			
			}
		}
	}
}

//for some stupid reason SK procs return theirs one base off...
uint16 Mob::GetProcID(uint16 spell_id, uint8 effect_index) {
	bool sk = false;
	bool other = false;
	for(int x = 0; x < 16; x++)
	{
		if(x == 4){
			if(spells[spell_id].classes[4] < 255)
				sk = true;
		}
		else{
			if(spells[spell_id].classes[x] < 255)
				other = true;
		}
	}

	if(sk && !other)
	{
		return(spells[spell_id].base[effect_index] + 1);
	}
	else{
		return(spells[spell_id].base[effect_index]);
	}
}

bool Mob::TryDivineSave()
{
	/*
	How Touch of the Divine AA works:
	-Gives chance to avoid death when client is killed.
	-Chance is determined by the sum of AA/item/spell chances.
	-If the chance is met a divine aura like effect 'Touch of the Divine' is applied to the client removing detrimental spell effects.
	-If desired, additional spells can be triggered from the AA/item/spell effect, generally a heal.
	*/

	int32 SuccessChance = aabonuses.DivineSaveChance[0] + itembonuses.DivineSaveChance[0] + spellbonuses.DivineSaveChance[0];
	if (SuccessChance && MakeRandomInt(0, 100) <= SuccessChance)
	{
		SetHP(1);

		int16 EffectsToTry[] =
		{
			aabonuses.DivineSaveChance[1],
			itembonuses.DivineSaveChance[1],
			spellbonuses.DivineSaveChance[1]
		};
		//Fade the divine save effect here after saving the old effects off.
		//That way, if desired, the effect could apply SE_DivineSave again.
		BuffFadeByEffect(SE_DivineSave);
		for(size_t i = 0; i < ( sizeof(EffectsToTry) / sizeof(EffectsToTry[0]) ); ++i)
		{
			if( EffectsToTry[i] )
			{
				SpellOnTarget(EffectsToTry[i], this);
			}
		}

		SpellOnTarget(4789, this); //Touch of the Divine=4789, an Invulnerability/HoT/Purify effect
		SendHPUpdate();
		return true;
	}
	return false;
}

bool Mob::TryDeathSave() {

	/*
	How Death Save works:
	-Chance for Death Save to fire is the same for Death Pact/Divine Intervention
	-Base value of these determines amount healed (1=partial(300HP), 2='full (8000HP)) HARD CODED
	-Charisma of client who has the effect determines fire rate, parses show this clearly with ratio used.
	-Unfailing Divinity AA - Allows for a chance to give a heal for a percentage of the orginal value
	 when your innate chance fails and removes the buff. The spell effect for Unfailing Divinity gives
	 the a value of a heal modifier of the base effects heal.
	 Ie. Divine Intervention is 8000 HP Max UD1=20, therefore heal is 8000*20/100
	-No evidence of chance rate increasing between UD1-3, numbers indicate it uses same CHA rate as first DI.
	-In later expansions this SE_DeathSave was given a level limit and a heal value in its effect data.
	*/

	if (spellbonuses.DeathSave[0]){

		int SuccessChance = 0;
		int buffSlot = spellbonuses.DeathSave[1];
		int16 UD_HealMod = 0;
		uint32 HealAmt = 300; //Death Pact max Heal

		if(buffSlot >= 0){

			UD_HealMod = buffs[buffSlot].ExtraDIChance;

			SuccessChance = ( (GetCHA() * (RuleI(Spells, DeathSaveCharismaMod))) + 1) / 10; //(CHA Mod Default = 3)

			if (SuccessChance > 95)
				SuccessChance = 95;

			if(SuccessChance >= MakeRandomInt(0, 100)) {

				if(spellbonuses.DeathSave[0] == 2)
					HealAmt = RuleI(Spells, DivineInterventionHeal); //8000HP is how much LIVE Divine Intervention max heals

				//Check if bonus Heal amount can be applied ([3] Bonus Heal [2] Level limit)
				if (spellbonuses.DeathSave[3] && (GetLevel() >= spellbonuses.DeathSave[2]))
					HealAmt += spellbonuses.DeathSave[3];

				if ((GetMaxHP() - GetHP()) < HealAmt)
					HealAmt = GetMaxHP() - GetHP();

				SetHP((GetHP()+HealAmt));
				Message(263, "The gods have healed you for %i points of damage.", HealAmt);

				if(spellbonuses.DeathSave[0] == 2)
					entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, DIVINE_INTERVENTION, GetCleanName());
				else
					entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, DEATH_PACT, GetCleanName());

				SendHPUpdate();
				BuffFadeBySlot(buffSlot);
				return true;
			}
			else if (UD_HealMod) {

				SuccessChance = ((GetCHA() * (RuleI(Spells, DeathSaveCharismaMod))) + 1) / 10;

				if (SuccessChance > 95)
					SuccessChance = 95;

				if(SuccessChance >= MakeRandomInt(0, 100)) {

					if(spellbonuses.DeathSave[0] == 2)
						HealAmt = RuleI(Spells, DivineInterventionHeal);

					//Check if bonus Heal amount can be applied ([3] Bonus Heal [2] Level limit)
					if (spellbonuses.DeathSave[3] && (GetLevel() >= spellbonuses.DeathSave[2]))
						HealAmt += spellbonuses.DeathSave[3];

					HealAmt = HealAmt*UD_HealMod/100;

					if ((GetMaxHP() - GetHP()) < HealAmt)
						HealAmt = GetMaxHP() - GetHP();

					SetHP((GetHP()+HealAmt));
					Message(263, "The gods have healed you for %i points of damage.", HealAmt);

					if(spellbonuses.DeathSave[0] == 2)
						entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, DIVINE_INTERVENTION, GetCleanName());
					else
						entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, DEATH_PACT, GetCleanName());

					SendHPUpdate();
					BuffFadeBySlot(buffSlot);
					return true;
				}
			}
		}

		BuffFadeBySlot(buffSlot);
	}
	return false;
}

bool Mob::AffectedBySpellExcludingSlot(int slot, int effect)
{
	for (int i = 0; i <= EFFECT_COUNT; i++)
	{
		if (i == slot)
			continue;

		if (IsEffectInSpell(buffs[i].spellid, effect))
			return true;
	}
	return false;
}

float Mob::GetSympatheticProcChances(float &ProcBonus, float &ProcChance, int32 cast_time, int16 ProcRateMod) {

	ProcChance = 0;

	if(cast_time > 0)
	{
		ProcChance = ((float)cast_time * RuleR(Casting, AvgSpellProcsPerMinute) / 60000.0f);
		ProcChance = ProcChance * (float)(ProcRateMod/100);
	}
	return ProcChance;
}

bool Mob::DoHPToManaCovert(uint16 mana_cost)
{
	if (spellbonuses.HPToManaConvert){
		int hp_cost = spellbonuses.HPToManaConvert * mana_cost / 100;
		if(hp_cost) {
			SetHP(GetHP()-hp_cost);
			return true;
		}
	return false;
	}

	return false;
}

int32 Mob::GetFcDamageAmtIncoming(Mob *caster, uint32 spell_id, bool use_skill, uint16 skill )
{
	//Used to check focus derived from SE_FcDamageAmtIncoming which adds direct damage to Spells or Skill based attacks.
	int32 dmg = 0;
	bool limit_exists = false;
	bool skill_found = false;

	if (!caster)
		return 0;

	if (spellbonuses.FocusEffects[focusFcDamageAmtIncoming]){
		uint32 buff_count = GetMaxTotalSlots();
		for(int i = 0; i < buff_count; i++){

			if( (IsValidSpell(buffs[i].spellid) && (IsEffectInSpell(buffs[i].spellid, SE_FcDamageAmtIncoming))) ){

				if (use_skill){
					int32 temp_dmg = 0;
					for (int e = 0; e < EFFECT_COUNT; e++) {

						if (spells[buffs[i].spellid].effectid[e] == SE_FcDamageAmtIncoming){
							temp_dmg += spells[buffs[i].spellid].base[e];
							continue;
						}

						if (!skill_found){
							if ((spells[buffs[i].spellid].effectid[e] == SE_LimitToSkill) ||
								(spells[buffs[i].spellid].effectid[e] == SE_LimitCastingSkill)){
								limit_exists = true;

								if (spells[buffs[i].spellid].base[e] == skill)
									skill_found = true;
							}
						}
					}
					if ((!limit_exists) || (limit_exists && skill_found)){
						dmg += temp_dmg;
						CheckNumHitsRemaining(7,i);
					}
				}

				else{
					int32 focus = caster->CalcFocusEffect(focusFcDamageAmtIncoming, buffs[i].spellid, spell_id);
					if(focus){
						dmg += focus;
						CheckNumHitsRemaining(7,i);
					}
				}
			}
		}
	}
	return dmg;
}

int32 Mob::GetFocusIncoming(focusType type, int effect, Mob *caster, uint32 spell_id) {

	/*
	This is a general function for calculating best focus effect values for focus effects that exist on targets but modify incoming spells.
	Should be used when checking for foci that can exist on clients or npcs ect.
	Example: When your target has a focus limited buff that increases amount of healing on them.
	*/

	if (!caster)
		return 0;

	int value = 0;

	if (spellbonuses.FocusEffects[type]){
		uint32 buff_count = GetMaxTotalSlots();
		for(int i = 0; i < buff_count; i++){

			int32 tmp_focus = 0;
			int tmp_buffslot = -1;

			int buff_count = GetMaxTotalSlots();
			for(int i = 0; i < buff_count; i++) {

				if((IsValidSpell(buffs[i].spellid) && IsEffectInSpell(buffs[i].spellid, effect))){

					int32 focus = caster->CalcFocusEffect(type, buffs[i].spellid, spell_id);

					if (!focus)
						continue;

					if (tmp_focus && focus > tmp_focus){
						tmp_focus = focus;
						tmp_buffslot = i;
					}

					else if (!tmp_focus){
						tmp_focus = focus;
						tmp_buffslot = i;
					}
				}
			}

			value = tmp_focus;

			if (tmp_buffslot >= 0)
				CheckNumHitsRemaining(7, tmp_buffslot);
		}

	}
	return value;
}

int32 Mob::ApplySpellEffectiveness(Mob* caster, int16 spell_id, int32 value, bool IsBard) {

	// 9-17-12: This is likely causing crashes, disabled till can resolve.
	if (IsBard)
		return value;

	if (!caster)
		return value;

	if (caster->IsClient()){
		int16 focus = caster->CastToClient()->GetFocusEffect(focusFcBaseEffects, spell_id);

			if (IsBard)
				value += focus;

			else
				value += value*focus/100;
	}
	return value;
}

bool Mob::PassLimitClass(uint32 Classes_, uint16 Class_)
{
	//The class value for SE_LimitClass is +1 to its equivelent value in item dbase
	//Example Bard on items is '128' while Bard on SE_LimitClass is '256', keep this in mind if making custom spells.
	if (Class_ > 16)
		return false;

	Class_ += 1;

	for (int CurrentClass = 1; CurrentClass <= PLAYER_CLASS_COUNT; ++CurrentClass){
		if (Classes_ % 2 == 1){
			if (CurrentClass == Class_)
				return true;
		}
		Classes_ >>= 1;
	}
	return false;
}

uint16 Mob::GetSpellEffectResistChance(uint16 spell_id)
{

	if(!IsValidSpell(spell_id))
		return 0;

	if (!aabonuses.SEResist[0] && !spellbonuses.SEResist[0] && !itembonuses.SEResist[0])
		return 0;

	uint16 resist_chance = 0;

	for(int i = 0; i < EFFECT_COUNT; ++i)
	{
		bool found = false;

		for(int d = 0; d < MAX_RESISTABLE_EFFECTS*2; d+=2)
		{
			if (spells[spell_id].effectid[i] == aabonuses.SEResist[d]){
				resist_chance += aabonuses.SEResist[d+1];
				found = true;
			}

			if (spells[spell_id].effectid[i] == itembonuses.SEResist[d]){
				resist_chance += itembonuses.SEResist[d+1];
				found = true;
			}


			if (spells[spell_id].effectid[i] == spellbonuses.SEResist[d]){
				resist_chance += spellbonuses.SEResist[d+1];
				found = true;
			}

			if (found)
				continue;
		}
	}
	return resist_chance;
}

bool Mob::TryDispel(uint8 caster_level, uint8 buff_level, int level_modifier){

	//Dispels - Check level of caster agianst buffs level (level of the caster who cast the buff)
	//Effect value of dispels are treated as a level modifier.
	//Values for scaling were obtain from live parses, best estimates.

	caster_level += level_modifier - 1; 
	int dispel_chance = 36; //Baseline chance if no level difference and no modifier
	int level_diff = caster_level - buff_level;

	if (level_diff > 0)
		dispel_chance += level_diff * 8;

	else if (level_diff < 0)
		dispel_chance += level_diff * 2;

	if (dispel_chance >= 100)
		return true;

	else if (dispel_chance < 10)
		dispel_chance = 10;

	if (MakeRandomInt(0,99) < dispel_chance)
		return true;
	else
		return false;
}


bool Mob::ImprovedTaunt(){

	if (spellbonuses.ImprovedTaunt[0]){

		if (GetLevel() > spellbonuses.ImprovedTaunt[0])
			return false;

		if (spellbonuses.ImprovedTaunt[2] >= 0){

			target = entity_list.GetMob(buffs[spellbonuses.ImprovedTaunt[2]].casterid);

			if (target){
				SetTarget(target);
				return true;
			}
			else {
				if(!TryFadeEffect(spellbonuses.ImprovedTaunt[2]))
					BuffFadeBySlot(spellbonuses.ImprovedTaunt[2], true); //If caster killed removed effect.
			}	
		}
	}

	return false;
}


bool Mob::PassCastRestriction(bool UseCastRestriction,  int16 value, bool IsDamage)
{
	/*If return TRUE spell met all restrictions and can continue (this = target).
	This check is used when the spell_new field CastRestriction is defined OR spell effect '0'(DD/Heal) has a defined limit
	Range 1			: UNKNOWN
	Range 100		: *Animal OR Humanoid
	Range 101		: *Dragon
	Range 102		: *Animal OR Insect
	Range 103		: NOT USED
	Range 104		: *Animal
	Range 105		: Plant
	Range 106		: *Giant
	Range 107		: NOT USED
	Range 108		: NOT Animal or Humaniod
	Range 109		: *Bixie
	Range 111		: *Harpy
	Range 112		: *Sporali
	Range 113		: *Kobold
	Range 114		: *Shade Giant
	Range 115		: *Drakkin
	Range 116		: NOT USED
	Range 117		: *Animal OR Plant
	Range 118		: *Summoned
	Range 119		: *Firepet
	Range 120		: Undead
	Range 121		: *Living (NOT Undead)
	Range 122		: *Fairy
	Range 123		: Humanoid
	Range 124		: *Undead HP < 10%
	Range 125		: *Clockwork HP < 10%
	Range 126		: *Wisp HP < 10%
	Range 127-130	: UNKNOWN
	Range 150		: UNKNOWN
	Range 190		: No Raid boss flag *not implemented
	Range 191		: This spell will deal less damage to 'exceptionally strong targets' - Raid boss flag *not implemented
	Range 201		: Damage if HP > 75%
	Range 203		: Damage if HP < 20%
	Range 216		: TARGET NOT IN COMBAT
	Range 221 - 249	: Causing damage dependent on how many pets/swarmpets are attacking your target.
	Range 250		: Damage if HP < 35%
	Range 300 - 303	: UNKOWN *not implemented
	Range 304		: Chain + Plate class (buffs)
	Range 399 - 409	: Heal if HP within a specified range (400 = 0-25% 401 = 25 - 35% 402 = 35-45% ect)
	Range 410 - 411 : UNKOWN
	Range 500 - 599	: Heal if HP less than a specified value
	Range 600 - 699	: Limit to Body Type [base2 - 600 = Body]
	Range 700		: UNKNOWN
	Range 701		: NOT PET
	Range 800		: UKNOWN
	Range 818 - 819 : If Undead/If Not Undead
	Range 820 - 822	: UKNOWN
	Range 835 		: Unknown *not implemented
	Range 836 -	837	: Progression Server / Live Server *not implemented
	Range 839 		: Unknown *not implemented
	Range 842 - 844 : Humaniod lv MAX ((842 - 800) * 2)
	Range 845 - 847	: UNKNOWN
	Range 10000 - 11000	: Limit to Race [base2 - 10000 = Race] (*Not on live: Too useful a function to not implement)
	THIS IS A WORK IN PROGRESS
	*/ 

	if (value <= 0)
		return true;

	if (IsDamage || UseCastRestriction) {

		switch(value)
		{
			case 100:	
				if ((GetBodyType() == BT_Animal) || (GetBodyType() == BT_Humanoid))
					return true;
				break;

			case 101:	
				if (GetBodyType() == BT_Dragon || GetBodyType() == BT_VeliousDragon || GetBodyType() == BT_Dragon3)
					return true;
				break;

			case 102:	
				if ((GetBodyType() == BT_Animal) || (GetBodyType() == BT_Insect))
					return true;
				break;

			case 104:	
				if (GetBodyType() == BT_Animal)
					return true;
				break;

			case 105:	
				if (GetBodyType() == BT_Plant)
					return true;
				break;

			case 106:	
				if (GetBodyType() == BT_Giant)
					return true;
				break;

			case 108:	
				if ((GetBodyType() != BT_Animal) || (GetBodyType() != BT_Humanoid))
					return true;
				break;

			case 109:	
				if ((GetRace() == 520) ||(GetRace() == 79))
					return true;
				break;

			case 111:	
				if ((GetRace() == 527) ||(GetRace() == 11))
					return true;
				break;

			case 112:	
				if ((GetRace() == 456) ||(GetRace() == 28))
					return true;
				break;

			case 113:	
				if ((GetRace() == 456) ||(GetRace() == 48))
					return true;
				break;

			case 114:	
				if (GetRace() == 526)
					return true;
				break;

			case 115:	
				if (GetRace() == 522)
					return true;
				break;

			case 117:	
				if ((GetBodyType() == BT_Animal) || (GetBodyType() == BT_Plant))
					return true;
				break;

			case 118:	
				if (GetBodyType() == BT_Summoned)
					return true;
				break;

			case 119:	
				if (IsPet() && ((GetRace() == 212) || ((GetRace() == 75) && GetTexture() == 1)))
					return true;
				break;

			case 120:	
				if (GetBodyType() == BT_Undead)
					return true;
				break;

			case 121:	
				if (GetBodyType() != BT_Undead)
					return true;
				break;

			case 122:	
				if ((GetRace() == 473) || (GetRace() == 425))
					return  true;
				break;

			case 123:	
				if (GetBodyType() == BT_Humanoid)
					return true;
				break;

			case 124:	
				if ((GetBodyType() == BT_Undead) && (GetHPRatio() < 10))
					return true;
				break;

			case 125:	
				if ((GetRace() == 457 || GetRace() == 88) && (GetHPRatio() < 10))
					return true;
				break;

			case 126:	
				if ((GetRace() == 581 || GetRace() == 69) && (GetHPRatio() < 10))
					return true;
				break;

			case 201:	
				if (GetHPRatio() > 75)
					return true;
				break;

			case 204:	
				if (GetHPRatio() < 20)
					return true;
				break;

			case 216:	
				if (!IsEngaged())
					return true;
				break;

			case 250:	
				if (GetHPRatio() < 35)
					return true;
				break;

			case 304:	
				if (IsClient() && 
					((GetClass() == WARRIOR) || (GetClass() == BARD)  || (GetClass() == SHADOWKNIGHT)  || (GetClass() == PALADIN)  || (GetClass() == CLERIC)
					 || (GetClass() == RANGER) || (GetClass() == SHAMAN) || (GetClass() == ROGUE)  || (GetClass() == BERSERKER)))
					return true;
				break;

			case 701:	
				if (!IsPet())
					return true;
				break;

			case 818:	
				if (GetBodyType() == BT_Undead)
					return true;
				break;

			case 819:	
				if (GetBodyType() != BT_Undead)
					return true;
				break;

			case 842:	
				if (GetBodyType() == BT_Humanoid && GetLevel() <= 84)
					return true;
				break;

			case 843:	
				if (GetBodyType() == BT_Humanoid && GetLevel() <= 86)
					return true;
				break;

			case 844:	
				if (GetBodyType() == BT_Humanoid && GetLevel() <= 88)
					return true;
				break;
		}

		//Limit to amount of pets
		if (value >= 221 && value <= 249){
			int count = hate_list.SummonedPetCount(this);
	
			for (int base2_value = 221; base2_value <= 249; ++base2_value){
				if (value == base2_value){
					if (count >= (base2_value - 220)){
						return true;
					}
				}
			}
		}

		//Limit to Body Type
		if (value >= 600 &&  value <= 699){
			if (GetBodyType() == (value - 600))
				return true;
		}

		//Limit to Race. *Not implemented on live
		if (value >= 10000 && value <= 11000){
			if (GetRace() == (value - 10000))
				return true;
		}
	} //End Damage

	if (!IsDamage || UseCastRestriction) {
	
		//Heal only if HP within specified range. [Doesn't follow a set forumla for all values...]
		if (value >= 400 && value <= 408){
			for (int base2_value = 400; base2_value <= 408; ++base2_value){
				if (value == base2_value){
																		
					if (value == 400 && GetHPRatio() <= 25)
							return true;

					else if (value == base2_value){
						if (GetHPRatio() > 25+((base2_value - 401)*10) && GetHPRatio() <= 35+((base2_value - 401)*10))
							return true;
					}
				}
			}
		}
						
		else if (value >= 500 && value <= 549){
			for (int base2_value = 500; base2_value <= 520; ++base2_value){
				if (value == base2_value){
					if (GetHPRatio() < (base2_value - 500)*5) 
						return true;
				}
			}
		}

		else if (value == 399) {
			if (GetHPRatio() > 15 && GetHPRatio() <= 25)
				return true;
		}
	} // End Heal
			
						
	return false;
}

