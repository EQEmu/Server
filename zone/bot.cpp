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

#include "bot.h"
#include "object.h"
#include "doors.h"
#include "quest_parser_collection.h"
#include "lua_parser.h"
#include "../common/strings.h"
#include "../common/say_link.h"
#include "../common/repositories/bot_spell_settings_repository.h"
#include "../common/data_verification.h"

extern volatile bool is_zone_loaded;

// This constructor is used during the bot create command
Bot::Bot(NPCType *npcTypeData, Client* botOwner) : NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), ping_timer(1) {
	GiveNPCTypeData(npcTypeData);

	if(botOwner) {
		SetBotOwner(botOwner);
		_botOwnerCharacterID = botOwner->CharacterID();
	} else {
		SetBotOwner(0);
		_botOwnerCharacterID = 0;
	}

	m_inv.SetInventoryVersion(EQ::versions::MobVersion::Bot);
	m_inv.SetGMInventory(false); // bot expansions are not currently implemented (defaults to static)

	_guildRank = 0;
	_guildId = 0;
	_lastTotalPlayTime = 0;
	_startTotalPlayTime = time(&_startTotalPlayTime);
	_lastZoneId = 0;
	_baseMR = npcTypeData->MR;
	_baseCR = npcTypeData->CR;
	_baseDR = npcTypeData->DR;
	_baseFR = npcTypeData->FR;
	_basePR = npcTypeData->PR;
	_baseCorrup = npcTypeData->Corrup;
	_baseAC = npcTypeData->AC;
	_baseSTR = npcTypeData->STR;
	_baseSTA = npcTypeData->STA;
	_baseDEX = npcTypeData->DEX;
	_baseAGI = npcTypeData->AGI;
	_baseINT = npcTypeData->INT;
	_baseWIS = npcTypeData->WIS;
	_baseCHA = npcTypeData->CHA;
	_baseATK = npcTypeData->ATK;
	_baseRace = npcTypeData->race;
	_baseGender = npcTypeData->gender;
	RestRegenHP = 0;
	RestRegenMana = 0;
	RestRegenEndurance = 0;
	m_enforce_spell_settings = 0;
	m_bot_archery_setting = 0;
	m_expansion_bitmask = -1;
	SetBotID(0);
	SetBotSpellID(0);
	SetSpawnStatus(false);
	SetBotCharmer(false);
	SetPetChooser(false);
	SetRangerAutoWeaponSelect(false);
	SetTaunting(GetClass() == WARRIOR);
	SetDefaultBotStance();

	SetAltOutOfCombatBehavior(GetClass() == BARD); // will need to be updated if more classes make use of this flag
	SetShowHelm(true);
	SetPauseAI(false);

	m_alt_combat_hate_timer.Start(250);
	m_auto_defend_timer.Disable();
	//m_combat_jitter_timer.Disable();
	//SetCombatJitterFlag(false);
	SetGuardFlag(false);
	SetHoldFlag(false);
	SetAttackFlag(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	m_previous_pet_order = SPO_Guard;

	rest_timer.Disable();
	ping_timer.Disable();
	SetFollowDistance(BOT_FOLLOW_DISTANCE_DEFAULT);
	if (IsCasterClass(GetClass()))
		SetStopMeleeLevel((uint8)RuleI(Bots, CasterStopMeleeLevel));
	else
		SetStopMeleeLevel(255);

	// Do this once and only in this constructor
	GenerateAppearance();
	GenerateBaseStats();
	// Calculate HitPoints Last As It Uses Base Stats
	current_hp = GenerateBaseHitPoints();
	current_mana = GenerateBaseManaPoints();
	cur_end = CalcBaseEndurance();
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
	for (int i = 0; i < MaxTimer; i++)
		timers[i] = 0;

	strcpy(name, GetCleanName());
	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
}

// This constructor is used when the bot is loaded out of the database
Bot::Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, double totalPlayTime, uint32 lastZoneId, NPCType *npcTypeData)
	: NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), ping_timer(1)
{
	GiveNPCTypeData(npcTypeData);

	_botOwnerCharacterID = botOwnerCharacterID;
	if(_botOwnerCharacterID > 0)
		SetBotOwner(entity_list.GetClientByCharID(_botOwnerCharacterID));

	auto bot_owner = GetBotOwner();

	m_inv.SetInventoryVersion(EQ::versions::MobVersion::Bot);
	m_inv.SetGMInventory(false); // bot expansions are not currently implemented (defaults to static)

	_guildRank = 0;
	_guildId = 0;
	_lastTotalPlayTime = totalPlayTime;
	_startTotalPlayTime = time(&_startTotalPlayTime);
	_lastZoneId = lastZoneId;
	berserk = false;
	_baseMR = npcTypeData->MR;
	_baseCR = npcTypeData->CR;
	_baseDR = npcTypeData->DR;
	_baseFR = npcTypeData->FR;
	_basePR = npcTypeData->PR;
	_baseCorrup = npcTypeData->Corrup;
	_baseAC = npcTypeData->AC;
	_baseSTR = npcTypeData->STR;
	_baseSTA = npcTypeData->STA;
	_baseDEX = npcTypeData->DEX;
	_baseAGI = npcTypeData->AGI;
	_baseINT = npcTypeData->INT;
	_baseWIS = npcTypeData->WIS;
	_baseCHA = npcTypeData->CHA;
	_baseATK = npcTypeData->ATK;
	_baseRace = npcTypeData->race;
	_baseGender = npcTypeData->gender;
	current_hp = npcTypeData->current_hp;
	current_mana = npcTypeData->Mana;
	RestRegenHP = 0;
	RestRegenMana = 0;
	RestRegenEndurance = 0;
	SetBotID(botID);
	SetBotSpellID(botSpellsID);
	SetSpawnStatus(false);
	SetBotCharmer(false);
	SetPetChooser(false);
	SetRangerAutoWeaponSelect(false);

	bool stance_flag = false;
	if (!database.botdb.LoadStance(this, stance_flag) && bot_owner) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Failed to load stance for '{}'.",
				GetCleanName()
			).c_str()
		);
	}

	if (!stance_flag && bot_owner) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Could not locate stance for '{}'.",
				GetCleanName()
			).c_str()
		);
	}

	SetTaunting((GetClass() == WARRIOR || GetClass() == PALADIN || GetClass() == SHADOWKNIGHT) && (GetBotStance() == EQ::constants::stanceAggressive));
	SetPauseAI(false);

	m_alt_combat_hate_timer.Start(250);
	m_auto_defend_timer.Disable();
	//m_combat_jitter_timer.Disable();
	//SetCombatJitterFlag(false);
	SetGuardFlag(false);
	SetHoldFlag(false);
	SetAttackFlag(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	m_previous_pet_order = SPO_Guard;

	rest_timer.Disable();
	ping_timer.Disable();
	SetFollowDistance(BOT_FOLLOW_DISTANCE_DEFAULT);
	if (IsCasterClass(GetClass()))
		SetStopMeleeLevel((uint8)RuleI(Bots, CasterStopMeleeLevel));
	else
		SetStopMeleeLevel(255);

	strcpy(name, GetCleanName());

	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
	if (!database.botdb.LoadInspectMessage(GetBotID(), _botInspectMessage) && bot_owner)
		bot_owner->Message(Chat::White, "%s for '%s'", BotDatabase::fail::LoadInspectMessage(), GetCleanName());

	std::string error_message;

	EquipBot(&error_message);
	if(!error_message.empty()) {
		if(bot_owner)
			bot_owner->Message(Chat::White, error_message.c_str());
		error_message.clear();
	}

	for (int i = 0; i < MaxTimer; i++)
		timers[i] = 0;

	if (GetClass() == ROGUE) {
		m_evade_timer.Start();
	}

	m_CastingRoles.GroupHealer = false;
	m_CastingRoles.GroupSlower = false;
	m_CastingRoles.GroupNuker = false;
	m_CastingRoles.GroupDoter = false;

	GenerateBaseStats();

	if (!database.botdb.LoadTimers(this) && bot_owner)
		bot_owner->Message(Chat::White, "%s for '%s'", BotDatabase::fail::LoadTimers(), GetCleanName());

	LoadAAs();

	// copied from client CompleteConnect() handler - watch for problems
	// (may have to move to post-spawn location if certain buffs still don't process correctly)
	if (database.botdb.LoadBuffs(this) && bot_owner) {

		//reapply some buffs
		uint32 buff_count = GetMaxTotalSlots();
		for (uint32 j1 = 0; j1 < buff_count; j1++) {
			if (!IsValidSpell(buffs[j1].spellid)) {
				continue;
			}

			const SPDat_Spell_Struct& spell = spells[buffs[j1].spellid];

			int NimbusEffect = GetNimbusEffect(buffs[j1].spellid);
			if (NimbusEffect) {
				if (!IsNimbusEffectActive(NimbusEffect)) {
					SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
				}
			}

			for (int x1 = 0; x1 < EFFECT_COUNT; x1++) {
				switch (spell.effect_id[x1]) {
				case SE_IllusionCopy:
				case SE_Illusion: {
					if (spell.base_value[x1] == -1) {
						if (gender == 1)
							gender = 0;
						else if (gender == 0)
							gender = 1;
						SendIllusionPacket(GetRace(), gender, 0xFF, 0xFF);
					}
					else if (spell.base_value[x1] == -2) // WTF IS THIS
					{
						if (GetRace() == IKSAR || GetRace() == VAHSHIR || GetRace() <= GNOME) {
							SendIllusionPacket(GetRace(), GetGender(), spell.limit_value[x1], spell.max_value[x1]);
						}
					}
					else if (spell.max_value[x1] > 0)
					{
						SendIllusionPacket(spell.base_value[x1], 0xFF, spell.limit_value[x1], spell.max_value[x1]);
					}
					else
					{
						SendIllusionPacket(spell.base_value[x1], 0xFF, 0xFF, 0xFF);
					}
					switch (spell.base_value[x1]) {
					case OGRE:
						SendAppearancePacket(AT_Size, 9);
						break;
					case TROLL:
						SendAppearancePacket(AT_Size, 8);
						break;
					case VAHSHIR:
					case BARBARIAN:
						SendAppearancePacket(AT_Size, 7);
						break;
					case HALF_ELF:
					case WOOD_ELF:
					case DARK_ELF:
					case FROGLOK:
						SendAppearancePacket(AT_Size, 5);
						break;
					case DWARF:
						SendAppearancePacket(AT_Size, 4);
						break;
					case HALFLING:
					case GNOME:
						SendAppearancePacket(AT_Size, 3);
						break;
					default:
						SendAppearancePacket(AT_Size, 6);
						break;
					}
					break;
				}
				//case SE_SummonHorse: {
				//	SummonHorse(buffs[j1].spellid);
				//	//hasmount = true;	//this was false, is that the correct thing?
				//	break;
				//}
				case SE_Silence:
				{
					Silence(true);
					break;
				}
				case SE_Amnesia:
				{
					Amnesia(true);
					break;
				}
				case SE_DivineAura:
				{
					invulnerable = true;
					break;
				}
				case SE_Invisibility2:
				case SE_Invisibility:
				{
					invisible = true;
					SendAppearancePacket(AT_Invis, 1);
					break;
				}
				case SE_Levitate:
				{
					if (!zone->CanLevitate())
					{
						//if (!GetGM())
						//{
							SendAppearancePacket(AT_Levitate, 0);
							BuffFadeByEffect(SE_Levitate);
							//Message(Chat::White, "You can't levitate in this zone.");
						//}
					}
					else {
						SendAppearancePacket(AT_Levitate, 2);
					}
					break;
				}
				case SE_InvisVsUndead2:
				case SE_InvisVsUndead:
				{
					invisible_undead = true;
					break;
				}
				case SE_InvisVsAnimals:
				{
					invisible_animals = true;
					break;
				}
				case SE_AddMeleeProc:
				case SE_WeaponProc:
				{
					AddProcToWeapon(GetProcID(buffs[j1].spellid, x1), false, 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, buffs[j1].casterlevel);
					break;
				}
				case SE_DefensiveProc:
				{
					AddDefensiveProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid);
					break;
				}
				case SE_RangedProc:
				{
					AddRangedProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid);
					break;
				}
				}
			}
		}
	}
	else {
		bot_owner->Message(Chat::White, "&s for '%s'", BotDatabase::fail::LoadBuffs(), GetCleanName());
	}

	CalcBotStats(false);
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
	if(current_hp > max_hp)
		current_hp = max_hp;

	if(current_hp <= 0) {
		if (RuleB(Spells, BuffsFadeOnDeath)) {
			BuffFadeNonPersistDeath();
		}

		if (RuleB(Bots, ResurrectionSickness)) {
			int resurrection_sickness_spell_id = (
				RuleB(Bots, OldRaceRezEffects) &&
				(
					GetRace() == BARBARIAN ||
					GetRace() == DWARF ||
					GetRace() == TROLL ||
					GetRace() == OGRE
				) ?
				RuleI(Bots, OldResurrectionSicknessSpell) :
				RuleI(Bots, ResurrectionSicknessSpell)
			);
			SetHP(max_hp / 5);
			SetMana(0);
			SpellOnTarget(resurrection_sickness_spell_id, this); // Rezz effects
		} else {
			SetHP(GetMaxHP());
			SetMana(GetMaxMana());
		}
	}

	if(current_mana > max_mana)
		current_mana = max_mana;

	cur_end = max_end;
}

Bot::~Bot() {
	AI_Stop();
	LeaveHealRotationMemberPool();

	if(HasGroup())
		Bot::RemoveBotFromGroup(this, GetGroup());

	if(HasPet())
		GetPet()->Depop();

	entity_list.RemoveBot(GetID());
}

void Bot::SetBotID(uint32 botID) {
	_botID = botID;
	npctype_id = botID;
}

void Bot::SetBotSpellID(uint32 newSpellID) {
	npc_spells_id = newSpellID;
}

void  Bot::SetSurname(std::string bot_surname) {

	_surname = bot_surname.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
		GMLastName_Struct* gmn = (GMLastName_Struct*)outapp->pBuffer;

		strcpy(gmn->name, GetCleanName());
		strcpy(gmn->gmname, GetCleanName());
		strcpy(gmn->lastname, GetSurname().c_str());
		gmn->unknown[0] = 1;
		gmn->unknown[1] = 1;
		gmn->unknown[2] = 1;
		gmn->unknown[3] = 1;

		entity_list.QueueClients(this, outapp);
		safe_delete(outapp);
	}
}

void  Bot::SetTitle(std::string bot_title) {

	_title = bot_title.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
		SetTitleReply_Struct* strs = (SetTitleReply_Struct*)outapp->pBuffer;

		strs->is_suffix = 0;
		strn0cpy(strs->title, _title.c_str(), sizeof(strs->title));
		strs->entity_id = GetID();

		entity_list.QueueClients(this, outapp, false);
		safe_delete(outapp);
	}
}

void  Bot::SetSuffix(std::string bot_suffix) {

	_suffix = bot_suffix.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
		SetTitleReply_Struct* strs = (SetTitleReply_Struct*)outapp->pBuffer;

		strs->is_suffix = 1;
		strn0cpy(strs->title, _suffix.c_str(), sizeof(strs->title));
		strs->entity_id = GetID();

		entity_list.QueueClients(this, outapp, false);
		safe_delete(outapp);
	}
}

uint32 Bot::GetBotArcheryRange() {
	const EQ::ItemInstance *range_inst = GetBotItem(EQ::invslot::slotRange);
	const EQ::ItemInstance *ammo_inst = GetBotItem(EQ::invslot::slotAmmo);
	if (!range_inst || !ammo_inst)
		return 0;

	const EQ::ItemData *range_item = range_inst->GetItem();
	const EQ::ItemData *ammo_item = ammo_inst->GetItem();
	if (!range_item || !ammo_item || range_item->ItemType != EQ::item::ItemTypeBow || ammo_item->ItemType != EQ::item::ItemTypeArrow)
		return 0;

	// everything is good!
	return (range_item->Range + ammo_item->Range);
}

void Bot::ChangeBotArcherWeapons(bool isArcher) {
	if((GetClass()==WARRIOR) || (GetClass()==PALADIN) || (GetClass()==RANGER) || (GetClass()==SHADOWKNIGHT) || (GetClass()==ROGUE)) {
		if(!isArcher) {
			BotAddEquipItem(EQ::invslot::slotPrimary, GetBotItemBySlot(EQ::invslot::slotPrimary));
			BotAddEquipItem(EQ::invslot::slotSecondary, GetBotItemBySlot(EQ::invslot::slotSecondary));
			SetAttackTimer();
			BotGroupSay(this, "My blade is ready");
		} else {
			BotRemoveEquipItem(EQ::invslot::slotPrimary);
			BotRemoveEquipItem(EQ::invslot::slotSecondary);
			BotAddEquipItem(EQ::invslot::slotAmmo, GetBotItemBySlot(EQ::invslot::slotAmmo));
			BotAddEquipItem(EQ::invslot::slotSecondary, GetBotItemBySlot(EQ::invslot::slotRange));
			SetAttackTimer();
			BotGroupSay(this, "My bow is true and ready");
		}
	}
	else
		BotGroupSay(this, "I don't know how to use a bow");
}

void Bot::Sit() {
	if(IsMoving()) {
		moved = false;
		StopNavigation();
	}

	SetAppearance(eaSitting);
}

void Bot::Stand() {
	SetAppearance(eaStanding);
}

bool Bot::IsSitting() {
	bool result = false;
	if(GetAppearance() == eaSitting && !IsMoving())
		result = true;

	return result;
}

bool Bot::IsStanding() {
	bool result = false;
	if(GetAppearance() == eaStanding)
		result = true;

	return result;
}

NPCType *Bot::FillNPCTypeStruct(
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
) {
	auto n = new NPCType{ 0 };

	strn0cpy(n->name, botName.c_str(), sizeof(n->name));
	strn0cpy(n->lastname, botLastName.c_str(), sizeof(n->lastname));

	n->current_hp = hp;
	n->max_hp = hp;
	n->size = size;
	n->runspeed = 0.7f;
	n->gender = gender;
	n->race = botRace;
	n->class_ = botClass;
	n->bodytype = 1;
	n->deity = EQ::deity::DeityAgnostic;
	n->level = botLevel;
	//n->npc_id = 0;
	//n->texture = 0;
	//n->helmtexture = 0;
	//n->herosforgemodel = 0;
	//n->loottable_id = 0;
	n->npc_spells_id = botSpellsID;
	//n->npc_spells_effects_id = 0;
	//n->npc_faction_id = 0;
	//n->merchanttype = 0;
	//n->alt_currency_type = 0;
	//n->adventure_template = 0;
	//n->trap_template = 0;
	//n->light = 0;
	n->AC = ac;
	n->Mana = mana;
	n->ATK = attack;
	n->STR = str;
	n->STA = sta;
	n->DEX = dex;
	n->AGI = agi;
	n->INT = _int;
	n->WIS = wis;
	n->CHA = cha;
	n->MR = mr;
	n->FR = fr;
	n->CR = cr;
	n->PR = pr;
	n->DR = dr;
	n->Corrup = corrup;
	//n->PhR = 0;
	n->haircolor = hairColor;
	n->beardcolor = beardColor;
	n->eyecolor1 = eyeColor;
	n->eyecolor2 = eyeColor2;
	n->hairstyle = hairStyle;
	n->luclinface = face;
	n->beard = beard;
	n->drakkin_heritage = drakkinHeritage;
	n->drakkin_tattoo = drakkinTattoo;
	n->drakkin_details = drakkinDetails;
	//n->armor_tint = { 0 };
	//n->min_dmg = 0;
	//n->max_dmg = 0;
	//n->charm_ac = 0;
	//n->charm_min_dmg = 0;
	//n->charm_max_dmg = 0;
	//n->charm_attack_delay = 0;
	//n->charm_accuracy_rating = 0;
	//n->charm_avoidance_rating = 0;
	//n->charm_atk = 0;
	//n->attack_count = 0;
	//*n->special_abilities = { 0 };
	//n->d_melee_texture1 = 0;
	//n->d_melee_texture2 = 0;
	//*n->ammo_idfile = { 0 };
	//n->prim_melee_type = 0;
	//n->sec_melee_type = 0;
	//n->ranged_type = 0;
	n->hp_regen = 1;
	n->mana_regen = 1;
	//n->aggroradius = 0;
	//n->assistradius = 0;
	//n->see_invis = 0;
	//n->see_invis_undead = false;
	//n->see_hide = false;
	//n->see_improved_hide = false;
	//n->qglobal = false;
	//n->npc_aggro = false;
	//n->spawn_limit = 0;
	//n->mount_color = 0;
	//n->attack_speed = 0.0f;
	//n->attack_delay = 0;
	//n->accuracy_rating = 0;
	//n->avoidance_rating = 0;
	//n->findable = false;
	n->trackable = true;
	//n->slow_mitigation = 0;
	n->maxlevel = botLevel;
	//n->scalerate = 0;
	//n->private_corpse = false;
	//n->unique_spawn_by_name = false;
	//n->underwater = false;
	//n->emoteid = 0;
	//n->spellscale = 0.0f;
	//n->healscale = 0.0f;
	//n->no_target_hotkey = false;
	//n->raid_target = false;
	//n->armtexture = 0;
	//n->bracertexture = 0;
	//n->handtexture = 0;
	//n->legtexture = 0;
	//n->feettexture = 0;
	//n->ignore_despawn = false;
	n->show_name = true;
	//n->untargetable = false;
	n->skip_global_loot = true;
	//n->rare_spawn = false;
	n->stuck_behavior = Ground;
	n->skip_auto_scale = true;

	return n;
}

NPCType *Bot::CreateDefaultNPCTypeStructForBot(
	std::string botName,
	std::string botLastName,
	uint8 botLevel,
	uint16 botRace,
	uint8 botClass,
	uint8 gender
) {
	auto n = new NPCType{ 0 };

	strn0cpy(n->name, botName.c_str(), sizeof(n->name));
	strn0cpy(n->lastname, botLastName.c_str(), sizeof(n->lastname));

	//n->current_hp = 0;
	//n->max_hp = 0;
	n->size = 6.0f;
	n->runspeed = 0.7f;
	n->gender = gender;
	n->race = botRace;
	n->class_ = botClass;
	n->bodytype = 1;
	n->deity = EQ::deity::DeityAgnostic;
	n->level = botLevel;
	//n->npc_id = 0;
	//n->texture = 0;
	//n->helmtexture = 0;
	//n->herosforgemodel = 0;
	//n->loottable_id = 0;
	//n->npc_spells_id = 0;
	//n->npc_spells_effects_id = 0;
	//n->npc_faction_id = 0;
	//n->merchanttype = 0;
	//n->alt_currency_type = 0;
	//n->adventure_template = 0;
	//n->trap_template = 0;
	//n->light = 0;
	n->AC = 12;
	//n->Mana = 0;
	n->ATK = 75;
	n->STR = 75;
	n->STA = 75;
	n->DEX = 75;
	n->AGI = 75;
	n->INT = 75;
	n->WIS = 75;
	n->CHA = 75;
	n->MR = 25;
	n->FR = 25;
	n->CR = 25;
	n->PR = 15;
	n->DR = 15;
	n->Corrup = 15;
	//n->PhR = 0;
	//n->haircolor = 0;
	//n->beardcolor = 0;
	//n->eyecolor1 = 0;
	//n->eyecolor2 = 0;
	//n->hairstyle = 0;
	//n->luclinface = 0;
	//n->beard = 0;
	//n->drakkin_heritage = 0;
	//n->drakkin_tattoo = 0;
	//n->drakkin_details = 0;
	//n->armor_tint = { 0 };
	//n->min_dmg = 0;
	//n->max_dmg = 0;
	//n->charm_ac = 0;
	//n->charm_min_dmg = 0;
	//n->charm_max_dmg = 0;
	//n->charm_attack_delay = 0;
	//n->charm_accuracy_rating = 0;
	//n->charm_avoidance_rating = 0;
	//n->charm_atk = 0;
	//n->attack_count = 0;
	//*n->special_abilities = { 0 };
	//n->d_melee_texture1 = 0;
	//n->d_melee_texture2 = 0;
	//*n->ammo_idfile = { 0 };
	//n->prim_melee_type = 0;
	//n->sec_melee_type = 0;
	//n->ranged_type = 0;
	n->hp_regen = 1;
	n->mana_regen = 1;
	//n->aggroradius = 0;
	//n->assistradius = 0;
	//n->see_invis = 0;
	//n->see_invis_undead = false;
	//n->see_hide = false;
	//n->see_improved_hide = false;
	//n->qglobal = false;
	//n->npc_aggro = false;
	//n->spawn_limit = 0;
	//n->mount_color = 0;
	//n->attack_speed = 0.0f;
	//n->attack_delay = 0;
	//n->accuracy_rating = 0;
	//n->avoidance_rating = 0;
	//n->findable = false;
	n->trackable = true;
	//n->slow_mitigation = 0;
	n->maxlevel = botLevel;
	//n->scalerate = 0;
	//n->private_corpse = false;
	//n->unique_spawn_by_name = false;
	//n->underwater = false;
	//n->emoteid = 0;
	//n->spellscale = 0.0f;
	//n->healscale = 0.0f;
	//n->no_target_hotkey = false;
	//n->raid_target = false;
	//n->armtexture = 0;
	//n->bracertexture = 0;
	//n->handtexture = 0;
	//n->legtexture = 0;
	//n->feettexture = 0;
	//n->ignore_despawn = false;
	n->show_name = true;
	//n->untargetable = false;
	n->skip_global_loot = true;
	//n->rare_spawn = false;
	n->stuck_behavior = Ground;

	return n;
}

void Bot::GenerateBaseStats()
{
	int BotSpellID = 0;

	// base stats
	uint32 Strength = _baseSTR;
	uint32 Stamina = _baseSTA;
	uint32 Dexterity = _baseDEX;
	uint32 Agility = _baseAGI;
	uint32 Wisdom = _baseWIS;
	uint32 Intelligence = _baseINT;
	uint32 Charisma = _baseCHA;
	uint32 Attack = _baseATK;
	int32 MagicResist = _baseMR;
	int32 FireResist = _baseFR;
	int32 DiseaseResist = _baseDR;
	int32 PoisonResist = _basePR;
	int32 ColdResist = _baseCR;
	int32 CorruptionResist = _baseCorrup;

	// pulling fixed values from an auto-increment field is dangerous...
	switch (GetClass()) {
		case WARRIOR:
			BotSpellID = 3001;
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case CLERIC:
			BotSpellID = 3002;
			Strength += 5;
			Stamina += 5;
			Agility += 10;
			Wisdom += 30;
			Attack += 8;
			break;
		case PALADIN:
			BotSpellID = 3003;
			Strength += 15;
			Stamina += 5;
			Wisdom += 15;
			Charisma += 10;
			Dexterity += 5;
			Attack += 17;
			break;
		case RANGER:
			BotSpellID = 3004;
			Strength += 15;
			Stamina += 10;
			Agility += 10;
			Wisdom += 15;
			Attack += 17;
			break;
		case SHADOWKNIGHT:
			BotSpellID = 3005;
			Strength += 10;
			Stamina += 15;
			Intelligence += 20;
			Charisma += 5;
			Attack += 17;
			break;
		case DRUID:
			BotSpellID = 3006;
			Stamina += 15;
			Wisdom += 35;
			Attack += 5;
			break;
		case MONK:
			BotSpellID = 3007;
			Strength += 5;
			Stamina += 15;
			Agility += 15;
			Dexterity += 15;
			Attack += 17;
			break;
		case BARD:
			BotSpellID = 3008;
			Strength += 15;
			Dexterity += 10;
			Charisma += 15;
			Intelligence += 10;
			Attack += 17;
			break;
		case ROGUE:
			BotSpellID = 3009;
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case SHAMAN:
			BotSpellID = 3010;
			Stamina += 10;
			Wisdom += 30;
			Charisma += 10;
			Attack += 28;
			break;
		case NECROMANCER:
			BotSpellID = 3011;
			Dexterity += 10;
			Agility += 10;
			Intelligence += 30;
			Attack += 5;
			break;
		case WIZARD:
			BotSpellID = 3012;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case MAGICIAN:
			BotSpellID = 3013;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case ENCHANTER:
			BotSpellID = 3014;
			Intelligence += 25;
			Charisma += 25;
			Attack += 5;
			break;
		case BEASTLORD:
			BotSpellID = 3015;
			Stamina += 10;
			Agility += 10;
			Dexterity += 5;
			Wisdom += 20;
			Charisma += 5;
			Attack += 31;
			break;
		case BERSERKER:
			BotSpellID = 3016;
			Strength += 10;
			Stamina += 15;
			Dexterity += 15;
			Agility += 10;
			Attack += 25;
			break;
		default:
			break;
	}

	float BotSize = GetSize();

	switch(GetRace()) {
		case HUMAN: // Humans have no race bonus
			break;
		case BARBARIAN:
			Strength += 28;
			Stamina += 20;
			Agility += 7;
			Dexterity -= 5;
			Wisdom -= 5;
			Intelligence -= 10;
			Charisma -= 20;
			BotSize = 7.0;
			ColdResist += 10;
			break;
		case ERUDITE:
			Strength -= 15;
			Stamina -= 5;
			Agility -= 5;
			Dexterity -= 5;
			Wisdom += 8;
			Intelligence += 32;
			Charisma -= 5;
			MagicResist += 5;
			DiseaseResist -= 5;
			break;
		case WOOD_ELF:
			Strength -= 10;
			Stamina -= 10;
			Agility += 20;
			Dexterity += 5;
			Wisdom += 5;
			BotSize = 5.0;
			break;
		case HIGH_ELF:
			Strength -= 20;
			Stamina -= 10;
			Agility += 10;
			Dexterity -= 5;
			Wisdom += 20;
			Intelligence += 12;
			Charisma += 5;
			break;
		case DARK_ELF:
			Strength -= 15;
			Stamina -= 10;
			Agility += 15;
			Wisdom += 8;
			Intelligence += 24;
			Charisma -= 15;
			BotSize = 5.0;
			break;
		case HALF_ELF:
			Strength -= 5;
			Stamina -= 5;
			Agility += 15;
			Dexterity += 10;
			Wisdom -= 15;
			BotSize = 5.5;
			break;
		case DWARF:
			Strength += 15;
			Stamina += 15;
			Agility -= 5;
			Dexterity += 15;
			Wisdom += 8;
			Intelligence -= 15;
			Charisma -= 30;
			BotSize = 4.0;
			MagicResist -= 5;
			PoisonResist += 5;
			break;
		case TROLL:
			Strength += 33;
			Stamina += 34;
			Agility += 8;
			Wisdom -= 15;
			Intelligence -= 23;
			Charisma -= 35;
			BotSize = 8.0;
			FireResist -= 20;
			break;
		case OGRE:
			Strength += 55;
			Stamina += 77;
			Agility -= 5;
			Dexterity -= 5;
			Wisdom -= 8;
			Intelligence -= 15;
			Charisma -= 38;
			BotSize = 9.0;
			break;
		case HALFLING:
			Strength -= 5;
			Agility += 20;
			Dexterity += 15;
			Wisdom += 5;
			Intelligence -= 8;
			Charisma -= 25;
			BotSize = 3.5;
			PoisonResist += 5;
			DiseaseResist += 5;
			break;
		case GNOME:
			Strength -= 15;
			Stamina -= 5;
			Agility += 10;
			Dexterity += 10;
			Wisdom -= 8;
			Intelligence += 23;
			Charisma -= 15;
			BotSize = 3.0;
			break;
		case IKSAR:
			Strength -= 5;
			Stamina -= 5;
			Agility += 15;
			Dexterity += 10;
			Wisdom += 5;
			Charisma -= 20;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case VAHSHIR:
			Strength += 15;
			Agility += 15;
			Dexterity -= 5;
			Wisdom -= 5;
			Intelligence -= 10;
			Charisma -= 10;
			BotSize = 7.0;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case FROGLOK:
			Strength -= 5;
			Stamina += 5;
			Agility += 25;
			Dexterity += 25;
			Charisma -= 25;
			BotSize = 5.0;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case DRAKKIN:
			Strength -= 5;
			Stamina += 5;
			Agility += 10;
			Intelligence += 10;
			Wisdom += 5;
			BotSize = 5.0;
			PoisonResist += 2;
			DiseaseResist += 2;
			MagicResist += 2;
			FireResist += 2;
			ColdResist += 2;
			break;
		default:
			break;
	}

	STR = Strength;
	STA = Stamina;
	DEX = Dexterity;
	AGI = Agility;
	WIS = Wisdom;
	INT = Intelligence;
	CHA = Charisma;
	ATK = Attack;
	MR = MagicResist;
	FR = FireResist;
	DR = DiseaseResist;
	PR = PoisonResist;
	CR = ColdResist;
	PhR = 0;
	Corrup = CorruptionResist;
	SetBotSpellID(BotSpellID);
	size = BotSize;
	pAggroRange = 0;
	pAssistRange = 0;
	raid_target = false;
	deity = 396;
}

void Bot::GenerateAppearance() {
	// Randomize facial appearance
	int iFace = 0;
	if (GetRace() == BARBARIAN) // Barbarian w/Tatoo
	{
		iFace = zone->random.Int(0, 79);
	}
	else
	{
		iFace = zone->random.Int(0, 7);
	}

	int iHair = 0;
	int iBeard = 0;
	int iBeardColor = 1;
	if (GetRace() == DRAKKIN) {
		iHair = zone->random.Int(0, 8);
		iBeard = zone->random.Int(0, 11);
		iBeardColor = zone->random.Int(0, 3);
	} else if (GetGender()) {
		iHair = zone->random.Int(0, 2);
		if (GetRace() == DWARF) { // Dwarven Females can have a beard
			if(zone->random.Int(1, 100) < 50)
				iFace += 10;
		}
	} else {
		iHair = zone->random.Int(0, 3);
		iBeard = zone->random.Int(0, 5);
		iBeardColor = zone->random.Int(0, 19);
	}

	int iHairColor = 0;
	if (GetRace() == DRAKKIN) {
		iHairColor = zone->random.Int(0, 3);
	} else {
		iHairColor = zone->random.Int(0, 19);
	}

	uint8 iEyeColor1 = (uint8)zone->random.Int(0, 9);
	uint8 iEyeColor2 = 0;
	if (GetRace() == DRAKKIN) {
		iEyeColor1 = iEyeColor2 = (uint8)zone->random.Int(0, 11);
	} else if(zone->random.Int(1, 100) > 96) {
		iEyeColor2 = zone->random.Int(0, 9);
	} else {
		iEyeColor2 = iEyeColor1;
	}

	int iHeritage = 0;
	int iTattoo = 0;
	int iDetails = 0;
	if (GetRace() == DRAKKIN) {
		iHeritage = zone->random.Int(0, 6);
		iTattoo = zone->random.Int(0, 7);
		iDetails = zone->random.Int(0, 7);
	}
	luclinface = iFace;
	hairstyle = iHair;
	beard = iBeard;
	beardcolor = iBeardColor;
	haircolor = iHairColor;
	eyecolor1 = iEyeColor1;
	eyecolor2 = iEyeColor2;
	drakkin_heritage = iHeritage;
	drakkin_tattoo = iTattoo;
	drakkin_details = iDetails;
}

int32 Bot::acmod() {
	int agility = GetAGI();
	int level = GetLevel();
	if(agility < 1 || level < 1)
		return 0;

	if(agility <= 74) {
		if(agility == 1)
			return -24;
		else if(agility <= 3)
			return -23;
		else if(agility == 4)
			return -22;
		else if(agility <= 6)
			return -21;
		else if(agility <= 8)
			return -20;
		else if(agility == 9)
			return -19;
		else if(agility <= 11)
			return -18;
		else if(agility == 12)
			return -17;
		else if(agility <= 14)
			return -16;
		else if(agility <= 16)
			return -15;
		else if(agility == 17)
			return -14;
		else if(agility <= 19)
			return -13;
		else if(agility == 20)
			return -12;
		else if(agility <= 22)
			return -11;
		else if(agility <= 24)
			return -10;
		else if(agility == 25)
			return -9;
		else if(agility <= 27)
			return -8;
		else if(agility == 28)
			return -7;
		else if(agility <= 30)
			return -6;
		else if(agility <= 32)
			return -5;
		else if(agility == 33)
			return -4;
		else if(agility <= 35)
			return -3;
		else if(agility == 36)
			return -2;
		else if(agility <= 38)
			return -1;
		else if(agility <= 65)
			return 0;
		else if(agility <= 70)
			return 1;
		else if(agility <= 74)
			return 5;
	} else if(agility <= 137) {
		if(agility == 75) {
			if(level <= 6)
				return 9;
			else if(level <= 19)
				return 23;
			else if(level <= 39)
				return 33;
			else
				return 39;
		} else if(agility >= 76 && agility <= 79) {
			if(level <= 6)
				return 10;
			else if(level <= 19)
				return 23;
			else if(level <= 39)
				return 33;
			else
				return 40;
		} else if(agility == 80) {
			if(level <= 6)
				return 11;
			else if(level <= 19)
				return 24;
			else if(level <= 39)
				return 34;
			else
				return 41;
		} else if(agility >= 81 && agility <= 85) {
			if(level <= 6)
				return 12;
			else if(level <= 19)
				return 25;
			else if(level <= 39)
				return 35;
			else
				return 42;
		} else if(agility >= 86 && agility <= 90) {
			if(level <= 6)
				return 12;
			else if(level <= 19)
				return 26;
			else if(level <= 39)
				return 36;
			else
				return 42;
		} else if(agility >= 91 && agility <= 95) {
			if(level <= 6)
				return 13;
			else if(level <= 19)
				return 26;
			else if(level <= 39)
				return 36;
			else
				return 43;
		} else if(agility >= 96 && agility <= 99) {
			if(level <= 6)
				return 14;
			else if(level <= 19)
				return 27;
			else if(level <= 39)
				return 37;
			else
				return 44;
		} else if(agility == 100 && level >= 7) {
			if(level <= 19)
				return 28;
			else if (level <= 39)
				return 38;
			else
				return 45;
		}
		else if(level <= 6)
			return 15;
		//level is >6
		else if(agility >= 101 && agility <= 105) {
			if(level <= 19)
				return 29;
			else if(level <= 39)
				return 39;// not verified
			else
				return 45;
		} else if(agility >= 106 && agility <= 110) {
			if(level <= 19)
				return 29;
			else if(level <= 39)
				return 39;// not verified
			else
				return 46;
		} else if(agility >= 111 && agility <= 115) {
			if(level <= 19)
				return 30;
			else if(level <= 39)
				return 40;// not verified
			else
				return 47;
		} else if(agility >= 116 && agility <= 119) {
			if(level <= 19)
				return 31;
			else if(level <= 39)
				return 41;
			else
				return 47;
		}
		else if(level <= 19)
			return 32;
		//level is > 19
		else if(agility == 120) {
			if(level <= 39)
				return 42;
			else
				return 48;
		} else if(agility <= 125) {
			if(level <= 39)
				return 42;
			else
				return 49;
		} else if(agility <= 135) {
			if(level <= 39)
				return 42;
			else
				return 50;
		} else {
			if(level <= 39)
				return 42;
			else
				return 51;
		}
	} else if(agility <= 300) {
		if(level <= 6) {
			if(agility <= 139)
				return 21;
			else if(agility == 140)
				return 22;
			else if(agility <= 145)
				return 23;
			else if(agility <= 150)
				return 23;
			else if(agility <= 155)
				return 24;
			else if(agility <= 159)
				return 25;
			else if(agility == 160)
				return 26;
			else if(agility <= 165)
				return 26;
			else if(agility <= 170)
				return 27;
			else if(agility <= 175)
				return 28;
			else if(agility <= 179)
				return 28;
			else if(agility == 180)
				return 29;
			else if(agility <= 185)
				return 30;
			else if(agility <= 190)
				return 31;
			else if(agility <= 195)
				return 31;
			else if(agility <= 199)
				return 32;
			else if(agility <= 219)
				return 33;
			else if(agility <= 239)
				return 34;
			else
				return 35;
		} else if(level <= 19) {
			if(agility <= 139)
				return 34;
			else if(agility == 140)
				return 35;
			else if(agility <= 145)
				return 36;
			else if(agility <= 150)
				return 37;
			else if(agility <= 155)
				return 37;
			else if(agility <= 159)
				return 38;
			else if(agility == 160)
				return 39;
			else if(agility <= 165)
				return 40;
			else if(agility <= 170)
				return 40;
			else if(agility <= 175)
				return 41;
			else if(agility <= 179)
				return 42;
			else if(agility == 180)
				return 43;
			else if(agility <= 185)
				return 43;
			else if(agility <= 190)
				return 44;
			else if(agility <= 195)
				return 45;
			else if(agility <= 199)
				return 45;
			else if(agility <= 219)
				return 46;
			else if(agility <= 239)
				return 47;
			else
				return 48;
		} else if(level <= 39) {
			if(agility <= 139)
				return 44;
			else if(agility == 140)
				return 45;
			else if(agility <= 145)
				return 46;
			else if(agility <= 150)
				return 47;
			else if(agility <= 155)
				return 47;
			else if(agility <= 159)
				return 48;
			else if(agility == 160)
				return 49;
			else if(agility <= 165)
				return 50;
			else if(agility <= 170)
				return 50;
			else if(agility <= 175)
				return 51;
			else if(agility <= 179)
				return 52;
			else if(agility == 180)
				return 53;
			else if(agility <= 185)
				return 53;
			else if(agility <= 190)
				return 54;
			else if(agility <= 195)
				return 55;
			else if(agility <= 199)
				return 55;
			else if(agility <= 219)
				return 56;
			else if(agility <= 239)
				return 57;
			else
				return 58;
		} else {	//lvl >= 40
			if(agility <= 139)
				return 51;
			else if(agility == 140)
				return 52;
			else if(agility <= 145)
				return 53;
			else if(agility <= 150)
				return 53;
			else if(agility <= 155)
				return 54;
			else if(agility <= 159)
				return 55;
			else if(agility == 160)
				return 56;
			else if(agility <= 165)
				return 56;
			else if(agility <= 170)
				return 57;
			else if(agility <= 175)
				return 58;
			else if(agility <= 179)
				return 58;
			else if(agility == 180)
				return 59;
			else if(agility <= 185)
				return 60;
			else if(agility <= 190)
				return 61;
			else if(agility <= 195)
				return 61;
			else if(agility <= 199)
				return 62;
			else if(agility <= 219)
				return 63;
			else if(agility <= 239)
				return 64;
			else
				return 65;
		}
	}
	else
		return (65 + ((agility - 300) / 21));

	LogError("Agility [{}] Level [{}]",agility,level);
	return 0;
}

uint16 Bot::GetPrimarySkillValue() {
	EQ::skills::SkillType skill = EQ::skills::HIGHEST_SKILL; //because nullptr == 0, which is 1H Slashing, & we want it to return 0 from GetSkill
	bool equiped = m_inv.GetItem(EQ::invslot::slotPrimary);
	if(!equiped)
		skill = EQ::skills::SkillHandtoHand;
	else {
		uint8 type = m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->ItemType; //is this the best way to do this?
		switch(type) {
		case EQ::item::ItemType1HSlash:
			skill = EQ::skills::Skill1HSlashing;
			break;
		case EQ::item::ItemType2HSlash:
			skill = EQ::skills::Skill2HSlashing;
			break;
		case EQ::item::ItemType1HPiercing:
			skill = EQ::skills::Skill1HPiercing;
			break;
		case EQ::item::ItemType1HBlunt:
			skill = EQ::skills::Skill1HBlunt;
			break;
		case EQ::item::ItemType2HBlunt:
			skill = EQ::skills::Skill2HBlunt;
			break;
		case EQ::item::ItemType2HPiercing:
			skill = EQ::skills::Skill2HPiercing;
			break;
		case EQ::item::ItemTypeMartial:
			skill = EQ::skills::SkillHandtoHand;
			break;
		default:
			skill = EQ::skills::SkillHandtoHand;
			break;
		}
	}

	return GetSkill(skill);
}

uint16 Bot::MaxSkill(EQ::skills::SkillType skillid, uint16 class_, uint16 level) const {
	return(content_db.GetSkillCap(class_, skillid, level));
}

uint32 Bot::GetTotalATK() {
	uint32 AttackRating = 0;
	uint32 WornCap = itembonuses.ATK;
	if(IsBot()) {
		AttackRating = ((WornCap * 1.342) + (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69));
		AttackRating += aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();
		if (AttackRating < 10)
			AttackRating = 10;
	}
	else
		AttackRating = GetATK();

	AttackRating += spellbonuses.ATK;
	return AttackRating;
}

uint32 Bot::GetATKRating() {
	uint32 AttackRating = 0;
	if(IsBot()) {
		AttackRating = (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69);
		if (AttackRating < 10)
			AttackRating = 10;
	}
	return AttackRating;
}

int32 Bot::GenerateBaseHitPoints() {
	// Calc Base Hit Points
	int new_base_hp = 0;
	uint32 lm = GetClassLevelFactor();
	int32 Post255;
	int32 NormalSTA = GetSTA();
	if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		float SoDPost255;
		if(((NormalSTA - 255) / 2) > 0)
			SoDPost255 = ((NormalSTA - 255) / 2);
		else
			SoDPost255 = 0;

		int hp_factor = GetClassHPFactor();

		if(level < 41)
			new_base_hp = (5 + (GetLevel() * hp_factor / 12) + ((NormalSTA - SoDPost255) * GetLevel() * hp_factor / 3600));
		else if(level < 81)
			new_base_hp = (5 + (40 * hp_factor / 12) + ((GetLevel() - 40) * hp_factor / 6) + ((NormalSTA - SoDPost255) * hp_factor / 90) + ((NormalSTA - SoDPost255) * (GetLevel() - 40) * hp_factor / 1800));
		else
			new_base_hp = (5 + (80 * hp_factor / 8) + ((GetLevel() - 80) * hp_factor / 10) + ((NormalSTA - SoDPost255) * hp_factor / 90) + ((NormalSTA - SoDPost255) * hp_factor / 45));
	} else {
		if(((NormalSTA - 255) / 2) > 0)
			Post255 = ((NormalSTA - 255) / 2);
		else
			Post255 = 0;

		new_base_hp = (5) + (GetLevel() * lm / 10) + (((NormalSTA - Post255) * GetLevel() * lm / 3000)) + ((Post255 * 1) * lm / 6000);
	}
	base_hp = new_base_hp;
	return new_base_hp;
}

void Bot::LoadAAs() {

	aa_ranks.clear();

	int id = 0;
	int points = 0;
	auto iter = zone->aa_abilities.begin();
	while(iter != zone->aa_abilities.end()) {
		AA::Ability *ability = (*iter).second.get();

		//skip expendables
		if(!ability->first || ability->charges > 0) {
			++iter;
			continue;
		}

		id = ability->first->id;
		points = 0;

		AA::Rank *current = ability->first;

		if (current->level_req > GetLevel()) {
			++iter;
			continue;
		}

		while(current) {
			if(!CanUseAlternateAdvancementRank(current)) {
				current = nullptr;
			} else {
				current = current->next;
				points++;
			}
		}

		if(points > 0) {
			SetAA(id, points);
		}

		++iter;
	}
}

bool Bot::IsValidRaceClassCombo()
{
	return Bot::IsValidRaceClassCombo(GetRace(), GetClass());
}

bool Bot::IsValidRaceClassCombo(uint16 bot_race, uint8 bot_class)
{
	bool is_valid = false;
	auto classes = database.botdb.GetRaceClassBitmask(bot_race);
	auto bot_class_bitmask = GetPlayerClassBit(bot_class);
	if (classes & bot_class_bitmask) {
		is_valid = true;
	}
	return is_valid;
}

bool Bot::IsValidName()
{
	std::string name = GetCleanName();
	return Bot::IsValidName(name);
}

bool Bot::IsValidName(std::string& name)
{
	if (name.length() < 4)
		return false;
	if (!isupper(name[0]))
		return false;

	for (int i = 1; i < name.length(); ++i) {
		if ((!RuleB(Bots, AllowCamelCaseNames) && !islower(name[i])) && name[i] != '_') {
			return false;
		}
	}

	return true;
}

bool Bot::Save()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	std::string error_message;

	if(!GetBotID()) { // New bot record
		uint32 bot_id = 0;
		if (!database.botdb.SaveNewBot(this, bot_id) || !bot_id) {
			bot_owner->Message(Chat::White, "%s '%s'", BotDatabase::fail::SaveNewBot(), GetCleanName());
			return false;
		}
		SetBotID(bot_id);
	}
	else { // Update existing bot record
		if (!database.botdb.SaveBot(this)) {
			bot_owner->Message(Chat::White, "%s '%s'", BotDatabase::fail::SaveBot(), GetCleanName());
			return false;
		}
	}

	// All of these continue to process if any fail
	if (!database.botdb.SaveBuffs(this))
		bot_owner->Message(Chat::White, "%s for '%s'", BotDatabase::fail::SaveBuffs(), GetCleanName());
	if (!database.botdb.SaveTimers(this))
		bot_owner->Message(Chat::White, "%s for '%s'", BotDatabase::fail::SaveTimers(), GetCleanName());

	if (!database.botdb.SaveStance(this)) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Failed to save stance for '{}'.",
				GetCleanName()
			).c_str()
		);
	}

	if (!SavePet())
		bot_owner->Message(Chat::White, "Failed to save pet for '%s'", GetCleanName());

	return true;
}

bool Bot::DeleteBot()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return false;
	}

	if (!database.botdb.DeleteHealRotation(GetBotID())) {
		bot_owner->Message(Chat::White, "%s", BotDatabase::fail::DeleteHealRotation());
		return false;
	}

	std::string query = StringFormat("DELETE FROM `bot_heal_rotation_members` WHERE `bot_id` = '%u'", GetBotID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(Chat::White, "Failed to delete heal rotation member '%s'", GetCleanName());
		return false;
	}

	query = StringFormat("DELETE FROM `bot_heal_rotation_targets` WHERE `target_name` LIKE '%s'", GetCleanName());
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(Chat::White, "Failed to delete heal rotation target '%s'", GetCleanName());
		return false;
	}

	if (!DeletePet()) {
		bot_owner->Message(Chat::White, "Failed to delete pet for '%s'", GetCleanName());
		return false;
	}

	if (GetGroup())
		RemoveBotFromGroup(this, GetGroup());

	std::string error_message;

	if (!database.botdb.RemoveMemberFromBotGroup(GetBotID())) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Failed to remove {} from their bot-group.",
				GetCleanName()
			).c_str()
		);
		return false;
	}

	if (!database.botdb.DeleteItems(GetBotID())) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} for '{}'.",
				BotDatabase::fail::DeleteItems(),
				GetCleanName()
			).c_str()
		);
		return false;
	}

	if (!database.botdb.DeleteTimers(GetBotID())) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} for '{}'.",
				BotDatabase::fail::DeleteTimers(),
				GetCleanName()
			).c_str()
		);
		return false;
	}

	if (!database.botdb.DeleteBuffs(GetBotID())) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} for '{}'.",
				BotDatabase::fail::DeleteBuffs(),
				GetCleanName()
			).c_str()
		);
		return false;
	}

	if (!database.botdb.DeleteStance(GetBotID())) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} for '{}'.",
				BotDatabase::fail::DeleteStance(),
				GetCleanName()
			).c_str()
		);
		return false;
	}

	if (!database.botdb.DeleteBot(GetBotID())) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} '{}'",
				BotDatabase::fail::DeleteBot(),
				GetCleanName()
			).c_str()
		);
		return false;
	}

	return true;
}

// Returns the current total play time for the bot
uint32 Bot::GetTotalPlayTime() {
	uint32 Result = 0;
	double TempTotalPlayTime = 0;
	time_t currentTime = time(&currentTime);
	TempTotalPlayTime = difftime(currentTime, _startTotalPlayTime);
	TempTotalPlayTime += _lastTotalPlayTime;
	Result = (uint32)TempTotalPlayTime;
	return Result;
}

bool Bot::LoadPet()
{
	if (GetPet())
		return true;

	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	if (GetClass() == WIZARD) {
		auto buffs_max = GetMaxBuffSlots();
		auto my_buffs = GetBuffs();
		if (buffs_max && my_buffs) {
			for (int index = 0; index < buffs_max; ++index) {
				if (IsEffectInSpell(my_buffs[index].spellid, SE_Familiar)) {
					MakePet(my_buffs[index].spellid, spells[my_buffs[index].spellid].teleport_zone);
					return true;
				}
			}
		}
	}

	std::string error_message;

	uint32 pet_index = 0;
	if (!database.botdb.LoadPetIndex(GetBotID(), pet_index)) {
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::LoadPetIndex(), GetCleanName());
		return false;
	}
	if (!pet_index)
		return true;

	uint32 saved_pet_spell_id = 0;
	if (!database.botdb.LoadPetSpellID(GetBotID(), saved_pet_spell_id)) {
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::LoadPetSpellID(), GetCleanName());
	}
	if (!IsValidSpell(saved_pet_spell_id)) {
		bot_owner->Message(Chat::White, "Invalid spell id for %s's pet", GetCleanName());
		DeletePet();
		return false;
	}

	std::string pet_name;
	uint32 pet_mana = 0;
	uint32 pet_hp = 0;
	uint32 pet_spell_id = 0;

	if (!database.botdb.LoadPetStats(GetBotID(), pet_name, pet_mana, pet_hp, pet_spell_id)) {
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::LoadPetStats(), GetCleanName());
		return false;
	}

	MakePet(pet_spell_id, spells[pet_spell_id].teleport_zone, pet_name.c_str());
	if (!GetPet() || !GetPet()->IsNPC()) {
		DeletePet();
		return false;
	}

	NPC *pet_inst = GetPet()->CastToNPC();

	SpellBuff_Struct pet_buffs[PET_BUFF_COUNT];
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * PET_BUFF_COUNT));
	if (!database.botdb.LoadPetBuffs(GetBotID(), pet_buffs))
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::LoadPetBuffs(), GetCleanName());

	uint32 pet_items[EQ::invslot::EQUIPMENT_COUNT];
	memset(pet_items, 0, (sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT));
	if (!database.botdb.LoadPetItems(GetBotID(), pet_items))
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::LoadPetItems(), GetCleanName());

	pet_inst->SetPetState(pet_buffs, pet_items);
	pet_inst->CalcBonuses();
	pet_inst->SetHP(pet_hp);
	pet_inst->SetMana(pet_mana);

	return true;
}

bool Bot::SavePet()
{
	if (!GetPet() || GetPet()->IsFamiliar()) // dead?
		return true;

	NPC *pet_inst = GetPet()->CastToNPC();
	if (!pet_inst->GetPetSpellID() || !IsValidSpell(pet_inst->GetPetSpellID()))
		return false;

	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	char* pet_name = new char[64];
	SpellBuff_Struct pet_buffs[PET_BUFF_COUNT];
	uint32 pet_items[EQ::invslot::EQUIPMENT_COUNT];

	memset(pet_name, 0, 64);
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * PET_BUFF_COUNT));
	memset(pet_items, 0, (sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT));

	pet_inst->GetPetState(pet_buffs, pet_items, pet_name);

	std::string pet_name_str = pet_name;
	safe_delete_array(pet_name);

	std::string error_message;

	if (!database.botdb.SavePetStats(GetBotID(), pet_name_str, pet_inst->GetMana(), pet_inst->GetHP(), pet_inst->GetPetSpellID())) {
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::SavePetStats(), GetCleanName());
		return false;
	}

	if (!database.botdb.SavePetBuffs(GetBotID(), pet_buffs))
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::SavePetBuffs(), GetCleanName());
	if (!database.botdb.SavePetItems(GetBotID(), pet_items))
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::SavePetItems(), GetCleanName());

	return true;
}

bool Bot::DeletePet()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	std::string error_message;

	if (!database.botdb.DeletePetItems(GetBotID())) {
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::DeletePetItems(), GetCleanName());
		return false;
	}
	if (!database.botdb.DeletePetBuffs(GetBotID())) {
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::DeletePetBuffs(), GetCleanName());
		return false;
	}
	if (!database.botdb.DeletePetStats(GetBotID())) {
		bot_owner->Message(Chat::White, "%s for %s's pet", BotDatabase::fail::DeletePetStats(), GetCleanName());
		return false;
	}

	if (!GetPet() || !GetPet()->IsNPC())
		return true;

	NPC* pet_inst = GetPet()->CastToNPC();
	pet_inst->SetOwnerID(0);

	SetPet(nullptr);

	return true;
}

bool Bot::Process()
{
	if (IsStunned() && stunned_timer.Check()) {
		Mob::UnStun();
	}

	if (!GetBotOwner()) {
		return false;
	}

	if (GetDepop()) {

		_botOwner = 0;
		_botOwnerCharacterID = 0;
		_previousTarget = 0;

		return false;
	}

	if (mob_close_scan_timer.Check()) {
		LogAIScanCloseDetail(
			"is_moving [{}] bot [{}] timer [{}]",
			moving ? "true" : "false",
			GetCleanName(),
			mob_close_scan_timer.GetDuration()
		);

		entity_list.ScanCloseClientMobs(close_mobs, this);
	}

	SpellProcess();

	if (tic_timer.Check()) {

		// 6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if (!IsEngaged()) {
			if (!rest_timer.Enabled()) {
				rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
			}
		}

		BuffProcess();
		CalcRestState();

		if (currently_fleeing) {
			ProcessFlee();
		}

		if (GetHP() < GetMaxHP()) {
			SetHP(GetHP() + CalcHPRegen() + RestRegenHP);
		}

		if (GetMana() < GetMaxMana()) {
			SetMana(GetMana() + CalcManaRegen() + RestRegenMana);
		}

		CalcATK();

		if (GetEndurance() < GetMaxEndurance()) {
			SetEndurance(GetEndurance() + CalcEnduranceRegen() + RestRegenEndurance);
		}
	}

	if (send_hp_update_timer.Check(false)) {

		SendHPUpdate();
		if (HasPet()) {
			GetPet()->SendHPUpdate();
		}

		// hack fix until percentage changes can be implemented
		auto g = GetGroup();
		if (g) {

			g->SendManaPacketFrom(this);
			g->SendEndurancePacketFrom(this);
		}
	}

	if (GetAppearance() == eaDead && GetHP() > 0) {
		SetAppearance(eaStanding);
	}

	if (IsMoving()) {
		ping_timer.Disable();
	}
	else {

		if (!ping_timer.Enabled()) {
			ping_timer.Start(BOT_KEEP_ALIVE_INTERVAL);
		}

		if (ping_timer.Check()) {
			SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);
		}
	}

	if (IsStunned() || IsMezzed()) {
		return true;
	}

	// Bot AI
	AI_Process();

	return true;
}

void Bot::AI_Bot_Start(uint32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);
	if (!pAIControlled) {
		return;
	}

	if (AIBot_spells.empty()) {
		AIautocastspell_timer = std::make_unique<Timer>(1000);
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer = std::make_unique<Timer>(500);
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
	}

	if (NPCTypedata) {
		ProcessSpecialAbilities(NPCTypedata->special_abilities);
		AI_AddNPCSpellsEffects(NPCTypedata->npc_spells_effects_id);
	}

	SendTo(GetX(), GetY(), GetZ());
	SaveGuardSpot(GetPosition());
}

void Bot::AI_Bot_Init()
{
	AIautocastspell_timer.reset(nullptr);
	casting_spell_AIindex = static_cast<uint8>(AIBot_spells.size());

	roambox_max_x = 0;
	roambox_max_y = 0;
	roambox_min_x = 0;
	roambox_min_y = 0;
	roambox_distance = 0;
	roambox_destination_x = 0;
	roambox_destination_y = 0;
	roambox_destination_z = 0;
	roambox_min_delay = 2500;
	roambox_delay = 2500;
}

void Bot::SpellProcess() {
	if(spellend_timer.Check(false))	{
		NPC::SpellProcess();
		if(GetClass() == BARD) {
			if (casting_spell_id != 0)
				casting_spell_id = 0;
		}
	}
}

void Bot::BotMeditate(bool isSitting) {
	if (isSitting) {
		if (GetManaRatio() < 99.0f || GetHPRatio() < 99.0f) {
			if (!IsEngaged() && !IsSitting()) {
				Sit();
			}
		} else {
			if (IsSitting()) {
				Stand();
			}
		}
	} else {
		if (IsSitting()) {
			Stand();
		}
	}
}

void Bot::BotRangedAttack(Mob* other) {
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check())) {
		LogCombatDetail("Bot Archery attack canceled. Timer not up. Attack [{}] ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	EQ::ItemInstance* rangedItem = GetBotItem(EQ::invslot::slotRange);
	const EQ::ItemData* RangeWeapon = nullptr;
	if(rangedItem)
		RangeWeapon = rangedItem->GetItem();

	EQ::ItemInstance* ammoItem = GetBotItem(EQ::invslot::slotAmmo);
	const EQ::ItemData* Ammo = nullptr;
	if(ammoItem)
		Ammo = ammoItem->GetItem();

	if(!RangeWeapon || !Ammo)
		return;

	LogCombatDetail("Shooting [{}] with bow [{}] ([{}]) and arrow [{}] ([{}])", other->GetCleanName(), RangeWeapon->Name, RangeWeapon->ID, Ammo->Name, Ammo->ID);
	if(!IsAttackAllowed(other) || IsCasting() || DivineAura() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead))
		return;

	SendItemAnimation(other, Ammo, EQ::skills::SkillArchery);
	//DoArcheryAttackDmg(GetTarget(), rangedItem, ammoItem);
	DoArcheryAttackDmg(other, rangedItem, ammoItem); // watch

	//break invis when you attack
	if(invisible) {
		LogCombatDetail("Removing invisibility due to melee attack");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}

	if(invisible_undead) {
		LogCombatDetail("Removing invisibility vs. undead due to melee attack");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}

	if(invisible_animals) {
		LogCombatDetail("Removing invisibility vs. animals due to melee attack");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if (hidden || improved_hidden) {
		hidden = false;
		improved_hidden = false;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
}

bool Bot::CheckBotDoubleAttack(bool tripleAttack) {
	//Check for bonuses that give you a double attack chance regardless of skill (ie Bestial Frenzy/Harmonious Attack AA)
	uint32 bonusGiveDA = (aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack);
	// If you don't have the double attack skill, return
	if (!GetSkill(EQ::skills::SkillDoubleAttack) && !(GetClass() == BARD || GetClass() == BEASTLORD))
		return false;

	// You start with no chance of double attacking
	float chance = 0.0f;
	uint16 skill = GetSkill(EQ::skills::SkillDoubleAttack);
	int32 bonusDA = (aabonuses.DoubleAttackChance + spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance);
	//Use skill calculations otherwise, if you only have AA applied GiveDoubleAttack chance then use that value as the base.
	if (skill)
		chance = ((float(skill + GetLevel()) * (float(100.0f + bonusDA + bonusGiveDA) / 100.0f)) / 500.0f);
	else
		chance = ((float(bonusGiveDA) * (float(100.0f + bonusDA) / 100.0f)) / 100.0f);

	//Live now uses a static Triple Attack skill (lv 46 = 2% lv 60 = 20%) - We do not have this skill on EMU ATM.
	//A reasonable forumla would then be TA = 20% * chance
	//AA's can also give triple attack skill over cap. (ie Burst of Power) NOTE: Skill ID in spell data is 76 (Triple Attack)
	//Kayen: Need to decide if we can implement triple attack skill before working in over the cap effect.
	if(tripleAttack) {
		// Only some Double Attack classes get Triple Attack [This is already checked in client_processes.cpp]
		int32 triple_bonus = (spellbonuses.TripleAttackChance + itembonuses.TripleAttackChance);
		chance *= 0.2f; //Baseline chance is 20% of your double attack chance.
		chance *= (float(100.0f + triple_bonus) / 100.0f); //Apply modifiers.
	}

	if((zone->random.Real(0, 1) < chance))
		return true;

	return false;
}

bool Bot::CanDoSpecialAttack(Mob *other) {
	//Make sure everything is valid before doing any attacks.
	if (!other) {
		SetTarget(nullptr);
		return false;
	}

	if(!GetTarget())
		SetTarget(other);

	if ((other == nullptr || ((GetAppearance() == eaDead) || (other->IsClient() && other->CastToClient()->IsDead())) || HasDied() || (!IsAttackAllowed(other))) || other->GetInvul() || other->GetSpecialAbility(IMMUNE_MELEE))
		return false;

	return true;
}

void Bot::SetTarget(Mob* mob) {
	if(mob != this) {
		if(mob != GetTarget())
			_previousTarget = GetTarget();

		NPC::SetTarget(mob);
	}
}

void Bot::SetStopMeleeLevel(uint8 level) {
	if (IsCasterClass(GetClass()) || IsHybridClass(GetClass()))
		_stopMeleeLevel = level;
	else
		_stopMeleeLevel = 255;
}

void Bot::SetGuardMode() {

	StopMoving();
	m_GuardPoint = GetPosition();
	SetGuardFlag();

	if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {
		GetPet()->StopMoving();
	}
}

void Bot::SetHoldMode() {

	SetHoldFlag();
}

// AI Processing for the Bot object

constexpr float MAX_CASTER_DISTANCE[PLAYER_CLASS_COUNT] = {
    0, (34 * 34), (24 * 24), (28 * 28), (26 * 26), (42 * 42), 0, (30 * 30), 0, (38 * 38), (54 * 54), (48 * 48), (52 * 52), (50 * 50), (32 * 32), 0
//  W      C          P          R          S          D      M      B      R      S          N          W          M          E          B      B
//  A      L          A          N          H          R      N      R      O      H          E          I          A          N          S      E
//  R      R          L          G          D          U      K      D      G      M          C          Z          G          C          T      R
};

void Bot::AI_Process()
{
#define TEST_COMBATANTS() if (!GetTarget() || GetAppearance() == eaDead) { return; }
#define PULLING_BOT (GetPullingFlag() || GetReturningFlag())
#define NOT_PULLING_BOT (!GetPullingFlag() && !GetReturningFlag())
#define GUARDING (GetGuardFlag())
#define NOT_GUARDING (!GetGuardFlag())
#define HOLDING (GetHoldFlag())
#define NOT_HOLDING (!GetHoldFlag())
#define PASSIVE (GetBotStance() == EQ::constants::stancePassive)
#define NOT_PASSIVE (GetBotStance() != EQ::constants::stancePassive)

	Client* bot_owner = (GetBotOwner() && GetBotOwner()->IsClient() ? GetBotOwner()->CastToClient() : nullptr);
	Group* bot_group = GetGroup();

//#pragma region PRIMARY AI SKIP CHECKS

	// Primary reasons for not processing AI
	if (!bot_owner || !bot_group || !IsAIControlled()) {
		return;
	}

	if (bot_owner->IsDead()) {

		SetTarget(nullptr);
		SetBotOwner(nullptr);

		return;
	}

	// We also need a leash owner and follow mob (subset of primary AI criteria)
	Client* leash_owner = (bot_group->GetLeader() && bot_group->GetLeader()->IsClient() ? bot_group->GetLeader()->CastToClient() : bot_owner);
	if (!leash_owner) {
		return;
	}

//#pragma endregion

	Mob* follow_mob = entity_list.GetMob(GetFollowID());
	if (!follow_mob) {

		follow_mob = leash_owner;
		SetFollowID(leash_owner->GetID());
	}

	// Berserk updates should occur if primary AI criteria are met
	if (GetClass() == WARRIOR || GetClass() == BERSERKER) {

		if (!berserk && GetHP() > 0 && GetHPRatio() < 30.0f) {

			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_START, GetName());
			berserk = true;
		}

		if (berserk && GetHPRatio() >= 30.0f) {

			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_END, GetName());
			berserk = false;
		}
	}

//#pragma region SECONDARY AI SKIP CHECKS

	// Secondary reasons for not processing AI
	if (GetPauseAI() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead)) {

		if (IsCasting()) {
			InterruptSpell();
		}

		if (IsMyHealRotationSet() || (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this)) {

			AdvanceHealRotation(false);
			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
		}

		return;
	}

//#pragma endregion

	float fm_distance = DistanceSquared(m_Position, follow_mob->GetPosition());
	float lo_distance = DistanceSquared(m_Position, leash_owner->GetPosition());
	float leash_distance = RuleR(Bots, LeashDistance);

//#pragma region CURRENTLY CASTING CHECKS

	if (IsCasting()) {

		if (IsHealRotationMember() &&
			m_member_of_heal_rotation->CastingOverride() &&
			m_member_of_heal_rotation->CastingTarget() != nullptr &&
			m_member_of_heal_rotation->CastingReady() &&
			m_member_of_heal_rotation->CastingMember() == this &&
			!m_member_of_heal_rotation->MemberIsCasting(this))
		{
			InterruptSpell();
		}
		else if (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this) {

			AdvanceHealRotation(false);
			return;
		}
		else if (GetClass() != BARD) {

			if (IsEngaged()) {
				return;
			}

			if (
				(NOT_GUARDING && fm_distance > GetFollowDistance()) || // Cancel out-of-combat casting if movement to follow mob is required
				(GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) > GetFollowDistance()) // Cancel out-of-combat casting if movement to guard point is required
			) {
				InterruptSpell();
			}

			return;
		}
	}
	else if (IsHealRotationMember()) {
		m_member_of_heal_rotation->SetMemberIsCasting(this, false);
	}

//#pragma endregion

	// Can't move if rooted...
	if (IsRooted() && IsMoving()) {

		StopMoving();
		return;
	}

//#pragma region HEAL ROTATION CASTING CHECKS

	if (IsMyHealRotationSet()) {

		if (AIHealRotation(HealRotationTarget(), UseHealRotationFastHeals())) {

			m_member_of_heal_rotation->SetMemberIsCasting(this);
			m_member_of_heal_rotation->UpdateTargetHealingStats(HealRotationTarget());
			AdvanceHealRotation();
		}
		else {

			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
			AdvanceHealRotation(false);
		}
	}

//#pragma endregion

	bool bo_alt_combat = (RuleB(Bots, AllowOwnerOptionAltCombat) && bot_owner->GetBotOption(Client::booAltCombat));

//#pragma region ATTACK FLAG

	if (GetAttackFlag()) { // Push owner's target onto our hate list

		if (GetPet() && PULLING_BOT) {
			GetPet()->SetPetOrder(m_previous_pet_order);
		}

		SetAttackFlag(false);
		SetAttackingFlag(false);
		SetPullFlag(false);
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (NOT_HOLDING && NOT_PASSIVE) {

			auto attack_target = bot_owner->GetTarget();
			if (attack_target) {

				InterruptSpell();
				WipeHateList();
				AddToHateList(attack_target, 1);
				SetTarget(attack_target);
				SetAttackingFlag();
				if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

					GetPet()->WipeHateList();
					GetPet()->AddToHateList(attack_target, 1);
					GetPet()->SetTarget(attack_target);
				}
			}
		}
	}

//#pragma endregion

//#pragma region PULL FLAG

	else if (GetPullFlag()) { // Push owner's target onto our hate list and set flags so other bots do not aggro

		SetAttackFlag(false);
		SetAttackingFlag(false);
		SetPullFlag(false);
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (NOT_HOLDING && NOT_PASSIVE) {

			auto pull_target = bot_owner->GetTarget();
			if (pull_target) {

				BotGroupSay(
					this,
					fmt::format(
						"Pulling {}.",
						pull_target->GetCleanName()
					).c_str()
				);
				InterruptSpell();
				WipeHateList();
				AddToHateList(pull_target, 1);
				SetTarget(pull_target);
				SetPullingFlag();
				bot_owner->SetBotPulling();
				if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

					GetPet()->WipeHateList();
					GetPet()->SetTarget(nullptr);
					m_previous_pet_order = GetPet()->GetPetOrder();
					GetPet()->SetPetOrder(SPO_Guard);
				}
			}
		}
	}

//#pragma endregion

//#pragma region ALT COMBAT (ACQUIRE HATE)

	else if (bo_alt_combat && m_alt_combat_hate_timer.Check(false)) { // 'Alt Combat' gives some more 'control' options on how bots process aggro

		// Empty hate list - let's find some aggro
		if (!IsEngaged() && NOT_HOLDING && NOT_PASSIVE && (!bot_owner->GetBotPulling() || NOT_PULLING_BOT)) {

			Mob* lo_target = leash_owner->GetTarget();
			if (lo_target &&
				lo_target->IsNPC() &&
				!lo_target->IsMezzed() &&
				((bot_owner->GetBotOption(Client::booAutoDefend) && lo_target->GetHateAmount(leash_owner)) || leash_owner->AutoAttackEnabled()) &&
				lo_distance <= leash_distance &&
				DistanceSquared(m_Position, lo_target->GetPosition()) <= leash_distance &&
				(CheckLosFN(lo_target) || leash_owner->CheckLosFN(lo_target)) &&
				IsAttackAllowed(lo_target))
			{
				AddToHateList(lo_target, 1);
				if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

					GetPet()->AddToHateList(lo_target, 1);
					GetPet()->SetTarget(lo_target);
				}
			}
			else {

				for (int counter = 0; counter < bot_group->GroupCount(); counter++) {

					Mob* bg_member = bot_group->members[counter];
					if (!bg_member) {
						continue;
					}

					Mob* bgm_target = bg_member->GetTarget();
					if (!bgm_target || !bgm_target->IsNPC()) {
						continue;
					}

					if (!bgm_target->IsMezzed() &&
						((bot_owner->GetBotOption(Client::booAutoDefend) && bgm_target->GetHateAmount(bg_member)) || leash_owner->AutoAttackEnabled()) &&
						lo_distance <= leash_distance &&
						DistanceSquared(m_Position, bgm_target->GetPosition()) <= leash_distance &&
						(CheckLosFN(bgm_target) || leash_owner->CheckLosFN(bgm_target)) &&
						IsAttackAllowed(bgm_target))
					{
						AddToHateList(bgm_target, 1);
						if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

							GetPet()->AddToHateList(bgm_target, 1);
							GetPet()->SetTarget(bgm_target);
						}

						break;
					}
				}
			}
		}
	}

//#pragma endregion

	glm::vec3 Goal(0, 0, 0);

	// We have aggro to choose from
	if (IsEngaged()) {

		if (rest_timer.Enabled()) {
			rest_timer.Disable();
		}

//#pragma region PULLING FLAG (TARGET VALIDATION)

		if (GetPullingFlag()) {

			if (!GetTarget()) {

				WipeHateList();
				SetTarget(nullptr);
				SetPullingFlag(false);
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}

				return;
			}
			else if (GetTarget()->GetHateList().size()) {

				WipeHateList();
				SetTarget(nullptr);
				SetPullingFlag(false);
				SetReturningFlag();

				return;
			}
			else {
				// Default action is to aggress towards enemy
			}
		}

//#pragma endregion

//#pragma region RETURNING FLAG

		else if (GetReturningFlag()) {

			// Need to make it back to group before clearing return flag
			if (fm_distance <= GetFollowDistance()) {

				// Once we're back, clear blocking flags so everyone else can join in
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}
			}

			// Need to keep puller out of combat until they reach their 'return to' destination
			if (HasTargetReflection()) {

				SetTarget(nullptr);
				WipeHateList();

				return;
			}
		}

//#pragma endregion

//#pragma region ALT COMBAT (ACQUIRE TARGET)

		else if (bo_alt_combat && m_alt_combat_hate_timer.Check()) { // Find a mob from hate list to target

			// Group roles can be expounded upon in the future
			auto assist_mob = entity_list.GetMob(bot_group->GetMainAssistName());
			bool find_target = true;

			if (assist_mob) {
				if (assist_mob->GetTarget()) {
					if (assist_mob != this) {
						if (GetTarget() != assist_mob->GetTarget()) {
							SetTarget(assist_mob->GetTarget());
						}

						if (
							HasPet() &&
							(
								GetClass() != ENCHANTER ||
								GetPet()->GetPetType() != petAnimation ||
								GetAA(aaAnimationEmpathy) >= 2
							)
						) {
							// This artificially inflates pet's target aggro..but, less expensive than checking hate each AI process
							GetPet()->AddToHateList(assist_mob->GetTarget(), 1);
							GetPet()->SetTarget(assist_mob->GetTarget());
						}
					}

					find_target = false;
				} else if (assist_mob != this) {
					if (GetTarget()) {
						SetTarget(nullptr);
					}

					if (
						HasPet() &&
						(
							GetClass() != ENCHANTER ||
							GetPet()->GetPetType() != petAnimation ||
							GetAA(aaAnimationEmpathy) >= 1
						)
					) {
						GetPet()->WipeHateList();
						GetPet()->SetTarget(nullptr);
					}

					find_target = false;
				}
			}

			if (find_target) {
				if (IsRooted()) {
					auto closest = hate_list.GetClosestEntOnHateList(this, true);
					if (closest) {
						SetTarget(closest);
					}
				} else {
					// This will keep bots on target for now..but, future updates will allow for rooting/stunning
					auto escaping = hate_list.GetEscapingEntOnHateList(leash_owner, leash_distance);
					if (escaping) {
						SetTarget(escaping);
					}

					if (!GetTarget()) {
						auto most_hate = hate_list.GetEntWithMostHateOnList(this, nullptr, true);
						if (most_hate) {
							SetTarget(most_hate);
						}
					}
				}
			}
		}

//#pragma endregion

//#pragma region DEFAULT (ACQUIRE TARGET)

		else {

			// Default behavior doesn't have a means of acquiring a target from the bot's hate list..
			// ..that action occurs through commands or out-of-combat checks
			// (Use current target, if already in combat)
		}

//#pragma endregion

//#pragma region VERIFY TARGET AND STANCE

		Mob* tar = GetTarget(); // We should have a target..if not, we're awaiting new orders
		if (!tar || PASSIVE) {
			if (GetTarget()) {
				SetTarget(nullptr);
			}

			WipeHateList();
			SetAttackFlag(false);
			SetAttackingFlag(false);
			if (PULLING_BOT) {

				// 'Flags' should only be set on the bot that is pulling
				SetPullingFlag(false);
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}
			}

			if (GetArchetype() == ARCHETYPE_CASTER) {
				BotMeditate(true);
			}

			return;
		}

//#pragma endregion

//#pragma region ATTACKING FLAG (HATE VALIDATION)

		if (GetAttackingFlag() && tar->CheckAggro(this)) {
			SetAttackingFlag(false);
		}

//#pragma endregion

		float tar_distance = DistanceSquared(m_Position, tar->GetPosition());

//#pragma region TARGET VALIDATION

		// DOUBLE-CHECK THIS CRITERIA

		// Verify that our target has attackable criteria
		if (HOLDING ||
			!tar->IsNPC() ||
			tar->IsMezzed() ||
			lo_distance > leash_distance ||
			tar_distance > leash_distance ||
			(!GetAttackingFlag() && !CheckLosFN(tar) && !leash_owner->CheckLosFN(tar)) || // This is suppose to keep bots from attacking things behind walls
			!IsAttackAllowed(tar) ||
			(bo_alt_combat &&
				(!GetAttackingFlag() && NOT_PULLING_BOT && !leash_owner->AutoAttackEnabled() && !tar->GetHateAmount(this) && !tar->GetHateAmount(leash_owner))
			)
		)
		{
			// Normally, we wouldn't want to do this without class checks..but, too many issues can arise if we let enchanter animation pets run rampant
			if (HasPet()) {

				GetPet()->RemoveFromHateList(tar);
				GetPet()->SetTarget(nullptr);
			}

			RemoveFromHateList(tar);
			SetTarget(nullptr);

			SetAttackFlag(false);
			SetAttackingFlag(false);
			if (PULLING_BOT) {

				SetPullingFlag(false);
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}
			}

			if (IsMoving()) {
				StopMoving();
			}

			return;
		}

//#pragma endregion

		// This causes conflicts with default pet handler (bounces between targets)
		if (NOT_PULLING_BOT && HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

			// We don't add to hate list here because it's assumed to already be on the list
			GetPet()->SetTarget(tar);
		}

		if (DivineAura()) {
			return;
		}

		if (!(m_PlayerState & static_cast<uint32>(PlayerState::Aggressive))) {
			SendAddPlayerState(PlayerState::Aggressive);
		}

//#pragma region PULLING FLAG (ACTIONABLE RANGE)

		if (GetPullingFlag()) {

			constexpr size_t PULL_AGGRO = 5225; // spells[5225]: 'Throw Stone' - 0 cast time

			if (tar_distance <= (spells[PULL_AGGRO].range * spells[PULL_AGGRO].range)) {

				StopMoving();
				CastSpell(PULL_AGGRO, tar->GetID());
				return;
			}
		}

//#pragma endregion

//#pragma region COMBAT RANGE CALCS

		bool atCombatRange = false;

		const auto* p_item = GetBotItem(EQ::invslot::slotPrimary);
		const auto* s_item = GetBotItem(EQ::invslot::slotSecondary);

		bool behind_mob = false;
		bool backstab_weapon = false;
		if (GetClass() == ROGUE) {

			behind_mob = BehindMob(tar, GetX(), GetY()); // Can be separated for other future use
			backstab_weapon = p_item && p_item->GetItemBackstabDamage();
		}

		// Calculate melee distances
		float melee_distance_max = 0.0f;
		float melee_distance = 0.0f;
		{
			float size_mod = GetSize();
			float other_size_mod = tar->GetSize();

			if (GetRace() == RT_DRAGON || GetRace() == RT_WURM || GetRace() == RT_DRAGON_7) { // For races with a fixed size
				size_mod = 60.0f;
			}
			else if (size_mod < 6.0f) {
				size_mod = 8.0f;
			}

			if (tar->GetRace() == RT_DRAGON || tar->GetRace() == RT_WURM || tar->GetRace() == RT_DRAGON_7) { // For races with a fixed size
				other_size_mod = 60.0f;
			}
			else if (other_size_mod < 6.0f) {
				other_size_mod = 8.0f;
			}

			if (other_size_mod > size_mod) {
				size_mod = other_size_mod;
			}

			if (size_mod > 29.0f) {
				size_mod *= size_mod;
			}
			else if (size_mod > 19.0f) {
				size_mod *= (size_mod * 2.0f);
			}
			else {
				size_mod *= (size_mod * 4.0f);
			}

			// Prevention of ridiculously sized hit boxes
			if (size_mod > 10000.0f) {
				size_mod = (size_mod / 7.0f);
			}

			melee_distance_max = size_mod;

			switch (GetClass()) {
			case WARRIOR:
			case PALADIN:
			case SHADOWKNIGHT:
				if (p_item && p_item->GetItem()->IsType2HWeapon()) {
					melee_distance = melee_distance_max * 0.45f;
				}
				else if ((s_item && s_item->GetItem()->IsTypeShield()) || (!p_item && !s_item)) {
					melee_distance = melee_distance_max * 0.35f;
				}
				else {
					melee_distance = melee_distance_max * 0.40f;
				}

				break;
			case NECROMANCER:
			case WIZARD:
			case MAGICIAN:
			case ENCHANTER:
				if (p_item && p_item->GetItem()->IsType2HWeapon()) {
					melee_distance = melee_distance_max * 0.95f;
				}
				else {
					melee_distance = melee_distance_max * 0.75f;
				}

				break;
			case ROGUE:
				if (behind_mob && backstab_weapon) {
					if (p_item->GetItem()->IsType2HWeapon()) { // 'p_item' tested in 'backstab_weapon' check above
						melee_distance = melee_distance_max * 0.30f;
					}
					else {
						melee_distance = melee_distance_max * 0.25f;
					}

					break;
				}
				// Fall-through
			default:
				if (p_item && p_item->GetItem()->IsType2HWeapon()) {
					melee_distance = melee_distance_max * 0.70f;
				}
				else {
					melee_distance = melee_distance_max * 0.50f;
				}

				break;
			}
		}
		float melee_distance_min = melee_distance / 2.0f;

		// Calculate caster distances
		float caster_distance_max = 0.0f;
		float caster_distance_min = 0.0f;
		float caster_distance = 0.0f;
		{
			if (GetLevel() >= GetStopMeleeLevel() && GetClass() >= WARRIOR && GetClass() <= BERSERKER) {
				caster_distance_max = MAX_CASTER_DISTANCE[(GetClass() - 1)];
			}

			if (caster_distance_max) {

				caster_distance_min = melee_distance_max;
				if (caster_distance_max <= caster_distance_min) {
					caster_distance_max = caster_distance_min * 1.25f;
				}

				caster_distance = ((caster_distance_max + caster_distance_min) / 2);
			}
		}

		bool atArcheryRange = IsArcheryRange(tar);

		if (GetRangerAutoWeaponSelect()) {

			bool changeWeapons = false;

			if (atArcheryRange && !IsBotArcher()) {

				SetBotArcherySetting(true);
				changeWeapons = true;
			}
			else if (!atArcheryRange && IsBotArcher()) {

				SetBotArcherySetting(false);
				changeWeapons = true;
			}

			if (changeWeapons) {
				ChangeBotArcherWeapons(IsBotArcher());
			}
		}

		if (IsBotArcher() && atArcheryRange) {
			atCombatRange = true;
		}
		else if (caster_distance_max && tar_distance <= caster_distance_max) {
			atCombatRange = true;
		}
		else if (tar_distance <= melee_distance) {
			atCombatRange = true;
		}

//#pragma endregion

//#pragma region ENGAGED AT COMBAT RANGE

		// We can fight
		if (atCombatRange) {

			//if (IsMoving() || GetCombatJitterFlag()) { // StopMoving() needs to be called so that the jitter timer can be reset
			if (IsMoving()) {

				// Since we're using a pseudo-shadowstep for jitter, disregard the combat jitter flag
				//if (!GetCombatJitterFlag()) {
					StopMoving(CalculateHeadingToTarget(tar->GetX(), tar->GetY()));
				//}

				return;
			}

			// Combat 'jitter' code
			// Note: Combat Jitter is disabled until a working movement solution can be found
			if (AI_movement_timer->Check() && (!spellend_timer.Enabled() || GetClass() == BARD)) {

				if (!IsRooted()) {

					if (HasTargetReflection()) {

						if (!tar->IsFeared() && !tar->IsStunned()) {

							if (GetClass() == ROGUE) {

								if (m_evade_timer.Check(false)) { // Attempt to evade

									int timer_duration = (HideReuseTime - GetSkillReuseTime(EQ::skills::SkillHide)) * 1000;
									if (timer_duration < 0) {
										timer_duration = 0;
									}

									m_evade_timer.Start(timer_duration);
									if (zone->random.Int(0, 260) < (int)GetSkill(EQ::skills::SkillHide)) {
										RogueEvade(tar);
									}

									return;
								}
							}

							//if (tar->IsRooted()) { // Move caster/rogue back from rooted mob - out of combat range, if necessary

							//	if (GetArchetype() == ARCHETYPE_CASTER || GetClass() == ROGUE) {

							//		if (tar_distance <= melee_distance_max) {

							//			if (PlotPositionAroundTarget(this, Goal.x, Goal.y, Goal.z)) {
							//			//if (PlotPositionBehindMeFacingTarget(tar, Goal.x, Goal.y, Goal.z)) {

							//				Teleport(Goal);
							//				//WalkTo(Goal.x, Goal.y, Goal.z);
							//				SetCombatJitterFlag();

							//				return;
							//			}
							//		}
							//	}
							//}
						}
					}
					//else {

					//	if (caster_distance_min && tar_distance < caster_distance_min && !tar->IsFeared()) { // Caster back-off adjustment

					//		if (PlotPositionAroundTarget(this, Goal.x, Goal.y, Goal.z)) {
					//		//if (PlotPositionBehindMeFacingTarget(tar, Goal.x, Goal.y, Goal.z)) {

					//			if (DistanceSquared(Goal, tar->GetPosition()) <= caster_distance_max) {

					//				Teleport(Goal);
					//				//WalkTo(Goal.x, Goal.y, Goal.z);
					//				SetCombatJitterFlag();

					//				return;
					//			}
					//		}
					//	}
					//	else if (tar_distance < melee_distance_min) { // Melee back-off adjustment

					//		if (PlotPositionAroundTarget(this, Goal.x, Goal.y, Goal.z)) {
					//		//if (PlotPositionBehindMeFacingTarget(tar, Goal.x, Goal.y, Goal.z)) {

					//			if (DistanceSquared(Goal, tar->GetPosition()) <= melee_distance_max) {

					//				Teleport(Goal);
					//				//WalkTo(Goal.x, Goal.y, Goal.z);
					//				SetCombatJitterFlag();

					//				return;
					//			}
					//		}
					//	}
					//	else if (backstab_weapon && !behind_mob) { // Move the rogue to behind the mob

					//		if (PlotPositionAroundTarget(tar, Goal.x, Goal.y, Goal.z)) {
					//		//if (PlotPositionOnArcBehindTarget(tar, Goal.x, Goal.y, Goal.z, melee_distance)) {

					//			float distance_squared = DistanceSquared(Goal, tar->GetPosition());
					//			if (/*distance_squared >= melee_distance_min && */distance_squared <= melee_distance_max) {

					//				Teleport(Goal);
					//				//RunTo(Goal.x, Goal.y, Goal.z);
					//				SetCombatJitterFlag();

					//				return;
					//			}
					//		}
					//	}
					//	else if (m_combat_jitter_timer.Check()) {

					//		if (!caster_distance && PlotPositionAroundTarget(tar, Goal.x, Goal.y, Goal.z)) {
					//		//if (!caster_distance && PlotPositionOnArcInFrontOfTarget(tar, Goal.x, Goal.y, Goal.z, melee_distance)) {

					//			float distance_squared = DistanceSquared(Goal, tar->GetPosition());
					//			if (/*distance_squared >= melee_distance_min && */distance_squared <= melee_distance_max) {

					//				Teleport(Goal);
					//				//WalkTo(Goal.x, Goal.y, Goal.z);
					//				SetCombatJitterFlag();

					//				return;
					//			}
					//		}
					//		else if (caster_distance && PlotPositionAroundTarget(tar, Goal.x, Goal.y, Goal.z)) {
					//		//else if (caster_distance && PlotPositionOnArcInFrontOfTarget(tar, Goal.x, Goal.y, Goal.z, caster_distance)) {

					//			float distance_squared = DistanceSquared(Goal, tar->GetPosition());
					//			if (/*distance_squared >= caster_distance_min && */distance_squared <= caster_distance_max) {

					//				Teleport(Goal);
					//				//WalkTo(Goal.x, Goal.y, Goal.z);
					//				SetCombatJitterFlag();

					//				return;
					//			}
					//		}
					//	}

					//	if (!IsFacingMob(tar)) {

					//		FaceTarget(tar);
					//		return;
					//	}
					//}
				}
				else {

					if (!IsSitting() && !IsFacingMob(tar)) {

						FaceTarget(tar);
						return;
					}
				}
			}

			if (!IsBotNonSpellFighter() && AI_EngagedCastCheck()) {
				return;
			}

			// Up to this point, GetTarget() has been safe to dereference since the initial
			// TEST_COMBATANTS() call. Due to the chance of the target dying and our pointer
			// being nullified, we need to test it before dereferencing to avoid crashes

			if (IsBotArcher() && ranged_timer.Check(false)) { // Can shoot mezzed, stunned and dead!?

				TEST_COMBATANTS();
				if (GetTarget()->GetHPRatio() <= 99.0f) {
					BotRangedAttack(tar);
				}
			}
			else if (!IsBotArcher() && GetLevel() < GetStopMeleeLevel()) {

				// We can't fight if we don't have a target, are stun/mezzed or dead..
				// Stop attacking if the target is enraged
				TEST_COMBATANTS();
				if (tar->IsEnraged() && !BehindMob(tar, GetX(), GetY())) {
					return;
				}

				// First, special attack per class (kick, backstab etc..)
				TEST_COMBATANTS();
				DoClassAttacks(tar);

				TEST_COMBATANTS();
				if (attack_timer.Check()) { // Process primary weapon attacks

					Attack(tar, EQ::invslot::slotPrimary);

					TEST_COMBATANTS();
					TriggerDefensiveProcs(tar, EQ::invslot::slotPrimary, false);

					TEST_COMBATANTS();
					TryCombatProcs(p_item, tar, EQ::invslot::slotPrimary);

					// bool tripleSuccess = false;

					TEST_COMBATANTS();
					if (CanThisClassDoubleAttack()) {

						if (CheckBotDoubleAttack()) {
							Attack(tar, EQ::invslot::slotPrimary, true);
						}

						TEST_COMBATANTS();
						if (GetSpecialAbility(SPECATK_TRIPLE) && CheckBotDoubleAttack(true)) {
							// tripleSuccess = true;
							Attack(tar, EQ::invslot::slotPrimary, true);
						}

						TEST_COMBATANTS();
						// quad attack, does this belong here??
						if (GetSpecialAbility(SPECATK_QUAD) && CheckBotDoubleAttack(true)) {
							Attack(tar, EQ::invslot::slotPrimary, true);
						}
					}

					TEST_COMBATANTS();
					// Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
					int32 flurrychance = (aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance);
					if (flurrychance) {

						if (zone->random.Int(0, 100) < flurrychance) {

							MessageString(Chat::NPCFlurry, YOU_FLURRY);
							Attack(tar, EQ::invslot::slotPrimary, false);

							TEST_COMBATANTS();
							Attack(tar, EQ::invslot::slotPrimary, false);
						}
					}

					TEST_COMBATANTS();
					auto ExtraAttackChanceBonus =
						(spellbonuses.ExtraAttackChance[0] + itembonuses.ExtraAttackChance[0] +
						aabonuses.ExtraAttackChance[0]);
					if (ExtraAttackChanceBonus) {

						if (p_item && p_item->GetItem()->IsType2HWeapon()) {

							if (zone->random.Int(0, 100) < ExtraAttackChanceBonus) {
								Attack(tar, EQ::invslot::slotPrimary, false);
							}
						}
					}
				}

				TEST_COMBATANTS();
				if (attack_dw_timer.Check() && CanThisClassDualWield()) { // Process secondary weapon attacks

					const EQ::ItemData* s_itemdata = nullptr;
					// Can only dual wield without a weapon if you're a monk
					if (s_item || (GetClass() == MONK)) {

						if (s_item) {
							s_itemdata = s_item->GetItem();
						}

						int weapon_type = 0; // No weapon type.
						bool use_fist = true;
						if (s_itemdata) {

							weapon_type = s_itemdata->ItemType;
							use_fist = false;
						}

						if (use_fist || !s_itemdata->IsType2HWeapon()) {

							float DualWieldProbability = 0.0f;

							int32 Ambidexterity = (aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity);
							DualWieldProbability = ((GetSkill(EQ::skills::SkillDualWield) + GetLevel() + Ambidexterity) / 400.0f); // 78.0 max

							int32 DWBonus = (spellbonuses.DualWieldChance + itembonuses.DualWieldChance);
							DualWieldProbability += (DualWieldProbability * float(DWBonus) / 100.0f);

							float random = zone->random.Real(0, 1);
							if (random < DualWieldProbability) { // Max 78% of DW

								Attack(tar, EQ::invslot::slotSecondary);	// Single attack with offhand

								TEST_COMBATANTS();
								TryCombatProcs(s_item, tar, EQ::invslot::slotSecondary);

								TEST_COMBATANTS();
								if (CanThisClassDoubleAttack() && CheckBotDoubleAttack()) {

									if (tar->GetHP() > -10) {
										Attack(tar, EQ::invslot::slotSecondary);	// Single attack with offhand
									}
								}
							}
						}
					}
				}
			}

			if (GetAppearance() == eaDead) {
				return;
			}
		}

//#pragma endregion

//#pragma region ENGAGED NOT AT COMBAT RANGE

		else { // To far away to fight (GetTarget() validity can be iffy below this point - including outer scopes)

			// This code actually gets processed when we are too far away from target and have not engaged yet, too
			if (/*!GetCombatJitterFlag() && */AI_movement_timer->Check() && (!spellend_timer.Enabled() || GetClass() == BARD)) { // Pursue processing

				if (GetTarget() && !IsRooted()) {

					LogAIDetail("Pursuing [{}] while engaged", GetTarget()->GetCleanName());
					Goal = GetTarget()->GetPosition();
					if (DistanceSquared(m_Position, Goal) <= leash_distance) {
						RunTo(Goal.x, Goal.y, Goal.z);
					}
					else {

						WipeHateList();
						SetTarget(nullptr);
						if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

							GetPet()->WipeHateList();
							GetPet()->SetTarget(nullptr);
						}
					}

					return;
				}
				else {

					if (IsMoving()) {
						StopMoving();
					}
					return;
				}
			}

			if (GetTarget() && GetTarget()->IsFeared() && !spellend_timer.Enabled() && AI_think_timer->Check()) {

				if (!IsFacingMob(GetTarget())) {
					FaceTarget(GetTarget());
				}

				// This is a mob that is fleeing either because it has been feared or is low on hitpoints
				AI_PursueCastCheck(); // This appears to always return true..can't trust for success/fail

				return;
			}
		} // End not in combat range

//#pragma endregion

		if (!IsMoving() && !spellend_timer.Enabled()) { // This may actually need work...

			if (GetTarget() && AI_EngagedCastCheck()) {
				BotMeditate(false);
			}
			else if (GetArchetype() == ARCHETYPE_CASTER) {
				BotMeditate(true);
			}

			return;
		}
	}
	else { // Out-of-combat behavior

		SetAttackFlag(false);
		SetAttackingFlag(false);
		if (!bot_owner->GetBotPulling()) {

			SetPullingFlag(false);
			SetReturningFlag(false);
		}

//#pragma region AUTO DEFEND

		// This is as close as I could get without modifying the aggro mechanics and making it an expensive process...
		// 'class Client' doesn't make use of hate_list...
		if (RuleB(Bots, AllowOwnerOptionAutoDefend) && bot_owner->GetBotOption(Client::booAutoDefend)) {

			if (!m_auto_defend_timer.Enabled()) {

				m_auto_defend_timer.Start(zone->random.Int(250, 1250)); // random timer to simulate 'awareness' (cuts down on scanning overhead)
				return;
			}

			if (m_auto_defend_timer.Check() && bot_owner->GetAggroCount()) {

				if (NOT_HOLDING && NOT_PASSIVE) {

					auto xhaters = bot_owner->GetXTargetAutoMgr();
					if (xhaters && !xhaters->empty()) {

						for (auto hater_iter : xhaters->get_list()) {

							if (!hater_iter.spawn_id) {
								continue;
							}

							if (bot_owner->GetBotPulling() && bot_owner->GetTarget() && hater_iter.spawn_id == bot_owner->GetTarget()->GetID()) {
								continue;
							}

							auto hater = entity_list.GetMob(hater_iter.spawn_id);
							if (hater && !hater->IsMezzed() && DistanceSquared(hater->GetPosition(), bot_owner->GetPosition()) <= leash_distance) {

								// This is roughly equivilent to npc attacking a client pet owner
								AddToHateList(hater, 1);
								SetTarget(hater);
								SetAttackingFlag();
								if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

									GetPet()->AddToHateList(hater, 1);
									GetPet()->SetTarget(hater);
								}

								m_auto_defend_timer.Disable();

								return;
							}
						}
					}
				}
			}
		}

//#pragma endregion

		SetTarget(nullptr);

		if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

			GetPet()->WipeHateList();
			GetPet()->SetTarget(nullptr);
		}

		if (m_PlayerState & static_cast<uint32>(PlayerState::Aggressive)) {
			SendRemovePlayerState(PlayerState::Aggressive);
		}

//#pragma region OK TO IDLE

		// Ok to idle
		if ((NOT_GUARDING && fm_distance <= GetFollowDistance()) || (GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) <= GetFollowDistance())) {

			if (!IsMoving() && AI_think_timer->Check() && !spellend_timer.Enabled()) {

				if (NOT_PASSIVE) {

					if (!AI_IdleCastCheck() && !IsCasting() && GetClass() != BARD) {
						BotMeditate(true);
					}
				}
				else {

					if (GetClass() != BARD) {
						BotMeditate(true);
					}
				}

				return;
			}
		}

		// Non-engaged movement checks
		if (AI_movement_timer->Check() && (!IsCasting() || GetClass() == BARD)) {

			if (GUARDING) {
				Goal = GetGuardPoint();
			}
			else {
				Goal = follow_mob->GetPosition();
			}
			float destination_distance = DistanceSquared(GetPosition(), Goal);

			if ((!bot_owner->GetBotPulling() || PULLING_BOT) && (destination_distance > GetFollowDistance())) {

				if (!IsRooted()) {

					if (rest_timer.Enabled()) {
						rest_timer.Disable();
					}

					bool running = true;

					if (destination_distance < GetFollowDistance() + BOT_FOLLOW_DISTANCE_WALK) {
						running = false;
					}

					if (running) {
						RunTo(Goal.x, Goal.y, Goal.z);
					}
					else {
						WalkTo(Goal.x, Goal.y, Goal.z);
					}

					return;
				}
			}
			else {

				if (IsMoving()) {

					StopMoving();
					return;
				}
			}
		}

		// Basically, bard bots get a chance to cast idle spells while moving
		if (GetClass() == BARD && IsMoving() && NOT_PASSIVE) {

			if (!spellend_timer.Enabled() && AI_think_timer->Check()) {

				AI_IdleCastCheck();
				return;
			}
		}

//#pragma endregion

	}

#undef TEST_COMBATANTS
#undef PULLING_BOT
#undef NOT_PULLING_BOT
#undef GUARDING
#undef NOT_GUARDING
#undef HOLDING
#undef NOT_HOLDING
#undef PASSIVE
#undef NOT_PASSIVE
}

// AI Processing for a Bot object's pet
void Bot::PetAIProcess() {
	if( !HasPet() || !GetPet() || !GetPet()->IsNPC())
		return;

	Mob* BotOwner = GetBotOwner();
	NPC* botPet = GetPet()->CastToNPC();
	if(!botPet->GetOwner() || !botPet->GetID() || !botPet->GetOwnerID()) {
		Kill();
		return;
	}

	if (!botPet->IsAIControlled() || botPet->GetAttackTimer().Check(false) || botPet->IsCasting() || !botPet->GetOwner()->IsBot())
		return;

	if (IsEngaged()) {
		if (botPet->IsRooted())
			botPet->SetTarget(hate_list.GetClosestEntOnHateList(botPet));
		else
			botPet->SetTarget(hate_list.GetEntWithMostHateOnList(botPet));

		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// It causes some cpu stress but without it, it was causing the bot/pet to aggro behind wall, floor etc...
		if(!botPet->CheckLosFN(botPet->GetTarget()) || botPet->GetTarget()->IsMezzed() || !botPet->IsAttackAllowed(GetTarget())) {
			botPet->WipeHateList();
			botPet->SetTarget(botPet->GetOwner());
			return;
		}

		botPet->FaceTarget(botPet->GetTarget());
		bool is_combat_range = botPet->CombatRange(botPet->GetTarget());
		// Ok, we're engaged, each class type has a special AI
		// Only melee class will go to melee. Casters and healers will stay behind, following the leader by default.
		// I should probably make the casters staying in place so they can cast..

		// Ok, we 're a melee or any other class lvl<12. Yes, because after it becomes hard to go in melee for casters.. even for bots..
		if(is_combat_range) {
			botPet->GetAIMovementTimer()->Check();
			if(botPet->IsMoving()) {
				botPet->SetHeading(botPet->GetTarget()->GetHeading());
				if(moved) {
					moved = false;
					botPet->SetRunAnimSpeed(0);
				}
			}

			if(!botPet->IsMoving()) {
				float newX = 0;
				float newY = 0;
				float newZ = 0;
				bool petHasAggro = false;
				if(botPet->GetTarget() && botPet->GetTarget()->GetHateTop() && botPet->GetTarget()->GetHateTop() == botPet)
					petHasAggro = true;

				if(botPet->GetClass() == ROGUE && !petHasAggro && !botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY())) {
					// Move the rogue to behind the mob
					if(botPet->PlotPositionAroundTarget(botPet->GetTarget(), newX, newY, newZ)) {
						botPet->RunTo(newX, newY, newZ);
						return;
					}
				}
				else if(GetTarget() == botPet->GetTarget() && !petHasAggro && !botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY())) {
					// If the bot owner and the bot are fighting the same mob, then move the pet to the rear arc of the mob
					if(botPet->PlotPositionAroundTarget(botPet->GetTarget(), newX, newY, newZ)) {
						botPet->RunTo(newX, newY, newZ);
						return;
					}
				}
				else if(DistanceSquaredNoZ(botPet->GetPosition(), botPet->GetTarget()->GetPosition()) < botPet->GetTarget()->GetSize()) {
					// Let's try to adjust our melee range so we don't appear to be bunched up
					bool isBehindMob = false;
					bool moveBehindMob = false;
					if(botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY()))
						isBehindMob = true;

					if (!isBehindMob && !petHasAggro)
						moveBehindMob = true;

					if(botPet->PlotPositionAroundTarget(botPet->GetTarget(), newX, newY, newZ, moveBehindMob)) {
						botPet->RunTo(newX, newY, newZ);
						return;
					}
				}
			}

			// we can't fight if we don't have a target, are stun/mezzed or dead..
			if(botPet->GetTarget() && !botPet->IsStunned() && !botPet->IsMezzed() && (botPet->GetAppearance() != eaDead)) {
				// check the delay on the attack
				if(botPet->GetAttackTimer().Check()) {
					// Stop attacking while we are on a front arc and the target is enraged
					if(!botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY()) && botPet->GetTarget()->IsEnraged())
						return;

					if (botPet->Attack(GetTarget(), EQ::invslot::slotPrimary))	// try the main hand
						if (botPet->GetTarget()) {
							// We're a pet so we re able to dual attack
							int32 RandRoll = zone->random.Int(0, 99);
							if (botPet->CanThisClassDoubleAttack() && (RandRoll < (botPet->GetLevel() + NPCDualAttackModifier))) {
								if (botPet->Attack(botPet->GetTarget(), EQ::invslot::slotPrimary)) {}
							}
						}

						if (botPet->GetOwner()->IsBot()) {
							int aa_chance = 0;
							int aa_skill = 0;
							// Magician AA
							aa_skill += botPet->GetOwner()->GetAA(aaElementalAlacrity);
							// Necromancer AA
							aa_skill += botPet->GetOwner()->GetAA(aaQuickeningofDeath);
							// Beastlord AA
							aa_skill += botPet->GetOwner()->GetAA(aaWardersAlacrity);
							if(aa_skill >= 1)
								aa_chance += ((aa_skill > 5 ? 5 : aa_skill) * 4);

							if(aa_skill >= 6)
								aa_chance += ((aa_skill - 5 > 3 ? 3 : aa_skill - 5) * 7);

							if(aa_skill >= 9)
								aa_chance += ((aa_skill - 8 > 3 ? 3 : aa_skill - 8) * 3);

							if(aa_skill >= 12)
								aa_chance += ((aa_skill - 11) * 1);


							//aa_chance += botPet->GetOwner()->GetAA(aaCompanionsAlacrity) * 3;

							if (zone->random.Int(1, 100) < aa_chance)
								Flurry(nullptr);
						}

						// Ok now, let's check pet's offhand.
						if (botPet->GetAttackDWTimer().Check() && botPet->GetOwnerID() && botPet->GetOwner() && ((botPet->GetOwner()->GetClass() == MAGICIAN) || (botPet->GetOwner()->GetClass() == NECROMANCER) || (botPet->GetOwner()->GetClass() == SHADOWKNIGHT) || (botPet->GetOwner()->GetClass() == BEASTLORD))) {
							if(botPet->GetOwner()->GetLevel() >= 24) {
								float DualWieldProbability = ((botPet->GetSkill(EQ::skills::SkillDualWield) + botPet->GetLevel()) / 400.0f);
								DualWieldProbability -= zone->random.Real(0, 1);
								if(DualWieldProbability < 0) {
									botPet->Attack(botPet->GetTarget(), EQ::invslot::slotSecondary);
									if (botPet->CanThisClassDoubleAttack()) {
										int32 RandRoll = zone->random.Int(0, 99);
										if (RandRoll < (botPet->GetLevel() + 20))
											botPet->Attack(botPet->GetTarget(), EQ::invslot::slotSecondary);
									}
								}
							}
						}
						if(!botPet->GetOwner())
							return;

						// Special attack
						botPet->DoClassAttacks(botPet->GetTarget());
				}
				// See if the pet can cast any spell
				botPet->AI_EngagedCastCheck();
			}
		} else {
			// Now, if we cannot reach our target
			if (!botPet->HateSummon()) {
				if(botPet->GetTarget() && botPet->AI_PursueCastCheck()) {}
				else if (botPet->GetTarget() && botPet->GetAIMovementTimer()->Check()) {
					botPet->SetRunAnimSpeed(0);
					if(!botPet->IsRooted()) {
						LogAIDetail("Pursuing [{}] while engaged", botPet->GetTarget()->GetCleanName());
						botPet->RunTo(botPet->GetTarget()->GetX(), botPet->GetTarget()->GetY(), botPet->GetTarget()->GetZ());
						return;
					} else {
						botPet->SetHeading(botPet->GetTarget()->GetHeading());
						if(moved) {
							moved = false;
							StopNavigation();
							botPet->StopNavigation();
						}
					}
				}
			}
		}
	} else {
		// Ok if we're not engaged, what's happening..
		if(botPet->GetTarget() != botPet->GetOwner())
			botPet->SetTarget(botPet->GetOwner());

		if(!IsMoving())
			botPet->AI_IdleCastCheck();

		if(botPet->GetAIMovementTimer()->Check()) {
			switch(pStandingPetOrder) {
				case SPO_Follow: {
					float dist = DistanceSquared(botPet->GetPosition(), botPet->GetTarget()->GetPosition());
					botPet->SetRunAnimSpeed(0);
					if(dist > 184) {
						botPet->RunTo(botPet->GetTarget()->GetX(), botPet->GetTarget()->GetY(), botPet->GetTarget()->GetZ());
						return;
					} else {
						botPet->SetHeading(botPet->GetTarget()->GetHeading());
						if(moved) {
							moved = false;
							StopNavigation();
							botPet->StopNavigation();
						}
					}
					break;
				}
				case SPO_Sit:
					botPet->SetAppearance(eaSitting);
					break;
				case SPO_Guard:
					botPet->NextGuardPosition();
					break;
			}
		}
	}
}

void Bot::Depop() {
	WipeHateList();
	entity_list.RemoveFromHateLists(this);
	if(HasGroup())
		Bot::RemoveBotFromGroup(this, GetGroup());

	if(HasPet())
		GetPet()->Depop();

	_botOwner = 0;
	_botOwnerCharacterID = 0;
	_previousTarget = 0;
	NPC::Depop(false);
}

bool Bot::Spawn(Client* botCharacterOwner) {
	if (
		GetBotID() &&
		_botOwnerCharacterID &&
		botCharacterOwner &&
		botCharacterOwner->CharacterID() == _botOwnerCharacterID
	) {
		// Rename the bot name to make sure that Mob::GetName() matches Mob::GetCleanName() so we dont have a bot named "Jesuschrist001"
		strcpy(name, GetCleanName());

		// Get the zone id this bot spawned in
		_lastZoneId = GetZoneID();

		// this change propagates to Bot::FillSpawnStruct()
		helmtexture = 0; //0xFF;
		texture = 0; //0xFF;

		if (Save()) {
			GetBotOwner()->CastToClient()->Message(
				Chat::White,
				fmt::format(
					"{} saved.",
					GetCleanName()
				).c_str()
			);
		} else {
			GetBotOwner()->CastToClient()->Message(
				Chat::White,
				fmt::format(
					"{} save failed!",
					GetCleanName()
				).c_str()
			);
		}

		// Spawn the bot at the bot owner's loc
		m_Position.x = botCharacterOwner->GetX();
		m_Position.y = botCharacterOwner->GetY();
		m_Position.z = botCharacterOwner->GetZ();

		// Make the bot look at the bot owner
		FaceTarget(botCharacterOwner);
		UpdateEquipmentLight();
		UpdateActiveLight();

		m_targetable = true;
		entity_list.AddBot(this, true, true);

		GetBotOwnerDataBuckets();
		GetBotDataBuckets();
		LoadBotSpellSettings();
		if (!AI_AddBotSpells(GetBotSpellID())) {
			GetBotOwner()->CastToClient()->Message(
				Chat::White,
				fmt::format(
					"Failed to load spells for '{}' (ID {}).",
					GetCleanName(),
					GetBotID()
				).c_str()
			);
		}

		// Load pet
		LoadPet();
		SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);
		ping_timer.Start(8000);
		// there is something askew with spawn struct appearance fields...
		// I re-enabled this until I can sort it out
		const auto& m = GetBotItemSlots();
		uint8 material_from_slot = 0xFF;
		for (int slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
			if (m.find(slot_id) != m.end()) {
				material_from_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id);
				if (material_from_slot != 0xFF) {
					SendWearChange(material_from_slot);
				}
			}
		}

		return true;
	}

	return false;
}

// Deletes the inventory record for the specified item from the database for this bot.
void Bot::RemoveBotItemBySlot(uint16 slot_id, std::string *error_message)
{
	if (!GetBotID()) {
		return;
	}

	if (!database.botdb.DeleteItemBySlot(GetBotID(), slot_id)) {
		*error_message = BotDatabase::fail::DeleteItemBySlot();
	}

	m_inv.DeleteItem(slot_id);
	UpdateEquipmentLight();
}

// Retrieves all the inventory records from the database for this bot.
void Bot::GetBotItems(EQ::InventoryProfile &inv, std::string* error_message)
{
	if (!GetBotID()) {
		return;
	}

	if (!database.botdb.LoadItems(GetBotID(), inv)) {
		*error_message = BotDatabase::fail::LoadItems();
		return;
	}

	UpdateEquipmentLight();
}

std::map<uint16, uint32> Bot::GetBotItemSlots()
{
	std::map<uint16, uint32> m;
	if (!GetBotID()) {
		return m;
	}

	if (!database.botdb.LoadItemSlots(GetBotID(), m)) {
		GetBotOwner()->CastToClient()->Message(
			Chat::White,
			fmt::format(
				"Failed to load inventory slots for {}.",
				GetCleanName()
			).c_str()
		);
	}

	return m;
}

// Returns the inventory record for this bot from the database for the specified equipment slot.
uint32 Bot::GetBotItemBySlot(uint16 slot_id)
{
	uint32 item_id = 0;
	if (!GetBotID()) {
		return item_id;
	}

	if (!database.botdb.LoadItemBySlot(GetBotID(), slot_id, item_id)) {
		if (GetBotOwner() && GetBotOwner()->IsClient()) {
			GetBotOwner()->CastToClient()->Message(
				Chat::White,
				fmt::format(
					"Failed to load slot ID {} for {}.",
					slot_id,
					GetCleanName()
				).c_str()
			);
		}
	}

	return item_id;
}

void Bot::SetLevel(uint8 in_level, bool command) {
	if(in_level > 0)
		Mob::SetLevel(in_level, command);
}

void Bot::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	if(ns) {
		Mob::FillSpawnStruct(ns, ForWho);
		ns->spawn.afk = 0;
		ns->spawn.lfg = 0;
		ns->spawn.anon = 0;
		ns->spawn.gm = 0;
		if(IsInAGuild())
			ns->spawn.guildID = GuildID();
		else
			ns->spawn.guildID = 0xFFFFFFFF;		// 0xFFFFFFFF = NO GUILD, 0 = Unknown Guild
		ns->spawn.is_npc = 0;				// 0=no, 1=yes
		ns->spawn.is_pet = 0;
		ns->spawn.guildrank = 0;
		ns->spawn.showhelm = GetShowHelm() ? 1 : 0;
		ns->spawn.flymode = 0;
		ns->spawn.size = 0;
		ns->spawn.NPC = 0;					// 0=player,1=npc,2=pc corpse,3=npc corpse
		UpdateActiveLight();
		ns->spawn.light = m_Light.Type[EQ::lightsource::LightActive];
		ns->spawn.helm = helmtexture; //(GetShowHelm() ? helmtexture : 0); //0xFF;
		ns->spawn.equip_chest2 = texture; //0xFF;
		ns->spawn.show_name = true;
		strcpy(ns->spawn.lastName, GetSurname().c_str());
		strcpy(ns->spawn.title, GetTitle().c_str());
		strcpy(ns->spawn.suffix, GetSuffix().c_str());
		const EQ::ItemData* item = nullptr;
		const EQ::ItemInstance* inst = nullptr;
		uint32 spawnedbotid = 0;
		spawnedbotid = GetBotID();
		for (int i = EQ::textures::textureBegin; i < EQ::textures::weaponPrimary; i++) {
			inst = GetBotItem(i);
			if (inst) {
				item = inst->GetItem();
				if (item != 0) {
					ns->spawn.equipment.Slot[i].Material = item->Material;
					ns->spawn.equipment.Slot[i].EliteModel = item->EliteMaterial;
					ns->spawn.equipment.Slot[i].HerosForgeModel = item->HerosForgeModel;
					if (armor_tint.Slot[i].Color)
						ns->spawn.equipment_tint.Slot[i].Color = armor_tint.Slot[i].Color;
					else
						ns->spawn.equipment_tint.Slot[i].Color = item->Color;
				} else {
					if (armor_tint.Slot[i].Color)
						ns->spawn.equipment_tint.Slot[i].Color = armor_tint.Slot[i].Color;
				}
			}
		}

		inst = GetBotItem(EQ::invslot::slotPrimary);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment.Primary.Material = atoi(&item->IDFile[2]);

				ns->spawn.equipment_tint.Primary.Color = GetEquipmentColor(EQ::textures::weaponPrimary);
			}
		}

		inst = GetBotItem(EQ::invslot::slotSecondary);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment.Secondary.Material = atoi(&item->IDFile[2]);

				ns->spawn.equipment_tint.Secondary.Color = GetEquipmentColor(EQ::textures::weaponSecondary);
			}
		}
	}
}

Bot* Bot::LoadBot(uint32 botID)
{
	Bot* loaded_bot = nullptr;
	if (!botID)
		return loaded_bot;

	if (!database.botdb.LoadBot(botID, loaded_bot)) // TODO: Consider update to message handler
		return loaded_bot;

	return loaded_bot;
}

// Load and spawn all zoned bots by bot owner character
void Bot::LoadAndSpawnAllZonedBots(Client* bot_owner) {
	if (bot_owner) {
		std::list<std::pair<uint32,std::string>> auto_spawn_botgroups;
		if (bot_owner->HasGroup()) {
			std::vector<int> bot_class_spawn_limits;
			std::vector<int> bot_class_spawned_count = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

			for (uint8 class_id = WARRIOR; class_id <= BERSERKER; class_id++) {
				auto bot_class_limit = bot_owner->GetBotSpawnLimit(class_id);
				bot_class_spawn_limits.push_back(bot_class_limit);
			}

			auto* g = bot_owner->GetGroup();
			if (g) {
				uint32 group_id = g->GetID();
				std::list<uint32> active_bots;

				auto spawned_bots_count = 0;
				auto bot_spawn_limit = bot_owner->GetBotSpawnLimit();

				if (!database.botdb.LoadAutoSpawnBotGroupsByOwnerID(bot_owner->CharacterID(), auto_spawn_botgroups)) {
					bot_owner->Message(Chat::White, "Failed to load auto spawn bot groups by group ID.");
					return;
				}

				for (const auto& botgroup : auto_spawn_botgroups) {
					Bot::SpawnBotGroupByName(bot_owner, botgroup.second, botgroup.first);
				}

				if (!database.botdb.LoadGroupedBotsByGroupID(bot_owner->CharacterID(), group_id, active_bots)) {
					bot_owner->Message(Chat::White, "Failed to load grouped bots by group ID.");
					return;
				}

				if (!active_bots.empty()) {
					for (const auto& bot_id : active_bots) {
						auto* b = Bot::LoadBot(bot_id);
						if (!b) {
							continue;
						}

						if (bot_spawn_limit >= 0 && spawned_bots_count >= bot_spawn_limit) {
							database.SetGroupID(b->GetCleanName(), 0, b->GetBotID());
							g->UpdatePlayer(bot_owner);
							continue;
						}

						auto spawned_bot_count_class = bot_class_spawned_count[b->GetClass() - 1];
						auto bot_spawn_limit_class = bot_class_spawn_limits[b->GetClass() - 1];

						if (bot_spawn_limit_class >= 0 && spawned_bot_count_class >= bot_spawn_limit_class) {
							database.SetGroupID(b->GetCleanName(), 0, b->GetBotID());
							g->UpdatePlayer(bot_owner);
							continue;
						}

						if (!b->Spawn(bot_owner)) {
							safe_delete(b);
							continue;
						}

						spawned_bots_count++;
						bot_class_spawned_count[b->GetClass() - 1]++;

						g->UpdatePlayer(b);

						if (g->IsGroupMember(bot_owner) && g->IsGroupMember(b)) {
							b->SetFollowID(bot_owner->GetID());
						}

						if (!bot_owner->HasGroup()) {
							database.SetGroupID(b->GetCleanName(), 0, b->GetBotID());
						}
					}
				}
			}
		} else {
			if (!database.botdb.LoadAutoSpawnBotGroupsByOwnerID(bot_owner->CharacterID(), auto_spawn_botgroups)) {
				bot_owner->Message(Chat::White, "Failed to load auto spawn bot groups by group ID.");
				return;
			}

			for (const auto& botgroup : auto_spawn_botgroups) {
				Bot::SpawnBotGroupByName(bot_owner, botgroup.second, botgroup.first);
			}
		}
	}
}

// Returns TRUE if there is atleast 1 bot in the specified group
bool Bot::GroupHasBot(Group* group) {
	bool Result = false;
	if(group) {
		for(int Counter = 0; Counter < MAX_GROUP_MEMBERS; Counter++) {
			if (group->members[Counter] == nullptr)
				continue;

			if(group->members[Counter]->IsBot()) {
				Result = true;
				break;
			}
		}
	}
	return Result;
}

uint32 Bot::SpawnedBotCount(const uint32 owner_id, uint8 class_id) {
	uint32 spawned_bot_count = 0;

	if (owner_id) {
		const auto& sbl = entity_list.GetBotListByCharacterID(owner_id, class_id);
		spawned_bot_count = sbl.size();
	}

	return spawned_bot_count;
}

void Bot::LevelBotWithClient(Client* client, uint8 level, bool sendlvlapp) {
	// This essentially performs a '#bot update,' with appearance packets, based on the current methods.
	// This should not be called outside of Client::SetEXP() due to it's lack of rule checks.
	if(client) {
		std::list<Bot*> blist = entity_list.GetBotsByBotOwnerCharacterID(client->CharacterID());

		for(std::list<Bot*>::iterator biter = blist.begin(); biter != blist.end(); ++biter) {
			Bot* bot = *biter;
			if(bot && (bot->GetLevel() != client->GetLevel())) {
				bot->SetPetChooser(false); // not sure what this does, but was in bot 'update' code
				bot->CalcBotStats(client->GetBotOption(Client::booStatsUpdate));
				if(sendlvlapp)
					bot->SendLevelAppearance();
				// modified from Client::SetLevel()
				bot->SendAppearancePacket(AT_WhoLevel, level, true, true); // who level change
			}
		}

		blist.clear();
	}
}

void Bot::SendBotArcheryWearChange(uint8 material_slot, uint32 material, uint32 color) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	WearChange_Struct* wc = (WearChange_Struct*)outapp->pBuffer;

	wc->spawn_id = GetID();
	wc->material = material;
	wc->color.Color = color;
	wc->wear_slot_id = material_slot;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

// Returns the item id that is in the bot inventory collection for the specified slot.
EQ::ItemInstance* Bot::GetBotItem(uint16 slot_id) {
	EQ::ItemInstance* item = m_inv.GetItem(slot_id);
	if (item) {
		return item;
	}

	return nullptr;
}

// Adds the specified item it bot to the NPC equipment array and to the bot inventory collection.
void Bot::BotAddEquipItem(uint16 slot_id, uint32 item_id) {
	// this is being called before bot is assigned an entity id..
	// ..causing packets to be sent out to zone with an id of '0'
	if (item_id) {
		uint8 material_from_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id);

		if (material_from_slot != EQ::textures::materialInvalid) {
			equipment[slot_id] = item_id; // npc has more than just material slots. Valid material should mean valid inventory index
			if (GetID()) { // temp hack fix
				SendWearChange(material_from_slot);
			}
		}

		UpdateEquipmentLight();
		if (UpdateActiveLight()) {
			if (GetID()) { // temp hack fix
				SendAppearancePacket(AT_Light, GetActiveLightType());
			}
		}
	}
}

// Erases the specified item from bot the NPC equipment array and from the bot inventory collection.
void Bot::BotRemoveEquipItem(uint16 slot_id)
{
	uint8 material_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id);

	if (material_slot != EQ::textures::materialInvalid) {
		equipment[slot_id] = 0; // npc has more than just material slots. Valid material should mean valid inventory index
		SendWearChange(material_slot);
		if (material_slot == EQ::textures::armorChest) {
			SendWearChange(EQ::textures::armorArms);
		}
	}

	UpdateEquipmentLight();
	if (UpdateActiveLight()) {
		SendAppearancePacket(AT_Light, GetActiveLightType());
	}
}

void Bot::BotTradeAddItem(const EQ::ItemInstance* inst, uint16 slot_id, std::string* error_message, bool save_to_database)
{
	if (save_to_database) {
		if (!database.botdb.SaveItemBySlot(this, slot_id, inst)) {
			*error_message = BotDatabase::fail::SaveItemBySlot();
			return;
		}

		m_inv.PutItem(slot_id, *inst);
	}

	auto item_id = inst ? inst->GetID() : 0;

	BotAddEquipItem(slot_id, item_id);
}

void Bot::AddBotItem(
	uint16 slot_id,
	uint32 item_id,
	int16 charges,
	bool attuned,
	uint32 augment_one,
	uint32 augment_two,
	uint32 augment_three,
	uint32 augment_four,
	uint32 augment_five,
	uint32 augment_six
) {
	auto inst = database.CreateItem(
		item_id,
		charges,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six,
		attuned
	);

	if (!inst) {
		LogError(
			"Bot:AddItem Invalid Item data: ID [{}] Charges [{}] Aug1 [{}] Aug2 [{}] Aug3 [{}] Aug4 [{}] Aug5 [{}] Aug6 [{}] Attuned [{}]",
			item_id,
			charges,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six,
			attuned
		);
		return;
	}

	if (!database.botdb.SaveItemBySlot(this, slot_id, inst)) {
		LogError("Failed to save item by slot to slot [{}] for [{}].", slot_id, GetCleanName());
		safe_delete(inst);
		return;
	}

	m_inv.PutItem(slot_id, *inst);
	safe_delete(inst);

	BotAddEquipItem(slot_id, item_id);
}

uint32 Bot::CountBotItem(uint32 item_id) {
	uint32 item_count = 0;
	EQ::ItemInstance *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}

		if (inst->GetID() == item_id) {
			item_count++;
		}
	}

	return item_count;
}

bool Bot::HasBotItem(uint32 item_id) {
	bool has_item = false;
	EQ::ItemInstance *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}

		if (inst->GetID() == item_id) {
			has_item = true;
			break;
		}
	}

	return has_item;
}

void Bot::RemoveBotItem(uint32 item_id) {
	EQ::ItemInstance *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}


		if (inst->GetID() == item_id) {
			std::string error_message;
			RemoveBotItemBySlot(slot_id, &error_message);
			if (!error_message.empty()) {
				if (GetOwner()) {
					GetOwner()->CastToClient()->Message(
						Chat::White,
						fmt::format(
							"Database Error: {}",
							error_message
						).c_str()
					);
				}
				return;
			}

			BotRemoveEquipItem(slot_id);
			CalcBotStats(GetOwner()->CastToClient()->GetBotOption(Client::booStatsUpdate));
			return;
		}
	}
}

bool Bot::RemoveBotFromGroup(Bot* bot, Group* group) {
	bool Result = false;
	if(bot && group) {
		if(bot->HasGroup()) {
			if(!group->IsLeader(bot)) {
				bot->SetFollowID(0);
				if(group->DelMember(bot))
					database.SetGroupID(bot->GetCleanName(), 0, bot->GetBotID());
			} else {
				for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if(!group->members[i])
						continue;

					group->members[i]->SetFollowID(0);
				}
				group->DisbandGroup();
				database.SetGroupID(bot->GetCleanName(), 0, bot->GetBotID());
			}
			Result = true;
		}
	}
	return Result;
}

bool Bot::AddBotToGroup(Bot* bot, Group* group) {
	bool Result = false;
	if(bot && group) {
		if(!bot->HasGroup()) {
			// Add bot to this group
			if(group->AddMember(bot)) {
				if(group->GetLeader()) {
					bot->SetFollowID(group->GetLeader()->GetID());
					// Need to send this only once when a group is formed with a bot so the client knows it is also the group leader
					if(group->GroupCount() == 2 && group->GetLeader()->IsClient()) {
						group->UpdateGroupAAs();
						Mob *TempLeader = group->GetLeader();
						group->SendUpdate(groupActUpdate, TempLeader);
					}
				}
				Result = true;
			}
		}
	}
	return Result;
}

// Completes a trade with a client bot owner
void Bot::FinishTrade(Client* client, BotTradeType trade_type)
{
	if (
		!client ||
		GetOwner() != client ||
		client->GetTradeskillObject() ||
		client->trade->state == Trading
	) {
		if (client) {
			client->ResetTrade();
		}

		return;
	}

	// these notes are not correct or obselete
	if (trade_type == BotTradeClientNormal) {
		// Items being traded are found in the normal trade window used to trade between a Client and a Client or NPC
		// Items in this mode are found in slot ids 3000 thru 3003 - thought bots used the full 8-slot window..?
		PerformTradeWithClient(EQ::invslot::TRADE_BEGIN, EQ::invslot::TRADE_END, client); // {3000..3007}
	}
	else if (trade_type == BotTradeClientNoDropNoTrade) {
		// Items being traded are found on the Client's cursor slot, slot id 30. This item can be either a single item or it can be a bag.
		// If it is a bag, then we have to search for items in slots 331 thru 340
		PerformTradeWithClient(EQ::invslot::slotCursor, EQ::invslot::slotCursor, client);

		// TODO: Add logic here to test if the item in SLOT_CURSOR is a container type, if it is then we need to call the following:
		// PerformTradeWithClient(331, 340, client);
	}
}

// Perfoms the actual trade action with a client bot owner
void Bot::PerformTradeWithClient(int16 begin_slot_id, int16 end_slot_id, Client* client)
{
	using namespace EQ;

	struct ClientTrade {
		ItemInstance* trade_item_instance;
		int16 from_client_slot;
		int16 to_bot_slot;

		ClientTrade(ItemInstance* item, int16 from) : trade_item_instance(item), from_client_slot(from), to_bot_slot(invslot::SLOT_INVALID) { }
	};

	struct ClientReturn {
		const ItemInstance* return_item_instance;
		int16 from_bot_slot;
		int16 to_client_slot;

		ClientReturn(const ItemInstance* item, int16 from) : return_item_instance(item), from_bot_slot(from), to_client_slot(invslot::SLOT_INVALID) { }
	};

	static const int16 bot_equip_order[invslot::EQUIPMENT_COUNT] = {
		invslot::slotCharm,			invslot::slotEar1,			invslot::slotHead,			invslot::slotFace,
		invslot::slotEar2,			invslot::slotNeck,			invslot::slotShoulders,		invslot::slotArms,
		invslot::slotBack,			invslot::slotWrist1,		invslot::slotWrist2,		invslot::slotRange,
		invslot::slotHands,			invslot::slotPrimary,		invslot::slotSecondary,		invslot::slotFinger1,
		invslot::slotFinger2,		invslot::slotChest,			invslot::slotLegs,			invslot::slotFeet,
		invslot::slotWaist,			invslot::slotPowerSource,	invslot::slotAmmo
	};

	enum { stageStackable = 0, stageEmpty, stageReplaceable };

	if (!client) {
		Emote("NO CLIENT");
		return;
	}

	if (client != GetOwner()) {
		client->Message(Chat::White, "You are not the owner of this bot, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (begin_slot_id != invslot::TRADE_BEGIN && begin_slot_id != invslot::slotCursor) {
		client->Message(Chat::White, "Trade request processing from illegal 'begin' slot, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (end_slot_id != invslot::TRADE_END && end_slot_id != invslot::slotCursor) {
		client->Message(Chat::White, "Trade request processing from illegal 'end' slot, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if ((begin_slot_id == invslot::slotCursor && end_slot_id != invslot::slotCursor) || (begin_slot_id != invslot::slotCursor && end_slot_id == invslot::slotCursor)) {
		client->Message(Chat::White, "Trade request processing illegal slot range, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (end_slot_id < begin_slot_id) {
		client->Message(Chat::White, "Trade request processing in reverse slot order, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (client->IsEngaged() || IsEngaged()) {
		client->Message(Chat::Yellow, "You may not perform a trade while engaged, the trade has been cancelled!");
		client->ResetTrade();
		return;
	}

	std::list<ClientTrade> client_trade;
	std::list<ClientTrade> event_trade;
	std::list<ClientReturn> client_return;

	bool trade_event_exists = false;
	if (parse->BotHasQuestSub(EVENT_TRADE)) {
		// There is a EVENT_TRADE, we will let the Event handle returning of items.
		trade_event_exists = true;
	}

	// pre-checks for incoming illegal transfers
	EQ::InventoryProfile& user_inv = client->GetInv();
	for (int16 trade_index = begin_slot_id; trade_index <= end_slot_id; ++trade_index) {
		auto trade_instance = user_inv.GetItem(trade_index);
		if (!trade_instance) {
			continue;
		}

		if (!trade_instance->GetItem()) {
			LogError("could not find item from instance in trade for [{}] with [{}] in slot [{}].", client->GetCleanName(), GetCleanName(), trade_index);
			client->Message(
				Chat::White,
				fmt::format(
					"A server error was encountered while processing client slot {}, the trade has been cancelled.",
					trade_index
				).c_str()
			);
			client->ResetTrade();
			return;
		}

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(trade_instance);

		auto item_link = linker.GenerateLink();

		if (trade_index != invslot::slotCursor && !trade_instance->IsDroppable()) {
			LogError("trade hack detected by [{}] with [{}].", client->GetCleanName(), GetCleanName());
			client->Message(Chat::White, "Trade hack detected, the trade has been cancelled.");
			client->ResetTrade();
			return;
		}

		if (trade_instance->IsStackable() && trade_instance->GetCharges() < trade_instance->GetItem()->StackSize) { // temp until partial stacks are implemented
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_instance, trade_index));
				continue;
			}
			else {
				client->Message(
					Chat::Yellow,
					fmt::format(
						"{} is only a partially stacked item, the trade has been cancelled!",
						item_link
					).c_str()
				);
				client->ResetTrade();
				return;
			}
		}

		if (CheckLoreConflict(trade_instance->GetItem())) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_instance, trade_index));
				continue;
			}
			else {
				client->Message(
					Chat::Yellow,
					fmt::format(
						"This bot already has {}, the trade has been cancelled!",
						item_link
					).c_str()
				);
				client->ResetTrade();
				return;
			}
		}

		if (!trade_instance->IsType(item::ItemClassCommon)) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_instance, trade_index));
				continue;
			}
			else {
				client->ResetTrade();
				return;
			}
		}

		if (
			!trade_instance->IsClassEquipable(GetClass()) ||
			GetLevel() < trade_instance->GetItem()->ReqLevel ||
			(!trade_instance->IsRaceEquipable(GetBaseRace()) && !RuleB(Bots, AllowBotEquipAnyRaceGear))
		) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_instance, trade_index));
				continue;
			}
			else {
				client->ResetTrade();
				return;
			}
		}

		client_trade.push_back(ClientTrade(trade_instance, trade_index));
	}

	// check for incoming lore hacks
	for (auto& trade_iterator : client_trade) {
		auto trade_instance = trade_iterator.trade_item_instance;
		auto trade_index = trade_iterator.from_client_slot;
		if (!trade_instance->GetItem()->LoreFlag) {
			continue;
		}

		for (const auto& check_iterator : client_trade) {
			if (check_iterator.from_client_slot == trade_iterator.from_client_slot) {
				continue;
			}

			if (!check_iterator.trade_item_instance->GetItem()->LoreFlag) {
				continue;
			}

			if (trade_instance->GetItem()->LoreGroup == -1 && check_iterator.trade_item_instance->GetItem()->ID == trade_instance->GetItem()->ID) {
				LogError("trade hack detected by [{}] with [{}].", client->GetCleanName(), GetCleanName());
				client->Message(Chat::White, "Trade hack detected, the trade has been cancelled.");
				client->ResetTrade();
				return;
			}

			if ((trade_instance->GetItem()->LoreGroup > 0) && (check_iterator.trade_item_instance->GetItem()->LoreGroup == trade_instance->GetItem()->LoreGroup)) {
				LogError("trade hack detected by [{}] with [{}].", client->GetCleanName(), GetCleanName());
				client->Message(Chat::White, "Trade hack detected, the trade has been cancelled.");
				client->ResetTrade();
				return;
			}
		}
	}

	// find equipment slots
	const bool can_dual_wield = (GetSkill(EQ::skills::SkillDualWield) > 0);
	bool melee_2h_weapon = false;
	bool melee_secondary = false;

	//for (unsigned stage_loop = stageStackable; stage_loop <= stageReplaceable; ++stage_loop) { // awaiting implementation
	for (unsigned stage_loop = stageEmpty; stage_loop <= stageReplaceable; ++stage_loop) {
		for (auto& trade_iterator : client_trade) {
			if (trade_iterator.to_bot_slot != invslot::SLOT_INVALID) {
				continue;
			}

			auto trade_instance = trade_iterator.trade_item_instance;
			//if ((stage_loop == stageStackable) && !trade_instance->IsStackable())
			//	continue;

			for (auto index : bot_equip_order) {
				if (!(trade_instance->GetItem()->Slots & (1 << index))) {
					continue;
				}

				//if (stage_loop == stageStackable) {
				//	// TODO: implement
				//	continue;
				//}

				if (stage_loop != stageReplaceable) {
					if (m_inv[index]) {
						continue;
					}
				}

				bool slot_taken = false;
				for (const auto& check_iterator : client_trade) {
					if (check_iterator.from_client_slot == trade_iterator.from_client_slot) {
						continue;
					}

					if (check_iterator.to_bot_slot == index) {
						slot_taken = true;
						break;
					}
				}

				if (slot_taken) {
					continue;
				}

				if (index == invslot::slotPrimary) {
					if (trade_instance->GetItem()->IsType2HWeapon()) {
						if (!melee_secondary) {
							melee_2h_weapon = true;
							auto equipped_secondary_weapon = m_inv[invslot::slotSecondary];
							if (equipped_secondary_weapon) {
								client_return.push_back(ClientReturn(equipped_secondary_weapon, invslot::slotSecondary));
							}
						} else {
							continue;
						}
					}
				} else if (index == invslot::slotSecondary) {
					if (!melee_2h_weapon) {
						if (
							(can_dual_wield && trade_instance->GetItem()->IsType1HWeapon()) ||
							trade_instance->GetItem()->IsTypeShield() ||
							!trade_instance->IsWeapon()
						) {
							melee_secondary = true;
							auto equipped_primary_weapon = m_inv[invslot::slotPrimary];
							if (equipped_primary_weapon && equipped_primary_weapon->GetItem()->IsType2HWeapon()) {
								client_return.push_back(ClientReturn(equipped_primary_weapon, invslot::slotPrimary));
							}
						} else {
							continue;
						}
					} else {
						continue;
					}
				}

				trade_iterator.to_bot_slot = index;

				if (m_inv[index]) {
					client_return.push_back(ClientReturn(m_inv[index], index));
				}

				break;
			}
		}
	}

	// move unassignable items from trade list to event list
	for (std::list<ClientTrade>::iterator trade_iterator = client_trade.begin(); trade_iterator != client_trade.end();) {
		if (trade_iterator->to_bot_slot == invslot::SLOT_INVALID) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_iterator->trade_item_instance, trade_iterator->from_client_slot));
				trade_iterator = client_trade.erase(trade_iterator);
				continue;
			}
			else {
				client_return.push_back(ClientReturn(trade_iterator->trade_item_instance, trade_iterator->from_client_slot));
				trade_iterator = client_trade.erase(trade_iterator);
				continue;
			}
		}
		++trade_iterator;
	}

	// out-going return checks for client
	for (auto& return_iterator : client_return) {
		auto return_instance = return_iterator.return_item_instance;
		if (!return_instance) {
			continue;
		}

		if (!return_instance->GetItem()) {
			LogError("error processing bot slot [{}] for [{}] in trade with [{}].", return_iterator.from_bot_slot, GetCleanName(), client->GetCleanName());
			client->Message(
				Chat::White,
				fmt::format(
					"A server error was encountered while processing bot slot {}, the trade has been cancelled.",
					return_iterator.from_bot_slot
				).c_str()
			);
			client->ResetTrade();
			return;
		}

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(return_instance);

		auto item_link = linker.GenerateLink();

		// non-failing checks above are causing this to trigger (i.e., !ItemClassCommon and !IsEquipable{race, class, min_level})
		// this process is hindered by not having bots use the inventory trade method (TODO: implement bot inventory use)
		if (client->CheckLoreConflict(return_instance->GetItem())) {
			client->Message(
				Chat::Yellow,
				fmt::format(
					"You already have {}, the trade has been cancelled!",
					item_link
				).c_str()
			);
			client->ResetTrade();
			return;
		}

		if (return_iterator.from_bot_slot == invslot::slotCursor) {
			return_iterator.to_client_slot = invslot::slotCursor;
		} else {
			int16 client_search_general = invslot::GENERAL_BEGIN;
			uint8 client_search_bag = invbag::SLOT_BEGIN;
			bool run_search = true;
			while (run_search) {
				int16 client_test_slot = client->GetInv().FindFreeSlotForTradeItem(return_instance, client_search_general, client_search_bag);
				if (client_test_slot == invslot::SLOT_INVALID) {
					run_search = false;
					continue;
				}

				bool slot_taken = false;
				for (const auto& check_iterator : client_return) {
					if (check_iterator.from_bot_slot == return_iterator.from_bot_slot) {
						continue;
					}

					if (check_iterator.to_client_slot == client_test_slot && client_test_slot != invslot::slotCursor) {
						slot_taken = true;
						break;
					}
				}

				if (slot_taken) {
					if (client_test_slot >= invslot::GENERAL_BEGIN && client_test_slot <= invslot::GENERAL_END) {
						++client_search_general;
						client_search_bag = invbag::SLOT_BEGIN;
					} else {
						client_search_general = InventoryProfile::CalcSlotId(client_test_slot);
						client_search_bag = InventoryProfile::CalcBagIdx(client_test_slot);
						++client_search_bag;
						if (client_search_bag >= invbag::SLOT_COUNT) {
							// incrementing this past legacy::GENERAL_END triggers the (client_test_slot == legacy::SLOT_INVALID) at the beginning of the search loop
							// ideally, this will never occur because we always start fresh with each loop iteration and should receive SLOT_CURSOR as a return value
							++client_search_general;
							client_search_bag = invbag::SLOT_BEGIN;
						}
					}

					continue;
				}

				return_iterator.to_client_slot = client_test_slot;
				run_search = false;
			}
		}

		if (return_iterator.to_client_slot == invslot::SLOT_INVALID) {
			client->Message(Chat::Yellow, "You do not have room to complete this trade, the trade has been cancelled!");
			client->ResetTrade();
			return;
		}
	}

	// perform actual trades
	// returns first since clients have trade slots and bots do not
	for (auto& return_iterator : client_return) {
		// TODO: code for stackables

		if (return_iterator.from_bot_slot == invslot::slotCursor) { // failed trade return
			// no movement action required
		} else if (return_iterator.from_bot_slot >= invslot::TRADE_BEGIN && return_iterator.from_bot_slot <= invslot::TRADE_END) { // failed trade returns
			client->PutItemInInventory(return_iterator.to_client_slot, *return_iterator.return_item_instance);
			client->SendItemPacket(return_iterator.to_client_slot, return_iterator.return_item_instance, ItemPacketTrade);
			client->DeleteItemInInventory(return_iterator.from_bot_slot);
		} else { // successful trade returns
			auto return_instance = m_inv.PopItem(return_iterator.from_bot_slot);
			//if (*return_instance != *return_iterator.return_item_instance) {
			//	// TODO: add logging
			//}

			if (!database.botdb.DeleteItemBySlot(GetBotID(), return_iterator.from_bot_slot)) {
				OwnerMessage(
					fmt::format(
						"Failed to delete item by slot from slot {}.",
						return_iterator.from_bot_slot
					).c_str()
				);
			}

			BotRemoveEquipItem(return_iterator.from_bot_slot);

			if (return_instance) {
				EQ::SayLinkEngine linker;
				linker.SetLinkType(EQ::saylink::SayLinkItemInst);
				linker.SetItemInst(return_instance);
				auto item_link = linker.GenerateLink();

				OwnerMessage(
					fmt::format(
						"I have returned {}.",
						item_link
					)
				);

				client->PutItemInInventory(return_iterator.to_client_slot, *return_instance, true);
			}

			InventoryProfile::MarkDirty(return_instance);
		}
		return_iterator.return_item_instance = nullptr;
	}

	// trades can now go in as empty slot inserts
	for (auto& trade_iterator : client_trade) {
		// TODO: code for stackables

		if (!database.botdb.SaveItemBySlot(this, trade_iterator.to_bot_slot, trade_iterator.trade_item_instance)) {
			OwnerMessage(
				fmt::format(
					"Failed to save item by slot to slot {}.",
					trade_iterator.to_bot_slot
				).c_str()
			);
		}

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(trade_iterator.trade_item_instance);
		auto item_link = linker.GenerateLink();

		OwnerMessage(
			fmt::format(
				"I have accepted {}.",
				item_link
			)
		);

		m_inv.PutItem(trade_iterator.to_bot_slot, *trade_iterator.trade_item_instance);
		BotAddEquipItem(trade_iterator.to_bot_slot, (trade_iterator.trade_item_instance ? trade_iterator.trade_item_instance->GetID() : 0));
		trade_iterator.trade_item_instance = nullptr; // actual deletion occurs in client delete below

		client->DeleteItemInInventory(trade_iterator.from_client_slot, 0, (trade_iterator.from_client_slot == EQ::invslot::slotCursor));

		// database currently has unattuned item saved in inventory..it will be attuned on next bot load
		// this prevents unattuned item returns in the mean time (TODO: re-work process)
		if (trade_iterator.to_bot_slot >= invslot::EQUIPMENT_BEGIN && trade_iterator.to_bot_slot <= invslot::EQUIPMENT_END) {
			auto attune_item = m_inv.GetItem(trade_iterator.to_bot_slot);
			if (attune_item && attune_item->GetItem()->Attuneable) {
				attune_item->SetAttuned(true);
			}
		}
	}

	size_t accepted_count = client_trade.size();
	size_t returned_count = client_return.size();

	if (accepted_count) {
		CalcBotStats(client->GetBotOption(Client::booStatsUpdate));
	}

	if (event_trade.size()) {
		// Get Traded Items

		// Accept Items from Cursor to support bot command ^inventorygive
		if (begin_slot_id == invslot::slotCursor && end_slot_id == invslot::slotCursor) {
			EQ::ItemInstance* insts[1] = { 0 };
			EQ::InventoryProfile& user_inv = client->GetInv();
			insts[0] = user_inv.GetItem(invslot::slotCursor);
			client->DeleteItemInInventory(invslot::slotCursor);

			// copy to be filtered by task updates, null trade slots preserved for quest event arg
			std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

			// Check if EVENT_TRADE accepts any items
			std::vector<std::any> item_list(items.begin(), items.end());
			parse->EventBot(EVENT_TRADE, this, client, "", 0, &item_list);
			CalcBotStats(false);

		} else {
			EQ::ItemInstance* insts[8] = { 0 };
			EQ::InventoryProfile& user_inv = client->GetInv();
			for (int i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_END; ++i) {
				insts[i - EQ::invslot::TRADE_BEGIN] = user_inv.GetItem(i);
				client->DeleteItemInInventory(i);
			}

			// copy to be filtered by task updates, null trade slots preserved for quest event arg
			std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

			// Check if EVENT_TRADE accepts any items
			std::vector<std::any> item_list(items.begin(), items.end());
			parse->EventBot(EVENT_TRADE, this, client, "", 0, &item_list);
			CalcBotStats(false);
		}
	}
}

bool Bot::Death(Mob *killerMob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill) {
	if(!NPC::Death(killerMob, damage, spell_id, attack_skill))
		return false;

	Save();

	Mob *my_owner = GetBotOwner();
	if (my_owner && my_owner->IsClient() && my_owner->CastToClient()->GetBotOption(Client::booDeathMarquee)) {
		if (killerMob)
			my_owner->CastToClient()->SendMarqueeMessage(Chat::White, 510, 0, 1000, 3000, StringFormat("%s has been slain by %s", GetCleanName(), killerMob->GetCleanName()));
		else
			my_owner->CastToClient()->SendMarqueeMessage(Chat::White, 510, 0, 1000, 3000, StringFormat("%s has been slain", GetCleanName()));
	}

	Mob *give_exp = hate_list.GetDamageTopOnHateList(this);
	Client *give_exp_client = nullptr;
	if(give_exp && give_exp->IsClient())
		give_exp_client = give_exp->CastToClient();

	bool IsLdonTreasure = (GetClass() == LDON_TREASURE);
	if(entity_list.GetCorpseByID(GetID()))
		entity_list.GetCorpseByID(GetID())->Depop();

	Group *g = GetGroup();
	if(g) {
		for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if(g->members[i]) {
				if(g->members[i] == this) {
					// If the leader dies, make the next bot the leader
					// and reset all bots followid
					if(g->IsLeader(g->members[i])) {
						if(g->members[i + 1]) {
							g->SetLeader(g->members[i + 1]);
							g->members[i + 1]->SetFollowID(g->members[i]->GetFollowID());
							for(int j = 0; j < MAX_GROUP_MEMBERS; j++) {
								if(g->members[j] && (g->members[j] != g->members[i + 1]))
									g->members[j]->SetFollowID(g->members[i + 1]->GetID());
							}
						}
					}

					// delete from group data
					RemoveBotFromGroup(this, g);
					//Make sure group still exists if it doesnt they were already updated in RemoveBotFromGroup
					g = GetGroup();
					if (!g)
						break;

					// if group members exist below this one, move
					// them all up one slot in the group list
					int j = (i + 1);
					for(; j < MAX_GROUP_MEMBERS; j++) {
						if(g->members[j]) {
							g->members[j-1] = g->members[j];
							strcpy(g->membername[j-1], g->members[j]->GetCleanName());
							g->membername[j][0] = '\0';
							memset(g->membername[j], 0, 64);
							g->members[j] = nullptr;
						}
					}

					// update the client group
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;
					gu->action = groupActLeave;
					strcpy(gu->membername, GetCleanName());
					if(g) {
						for(int k = 0; k < MAX_GROUP_MEMBERS; k++) {
							if(g->members[k] && g->members[k]->IsClient())
								g->members[k]->CastToClient()->QueuePacket(outapp);
						}
					}
					safe_delete(outapp);
				}
			}
		}
	}

	LeaveHealRotationMemberPool();

	if ((GetPullingFlag() || GetReturningFlag()) && my_owner && my_owner->IsClient()) {
		my_owner->CastToClient()->SetBotPulling(false);
	}

	const auto export_string = fmt::format(
		"{} {} {} {}",
		killerMob ? killerMob->GetID() : 0,
		damage,
		spell_id,
		static_cast<int>(attack_skill)
	);

	parse->EventBot(EVENT_DEATH_COMPLETE, this, killerMob, export_string, 0);

	entity_list.RemoveBot(GetID());
	return true;
}

void Bot::Damage(Mob *from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special) {
	if(spell_id == 0)
		spell_id = SPELL_UNKNOWN;

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if(attacked_timer.Check()) {
		LogCombat("Triggering EVENT_ATTACK due to attack by [{}]", from->GetName());
		parse->EventBot(EVENT_ATTACK, this, from, "", 0);
	}

	attacked_timer.Start(CombatEventTimer_expire);
	// if spell is lifetap add hp to the caster
	if (spell_id != SPELL_UNKNOWN && IsLifetapSpell(spell_id)) {
		int64 healed = GetActSpellHealing(spell_id, damage);
		LogCombatDetail("Applying lifetap heal of [{}] to [{}]", healed, GetCleanName());
		HealDamage(healed);
		entity_list.FilteredMessageClose(this, true, RuleI(Range, SpellMessages), Chat::Emote, FilterSocials, "%s beams a smile at %s", GetCleanName(), from->GetCleanName() );
	}

	CommonDamage(from, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);
	if(GetHP() < 0) {
		if(IsCasting())
			InterruptSpell();
		SetAppearance(eaDead);
	}

	SendHPUpdate();
	if(this == from)
		return;

	// Aggro the bot's group members
	if(IsGrouped()) {
		Group *g = GetGroup();
		if(g) {
			for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if(g->members[i] && g->members[i]->IsBot() && from && !g->members[i]->CheckAggro(from) && g->members[i]->IsAttackAllowed(from))
					g->members[i]->AddToHateList(from, 1);
			}
		}
	}
}

//proc chance includes proc bonus
float Bot::GetProcChances(float ProcBonus, uint16 hand) {
	int mydex = GetDEX();
	float ProcChance = 0.0f;
	uint32 weapon_speed = 0;
	switch (hand) {
	case EQ::invslot::slotPrimary:
		weapon_speed = attack_timer.GetDuration();
		break;
	case EQ::invslot::slotSecondary:
		weapon_speed = attack_dw_timer.GetDuration();
		break;
	case EQ::invslot::slotRange:
		weapon_speed = ranged_timer.GetDuration();
		break;
	}

	if (weapon_speed < RuleI(Combat, MinHastedDelay))
		weapon_speed = RuleI(Combat, MinHastedDelay);

	if (RuleB(Combat, AdjustProcPerMinute)) {
		ProcChance = (static_cast<float>(weapon_speed) * RuleR(Combat, AvgProcsPerMinute) / 60000.0f);
		ProcBonus += static_cast<float>(mydex) * RuleR(Combat, ProcPerMinDexContrib);
		ProcChance += (ProcChance * ProcBonus / 100.0f);
	} else {
		ProcChance = (RuleR(Combat, BaseProcChance) + static_cast<float>(mydex) / RuleR(Combat, ProcDexDivideBy));
		ProcChance += (ProcChance * ProcBonus / 100.0f);
	}

	LogCombat("Proc chance [{}] ([{}] from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

int Bot::GetHandToHandDamage(void) {
	if (RuleB(Combat, UseRevampHandToHand)) {
		// everyone uses this in the revamp!
		int skill = GetSkill(EQ::skills::SkillHandtoHand);
		int epic = 0;
		if (CastToNPC()->GetEquippedItemFromTextureSlot(EQ::textures::armorHands) == 10652 && GetLevel() > 46)
			epic = 280;
		if (epic > skill)
			skill = epic;
		return skill / 15 + 3;
	}

	static uint8 mnk_dmg[] = {99,
				4, 4, 4, 4, 5, 5, 5, 5, 5, 6,           // 1-10
				6, 6, 6, 6, 7, 7, 7, 7, 7, 8,           // 11-20
				8, 8, 8, 8, 9, 9, 9, 9, 9, 10,          // 21-30
				10, 10, 10, 10, 11, 11, 11, 11, 11, 12, // 31-40
				12, 12, 12, 12, 13, 13, 13, 13, 13, 14, // 41-50
				14, 14, 14, 14, 14, 14, 14, 14, 14, 14, // 51-60
				14, 14};                                // 61-62
	static uint8 bst_dmg[] = {99,
				4, 4, 4, 4, 4, 5, 5, 5, 5, 5,        // 1-10
				5, 6, 6, 6, 6, 6, 6, 7, 7, 7,        // 11-20
				7, 7, 7, 8, 8, 8, 8, 8, 8, 9,        // 21-30
				9, 9, 9, 9, 9, 10, 10, 10, 10, 10,   // 31-40
				10, 11, 11, 11, 11, 11, 11, 12, 12}; // 41-49
	if (GetClass() == MONK) {
		if (CastToNPC()->GetEquippedItemFromTextureSlot(EQ::textures::armorHands) == 10652 && GetLevel() > 50)
			return 9;
		if (level > 62)
			return 15;
		return mnk_dmg[level];
	} else if (GetClass() == BEASTLORD) {
		if (level > 49)
			return 13;
		return bst_dmg[level];
	}
	return 2;
}

bool Bot::TryFinishingBlow(Mob *defender, int64 &damage)
{
	if (!defender)
		return false;

	if (aabonuses.FinishingBlow[1] && !defender->IsClient() && defender->GetHPRatio() < 10) {
		int chance = aabonuses.FinishingBlow[0];
		int fb_damage = aabonuses.FinishingBlow[1];
		int levelreq = aabonuses.FinishingBlowLvl[0];
		if (defender->GetLevel() <= levelreq && (chance >= zone->random.Int(1, 1000))) {
			LogCombat("Landed a finishing blow: levelreq at [{}] other level [{}]",
				levelreq, defender->GetLevel());
			entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, FINISHING_BLOW, GetName());
			damage = fb_damage;
			return true;
		} else {
			LogCombat("failed a finishing blow: levelreq at [{}] other level [{}]",
				levelreq, defender->GetLevel());
			return false;
		}
	}
	return false;
}

void Bot::DoRiposte(Mob* defender) {
	LogCombatDetail("Preforming a riposte");
	if (!defender)
		return;

	defender->Attack(this, EQ::invslot::slotPrimary, true);
	int32 DoubleRipChance = (defender->GetAABonuses().GiveDoubleRiposte[0] + defender->GetSpellBonuses().GiveDoubleRiposte[0] + defender->GetItemBonuses().GiveDoubleRiposte[0]);
	if(DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		LogCombatDetail("Preforming a double riposte ([{}] percent chance)", DoubleRipChance);
		defender->Attack(this, EQ::invslot::slotPrimary, true);
	}

	DoubleRipChance = defender->GetAABonuses().GiveDoubleRiposte[1];
	if(DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		if (defender->GetClass() == MONK)
			defender->MonkSpecialAttack(this, defender->GetAABonuses().GiveDoubleRiposte[2]);
		else if (defender->IsBot())
			defender->CastToClient()->DoClassAttacks(this,defender->GetAABonuses().GiveDoubleRiposte[2], true);
	}
}

int Bot::GetBaseSkillDamage(EQ::skills::SkillType skill, Mob *target)
{
	int base = EQ::skills::GetBaseDamage(skill);
	auto skill_level = GetSkill(skill);
	switch (skill) {
	case EQ::skills::SkillDragonPunch:
	case EQ::skills::SkillEagleStrike:
	case EQ::skills::SkillTigerClaw:
		if (skill_level >= 25)
			base++;
		if (skill_level >= 75)
			base++;
		if (skill_level >= 125)
			base++;
		if (skill_level >= 175)
			base++;
		return base;
	case EQ::skills::SkillFrenzy:
		if (GetBotItem(EQ::invslot::slotPrimary)) {
			if (GetLevel() > 15)
				base += GetLevel() - 15;
			if (base > 23)
				base = 23;
			if (GetLevel() > 50)
				base += 2;
			if (GetLevel() > 54)
				base++;
			if (GetLevel() > 59)
				base++;
		}
		return base;
	case EQ::skills::SkillFlyingKick: {
		float skill_bonus = skill_level / 9.0f;
		float ac_bonus = 0.0f;
		auto inst = GetBotItem(EQ::invslot::slotFeet);
		if (inst)
			ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
		if (ac_bonus > skill_bonus)
			ac_bonus = skill_bonus;
		return static_cast<int>(ac_bonus + skill_bonus);
	}
	case EQ::skills::SkillKick: {
		float skill_bonus = skill_level / 10.0f;
		float ac_bonus = 0.0f;
		auto inst = GetBotItem(EQ::invslot::slotFeet);
		if (inst)
			ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
		if (ac_bonus > skill_bonus)
			ac_bonus = skill_bonus;
		return static_cast<int>(ac_bonus + skill_bonus);
	}
	case EQ::skills::SkillBash: {
		float skill_bonus = skill_level / 10.0f;
		float ac_bonus = 0.0f;
		const EQ::ItemInstance *inst = nullptr;
		if (HasShieldEquiped())
			inst = GetBotItem(EQ::invslot::slotSecondary);
		else if (HasTwoHanderEquipped())
			inst = GetBotItem(EQ::invslot::slotPrimary);
		if (inst)
			ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
		if (ac_bonus > skill_bonus)
			ac_bonus = skill_bonus;
		return static_cast<int>(ac_bonus + skill_bonus);
	}
	case EQ::skills::SkillBackstab: {
		float skill_bonus = static_cast<float>(skill_level) * 0.02f;
		auto inst = GetBotItem(EQ::invslot::slotPrimary);
		if (inst && inst->GetItem() && inst->GetItem()->ItemType == EQ::item::ItemType1HPiercing) {
			base = inst->GetItemBackstabDamage(true);
			if (!inst->GetItemBackstabDamage())
				base += inst->GetItemWeaponDamage(true);
			if (target) {
				if (inst->GetItemElementalFlag(true) && inst->GetItemElementalDamage(true))
					base += target->ResistElementalWeaponDmg(inst);
				if (inst->GetItemBaneDamageBody(true) || inst->GetItemBaneDamageRace(true))
					base += target->CheckBaneDamage(inst);
			}
		}
		return static_cast<int>(static_cast<float>(base) * (skill_bonus + 2.0f));
	}
	default:
		return 0;
	}
}

void Bot::DoSpecialAttackDamage(Mob *who, EQ::skills::SkillType skill, int32 max_damage, int32 min_damage, int32 hate_override, int ReuseTime, bool HitChance) {
	int32 hate = max_damage;
	if(hate_override > -1)
		hate = hate_override;

	if (skill == EQ::skills::SkillBash) {
		const EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotSecondary);
		const EQ::ItemData* botweapon = nullptr;
		if(inst)
			botweapon = inst->GetItem();

		if(botweapon) {
			if (botweapon->ItemType == EQ::item::ItemTypeShield)
				hate += botweapon->AC;

			hate = (hate * (100 + GetFuriousBash(botweapon->Focus.Effect)) / 100);
		}
	}

	DamageHitInfo my_hit;
	my_hit.base_damage = max_damage;
	my_hit.min_damage = min_damage;
	my_hit.damage_done = 1;

	my_hit.skill = skill;
	my_hit.offense = offense(my_hit.skill);
	my_hit.tohit = GetTotalToHit(my_hit.skill, 0);
	my_hit.hand = EQ::invslot::slotPrimary;

	if (skill == EQ::skills::SkillThrowing || skill == EQ::skills::SkillArchery)
		my_hit.hand = EQ::invslot::slotRange;

	DoAttack(who, my_hit);

	who->AddToHateList(this, hate);

	who->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, skill, false);

	if(!GetTarget() || HasDied())
		return;

	if (my_hit.damage_done > 0)
		CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);

	//[AA Dragon Punch] value[0] = 100 for 25%, chance value[1] = skill
	//if(aabonuses.SpecialAttackKBProc[0] && aabonuses.SpecialAttackKBProc[1] == skill){
	//	int kb_chance = 25;
	//	kb_chance += (kb_chance * (100 - aabonuses.SpecialAttackKBProc[0]) / 100);

	//	if (zone->random.Int(0, 99) < kb_chance)
	//		SpellFinished(904, who, 10, 0, -1, spells[904].ResistDiff);
	//		//who->Stun(100); Kayen: This effect does not stun on live, it only moves the NPC.
	//}

	if (HasSkillProcs())
		TrySkillProc(who, skill, (ReuseTime * 1000));

	if (my_hit.damage_done > 0 && HasSkillProcSuccess())
		TrySkillProc(who, skill, (ReuseTime * 1000), true);
}

void Bot::TryBackstab(Mob *other, int ReuseTime) {
	if(!other)
		return;

	bool bIsBehind = false;
	bool bCanFrontalBS = false;
	const EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotPrimary);
	const EQ::ItemData* botpiercer = nullptr;
	if(inst)
		botpiercer = inst->GetItem();

	if (!botpiercer || (botpiercer->ItemType != EQ::item::ItemType1HPiercing)) {
		BotGroupSay(this, "I can't backstab with this weapon!");
		return;
	}

	int tripleChance = (itembonuses.TripleBackstab + spellbonuses.TripleBackstab + aabonuses.TripleBackstab);
	if (BehindMob(other, GetX(), GetY()))
		bIsBehind = true;
	else {
		int FrontalBSChance = (itembonuses.FrontalBackstabChance + spellbonuses.FrontalBackstabChance + aabonuses.FrontalBackstabChance);
		if (FrontalBSChance && (FrontalBSChance > zone->random.Int(0, 100)))
			bCanFrontalBS = true;
	}

	if (bIsBehind || bCanFrontalBS) {
		int chance = (10 + (GetDEX() / 10) + (itembonuses.HeroicDEX / 10));
		if(level >= 60 && other->GetLevel() <= 45 && !other->CastToNPC()->IsEngaged() && other->GetHP()<= 32000 && other->IsNPC() && zone->random.Real(0, 99) < chance) {
			entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, ASSASSINATES, GetName());
			RogueAssassinate(other);
		} else {
			RogueBackstab(other);
			if (level > 54) {
				float DoubleAttackProbability = ((GetSkill(EQ::skills::SkillDoubleAttack) + GetLevel()) / 500.0f);
				if(zone->random.Real(0, 1) < DoubleAttackProbability) {
					if(other->GetHP() > 0)
						RogueBackstab(other,false,ReuseTime);

					if (tripleChance && other->GetHP() > 0 && tripleChance > zone->random.Int(0, 100))
						RogueBackstab(other,false,ReuseTime);
				}
			}
		}
	} else if(aabonuses.FrontalBackstabMinDmg || itembonuses.FrontalBackstabMinDmg || spellbonuses.FrontalBackstabMinDmg) {
		m_specialattacks = eSpecialAttacks::ChaoticStab;
		RogueBackstab(other, true);
		m_specialattacks = eSpecialAttacks::None;
	}
	else
		Attack(other, EQ::invslot::slotPrimary);
}

void Bot::RogueBackstab(Mob *other, bool min_damage, int ReuseTime)
{
	if (!other)
		return;

	EQ::ItemInstance *botweaponInst = GetBotItem(EQ::invslot::slotPrimary);
	if (botweaponInst) {
		if (!GetWeaponDamage(other, botweaponInst))
			return;
	} else if (!GetWeaponDamage(other, (const EQ::ItemData *)nullptr)) {
		return;
	}

	int64 hate = 0;

	int base_damage = GetBaseSkillDamage(EQ::skills::SkillBackstab, other);
	hate = base_damage;

	DoSpecialAttackDamage(other, EQ::skills::SkillBackstab, base_damage, 0, hate, ReuseTime);
	DoAnim(anim1HPiercing);
}

void Bot::RogueAssassinate(Mob* other) {
	EQ::ItemInstance* botweaponInst = GetBotItem(EQ::invslot::slotPrimary);
	if(botweaponInst) {
		if(GetWeaponDamage(other, botweaponInst))
			other->Damage(this, 32000, SPELL_UNKNOWN, EQ::skills::SkillBackstab);
		else
			other->Damage(this, -5, SPELL_UNKNOWN, EQ::skills::SkillBackstab);
	}

	DoAnim(anim1HPiercing);
}

void Bot::DoClassAttacks(Mob *target, bool IsRiposte) {
	if(!target || spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || GetHP() < 0 || !IsAttackAllowed(target))
		return;

	bool taunt_time = taunt_timer.Check();
	bool ca_time = classattack_timer.Check(false);
	bool ma_time = monkattack_timer.Check(false);
	bool ka_time = knightattack_timer.Check(false);

	if (taunt_time) {

		// Bots without this skill shouldn't be 'checking' on this timer..let's just disable it and avoid the extra IsAttackAllowed() checks
		// Note: this is done here instead of NPC::ctor() because taunt skill can be acquired during level ups (the timer is re-enabled in CalcBotStats())
		if (!GetSkill(EQ::skills::SkillTaunt)) {

			taunt_timer.Disable();
			return;
		}

		if (!IsAttackAllowed(target)) {
			return;
		}
	}

	if ((ca_time || ma_time || ka_time) && !IsAttackAllowed(target)) {
		return;
	}

	if (ka_time) {

		switch (GetClass()) {
			case SHADOWKNIGHT: {
				CastSpell(SPELL_NPC_HARM_TOUCH, target->GetID());
				knightattack_timer.Start(HarmTouchReuseTime * 1000);

				break;
			}
			case PALADIN: {
				if(GetHPRatio() < 20) {
					CastSpell(SPELL_LAY_ON_HANDS, GetID());
					knightattack_timer.Start(LayOnHandsReuseTime * 1000);
				}
				else {
					knightattack_timer.Start(2000);
				}

				break;
			}
			default: {
				break;
			}
		}
	}

	if(taunting && target && target->IsNPC() && taunt_time) {
		if(GetTarget() && GetTarget()->GetHateTop() && GetTarget()->GetHateTop() != this) {
			BotGroupSay(
				this,
				fmt::format(
					"Taunting {}.",
					target->GetCleanName()
				).c_str()
			);
			Taunt(target->CastToNPC(), false);
			taunt_timer.Start(TauntReuseTime * 1000);
		}
	}

	if (ma_time) {
		switch (GetClass()) {
		case MONK: {
			int reuse = (MonkSpecialAttack(target, EQ::skills::SkillTigerClaw) - 1);

			// Live AA - Technique of Master Wu
			int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;

			if (wuchance) {
				const int MonkSPA[5] = {
					EQ::skills::SkillFlyingKick,
					EQ::skills::SkillDragonPunch,
					EQ::skills::SkillEagleStrike,
					EQ::skills::SkillTigerClaw,
					EQ::skills::SkillRoundKick
				};
				int extra = 0;
				// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
				while (wuchance > 0) {
					if (zone->random.Roll(wuchance)) {
						++extra;
					}
					else {
						break;
					}
					wuchance /= 4;
				}

				Mob* bo = GetBotOwner();
				if (bo && bo->IsClient() && bo->CastToClient()->GetBotOption(Client::booMonkWuMessage)) {

					bo->Message(
						GENERIC_EMOTE,
						"The spirit of Master Wu fills %s!  %s gains %d additional attack(s).",
						GetCleanName(),
						GetCleanName(),
						extra
					);
				}

				auto classic = RuleB(Combat, ClassicMasterWu);
				while (extra) {
					MonkSpecialAttack(GetTarget(), (classic ? MonkSPA[zone->random.Int(0, 4)] : EQ::skills::SkillTigerClaw));
					--extra;
				}
			}

			float HasteModifier = (GetHaste() * 0.01f);
			monkattack_timer.Start((reuse * 1000) / HasteModifier);

			break;
		}
		default:
			break;;
		}
	}

	if (!ca_time) {
		return;
	}

	float HasteModifier = (GetHaste() * 0.01f);
	uint16 skill_to_use = -1;
	int level = GetLevel();
	int reuse = (TauntReuseTime * 1000);
	bool did_attack = false;
	switch (GetClass()) {
		case WARRIOR:
			if (level >= RuleI(Combat, NPCBashKickLevel)) {
				bool canBash = false;
				if ((GetRace() == OGRE || GetRace() == TROLL || GetRace() == BARBARIAN) || (m_inv.GetItem(EQ::invslot::slotSecondary) && m_inv.GetItem(EQ::invslot::slotSecondary)->GetItem()->ItemType == EQ::item::ItemTypeShield) || (m_inv.GetItem(EQ::invslot::slotPrimary) && m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->IsType2HWeapon() && GetAA(aa2HandBash) >= 1))
					canBash = true;

				if(!canBash || zone->random.Int(0, 100) > 25)
					skill_to_use = EQ::skills::SkillKick;
				else
					skill_to_use = EQ::skills::SkillBash;
			}
		case RANGER:
		case BEASTLORD:
			skill_to_use = EQ::skills::SkillKick;
			break;
		case BERSERKER:
			skill_to_use = EQ::skills::SkillFrenzy;
			break;
		case CLERIC:
		case SHADOWKNIGHT:
		case PALADIN:
			if (level >= RuleI(Combat, NPCBashKickLevel)) {
				if ((GetRace() == OGRE || GetRace() == TROLL || GetRace() == BARBARIAN) || (m_inv.GetItem(EQ::invslot::slotSecondary) && m_inv.GetItem(EQ::invslot::slotSecondary)->GetItem()->ItemType == EQ::item::ItemTypeShield) || (m_inv.GetItem(EQ::invslot::slotPrimary) && m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->IsType2HWeapon() && GetAA(aa2HandBash) >= 1))
					skill_to_use = EQ::skills::SkillBash;
			}
			break;
		case MONK:
			if (GetLevel() >= 30) {
				skill_to_use = EQ::skills::SkillFlyingKick;
			}
			else if (GetLevel() >= 25) {
				skill_to_use = EQ::skills::SkillDragonPunch;
			}
			else if (GetLevel() >= 20) {
				skill_to_use = EQ::skills::SkillEagleStrike;
			}
			else if (GetLevel() >= 5) {
				skill_to_use = EQ::skills::SkillRoundKick;
			}
			else {
				skill_to_use = EQ::skills::SkillKick;
			}

			break;
		case ROGUE:
			skill_to_use = EQ::skills::SkillBackstab;
			break;
	}

	if(skill_to_use == -1)
		return;

	int64 dmg = GetBaseSkillDamage(static_cast<EQ::skills::SkillType>(skill_to_use), GetTarget());

	if (skill_to_use == EQ::skills::SkillBash) {
		if (target != this) {
			DoAnim(animTailRake);
			if (GetWeaponDamage(target, GetBotItem(EQ::invslot::slotSecondary)) <= 0 && GetWeaponDamage(target, GetBotItem(EQ::invslot::slotShoulders)) <= 0)
				dmg = DMG_INVULNERABLE;

			reuse = (BashReuseTime * 1000);
			DoSpecialAttackDamage(target, EQ::skills::SkillBash, dmg, 0, -1, reuse);
			did_attack = true;
		}
	}

	if (skill_to_use == EQ::skills::SkillFrenzy) {
		int AtkRounds = 3;
		DoAnim(anim2HSlashing);

		reuse = (FrenzyReuseTime * 1000);
		did_attack = true;
		while(AtkRounds > 0) {
			if (GetTarget() && (AtkRounds == 1 || zone->random.Int(0, 100) < 75)) {
				DoSpecialAttackDamage(GetTarget(), EQ::skills::SkillFrenzy, dmg, 0, dmg, reuse, true);
			}

			AtkRounds--;
		}
	}

	if (skill_to_use == EQ::skills::SkillKick) {
		if(target != this) {
			DoAnim(animKick);
			if (GetWeaponDamage(target, GetBotItem(EQ::invslot::slotFeet)) <= 0)
				dmg = DMG_INVULNERABLE;

			reuse = (KickReuseTime * 1000);
			DoSpecialAttackDamage(target, EQ::skills::SkillKick, dmg, 0, -1, reuse);
			did_attack = true;
		}
	}

	if (
		skill_to_use == EQ::skills::SkillFlyingKick ||
		skill_to_use == EQ::skills::SkillDragonPunch ||
		skill_to_use == EQ::skills::SkillEagleStrike ||
		skill_to_use == EQ::skills::SkillRoundKick
	) {
		reuse = (MonkSpecialAttack(target, skill_to_use) - 1);

		// Live AA - Technique of Master Wu
		int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;

		if (wuchance) {
			const int MonkSPA[5] = {
				EQ::skills::SkillFlyingKick,
				EQ::skills::SkillDragonPunch,
				EQ::skills::SkillEagleStrike,
				EQ::skills::SkillTigerClaw,
				EQ::skills::SkillRoundKick
			};
			int extra = 0;
			// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
			while (wuchance > 0) {
				if (zone->random.Roll(wuchance)) {
					++extra;
				}
				else {
					break;
				}
				wuchance /= 4;
			}

			Mob* bo = GetBotOwner();
			if (bo && bo->IsClient() && bo->CastToClient()->GetBotOption(Client::booMonkWuMessage)) {

				bo->Message(
					GENERIC_EMOTE,
					"The spirit of Master Wu fills %s!  %s gains %d additional attack(s).",
					GetCleanName(),
					GetCleanName(),
					extra
				);
			}

			auto classic = RuleB(Combat, ClassicMasterWu);
			while (extra) {
				MonkSpecialAttack(GetTarget(), (classic ? MonkSPA[zone->random.Int(0, 4)] : skill_to_use));
				--extra;
			}
		}

		reuse *= 1000;
		did_attack = true;
	}

	if (skill_to_use == EQ::skills::SkillBackstab) {
		reuse = (BackstabReuseTime * 1000);
		did_attack = true;
		if (IsRiposte)
			reuse = 0;

		TryBackstab(target,reuse);
	}
	classattack_timer.Start(reuse / HasteModifier);
}

void Bot::MakePet(uint16 spell_id, const char* pettype, const char *petname) {
	Mob::MakePet(spell_id, pettype, petname);
}

void Bot::AI_Stop() {
	NPC::AI_Stop();
	Mob::AI_Stop();
}

FACTION_VALUE Bot::GetReverseFactionCon(Mob* iOther) {
	if(iOther->IsBot())
		return FACTION_ALLY;

	return NPC::GetReverseFactionCon(iOther);
}

Mob* Bot::GetOwnerOrSelf() {
	Mob* Result = nullptr;
	if(GetBotOwner())
		Result = GetBotOwner();
	else
		Result = this;

	return Result;
}

Mob* Bot::GetOwner() {
	Mob* Result = nullptr;
	Result = GetBotOwner();
	if(!Result)
		SetBotOwner(0);

	return Result;
}

bool Bot::IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined) {
	bool Result = false;
	if(attacker && target) {
		if(attacker == target) {
			hasRuleDefined = true;
			Result = false;
		} else if(attacker->IsClient() && target->IsBot() && attacker->CastToClient()->GetPVP() && target->CastToBot()->GetBotOwner()->CastToClient()->GetPVP()) {
			hasRuleDefined = true;
			Result = true;
		} else if(attacker->IsClient() && target->IsBot()) {
			hasRuleDefined = true;
			Result = false;
		} else if(attacker->IsBot() && target->IsNPC()) {
			hasRuleDefined = true;
			Result = true;
		} else if(attacker->IsBot() && !target->IsNPC()) {
			hasRuleDefined = true;
			Result = false;
		} else if(attacker->IsPet() && attacker->IsFamiliar()) {
			hasRuleDefined = true;
			Result = false;
		} else if(attacker->IsBot() && attacker->CastToBot()->GetBotOwner() && attacker->CastToBot()->GetBotOwner()->CastToClient()->GetPVP()) {
			if(target->IsBot() && target->GetOwner() && target->GetOwner()->CastToClient()->GetPVP()) {
				hasRuleDefined = true;
				if(target->GetOwner() == attacker->GetOwner())
					Result = false;
				else
					Result = true;
			} else if(target->IsClient() && target->CastToClient()->GetPVP()) {
				hasRuleDefined = true;
				if(target == attacker->GetOwner())
					Result = false;
				else
					Result = true;
			} else if(target->IsNPC()) {
				hasRuleDefined = true;
				Result = true;
			} else if(!target->IsNPC()) {
				hasRuleDefined = true;
				Result = false;
			}
		}
	}
	return Result;
}

void Bot::EquipBot(std::string* error_message) {
	GetBotItems(m_inv, error_message);
	const EQ::ItemInstance* inst = nullptr;
	const EQ::ItemData* item = nullptr;
	for (int slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (inst) {
			item = inst->GetItem();
			BotTradeAddItem(inst, slot_id, error_message, false);
			if (!error_message->empty()) {
				return;
			}
		}
	}
	UpdateEquipmentLight();
}

void Bot::BotOrderCampAll(Client* c, uint8 class_id) {
	if (c) {
		const auto& l = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());
		for (const auto& b : l) {
			if (!class_id || b->GetClass() == class_id) {
				b->Camp();
			}
		}
	}
}

void Bot::ProcessBotOwnerRefDelete(Mob* botOwner) {
	if(botOwner) {
		if(botOwner->IsClient()) {
			std::list<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(botOwner->CastToClient()->CharacterID());
			if(!BotList.empty()) {
				for(std::list<Bot*>::iterator botListItr = BotList.begin(); botListItr != BotList.end(); ++botListItr) {
					Bot* tempBot = *botListItr;
					if(tempBot) {
						tempBot->SetTarget(0);
						tempBot->SetBotOwner(0);
					}
				}
			}
		}
	}
}

int64 Bot::CalcMaxMana() {
	switch(GetCasterClass()) {
		case 'I':
		case 'W': {
			max_mana = (GenerateBaseManaPoints() + itembonuses.Mana + spellbonuses.Mana + GroupLeadershipAAManaEnhancement());
			break;
		}
		case 'N': {
			max_mana = 0;
			break;
		}
		default: {
			LogDebug("Invalid Class [{}] in CalcMaxMana", GetCasterClass());
			max_mana = 0;
			break;
		}
	}

	if(current_mana > max_mana)
		current_mana = max_mana;
	else if(max_mana < 0)
		max_mana = 0;

	return max_mana;
}

void Bot::SetAttackTimer() {
	float haste_mod = (GetHaste() * 0.01f);
	attack_timer.SetAtTrigger(4000, true);
	Timer* TimerToUse = nullptr;
	const EQ::ItemData* PrimaryWeapon = nullptr;
	for (int i = EQ::invslot::slotRange; i <= EQ::invslot::slotSecondary; i++) {
		if (i == EQ::invslot::slotPrimary)
			TimerToUse = &attack_timer;
		else if (i == EQ::invslot::slotRange)
			TimerToUse = &ranged_timer;
		else if (i == EQ::invslot::slotSecondary)
			TimerToUse = &attack_dw_timer;
		else
			continue;

		const EQ::ItemData* ItemToUse = nullptr;
		EQ::ItemInstance* ci = GetBotItem(i);
		if (ci)
			ItemToUse = ci->GetItem();

		if (i == EQ::invslot::slotSecondary) {
			if (PrimaryWeapon != nullptr) {
				if (PrimaryWeapon->IsClassCommon() && PrimaryWeapon->IsType2HWeapon()) {
					attack_dw_timer.Disable();
					continue;
				}
			}

			if (!GetSkill(EQ::skills::SkillDualWield)) {
				attack_dw_timer.Disable();
				continue;
			}
		}

		if (ItemToUse != nullptr) {
			if (!ItemToUse->IsClassCommon() || ItemToUse->Damage == 0 || ItemToUse->Delay == 0 || ((ItemToUse->ItemType > EQ::item::ItemTypeLargeThrowing) && (ItemToUse->ItemType != EQ::item::ItemTypeMartial) && (ItemToUse->ItemType != EQ::item::ItemType2HPiercing)))
				ItemToUse = nullptr;
		}

		int hhe = (itembonuses.HundredHands + spellbonuses.HundredHands);
		int speed = 0;
		int delay = 36;
		if (ItemToUse == nullptr) {
			delay = GetHandToHandDelay();
		} else {
			delay = ItemToUse->Delay;
		}

		speed = (RuleB(Spells, Jun182014HundredHandsRevamp) ? static_cast<int>(((delay / haste_mod) + ((hhe / 1000.0f) * (delay / haste_mod))) * 100) : static_cast<int>(((delay / haste_mod) + ((hhe / 100.0f) * delay)) * 100));
		TimerToUse->SetAtTrigger(std::max(RuleI(Combat, MinHastedDelay), speed), true, true);

		if (i == EQ::invslot::slotPrimary)
			PrimaryWeapon = ItemToUse;
	}
}

int32 Bot::GetActSpellDuration(uint16 spell_id, int32 duration) {
	int increase = 100;
	increase += GetFocusEffect(focusSpellDuration, spell_id);
	int64 tic_inc = 0;
	tic_inc = GetFocusEffect(focusSpellDurByTic, spell_id);

	if(IsBeneficialSpell(spell_id)) {
		switch (GetAA(aaSpellCastingReinforcement)) {
			case 1:
				increase += 5;
				break;
			case 2:
				increase += 15;
				break;
			case 3:
				increase += 30;
				if (GetAA(aaSpellCastingReinforcementMastery) == 1)
					increase += 20;

				break;
		}

		if (GetAA(aaSpellCastingReinforcementMastery))
			increase += 20;
	}

	if(IsMezSpell(spell_id))
		tic_inc += GetAA(aaMesmerizationMastery);

	return (((duration * increase) / 100) + tic_inc);
}

float Bot::GetAOERange(uint16 spell_id) {
	float range;
	range = spells[spell_id].aoe_range;
	if(range == 0)
		range = spells[spell_id].range;

	if(range == 0)
		range = 10;

	if(IsBardSong(spell_id) && IsBeneficialSpell(spell_id)) {
		float song_bonus = (aabonuses.SongRange + spellbonuses.SongRange + itembonuses.SongRange);
		range += (range * song_bonus / 100.0f);
	}
	range = GetActSpellRange(spell_id, range);
	return range;
}

bool Bot::SpellEffect(Mob* caster, uint16 spell_id, float partial) {
	bool Result = false;
	Result = Mob::SpellEffect(caster, spell_id, partial);
	if(IsGrouped()) {
		Group *g = GetGroup();
		if(g) {
			EQApplicationPacket hp_app;
			CreateHPPacket(&hp_app);
			for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if(g->members[i] && g->members[i]->IsClient())
					g->members[i]->CastToClient()->QueuePacket(&hp_app);
			}
		}
	}
	return Result;
}

void Bot::DoBuffTic(const Buffs_Struct &buff, int slot, Mob* caster) {
	Mob::DoBuffTic(buff, slot, caster);
}

bool Bot::CastSpell(
	uint16 spell_id,
	uint16 target_id,
	EQ::spells::CastingSlot slot,
	int32 cast_time,
	int32 mana_cost,
	uint32* oSpellWillFinish,
	uint32 item_slot,
	int16 *resist_adjust,
	uint32 aa_id
) {
	bool Result = false;
	if (zone && !zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))) {
		// LogSpells("CastSpell called for spell [{}] ([{}]) on entity [{}], slot [{}], time [{}], mana [{}], from item slot [{}]", spells[spell_id].name, spell_id, target_id, slot, cast_time, mana_cost, (item_slot==0xFFFFFFFF)?999:item_slot);

		if (casting_spell_id == spell_id) {
			ZeroCastingVars();
		}

		if (GetClass() != BARD) {
			if (
				!IsValidSpell(spell_id) ||
				casting_spell_id ||
				delaytimer ||
				spellend_timer.Enabled() ||
				IsStunned() ||
				IsFeared() ||
				IsMezzed() ||
				(IsSilenced() && !IsDiscipline(spell_id)) ||
				(IsAmnesiad() && IsDiscipline(spell_id))
			) {
				LogSpellsDetail("Spell casting canceled: not able to cast now. Valid? [{}] casting [{}] waiting? [{}] spellend? [{}] stunned? [{}] feared? [{}] mezed? [{}] silenced? [{}]",
					IsValidSpell(spell_id), casting_spell_id, delaytimer, spellend_timer.Enabled(), IsStunned(), IsFeared(), IsMezzed(), IsSilenced()
				);
				if (IsSilenced() && !IsDiscipline(spell_id)) {
					MessageString(Chat::White, SILENCED_STRING);
				}

				if (IsAmnesiad() && IsDiscipline(spell_id)) {
					MessageString(Chat::White, MELEE_SILENCE);
				}

				if (casting_spell_id) {
					AI_Bot_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
				}

				return false;
			}
		}

		if (IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()) {
			MessageString(Chat::White, SPELL_WOULDNT_HOLD);
			if (casting_spell_id) {
				AI_Bot_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
			}

			return false;
		}

		if (DivineAura()) {
			LogSpellsDetail("Spell casting canceled: cannot cast while Divine Aura is in effect");
			InterruptSpell(173, 0x121, false);
			return false;
		}

		if (slot < EQ::spells::CastingSlot::MaxGems && !CheckFizzle(spell_id)) {
			int fizzle_msg = IsBardSong(spell_id) ? MISS_NOTE : SPELL_FIZZLE;
			InterruptSpell(fizzle_msg, 0x121, spell_id);

			uint32 use_mana = ((spells[spell_id].mana) / 4);
			LogSpellsDetail("Spell casting canceled: fizzled. [{}] mana has been consumed", use_mana);
			SetMana(GetMana() - use_mana);
			return false;
		}

		if (HasActiveSong()) {
			LogSpellsDetail("Casting a new spell/song while singing a song. Killing old song [{}]", bardsong);
			bardsong = 0;
			bardsong_target_id = 0;
			bardsong_slot = EQ::spells::CastingSlot::Gem1;
			bardsong_timer.Disable();
		}

		Result = Mob::CastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, 0xFFFFFFFF, 0, resist_adjust, aa_id);

	}
	return Result;
}

bool Bot::SpellOnTarget(uint16 spell_id, Mob* spelltar) {
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (spelltar) {
		if (
			IsDetrimentalSpell(spell_id) &&
			(spelltar->IsBot() || spelltar->IsClient()) &&
			!IsResurrectionEffects(spell_id)
		) {
			return false;
		}

		if (spelltar->IsPet()) {
			for (int i = 0; i < EFFECT_COUNT; ++i) {
				if (spells[spell_id].effect_id[i] == SE_Illusion) {
					return false;
				}
			}
		}

		return Mob::SpellOnTarget(spell_id, spelltar);
	}

	return false;
}

bool Bot::IsImmuneToSpell(uint16 spell_id, Mob *caster) {
	bool Result = false;
	if(!caster)
		return false;

	if(!IsSacrificeSpell(spell_id) && !(zone->GetZoneID() == 202) && !(this == caster)) {
		Result = Mob::IsImmuneToSpell(spell_id, caster);
		if(!Result) {
			if(caster->IsBot()) {
				if(spells[spell_id].target_type == ST_Undead) {
					if((GetBodyType() != BT_SummonedUndead) && (GetBodyType() != BT_Undead) && (GetBodyType() != BT_Vampire)) {
						LogSpellsDetail("Bot's target is not an undead");
						return true;
					}
				}
				if(spells[spell_id].target_type == ST_Summoned) {
					if((GetBodyType() != BT_SummonedUndead) && (GetBodyType() != BT_Summoned) && (GetBodyType() != BT_Summoned2) && (GetBodyType() != BT_Summoned3)) {
						LogSpellsDetail("Bot's target is not a summoned creature");
						return true;
					}
				}
			}

			LogSpellsDetail("No bot immunities to spell [{}] found", spell_id);
		}
	}

	return Result;
}

bool Bot::DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, EQ::spells::CastingSlot slot) {
	bool Result = false;
	SpellTargetType targetType = spells[spell_id].target_type;
	if(targetType == ST_GroupClientAndPet) {
		if((spell_id == 1768 && zone->GetZoneID() == 202) || (!IsDetrimentalSpell(spell_id))) {
			CastAction = SingleTarget;
			return true;
		}
	}
	Result = Mob::DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction, slot);
	return Result;
}

bool Bot::DoCastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot, int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish, uint32 item_slot, uint32 aa_id) {
	bool Result = false;
	if(GetClass() == BARD)
		cast_time = 0;

	Result = Mob::DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, aa_id);

	if(oSpellWillFinish) {
		const SPDat_Spell_Struct &spell = spells[spell_id];
		*oSpellWillFinish = Timer::GetCurrentTime() + ((spell.recast_time > 20000) ? 10000 : spell.recast_time);
	}
	return Result;
}

int32 Bot::GenerateBaseManaPoints() {
	int32 bot_mana = 0;
	int32 WisInt = 0;
	int32 MindLesserFactor, MindFactor;
	int wisint_mana = 0;
	int base_mana = 0;
	int ConvertedWisInt = 0;
	switch(GetCasterClass()) {
		case 'I':
			WisInt = INT;
			if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
				if(WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if(WisInt > 201)
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
				}
				else
					ConvertedWisInt = WisInt;

				if(GetLevel() < 41) {
					wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
					base_mana = (GetLevel() * 15);
				} else if(GetLevel() < 81) {
					wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
					base_mana = (600 + ((GetLevel() - 40) * 30));
				} else {
					wisint_mana = (9 * ConvertedWisInt);
					base_mana = (1800 + ((GetLevel() - 80) * 18));
				}
				bot_mana = (base_mana + wisint_mana);
			} else {
				if(((WisInt - 199) / 2) > 0)
					MindLesserFactor = ((WisInt - 199) / 2);
				else
					MindLesserFactor = 0;

				MindFactor = WisInt - MindLesserFactor;
				if(WisInt > 100)
					bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				else
					bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
			break;
		case 'W':
			WisInt = WIS;
			if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
				if(WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if(WisInt > 201)
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
				} else
					ConvertedWisInt = WisInt;

				if(GetLevel() < 41) {
					wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
					base_mana = (GetLevel() * 15);
				} else if(GetLevel() < 81) {
					wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
					base_mana = (600 + ((GetLevel() - 40) * 30));
				} else {
					wisint_mana = (9 * ConvertedWisInt);
					base_mana = (1800 + ((GetLevel() - 80) * 18));
				}
				bot_mana = (base_mana + wisint_mana);
			} else {
				if(((WisInt - 199) / 2) > 0)
					MindLesserFactor = ((WisInt - 199) / 2);
				else
					MindLesserFactor = 0;

				MindFactor = (WisInt - MindLesserFactor);
				if(WisInt > 100)
					bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				else
					bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
			break;
		default:
			bot_mana = 0;
			break;
	}
	max_mana = bot_mana;
	return bot_mana;
}

void Bot::GenerateSpecialAttacks() {
	if(((GetClass() == MONK) || (GetClass() == WARRIOR) || (GetClass() == RANGER) || (GetClass() == BERSERKER))	&& (GetLevel() >= 60))
		SetSpecialAbility(SPECATK_TRIPLE, 1);
}

bool Bot::DoFinishedSpellAETarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool& stopLogic) {
	if(GetClass() == BARD) {
		if(!ApplyBardPulse(bardsong, this, bardsong_slot))
			InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);

		stopLogic = true;
	}
	return true;
}

bool Bot::DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool& stopLogic) {
	if(spellTarget) {
		if(IsGrouped() && (spellTarget->IsBot() || spellTarget->IsClient()) && RuleB(Bots, GroupBuffing)) {
			bool noGroupSpell = false;
			uint16 thespell = spell_id;
			for (int i = 0; i < AIBot_spells.size(); i++) {
				int j = BotGetSpells(i);
				int spelltype = BotGetSpellType(i);
				bool spellequal = (j == thespell);
				bool spelltypeequal = ((spelltype == 2) || (spelltype == 16) || (spelltype == 32));
				bool spelltypetargetequal = ((spelltype == 8) && (spells[thespell].target_type == ST_Self));
				bool spelltypeclassequal = ((spelltype == 1024) && (GetClass() == SHAMAN));
				bool slotequal = (slot == EQ::spells::CastingSlot::Item);
				if(spellequal || slotequal) {
					if((spelltypeequal || spelltypetargetequal) || spelltypeclassequal || slotequal) {
						if(((spells[thespell].effect_id[0] == 0) && (spells[thespell].base_value[0] < 0)) &&
							(spellTarget->GetHP() < ((spells[thespell].base_value[0] * (-1)) + 100))) {
							LogSpells("GroupBuffing failure");
							return false;
						}

						SpellOnTarget(thespell, spellTarget);
						noGroupSpell = true;
						stopLogic = true;
					}
				}
			}

			if(!noGroupSpell) {
				Group *g = GetGroup();
				if(g) {
					for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
						if(g->members[i]) {
							if((g->members[i]->GetClass() == NECROMANCER) && (IsEffectInSpell(thespell, SE_AbsorbMagicAtt) || IsEffectInSpell(thespell, SE_Rune))) {
							}
							else
								SpellOnTarget(thespell, g->members[i]);

							if(g->members[i] && g->members[i]->GetPetID())
								SpellOnTarget(thespell, g->members[i]->GetPet());
						}
					}
					SetMana(GetMana() - (GetActSpellCost(thespell, spells[thespell].mana) * (g->GroupCount() - 1)));
				}
			}
			stopLogic = true;
		}
	}
	return true;
}

bool Bot::DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool& stopLogic) {
	bool isMainGroupMGB = false;
	if(isMainGroupMGB && (GetClass() != BARD)) {
		BotGroupSay(
			this,
			fmt::format(
				"Casting {} as a Mass Group Buff.",
				spells[spell_id].name
			).c_str()
		);
		SpellOnTarget(spell_id, this);
		entity_list.AESpell(this, this, spell_id, true);
	} else {
		Group *g = GetGroup();
		if(g) {
			for(int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if(g->members[i]) {
					SpellOnTarget(spell_id, g->members[i]);
					if(g->members[i] && g->members[i]->GetPetID())
						SpellOnTarget(spell_id, g->members[i]->GetPet());
				}
			}
		}
	}
	stopLogic = true;
	return true;
}

void Bot::CalcBonuses() {
	memset(&itembonuses, 0, sizeof(StatBonuses));
	GenerateBaseStats();
	CalcItemBonuses(&itembonuses);
	CalcSpellBonuses(&spellbonuses);
	CalcAABonuses(&aabonuses);
	SetAttackTimer();
	CalcSeeInvisibleLevel();
	CalcInvisibleLevel();
	CalcATK();
	CalcSTR();
	CalcSTA();
	CalcDEX();
	CalcAGI();
	CalcINT();
	CalcWIS();
	CalcCHA();
	CalcMR();
	CalcFR();
	CalcDR();
	CalcPR();
	CalcCR();
	CalcCorrup();
	CalcAC();
	CalcMaxHP();
	CalcMaxMana();
	CalcMaxEndurance();
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
}

int64 Bot::CalcHPRegenCap() {
	int level = GetLevel();
	int64 hpregen_cap = 0;
	hpregen_cap = (RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA / 25);
	hpregen_cap += (aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap);
	return (hpregen_cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int64 Bot::CalcManaRegenCap() {
	int64 cap = RuleI(Character, ItemManaRegenCap) + aabonuses.ItemManaRegenCap;
	switch(GetCasterClass()) {
		case 'I':
			cap += (itembonuses.HeroicINT / 25);
			break;
		case 'W':
			cap += (itembonuses.HeroicWIS / 25);
			break;
	}
	return (cap * RuleI(Character, ManaRegenMultiplier) / 100);
}

int32 Bot::GetMaxStat() {
	int level = GetLevel();
	int32 base = 0;
	if (level < 61)
		base = 255;
	else if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoF)
		base = (255 + 5 * (level - 60));
	else if (level < 71)
		base = (255 + 5 * (level - 60));
	else
		base = 330;

	return base;
}

int32 Bot::GetMaxResist() {
	int level = GetLevel();
	int32 base = 500;
	if(level > 60)
		base += ((level - 60) * 5);

	return base;
}

int32 Bot::GetMaxSTR() {
	return (GetMaxStat() + itembonuses.STRCapMod + spellbonuses.STRCapMod + aabonuses.STRCapMod);
}

int32 Bot::GetMaxSTA() {
	return (GetMaxStat() + itembonuses.STACapMod + spellbonuses.STACapMod + aabonuses.STACapMod);
}

int32 Bot::GetMaxDEX() {
	return (GetMaxStat() + itembonuses.DEXCapMod + spellbonuses.DEXCapMod + aabonuses.DEXCapMod);
}

int32 Bot::GetMaxAGI() {
	return (GetMaxStat() + itembonuses.AGICapMod + spellbonuses.AGICapMod + aabonuses.AGICapMod);
}

int32 Bot::GetMaxINT() {
	return (GetMaxStat() + itembonuses.INTCapMod + spellbonuses.INTCapMod + aabonuses.INTCapMod);
}

int32 Bot::GetMaxWIS() {
	return (GetMaxStat() + itembonuses.WISCapMod + spellbonuses.WISCapMod + aabonuses.WISCapMod);
}
int32 Bot::GetMaxCHA() {
	return (GetMaxStat() + itembonuses.CHACapMod + spellbonuses.CHACapMod + aabonuses.CHACapMod);
}

int32 Bot::GetMaxMR() {
	return (GetMaxResist() + itembonuses.MRCapMod + spellbonuses.MRCapMod + aabonuses.MRCapMod);
}

int32 Bot::GetMaxPR() {
	return (GetMaxResist() + itembonuses.PRCapMod + spellbonuses.PRCapMod + aabonuses.PRCapMod);
}

int32 Bot::GetMaxDR() {
	return (GetMaxResist() + itembonuses.DRCapMod + spellbonuses.DRCapMod + aabonuses.DRCapMod);
}

int32 Bot::GetMaxCR() {
	return (GetMaxResist() + itembonuses.CRCapMod + spellbonuses.CRCapMod + aabonuses.CRCapMod);
}

int32 Bot::GetMaxFR() {
	return (GetMaxResist() + itembonuses.FRCapMod + spellbonuses.FRCapMod + aabonuses.FRCapMod);
}

int32 Bot::GetMaxCorrup() {
	return (GetMaxResist() + itembonuses.CorrupCapMod + spellbonuses.CorrupCapMod + aabonuses.CorrupCapMod);
}

int32 Bot::CalcSTR() {
	int32 val = (STR + itembonuses.STR + spellbonuses.STR);
	int32 mod = aabonuses.STR;
	if(val > 255 && GetLevel() <= 60)
		val = 255;

	STR = (val + mod);
	if(STR < 1)
		STR = 1;

	int m = GetMaxSTR();
	if(STR > m)
		STR = m;

	return STR;
}

int32 Bot::CalcSTA() {
	int32 val = (STA + itembonuses.STA + spellbonuses.STA);
	int32 mod = aabonuses.STA;
	if(val > 255 && GetLevel() <= 60)
		val = 255;

	STA = (val + mod);
	if(STA < 1)
		STA = 1;

	int m = GetMaxSTA();
	if(STA > m)
		STA = m;

	return STA;
}

int32 Bot::CalcAGI() {
	int32 val = (AGI + itembonuses.AGI + spellbonuses.AGI);
	int32 mod = aabonuses.AGI;
	if(val > 255 && GetLevel() <= 60)
		val = 255;

	AGI = (val + mod);
	if(AGI < 1)
		AGI = 1;

	int m = GetMaxAGI();
	if(AGI > m)
		AGI = m;

	return AGI;
}

int32 Bot::CalcDEX() {
	int32 val = (DEX + itembonuses.DEX + spellbonuses.DEX);
	int32 mod = aabonuses.DEX;
	if(val > 255 && GetLevel() <= 60)
		val = 255;

	DEX = (val + mod);
	if(DEX < 1)
		DEX = 1;

	int m = GetMaxDEX();
	if(DEX > m)
		DEX = m;

	return DEX;
}

int32 Bot::CalcINT() {
	int32 val = (INT + itembonuses.INT + spellbonuses.INT);
	int32 mod = aabonuses.INT;
	if(val > 255 && GetLevel() <= 60)
		val = 255;

	INT = (val + mod);

	if(INT < 1)
		INT = 1;

	int m = GetMaxINT();
	if(INT > m)
		INT = m;

	return INT;
}

int32 Bot::CalcWIS() {
	int32 val = (WIS + itembonuses.WIS + spellbonuses.WIS);
	int32 mod = aabonuses.WIS;
	if(val > 255 && GetLevel() <= 60)
		val = 255;

	WIS = (val + mod);

	if(WIS < 1)
		WIS = 1;

	int m = GetMaxWIS();
	if(WIS > m)
		WIS = m;

	return WIS;
}

int32 Bot::CalcCHA() {
	int32 val = (CHA + itembonuses.CHA + spellbonuses.CHA);
	int32 mod = aabonuses.CHA;
	if(val > 255 && GetLevel() <= 60)
		val = 255;

	CHA = (val + mod);

	if(CHA < 1)
		CHA = 1;

	int m = GetMaxCHA();
	if(CHA > m)
		CHA = m;

	return CHA;
}

int32 Bot::CalcMR() {
	MR += (itembonuses.MR + spellbonuses.MR + aabonuses.MR);
	if(GetClass() == WARRIOR)
		MR += (GetLevel() / 2);

	if(MR < 1)
		MR = 1;

	if(MR > GetMaxMR())
		MR = GetMaxMR();

	return MR;
}

int32 Bot::CalcFR() {
	int c = GetClass();
	if(c == RANGER) {
		FR += 4;
		int l = GetLevel();
		if(l > 49)
			FR += (l - 49);
	}

	FR += (itembonuses.FR + spellbonuses.FR + aabonuses.FR);

	if(FR < 1)
		FR = 1;

	if(FR > GetMaxFR())
		FR = GetMaxFR();

	return FR;
}

int32 Bot::CalcDR() {
	int c = GetClass();
	if(c == PALADIN) {
		DR += 8;
		int l = GetLevel();
		if(l > 49)
			DR += (l - 49);
	} else if(c == SHADOWKNIGHT) {
		DR += 4;
		int l = GetLevel();
		if(l > 49)
			DR += (l - 49);
	}

	DR += (itembonuses.DR + spellbonuses.DR + aabonuses.DR);
	if(DR < 1)
		DR = 1;

	if(DR > GetMaxDR())
		DR = GetMaxDR();

	return DR;
}

int32 Bot::CalcPR() {
	int c = GetClass();
	if(c == ROGUE) {
		PR += 8;
		int l = GetLevel();
		if(l > 49)
			PR += (l - 49);
	} else if(c == SHADOWKNIGHT) {
		PR += 4;
		int l = GetLevel();
		if(l > 49)
			PR += (l - 49);
	}

	PR += (itembonuses.PR + spellbonuses.PR + aabonuses.PR);

	if(PR < 1)
		PR = 1;

	if(PR > GetMaxPR())
		PR = GetMaxPR();

	return PR;
}

int32 Bot::CalcCR() {
	int c = GetClass();
	if(c == RANGER) {
		CR += 4;
		int l = GetLevel();
		if(l > 49)
			CR += (l - 49);
	}

	CR += (itembonuses.CR + spellbonuses.CR + aabonuses.CR);

	if(CR < 1)
		CR = 1;

	if(CR > GetMaxCR())
		CR = GetMaxCR();

	return CR;
}

int32 Bot::CalcCorrup() {
	Corrup = (Corrup + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup);
	if(Corrup > GetMaxCorrup())
		Corrup = GetMaxCorrup();

	return Corrup;
}

int32 Bot::CalcATK() {
	ATK = (itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement());
	return ATK;
}

void Bot::CalcRestState() {
	if(!RuleB(Character, RestRegenEnabled))
		return;

	RestRegenHP = RestRegenMana = RestRegenEndurance = 0;
	if(IsEngaged() || !IsSitting() || !rest_timer.Check(false))
		return;

	uint32 buff_count = GetMaxTotalSlots();
	for (unsigned int j = 0; j < buff_count; j++) {
		if(buffs[j].spellid != SPELL_UNKNOWN) {
			if(IsDetrimentalSpell(buffs[j].spellid) && (buffs[j].ticsremaining > 0))
				if(!DetrimentalSpellAllowsRest(buffs[j].spellid))
					return;
		}
	}

	RestRegenHP = 6 * (GetMaxHP() / zone->newzone_data.fast_regen_hp);
	RestRegenMana = 6 * (GetMaxMana() / zone->newzone_data.fast_regen_mana);
	RestRegenEndurance = 6 * (GetMaxEndurance() / zone->newzone_data.fast_regen_endurance);
}

int32 Bot::LevelRegen() {
	int level = GetLevel();
	bool bonus = GetPlayerRaceBit(_baseRace) & RuleI(Character, BaseHPRegenBonusRaces);
	uint8 multiplier1 = bonus ? 2 : 1;
	int64 hp = 0;
	if (level < 51) {
		if (IsSitting()) {
			if (level < 20)
				hp += (2 * multiplier1);
			else if (level < 50)
				hp += (3 * multiplier1);
			else
				hp += (4 * multiplier1);
		} else
			hp += (1 * multiplier1);
	} else {
		int32 tmp = 0;
		float multiplier2 = 1;
		if (level < 56) {
			tmp = 2;
			if (bonus)
				multiplier2 = 3;
		} else if (level < 60) {
			tmp = 3;
			if (bonus)
				multiplier2 = 3.34;
		}
		else if (level < 61) {
			tmp = 4;
			if (bonus)
				multiplier2 = 3;
		} else if (level < 63) {
			tmp = 5;
			if (bonus)
				multiplier2 = 2.8;
		} else if (level < 65) {
			tmp = 6;
			if (bonus)
				multiplier2 = 2.67;
		} else {
			tmp = 7;
			if (bonus)
				multiplier2 = 2.58;
		}
		hp += (int32(float(tmp) * multiplier2));
	}
	return hp;
}

int64 Bot::CalcHPRegen() {
	int32 regen = (LevelRegen() + itembonuses.HPRegen + spellbonuses.HPRegen);
	regen += (aabonuses.HPRegen + GroupLeadershipAAHealthRegeneration());
	regen = ((regen * RuleI(Character, HPRegenMultiplier)) / 100);
	return regen;
}

int64 Bot::CalcManaRegen() {
	uint8 level = GetLevel();
	uint8 botclass = GetClass();
	int32 regen = 0;
	if (IsSitting()) {
		BuffFadeBySitModifier();
		if(botclass != WARRIOR && botclass != MONK && botclass != ROGUE && botclass != BERSERKER) {
			regen = ((((GetSkill(EQ::skills::SkillMeditate) / 10) + (level - (level / 4))) / 4) + 4);
			regen += (spellbonuses.ManaRegen + itembonuses.ManaRegen);
		} else
			regen = (2 + spellbonuses.ManaRegen + itembonuses.ManaRegen);
	} else
		regen = (2 + spellbonuses.ManaRegen + itembonuses.ManaRegen);

	if(GetCasterClass() == 'I')
		regen += (itembonuses.HeroicINT / 25);
	else if(GetCasterClass() == 'W')
		regen += (itembonuses.HeroicWIS / 25);
	else
		regen = 0;

	regen += aabonuses.ManaRegen;
	regen = ((regen * RuleI(Character, ManaRegenMultiplier)) / 100);
	float mana_regen_rate = RuleR(Bots, ManaRegen);
	if(mana_regen_rate < 0.0f)
		mana_regen_rate = 0.0f;

	regen = (regen * mana_regen_rate);
	return regen;
}

uint64 Bot::GetClassHPFactor() {
	uint32 factor;
	switch (GetClass()) {
		case BEASTLORD:
		case BERSERKER:
		case MONK:
		case ROGUE:
		case SHAMAN:
			factor = 255;
			break;
		case BARD:
		case CLERIC:
			factor = 264;
			break;
		case SHADOWKNIGHT:
		case PALADIN:
			factor = 288;
			break;
		case RANGER:
			factor = 276;
			break;
		case WARRIOR:
			factor = 300;
			break;
		default:
			factor = 240;
			break;
	}
	return factor;
}

int64 Bot::CalcMaxHP() {
	int32 bot_hp = 0;
	uint32 nd = 10000;
	bot_hp += (GenerateBaseHitPoints() + itembonuses.HP);
	nd += aabonuses.MaxHP;
	bot_hp = ((float)bot_hp * (float)nd / (float)10000);
	bot_hp += (spellbonuses.HP + aabonuses.HP);
	bot_hp += GroupLeadershipAAHealthEnhancement();
	bot_hp += (bot_hp * ((spellbonuses.MaxHPChange + itembonuses.MaxHPChange) / 10000.0f));
	max_hp = bot_hp;
	if (current_hp > max_hp)
		current_hp = max_hp;

	int hp_perc_cap = spellbonuses.HPPercCap[0];
	if(hp_perc_cap) {
		int curHP_cap = ((max_hp * hp_perc_cap) / 100);
		if (current_hp > curHP_cap || (spellbonuses.HPPercCap[1] && current_hp > spellbonuses.HPPercCap[1]))
			current_hp = curHP_cap;
	}
	return max_hp;
}

int64 Bot::CalcMaxEndurance() {
	max_end = (CalcBaseEndurance() + spellbonuses.Endurance + itembonuses.Endurance);
	if (max_end < 0)
		max_end = 0;

	if (cur_end > max_end)
		cur_end = max_end;

	int end_perc_cap = spellbonuses.EndPercCap[0];
	if(end_perc_cap) {
		int curEnd_cap = ((max_end * end_perc_cap) / 100);
		if (cur_end > curEnd_cap || (spellbonuses.EndPercCap[1] && cur_end > spellbonuses.EndPercCap[1]))
			cur_end = curEnd_cap;
	}

	return max_end;
}

int64 Bot::CalcBaseEndurance() {
	int32 base_end = 0;
	int32 base_endurance = 0;
	int32 ConvertedStats = 0;
	int32 sta_end = 0;
	int Stats = 0;
	if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		int HeroicStats = 0;
		Stats = ((GetSTR() + GetSTA() + GetDEX() + GetAGI()) / 4);
		HeroicStats = ((GetHeroicSTR() + GetHeroicSTA() + GetHeroicDEX() + GetHeroicAGI()) / 4);
		if (Stats > 100) {
			ConvertedStats = (((Stats - 100) * 5 / 2) + 100);
			if (Stats > 201)
				ConvertedStats -= ((Stats - 201) * 5 / 4);
		} else
			ConvertedStats = Stats;

		if (GetLevel() < 41) {
			sta_end = (GetLevel() * 75 * ConvertedStats / 1000);
			base_endurance = (GetLevel() * 15);
		} else if (GetLevel() < 81) {
			sta_end = ((3 * ConvertedStats) + ((GetLevel() - 40) * 15 * ConvertedStats / 100));
			base_endurance = (600 + ((GetLevel() - 40) * 30));
		} else {
			sta_end = (9 * ConvertedStats);
			base_endurance = (1800 + ((GetLevel() - 80) * 18));
		}
		base_end = (base_endurance + sta_end + (HeroicStats * 10));
	} else {
		Stats = (GetSTR()+GetSTA()+GetDEX()+GetAGI());
		int LevelBase = (GetLevel() * 15);
		int at_most_800 = Stats;
		if(at_most_800 > 800)
			at_most_800 = 800;

		int Bonus400to800 = 0;
		int HalfBonus400to800 = 0;
		int Bonus800plus = 0;
		int HalfBonus800plus = 0;
		int BonusUpto800 = int(at_most_800 / 4) ;
		if(Stats > 400) {
			Bonus400to800 = int((at_most_800 - 400) / 4);
			HalfBonus400to800 = int(std::max((at_most_800 - 400), 0) / 8);
			if(Stats > 800) {
				Bonus800plus = (int((Stats - 800) / 8) * 2);
				HalfBonus800plus = int((Stats - 800) / 16);
			}
		}
		int bonus_sum = (BonusUpto800 + Bonus400to800 + HalfBonus400to800 + Bonus800plus + HalfBonus800plus);
		base_end = LevelBase;
		base_end += ((bonus_sum * 3 * GetLevel()) / 40);
	}
	return base_end;
}

int64 Bot::CalcEnduranceRegen() {
	int32 regen = (int32(GetLevel() * 4 / 10) + 2);
	regen += (spellbonuses.EnduranceRegen + itembonuses.EnduranceRegen);
	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int64 Bot::CalcEnduranceRegenCap() {
	int cap = (RuleI(Character, ItemEnduranceRegenCap) + itembonuses.HeroicSTR / 25 + itembonuses.HeroicDEX / 25 + itembonuses.HeroicAGI / 25 + itembonuses.HeroicSTA / 25);
	return (cap * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

void Bot::SetEndurance(int32 newEnd) {
	if(newEnd < 0)
		newEnd = 0;
	else if(newEnd > GetMaxEndurance())
		newEnd = GetMaxEndurance();

	cur_end = newEnd;
}

void Bot::DoEnduranceUpkeep() {
	int upkeep_sum = 0;
	int cost_redux = (spellbonuses.EnduranceReduction + itembonuses.EnduranceReduction);
	uint32 buffs_i;
	uint32 buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if (buffs[buffs_i].spellid != SPELL_UNKNOWN) {
			int upkeep = spells[buffs[buffs_i].spellid].endurance_upkeep;
			if(upkeep > 0) {
				if(cost_redux > 0) {
					if(upkeep <= cost_redux)
						continue;

					upkeep -= cost_redux;
				}

				if((upkeep+upkeep_sum) > GetEndurance())
					BuffFadeBySlot(buffs_i);
				else
					upkeep_sum += upkeep;
			}
		}
	}

	if(upkeep_sum != 0)
		SetEndurance(GetEndurance() - upkeep_sum);
}

void Bot::Camp(bool save_to_database) {
	Sit();

	if (GetGroup()) {
		RemoveBotFromGroup(this, GetGroup());
	}

	LeaveHealRotationMemberPool();

	if (save_to_database) {
		Save();
	}

	Depop();
}

void Bot::Zone() {
	if(HasGroup())
		GetGroup()->MemberZoned(this);

	Save();
	Depop();
}

bool Bot::IsArcheryRange(Mob *target) {
	bool result = false;
	if(target) {
		float range = (GetBotArcheryRange() + 5.0);
		range *= range;
		float targetDistance = DistanceSquaredNoZ(m_Position, target->GetPosition());
		float minRuleDistance = (RuleI(Combat, MinRangedAttackDist) * RuleI(Combat, MinRangedAttackDist));
		if((targetDistance > range) || (targetDistance < minRuleDistance))
			result = false;
		else
			result = true;
	}
	return result;
}

void Bot::UpdateGroupCastingRoles(const Group* group, bool disband)
{
	if (!group)
		return;

	for (auto iter : group->members) {
		if (!iter)
			continue;

		if (iter->IsBot()) {
			iter->CastToBot()->SetGroupHealer(false);
			iter->CastToBot()->SetGroupSlower(false);
			iter->CastToBot()->SetGroupNuker(false);
			iter->CastToBot()->SetGroupDoter(false);
		}
	}

	if (disband)
		return;

	Mob* healer = nullptr;
	Mob* slower = nullptr;
	Mob* nuker = nullptr;
	Mob* doter = nullptr;

	for (auto iter : group->members) {
		if (!iter)
			continue;

		// GroupHealer
		switch (iter->GetClass()) {
		case CLERIC:
			if (!healer)
				healer = iter;
			else
				switch (healer->GetClass()) {
				case CLERIC:
					break;
				default:
					healer = iter;
				}

			break;
		case DRUID:
			if (!healer)
				healer = iter;
			else
				switch (healer->GetClass()) {
				case CLERIC:
				case DRUID:
					break;
				default:
					healer = iter;
				}
			break;
		case SHAMAN:
			if (!healer)
				healer = iter;
			else
				switch (healer->GetClass()) {
				case CLERIC:
				case DRUID:
				case SHAMAN:
					break;
				default:
					healer = iter;
				}
			break;
		case PALADIN:
		case RANGER:
		case BEASTLORD:
			if (!healer)
				healer = iter;
			break;
		default:
			break;
		}

		// GroupSlower
		switch (iter->GetClass()) {
		case SHAMAN:
			if (!slower)
				slower = iter;
			else
				switch (slower->GetClass()) {
				case SHAMAN:
					break;
				default:
					slower = iter;
				}
			break;
		case ENCHANTER:
			if (!slower)
				slower = iter;
			else
				switch (slower->GetClass()) {
				case SHAMAN:
				case ENCHANTER:
					break;
				default:
					slower = iter;
				}
			break;
		case BEASTLORD:
			if (!slower)
				slower = iter;
			break;
		default:
			break;
		}

		// GroupNuker
		switch (iter->GetClass()) {
			// wizard
			// magician
			// necromancer
			// enchanter
			// druid
			// cleric
			// shaman
			// shadowknight
			// paladin
			// ranger
			// beastlord
		default:
			break;
		}

		// GroupDoter
		switch (iter->GetClass()) {
		default:
			break;
		}
	}

	if (healer && healer->IsBot())
		healer->CastToBot()->SetGroupHealer();
	if (slower && slower->IsBot())
		slower->CastToBot()->SetGroupSlower();
	if (nuker && nuker->IsBot())
		nuker->CastToBot()->SetGroupNuker();
	if (doter && doter->IsBot())
		doter->CastToBot()->SetGroupDoter();
}

//void Bot::UpdateRaidCastingRoles(const Raid* raid, bool disband = false) { }

bool Bot::CanHeal() {
	bool result = false;

	if(!AI_HasSpells())
		return false;

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	botSpell = GetFirstBotSpellBySpellType(this, SpellType_Heal);

	if(botSpell.SpellId != 0)
		result = true;

	return result;
}

Bot* Bot::GetBotByBotClientOwnerAndBotName(Client* c, std::string botName) {
	Bot* Result = nullptr;
	if(c) {
		std::list<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());
		if(!BotList.empty()) {
			for(std::list<Bot*>::iterator botListItr = BotList.begin(); botListItr != BotList.end(); ++botListItr) {
				if(std::string((*botListItr)->GetCleanName()) == botName) {
					Result = (*botListItr);
					break;
				}
			}
		}
	}
	return Result;
}

void Bot::ProcessBotGroupInvite(Client* c, std::string botName) {
	if(c) {
		Bot* invitedBot = GetBotByBotClientOwnerAndBotName(c, botName);

		if(invitedBot && !invitedBot->HasGroup()) {
			if(!c->IsGrouped()) {
				Group *g = new Group(c);
				if(AddBotToGroup(invitedBot, g)) {
					entity_list.AddGroup(g);
					database.SetGroupLeaderName(g->GetID(), c->GetName());
					g->SaveGroupLeaderAA();
					database.SetGroupID(c->GetName(), g->GetID(), c->CharacterID());
					database.SetGroupID(invitedBot->GetCleanName(), g->GetID(), invitedBot->GetBotID());
				} else {
					delete g;
				}
			} else {
				AddBotToGroup(invitedBot, c->GetGroup());
				database.SetGroupID(invitedBot->GetCleanName(), c->GetGroup()->GetID(), invitedBot->GetBotID());
			}
		}
	}
}

// Processes a group disband request from a Client for a Bot.
void Bot::ProcessBotGroupDisband(Client* c, std::string botName) {
	if(c) {
		Bot* tempBot = nullptr;

		if(botName.empty())
			tempBot = GetFirstBotInGroup(c->GetGroup());
		else
			tempBot = GetBotByBotClientOwnerAndBotName(c, botName);

		RemoveBotFromGroup(tempBot, c->GetGroup());
	}
}

// Handles all client zone change event
void Bot::ProcessClientZoneChange(Client* botOwner) {
	if(botOwner) {
		std::list<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(botOwner->CharacterID());

		for(std::list<Bot*>::iterator itr = BotList.begin(); itr != BotList.end(); ++itr) {
			Bot* tempBot = *itr;

			if(tempBot) {
				if(tempBot->HasGroup()) {
					Group* g = tempBot->GetGroup();
					if(g && g->IsGroupMember(botOwner)) {
						if(botOwner && botOwner->IsClient()) {
							// Modified to not only zone bots if you're the leader.
							// Also zone bots of the non-leader when they change zone.
							if(tempBot->GetBotOwnerCharacterID() == botOwner->CharacterID() && g->IsGroupMember(botOwner))
								tempBot->Zone();
							else
								tempBot->Camp();
						}
					}
					else
						tempBot->Camp();
				}
				else
					tempBot->Camp();
			}
		}
	}
}

// Finds and returns the first Bot object found in specified group
Bot* Bot::GetFirstBotInGroup(Group* group) {
	Bot* Result = nullptr;

	if(group) {
		for(int Counter = 0; Counter < MAX_GROUP_MEMBERS; Counter++) {
			if (group->members[Counter] == nullptr) {
				continue;
			}

			if(group->members[Counter]->IsBot()) {
				Result = group->members[Counter]->CastToBot();
				break;
			}
		}
	}

	return Result;
}

// Processes a client request to inspect a bot's equipment.
void Bot::ProcessBotInspectionRequest(Bot* inspectedBot, Client* client) {
	if(inspectedBot && client) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_InspectAnswer, sizeof(InspectResponse_Struct));
		InspectResponse_Struct* insr = (InspectResponse_Struct*) outapp->pBuffer;
		insr->TargetID = inspectedBot->GetNPCTypeID();
		insr->playerid = inspectedBot->GetID();

		const EQ::ItemData* item = nullptr;
		const EQ::ItemInstance* inst = nullptr;

		for (int16 L = EQ::invslot::EQUIPMENT_BEGIN; L <= EQ::invslot::EQUIPMENT_END; L++) {
			inst = inspectedBot->GetBotItem(L);

			if(inst) {
				item = inst->GetItem();
				if(item) {
					strcpy(insr->itemnames[L], item->Name);
					insr->itemicons[L] = item->Icon;
				}
				else {
					insr->itemnames[L][0] = '\0';
					insr->itemicons[L] = 0xFFFFFFFF;
				}
			}
			else {
				insr->itemnames[L][0] = '\0';
				insr->itemicons[L] = 0xFFFFFFFF;
			}
		}

		strcpy(insr->text, inspectedBot->GetInspectMessage().text);

		client->QueuePacket(outapp); // Send answer to requester
		safe_delete(outapp);
	}
}

void Bot::CalcItemBonuses(StatBonuses* newbon)
{
	const EQ::ItemData* itemtmp = nullptr;

	for (int i = EQ::invslot::BONUS_BEGIN; i <= EQ::invslot::BONUS_STAT_END; ++i) {
		const EQ::ItemInstance* item = GetBotItem(i);
		if(item) {
			AddItemBonuses(item, newbon);
		}
	}

	// Caps
	if(newbon->HPRegen > CalcHPRegenCap())
		newbon->HPRegen = CalcHPRegenCap();

	if(newbon->ManaRegen > CalcManaRegenCap())
		newbon->ManaRegen = CalcManaRegenCap();

	if(newbon->EnduranceRegen > CalcEnduranceRegenCap())
		newbon->EnduranceRegen = CalcEnduranceRegenCap();
}

void Bot::AddItemBonuses(const EQ::ItemInstance *inst, StatBonuses* newbon, bool isAug, bool isTribute, int rec_override) {
	if (!inst || !inst->IsClassCommon())
	{
		return;
	}

	if(inst->GetAugmentType()==0 && isAug == true)
	{
		return;
	}

	const EQ::ItemData *item = inst->GetItem();

	if(!isTribute && !inst->IsEquipable(GetBaseRace(),GetClass()))
	{
		if (item->ItemType != EQ::item::ItemTypeFood && item->ItemType != EQ::item::ItemTypeDrink)
			return;
	}

	if(GetLevel() < inst->GetItemRequiredLevel(true))
	{
		return;
	}

	auto rec_level = isAug ? rec_override : inst->GetItemRecommendedLevel(true);
	if(GetLevel() >= rec_level)
	{
		newbon->AC += item->AC;
		newbon->HP += item->HP;
		newbon->Mana += item->Mana;
		newbon->Endurance += item->Endur;
		newbon->ATK += item->Attack;
		newbon->STR += (item->AStr + item->HeroicStr);
		newbon->STA += (item->ASta + item->HeroicSta);
		newbon->DEX += (item->ADex + item->HeroicDex);
		newbon->AGI += (item->AAgi + item->HeroicAgi);
		newbon->INT += (item->AInt + item->HeroicInt);
		newbon->WIS += (item->AWis + item->HeroicWis);
		newbon->CHA += (item->ACha + item->HeroicCha);

		newbon->MR += (item->MR + item->HeroicMR);
		newbon->FR += (item->FR + item->HeroicFR);
		newbon->CR += (item->CR + item->HeroicCR);
		newbon->PR += (item->PR + item->HeroicPR);
		newbon->DR += (item->DR + item->HeroicDR);
		newbon->Corrup += (item->SVCorruption + item->HeroicSVCorrup);

		newbon->STRCapMod += item->HeroicStr;
		newbon->STACapMod += item->HeroicSta;
		newbon->DEXCapMod += item->HeroicDex;
		newbon->AGICapMod += item->HeroicAgi;
		newbon->INTCapMod += item->HeroicInt;
		newbon->WISCapMod += item->HeroicWis;
		newbon->CHACapMod += item->HeroicCha;
		newbon->MRCapMod += item->HeroicMR;
		newbon->CRCapMod += item->HeroicFR;
		newbon->FRCapMod += item->HeroicCR;
		newbon->PRCapMod += item->HeroicPR;
		newbon->DRCapMod += item->HeroicDR;
		newbon->CorrupCapMod += item->HeroicSVCorrup;

		newbon->HeroicSTR += item->HeroicStr;
		newbon->HeroicSTA += item->HeroicSta;
		newbon->HeroicDEX += item->HeroicDex;
		newbon->HeroicAGI += item->HeroicAgi;
		newbon->HeroicINT += item->HeroicInt;
		newbon->HeroicWIS += item->HeroicWis;
		newbon->HeroicCHA += item->HeroicCha;
		newbon->HeroicMR += item->HeroicMR;
		newbon->HeroicFR += item->HeroicFR;
		newbon->HeroicCR += item->HeroicCR;
		newbon->HeroicPR += item->HeroicPR;
		newbon->HeroicDR += item->HeroicDR;
		newbon->HeroicCorrup += item->HeroicSVCorrup;

	}
	else
	{
		int lvl = GetLevel();

		newbon->AC += CalcRecommendedLevelBonus( lvl, rec_level, item->AC );
		newbon->HP += CalcRecommendedLevelBonus( lvl, rec_level, item->HP );
		newbon->Mana += CalcRecommendedLevelBonus( lvl, rec_level, item->Mana );
		newbon->Endurance += CalcRecommendedLevelBonus( lvl, rec_level, item->Endur );
		newbon->ATK += CalcRecommendedLevelBonus( lvl, rec_level, item->Attack );
		newbon->STR += CalcRecommendedLevelBonus( lvl, rec_level, (item->AStr + item->HeroicStr) );
		newbon->STA += CalcRecommendedLevelBonus( lvl, rec_level, (item->ASta + item->HeroicSta) );
		newbon->DEX += CalcRecommendedLevelBonus( lvl, rec_level, (item->ADex + item->HeroicDex) );
		newbon->AGI += CalcRecommendedLevelBonus( lvl, rec_level, (item->AAgi + item->HeroicAgi) );
		newbon->INT += CalcRecommendedLevelBonus( lvl, rec_level, (item->AInt + item->HeroicInt) );
		newbon->WIS += CalcRecommendedLevelBonus( lvl, rec_level, (item->AWis + item->HeroicWis) );
		newbon->CHA += CalcRecommendedLevelBonus( lvl, rec_level, (item->ACha + item->HeroicCha) );

		newbon->MR += CalcRecommendedLevelBonus( lvl, rec_level, (item->MR + item->HeroicMR) );
		newbon->FR += CalcRecommendedLevelBonus( lvl, rec_level, (item->FR + item->HeroicFR) );
		newbon->CR += CalcRecommendedLevelBonus( lvl, rec_level, (item->CR + item->HeroicCR) );
		newbon->PR += CalcRecommendedLevelBonus( lvl, rec_level, (item->PR + item->HeroicPR) );
		newbon->DR += CalcRecommendedLevelBonus( lvl, rec_level, (item->DR + item->HeroicDR) );
		newbon->Corrup += CalcRecommendedLevelBonus( lvl, rec_level, (item->SVCorruption + item->HeroicSVCorrup) );

		newbon->STRCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicStr );
		newbon->STACapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicSta );
		newbon->DEXCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicDex );
		newbon->AGICapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicAgi );
		newbon->INTCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicInt );
		newbon->WISCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicWis );
		newbon->CHACapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicCha );
		newbon->MRCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicMR );
		newbon->CRCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicFR );
		newbon->FRCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicCR );
		newbon->PRCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicPR );
		newbon->DRCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicDR );
		newbon->CorrupCapMod += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicSVCorrup );

		newbon->HeroicSTR += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicStr );
		newbon->HeroicSTA += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicSta );
		newbon->HeroicDEX += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicDex );
		newbon->HeroicAGI += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicAgi );
		newbon->HeroicINT += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicInt );
		newbon->HeroicWIS += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicWis );
		newbon->HeroicCHA += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicCha );
		newbon->HeroicMR += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicMR );
		newbon->HeroicFR += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicFR );
		newbon->HeroicCR += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicCR );
		newbon->HeroicPR += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicPR );
		newbon->HeroicDR += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicDR );
		newbon->HeroicCorrup += CalcRecommendedLevelBonus( lvl, rec_level, item->HeroicSVCorrup );
	}

	//FatherNitwit: New style haste, shields, and regens
	if(newbon->haste < (int32)item->Haste) {
		newbon->haste = item->Haste;
	}
	if(item->Regen > 0)
		newbon->HPRegen += item->Regen;

	if(item->ManaRegen > 0)
		newbon->ManaRegen += item->ManaRegen;

	if(item->EnduranceRegen > 0)
		newbon->EnduranceRegen += item->EnduranceRegen;

	if(item->DamageShield > 0) {
		if((newbon->DamageShield + item->DamageShield) > RuleI(Character, ItemDamageShieldCap))
			newbon->DamageShield = RuleI(Character, ItemDamageShieldCap);
		else
			newbon->DamageShield += item->DamageShield;
	}
	if(item->SpellShield > 0) {
		if((newbon->SpellShield + item->SpellShield) > RuleI(Character, ItemSpellShieldingCap))
			newbon->SpellShield = RuleI(Character, ItemSpellShieldingCap);
		else
			newbon->SpellShield += item->SpellShield;
	}
	if(item->Shielding > 0) {
		if((newbon->MeleeMitigation + item->Shielding) > RuleI(Character, ItemShieldingCap))
			newbon->MeleeMitigation = RuleI(Character, ItemShieldingCap);
		else
			newbon->MeleeMitigation += item->Shielding;
	}
	if(item->StunResist > 0) {
		if((newbon->StunResist + item->StunResist) > RuleI(Character, ItemStunResistCap))
			newbon->StunResist = RuleI(Character, ItemStunResistCap);
		else
			newbon->StunResist += item->StunResist;
	}
	if(item->StrikeThrough > 0) {
		if((newbon->StrikeThrough + item->StrikeThrough) > RuleI(Character, ItemStrikethroughCap))
			newbon->StrikeThrough = RuleI(Character, ItemStrikethroughCap);
		else
			newbon->StrikeThrough += item->StrikeThrough;
	}
	if(item->Avoidance > 0) {
		if((newbon->AvoidMeleeChance + item->Avoidance) > RuleI(Character, ItemAvoidanceCap))
			newbon->AvoidMeleeChance = RuleI(Character, ItemAvoidanceCap);
		else
			newbon->AvoidMeleeChance += item->Avoidance;
	}
	if(item->Accuracy > 0) {
		if((newbon->HitChance + item->Accuracy) > RuleI(Character, ItemAccuracyCap))
			newbon->HitChance = RuleI(Character, ItemAccuracyCap);
		else
			newbon->HitChance += item->Accuracy;
	}
	if(item->CombatEffects > 0) {
		if((newbon->ProcChance + item->CombatEffects) > RuleI(Character, ItemCombatEffectsCap))
			newbon->ProcChance = RuleI(Character, ItemCombatEffectsCap);
		else
			newbon->ProcChance += item->CombatEffects;
	}
	if(item->DotShielding > 0) {
		if((newbon->DoTShielding + item->DotShielding) > RuleI(Character, ItemDoTShieldingCap))
			newbon->DoTShielding = RuleI(Character, ItemDoTShieldingCap);
		else
			newbon->DoTShielding += item->DotShielding;
	}

	if(item->HealAmt > 0) {
		if((newbon->HealAmt + item->HealAmt) > RuleI(Character, ItemHealAmtCap))
			newbon->HealAmt = RuleI(Character, ItemHealAmtCap);
		else
			newbon->HealAmt += item->HealAmt;
	}
	if(item->SpellDmg > 0) {
		if((newbon->SpellDmg + item->SpellDmg) > RuleI(Character, ItemSpellDmgCap))
			newbon->SpellDmg = RuleI(Character, ItemSpellDmgCap);
		else
			newbon->SpellDmg += item->SpellDmg;
	}
	if(item->Clairvoyance > 0) {
		if((newbon->Clairvoyance + item->Clairvoyance) > RuleI(Character, ItemClairvoyanceCap))
			newbon->Clairvoyance = RuleI(Character, ItemClairvoyanceCap);
		else
			newbon->Clairvoyance += item->Clairvoyance;
	}

	if(item->DSMitigation > 0) {
		if((newbon->DSMitigation + item->DSMitigation) > RuleI(Character, ItemDSMitigationCap))
			newbon->DSMitigation = RuleI(Character, ItemDSMitigationCap);
		else
			newbon->DSMitigation += item->DSMitigation;
	}
	if (item->Worn.Effect > 0 && item->Worn.Type == EQ::item::ItemEffectWorn) {// latent effects
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, item->Worn.Type);
	}

	if (item->Focus.Effect>0 && (item->Focus.Type == EQ::item::ItemEffectFocus)) { // focus effects
		ApplySpellsBonuses(item->Focus.Effect, item->Focus.Level, newbon, 0);
	}

	switch(item->BardType)
	{
	case EQ::item::ItemTypeAllInstrumentTypes: // (e.g. Singing Short Sword)
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case EQ::item::ItemTypeSinging:
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			break;
		}
	case EQ::item::ItemTypeWindInstrument:
		{
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case EQ::item::ItemTypeStringedInstrument:
		{
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			break;
		}
	case EQ::item::ItemTypeBrassInstrument:
		{
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			break;
		}
	case EQ::item::ItemTypePercussionInstrument:
		{
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			break;
		}
	}

	if (item->SkillModValue != 0 && item->SkillModType <= EQ::skills::HIGHEST_SKILL) {
		if ((item->SkillModValue > 0 && newbon->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && newbon->skillmod[item->SkillModType] > item->SkillModValue))
		{
			newbon->skillmod[item->SkillModType] = item->SkillModValue;
		}
	}

	if (item->ExtraDmgSkill != 0 && item->ExtraDmgSkill <= EQ::skills::HIGHEST_SKILL) {
		if((newbon->SkillDamageAmount[item->ExtraDmgSkill] + item->ExtraDmgAmt) > RuleI(Character, ItemExtraDmgCap))
			newbon->SkillDamageAmount[item->ExtraDmgSkill] = RuleI(Character, ItemExtraDmgCap);
		else
			newbon->SkillDamageAmount[item->ExtraDmgSkill] += item->ExtraDmgAmt;
	}

	if (!isAug)
	{
		for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++)
			AddItemBonuses(inst->GetAugment(i),newbon,true, false, rec_level);
	}

}

int Bot::CalcRecommendedLevelBonus(uint8 level, uint8 reclevel, int basestat)
{
	if( (reclevel > 0) && (level < reclevel) )
	{
		int32 statmod = (level * 10000 / reclevel) * basestat;

		if( statmod < 0 )
		{
			statmod -= 5000;
			return (statmod/10000);
		}
		else
		{
			statmod += 5000;
			return (statmod/10000);
		}
	}

	return 0;
}

// This method is intended to call all necessary methods to do all bot stat calculations, including spell buffs, equipment, AA bonsues, etc.
void Bot::CalcBotStats(bool showtext) {
	if(!GetBotOwner())
		return;

	if(showtext) {
		GetBotOwner()->Message(Chat::Yellow, "Updating %s...", GetCleanName());
	}

	// this code is annoying since many classes change their name and illusions change the race id
	/*if(!IsValidRaceClassCombo()) {
		GetBotOwner()->Message(Chat::Yellow, "A %s - %s bot was detected. Is this Race/Class combination allowed?.", GetRaceIDName(GetRace()), GetClassIDName(GetClass(), GetLevel()));
		GetBotOwner()->Message(Chat::Yellow, "Previous Bots Code releases did not check Race/Class combinations during create.");
		GetBotOwner()->Message(Chat::Yellow, "Unless you are experiencing heavy lag, you should delete and remake this bot.");
	}*/

	if(GetBotOwner()->GetLevel() != GetLevel())
		SetLevel(GetBotOwner()->GetLevel());

	for (int sindex = 0; sindex <= EQ::skills::HIGHEST_SKILL; ++sindex) {
		skills[sindex] = content_db.GetSkillCap(GetClass(), (EQ::skills::SkillType)sindex, GetLevel());
	}

	taunt_timer.Start(1000);

	if (GetClass() == MONK && GetLevel() >= 10) {
		monkattack_timer.Start(1000);
	}

	LoadAAs();
	GenerateSpecialAttacks();

	if(showtext) {
		GetBotOwner()->Message(Chat::Yellow, "Base stats:");
		GetBotOwner()->Message(Chat::Yellow, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), base_hp, AC, max_mana, STR, STA, DEX, AGI, INT, WIS, CHA);
		GetBotOwner()->Message(Chat::Yellow, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",MR,PR,FR,CR,DR,Corrup);
		// Test Code
		if(GetClass() == BARD)
			GetBotOwner()->Message(Chat::Yellow, "Bard Skills-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i",
			GetSkill(EQ::skills::SkillBrassInstruments), GetSkill(EQ::skills::SkillPercussionInstruments), GetSkill(EQ::skills::SkillSinging), GetSkill(EQ::skills::SkillStringedInstruments), GetSkill(EQ::skills::SkillWindInstruments));
	}

	//if(Save())
	//	GetBotOwner()->CastToClient()->Message(Chat::White, "%s saved.", GetCleanName());
	//else
	//	GetBotOwner()->CastToClient()->Message(Chat::White, "%s save failed!", GetCleanName());

	CalcBonuses();

	if(showtext) {
		GetBotOwner()->Message(Chat::Yellow, "%s has been updated.", GetCleanName());
		GetBotOwner()->Message(Chat::Yellow, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), max_hp, GetAC(), max_mana, GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA());
		GetBotOwner()->Message(Chat::Yellow, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",GetMR(),GetPR(),GetFR(),GetCR(),GetDR(),GetCorrup());
		// Test Code
		if(GetClass() == BARD) {
			GetBotOwner()->Message(Chat::Yellow, "Bard Skills-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i",
				GetSkill(EQ::skills::SkillBrassInstruments) + GetBrassMod(),
				GetSkill(EQ::skills::SkillPercussionInstruments) + GetPercMod(),
				GetSkill(EQ::skills::SkillSinging) + GetSingMod(),
				GetSkill(EQ::skills::SkillStringedInstruments) + GetStringMod(),
				GetSkill(EQ::skills::SkillWindInstruments) + GetWindMod());
			GetBotOwner()->Message(Chat::Yellow, "Bard Skill Mods-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i", GetBrassMod(), GetPercMod(), GetSingMod(), GetStringMod(), GetWindMod());
		}
	}
}

bool Bot::CheckLoreConflict(const EQ::ItemData* item) {
	if (!item || !(item->LoreFlag))
		return false;

	if (item->LoreGroup == -1)	// Standard lore items; look everywhere except the shared bank, return the result
		return (m_inv.HasItem(item->ID, 0, invWhereWorn) != INVALID_INDEX);

	//If the item has a lore group, we check for other items with the same group and return the result
	return (m_inv.HasItemByLoreGroup(item->LoreGroup, invWhereWorn) != INVALID_INDEX);
}

bool EntityList::Bot_AICheckCloseBeneficialSpells(Bot* caster, uint8 iChance, float iRange, uint32 iSpellTypes) {
	if((iSpellTypes & SPELL_TYPES_DETRIMENTAL) != 0) {
		LogError("[EntityList::Bot_AICheckCloseBeneficialSpells] detrimental spells requested");
		return false;
	}

	if(!caster || !caster->AI_HasSpells())
		return false;

	if (iChance < 100) {
		uint8 tmp = zone->random.Int(1, 100);
		if (tmp > iChance)
			return false;
	}

	uint8 botCasterClass = caster->GetClass();

	if( iSpellTypes == SpellType_Heal )	{
		if( botCasterClass == CLERIC || botCasterClass == DRUID || botCasterClass == SHAMAN) {
			if(caster->HasGroup()) {
				Group *g = caster->GetGroup();
				if(g) {
					for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
						if(g->members[i] && !g->members[i]->qglobal) {
							if(g->members[i]->IsClient() && g->members[i]->GetHPRatio() < 90) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if((g->members[i]->GetClass() == WARRIOR || g->members[i]->GetClass() == PALADIN || g->members[i]->GetClass() == SHADOWKNIGHT) && g->members[i]->GetHPRatio() < 95) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if(g->members[i]->GetClass() == ENCHANTER && g->members[i]->GetHPRatio() < 80) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if(g->members[i]->GetHPRatio() < 70) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							}
						}

						if(g->members[i] && !g->members[i]->qglobal && g->members[i]->HasPet() && g->members[i]->GetPet()->GetHPRatio() < 50) {
							if(g->members[i]->GetPet()->GetOwner() != caster && caster->IsEngaged() && g->members[i]->IsCasting() && g->members[i]->GetClass() != ENCHANTER )
								continue;

							if(caster->AICastSpell(g->members[i]->GetPet(), 100, SpellType_Heal))
								return true;
						}
					}
				}
			}
		}

		if( botCasterClass == PALADIN || botCasterClass == BEASTLORD || botCasterClass == RANGER) {
			if(caster->HasGroup()) {
				Group *g = caster->GetGroup();
				float hpRatioToHeal = 25.0f;
				switch(caster->GetBotStance()) {
				case EQ::constants::stanceReactive:
				case EQ::constants::stanceBalanced:
					hpRatioToHeal = 50.0f;
					break;
				case EQ::constants::stanceBurn:
				case EQ::constants::stanceBurnAE:
					hpRatioToHeal = 20.0f;
					break;
				case EQ::constants::stanceAggressive:
				case EQ::constants::stanceEfficient:
				default:
					hpRatioToHeal = 25.0f;
					break;
				}

				if(g) {
					for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
						if(g->members[i] && !g->members[i]->qglobal) {
							if(g->members[i]->IsClient() && g->members[i]->GetHPRatio() < hpRatioToHeal) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if((g->members[i]->GetClass() == WARRIOR || g->members[i]->GetClass() == PALADIN || g->members[i]->GetClass() == SHADOWKNIGHT) && g->members[i]->GetHPRatio() < hpRatioToHeal) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if(g->members[i]->GetClass() == ENCHANTER && g->members[i]->GetHPRatio() < hpRatioToHeal) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if(g->members[i]->GetHPRatio() < hpRatioToHeal/2) {
								if(caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							}
						}

						if(g->members[i] && !g->members[i]->qglobal && g->members[i]->HasPet() && g->members[i]->GetPet()->GetHPRatio() < 25) {
							if(g->members[i]->GetPet()->GetOwner() != caster && caster->IsEngaged() && g->members[i]->IsCasting() && g->members[i]->GetClass() != ENCHANTER )
								continue;

							if(caster->AICastSpell(g->members[i]->GetPet(), 100, SpellType_Heal))
								return true;
						}
					}
				}
			}
		}
	}

	if (iSpellTypes == SpellType_Buff) {
		uint8 chanceToCast = caster->IsEngaged() ? caster->GetChanceToCastBySpellType(SpellType_Buff) : 100;
		if (botCasterClass == BARD) {
			if(caster->AICastSpell(caster, chanceToCast, SpellType_Buff))
				return true;
			else
				return false;
		}

		if (caster->HasGroup()) {
			Group *g = caster->GetGroup();
			if(g) {
				for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if(g->members[i]) {
						if(caster->AICastSpell(g->members[i], chanceToCast, SpellType_Buff) || caster->AICastSpell(g->members[i]->GetPet(), chanceToCast, SpellType_Buff))
							return true;
					}
				}
			}
		}
	}

	if( iSpellTypes == SpellType_Cure) {
		if(caster->HasGroup()) {
			Group *g = caster->GetGroup();
			if(g) {
				for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if(g->members[i] && caster->GetNeedsCured(g->members[i])) {
						if(caster->AICastSpell(g->members[i], caster->GetChanceToCastBySpellType(SpellType_Cure), SpellType_Cure))
							return true;
						else if(botCasterClass == BARD)
							return false;
					}

					if(g->members[i] && g->members[i]->GetPet() && caster->GetNeedsCured(g->members[i]->GetPet())) {
						if(caster->AICastSpell(g->members[i]->GetPet(), (int)caster->GetChanceToCastBySpellType(SpellType_Cure)/4, SpellType_Cure))
							return true;
					}
				}
			}
		}
	}

	if (iSpellTypes == SpellType_HateRedux) {
		if (!caster->IsEngaged())
			return false;

		if (caster->HasGroup()) {
			Group *g = caster->GetGroup();
			if (g) {
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (g->members[i] && caster->GetNeedsHateRedux(g->members[i])) {
						if (caster->AICastSpell(g->members[i], caster->GetChanceToCastBySpellType(SpellType_HateRedux), SpellType_HateRedux))
							return true;
					}
				}
			}
		}
	}

	if (iSpellTypes == SpellType_PreCombatBuff) {
		if (botCasterClass == BARD || caster->IsEngaged())
			return false;

		if (caster->HasGroup()) {
			Group *g = caster->GetGroup();
			if (g) {
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (g->members[i]) {
						if (caster->AICastSpell(g->members[i], iChance, SpellType_PreCombatBuff) || caster->AICastSpell(g->members[i]->GetPet(), iChance, SpellType_PreCombatBuff))
							return true;
					}
				}
			}
		}
	}

	if (iSpellTypes == SpellType_InCombatBuff) {
		if (botCasterClass == BARD) {
			if (caster->AICastSpell(caster, iChance, SpellType_InCombatBuff)) {
				return true;
			}
			else {
				return false;
			}
		}

		if (caster->HasGroup()) {
			Group* g = caster->GetGroup();
			if (g) {
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (g->members[i]) {
						if (caster->AICastSpell(g->members[i], iChance, SpellType_InCombatBuff) || caster->AICastSpell(g->members[i]->GetPet(), iChance, SpellType_InCombatBuff)) {
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}


Mob* EntityList::GetMobByBotID(uint32 botID) {
	Mob* Result = nullptr;
	if(botID > 0) {
		auto it = mob_list.begin();
		for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
			if(!it->second)
				continue;

			if(it->second->IsBot() && it->second->CastToBot()->GetBotID() == botID) {
				Result = it->second;
				break;
			}
		}
	}
	return Result;
}

Bot* EntityList::GetBotByBotID(uint32 botID) {
	Bot* Result = nullptr;
	if(botID > 0) {
		for(std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if(tempBot && tempBot->GetBotID() == botID) {
				Result = tempBot;
				break;
			}
		}
	}
	return Result;
}

Bot* EntityList::GetBotByBotName(std::string botName) {
	Bot* Result = nullptr;
	if(!botName.empty()) {
		for(std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if(tempBot && std::string(tempBot->GetName()) == botName) {
				Result = tempBot;
				break;
			}
		}
	}
	return Result;
}

Client* EntityList::GetBotOwnerByBotEntityID(uint32 entity_id) {
	Client* c = nullptr;

	if (entity_id) {
		for (const auto& b : bot_list) {
			if (b && b->GetID() == entity_id) {
				c = b->GetBotOwner()->CastToClient();
				break;
			}
		}
	}

	return c;
}

Client* EntityList::GetBotOwnerByBotID(const uint32 bot_id)  {
	Client* c = nullptr;

	if (bot_id) {
		const auto owner_id = database.botdb.GetOwnerID(bot_id);
		if (owner_id) {
			c = GetClientByCharID(owner_id);
		}
	}

	return c;
}

void EntityList::AddBot(Bot *new_bot, bool send_spawn_packet, bool dont_queue) {
	if (new_bot) {
		new_bot->SetID(GetFreeID());
		bot_list.push_back(new_bot);
		mob_list.insert(std::pair<uint16, Mob*>(new_bot->GetID(), new_bot));
		parse->EventBot(EVENT_SPAWN, new_bot, nullptr, "", 0);
		new_bot->SetSpawned();
		if (send_spawn_packet) {
			if (dont_queue) {
				EQApplicationPacket* outapp = new EQApplicationPacket();
				new_bot->CreateSpawnPacket(outapp);
				outapp->priority = 6;
				QueueClients(new_bot, outapp, true);
				safe_delete(outapp);
			} else {
				NewSpawn_Struct* ns = new NewSpawn_Struct;
				memset(ns, 0, sizeof(NewSpawn_Struct));
				new_bot->FillSpawnStruct(ns, new_bot);
				AddToSpawnQueue(new_bot->GetID(), &ns);
				safe_delete(ns);
			}
		}

		new_bot->DispatchZoneControllerEvent(EVENT_SPAWN_ZONE, new_bot, "", 0, nullptr);
	}
}

std::list<Bot*> EntityList::GetBotsByBotOwnerCharacterID(uint32 botOwnerCharacterID) {
	std::list<Bot*> Result;
	if(botOwnerCharacterID > 0) {
		for(std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if(tempBot && tempBot->GetBotOwnerCharacterID() == botOwnerCharacterID)
				Result.push_back(tempBot);
		}
	}
	return Result;
}

bool EntityList::RemoveBot(uint16 entityID) {
	bool Result = false;
	if(entityID > 0) {
		for(std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if(tempBot && tempBot->GetID() == entityID) {
				bot_list.erase(botListItr);
				Result = true;
				break;
			}
		}
	}
	return Result;
}

void EntityList::ShowSpawnWindow(Client* client, int Distance, bool NamedOnly) {
	const char *WindowTitle = "Bot Tracking Window";
	std::string WindowText;
	int LastCon = -1;
	int CurrentCon = 0;
	Mob* curMob = nullptr;
	uint32 array_counter = 0;
	auto it = mob_list.begin();
	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
	curMob = it->second;
		if (curMob && DistanceNoZ(curMob->GetPosition(), client->GetPosition()) <= Distance) {
			if(curMob->IsTrackable()) {
				Mob* cur_entity = curMob;
				int Extras = (cur_entity->IsBot() || cur_entity->IsPet() || cur_entity->IsFamiliar() || cur_entity->IsClient());
				const char *const MyArray[] = {
					"a_","an_","Innkeep_","Barkeep_",
					"Guard_","Merchant_","Lieutenant_",
					"Banker_","Centaur_","Aviak_","Baker_",
					"Sir_","Armorer_","Deathfist_","Deputy_",
					"Sentry_","Sentinel_","Leatherfoot_",
					"Corporal_","goblin_","Bouncer_","Captain_",
					"orc_","fire_","inferno_","young_","cinder_",
					"flame_","gnomish_","CWG_","sonic_","greater_",
					"ice_","dry_","Priest_","dark-boned_",
					"Tentacle_","Basher_","Dar_","Greenblood_",
					"clockwork_","guide_","rogue_","minotaur_",
					"brownie_","Teir'","dark_","tormented_",
					"mortuary_","lesser_","giant_","infected_",
					"wharf_","Apprentice_","Scout_","Recruit_",
					"Spiritist_","Pit_","Royal_","scalebone_",
					"carrion_","Crusader_","Trooper_","hunter_",
					"decaying_","iksar_","klok_","templar_","lord_",
					"froglok_","war_","large_","charbone_","icebone_",
					"Vicar_","Cavalier_","Heretic_","Reaver_","venomous_",
					"Sheildbearer_","pond_","mountain_","plaguebone_","Brother_",
					"great_","strathbone_","briarweb_","strathbone_","skeletal_",
					"minion_","spectral_","myconid_","spurbone_","sabretooth_",
					"Tin_","Iron_","Erollisi_","Petrifier_","Burynai_",
					"undead_","decayed_","You_","smoldering_","gyrating_",
					"lumpy_","Marshal_","Sheriff_","Chief_","Risen_",
					"lascar_","tribal_","fungi_","Xi_","Legionnaire_",
					"Centurion_","Zun_","Diabo_","Scribe_","Defender_","Capt_",
					"blazing_","Solusek_","imp_","hexbone_","elementalbone_",
					"stone_","lava_","_",""
				};
				unsigned int MyArraySize;
				for ( MyArraySize = 0; true; MyArraySize++) {
					if (!(*(MyArray[MyArraySize])))
						break;
				};
				if (NamedOnly) {
					bool ContinueFlag = false;
					const char *CurEntityName = cur_entity->GetName();
					for (int Index = 0; Index < MyArraySize; Index++) {
						if (!strncasecmp(CurEntityName, MyArray[Index], strlen(MyArray[Index])) || (Extras)) {
							ContinueFlag = true;
							break;
						};
					};
					if (ContinueFlag)
						continue;
				};

				CurrentCon = client->GetLevelCon(cur_entity->GetLevel());
				if(CurrentCon != LastCon) {
					if(LastCon != -1)
						WindowText += "</c>";

					LastCon = CurrentCon;
					switch(CurrentCon) {
						case CON_GREEN: {
							WindowText += "<c \"#00FF00\">";
							break;
						}
						case CON_LIGHTBLUE: {
							WindowText += "<c \"#8080FF\">";
							break;
						}
						case CON_BLUE: {
							WindowText += "<c \"#2020FF\">";
							break;
						}
						case CON_YELLOW: {
							WindowText += "<c \"#FFFF00\">";
							break;
						}
						case CON_RED: {
							WindowText += "<c \"#FF0000\">";
							break;
						}
						default: {
							WindowText += "<c \"#FFFFFF\">";
							break;
						}
					}
				}
				WindowText += cur_entity->GetCleanName();
				WindowText += "<br>";
				if(strlen(WindowText.c_str()) > 4000) {
					WindowText += "</c><br><br>List truncated... too many mobs to display";
					break;
				}
			}
		}
	}
	WindowText += "</c>";
	client->SendPopupToClient(WindowTitle, WindowText.c_str());
	return;
}

/**
 * @param close_mobs
 * @param scanning_mob
 */
void EntityList::ScanCloseClientMobs(std::unordered_map<uint16, Mob*>& close_mobs, Mob* scanning_mob)
{
	float scan_range = RuleI(Range, MobCloseScanDistance) * RuleI(Range, MobCloseScanDistance);

	close_mobs.clear();

	for (auto& e : mob_list) {
		auto mob = e.second;

		if (!mob->IsClient()) {
			continue;
		}

		if (mob->GetID() <= 0) {
			continue;
		}

		float distance = DistanceSquared(scanning_mob->GetPosition(), mob->GetPosition());
		if (distance <= scan_range) {
			close_mobs.insert(std::pair<uint16, Mob*>(mob->GetID(), mob));
		}
		else if (mob->GetAggroRange() >= scan_range) {
			close_mobs.insert(std::pair<uint16, Mob*>(mob->GetID(), mob));
		}
	}

	LogAIScanCloseDetail("Close Client Mob List Size [{}] for mob [{}]", close_mobs.size(), scanning_mob->GetCleanName());
}

uint8 Bot::GetNumberNeedingHealedInGroup(uint8 hpr, bool includePets) {
	uint8 needHealed = 0;
	Group *g = nullptr;
	if(HasGroup()) {
		g = GetGroup();
		if(g) {
			for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if(g->members[i] && !g->members[i]->qglobal) {
					if(g->members[i]->GetHPRatio() <= hpr)
						needHealed++;

					if(includePets) {
						if(g->members[i]->GetPet() && g->members[i]->GetPet()->GetHPRatio() <= hpr)
							needHealed++;
					}
				}
			}
		}
	}
	return needHealed;
}

int Bot::GetRawACNoShield(int &shield_ac) {
	int ac = itembonuses.AC + spellbonuses.AC;
	shield_ac = 0;
	EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotSecondary);
	if(inst) {
		if (inst->GetItem()->ItemType == EQ::item::ItemTypeShield) {
			ac -= inst->GetItem()->AC;
			shield_ac = inst->GetItem()->AC;
			for (uint8 i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if(inst->GetAugment(i)) {
					ac -= inst->GetAugment(i)->GetItem()->AC;
					shield_ac += inst->GetAugment(i)->GetItem()->AC;
				}
			}
		}
	}
	return ac;
}

uint32 Bot::CalcCurrentWeight() {
	const EQ::ItemData* TempItem = nullptr;
	EQ::ItemInstance* inst = nullptr;
	uint32 Total = 0;
	for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; ++i) {
		inst = GetBotItem(i);
		if(inst) {
			TempItem = inst->GetItem();
			if (TempItem)
				Total += TempItem->Weight;
		}
	}

	float Packrat = ((float)spellbonuses.Packrat + (float)aabonuses.Packrat);
	if (Packrat > 0)
		Total = (uint32)((float)Total * (1.0f - ((Packrat * 1.0f) / 100.0f)));

	return Total;
}

int Bot::GroupLeadershipAAHealthEnhancement() {
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthEnhancement)) {
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}
	return 0;
}

int Bot::GroupLeadershipAAManaEnhancement() {
	Group *g = GetGroup();
	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAManaEnhancement)) {
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}
	return 0;
}

int Bot::GroupLeadershipAAHealthRegeneration() {
	Group *g = GetGroup();
	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthRegeneration)) {
		case 0:
			return 0;
		case 1:
			return 4;
		case 2:
			return 6;
		case 3:
			return 8;
	}

	return 0;
}

int Bot::GroupLeadershipAAOffenseEnhancement() {
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAOffenseEnhancement)) {
		case 0:
			return 0;
		case 1:
			return 10;
		case 2:
			return 19;
		case 3:
			return 28;
		case 4:
			return 34;
		case 5:
			return 40;
	}
	return 0;
}

bool Bot::GetNeedsCured(Mob *tar) {
	bool needCured = false;
	if(tar) {
		if(tar->FindType(SE_PoisonCounter) || tar->FindType(SE_DiseaseCounter) || tar->FindType(SE_CurseCounter) || tar->FindType(SE_CorruptionCounter)) {
			uint32 buff_count = tar->GetMaxTotalSlots();
			int buffsWithCounters = 0;
			needCured = true;
			for (unsigned int j = 0; j < buff_count; j++) {
				if(tar->GetBuffs()[j].spellid != SPELL_UNKNOWN) {
					if(CalculateCounters(tar->GetBuffs()[j].spellid) > 0) {
						buffsWithCounters++;
						if(buffsWithCounters == 1 && (tar->GetBuffs()[j].ticsremaining < 2 || (int32)((tar->GetBuffs()[j].ticsremaining * 6) / tar->GetBuffs()[j].counters) < 2)) {
							needCured = false;
							break;
						}
					}
				}
			}
		}
	}
	return needCured;
}

bool Bot::GetNeedsHateRedux(Mob *tar) {
	// This really should be a scalar function based in class Mob that returns 'this' state..but, is inline with current Bot coding...
	// TODO: Good starting point..but, can be refined..
	// TODO: Still awaiting bot spell rework..
	if (!tar || !tar->IsEngaged() || !tar->HasTargetReflection() || !tar->GetTarget()->IsNPC())
		return false;

	//if (tar->IsClient()) {
	//	switch (tar->GetClass()) {
	//		// TODO: figure out affectable classes..
	//		// Might need flag to allow player to determine redux req...
	//	default:
	//		return false;
	//	}
	//}
	//else if (tar->IsBot()) {
	if (tar->IsBot()) {
		switch (tar->GetClass()) {
		case ROGUE:
			if (tar->CanFacestab() || tar->CastToBot()->m_evade_timer.Check(false))
				return false;
		case CLERIC:
		case DRUID:
		case SHAMAN:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		default:
			return false;
		}
	}

	return false;
}

bool Bot::HasOrMayGetAggro() {
	bool mayGetAggro = false;
	if(GetTarget() && GetTarget()->GetHateTop()) {
		Mob *topHate = GetTarget()->GetHateTop();
		if(topHate == this)
			mayGetAggro = true;
		else {
			uint32 myHateAmt = GetTarget()->GetHateAmount(this);
			uint32 topHateAmt = GetTarget()->GetHateAmount(topHate);

			if(myHateAmt > 0 && topHateAmt > 0 && (uint8)((myHateAmt / topHateAmt) * 100) > 90)
				mayGetAggro = true;
		}
	}
	return mayGetAggro;
}

void Bot::SetDefaultBotStance() {
	EQ::constants::StanceType defaultStance = EQ::constants::stanceBalanced;
	if (GetClass() == WARRIOR)
		defaultStance = EQ::constants::stanceAggressive;

	_baseBotStance = EQ::constants::stancePassive;
	_botStance = defaultStance;
}

void Bot::BotGroupSay(Mob *speaker, const char *msg, ...) {
	char buf[1000];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, 1000, msg, ap);
	va_end(ap);
	if(speaker->HasGroup()) {
		Group *g = speaker->GetGroup();
		if(g)
			g->GroupMessage(speaker->CastToMob(), 0, 100, buf);
	} else
		speaker->Say("%s", buf);
}

bool Bot::UseDiscipline(uint32 spell_id, uint32 target) {
	if(!IsValidSpell(spell_id)) {
		BotGroupSay(this, "Not a valid spell.");
		return false;
	}

	const SPDat_Spell_Struct &spell = spells[spell_id];
	uint8 level_to_use = spell.classes[GetClass() - 1];
	if(level_to_use == 255 || level_to_use > GetLevel()) {
		return false;
	}

	if(GetEndurance() > spell.endurance_cost)
		SetEndurance(GetEndurance() - spell.endurance_cost);
	else
		return false;

	if(spell.recast_time > 0) {
		if(CheckDisciplineRecastTimers(this, spells[spell_id].timer_id)) {
			if(spells[spell_id].timer_id > 0 && spells[spell_id].timer_id < MAX_DISCIPLINE_TIMERS)
				SetDisciplineRecastTimer(spells[spell_id].timer_id, spell.recast_time);
		} else {
			uint32 remaining_time = (GetDisciplineRemainingTime(this, spells[spell_id].timer_id) / 1000);
			GetOwner()->Message(
				Chat::White,
				fmt::format(
					"{} can use this discipline in {}.",
					GetCleanName(),
					Strings::SecondsToTime(remaining_time)
				).c_str()
			);
			return false;
		}
	}

	if(IsCasting())
		InterruptSpell();

	CastSpell(spell_id, target, EQ::spells::CastingSlot::Discipline);
	return true;
}

// new healrotation code
bool Bot::CreateHealRotation(uint32 interval_ms, bool fast_heals, bool adaptive_targeting, bool casting_override)
{
	if (IsHealRotationMember())
		return false;
	if (!IsHealRotationMemberClass(GetClass()))
		return false;

	m_member_of_heal_rotation = std::make_shared<HealRotation>(this, interval_ms, fast_heals, adaptive_targeting, casting_override);

	return IsHealRotationMember();
}

bool Bot::DestroyHealRotation()
{
	if (!IsHealRotationMember())
		return true;

	m_member_of_heal_rotation->ClearTargetPool();
	m_member_of_heal_rotation->ClearMemberPool();

	return !IsHealRotationMember();
}

bool Bot::JoinHealRotationMemberPool(std::shared_ptr<HealRotation>* heal_rotation)
{
	if (IsHealRotationMember())
		return false;
	if (!heal_rotation->use_count())
		return false;
	if (!(*heal_rotation))
		return false;
	if (!IsHealRotationMemberClass(GetClass()))
		return false;

	if (!(*heal_rotation)->AddMemberToPool(this))
		return false;

	m_member_of_heal_rotation = *heal_rotation;

	return true;
}

bool Bot::LeaveHealRotationMemberPool()
{
	if (!IsHealRotationMember()) {
		m_member_of_heal_rotation.reset();
		return true;
	}

	m_member_of_heal_rotation->RemoveMemberFromPool(this);
	m_member_of_heal_rotation.reset();

	return !IsHealRotationMember();
}

bool Bot::UseHealRotationFastHeals()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->FastHeals();
}

bool Bot::UseHealRotationAdaptiveTargeting()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->AdaptiveTargeting();
}

bool Bot::IsHealRotationActive()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->IsActive();
}

bool Bot::IsHealRotationReady()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->CastingReady();
}

bool Bot::IsHealRotationCaster()
{
	if (!IsHealRotationMember())
		return false;

	return (m_member_of_heal_rotation->CastingMember() == this);
}

bool Bot::HealRotationPokeTarget()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->PokeCastingTarget();
}

Mob* Bot::HealRotationTarget()
{
	if (!IsHealRotationMember())
		return nullptr;

	return m_member_of_heal_rotation->CastingTarget();
}

bool Bot::AdvanceHealRotation(bool use_interval)
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->AdvanceRotation(use_interval);
}

bool Bot::IsMyHealRotationSet()
{
	if (!IsHealRotationMember())
		return false;
	if (!m_member_of_heal_rotation->IsActive() && !m_member_of_heal_rotation->IsHOTActive())
		return false;
	if (!m_member_of_heal_rotation->CastingReady())
		return false;
	if (m_member_of_heal_rotation->CastingMember() != this)
		return false;
	if (m_member_of_heal_rotation->MemberIsCasting(this))
		return false;
	if (!m_member_of_heal_rotation->PokeCastingTarget())
		return false;

	return true;
}

bool Bot::AmICastingForHealRotation()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->MemberIsCasting(this);
}

void Bot::SetMyCastingForHealRotation(bool flag)
{
	if (!IsHealRotationMember())
		return;

	m_member_of_heal_rotation->SetMemberIsCasting(this, flag);
}

bool Bot::DyeArmor(int16 slot_id, uint32 rgb, bool all_flag, bool save_flag)
{
	if (all_flag) {
		if (slot_id != INVALID_INDEX)
			return false;

		for (uint8 i = EQ::textures::textureBegin; i < EQ::textures::weaponPrimary; ++i) {
			uint8 inv_slot = EQ::InventoryProfile::CalcSlotFromMaterial(i);
			EQ::ItemInstance* inst = m_inv.GetItem(inv_slot);
			if (!inst)
				continue;

			inst->SetColor(rgb);
			SendWearChange(i);
		}
	}
	else {
		uint8 mat_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id);
		if (mat_slot == EQ::textures::materialInvalid || mat_slot >= EQ::textures::weaponPrimary)
			return false;

		EQ::ItemInstance* inst = m_inv.GetItem(slot_id);
		if (!inst)
			return false;

		inst->SetColor(rgb);
		SendWearChange(mat_slot);
	}

	if (save_flag) {
		int16 save_slot = slot_id;
		if (all_flag)
			save_slot = -2;

		if (!database.botdb.SaveEquipmentColor(GetBotID(), save_slot, rgb)) {
			if (GetBotOwner() && GetBotOwner()->IsClient())
				GetBotOwner()->CastToClient()->Message(Chat::White, "%s", BotDatabase::fail::SaveEquipmentColor());
			return false;
		}
	}

	return true;
}

std::string Bot::CreateSayLink(Client* c, const char* message, const char* name)
{
	// TODO: review

	int saylink_size = strlen(message);
	char* escaped_string = new char[saylink_size * 2];

	database.DoEscapeString(escaped_string, message, saylink_size);

	uint32 saylink_id = database.LoadSaylinkID(escaped_string);
	safe_delete_array(escaped_string);

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemData);
	linker.SetProxyItemID(SAYLINK_ITEM_ID);
	linker.SetProxyAugment1ID(saylink_id);
	linker.SetProxyText(name);

	auto saylink = linker.GenerateLink();
	return saylink;
}

void Bot::StopMoving()
{
	//SetCombatJitterFlag(false);
	//m_combat_jitter_timer.Start(zone->random.Int(BOT_COMBAT_JITTER_INTERVAL_MIN, BOT_COMBAT_JITTER_INTERVAL_MAX));

	Mob::StopMoving();
}

void Bot::StopMoving(float new_heading)
{
	//SetCombatJitterFlag(false);
	//m_combat_jitter_timer.Start(zone->random.Int(BOT_COMBAT_JITTER_INTERVAL_MIN, BOT_COMBAT_JITTER_INTERVAL_MAX));

	Mob::StopMoving(new_heading);
}

void Bot::SpawnBotGroupByName(Client* c, std::string botgroup_name, uint32 leader_id)
{
	auto leader = Bot::LoadBot(leader_id);
	if (!leader) {
		c->Message(
			Chat::White,
			fmt::format(
				"Could not load bot-group leader for '{}'.",
				botgroup_name
			).c_str()
		);
		safe_delete(leader);
		return;
	}

	if (!leader->Spawn(c)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Could not spawn bot-group leader {} for '{}'.",
				leader->GetName(),
				botgroup_name
			).c_str()
		);
		safe_delete(leader);
		return;
	}

	auto* g = new Group(leader);

	entity_list.AddGroup(g);
	database.SetGroupID(leader->GetCleanName(), g->GetID(), leader->GetBotID());
	database.SetGroupLeaderName(g->GetID(), leader->GetCleanName());
	leader->SetFollowID(c->GetID());

	uint32 botgroup_id = 0;
	database.botdb.LoadBotGroupIDByBotGroupName(botgroup_name, botgroup_id);

	std::map<uint32, std::list<uint32>> member_list;
	if (!database.botdb.LoadBotGroup(botgroup_name, member_list)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to load bot-group '{}'.",
				botgroup_name
			).c_str()
		);
		return;
	}

	if (member_list.find(botgroup_id) == member_list.end() || member_list[botgroup_id].empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Could not locate member list for bot-group '{}'.",
				botgroup_name
			).c_str()
		);
		return;
	}

	member_list[botgroup_id].remove(0);
	member_list[botgroup_id].remove(leader->GetBotID());

	auto bot_spawn_limit = c->GetBotSpawnLimit();
	auto spawned_bot_count = 0;

	std::vector<int> bot_class_spawn_limits;
	std::vector<int> bot_class_spawned_count = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (uint8 class_id = WARRIOR; class_id <= BERSERKER; class_id++) {
		auto bot_class_limit = c->GetBotSpawnLimit(class_id);
		bot_class_spawn_limits.push_back(bot_class_limit);
	}

	for (const auto& member_iter : member_list[botgroup_id]) {
		auto member = Bot::LoadBot(member_iter);
		if (!member) {
			c->Message(
				Chat::White,
				fmt::format(
					"Could not load bot id {}.",
					member_iter
				).c_str()
			);
			safe_delete(member);
			return;
		}

		if (bot_spawn_limit >= 0 && spawned_bot_count >= bot_spawn_limit) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to spawn {} because you have a max of {} bot{} spawned.",
					member->GetCleanName(),
					bot_spawn_limit,
					bot_spawn_limit != 1 ? "s" : ""
				).c_str()
			);
			return;
		}

		auto spawned_bot_count_class = bot_class_spawned_count[member->GetClass() - 1];
		auto bot_spawn_limit_class = bot_class_spawn_limits[member->GetClass() - 1];

		if (bot_spawn_limit_class >= 0 && spawned_bot_count_class >= bot_spawn_limit_class) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to spawn {} because you have a max of {} {} bot{} spawned.",
					member->GetCleanName(),
					bot_spawn_limit_class,
					GetClassIDName(member->GetClass()),
					bot_spawn_limit_class != 1 ? "s" : ""
				).c_str()
			);
			continue;
		}

		if (!member->Spawn(c)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Could not spawn bot '{}' (ID {}).",
					member->GetName(),
					member_iter
				).c_str()
			);
			safe_delete(member);
			return;
		}

		spawned_bot_count++;
		bot_class_spawned_count[member->GetClass() - 1]++;

		Bot::AddBotToGroup(member, g);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully loaded bot-group {}.",
			botgroup_name
		).c_str()
	);
}

void Bot::Signal(int signal_id)
{
	const auto export_string = fmt::format("{}", signal_id);
	parse->EventBot(EVENT_SIGNAL, this, nullptr, export_string, 0);
}

void Bot::SendPayload(int payload_id, std::string payload_value)
{
	const auto export_string = fmt::format("{} {}", payload_id, payload_value);
	parse->EventBot(EVENT_PAYLOAD, this, nullptr, export_string, 0);
}

void Bot::OwnerMessage(std::string message)
{
	if (!GetBotOwner() || !GetBotOwner()->IsClient()) {
		return;
	}

	GetBotOwner()->Message(
		Chat::Tell,
		fmt::format(
			"{} tells you, '{}'",
			GetCleanName(),
			message
		).c_str()
	);
}

bool Bot::GetBotOwnerDataBuckets()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return false;
	}

	const auto query = fmt::format(
		"SELECT `key`, `value` FROM data_buckets WHERE `key` LIKE '{}-%'",
		Strings::Escape(bot_owner->GetBucketKey())
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	bot_owner_data_buckets.clear();

	if (!results.RowCount()) {
		return true;
	}

	for (auto row : results) {
		bot_owner_data_buckets.insert(std::pair<std::string,std::string>(row[0], row[1]));
	}

	return true;
}

bool Bot::GetBotDataBuckets()
{
	const auto query = fmt::format(
		"SELECT `key`, `value` FROM data_buckets WHERE `key` LIKE '{}-%'",
		Strings::Escape(GetBucketKey())
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	bot_data_buckets.clear();

	if (!results.RowCount()) {
		return true;
	}

	for (auto row : results) {
		bot_data_buckets.insert(std::pair<std::string,std::string>(row[0], row[1]));
	}

	return true;
}

bool Bot::CheckDataBucket(std::string bucket_name, std::string bucket_value, uint8 bucket_comparison)
{
	if (!bucket_name.empty() && !bucket_value.empty()) {
		auto full_name = fmt::format(
			"{}-{}",
			GetBucketKey(),
			bucket_name
		);

		auto player_value = bot_data_buckets[full_name];
		if (player_value.empty() && GetBotOwner()) {
			full_name = fmt::format(
				"{}-{}",
				GetBotOwner()->GetBucketKey(),
				bucket_name
			);

			player_value = bot_owner_data_buckets[full_name];
			if (player_value.empty()) {
				return false;
			}
		}

		if (zone->CheckDataBucket(bucket_comparison, bucket_value, player_value)) {
			return true;
		}
	}

	return false;
}

int Bot::GetExpansionBitmask()
{
	if (m_expansion_bitmask >= 0) {
		return m_expansion_bitmask;
	}

	return RuleI(Bots, BotExpansionSettings);
}

void Bot::SetExpansionBitmask(int expansion_bitmask, bool save)
{
	m_expansion_bitmask = expansion_bitmask;

	if (save) {
		if (!database.botdb.SaveExpansionBitmask(GetBotID(), expansion_bitmask)) {
			if (GetBotOwner() && GetBotOwner()->IsClient()) {
				GetBotOwner()->CastToClient()->Message(
					Chat::White,
					fmt::format(
						"Failed to save expansion bitmask for {}.",
						GetCleanName()
					).c_str()
				);
			}
		}
	}

	LoadAAs();
}

void Bot::SetBotEnforceSpellSetting(bool enforce_spell_settings, bool save)
{
	m_enforce_spell_settings = enforce_spell_settings;

	if (save) {
		if (!database.botdb.SaveEnforceSpellSetting(GetBotID(), enforce_spell_settings)) {
			if (GetBotOwner() && GetBotOwner()->IsClient()) {
				GetBotOwner()->CastToClient()->Message(
					Chat::White,
					fmt::format(
						"Failed to save enforce spell settings for {}.",
						GetCleanName()
					).c_str()
				);
			}
		}
	}
	LoadBotSpellSettings();
	AI_AddBotSpells(GetBotSpellID());
}

bool Bot::AddBotSpellSetting(uint16 spell_id, BotSpellSetting* bs)
{
	if (!IsValidSpell(spell_id) || !bs) {
		return false;
	}

	auto obs = GetBotSpellSetting(spell_id);
	if (obs) {
		return false;
	}

	auto s = BotSpellSettingsRepository::NewEntity();

	s.spell_id = spell_id;
	s.bot_id = GetBotID();

	s.priority = bs->priority;
	s.min_hp = bs->min_hp;
	s.max_hp = bs->max_hp;
	s.is_enabled = bs->is_enabled;

	const auto& nbs = BotSpellSettingsRepository::InsertOne(content_db, s);
	if (!nbs.id) {
		return false;
	}

	LoadBotSpellSettings();
	return true;
}

bool Bot::DeleteBotSpellSetting(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	auto bs = GetBotSpellSetting(spell_id);
	if (!bs) {
		return false;
	}

	BotSpellSettingsRepository::DeleteWhere(
		content_db,
		fmt::format(
			"bot_id = {} AND spell_id = {}",
			GetBotID(),
			spell_id
		)
	);
	LoadBotSpellSettings();
	return true;
}

BotSpellSetting* Bot::GetBotSpellSetting(uint16 spell_id)
{
	if (!IsValidSpell(spell_id) || !bot_spell_settings.count(spell_id)) {
		return nullptr;
	}

	auto b = bot_spell_settings.find(spell_id);
	if (b != bot_spell_settings.end()) {
		return &b->second;
	}

	return nullptr;
}

void Bot::ListBotSpells(uint8 min_level)
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return;
	}

	if (AIBot_spells.empty() && AIBot_spells_enforced.empty()) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} has no AI Spells.",
				GetCleanName()
			).c_str()
		);
		return;
	}

	auto spell_count = 0;
	auto spell_number = 1;

	for (const auto& s : (AIBot_spells.size() > AIBot_spells_enforced.size()) ? AIBot_spells : AIBot_spells_enforced) {
		auto b = bot_spell_settings.find(s.spellid);
		if (b == bot_spell_settings.end() && s.minlevel >= min_level) {
			bot_owner->Message(
				Chat::White,
				fmt::format(
					"Spell {} | Spell: {} | Add Spell: {}",
					spell_number,
					Saylink::Silent(
						fmt::format("^spellinfo {}", s.spellid),
						spells[s.spellid].name
					),
					Saylink::Silent(
						fmt::format("^spellsettingsadd {} {} {} {}", s.spellid, s.priority, s.min_hp, s.max_hp), "Add")
				).c_str()
			);

			spell_count++;
			spell_number++;
		}
	}

	bot_owner->Message(
		Chat::White,
		fmt::format(
			"{} has {} AI Spell{}.",
			GetCleanName(),
			spell_count,
			spell_count != 1 ? "s" :""
		).c_str()
	);
}

void Bot::ListBotSpellSettings()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return;
	}

	if (!bot_spell_settings.size()) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} does not have any spell settings.",
				GetCleanName()
			).c_str()
		);
		return;
	}

	auto setting_count = 0;
	auto setting_number = 1;

	for (const auto& bs : bot_spell_settings) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Setting {} | Spell: {} | State: {} | {}",
				setting_number,
				Saylink::Silent(fmt::format("^spellinfo {}", bs.first), spells[bs.first].name),
				Saylink::Silent(
					fmt::format("^spellsettingstoggle {} {}",
					bs.first, bs.second.is_enabled ? "False" : "True"),
					bs.second.is_enabled ? "Enabled" : "Disabled"
				),
				Saylink::Silent(fmt::format("^spellsettingsdelete {}", bs.first), "Remove")
			).c_str()
		);

		setting_count++;
		setting_number++;
	}

	bot_owner->Message(
		Chat::White,
		fmt::format(
			"{} has {} spell setting{}.",
			GetCleanName(),
			setting_count,
			setting_count != 1 ? "s" : ""
		).c_str()
	);
}

void Bot::LoadBotSpellSettings()
{
	bot_spell_settings.clear();

	auto s = BotSpellSettingsRepository::GetWhere(content_db, fmt::format("bot_id = {}", GetBotID()));
	if (s.empty()) {
		return;
	}

	for (const auto& e : s) {
		BotSpellSetting b;

		b.priority = e.priority;
		b.min_hp = e.min_hp;
		b.max_hp = e.max_hp;
		b.is_enabled = e.is_enabled;
		bot_spell_settings[e.spell_id] = b;
	}
}

bool Bot::UpdateBotSpellSetting(uint16 spell_id, BotSpellSetting* bs)
{
	if (!IsValidSpell(spell_id) || !bs) {
		return false;
	}

	auto s = BotSpellSettingsRepository::NewEntity();

	s.spell_id = spell_id;
	s.bot_id = GetBotID();
	s.priority = bs->priority;
	s.min_hp = bs->min_hp;
	s.max_hp = bs->max_hp;
	s.is_enabled = bs->is_enabled;

	auto obs = GetBotSpellSetting(spell_id);
	if (!obs) {
		return false;
	}

	if (!BotSpellSettingsRepository::UpdateSpellSetting(content_db, s)) {
		return false;
	}

	LoadBotSpellSettings();
	return true;
}

std::string Bot::GetHPString(int8 min_hp, int8 max_hp)
{
	std::string hp_string = "Any";
	if (min_hp && max_hp) {
		hp_string = fmt::format(
			"{}%% to {}%%",
			min_hp,
			max_hp
		);
	} else if (min_hp && !max_hp) {
		hp_string = fmt::format(
			"{}%% to 100%%",
			min_hp
		);
	} else if (!min_hp && max_hp) {
		hp_string = fmt::format(
			"1%% to {}%%",
			max_hp
		);
	}

	return hp_string;
}

void Bot::SetBotArcherySetting(bool bot_archer_setting, bool save)
{
	m_bot_archery_setting = bot_archer_setting;
	if (save) {
		if (!database.botdb.SaveBotArcherSetting(GetBotID(), bot_archer_setting)) {
			if (GetBotOwner() && GetBotOwner()->IsClient()) {
				GetBotOwner()->CastToClient()->Message(
					Chat::White,
					fmt::format(
						"Failed to save archery settings for {}.",
						GetCleanName()
					).c_str()
				);
			}
		}
	}
}

std::vector<Mob*> Bot::GetApplySpellList(
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	std::vector<Mob*> l;

	if (apply_type == ApplySpellType::Raid && IsRaidGrouped()) {
		auto* r = GetRaid();
		auto group_id = r->GetGroup(this->GetCleanName());
		if (r && EQ::ValueWithin(group_id, 0, (MAX_RAID_GROUPS - 1))) {
			for (auto i = 0; i < MAX_RAID_MEMBERS; i++) {
				auto* m = r->members[i].member;
				if (m && m->IsClient() && (!is_raid_group_only || r->GetGroup(m) == group_id)) {
					l.push_back(m);

					if (allow_pets && m->HasPet()) {
						l.push_back(m->GetPet());
					}

					const auto& sbl = entity_list.GetBotListByCharacterID(m->CharacterID());
					for (const auto& b : sbl) {
						l.push_back(b);
					}
				}
			}
		}
	} else if (apply_type == ApplySpellType::Group && IsGrouped()) {
		auto* g = GetGroup();
		if (g) {
			for (auto i = 0; i < MAX_GROUP_MEMBERS; i++) {
				auto* m = g->members[i];
				if (m && m->IsClient()) {
					l.push_back(m->CastToClient());

					if (allow_pets && m->HasPet()) {
						l.push_back(m->GetPet());
					}
					const auto& sbl = entity_list.GetBotListByCharacterID(m->CastToClient()->CharacterID());
					for (const auto& b : sbl) {
						l.push_back(b);
					}
				}
			}
		}
	} else {
		l.push_back(this);

		if (allow_pets && HasPet()) {
			l.push_back(GetPet());
		}
		const auto& sbl = entity_list.GetBotListByCharacterID(CharacterID());
		for (const auto& b : sbl) {
			l.push_back(b);
		}
	}

	return l;
}

void Bot::ApplySpell(
	int spell_id,
	int duration,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only);

	for (const auto& m : l) {
		m->ApplySpellBuff(spell_id, duration);
	}
}

void Bot::SetSpellDuration(
	int spell_id,
	int duration,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only);

	for (const auto& m : l) {
		m->SetBuffDuration(spell_id, duration);
	}
}

void Bot::Escape()
{
	entity_list.RemoveFromTargets(this, true);
	SetInvisible(Invisibility::Invisible);
}

void Bot::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls, bool calculate_speed) {
	BuffFadeByEffect(SE_Levitate);
	if (CheckLosFN(target_x, target_y, target_z, 6.0f) || ignore_los) {
		auto p = new EQApplicationPacket(OP_Fling, sizeof(fling_struct));
		auto* f = (fling_struct*) p->pBuffer;

		if (!calculate_speed) {
			f->speed_z = value;
		} else {
			auto speed = 1.0f;
			const auto distance = CalculateDistance(target_x, target_y, target_z);

			auto z_diff = target_z - GetZ();
			if (z_diff != 0.0f) {
				speed += std::abs(z_diff) / 12.0f;
			}

			speed += distance / 200.0f;

			speed++;

			speed = std::abs(speed);

			f->speed_z = speed;
		}

		f->collision = clip_through_walls ? 0 : -1;
		f->travel_time = -1;
		f->unk3 = 1;
		f->disable_fall_damage = 1;
		f->new_y = target_y;
		f->new_x = target_x;
		f->new_z = target_z;
		p->priority = 6;
		GetBotOwner()->CastToClient()->FastQueuePacket(&p);
	}
}

// This should return the combined AC of all the items the Bot is wearing.
int32 Bot::GetRawItemAC()
{
	int32 Total = 0;
	// this skips MainAmmo..add an '=' conditional if that slot is required (original behavior)
	for (int16 slot_id = EQ::invslot::BONUS_BEGIN; slot_id <= EQ::invslot::BONUS_STAT_END; slot_id++) {
		const EQ::ItemInstance* inst = m_inv[slot_id];
		if (inst && inst->IsClassCommon()) {
			Total += inst->GetItem()->AC;
		}
	}
	return Total;
}

void Bot::SendSpellAnim(uint16 target_id, uint16 spell_id)
{
	if (!target_id || !IsValidSpell(spell_id)) {
		return;
	}

	EQApplicationPacket app(OP_Action, sizeof(Action_Struct));
	auto* a = (Action_Struct*) app.pBuffer;

	a->target      = target_id;
	a->source      = GetID();
	a->type        = 231;
	a->spell       = spell_id;
	a->hit_heading = GetHeading();

	app.priority = 1;
	entity_list.QueueCloseClients(this, &app, false, RuleI(Range, SpellParticles));
}

uint8 Bot::spell_casting_chances[SPELL_TYPE_COUNT][PLAYER_CLASS_COUNT][EQ::constants::STANCE_TYPE_COUNT][cntHSND] = { 0 };
