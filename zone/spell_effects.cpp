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

#include "../common/eqemu_logsys.h"
#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/global_define.h"
#include "../common/item_instance.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/data_verification.h"
#include "../common/misc_functions.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"

#include <math.h>

#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif


extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;


// the spell can still fail here, if the buff can't stack
// in this case false will be returned, true otherwise
bool Mob::SpellEffect(Mob* caster, uint16 spell_id, float partial, int level_override)
{
	int caster_level, buffslot, effect, effect_value, i;
	EQEmu::ItemInstance *SummonedItem=nullptr;
#ifdef SPELL_EFFECT_SPAM
#define _EDLEN	200
	char effect_desc[_EDLEN];
#endif

	if(!IsValidSpell(spell_id))
		return false;

	const SPDat_Spell_Struct &spell = spells[spell_id];

	if (spell.disallow_sit && IsBuffSpell(spell_id) && IsClient() && (CastToClient()->IsSitting() || CastToClient()->GetHorseId() != 0))
		return false;

	bool c_override = false;
	if (caster && caster->IsClient() && GetCastedSpellInvSlot() > 0) {
		const EQEmu::ItemInstance *inst = caster->CastToClient()->GetInv().GetItem(GetCastedSpellInvSlot());
		if (inst) {
			if (inst->GetItem()->Click.Level > 0) {
				caster_level = inst->GetItem()->Click.Level;
				c_override = true;
			} else {
				caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
			}
		} else if (level_override > 0) {
			caster_level = level_override;
			c_override = true;
		} else {
			caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
		}
	} else if (level_override > 0) {
		caster_level = level_override;
		c_override = true;
	} else {
		caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
	}

	if(c_override)
	{
		int durat = CalcBuffDuration(caster, this, spell_id, caster_level);
		if(durat) // negatives are perma buffs
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

		if (spells[spell_id].numhits > 0) {

			int numhit = spells[spell_id].numhits;

			numhit += numhit * caster->GetFocusEffect(focusFcLimitUse, spell_id) / 100;
			numhit += caster->GetFocusEffect(focusIncreaseNumHits, spell_id);
			buffs[buffslot].numhits = numhit;
		}

		if (spells[spell_id].EndurUpkeep > 0)
			SetEndurUpkeep(true);

		if (IsClient() && CastToClient()->ClientVersionBit() & EQEmu::versions::bit_UFAndLater)
		{
			EQApplicationPacket *outapp = MakeBuffsPacket(false);
			CastToClient()->FastQueuePacket(&outapp);
		}
	}

	if(IsNPC())
	{
		std::vector<EQEmu::Any> args;
		args.push_back(&buffslot);
		int i = parse->EventSpell(EVENT_SPELL_EFFECT_NPC, CastToNPC(), nullptr, spell_id, caster ? caster->GetID() : 0, &args);
		if(i != 0){
			CalcBonuses();
			return true;
		}
	}
	else if(IsClient())
	{
		std::vector<EQEmu::Any> args;
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


	if (!IsPowerDistModSpell(spell_id))
		SetSpellPowerDistanceMod(0);

	bool SE_SpellTrigger_HasCast = false;

	// if buff slot, use instrument mod there, otherwise calc it
	uint32 instrument_mod = buffslot > -1 ? buffs[buffslot].instrument_mod : caster ? caster->GetInstrumentMod(spell_id) : 10;
	// iterate through the effects in the spell
	for (i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spell_id, i))
			continue;

		effect = spell.effectid[i];
		effect_value = CalcSpellEffectValue(spell_id, i, caster_level, instrument_mod, caster ? caster : this);

		if(spell_id == SPELL_LAY_ON_HANDS && caster && caster->GetAA(aaImprovedLayOnHands))
			effect_value = GetMaxHP();

		if (GetSpellPowerDistanceMod())
			effect_value = effect_value*GetSpellPowerDistanceMod()/100;

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
					if(caster) {
						dmg = caster->GetActSpellDamage(spell_id, dmg, this);
						caster->ResourceTap(-dmg, spell_id);
					}

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
					dmg = caster->GetHP(); // just your current HP
					caster->SetHP(dmg / 4); // 2003 patch notes say ~ 1/4 HP. Should this be 1/4 your current HP or do 3/4 max HP dmg? Can it kill you?
					dmg = -dmg;
				}

				//do any AAs apply to these spells?
				if(dmg < 0) {
					if (!PassCastRestriction(false, spells[spell_id].base2[i], true))
						break;
					dmg = -dmg;
					Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
				} else {
					if (!PassCastRestriction(false, spells[spell_id].base2[i], false))
						break;
					HealDamage(dmg, caster);
				}
				break;
			}

			case SE_PercentalHeal:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Percental Heal: %+i (%d%% max)", spell.max[i], effect_value);
#endif
				int32 val = GetMaxHP() * spell.base[i] / 100;

				//This effect can also do damage by percent.
				if (val < 0) {

					if (spell.max[i] && -val > spell.max[i])
						val = -spell.max[i];

					if (caster)
						val = caster->GetActSpellDamage(spell_id, val, this);

				}

				else
				{
					if (spell.max[i] && val > spell.max[i])
						val = spell.max[i];

					if(caster)
						val = caster->GetActSpellHealing(spell_id, val, this);
				}

				if (val < 0)
					Damage(caster, -val, spell_id, spell.skill, false, buffslot, false);
				else
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
				int buff_count = GetMaxTotalSlots();
				for(i = 0; i < buff_count; i++) {
					if(buffs[i].spellid == spell_id && i != buffslot) {
						Message(0, "You must wait before you can be affected by this spell again.");
						inuse = true;
						break;
					}
				}
				if(inuse)
					break;

				int32 val = 0;
				val = 7500 * effect_value;
				if (caster)
					val = caster->GetActSpellHealing(spell_id, val, this);

				if (val > 0)
					HealDamage(val, caster);

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
						if (caster)
							caster->SetMana(caster->GetMana() + std::abs(effect_value));

						if (effect_value < 0)
							TryTriggerOnValueAmount(false, true);
#ifdef SPELL_EFFECT_SPAM
						if (caster)
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
				const char *target_zone = nullptr;

				x = static_cast<float>(spell.base[1]);
				y = static_cast<float>(spell.base[0]);
				z = static_cast<float>(spell.base[2]);
				heading = static_cast<float>(spell.base[3]);

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
					if(zone->random.Roll(RuleI(Spells, SuccorFailChance))) { //2% Fail chance by default

						if(IsClient()) {
							CastToClient()->Message_StringID(MT_SpellFailure,SUCCOR_FAIL);
						}
						break;
					}

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
						Log(Logs::General, Logs::None, "Succor/Evacuation Spell In Same Zone.");
#endif
							if(IsClient())
								CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading, 0, EvacToSafeCoords);
							else
								GMMove(x, y, z, heading);
					}
					else {
#ifdef SPELL_EFFECT_SPAM
						Log(Logs::General, Logs::None, "Succor/Evacuation Spell To Another Zone.");
#endif
						if(IsClient())
							CastToClient()->MovePC(target_zone, x, y, z, heading);
					}
				}

				break;
			}
			case SE_GateCastersBindpoint: // Used on Teleport Bind.
			case SE_Teleport:	// gates, rings, circles, etc
			case SE_Teleport2:
			{
				float x, y, z, heading;
				const char *target_zone = nullptr;

				x = static_cast<float>(spell.base[1]);
				y = static_cast<float>(spell.base[0]);
				z = static_cast<float>(spell.base[2]);
				heading = static_cast<float>(spell.base[3]);

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

				if (effect == SE_GateCastersBindpoint && caster && caster->IsClient())
				{ // Teleport Bind uses caster's bind point
					int index = spells[spell_id].base[i] - 1;
					if (index < 0 || index > 4)
						index = 0;
					x = caster->CastToClient()->GetBindX(index);
					y = caster->CastToClient()->GetBindY(index);
					z = caster->CastToClient()->GetBindZ(index);
					heading = caster->CastToClient()->GetBindHeading(index);
					//target_zone = caster->CastToClient()->GetBindZoneId(); target_zone doesn't work due to const char
					CastToClient()->MovePC(caster->CastToClient()->GetBindZoneID(index), 0, x, y, z, heading);
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
					EQEmu::ItemInstance* transI = CastToClient()->GetInv().GetItem(EQEmu::inventory::slotCursor);
					if (transI && transI->IsClassCommon() && transI->IsStackable()){
						uint32 fcharges = transI->GetCharges();
							//Does it sound like meat... maybe should check if it looks like meat too...
							if(strstr(transI->GetItem()->Name, "meat") ||
								strstr(transI->GetItem()->Name, "Meat") ||
								strstr(transI->GetItem()->Name, "flesh") ||
								strstr(transI->GetItem()->Name, "Flesh") ||
								strstr(transI->GetItem()->Name, "parts") ||
								strstr(transI->GetItem()->Name, "Parts")){
								CastToClient()->DeleteItemInInventory(EQEmu::inventory::slotCursor, fcharges, true);
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
				// Is there a message generated? Too disgusted by raids.
				uint32 time = spell.base[i] * 10 * 1000;
				if (caster && caster->IsClient()) {
					if (caster->IsGrouped()) {
						auto group = caster->GetGroup();
						for (int i = 0; i < 6; ++i)
							if (group->members[i])
								group->members[i]->aa_timers[aaTimerWarcry].Start(time);
					} else if (caster->IsRaidGrouped()) {
						auto raid = caster->GetRaid();
						uint32 gid = raid->GetGroup(caster->CastToClient());
						if (gid < 12)
							for (int i = 0; i < MAX_RAID_MEMBERS; ++i)
								if (raid->members[i].member && raid->members[i].GroupNumber == gid)
									raid->members[i].member->aa_timers[aaTimerWarcry].Start(time);
					}
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
					if (caster)
						caster->Message_StringID(MT_SpellFailure, IMMUNE_STUN);
				} else {
					int stun_resist = itembonuses.StunResist+spellbonuses.StunResist;
					if (IsClient())
						stun_resist += aabonuses.StunResist;

					if (stun_resist <= 0 || zone->random.Int(0,99) >= stun_resist) {
						Log(Logs::Detail, Logs::Combat, "Stunned. We had %d percent resist chance.", stun_resist);

						if (caster && caster->IsClient())
							effect_value += effect_value*caster->GetFocusEffect(focusFcStunTimeMod, spell_id)/100;

						Stun(effect_value);
					} else {
						if (IsClient())
							Message_StringID(MT_Stun, SHAKE_OFF_STUN);

						Log(Logs::Detail, Logs::Combat, "Stun Resisted. We had %d percent resist chance.", stun_resist);
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
				SetPetType(petCharmed);

				if(caster->IsClient()){
					auto app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
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
					CastToClient()->AI_Start();
				} else if(IsNPC()) {
					CastToNPC()->SetPetSpellID(0);	//not a pet spell.
				}

				bool bBreak = false;

				// define spells with fixed duration
				// charm spells with -1 in field 209 are all of fixed duration, so lets use that instead of spell_ids
				if(spells[spell_id].no_resist)
					bBreak = true;

				if (!bBreak)
				{
					int resistMod = static_cast<int>(partial) + (GetCHA()/25);
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

					if (CastToClient()->ClientVersionBit() & EQEmu::versions::bit_SoDAndLater)
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
							SendPositionUpdate(2);
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
				if(IsClient())
				{
					if(buffs[buffslot].ticsremaining > RuleI(Character, MaxFearDurationForPlayerCharacter))
						buffs[buffslot].ticsremaining = RuleI(Character, MaxFearDurationForPlayerCharacter);
				}


				if(RuleB(Combat, EnableFearPathing)){
					if(IsClient())
					{
						CastToClient()->AI_Start();
					}

					CalculateNewFearpoint();
					if(currently_fleeing)
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

			case SE_BindAffinity: //TO DO: Add support for secondary and tertiary gate abilities
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Bind Affinity");
#endif
				if (IsClient())
				{
					if(CastToClient()->GetGM() || RuleB(Character, BindAnywhere))
					{
						auto action_packet =
						    new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
						Action_Struct* action = (Action_Struct*) action_packet->pBuffer;
						auto message_packet =
						    new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
						CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;

						action->target = GetID();
						action->source = caster ? caster->GetID() : GetID();
						action->level = 65;
						action->instrument_mod = 10;
						action->sequence = static_cast<uint32>((GetHeading() * 12345 / 2));
						action->type = 231;
						action->spell = spell_id;
						action->buff_unknown = 4;

						cd->target = action->target;
						cd->source = action->source;
						cd->type = action->type;
						cd->spellid = action->spell;
						cd->meleepush_xy = action->sequence;

						CastToClient()->QueuePacket(action_packet);
						if(caster && caster->IsClient() && caster != this)
							caster->CastToClient()->QueuePacket(action_packet);

						CastToClient()->QueuePacket(message_packet);
						if(caster && caster->IsClient() && caster != this)
							caster->CastToClient()->QueuePacket(message_packet);

						CastToClient()->SetBindPoint(spells[spell_id].base[i] - 1);
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
								auto action_packet = new EQApplicationPacket(
								    OP_Action, sizeof(Action_Struct));
								Action_Struct* action = (Action_Struct*) action_packet->pBuffer;
								auto message_packet = new EQApplicationPacket(
								    OP_Damage, sizeof(CombatDamage_Struct));
								CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;

								action->target = GetID();
								action->source = caster ? caster->GetID() : GetID();
								action->level = 65;
								action->instrument_mod = 10;
								action->sequence = static_cast<uint32>((GetHeading() * 12345 / 2));
								action->type = 231;
								action->spell = spell_id;
								action->buff_unknown = 4;

								cd->target = action->target;
								cd->source = action->source;
								cd->type = action->type;
								cd->spellid = action->spell;
								cd->meleepush_xy = action->sequence;

								CastToClient()->QueuePacket(action_packet);
								if(caster->IsClient() && caster != this)
									caster->CastToClient()->QueuePacket(action_packet);

								CastToClient()->QueuePacket(message_packet);
								if(caster->IsClient() && caster != this)
									caster->CastToClient()->QueuePacket(message_packet);

								CastToClient()->SetBindPoint(spells[spell_id].base[i] - 1);
								Save();
								safe_delete(action_packet);
								safe_delete(message_packet);
							}
						}
						else
						{
							auto action_packet =
							    new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
							Action_Struct* action = (Action_Struct*) action_packet->pBuffer;
							auto message_packet = new EQApplicationPacket(
							    OP_Damage, sizeof(CombatDamage_Struct));
							CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;

							action->target = GetID();
							action->source = caster ? caster->GetID() : GetID();
							action->level = 65;
							action->instrument_mod = 10;
							action->sequence = static_cast<uint32>((GetHeading() * 12345 / 2));
							action->type = 231;
							action->spell = spell_id;
							action->buff_unknown = 4;

							cd->target = action->target;
							cd->source = action->source;
							cd->type = action->type;
							cd->spellid = action->spell;
							cd->meleepush_xy = action->sequence;

							CastToClient()->QueuePacket(action_packet);
							if(caster->IsClient() && caster != this)
								caster->CastToClient()->QueuePacket(action_packet);

							CastToClient()->QueuePacket(message_packet);
							if(caster->IsClient() && caster != this)
								caster->CastToClient()->QueuePacket(message_packet);

							CastToClient()->SetBindPoint(spells[spell_id].base[i] - 1);
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
				if(!spellbonuses.AntiGate){

					if(zone->random.Roll(effect_value))
						Gate(spells[spell_id].base2[i] - 1);
					else if (caster)
						caster->Message_StringID(MT_SpellFailure,GATE_FAIL);
				}
				break;
			}

			case SE_CancelMagic:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Cancel Magic: %d", effect_value);
#endif
				if(GetSpecialAbility(UNDISPELLABLE)){
					if (caster)
						caster->Message_StringID(MT_SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}

				int buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if(	buffs[slot].spellid != SPELL_UNKNOWN &&
						spells[buffs[slot].spellid].dispel_flag == 0 &&
						!IsDiscipline(buffs[slot].spellid))
					{
						if (caster && TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
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
					if (caster)
						caster->Message_StringID(MT_SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}

				int buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (buffs[slot].spellid != SPELL_UNKNOWN &&
						IsDetrimentalSpell(buffs[slot].spellid) &&
						spells[buffs[slot].spellid].dispel_flag == 0)
					{
						if (caster && TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
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
					if (caster)
						caster->Message_StringID(MT_SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}

				int buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (buffs[slot].spellid != SPELL_UNKNOWN &&
						IsBeneficialSpell(buffs[slot].spellid) &&
						spells[buffs[slot].spellid].dispel_flag == 0)
					{
						if (caster && TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
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
				int buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (buffs[slot].spellid != SPELL_UNKNOWN &&
						IsDetrimentalSpell(buffs[slot].spellid))
					{
						if (caster && TryDispel(caster->GetLevel(),buffs[slot].casterlevel, effect_value)){
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
				const EQEmu::ItemData *item = database.GetItem(spell.base[i]);
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
							c->SendItemPacket(EQEmu::inventory::slotCursor, SummonedItem, ItemPacketLimbo);
							safe_delete(SummonedItem);
						}
						SummonedItem = database.CreateItem(spell.base[i], charges);
					}
				}

				break;
			}
			case SE_SummonItemIntoBag:
			{
				const EQEmu::ItemData *item = database.GetItem(spell.base[i]);
#ifdef SPELL_EFFECT_SPAM
				const char *itemname = item ? item->Name : "*Unknown Item*";
				snprintf(effect_desc, _EDLEN, "Summon Item In Bag: %s (id %d)", itemname, spell.base[i]);
#endif
				uint8 slot;

				if (!SummonedItem || !SummonedItem->IsClassBag()) {
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

						EQEmu::ItemInstance *SubItem = database.CreateItem(spell.base[i], charges);
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
					// TODO: we need to sync the states for these clients ...
					// Will fix buttons for now
					if (IsClient()) {
						auto c = CastToClient();
						if (c->ClientVersionBit() & EQEmu::versions::bit_UFAndLater) {
							c->SetPetCommandState(PET_BUTTON_SIT, 0);
							c->SetPetCommandState(PET_BUTTON_STOP, 0);
							c->SetPetCommandState(PET_BUTTON_REGROUP, 0);
							c->SetPetCommandState(PET_BUTTON_FOLLOW, 1);
							c->SetPetCommandState(PET_BUTTON_GUARD, 0);
							c->SetPetCommandState(PET_BUTTON_TAUNT, 1);
							c->SetPetCommandState(PET_BUTTON_HOLD, 0);
							c->SetPetCommandState(PET_BUTTON_GHOLD, 0);
							c->SetPetCommandState(PET_BUTTON_FOCUS, 0);
							c->SetPetCommandState(PET_BUTTON_SPELLHOLD, 0);
						}
					}
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
				// this should catch the cures
				if (BeneficialSpell(spell_id) && spells[spell_id].buffduration == 0)
					BuffFadeByEffect(SE_Blind);
				else if (!IsClient())
					CalculateNewFearpoint();
				break;
			}

			case SE_Rune:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Melee Absorb Rune: %+i", effect_value);
#endif
				if (caster)
					effect_value = caster->ApplySpellEffectiveness(spell_id, effect_value);
				
				buffs[buffslot].melee_rune = effect_value;
				break;
			}

			case SE_AbsorbMagicAtt:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Spell Absorb Rune: %+i", effect_value);
#endif
				if(effect_value > 0)
					buffs[buffslot].magic_rune = effect_value;

				break;
			}

			case SE_MitigateMeleeDamage:
			{
				buffs[buffslot].melee_rune = spells[spell_id].max[i];
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
				buffs[buffslot].magic_rune = spells[spell_id].max[i];
				break;
			}

			case SE_MitigateDotDamage:
			{
				buffs[buffslot].dot_rune = spells[spell_id].max[i];
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
						spell.base2[i],
						spell.max[i]
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

				for (int x = EQEmu::textures::textureBegin; x <= EQEmu::textures::LastTintableTexture; x++)
					SendWearChange(x);

				if (caster == this && spell.id != 287 && spell.id != 601 &&
				    (spellbonuses.IllusionPersistence || aabonuses.IllusionPersistence ||
				     itembonuses.IllusionPersistence))
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
						caster->SendAppearancePacket(AT_Size, static_cast<uint32>(caster->GetTarget()->GetSize()));

						for (int x = EQEmu::textures::textureBegin; x <= EQEmu::textures::LastTintableTexture; x++)
							caster->SendWearChange(x);
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

				if(zone->random.Roll(wipechance))
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
					if (caster)
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
				uint16 pet_spellid =  CastToNPC()->GetPetSpellID();
				uint16 pet_ActSpellCost = caster->GetActSpellCost(pet_spellid, spells[pet_spellid].mana);
				int16 ImprovedReclaimMod =	caster->spellbonuses.ImprovedReclaimEnergy +
											caster->itembonuses.ImprovedReclaimEnergy +
											caster->aabonuses.ImprovedReclaimEnergy;

				if (!ImprovedReclaimMod)
					ImprovedReclaimMod = 75; //Reclaim Energy default is 75% of actual mana cost

				pet_ActSpellCost = pet_ActSpellCost*ImprovedReclaimMod/100;

				caster->SetMana(caster->GetMana() + pet_ActSpellCost);

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

				if(IsClient()) {

					if (zone->random.Int(0, 99) > spells[spell_id].base[i]) {
						CastToClient()->SetFeigned(false);
						entity_list.MessageClose_StringID(this, false, 200, 10, STRING_FEIGNFAILED, GetName());
						}
					else
						CastToClient()->SetFeigned(true);
				}
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
				if (IsClient() && (CastToClient()->ClientVersionBit() & EQEmu::versions::bit_SoDAndLater))
					CastToClient()->LocateCorpse();

				break;
			}

			case SE_SummonToCorpse:
			case SE_Revive:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Revive");	// heh the corpse won't see this
#endif
				if (IsCorpse() && CastToCorpse()->IsPlayerCorpse()) {

					if(caster)
						Log(Logs::Detail, Logs::Spells, " corpse being rezzed using spell %i by %s",
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
				// Only allow 2 size changes from Base Size
				float modifyAmount = (static_cast<float>(effect_value) / 100.0f);
				float maxModAmount = GetBaseSize() * modifyAmount * modifyAmount;
				if ((GetSize() <= GetBaseSize() && GetSize() > maxModAmount) ||
					(GetSize() >= GetBaseSize() && GetSize() < maxModAmount) ||
					(GetSize() <= GetBaseSize() && maxModAmount > 1.0f) ||
					(GetSize() >= GetBaseSize() && maxModAmount < 1.0f))
				{
					ChangeSize(GetSize() * modifyAmount);
				}
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
					Client* TargetClient = nullptr;
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
					else if (caster) {
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

						if (TargetClient->GetLevel() <= effect_value){

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
						else if (caster) {
							char level[4];
							ConvertArray(effect_value, level);
							caster->Message_StringID(MT_SpellFailure, 
								SPELL_LEVEL_REQ, level);
						}
					}
					else {
						Message_StringID(4, TARGET_NOT_FOUND);
						Log(Logs::General, Logs::Error, "%s attempted to cast spell id %u with spell effect SE_SummonCorpse, but could not cast target into a Client object.", GetCleanName(), spell_id);
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
					AddProcToWeapon(procid, false, 100, spell_id, caster_level);
				else
					AddProcToWeapon(procid, false, spells[spell_id].base2[i]+100, spell_id, caster_level);
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
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculatePoisonCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters)) {
							if (caster) {
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
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
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculateDiseaseCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters))
						{
							if (caster) {
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
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
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculateCurseCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters))
						{
							if (caster) {
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
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
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculateCorruptionCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters)) {
							if (caster) {
								caster->Message(MT_Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
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
						CastToNPC()->Depop(true);
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
					Stun(static_cast<int>(toss_amt));
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

				auto outapp_push =
				    new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
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
				if(!caster || !IsClient() || !caster->IsClient()){
					break;
				}
				CastToClient()->SacrificeConfirm(caster->CastToClient());
				break;
			}

			case SE_SummonPC:
			{
				if (!caster)
					break;
				if (IsClient()) {
					CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), caster->GetX(),
							       caster->GetY(), caster->GetZ(), caster->GetHeading(), 2,
							       SummonPC);
					Message(15, "You have been summoned!");
					entity_list.ClearAggro(this);
				} else
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
				if (!caster)
					break;
				// this makes necro epic 1.5/2.0 proc work properly
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
				if(zone->random.Roll(spells[spell_id].base[i])) {

					if(caster && caster->IsClient())
						caster->CastToClient()->Escape();
					else
					{
						entity_list.RemoveFromTargets(caster);
						SetInvisible(1);
					}
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
					entity_list.AEAttack(caster, 30, EQEmu::inventory::slotPrimary, 0, true); // on live wars dont get a duration ramp, its a one shot deal

				break;
			}

			case SE_AEMelee:
			{
				//old aa

#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Duration Rampage");
#endif
				aa_timers[aaTimerRampage].Start(effect_value * 10 * 1000); // Live bug, was suppose to be 1 second per value
				break;
			}

			case SE_AETaunt:
			{			
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "AE Taunt");
#endif
				if(caster && caster->IsClient()){
					//Live AE Taunt range is hardcoded at 40 (Spells for AE taunt all use zero range) Target type should be self only.
					float range = 40;
					if (spells[spell_id].max[i])//custom support if you want to alter range of AE Taunt.
						range = spells[spell_id].max[i];

					entity_list.AETaunt(caster->CastToClient(), range, spells[spell_id].base[i]);
				}
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
				int ReuseTime = spells[spell_id].recast_time + spells[spell_id].recovery_time;
				if (!caster)
					break;

				focus = caster->GetFocusEffect(focusFcBaseEffects, spell_id);

				switch(spells[spell_id].skill) {
				case EQEmu::skills::SkillThrowing:
					caster->DoThrowingAttackDmg(this, nullptr, nullptr, spells[spell_id].base[i],spells[spell_id].base2[i], focus,  ReuseTime);
					break;
				case EQEmu::skills::SkillArchery:
					caster->DoArcheryAttackDmg(this, nullptr, nullptr, spells[spell_id].base[i],spells[spell_id].base2[i],focus,  ReuseTime);
					break;
				default:
					caster->DoMeleeSkillAttackDmg(this, spells[spell_id].base[i], spells[spell_id].skill, spells[spell_id].base2[i], focus, false, ReuseTime);
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
				if(caster && caster->IsClient()){
					int dur = spells[spell_id].max[i];
					if (!dur)
						dur = 60;

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
				//This effect does no damage if target is moving.
				if (!RuleB(Spells, PreNerfBardAEDoT) && IsMoving())
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
						r->BalanceHP(spell.base[i], gid, spell.range, caster, spell.base2[i]);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g)
					break;

				g->BalanceHP(spell.base[i], spell.range, caster, spell.base2[i]);
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
						r->BalanceMana(spell.base[i], gid, spell.range, caster, spell.base2[i]);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g)
					break;

				g->BalanceMana(spell.base[i], spell.range, caster, spell.base2[i]);
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

				int32 max_mana = spell.base[i];
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
				int32 max_mana = spell.base[i];
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

				break;
			}

			case SE_Taunt:
			{
				if (caster && IsNPC()){
					caster->Taunt(this->CastToNPC(), false, spell.base[i], true, spell.base2[i]);
				}
				break;
			}

			case SE_AttackSpeed:
				if (spell.base[i] < 100)
					SlowMitigation(caster);
				break;

			case SE_AttackSpeed2:
				if (spell.base[i] < 100)
					SlowMitigation(caster);
				break;

			case SE_AttackSpeed3:
				if (spell.base[i] < 0)
					SlowMitigation(caster);
				break;

			case SE_AttackSpeed4:
				SlowMitigation(caster);
				break;

			case SE_AddHatePct:
			{
				if (IsNPC()){
					int32 new_hate = CastToNPC()->GetHateAmount(caster) * (100 + spell.base[i]) / 100;
					if (new_hate <= 0)
						new_hate = 1;

					CastToNPC()->SetHateAmountOnEnt(caster, new_hate);
				}
				break;
			}

			case SE_Hate:{

				if (buffslot >= 0)
					break;

				if(caster){
					if(effect_value > 0){
						if(caster){
							if(caster->IsClient() && !caster->CastToClient()->GetFeigned())
								AddToHateList(caster, effect_value);
							else if(!caster->IsClient())
								AddToHateList(caster, effect_value);
						}
					}else{
						int32 newhate = GetHateAmount(caster) + effect_value;
						if (newhate < 1)
							SetHateAmountOnEnt(caster,1);
						else
							SetHateAmountOnEnt(caster,newhate);
						}
				}
				break;
			}

			case SE_InterruptCasting:{
				if (buffslot >= 0)
					break;

				if(!spells[spell_id].uninterruptable && IsCasting() && zone->random.Roll(spells[spell_id].base[i]))
					InterruptSpell();

				break;
			}

			case SE_MassGroupBuff:{

				SetMGB(true);
				Message_StringID(MT_Disciplines, MGB_STRING);
				break;
			}

			case SE_IllusionOther: {
				SetProjectIllusion(true);
				Message(10, "The power of your next illusion spell will flow to your grouped target in your place.");
				break;
			}

			case SE_ApplyEffect: {

				if (caster && IsValidSpell(spells[spell_id].base2[i])){
					if(zone->random.Roll(spells[spell_id].base[i]))
						caster->SpellFinished(spells[spell_id].base2[i], this, EQEmu::CastingSlot::Item, 0, -1, spells[spells[spell_id].base2[i]].ResistDiff);
				}
				break;
			}

			case SE_SpellTrigger: {

				if (!SE_SpellTrigger_HasCast) {
					if (caster && caster->TrySpellTrigger(this, spell_id, i))
						SE_SpellTrigger_HasCast = true;
				}
				break;
			}

			case SE_PersistentEffect:
				MakeAura(spell_id);
				break;

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
			case SE_ImprovedDamage:
			case SE_ImprovedDamage2:
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
			case SE_FcTwincast:
			case SE_DelayDeath:
			case SE_CastOnFadeEffect:
			case SE_CastOnFadeEffectNPC:
			case SE_CastOnFadeEffectAlways:
			case SE_CastOnRuneFadeEffect:
			case SE_MaxHPChange:
			case SE_SympatheticProc:
			case SE_FcDamageAmt:
			case SE_FcDamageAmt2:
			case SE_CriticalSpellChance:
			case SE_SpellCritChance:
			case SE_SpellCritDmgIncrease:
			case SE_DotCritDmgIncrease:
			case SE_CriticalHealChance:
			case SE_CriticalHealOverTime:
			case SE_CriticalDoTChance:
			case SE_ProcOnKillShot:
			case SE_ProcOnSpellKillShot:
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
			case SE_FcHealAmt:
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
			case SE_StrikeThrough:
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
			case SE_HeadShot:
			case SE_HeadShotLevel:
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
			case SE_ShieldEquipDmgMod:
			case SE_GroupShielding:
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
			case SE_AStacker:
			case SE_BStacker:
			case SE_CStacker:
			case SE_DStacker:
			case SE_DoubleRiposte:
			case SE_Berserk:
			case SE_Vampirism:
			case SE_Metabolism:
			case SE_FinishingBlow:
			case SE_FinishingBlowLvl:
			case SE_Assassinate:
			case SE_AssassinateLevel:
			case SE_FactionModPct:
			case SE_LimitSpellClass:
			case SE_Sanctuary:
			case SE_PetMeleeMitigation:
			case SE_SkillProc:
			case SE_SkillProcSuccess:
			case SE_SpellResistReduction:
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
		c->SendItemPacket(EQEmu::inventory::slotCursor, SummonedItem, ItemPacketLimbo);
		safe_delete(SummonedItem);
	}

	return true;
}

int Mob::CalcSpellEffectValue(uint16 spell_id, int effect_id, int caster_level, uint32 instrument_mod, Mob *caster,
			      int ticsremaining, uint16 caster_id)
{
	int formula, base, max, effect_value;

	if (!IsValidSpell(spell_id) || effect_id < 0 || effect_id >= EFFECT_COUNT)
		return 0;

	formula = spells[spell_id].formula[effect_id];
	base = spells[spell_id].base[effect_id];
	max = spells[spell_id].max[effect_id];

	if (IsBlankSpellEffect(spell_id, effect_id))
		return 0;

	effect_value = CalcSpellEffectValue_formula(formula, base, max, caster_level, spell_id, ticsremaining);

	// this doesn't actually need to be a song to get mods, just the right skill
	if (EQEmu::skills::IsBardInstrumentSkill(spells[spell_id].skill) &&
	    spells[spell_id].effectid[effect_id] != SE_AttackSpeed &&
	    spells[spell_id].effectid[effect_id] != SE_AttackSpeed2 &&
	    spells[spell_id].effectid[effect_id] != SE_AttackSpeed3 &&
	    spells[spell_id].effectid[effect_id] != SE_Lull &&
	    spells[spell_id].effectid[effect_id] != SE_ChangeFrenzyRad &&
	    spells[spell_id].effectid[effect_id] != SE_Harmony &&
	    spells[spell_id].effectid[effect_id] != SE_CurrentMana &&
	    spells[spell_id].effectid[effect_id] != SE_ManaRegen_v2 &&
		spells[spell_id].effectid[effect_id] != SE_AddFaction) {

		int oval = effect_value;
		int mod = ApplySpellEffectiveness(spell_id, instrument_mod, true, caster_id);
		effect_value = effect_value * mod / 10;
		Log(Logs::Detail, Logs::Spells, "Effect value %d altered with bard modifier of %d to yeild %d",
			oval, mod, effect_value);
	}

	effect_value = mod_effect_value(effect_value, spell_id, spells[spell_id].effectid[effect_id], caster, caster_id);

	return effect_value;
}

// generic formula calculations
int Mob::CalcSpellEffectValue_formula(int formula, int base, int max, int caster_level, uint16 spell_id, int ticsremaining)
{
/*
i need those formulas checked!!!!

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

	Log(Logs::Detail, Logs::Spells, "CSEV: spell %d, formula %d, base %d, max %d, lvl %d. Up/Down %d",
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
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buffdurationformula, spells[spell_id].buffduration) - std::max((ticsremaining - 1), 0);
			if (ticdif < 0)
				ticdif = 0;
			result = updownsign * (ubase - ticdif);
			degenerating_effects = true;
			break;
		}
		case 108:
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buffdurationformula, spells[spell_id].buffduration) - std::max((ticsremaining - 1), 0);
			if (ticdif < 0)
				ticdif = 0;
			result = updownsign * (ubase - (2 * ticdif));
			degenerating_effects = true;
			break;
		}
		case 109:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level / 4)); break;

		case 110:	// confirmed 2/6/04
			//is there a reason we dont use updownsign here???
			result = ubase + (caster_level / 6);
			break;

		case 111:
			result = updownsign * (ubase + 6 * (caster_level - 16));
			break;
		case 112:
			result = updownsign * (ubase + 8 * (caster_level - 24));
			break;
		case 113:
			result = updownsign * (ubase + 10 * (caster_level - 34));
			break;
		case 114:
			result = updownsign * (ubase + 15 * (caster_level - 44));
			break;

		case 115:	// this is only in symbol of transal
			result = ubase;
			if (caster_level > 15)
				result += 7 * (caster_level - 15);
			break;
		case 116:	// this is only in symbol of ryltan
			result = ubase;
			if (caster_level > 24)
				result += 10 * (caster_level - 24);
			break;
		case 117:	// this is only in symbol of pinzarn
			result = ubase;
			if (caster_level > 34)
				result += 13 * (caster_level - 34);
			break;
		case 118:	// used in naltron and a few others
			result = ubase;
			if (caster_level > 44)
				result += 20 * (caster_level - 44);
			break;

		case 119:	// confirmed 2/6/04
			result = ubase + (caster_level / 8); break;
		case 120:
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buffdurationformula, spells[spell_id].buffduration) - std::max((ticsremaining - 1), 0);
			if (ticdif < 0)
				ticdif = 0;
			result = updownsign * (ubase - (5 * ticdif));
			degenerating_effects = true;
			break;
		}
		case 121:	// corrected 2/6/04
			result = ubase + (caster_level / 3); break;
		case 122:
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buffdurationformula, spells[spell_id].buffduration) - std::max((ticsremaining - 1), 0);
			if(ticdif < 0)
				ticdif = 0;

			result = updownsign * (ubase - (12 * ticdif));
			degenerating_effects = true;
			break;
		}
		case 123:	// added 2/6/04
			result = zone->random.Int(ubase, std::abs(max));
			break;

		case 124:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * (caster_level - 50);
			break;

		case 125:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 2 * (caster_level - 50);
			break;

		case 126:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 3 * (caster_level - 50);
			break;

		case 127:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 4 * (caster_level - 50);
			break;

		case 128:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 5 * (caster_level - 50);
			break;

		case 129:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 10 * (caster_level - 50);
			break;

		case 130:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 15 * (caster_level - 50);
			break;

		case 131:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 20 * (caster_level - 50);
			break;

		case 132:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 25 * (caster_level - 50);
			break;

		case 137:	// used in berserker AA desperation
			result = ubase - static_cast<int>((ubase * (GetHPRatio() / 100.0f)));
			break;

		case 138: { // unused on live?
			int maxhps = GetMaxHP() / 2;
			if (GetHP() <= maxhps)
				result = -(ubase * GetHP() / maxhps);
			else
				result = -ubase;
			break;
		}

		case 139:	// check sign
			result = ubase + (caster_level > 30 ? (caster_level - 30) / 2 : 0);
			break;

		case 140:	// check sign
			result = ubase + (caster_level > 30 ? caster_level - 30 : 0);
			break;

		case 141:	// check sign
			result = ubase + (caster_level > 30 ? (3 * caster_level - 90) / 2 : 0);
			break;

		case 142:	// check sign
			result = ubase + (caster_level > 30 ? 2 * caster_level - 60 : 0);
			break;

		case 143:	// check sign
			result = ubase + (3 * caster_level / 4);
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
				int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buffdurationformula, spells[spell_id].buffduration) - std::max((ticsremaining - 1), 0);
				if(ticdif < 0)
					ticdif = 0;

				result = updownsign * (ubase - ((formula - 1000) * ticdif));
				degenerating_effects = true;
			}
			else if((formula >= 2000) && (formula <= 2650))
			{
				// Source: http://crucible.samanna.net/viewtopic.php?f=38&t=6259
				result = ubase * (caster_level * (formula - 2000) + 1);
			}
			else
				Log(Logs::General, Logs::None, "Unknown spell effect value forumula %d", formula);
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

	Log(Logs::Detail, Logs::Spells, "Result: %d (orig %d), cap %d %s", result, oresult, max, (base < 0 && result > 0)?"Inverted due to negative base":"");

	return result;
}


void Mob::BuffProcess()
{
	int buff_count = GetMaxTotalSlots();

	for (int buffs_i = 0; buffs_i < buff_count; ++buffs_i)
	{
		if (buffs[buffs_i].spellid != SPELL_UNKNOWN)
		{
			DoBuffTic(buffs[buffs_i], buffs_i, entity_list.GetMob(buffs[buffs_i].casterid));
			// If the Mob died during DoBuffTic, then the buff we are currently processing will have been removed
			if(buffs[buffs_i].spellid == SPELL_UNKNOWN)
				continue;

			// DF_Permanent uses -1 DF_Aura uses -4 but we need to check negatives for some spells for some reason?
			if (spells[buffs[buffs_i].spellid].buffdurationformula != DF_Permanent &&
			    spells[buffs[buffs_i].spellid].buffdurationformula != DF_Aura) {
				if(!zone->BuffTimersSuspended() || !IsSuspendableSpell(buffs[buffs_i].spellid))
				{
					--buffs[buffs_i].ticsremaining;

					if (buffs[buffs_i].ticsremaining < 0) {
						Log(Logs::Detail, Logs::Spells, "Buff %d in slot %d has expired. Fading.", buffs[buffs_i].spellid, buffs_i);
						BuffFadeBySlot(buffs_i);
					}
					else
					{
						Log(Logs::Detail, Logs::Spells, "Buff %d in slot %d has %d tics remaining.", buffs[buffs_i].spellid, buffs_i, buffs[buffs_i].ticsremaining);
					}
				}
				else if (IsClient() && !(CastToClient()->ClientVersionBit() & EQEmu::versions::bit_SoFAndLater))
				{
					buffs[buffs_i].UpdateClient = true;
				}
			}

			if(IsClient())
			{
				if(buffs[buffs_i].UpdateClient == true)
				{
					CastToClient()->SendBuffDurationPacket(buffs[buffs_i], buffs_i);
					// Hack to get UF to play nicer, RoF seems fine without it
					if (CastToClient()->ClientVersion() == EQEmu::versions::ClientVersion::UF && buffs[buffs_i].numhits > 0)
						CastToClient()->SendBuffNumHitPacket(buffs[buffs_i], buffs_i);
					buffs[buffs_i].UpdateClient = false;
				}
			}
		}
	}
}

void Mob::DoBuffTic(const Buffs_Struct &buff, int slot, Mob *caster)
{
	int effect, effect_value;

	if (!IsValidSpell(buff.spellid))
		return;

	const SPDat_Spell_Struct &spell = spells[buff.spellid];

	if (IsNPC()) {
		std::vector<EQEmu::Any> args;
		args.push_back(&buff.ticsremaining);
		args.push_back(&buff.casterlevel);
		args.push_back(&slot);
		int i = parse->EventSpell(EVENT_SPELL_BUFF_TIC_NPC, CastToNPC(), nullptr, buff.spellid,
					  caster ? caster->GetID() : 0, &args);
		if (i != 0) {
			return;
		}
	} else {
		std::vector<EQEmu::Any> args;
		args.push_back(&buff.ticsremaining);
		args.push_back(&buff.casterlevel);
		args.push_back(&slot);
		int i = parse->EventSpell(EVENT_SPELL_BUFF_TIC_CLIENT, nullptr, CastToClient(), buff.spellid,
					  caster ? caster->GetID() : 0, &args);
		if (i != 0) {
			return;
		}
	}

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (IsBlankSpellEffect(buff.spellid, i))
			continue;

		effect = spell.effectid[i];
		// I copied the calculation into each case which needed it instead of
		// doing it every time up here, since most buff effects dont need it

		switch (effect) {
		case SE_CurrentHP: {
			if (!PassCastRestriction(false, spells[buff.spellid].base2[i], true))
				break;
			effect_value = CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod,
							    caster, buff.ticsremaining);
			// Handle client cast DOTs here.
			if (caster && effect_value < 0) {

				if (IsDetrimentalSpell(buff.spellid)) {
					if (caster->IsClient()) {
						if (!caster->CastToClient()->GetFeigned())
							AddToHateList(caster, -effect_value);
					} else if (!IsClient()) // Allow NPC's to generate hate if casted on other
								// NPC's.
						AddToHateList(caster, -effect_value);
				}

				effect_value = caster->GetActDoTDamage(buff.spellid, effect_value, this);

				caster->ResourceTap(-effect_value, buff.spellid);
				effect_value = -effect_value;
				Damage(caster, effect_value, buff.spellid, spell.skill, false, i, true);
			} else if (effect_value > 0) {
				// Regen spell...
				// handled with bonuses
			}
			break;
		}
		case SE_HealOverTime: {
			effect_value = CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod);
			if (caster)
				effect_value = caster->GetActSpellHealing(buff.spellid, effect_value);

			HealDamage(effect_value, caster, buff.spellid);
			// healing aggro would go here; removed for now
			break;
		}

		case SE_CurrentEndurance: {
			// Handled with bonuses
			break;
		}

		case SE_BardAEDot: {
			effect_value =
			    CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod, caster);

			if ((!RuleB(Spells, PreNerfBardAEDoT) && IsMoving()) || invulnerable ||
			    /*effect_value > 0 ||*/ DivineAura())
				break;

			if (effect_value < 0) {
				effect_value = -effect_value;
				if (caster) {
					if (caster->IsClient() && !caster->CastToClient()->GetFeigned()) {
						AddToHateList(caster, effect_value);
					} else if (!caster->IsClient())
						AddToHateList(caster, effect_value);
				}
				Damage(caster, effect_value, buff.spellid, spell.skill, false, i, true);
			} else if (effect_value > 0) {
				// healing spell...
				HealDamage(effect_value, caster);
				// healing aggro would go here; removed for now
			}
			break;
		}

		case SE_Hate: {
			effect_value = CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod);
			if (caster) {
				if (effect_value > 0) {
					if (caster) {
						if (caster->IsClient() && !caster->CastToClient()->GetFeigned()) {
							AddToHateList(caster, effect_value);
						} else if (!caster->IsClient())
							AddToHateList(caster, effect_value);
					}
				} else {
					int32 newhate = GetHateAmount(caster) + effect_value;
					if (newhate < 1) {
						SetHateAmountOnEnt(caster, 1);
					} else {
						SetHateAmountOnEnt(caster, newhate);
					}
				}
			}
			break;
		}

		case SE_WipeHateList: {
			if (IsMezSpell(buff.spellid))
				break;

			int wipechance = spells[buff.spellid].base[i];
			int bonus = 0;

			if (caster) {
				bonus = caster->spellbonuses.IncreaseChanceMemwipe +
					caster->itembonuses.IncreaseChanceMemwipe +
					caster->aabonuses.IncreaseChanceMemwipe;
			}

			wipechance += wipechance * bonus / 100;

			if (zone->random.Roll(wipechance)) {
				if (IsAIControlled()) {
					WipeHateList();
				}
				Message(13, "Your mind fogs. Who are my friends? Who are my enemies?... it was all so "
					    "clear a moment ago...");
			}
			break;
		}

		case SE_Charm: {
			if (!caster || !PassCharismaCheck(caster, buff.spellid)) {
				BuffFadeByEffect(SE_Charm);
			}

			break;
		}

		case SE_Root: {
			/* Root formula derived from extensive personal live parses - Kayen
			ROOT has a 70% chance to do a resist check to break.
			*/

			if (zone->random.Roll(RuleI(Spells, RootBreakCheckChance))) {
				float resist_check =
				    ResistSpell(spells[buff.spellid].resisttype, buff.spellid, caster, 0, 0, 0, 0, true);

				if (resist_check == 100)
					break;
				else if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}

			break;
		}

		case SE_Fear: {
			if (zone->random.Roll(RuleI(Spells, FearBreakCheckChance))) {
				float resist_check = ResistSpell(spells[buff.spellid].resisttype, buff.spellid, caster,0,0,true);

				if (resist_check == 100)
					break;
				else if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}

			break;
		}

		case SE_Invisibility:
		case SE_InvisVsAnimals:
		case SE_InvisVsUndead: {
			if (buff.ticsremaining > 3) {
				if (!IsBardSong(buff.spellid)) {
					double break_chance = 2.0;
					if (caster) {
						break_chance -= (2 * (((double)caster->GetSkill(EQEmu::skills::SkillDivination) +
								       ((double)caster->GetLevel() * 3.0)) /
								      650.0));
					} else {
						break_chance -=
						    (2 *
							(((double)GetSkill(EQEmu::skills::SkillDivination) + ((double)GetLevel() * 3.0)) /
						      650.0));
					}

					if (zone->random.Real(0.0, 100.0) < break_chance) {
						BuffModifyDurationBySpellID(buff.spellid, 3);
					}
				}
			}
		}
		case SE_Invisibility2:
		case SE_InvisVsUndead2: {
			if (buff.ticsremaining <= 3 && buff.ticsremaining > 1) {
				Message_StringID(MT_Spells, INVIS_BEGIN_BREAK);
			}
			break;
		}
		case SE_InterruptCasting: {
			if (IsCasting()) {
				const auto &spell = spells[casting_spell_id];
				if (!spell.cast_not_standing && zone->random.Roll(spells[buff.spellid].base[i])) {
					InterruptSpell();
				}
			}
			break;
		}
		// These effects always trigger when they fade.
		// Should we have this triggered from else where?
		case SE_CastOnFadeEffect:
		case SE_CastOnFadeEffectNPC:
		case SE_CastOnFadeEffectAlways: {
			if (buff.ticsremaining == 0) {
				SpellOnTarget(spells[buff.spellid].base[i], this);
			}
			break;
		}
		case SE_LocateCorpse: {
			// This is handled by the client prior to SoD.

			if (IsClient() && (CastToClient()->ClientVersionBit() & EQEmu::versions::bit_SoDAndLater))
				CastToClient()->LocateCorpse();
		}

		case SE_DistanceRemoval: {
			if (spellbonuses.DistanceRemoval) {

				int distance =
				    ((int(GetX()) - buff.caston_x) * (int(GetX()) - buff.caston_x)) +
				    ((int(GetY()) - buff.caston_y) * (int(GetY()) - buff.caston_y)) +
				    ((int(GetZ()) - buff.caston_z) * (int(GetZ()) - buff.caston_z));

				if (distance > (spells[buff.spellid].base[i] * spells[buff.spellid].base[i])) {

					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot, true);
				}
				break;
			}
		}

		case SE_AddHateOverTimePct: {
			if (IsNPC()) {
				uint32 new_hate = CastToNPC()->GetHateAmount(caster) * (100 + spell.base[i]) / 100;
				if (new_hate <= 0)
					new_hate = 1;

				CastToNPC()->SetHateAmountOnEnt(caster, new_hate);
			}
			break;
		}

		default: {
			// do we need to do anyting here?
		}
		}
		if (!IsValidSpell(buff.spellid)) // if we faded we're no longer valid!
			break;
	}

	/* Is this the best place for this?
	 * Ideally we would only recalc spell bonuses
	 * but we would also have to call all the Calc functions like Max HP
	 * so lets just call the main CalcBonuses
	 */
	if (degenerating_effects)
		CalcBonuses();
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

	Log(Logs::Detail, Logs::Spells, "Fading buff %d from slot %d", buffs[slot].spellid, slot);

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
		std::vector<EQEmu::Any> args;
		args.push_back(&buffs[slot].casterid);

		parse->EventSpell(EVENT_SPELL_FADE, nullptr, CastToClient(), buffs[slot].spellid, slot, &args);
	} else if(IsNPC()) {
		std::vector<EQEmu::Any> args;
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
				for (int x = EQEmu::textures::textureBegin; x <= EQEmu::textures::LastTintableTexture; x++){
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
				}

				SendAppearancePacket(AT_Pet, 0, true, true);
				Mob* tempmob = GetOwner();
				SetOwnerID(0);
				SetPetType(petNone);
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
					auto app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
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
						CastToClient()->AI_Start(CLIENT_LD_TIMEOUT);
					else
					{
						bool feared = FindType(SE_Fear);
						if(!feared)
							CastToClient()->AI_Stop();
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

			case SE_Blind:
				if (currently_fleeing && !FindType(SE_Fear))
					currently_fleeing = false;
				break;

			case SE_Fear:
			{
				if(RuleB(Combat, EnableFearPathing)){
					if(IsClient())
					{
						bool charmed = FindType(SE_Charm);
						if(!charmed)
							CastToClient()->AI_Stop();
					}

					if(currently_fleeing) {
						currently_fleeing = false;
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
						currently_fleeing = true;
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

			case SE_AlterNPCLevel:
			{
				if (IsNPC())
					SetLevel(GetOrigLevel());
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

	if (spells[buffs[slot].spellid].NimbusEffect > 0)
		RemoveNimbusEffect(spells[buffs[slot].spellid].NimbusEffect);

	buffs[slot].spellid = SPELL_UNKNOWN;
	if(IsPet() && GetOwner() && GetOwner()->IsClient()) {
		SendPetBuffsToClient();
	}
	if((IsClient() && !CastToClient()->GetPVP()) ||
		(IsPet() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
#ifdef BOTS
		(IsBot() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
#endif
		(IsMerc() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()))
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();

		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQEmu::versions::bit_SoDAndLater);
		if(IsClient() && GetTarget() == this) {
			CastToClient()->QueuePacket(outapp);
		}

		safe_delete(outapp);
	}

	if (IsNPC()) {
		EQApplicationPacket *outapp = MakeBuffsPacket();
		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQEmu::versions::bit_SoDAndLater, true);
		safe_delete(outapp);
	}

	if (IsClient() && CastToClient()->ClientVersionBit() & EQEmu::versions::bit_UFAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket(false);
		CastToClient()->FastQueuePacket(&outapp);
	}

	// we will eventually call CalcBonuses() even if we skip it right here, so should correct itself if we still have them
	degenerating_effects = false;
	if (iRecalcBonuses)
		CalcBonuses();
}

int16 Client::CalcAAFocus(focusType type, const AA::Rank &rank, uint16 spell_id)
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
	bool LimitInclude[MaxLimitInclude] = {false};
	/* Certain limits require only one of several Include conditions to be true. Ie. Add damage to fire OR ice
	spells.
	0/1   SE_LimitResist
	2/3   SE_LimitSpell
	4/5   SE_LimitEffect
	6/7   SE_LimitTarget
	8/9   SE_LimitSpellGroup:
	10/11 SE_LimitCastingSkill:
	12/13 SE_LimitSpellClass:
	14/15 SE_LimitSpellSubClass:
	Remember: Update MaxLimitInclude in spdat.h if adding new limits that require Includes
	*/
	int FocusCount = 0;

	for (const auto &e : rank.effects) {
		effect = e.effect_id;
		base1 = e.base1;
		base2 = e.base2;
		slot = e.slot;

		/*
		AA Foci's can contain multiple focus effects within the same AA.
		To handle this we will not automatically return zero if a limit is found.
		Instead if limit is found and multiple focus effects, we will reset the limit check
		when the next valid focus effect is found.
		*/

		if (IsFocusEffect(0, 0, true, effect) || (effect == SE_TriggerOnCast)) {
			FocusCount++;
			// If limit found on prior check next, else end loop.
			if (FocusCount > 1) {

				for (int e = 0; e < MaxLimitInclude; e += 2) {
					if (LimitInclude[e] && !LimitInclude[e + 1])
						LimitFailure = true;
				}

				if (LimitFailure) {
					value = 0;
					LimitFailure = false;

					for (int e = 0; e < MaxLimitInclude; e++) {
						LimitInclude[e] = false; // Reset array
					}
				}

				else {
					break;
				}
			}
		}

		switch (effect) {
		case SE_Blank:
			break;

		// Handle Focus Limits

		case SE_LimitResist:
			if (base1 < 0) {
				if (spell.resisttype == -base1) // Exclude
					LimitFailure = true;
			} else {
				LimitInclude[0] = true;
				if (spell.resisttype == base1) // Include
					LimitInclude[1] = true;
			}
			break;

		case SE_LimitInstant:
			if (base1 == 1 && spell.buffduration) // Fail if not instant
				LimitFailure = true;
			if (base1 == 0 && (spell.buffduration == 0)) // Fail if instant
				LimitFailure = true;

			break;

		case SE_LimitMaxLevel:
			spell_level = spell.classes[(GetClass() % 17) - 1];
			lvldiff = spell_level - base1;
			// every level over cap reduces the effect by base2 percent unless from a clicky when
			// ItemCastsUseFocus is true
			if (lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) ||
					    RuleB(Character, ItemCastsUseFocus) == false)) {
				if (base2 > 0) {
					lvlModifier -= base2 * lvldiff;
					if (lvlModifier < 1)
						LimitFailure = true;
				} else
					LimitFailure = true;
			}
			break;

		case SE_LimitMinLevel:
			if ((spell.classes[(GetClass() % 17) - 1]) < base1)
				LimitFailure = true;
			break;

		case SE_LimitCastTimeMin:
			if (static_cast<int32>(spell.cast_time) < base1)
				LimitFailure = true;
			break;

		case SE_LimitCastTimeMax:
			if (static_cast<int32>(spell.cast_time) > base1)
				LimitFailure = true;
			break;

		case SE_LimitSpell:
			if (base1 < 0) { // Exclude
				if (spell_id == -base1)
					LimitFailure = true;
			} else {
				LimitInclude[2] = true;
				if (spell_id == base1) // Include
					LimitInclude[3] = true;
			}
			break;

		case SE_LimitMinDur:
			if (base1 > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
				LimitFailure = true;

			break;

		case SE_LimitEffect:
			if (base1 < 0) {
				if (IsEffectInSpell(spell_id, -base1)) // Exclude
					LimitFailure = true;
			} else {
				LimitInclude[4] = true;
				// they use 33 here for all classes ... unsure if the type check is really needed
				if (base1 == SE_SummonPet && type == focusReagentCost) {
					if (IsSummonPetSpell(spell_id) || IsSummonSkeletonSpell(spell_id))
						LimitInclude[5] = true;
				} else {
					if (IsEffectInSpell(spell_id, base1)) // Include
						LimitInclude[5] = true;
				}
			}
			break;

		case SE_LimitSpellType:
			switch (base1) {
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
			if (spell.mana < base1)
				LimitFailure = true;
			break;

		case SE_LimitManaMax:
			if (spell.mana > base1)
				LimitFailure = true;
			break;

		case SE_LimitTarget:
			if (base1 < 0) {
				if (-base1 == spell.targettype) // Exclude
					LimitFailure = true;
			} else {
				LimitInclude[6] = true;
				if (base1 == spell.targettype) // Include
					LimitInclude[7] = true;
			}
			break;

		case SE_LimitCombatSkills:
			if (base1 == 0 && (IsCombatSkill(spell_id) || IsCombatProc(spell_id))) // Exclude Discs / Procs
				LimitFailure = true;
			else if (base1 == 1 && (!IsCombatSkill(spell_id) || !IsCombatProc(spell_id))) // Exclude Spells
				LimitFailure = true;

			break;

		case SE_LimitSpellGroup:
			if (base1 < 0) {
				if (-base1 == spell.spellgroup) // Exclude
					LimitFailure = true;
			} else {
				LimitInclude[8] = true;
				if (base1 == spell.spellgroup) // Include
					LimitInclude[9] = true;
			}
			break;

		case SE_LimitCastingSkill:
			if (base1 < 0) {
				if (-base1 == spell.skill)
					LimitFailure = true;
			} else {
				LimitInclude[10] = true;
				if (base1 == spell.skill)
					LimitInclude[11] = true;
			}
			break;

		case SE_LimitSpellClass:
			if (base1 < 0) { // Exclude
				if (CheckSpellCategory(spell_id, base1, SE_LimitSpellClass))
					return (0);
			} else {
				LimitInclude[12] = true;
				if (CheckSpellCategory(spell_id, base1, SE_LimitSpellClass)) // Include
					LimitInclude[13] = true;
			}
			break;

		case SE_LimitSpellSubclass:
			if (base1 < 0) { // Exclude
				if (CheckSpellCategory(spell_id, base1, SE_LimitSpellSubclass))
					return (0);
			} else {
				LimitInclude[14] = true;
				if (CheckSpellCategory(spell_id, base1, SE_LimitSpellSubclass)) // Include
					LimitInclude[15] = true;
			}
			break;

		case SE_LimitClass:
			// Do not use this limit more then once per spell. If multiple class, treat value like items
			// would.
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

		// Handle Focus Effects
		case SE_ImprovedDamage:
			if (type == focusImprovedDamage && base1 > value)
				value = base1;
			break;

		case SE_ImprovedDamage2:
			if (type == focusImprovedDamage2 && base1 > value)
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
			if (type == focusSpellHateMod) {
				if (value != 0) {
					if (value > 0) {
						if (base1 > value)
							value = base1;
					} else {
						if (base1 < value)
							value = base1;
					}
				} else
					value = base1;
			}
			break;

		case SE_ReduceReuseTimer:
			if (type == focusReduceRecastTime)
				value = base1 / 1000;
			break;

		case SE_TriggerOnCast:
			if (type == focusTriggerOnCast) {
				if (zone->random.Roll(base1)) {
					value = base2;
				} else {
					value = 0;
					LimitFailure = true;
				}
				break;
			}

		case SE_FcSpellVulnerability:
			if (type == focusSpellVulnerability)
				value = base1;
			break;

		case SE_BlockNextSpellFocus:
			if (type == focusBlockNextSpell) {
				if (zone->random.Roll(base1))
					value = 1;
			}
			break;

		case SE_FcTwincast:
			if (type == focusTwincast)
				value = base1;
			break;

		// Note if using these as AA, make sure this is first focus used.
		case SE_SympatheticProc:
			if (type == focusSympatheticProc)
				value = base2;
			break;

		case SE_FcDamageAmt:
			if (type == focusFcDamageAmt)
				value = base1;
			break;

		case SE_FcDamageAmt2:
			if (type == focusFcDamageAmt2)
				value = base1;
			break;

		case SE_FcDamageAmtCrit:
			if (type == focusFcDamageAmtCrit)
				value = base1;
			break;

		case SE_FcDamageAmtIncoming:
			if (type == focusFcDamageAmtIncoming)
				value = base1;
			break;

		case SE_FcHealAmtIncoming:
			if (type == focusFcHealAmtIncoming)
				value = base1;
			break;

		case SE_FcHealPctCritIncoming:
			if (type == focusFcHealPctCritIncoming)
				value = base1;
			break;

		case SE_FcHealAmtCrit:
			if (type == focusFcHealAmtCrit)
				value = base1;
			break;

		case SE_FcHealAmt:
			if (type == focusFcHealAmt)
				value = base1;
			break;

		case SE_FcHealPctIncoming:
			if (type == focusFcHealPctIncoming)
				value = base1;
			break;

		case SE_FcBaseEffects:
			if (type == focusFcBaseEffects)
				value = base1;
			break;

		case SE_FcDamagePctCrit:
			if (type == focusFcDamagePctCrit)
				value = base1;
			break;

		case SE_FcIncreaseNumHits:
			if (type == focusIncreaseNumHits)
				value = base1;
			break;

		case SE_FcLimitUse:
			if (type == focusFcLimitUse)
				value = base1;
			break;

		case SE_FcMute:
			if (type == focusFcMute)
				value = base1;
			break;

		case SE_FcStunTimeMod:
			if (type == focusFcStunTimeMod)
				value = base1;
			break;
		}
	}

	for (int e = 0; e < MaxLimitInclude; e += 2) {
		if (LimitInclude[e] && !LimitInclude[e + 1])
			return 0;
	}

	if (LimitFailure)
		return 0;

	return (value * lvlModifier / 100);
}

//given an item/spell's focus ID and the spell being cast, determine the focus ammount, if any
//assumes that spell_id is not a bard spell and that both ids are valid spell ids
int16 Mob::CalcFocusEffect(focusType type, uint16 focus_id, uint16 spell_id, bool best_focus)
{
	if (!IsValidSpell(focus_id) || !IsValidSpell(spell_id))
		return 0;

	const SPDat_Spell_Struct &focus_spell = spells[focus_id];
	const SPDat_Spell_Struct &spell = spells[spell_id];

	int16 value = 0;
	int lvlModifier = 100;
	int spell_level = 0;
	int lvldiff = 0;
	uint32 Caston_spell_id = 0;

	bool LimitInclude[MaxLimitInclude] = {false};
	/* Certain limits require only one of several Include conditions to be true. Ie. Add damage to fire OR ice
	spells.
	0/1   SE_LimitResist
	2/3   SE_LimitSpell
	4/5   SE_LimitEffect
	6/7   SE_LimitTarget
	8/9   SE_LimitSpellGroup:
	10/11 SE_LimitCastingSkill:
	12/13 SE_LimitSpellClass:
	14/15 SE_LimitSpellSubClass:
	Remember: Update MaxLimitInclude in spdat.h if adding new limits that require Includes
	*/

	for (int i = 0; i < EFFECT_COUNT; i++) {

		switch (focus_spell.effectid[i]) {

		case SE_Blank:
			break;

		case SE_LimitResist:
			if (focus_spell.base[i] < 0) {
				if (spell.resisttype == -focus_spell.base[i]) // Exclude
					return 0;
			} else {
				LimitInclude[0] = true;
				if (spell.resisttype == focus_spell.base[i]) // Include
					LimitInclude[1] = true;
			}
			break;

		case SE_LimitInstant:
			if (focus_spell.base[i] == 1 && spell.buffduration) // Fail if not instant
				return 0;
			if (focus_spell.base[i] == 0 && (spell.buffduration == 0)) // Fail if instant
				return 0;

			break;

		case SE_LimitMaxLevel:
			if (IsNPC())
				break;
			spell_level = spell.classes[(GetClass() % 17) - 1];
			lvldiff = spell_level - focus_spell.base[i];
			// every level over cap reduces the effect by focus_spell.base2[i] percent unless from a clicky
			// when ItemCastsUseFocus is true
			if (lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) ||
					    RuleB(Character, ItemCastsUseFocus) == false)) {
				if (focus_spell.base2[i] > 0) {
					lvlModifier -= focus_spell.base2[i] * lvldiff;
					if (lvlModifier < 1)
						return 0;
				} else
					return 0;
			}
			break;

		case SE_LimitMinLevel:
			if (IsNPC())
				break;
			if (spell.classes[(GetClass() % 17) - 1] < focus_spell.base[i])
				return (0);
			break;

		case SE_LimitCastTimeMin:
			if (spells[spell_id].cast_time < (uint16)focus_spell.base[i])
				return (0);
			break;

		case SE_LimitCastTimeMax:
			if (spells[spell_id].cast_time > (uint16)focus_spell.base[i])
				return (0);
			break;

		case SE_LimitSpell:
			if (focus_spell.base[i] < 0) { // Exclude
				if (spell_id == -focus_spell.base[i])
					return (0);
			} else {
				LimitInclude[2] = true;
				if (spell_id == focus_spell.base[i]) // Include
					LimitInclude[3] = true;
			}
			break;

		case SE_LimitMinDur:
			if (focus_spell.base[i] >
			    CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
				return (0);
			break;

		case SE_LimitEffect:
			if (focus_spell.base[i] < 0) {
				if (IsEffectInSpell(spell_id, -focus_spell.base[i])) // Exclude
					return 0;
			} else {
				LimitInclude[4] = true;
				if (IsEffectInSpell(spell_id, focus_spell.base[i])) // Include
					LimitInclude[5] = true;
			}
			break;

		case SE_LimitSpellType:
			switch (focus_spell.base[i]) {
			case 0:
				if (!IsDetrimentalSpell(spell_id))
					return 0;
				break;
			case 1:
				if (!IsBeneficialSpell(spell_id))
					return 0;
				break;
			default:
				Log(Logs::General, Logs::Normal, "CalcFocusEffect: unknown limit spelltype %d",
					focus_spell.base[i]);
			}
			break;

		case SE_LimitManaMin:
			if (spell.mana < focus_spell.base[i])
				return 0;
			break;

		case SE_LimitManaMax:
			if (spell.mana > focus_spell.base[i])
				return 0;
			break;

		case SE_LimitTarget:
			if (focus_spell.base[i] < 0) {
				if (-focus_spell.base[i] == spell.targettype) // Exclude
					return 0;
			} else {
				LimitInclude[6] = true;
				if (focus_spell.base[i] == spell.targettype) // Include
					LimitInclude[7] = true;
			}
			break;

		case SE_LimitCombatSkills:
			if (focus_spell.base[i] == 0 &&
			    (IsCombatSkill(spell_id) || IsCombatProc(spell_id))) // Exclude Discs / Procs
				return 0;
			else if (focus_spell.base[i] == 1 &&
				 (!IsCombatSkill(spell_id) || !IsCombatProc(spell_id))) // Exclude Spells
				return 0;

			break;

		case SE_LimitSpellGroup:
			if (focus_spell.base[i] < 0) {
				if (-focus_spell.base[i] == spell.spellgroup) // Exclude
					return 0;
			} else {
				LimitInclude[8] = true;
				if (focus_spell.base[i] == spell.spellgroup) // Include
					LimitInclude[9] = true;
			}
			break;

		case SE_LimitCastingSkill:
			if (focus_spell.base[i] < 0) {
				if (-focus_spell.base[i] == spell.skill)
					return 0;
			} else {
				LimitInclude[10] = true;
				if (focus_spell.base[i] == spell.skill)
					LimitInclude[11] = true;
			}
			break;

		case SE_LimitClass:
			// Do not use this limit more then once per spell. If multiple class, treat value like items
			// would.
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

		case SE_LimitSpellClass:
			if (focus_spell.base[i] < 0) { // Exclude
				if (CheckSpellCategory(spell_id, focus_spell.base[i], SE_LimitSpellClass))
					return (0);
			} else {
				LimitInclude[12] = true;
				if (CheckSpellCategory(spell_id, focus_spell.base[i], SE_LimitSpellClass)) // Include
					LimitInclude[13] = true;
			}
			break;

		case SE_LimitSpellSubclass:
			if (focus_spell.base[i] < 0) { // Exclude
				if (CheckSpellCategory(spell_id, focus_spell.base[i], SE_LimitSpellSubclass))
					return (0);
			} else {
				LimitInclude[14] = true;
				if (CheckSpellCategory(spell_id, focus_spell.base[i], SE_LimitSpellSubclass)) // Include
					LimitInclude[15] = true;
			}
			break;

		// handle effects
		case SE_ImprovedDamage:
			if (type == focusImprovedDamage) {
				// This is used to determine which focus should be used for the random calculation
				if (best_focus) {
					// If the spell contains a value in the base2 field then that is the max value
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					}
					// If the spell does not contain a base2 value, then its a straight non random
					// value
					else {
						value = focus_spell.base[i];
					}
				}
				// Actual focus calculation starts here
				else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				} else {
					value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_ImprovedDamage2:
			if (type == focusImprovedDamage2) {
				if (best_focus) {
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					}
					else {
						value = focus_spell.base[i];
					}
				}
				else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				} else {
					value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_ImprovedHeal:
			if (type == focusImprovedHeal) {
				if (best_focus) {
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					} else {
						value = focus_spell.base[i];
					}
				} else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				} else {
					value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_ReduceManaCost:
			if (type == focusManaCost) {
				if (best_focus) {
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					} else {
						value = focus_spell.base[i];
					}
				} else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				} else {
					value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
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
			if (type == focusResistRate) {
				if (best_focus) {
					if (focus_spell.base2[i] != 0) {
						value = focus_spell.base2[i];
					} else {
						value = focus_spell.base[i];
					}
				} else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				} else {
					value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_SpellHateMod:
			if (type == focusSpellHateMod) {
				if (value != 0) {
					if (value > 0) {
						if (focus_spell.base[i] > value)
							value = focus_spell.base[i];
					} else {
						if (focus_spell.base[i] < value)
							value = focus_spell.base[i];
					}
				} else
					value = focus_spell.base[i];
			}
			break;

		case SE_ReduceReuseTimer:
			if (type == focusReduceRecastTime)
				value = focus_spell.base[i] / 1000;
			break;

		case SE_TriggerOnCast:
			if (type == focusTriggerOnCast) {
				if (zone->random.Roll(focus_spell.base[i]))
					value = focus_spell.base2[i];
				else
					value = 0;
			}
			break;

		case SE_BlockNextSpellFocus:
			if (type == focusBlockNextSpell) {
				if (zone->random.Roll(focus_spell.base[i]))
					value = 1;
			}
			break;

		case SE_SympatheticProc:
			if (type == focusSympatheticProc) {
				value = focus_id;
			}
			break;

		case SE_FcSpellVulnerability:
			if (type == focusSpellVulnerability) {
				if (best_focus) {
					if (focus_spell.base2[i] != 0)
						value = focus_spell.base2[i];
					else
						value = focus_spell.base[i];
				} else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i]) {
					value = focus_spell.base[i];
				} else {
					value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
			}
			break;

		case SE_FcTwincast:
			if (type == focusTwincast)
				value = focus_spell.base[i];
			break;

		case SE_FcDamageAmt:
			if (type == focusFcDamageAmt)
				value = focus_spell.base[i];
			break;

		case SE_FcDamageAmt2:
			if (type == focusFcDamageAmt2)
				value = focus_spell.base[i];
			break;

		case SE_FcDamageAmtCrit:
			if (type == focusFcDamageAmtCrit)
				value = focus_spell.base[i];
			break;

		case SE_FcDamageAmtIncoming:
			if (type == focusFcDamageAmtIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcHealAmtIncoming:
			if (type == focusFcHealAmtIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcDamagePctCrit:
			if (type == focusFcDamagePctCrit)
				value = focus_spell.base[i];
			break;

		case SE_FcHealPctCritIncoming:
			if (type == focusFcHealPctCritIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcHealAmtCrit:
			if (type == focusFcHealAmtCrit)
				value = focus_spell.base[i];
			break;

		case SE_FcHealAmt:
			if (type == focusFcHealAmt)
				value = focus_spell.base[i];
			break;

		case SE_FcHealPctIncoming:
			if (type == focusFcHealPctIncoming)
				value = focus_spell.base[i];
			break;

		case SE_FcBaseEffects:
			if (type == focusFcBaseEffects)
				value = focus_spell.base[i];
			break;

		case SE_FcIncreaseNumHits:
			if (type == focusIncreaseNumHits)
				value = focus_spell.base[i];
			break;

		case SE_FcLimitUse:
			if (type == focusFcLimitUse)
				value = focus_spell.base[i];
			break;

		case SE_FcMute:
			if (type == focusFcMute)
				value = focus_spell.base[i];
			break;

		case SE_FcStunTimeMod:
			if (type == focusFcStunTimeMod)
				value = focus_spell.base[i];
			break;

		case SE_FcTimerRefresh:
			if (type == focusFcTimerRefresh)
				value = focus_spell.base[i];
			break;

#if EQDEBUG >= 6
		// this spits up a lot of garbage when calculating spell focuses
		// since they have all kinds of extra effects on them.
		default:
			Log(Logs::General, Logs::Normal, "CalcFocusEffect: unknown effectid %d",
				focus_spell.effectid[i]);
#endif
		}
	}

	for (int e = 0; e < MaxLimitInclude; e += 2) {
		if (LimitInclude[e] && !LimitInclude[e + 1])
			return 0;
	}

	if (Caston_spell_id) {
		if (IsValidSpell(Caston_spell_id) && (Caston_spell_id != spell_id))
			SpellFinished(Caston_spell_id, this, EQEmu::CastingSlot::Item, 0, -1, spells[Caston_spell_id].ResistDiff);
	}

	return (value * lvlModifier / 100);
}

uint16 Client::GetSympatheticFocusEffect(focusType type, uint16 spell_id) {

	if (IsBardSong(spell_id))
		return 0;

	uint16 proc_spellid = 0;
	float ProcChance = 0.0f;

	std::vector<int> SympatheticProcList;

	//item focus
	if (itembonuses.FocusEffects[type]){

		const EQEmu::ItemData* TempItem = nullptr;

		for (int x = EQEmu::legacy::EQUIPMENT_BEGIN; x <= EQEmu::legacy::EQUIPMENT_END; x++)
		{
			if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS)
				continue;

			TempItem = nullptr;
			EQEmu::ItemInstance* ins = GetInv().GetItem(x);
			if (!ins)
				continue;
			TempItem = ins->GetItem();
			if (TempItem && TempItem->Focus.Effect > 0 && IsValidSpell(TempItem->Focus.Effect)) {

				proc_spellid = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
				if (IsValidSpell(proc_spellid)){
					ProcChance = GetSympatheticProcChances(spell_id, spells[TempItem->Focus.Effect].base[0], TempItem->ProcRate);
					if(zone->random.Roll(ProcChance))
 						SympatheticProcList.push_back(proc_spellid);
				}
			}

			for (int y = EQEmu::inventory::socketBegin; y < EQEmu::inventory::SocketCount; ++y)
			{
				if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS)
					continue;

				EQEmu::ItemInstance *aug = nullptr;
				aug = ins->GetAugment(y);
				if(aug)
				{
					const EQEmu::ItemData* TempItemAug = aug->GetItem();
					if (TempItemAug && TempItemAug->Focus.Effect > 0 && IsValidSpell(TempItemAug->Focus.Effect)) {
						proc_spellid = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_id);
						if (IsValidSpell(proc_spellid)){
							ProcChance = GetSympatheticProcChances(spell_id, spells[TempItemAug->Focus.Effect].base[0], TempItemAug->ProcRate);
							if(zone->random.Roll(ProcChance))
								SympatheticProcList.push_back(proc_spellid);
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
		int buff_max = GetMaxTotalSlots();
		for (buff_slot = 0; buff_slot < buff_max; buff_slot++) {

			if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS)
				continue;

			focusspellid = buffs[buff_slot].spellid;
			if (!IsValidSpell(focusspellid))
				continue;

				proc_spellid = CalcFocusEffect(type, focusspellid, spell_id);

			if (IsValidSpell(proc_spellid)){

				ProcChance = GetSympatheticProcChances(spell_id, GetSympatheticSpellProcRate(proc_spellid));
				if(zone->random.Roll(ProcChance))
 					SympatheticProcList.push_back(proc_spellid);
			}
		}
	}

	/*Note: At present, ff designing custom AA to have a sympathetic proc effect, only use one focus
	effect within the aa_effects data for each AA*[No live AA's use this effect to my knowledge]*/
	if (aabonuses.FocusEffects[type]) {
		for (const auto &aa : aa_ranks) {
			if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS)
				break;

			auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
			auto ability = ability_rank.first;
			auto rank = ability_rank.second;

			if(!ability) {
				continue;
			}

			if (rank->effects.empty())
				continue;

			proc_spellid = CalcAAFocus(type, *rank, spell_id);
			if (IsValidSpell(proc_spellid)) {
				ProcChance = GetSympatheticProcChances(spell_id, rank->effects[0].base1);
				if (zone->random.Roll(ProcChance))
					SympatheticProcList.push_back(proc_spellid);
			}
		}
	}

	if (!SympatheticProcList.empty())
	{
		uint8 random = zone->random.Int(0, SympatheticProcList.size()-1);
		int FinalSympatheticProc = SympatheticProcList[random];
		SympatheticProcList.clear();
		return FinalSympatheticProc;
	}

	return 0;
}

int16 Client::GetFocusEffect(focusType type, uint16 spell_id)
{
	if (IsBardSong(spell_id) && type != focusFcBaseEffects && type != focusSpellDuration)
		return 0;

	if (spells[spell_id].not_focusable)
		return 0;

	int16 realTotal = 0;
	int16 realTotal2 = 0;
	int16 realTotal3 = 0;
	bool rand_effectiveness = false;

	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if(RuleB(Spells, LiveLikeFocusEffects) && (type == focusManaCost || type == focusImprovedHeal || type == focusImprovedDamage || type == focusImprovedDamage2 || type == focusResistRate))
		rand_effectiveness = true;

	//Check if item focus effect exists for the client.
	if (itembonuses.FocusEffects[type]){

		const EQEmu::ItemData* TempItem = nullptr;
		const EQEmu::ItemData* UsedItem = nullptr;
		uint16 UsedFocusID = 0;
		int16 Total = 0;
		int16 focus_max = 0;
		int16 focus_max_real = 0;

		//item focus
		for (int x = EQEmu::legacy::EQUIPMENT_BEGIN; x <= EQEmu::legacy::EQUIPMENT_END; x++)
		{
			TempItem = nullptr;
			EQEmu::ItemInstance* ins = GetInv().GetItem(x);
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

			for (int y = EQEmu::inventory::socketBegin; y < EQEmu::inventory::SocketCount; ++y)
			{
				EQEmu::ItemInstance *aug = nullptr;
				aug = ins->GetAugment(y);
				if(aug)
				{
					const EQEmu::ItemData* TempItemAug = aug->GetItem();
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
		for (int x = EQEmu::legacy::TRIBUTE_BEGIN; x <= EQEmu::legacy::TRIBUTE_END; ++x)
		{
			TempItem = nullptr;
			EQEmu::ItemInstance* ins = GetInv().GetItem(x);
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

		if ((rand_effectiveness && UsedItem) || (realTotal != 0 && UsedItem)) {
			// there are a crap ton more of these, I was able to verify these ones though
			// the RNG effective ones appear to have a different message for failing to focus
			uint32 string_id = BEGINS_TO_GLOW; // this is really just clicky message ...
			switch (type) {
			case focusSpellHaste:
				string_id = SHIMMERS_BRIEFLY;
				break;
			case focusManaCost: // this might be GROWS_DIM for fail
				string_id = FLICKERS_PALE_LIGHT;
				break;
			case focusSpellDuration:
				string_id = SPARKLES;
				break;
			case focusImprovedDamage:
			case focusImprovedDamage2:
				if (realTotal)
					string_id = ALIVE_WITH_POWER;
				else
					string_id = SEEMS_DRAINED;
				break;
			case focusRange:
				string_id = PULSES_WITH_LIGHT;
				break;
			case focusSpellHateMod: // GLOWS_RED for increasing hate
				string_id = GLOWS_BLUE;
				break;
			case focusImprovedHeal:
				if (realTotal)
					string_id = FEEDS_WITH_POWER;
				else
					string_id = POWER_DRAIN_INTO;
				break;
			case focusReagentCost: // this might be GROWS_DIM for fail as well ...
				string_id = BEGINS_TO_SHINE;
				break;
			default:
				break;
			}
			Message_StringID(MT_Spells, string_id, UsedItem->Name);
		}
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
		int buff_max = GetMaxTotalSlots();
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

		for (const auto &aa : aa_ranks) {
			auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
			auto ability = ability_rank.first;
			auto rank = ability_rank.second;

			if(!ability) {
				continue;
			}

			if (rank->effects.empty())
				continue;

			Total3 = CalcAAFocus(type, *rank, spell_id);
			if (Total3 > 0 && realTotal3 >= 0 && Total3 > realTotal3) {
				realTotal3 = Total3;
			}
			else if (Total3 < 0 && Total3 < realTotal3) {
				realTotal3 = Total3;
			}
		}
	}

	if(type == focusReagentCost && (IsEffectInSpell(spell_id, SE_SummonItem) || IsSacrificeSpell(spell_id)))
		return 0;
	//Summon Spells that require reagents are typically imbue type spells, enchant metal, sacrifice and shouldn't be affected
	//by reagent conservation for obvious reasons.

	//Non-Live like feature to allow for an additive focus bonus to be applied from foci that are placed in worn slot. (No limit checks)
	int16 worneffect_bonus = 0;
	if (RuleB(Spells, UseAdditiveFocusFromWornSlot))
		worneffect_bonus = itembonuses.FocusEffectsWorn[type];

	return realTotal + realTotal2 + realTotal3 + worneffect_bonus;
}

int16 NPC::GetFocusEffect(focusType type, uint16 spell_id) {

	if (spells[spell_id].not_focusable)
		return 0;

	int16 realTotal = 0;
	int16 realTotal2 = 0;
	bool rand_effectiveness = false;

	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if(RuleB(Spells, LiveLikeFocusEffects) && (type == focusManaCost || type == focusImprovedHeal || type == focusImprovedDamage || type == focusImprovedDamage2))
		rand_effectiveness = true;

	if (RuleB(Spells, NPC_UseFocusFromItems) && itembonuses.FocusEffects[type]){

		const EQEmu::ItemData* TempItem = nullptr;
		const EQEmu::ItemData* UsedItem = nullptr;
		uint16 UsedFocusID = 0;
		int16 Total = 0;
		int16 focus_max = 0;
		int16 focus_max_real = 0;

		//item focus
		for (int i = 0; i < EQEmu::legacy::EQUIPMENT_SIZE; i++){
			const EQEmu::ItemData *cur = database.GetItem(equipment[i]);

			if(!cur)
				continue;

			TempItem = cur;

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
		}

		if(UsedItem && rand_effectiveness && focus_max_real != 0)
			realTotal = CalcFocusEffect(type, UsedFocusID, spell_id);
	}

	if (RuleB(Spells, NPC_UseFocusFromSpells) && spellbonuses.FocusEffects[type]){

		//Spell Focus
		int16 Total2 = 0;
		int16 focus_max2 = 0;
		int16 focus_max_real2 = 0;

		int buff_tracker = -1;
		int buff_slot = 0;
		uint16 focusspellid = 0;
		uint16 focusspell_tracker = 0;
		int buff_max = GetMaxTotalSlots();
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

	return realTotal + realTotal2;
}

void Mob::CheckNumHitsRemaining(NumHit type, int32 buff_slot, uint16 spell_id)
{
	/*
	Field 175 = numhits type
	1:  [Incoming Hit Attempts] (323=SE_DefensiveProc, 172=SE_AvoidMeleeChance, 1=SE_ArmorClass, 40=SE_DivineAura)
	2:  [Outgoing Hit Attempts]  (185=SE_DamageModifer, 184=SE_HitChance)
	3:  [Incoming Spells]  (180=SE_ResistSpellChance, 296=SE_FcSpellVulnerability) //Note: Determinetal spells only unless proven otherwise
	4:  [Outgoing Spells]
	5:  [Outgoing Hit Successes] (220=SE_SkillDamageAmount, 178=SE_MeleeLifetap, 121=SE_ReverseDS, ?373=SE_CastOnFadeEffectAlways)
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
	int buff_max = GetMaxTotalSlots();

	//Spell specific procs [Type 7,10,11]
	if (IsValidSpell(spell_id)) {
		for (int d = 0; d < buff_max; d++) {
			if (buffs[d].spellid == spell_id && buffs[d].numhits > 0 &&
			    spells[buffs[d].spellid].numhitstype == static_cast<int>(type)) {
				if (--buffs[d].numhits == 0) {
					CastOnNumHitFade(buffs[d].spellid);
					if (!TryFadeEffect(d))
						BuffFadeBySlot(d, true);
				} else if (IsClient()) { // still have numhits and client, update
					CastToClient()->SendBuffNumHitPacket(buffs[d], d);
				}
			}
		}
	} else if (type == NumHit::MatchingSpells) {
		if (buff_slot >= 0) {
			if (--buffs[buff_slot].numhits == 0) {
				CastOnNumHitFade(buffs[buff_slot].spellid);
				if (!TryFadeEffect(buff_slot))
					BuffFadeBySlot(buff_slot , true);
			} else if (IsClient()) { // still have numhits and client, update
				CastToClient()->SendBuffNumHitPacket(buffs[buff_slot], buff_slot);
			}
		} else {
			for (int d = 0; d < buff_max; d++) {
				if (!m_spellHitsLeft[d])
					continue;

				if (IsValidSpell(buffs[d].spellid) && m_spellHitsLeft[d] == buffs[d].spellid) {
					if (--buffs[d].numhits == 0) {
						CastOnNumHitFade(buffs[d].spellid);
						m_spellHitsLeft[d] = 0;
						if (!TryFadeEffect(d))
							BuffFadeBySlot(d, true);
					} else if (IsClient()) { // still have numhits and client, update
						CastToClient()->SendBuffNumHitPacket(buffs[d], d);
					}
				}
			}
		}
	} else {
		for (int d = 0; d < buff_max; d++) {
			if (IsValidSpell(buffs[d].spellid) && buffs[d].numhits > 0 &&
			    spells[buffs[d].spellid].numhitstype == static_cast<int>(type)) {
				if (--buffs[d].numhits == 0) {
					CastOnNumHitFade(buffs[d].spellid);
					if (!TryFadeEffect(d))
						BuffFadeBySlot(d, true);
				} else if (IsClient()) { // still have numhits and client, update
					CastToClient()->SendBuffNumHitPacket(buffs[d], d);
				}
			}
		}
	}
}

//for some stupid reason SK procs return theirs one base off...
uint16 Mob::GetProcID(uint16 spell_id, uint8 effect_index)
{
	if (!RuleB(Spells, SHDProcIDOffByOne)) // UF+ spell files
		return spells[spell_id].base[effect_index];

	// We should actually just be checking if the mob is SHD, but to not force
	// custom servers to create new spells, we will still do this
	bool sk = false;
	bool other = false;
	for (int x = 0; x < 16; x++) {
		if (x == 4) {
			if (spells[spell_id].classes[4] < 255)
				sk = true;
		} else {
			if (spells[spell_id].classes[x] < 255)
				other = true;
		}
	}

	if (sk && !other)
		return spells[spell_id].base[effect_index] + 1;
	else
		return spells[spell_id].base[effect_index];
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
	if (SuccessChance && zone->random.Roll(SuccessChance))
	{
		SetHP(1);

		int32 EffectsToTry[] =
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
		int32 UD_HealMod = 0;
		int HealAmt = 300; //Death Pact max Heal

		if(buffSlot >= 0){

			UD_HealMod = buffs[buffSlot].ExtraDIChance;

			SuccessChance = ( (GetCHA() * (RuleI(Spells, DeathSaveCharismaMod))) + 1) / 10; //(CHA Mod Default = 3)

			if (SuccessChance > 95)
				SuccessChance = 95;

			if(zone->random.Roll(SuccessChance)) {

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

				if(zone->random.Roll(SuccessChance)) {

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
	int buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++)
	{
		if (i == slot)
			continue;

		if (IsEffectInSpell(buffs[i].spellid, effect))
			return true;
	}
	return false;
}

float Mob::GetSympatheticProcChances(uint16 spell_id, int16 ProcRateMod, int32 ItemProcRate) {

	float ProcChance = 0.0f;
	int32 total_cast_time = 0;
	float cast_time_mod = 0.0f;
	ProcRateMod -= 100;


	if (spells[spell_id].recast_time >= spells[spell_id].recovery_time)
		total_cast_time = spells[spell_id].recast_time + spells[spell_id].cast_time;
	else
		total_cast_time = spells[spell_id].recovery_time + spells[spell_id].cast_time;

	if (total_cast_time > 0 && total_cast_time <= 2500)
		cast_time_mod = 0.25f;
	 else if (total_cast_time > 2500 && total_cast_time < 7000)
		 cast_time_mod = 0.167f*((static_cast<float>(total_cast_time) - 1000.0f)/1000.0f);
	 else
		 cast_time_mod = static_cast<float>(total_cast_time) / 7000.0f;

	ProcChance = (RuleR(Casting, AvgSpellProcsPerMinute)/100.0f) * (static_cast<float>(100.0f + ProcRateMod) / 10.0f)
		* cast_time_mod * (static_cast<float>(100.0f + ItemProcRate)/100.0f);

 	return ProcChance;
 }

int16 Mob::GetSympatheticSpellProcRate(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; i++){
		if (spells[spell_id].effectid[i] == SE_SympatheticProc)
			return spells[spell_id].base[i];
	}

	return 0;
}

uint16 Mob::GetSympatheticSpellProcID(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; i++){
		if (spells[spell_id].effectid[i] == SE_SympatheticProc)
			return spells[spell_id].base2[i];
	}

	return 0;
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
		int buff_count = GetMaxTotalSlots();
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
						CheckNumHitsRemaining(NumHit::MatchingSpells, i);
					}
				}

				else{
					int32 focus = caster->CalcFocusEffect(focusFcDamageAmtIncoming, buffs[i].spellid, spell_id);
					if(focus){
						dmg += focus;
						CheckNumHitsRemaining(NumHit::MatchingSpells, i);
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
				CheckNumHitsRemaining(NumHit::MatchingSpells, tmp_buffslot);
		}


	return value;
}

int32 Mob::ApplySpellEffectiveness(int16 spell_id, int32 value, bool IsBard, uint16 caster_id) {

	// 9-17-12: This is likely causing crashes, disabled till can resolve.
	if (IsBard)
		return value;

	Mob* caster = this;

	if (caster_id && caster_id != GetID())//Make sure we are checking the casters focus
		caster = entity_list.GetMob(caster_id);

	if (!caster)
		return value;

	int16 focus = caster->GetFocusEffect(focusFcBaseEffects, spell_id);

	if (IsBard)
		value += focus;
	else
		value += value*focus/100;

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

	if (!aabonuses.SEResist[1] && !spellbonuses.SEResist[1] && !itembonuses.SEResist[1])
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

	/*Live 5-20-14 Patch Note: Updated all spells which use Remove Detrimental and
	Cancel Beneficial spell effects to use a new method. The chances for those spells to
	affect their targets have not changed unless otherwise noted.*/

	/*This should provide a somewhat accurate conversion between pre 5/14 base values and post.
	until more information is avialble - Kayen*/
	if (level_modifier >= 100)
		level_modifier = level_modifier/100;

	//Dispels - Check level of caster agianst buffs level (level of the caster who cast the buff)
	//Effect value of dispels are treated as a level modifier.
	//Values for scaling were obtain from live parses, best estimates.

	caster_level += level_modifier - 1;
	int dispel_chance = 32; //Baseline chance if no level difference and no modifier
	int level_diff = caster_level - buff_level;

	if (level_diff > 0)
		dispel_chance += level_diff * 7;

	else if (level_diff < 0)
		dispel_chance += level_diff * 2;

	if (dispel_chance >= 100)
		return true;

	else if (dispel_chance < 10)
		dispel_chance = 10;

	if (zone->random.Roll(dispel_chance))
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
	Range 1			: UNKNOWN -- the spells with this seem to not have a restiction, true for now
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
	Range 127		: UNKNOWN
	Range 128		: pure melee -- guess
	Range 129		: pure caster -- guess
	Range 130		: hybrid -- guess
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
	Range 410 - 411 : UNKOWN -- examples are auras that cast on NPCs maybe in combat/out of combat?
	Range 500 - 599	: Heal if HP less than a specified value
	Range 600 - 699	: Limit to Body Type [base2 - 600 = Body]
	Range 700		: NPC only -- from patch notes "Wizard - Arcane Fusion no longer deals damage to non-NPC targets. This should ensure that wizards who fail their Bucolic Gambit are slightly less likely to annihilate themselves."
	Range 701		: NOT PET
	Range 800		: UKNOWN -- Target's Target Test (16598)
	Range 812		: UNKNOWN -- triggered by Thaumatize Owner
	Range 814		: UNKNOWN -- Vegetentacles
	Range 815		: UNKNOWN -- Pumpkin Pulp Splash
	Range 816		: UNKNOWN -- Rotten Fruit Splash
	Range 817		: UNKNOWN -- Tainted Bixie Pollen Splash
	Range 818 - 819 : If Undead/If Not Undead
	Range 820 - 822	: UKNOWN
	Range 835 		: Unknown *not implemented
	Range 836 -	837	: Progression Server / Live Server *not fully implemented
	Range 839 		: Progression Server and GoD released -- broken until Oct 21 2015 on live *not fully implemented
	Range 842 - 844 : Humaniod lv MAX ((842 - 800) * 2)
	Range 845 - 847	: UNKNOWN
	Range 860 - 871	: Humanoid lv MAX 860 = 90, 871 = 104 *not implemented
	Range 10000 - 11000	: Limit to Race [base2 - 10000 = Race] (*Not on live: Too useful a function to not implement)
	THIS IS A WORK IN PROGRESS
	*/

	if (value <= 0)
		return true;

	if (IsDamage || UseCastRestriction) {

		switch(value)
		{
			case 1:
				return true;

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

			case 700:
				if (IsNPC())
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

			case 836:
				return true; // todo implement progression flag assume not progression for now

			case 837:
				return false; // todo implement progression flag assume not progression for now

			case 839:
				return true; // todo implement progression flag assume not progression for now, this one is a check if GoD is live

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
			int count = hate_list.GetSummonedPetCountOnHateList(this);

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

bool Mob::TrySpellProjectile(Mob* spell_target,  uint16 spell_id, float speed){

	/*For mage 'Bolt' line and other various spells.
	-This is mostly accurate for how the modern clients handle this effect.
	-It was changed at some point to use an actual projectile as done here (opposed to a particle effect in classic)
	-There is no LOS check to prevent a bolt from being cast. If you don't have LOS your bolt simply goes into whatever barrier
	and you lose your mana. If there is LOS the bolt will lock onto your target and the damage is applied when it hits the target.
	-If your target moves the bolt moves with it in any direction or angle (consistent with other projectiles).
	-The way this is written once a bolt is cast a the distance from the initial cast to the target repeatedly
	check and if target is moving recalculates at what predicted time the bolt should hit that target in client_process
	When bolt hits its predicted point the damage is then done to target.
	Note: Projectile speed of 1 takes 3 seconds to go 100 distance units. Calculations are based on this constant.
	Live Bolt speed: Projectile speed of X takes 5 seconds to go 300 distance units.
	Pending Implementation: What this code can not do is prevent damage if the bolt hits a barrier after passing the initial LOS check
	because the target has moved while the bolt is in motion. (it is rare to actual get this to occur on live in normal game play)
	*/

	if (!spell_target)
		return false;

	uint8 anim = spells[spell_id].CastingAnim;
	int slot = -1;

	//Make sure there is an avialable bolt to be cast.
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].target_id == 0){
			slot = i;
			break;
		}
	}

	if (slot < 0)
		return false;

	if (CheckLosFN(spell_target)) {

		float speed_mod = speed; //Constant for adjusting speeds to match calculated impact time.
		float distance = spell_target->CalculateDistance(GetX(), GetY(), GetZ());
		float hit = 1200.0f + (10 * distance / speed_mod);

		ProjectileAtk[slot].increment = 1;
		ProjectileAtk[slot].hit_increment = static_cast<uint16>(hit); //This projected hit time if target does NOT MOVE
		ProjectileAtk[slot].target_id = spell_target->GetID();
		ProjectileAtk[slot].wpn_dmg = spell_id; //Store spell_id in weapon damage field
		ProjectileAtk[slot].origin_x = GetX();
		ProjectileAtk[slot].origin_y = GetY();
		ProjectileAtk[slot].origin_z = GetZ();
		ProjectileAtk[slot].skill = EQEmu::skills::SkillConjuration;
		ProjectileAtk[slot].speed_mod = speed_mod;

		SetProjectileAttack(true);
	}

	//This will use the correct graphic as defined in the player_1 field of spells_new table. Found in UF+ spell files.
	if (RuleB(Spells, UseLiveSpellProjectileGFX)) {
		ProjectileAnimation(spell_target,0, false, speed,0,0,0, spells[spell_id].player_1);
	}

	//This allows limited support for server using older spell files that do not contain data for bolt graphics.
	else {
		//Only use fire graphic for fire spells.
		if (spells[spell_id].resisttype == RESIST_FIRE) {

			if (IsClient()){
				if (CastToClient()->ClientVersionBit() <= 4) //Titanium needs alternate graphic.
					ProjectileAnimation(spell_target,(RuleI(Spells, FRProjectileItem_Titanium)), false, speed);
				else
					ProjectileAnimation(spell_target,(RuleI(Spells, FRProjectileItem_SOF)), false, speed);
				}

			else
				ProjectileAnimation(spell_target,(RuleI(Spells, FRProjectileItem_NPC)), false, speed);
		}
		//Default to an arrow if not using a mage bolt (Use up to date spell file and enable above rules for best results)
		else
			ProjectileAnimation(spell_target,0, 1, speed);
	}

	if (spells[spell_id].CastingAnim == 64)
		anim = 44; //Corrects for animation error.

	DoAnim(anim, 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells); //Override the default projectile animation.
	return true;
}

void Mob::ResourceTap(int32 damage, uint16 spellid)
{
	//'this' = caster
	if (!IsValidSpell(spellid))
		return;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (spells[spellid].effectid[i] == SE_ResourceTap) {
			damage = (damage * spells[spellid].base[i]) / 1000;
			
			if (damage) {
				if (spells[spellid].max[i] && (damage > spells[spellid].max[i]))
					damage = spells[spellid].max[i];
	
				if (spells[spellid].base2[i] == 0) { // HP Tap
					if (damage > 0)
						HealDamage(damage);
					else
						Damage(this, -damage, 0, EQEmu::skills::SkillEvocation, false);
				}
	
				if (spells[spellid].base2[i] == 1) // Mana Tap
					SetMana(GetMana() + damage);
	
				if (spells[spellid].base2[i] == 2 && IsClient()) // Endurance Tap
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() + damage);
			
			}
		}
	}
}

void Mob::TryTriggerThreshHold(int32 damage, int effect_id,  Mob* attacker){

	if (damage <= 0)
		return;

	if ((SE_TriggerMeleeThreshold == effect_id) && !spellbonuses.TriggerMeleeThreshold )
		return;
	else if ((SE_TriggerSpellThreshold == effect_id) && !spellbonuses.TriggerSpellThreshold)
		return;

	int buff_count = GetMaxTotalSlots();

	for(int slot = 0; slot < buff_count; slot++) {

		if(IsValidSpell(buffs[slot].spellid)){

			for(int i = 0; i < EFFECT_COUNT; i++){

				if (spells[buffs[slot].spellid].effectid[i] == effect_id){

					uint16 spell_id = spells[buffs[slot].spellid].base[i];

					if (damage > spells[buffs[slot].spellid].base2[i]){

						BuffFadeBySlot(slot);

						if (IsValidSpell(spell_id)) {

							if (IsBeneficialSpell(spell_id))
								SpellFinished(spell_id, this, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);

							else if(attacker)
								SpellFinished(spell_id, attacker, EQEmu::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);
						}
					}
				}
			}
		}
	}
}

bool Mob::CheckSpellCategory(uint16 spell_id, int category_id, int effect_id){

	if (!IsValidSpell(spell_id) || !category_id)
		return false;

	int effectid = 0;
	int category = 0;

	/*Category ID SE_LimitSpellClass [(+) Include (-) Exclude]
	1 = UNK
	2 = Cures
	3 = Offensive Spells
	4 = UNK
	5 = UNK
	6 = Lifetap
	*/

	/*Category ID SE_LimitSpellSubClass [(+) Include (-) Exclude]
	5 = UNK
	8 = UNK
	*/

	if (effect_id == SE_LimitSpellClass) {

		switch(category_id)
		{
			case 2:
			if (IsCureSpell(spell_id))
				return true;
			break;

			case 3:
			if (IsDetrimentalSpell(spell_id))
				return true;
			break;

			case 6:
			if (spells[spell_id].targettype == ST_Tap || spells[spell_id].targettype == ST_TargetAETap)
				return true;
			break;
		}
	}

	else if (effect_id == SE_LimitSpellSubclass) {
		//Pending Implementation when category types are figured out.
		return false;
	}

	return false;
}

void Mob::CalcSpellPowerDistanceMod(uint16 spell_id, float range, Mob* caster)
{
	if (IsPowerDistModSpell(spell_id)){

		float distance = 0;

		if (caster && !range)
			distance = caster->CalculateDistance(GetX(), GetY(), GetZ());
		else
			distance = sqrt(range);

		distance = EQEmu::Clamp(distance, spells[spell_id].min_dist, spells[spell_id].max_dist);

		float dm_range = spells[spell_id].max_dist - spells[spell_id].min_dist;
		float dm_mod_interval = spells[spell_id].max_dist_mod - spells[spell_id].min_dist_mod;
		float dist_from_min = distance - spells[spell_id].min_dist;
		float mod = spells[spell_id].min_dist_mod + (dist_from_min * (dm_mod_interval/dm_range));
		mod *= 100.0f;

		SetSpellPowerDistanceMod(static_cast<int>(mod));
	}
}

void Mob::BreakInvisibleSpells()
{
	if(invisible) {
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if(invisible_undead) {
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if(invisible_animals){
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}
}

void Client::BreakSneakWhenCastOn(Mob *caster, bool IsResisted)
{
	bool IsCastersTarget = false; // Chance to avoid only applies to AOE spells when not targeted.
	if (hidden || improved_hidden) {
		if (caster) {
			Mob *target = nullptr;
			target = caster->GetTarget();
			IsCastersTarget = target && target == this;
		}

		if (!IsCastersTarget) {
			int chance =
			    spellbonuses.NoBreakAESneak + itembonuses.NoBreakAESneak + aabonuses.NoBreakAESneak;

			if (IsResisted)
				chance *= 2;

			if (chance && zone->random.Roll(chance))
				return; // Do not drop Sneak/Hide
		}

		CancelSneakHide();
	}
}

void Client::BreakFeignDeathWhenCastOn(bool IsResisted)
{
	if(GetFeigned()){

		int chance = spellbonuses.FeignedCastOnChance + itembonuses.FeignedCastOnChance + aabonuses.FeignedCastOnChance;

		if (IsResisted)
			chance *= 2;

		if(chance && (zone->random.Roll(chance))){
			Message_StringID(MT_SpellFailure,FD_CAST_ON_NO_BREAK);
			return;
		}
	
		SetFeigned(false);
		Message_StringID(MT_SpellFailure,FD_CAST_ON);
	}
}
