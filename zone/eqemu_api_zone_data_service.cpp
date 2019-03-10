/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "eqemu_api_zone_data_service.h"
#include "npc.h"
#include "zone.h"
#include "entity.h"
#include <iostream>

extern Zone *zone;

void callGetNpcListDetail(Json::Value &response)
{
	auto &list = entity_list.GetNPCList();

	for (auto &iter : list) {
		auto        npc = iter.second;
		Json::Value row;

		/**
		 * Main
		 */
		row["id"]         = npc->GetID();
		row["clean_name"] = npc->GetCleanName();
		row["x"]          = npc->GetX();
		row["y"]          = npc->GetY();
		row["z"]          = npc->GetZ();
		row["heading"]    = npc->GetHeading();

		/**
		 * Rest
		 */
		row["accuracy_rating"]               = npc->GetAccuracyRating();
		row["ai_has_spells"]                 = npc->AI_HasSpells();
		row["ai_has_spells_effects"]         = npc->AI_HasSpellsEffects();
		row["ammo_id_file"]                  = npc->GetAmmoIDfile();
		row["attack_delay"]                  = npc->GetAttackDelay();
		row["attack_speed"]                  = npc->GetAttackSpeed();
		row["avoidance_rating"]              = npc->GetAvoidanceRating();
		row["base_damage"]                   = npc->GetBaseDamage();
		row["combat_event"]                  = npc->GetCombatEvent();
		row["copper"]                        = npc->GetCopper();
		row["count_loot"]                    = npc->CountLoot();
		row["drops_global_loot"]             = npc->DropsGlobalLoot();
		row["gold"]                          = npc->GetGold();
		row["grid"]                          = npc->GetGrid();
		row["has_private_corpse"]            = npc->HasPrivateCorpse();
		row["is_animal"]                     = npc->IsAnimal();
		row["is_guarding"]                   = npc->IsGuarding();
		row["is_ldon_locked"]                = npc->IsLDoNLocked();
		row["is_ldon_trap_detected"]         = npc->IsLDoNTrapDetected();
		row["is_ldon_trapped"]               = npc->IsLDoNTrapped();
		row["is_merchant_open"]              = npc->IsMerchantOpen();
		row["is_not_targetable_with_hotkey"] = npc->IsNotTargetableWithHotkey();
		row["is_proximity_set"]              = npc->IsProximitySet();
		row["is_taunting"]                   = npc->IsTaunting();
		row["ldon_locked_skill"]             = npc->GetLDoNLockedSkill();
		row["ldon_trap_spell_id"]            = npc->GetLDoNTrapSpellID();
		row["ldon_trap_type"]                = npc->GetLDoNTrapType();
		row["loottable_id"]                  = npc->GetLoottableID();
		row["max_dmg"]                       = npc->GetMaxDMG();
		row["max_wp"]                        = npc->GetMaxWp();
		row["min_damage"]                    = npc->GetMinDamage();
		row["min_dmg"]                       = npc->GetMinDMG();
		row["npc_spells_effects_id"]         = npc->GetNPCSpellsEffectsID();
		row["npc_spells_id"]                 = npc->GetNPCSpellsID();
		row["npchp_regen"]                   = npc->GetNPCHPRegen();
		row["num_merc_types"]                = npc->GetNumMercTypes();
		row["num_mercs"]                     = npc->GetNumMercs();
		row["number_of_attacks"]             = npc->GetNumberOfAttacks();
		row["pet_spell_id"]                  = npc->GetPetSpellID();
		row["platinum"]                      = npc->GetPlatinum();
		row["prim_skill"]                    = npc->GetPrimSkill();

		if (npc->IsProximitySet()) {
			row["proximity_max_x"] = npc->GetProximityMaxX();
			row["proximity_max_y"] = npc->GetProximityMaxY();
			row["proximity_max_z"] = npc->GetProximityMaxZ();
			row["proximity_min_x"] = npc->GetProximityMinX();
			row["proximity_min_y"] = npc->GetProximityMinY();
			row["proximity_min_z"] = npc->GetProximityMinZ();
		}

		row["ranged_skill"]    = npc->GetRangedSkill();
		row["raw_ac"]          = npc->GetRawAC();
		row["sec_skill"]       = npc->GetSecSkill();
		row["silver"]          = npc->GetSilver();
		row["slow_mitigation"] = npc->GetSlowMitigation();
		row["sp2"]             = npc->GetSp2();
		row["swarm_owner"]     = npc->GetSwarmOwner();
		row["swarm_target"]    = npc->GetSwarmTarget();
		row["waypoint_max"]    = npc->GetWaypointMax();
		row["will_aggro_npcs"] = npc->WillAggroNPCs();

		response.append(row);
	}
}

void callGetMobListDetail(Json::Value &response)
{
	auto &list = entity_list.GetMobList();

	for (auto &iter : list) {
		auto mob = iter.second;

		Json::Value row;

		/**
		 * Main
		 */
		row["id"]         = mob->GetID();
		row["clean_name"] = mob->GetCleanName();
		row["x"]          = mob->GetX();
		row["y"]          = mob->GetY();
		row["z"]          = mob->GetZ();
		row["heading"]    = mob->GetHeading();

		/**
		 * Rest
		 */
		row["ac"]                         = mob->GetAC();
		row["ac_softcap"]                 = mob->GetACSoftcap();
		row["ac_sum"]                     = mob->ACSum();
		row["active_light_type"]          = mob->GetActiveLightType();
		row["aggro_range"]                = mob->GetAggroRange();
		row["allow_beneficial"]           = mob->GetAllowBeneficial();
		row["animation"]                  = mob->GetAnimation();
		row["assist_range"]               = mob->GetAssistRange();
		row["aura_slots"]                 = mob->GetAuraSlots();
		row["base_fear_speed"]            = mob->GetBaseFearSpeed();
		row["base_runspeed"]              = mob->GetBaseRunspeed();
		row["base_size"]                  = mob->GetBaseSize();
		row["base_walkspeed"]             = mob->GetBaseWalkspeed();
		row["beard"]                      = mob->GetBeard();
		row["beard_color"]                = mob->GetBeardColor();
		row["best_melee_skill"]           = mob->GetBestMeleeSkill();
		row["calc_fear_resist_chance"]    = mob->CalcFearResistChance();
		row["calc_resist_chance_bonus"]   = mob->CalcResistChanceBonus();
		row["can_block_spell"]            = mob->CanBlockSpell();
		row["can_facestab"]               = mob->CanFacestab();
		row["casted_spell_inv_slot"]      = mob->GetCastedSpellInvSlot();
		row["casting_spell_id"]           = mob->CastingSpellID();
		row["charmed"]                    = mob->Charmed();
		row["check_last_los_state"]       = mob->CheckLastLosState();
		row["class"]                      = mob->GetClass();
		row["class_level_factor"]         = mob->GetClassLevelFactor();
		row["class_race_ac_bonus"]        = mob->GetClassRaceACBonus();
		row["compute_defense"]            = mob->compute_defense();
		row["count_dispellable_buffs"]    = mob->CountDispellableBuffs();
		row["cripp_blow_chance"]          = mob->GetCrippBlowChance();
		row["cur_wp"]                     = mob->GetCurWp();
		row["cwp"]                        = mob->GetCWP();
		row["cwpp"]                       = mob->GetCWPP();
		row["divine_aura"]                = mob->DivineAura();
		row["do_casting_checks"]          = mob->DoCastingChecks();
		row["dont_buff_me_before"]        = mob->DontBuffMeBefore();
		row["dont_cure_me_before"]        = mob->DontCureMeBefore();
		row["dont_dot_me_before"]         = mob->DontDotMeBefore();
		row["dont_heal_me_before"]        = mob->DontHealMeBefore();
		row["dont_root_me_before"]        = mob->DontRootMeBefore();
		row["dont_snare_me_before"]       = mob->DontSnareMeBefore();
		row["drakkin_details"]            = mob->GetDrakkinDetails();
		row["drakkin_heritage"]           = mob->GetDrakkinHeritage();
		row["drakkin_tattoo"]             = mob->GetDrakkinTattoo();
		row["emote_id"]                   = mob->GetEmoteID();
		row["equipment_light_type"]       = mob->GetEquipmentLightType();
		row["eye_color1"]                 = mob->GetEyeColor1();
		row["eye_color2"]                 = mob->GetEyeColor2();
		row["fear_speed"]                 = mob->GetFearSpeed();
		row["flurry_chance"]              = mob->GetFlurryChance();
		row["follow_can_run"]             = mob->GetFollowCanRun();
		row["follow_distance"]            = mob->GetFollowDistance();
		row["follow_id"]                  = mob->GetFollowID();
		row["gender"]                     = mob->GetGender();
		row["hair_color"]                 = mob->GetHairColor();
		row["hair_style"]                 = mob->GetHairStyle();
		row["has_active_song"]            = mob->HasActiveSong();
		row["has_assist_aggro"]           = mob->HasAssistAggro();
		row["has_died"]                   = mob->HasDied();
		row["has_disc_buff"]              = mob->HasDiscBuff();
		row["has_endur_upkeep"]           = mob->HasEndurUpkeep();
		row["has_free_aura_slots"]        = mob->HasFreeAuraSlots();
		row["has_free_trap_slots"]        = mob->HasFreeTrapSlots();
		row["has_mgb"]                    = mob->HasMGB();
		row["has_numhits"]                = mob->HasNumhits();
		row["has_pet"]                    = mob->HasPet();
		row["has_pet_affinity"]           = mob->HasPetAffinity();
		row["has_primary_aggro"]          = mob->HasPrimaryAggro();
		row["has_project_illusion"]       = mob->HasProjectIllusion();
		row["has_projectile_attack"]      = mob->HasProjectileAttack();
		row["has_shield_equiped"]         = mob->HasShieldEquiped();
		row["has_special_abilities"]      = mob->HasSpecialAbilities();
		row["has_tar_reflection"]         = mob->HasTargetReflection();
		row["has_temp_pets_active"]       = mob->HasTempPetsActive();
		row["has_two_hand_blunt_equiped"] = mob->HasTwoHandBluntEquiped();
		row["has_two_hander_equipped"]    = mob->HasTwoHanderEquipped();
		row["has_virus"]                  = mob->HasVirus();
		row["hate_summon"]                = mob->HateSummon();
		row["helm_texture"]               = mob->GetHelmTexture();
		row["hp"]                         = mob->GetHP();
		row["improved_taunt"]             = mob->ImprovedTaunt();
		row["innate_light_type"]          = mob->GetInnateLightType();
		row["is_ai_controlled"]           = mob->IsAIControlled();
		row["is_amnesiad"]                = mob->IsAmnesiad();
		row["is_animation"]               = mob->IsAnimation();
		row["is_blind"]                   = mob->IsBlind();
		row["is_casting"]                 = mob->IsCasting();
		row["is_charmed"]                 = mob->IsCharmed();
		row["is_destructible_object"]     = mob->IsDestructibleObject();
		row["is_engaged"]                 = mob->IsEngaged();
		row["is_enraged"]                 = mob->IsEnraged();
		row["is_familiar"]                = mob->IsFamiliar();
		row["is_feared"]                  = mob->IsFeared();
		row["is_findable"]                = mob->IsFindable();
		row["is_focused"]                 = mob->IsFocused();
		row["is_g_held"]                  = mob->IsGHeld();
		row["is_grouped"]                 = mob->IsGrouped();
		row["is_held"]                    = mob->IsHeld();
		row["is_looting"]                 = mob->IsLooting();
		row["is_melee_disabled"]          = mob->IsMeleeDisabled();
		row["is_mezzed"]                  = mob->IsMezzed();
		row["is_moved"]                   = mob->IsMoved();
		row["is_moving"]                  = mob->IsMoving();
		row["is_no_cast"]                 = mob->IsNoCast();
		row["is_off_hand_atk"]            = mob->IsOffHandAtk();
		row["is_pet_owner_client"]        = mob->IsPetOwnerClient();
		row["is_pet_regroup"]             = mob->IsPetRegroup();
		row["is_pet_stop"]                = mob->IsPetStop();
		row["is_pseudo_rooted"]           = mob->IsPseudoRooted();
		row["is_raid_grouped"]            = mob->IsRaidGrouped();
		row["is_rare_spawn"]              = mob->IsRareSpawn();
		row["is_roamer"]                  = mob->IsRoamer();
		row["is_rooted"]                  = mob->IsRooted();
		row["is_running"]                 = mob->IsRunning();
		row["is_silenced"]                = mob->IsSilenced();
		row["is_stunned"]                 = mob->IsStunned();
		row["is_tar_lock_pet"]            = mob->IsTargetLockPet();
		row["is_tarable"]                 = mob->IsTargetable();
		row["is_tared"]                   = mob->IsTargeted();
		row["is_temp_pet"]                = mob->IsTempPet();
		row["is_trackable"]               = mob->IsTrackable();
		row["item_hp_bonuses"]            = mob->GetItemHPBonuses();
		row["last_name"]                  = mob->GetLastName();
		row["level"]                      = mob->GetLevel();
		row["luclin_face"]                = mob->GetLuclinFace();
		row["mana"]                       = mob->GetMana();
		row["mana_percent"]               = mob->GetManaPercent();
		row["mana_ratio"]                 = mob->GetManaRatio();
		row["max_hp"]                     = mob->GetMaxHP();
		row["max_mana"]                   = mob->GetMaxMana();
		row["melee_mitigation"]           = mob->GetMeleeMitigation();
		row["mitigation_ac"]              = mob->GetMitigationAC();
		row["movespeed"]                  = mob->GetMovespeed();
		row["name"]                       = mob->GetName();
		row["next_hp_event"]              = mob->GetNextHPEvent();
		row["next_inc_hp_event"]          = mob->GetNextIncHPEvent();
		row["npc_assist_cap"]             = mob->NPCAssistCap();
		row["npc_type_id"]                = mob->GetNPCTypeID();
		row["orig_level"]                 = mob->GetOrigLevel();
		row["orig_name"]                  = mob->GetOrigName();
		row["owner_id"]                   = mob->GetOwnerID();
		row["pet_id"]                     = mob->GetPetID();
		row["pet_power"]                  = mob->GetPetPower();
		row["pet_tar_lock_id"]            = mob->GetPetTargetLockID();
		row["qglobal"]                    = mob->GetQglobal();
		row["race"]                       = mob->GetRace();
		row["run_anim_speed"]             = mob->GetRunAnimSpeed();
		row["sanctuary"]                  = mob->Sanctuary();
		row["see_hide"]                   = mob->SeeHide();
		row["see_improved_hide"]          = mob->SeeImprovedHide();
		row["see_invisible"]              = mob->SeeInvisible();
		row["see_invisible_undead"]       = mob->SeeInvisibleUndead();
		row["size"]                       = mob->GetSize();
		row["slow_mitigation"]            = mob->GetSlowMitigation();
		row["snared_amount"]              = mob->GetSnaredAmount();
		row["spawned"]                    = mob->Spawned();
		row["spell_hp_bonuses"]           = mob->GetSpellHPBonuses();
		row["spell_light_type"]           = mob->GetSpellLightType();
		row["spell_power_distance_mod"]   = mob->GetSpellPowerDistanceMod();
		row["spell_x"]                    = mob->GetSpellX();
		row["spell_y"]                    = mob->GetSpellY();
		row["spell_z"]                    = mob->GetSpellZ();
		row["tar_ring_x"]                 = mob->GetTargetRingX();
		row["tar_ring_y"]                 = mob->GetTargetRingY();
		row["tar_ring_z"]                 = mob->GetTargetRingZ();
		row["temp_pet_count"]             = mob->GetTempPetCount();
		row["texture"]                    = mob->GetTexture();
		row["trap_slots"]                 = mob->GetTrapSlots();
		row["try_death_save"]             = mob->TryDeathSave();
		row["try_divine_save"]            = mob->TryDivineSave();
		row["try_spell_on_death"]         = mob->TrySpellOnDeath();
		row["update_active_light"]        = mob->UpdateActiveLight();
		row["wander_type"]                = mob->GetWanderType();

		response.append(row);
	}
}

void callGetZoneAttributes(Json::Value &response)
{
	Json::Value row;

	row["aggro_limit_reached"]     = zone->AggroLimitReached();
	row["allow_mercs"]             = zone->AllowMercs();
	row["buff_timers_suspended"]   = zone->BuffTimersSuspended();
	row["can_bind"]                = zone->CanBind();
	row["can_cast_outdoor"]        = zone->CanCastOutdoor();
	row["can_do_combat"]           = zone->CanDoCombat();
	row["can_levitate"]            = zone->CanLevitate();
	row["count_auth"]              = zone->CountAuth();
	row["count_spawn2"]            = zone->CountSpawn2();
	row["file_name"]               = zone->GetFileName();
	row["graveyard_id"]            = zone->graveyard_id();
	row["graveyard_zoneid"]        = zone->graveyard_zoneid();
	row["has_graveyard"]           = zone->HasGraveyard();
	row["has_map"]                 = zone->HasMap();
	row["has_water_map"]           = zone->HasWaterMap();
	row["has_weather"]             = zone->HasWeather();
	row["instance_id"]             = zone->GetInstanceID();
	row["instance_version"]        = zone->GetInstanceVersion();
	row["instant_grids"]           = zone->InstantGrids();
	row["is_city"]                 = zone->IsCity();
	row["is_hotzone"]              = zone->IsHotzone();
	row["is_instance_persistent"]  = zone->IsInstancePersistent();
	row["is_pvp_zone"]             = zone->IsPVPZone();
	row["is_static_zone"]          = zone->IsStaticZone();
	row["is_ucs_server_available"] = zone->IsUCSServerAvailable();
	row["long_name"]               = zone->GetLongName();
	row["max_clients"]             = zone->GetMaxClients();
	row["mobs_aggro_count"]        = zone->MobsAggroCount();
	row["save_zone_cfg"]           = zone->SaveZoneCFG();
	row["short_name"]              = zone->GetShortName();
	row["total_blocked_spells"]    = zone->GetTotalBlockedSpells();
	row["zone_id"]                 = zone->GetZoneID();
	row["zone_type"]               = zone->GetZoneType();

	response.append(row);
}

void EQEmuApiZoneDataService::get(Json::Value &response, const std::vector<std::string> &args)
{
	std::string method = args[0];

	if (method == "get_npc_list_detail") {
		callGetNpcListDetail(response);
	}
	if (method == "get_mob_list_detail") {
		callGetMobListDetail(response);
	}
	if (method == "get_zone_attributes") {
		callGetZoneAttributes(response);
	}
}
