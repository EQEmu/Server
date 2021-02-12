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

#ifdef BOTS

#include "bot.h"
#include "object.h"
#include "doors.h"
#include "quest_parser_collection.h"
#include "lua_parser.h"
#include "../common/string_util.h"
#include "../common/say_link.h"

extern volatile bool is_zone_loaded;

// This constructor is used during the bot create command
Bot::Bot(NPCType *npcTypeData, Client* botOwner) : NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), ping_timer(1) {
	GiveNPCTypeData(npcTypeData);
	
	if(botOwner) {
		this->SetBotOwner(botOwner);
		this->_botOwnerCharacterID = botOwner->CharacterID();
	} else {
		this->SetBotOwner(0);
		this->_botOwnerCharacterID = 0;
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
	SetBotID(0);
	SetBotSpellID(0);
	SetSpawnStatus(false);
	SetBotArcher(false);
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

	strcpy(this->name, this->GetCleanName());
	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
}

// This constructor is used when the bot is loaded out of the database
Bot::Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, double totalPlayTime, uint32 lastZoneId, NPCType *npcTypeData) 
	: NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), ping_timer(1)
{
	GiveNPCTypeData(npcTypeData);
	
	this->_botOwnerCharacterID = botOwnerCharacterID;
	if(this->_botOwnerCharacterID > 0)
		this->SetBotOwner(entity_list.GetClientByCharID(this->_botOwnerCharacterID));

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
	SetBotArcher(false);
	SetBotCharmer(false);
	SetPetChooser(false);
	SetRangerAutoWeaponSelect(false);

	bool stance_flag = false;
	if (!database.botdb.LoadStance(this, stance_flag) && bot_owner)
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::LoadStance(), GetCleanName());
	if (!stance_flag && bot_owner)
		bot_owner->Message(Chat::Red, "Could not locate stance for '%s'", GetCleanName());

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

	strcpy(this->name, this->GetCleanName());

	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
	if (!database.botdb.LoadInspectMessage(GetBotID(), _botInspectMessage) && bot_owner)
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::LoadInspectMessage(), GetCleanName());

	if (!database.botdb.LoadGuildMembership(GetBotID(), _guildId, _guildRank, _guildName) && bot_owner)
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::LoadGuildMembership(), GetCleanName());
	
	std::string error_message;

	EquipBot(&error_message);
	if(!error_message.empty()) {
		if(bot_owner)
			bot_owner->Message(Chat::Red, error_message.c_str());
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
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::LoadTimers(), GetCleanName());

	LoadAAs();

	// copied from client CompleteConnect() handler - watch for problems
	// (may have to move to post-spawn location if certain buffs still don't process correctly)
	if (database.botdb.LoadBuffs(this) && bot_owner) {

		//reapply some buffs
		uint32 buff_count = GetMaxTotalSlots();
		for (uint32 j1 = 0; j1 < buff_count; j1++) {
			if (!IsValidSpell(buffs[j1].spellid))
				continue;

			const SPDat_Spell_Struct& spell = spells[buffs[j1].spellid];

			int NimbusEffect = GetNimbusEffect(buffs[j1].spellid);
			if (NimbusEffect) {
				if (!IsNimbusEffectActive(NimbusEffect))
					SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
			}

			for (int x1 = 0; x1 < EFFECT_COUNT; x1++) {
				switch (spell.effectid[x1]) {
				case SE_IllusionCopy:
				case SE_Illusion: {
					if (spell.base[x1] == -1) {
						if (gender == 1)
							gender = 0;
						else if (gender == 0)
							gender = 1;
						SendIllusionPacket(GetRace(), gender, 0xFF, 0xFF);
					}
					else if (spell.base[x1] == -2) // WTF IS THIS
					{
						if (GetRace() == 128 || GetRace() == 130 || GetRace() <= 12)
							SendIllusionPacket(GetRace(), GetGender(), spell.base2[x1], spell.max[x1]);
					}
					else if (spell.max[x1] > 0)
					{
						SendIllusionPacket(spell.base[x1], 0xFF, spell.base2[x1], spell.max[x1]);
					}
					else
					{
						SendIllusionPacket(spell.base[x1], 0xFF, 0xFF, 0xFF);
					}
					switch (spell.base[x1]) {
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
							//Message(Chat::Red, "You can't levitate in this zone.");
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
					AddProcToWeapon(GetProcID(buffs[j1].spellid, x1), false, 100 + spells[buffs[j1].spellid].base2[x1], buffs[j1].spellid, buffs[j1].casterlevel);
					break;
				}
				case SE_DefensiveProc:
				{
					AddDefensiveProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].base2[x1], buffs[j1].spellid);
					break;
				}
				case SE_RangedProc:
				{
					AddRangedProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].base2[x1], buffs[j1].spellid);
					break;
				}
				}
			}
		}

		
	}
	else {
		bot_owner->Message(Chat::Red, "&s for '%s'", BotDatabase::fail::LoadBuffs(), GetCleanName());
	}

	CalcBotStats(false);
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
	if(current_hp > max_hp)
		current_hp = max_hp;

	if(current_hp <= 0) {
		SetHP(max_hp/5);
		SetMana(0);
		BuffFadeAll();
		SpellOnTarget(756, this); // Rezz effects
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
	this->_botID = botID;
	this->npctype_id = botID;
}

void Bot::SetBotSpellID(uint32 newSpellID) {
	this->npc_spells_id = newSpellID;
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

NPCType *Bot::FillNPCTypeStruct(uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender, float size, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, int32 hp, int32 mana, int32 mr, int32 cr, int32 dr, int32 fr, int32 pr, int32 corrup, int32 ac, uint32 str, uint32 sta, uint32 dex, uint32 agi, uint32 _int, uint32 wis, uint32 cha, uint32 attack) {
	auto bot_npc_type = new NPCType{ 0 };
	int copy_length = 0;

	copy_length = botName.copy(bot_npc_type->name, 63);
	bot_npc_type->name[copy_length] = '\0';
	copy_length = 0;

	copy_length = botLastName.copy(bot_npc_type->lastname, 69);
	bot_npc_type->lastname[copy_length] = '\0';
	copy_length = 0;

	bot_npc_type->current_hp = hp;
	bot_npc_type->max_hp = hp;
	bot_npc_type->size = size;
	bot_npc_type->runspeed = 0.7f;
	bot_npc_type->gender = gender;
	bot_npc_type->race = botRace;
	bot_npc_type->class_ = botClass;
	bot_npc_type->bodytype = 1;
	bot_npc_type->deity = EQ::deity::DeityAgnostic;
	bot_npc_type->level = botLevel;
	//bot_npc_type->npc_id = 0;
	//bot_npc_type->texture = 0;
	//bot_npc_type->helmtexture = 0;
	//bot_npc_type->herosforgemodel = 0;
	//bot_npc_type->loottable_id = 0;
	bot_npc_type->npc_spells_id = botSpellsID;
	//bot_npc_type->npc_spells_effects_id = 0;
	//bot_npc_type->npc_faction_id = 0;
	//bot_npc_type->merchanttype = 0;
	//bot_npc_type->alt_currency_type = 0;
	//bot_npc_type->adventure_template = 0;
	//bot_npc_type->trap_template = 0;
	//bot_npc_type->light = 0;
	bot_npc_type->AC = ac;
	bot_npc_type->Mana = mana;
	bot_npc_type->ATK = attack;
	bot_npc_type->STR = str;
	bot_npc_type->STA = sta;
	bot_npc_type->DEX = dex;
	bot_npc_type->AGI = agi;
	bot_npc_type->INT = _int;
	bot_npc_type->WIS = wis;
	bot_npc_type->CHA = cha;
	bot_npc_type->MR = mr;
	bot_npc_type->FR = fr;
	bot_npc_type->CR = cr;
	bot_npc_type->PR = pr;
	bot_npc_type->DR = dr;
	bot_npc_type->Corrup = corrup;
	//bot_npc_type->PhR = 0;
	bot_npc_type->haircolor = hairColor;
	bot_npc_type->beardcolor = beardColor;
	bot_npc_type->eyecolor1 = eyeColor;
	bot_npc_type->eyecolor2 = eyeColor2;
	bot_npc_type->hairstyle = hairStyle;
	bot_npc_type->luclinface = face;
	bot_npc_type->beard = beard;
	bot_npc_type->drakkin_heritage = drakkinHeritage;
	bot_npc_type->drakkin_tattoo = drakkinTattoo;
	bot_npc_type->drakkin_details = drakkinDetails;
	//bot_npc_type->armor_tint = { 0 };
	//bot_npc_type->min_dmg = 0;
	//bot_npc_type->max_dmg = 0;
	//bot_npc_type->charm_ac = 0;
	//bot_npc_type->charm_min_dmg = 0;
	//bot_npc_type->charm_max_dmg = 0;
	//bot_npc_type->charm_attack_delay = 0;
	//bot_npc_type->charm_accuracy_rating = 0;
	//bot_npc_type->charm_avoidance_rating = 0;
	//bot_npc_type->charm_atk = 0;
	//bot_npc_type->attack_count = 0;
	//*bot_npc_type->special_abilities = { 0 };
	//bot_npc_type->d_melee_texture1 = 0;
	//bot_npc_type->d_melee_texture2 = 0;
	//*bot_npc_type->ammo_idfile = { 0 };
	//bot_npc_type->prim_melee_type = 0;
	//bot_npc_type->sec_melee_type = 0;
	//bot_npc_type->ranged_type = 0;
	bot_npc_type->hp_regen = 1;
	bot_npc_type->mana_regen = 1;
	//bot_npc_type->aggroradius = 0;
	//bot_npc_type->assistradius = 0;
	//bot_npc_type->see_invis = 0;
	//bot_npc_type->see_invis_undead = false;
	//bot_npc_type->see_hide = false;
	//bot_npc_type->see_improved_hide = false;
	//bot_npc_type->qglobal = false;
	//bot_npc_type->npc_aggro = false;
	//bot_npc_type->spawn_limit = 0;
	//bot_npc_type->mount_color = 0;
	//bot_npc_type->attack_speed = 0.0f;
	//bot_npc_type->attack_delay = 0;
	//bot_npc_type->accuracy_rating = 0;
	//bot_npc_type->avoidance_rating = 0;
	//bot_npc_type->findable = false;
	bot_npc_type->trackable = true;
	//bot_npc_type->slow_mitigation = 0;
	bot_npc_type->maxlevel = botLevel;
	//bot_npc_type->scalerate = 0;
	//bot_npc_type->private_corpse = false;
	//bot_npc_type->unique_spawn_by_name = false;
	//bot_npc_type->underwater = false;
	//bot_npc_type->emoteid = 0;
	//bot_npc_type->spellscale = 0.0f;
	//bot_npc_type->healscale = 0.0f;
	//bot_npc_type->no_target_hotkey = false;
	//bot_npc_type->raid_target = false;
	//bot_npc_type->armtexture = 0;
	//bot_npc_type->bracertexture = 0;
	//bot_npc_type->handtexture = 0;
	//bot_npc_type->legtexture = 0;
	//bot_npc_type->feettexture = 0;
	//bot_npc_type->ignore_despawn = false;
	bot_npc_type->show_name = true;
	//bot_npc_type->untargetable = false;
	bot_npc_type->skip_global_loot = true;
	//bot_npc_type->rare_spawn = false;
	bot_npc_type->stuck_behavior = Ground;
	bot_npc_type->skip_auto_scale = true;

	return bot_npc_type;
}

NPCType *Bot::CreateDefaultNPCTypeStructForBot(std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender) {
	auto bot_npc_type = new NPCType{ 0 };
	int copy_length = 0;

	copy_length = botName.copy(bot_npc_type->name, 63);
	bot_npc_type->name[copy_length] = '\0';
	copy_length = 0;

	copy_length = botLastName.copy(bot_npc_type->lastname, 69);
	bot_npc_type->lastname[copy_length] = '\0';
	copy_length = 0;

	//bot_npc_type->current_hp = 0;
	//bot_npc_type->max_hp = 0;
	bot_npc_type->size = 6.0f;
	bot_npc_type->runspeed = 0.7f;
	bot_npc_type->gender = gender;
	bot_npc_type->race = botRace;
	bot_npc_type->class_ = botClass;
	bot_npc_type->bodytype = 1;
	bot_npc_type->deity = EQ::deity::DeityAgnostic;
	bot_npc_type->level = botLevel;
	//bot_npc_type->npc_id = 0;
	//bot_npc_type->texture = 0;
	//bot_npc_type->helmtexture = 0;
	//bot_npc_type->herosforgemodel = 0;
	//bot_npc_type->loottable_id = 0;
	//bot_npc_type->npc_spells_id = 0;
	//bot_npc_type->npc_spells_effects_id = 0;
	//bot_npc_type->npc_faction_id = 0;
	//bot_npc_type->merchanttype = 0;
	//bot_npc_type->alt_currency_type = 0;
	//bot_npc_type->adventure_template = 0;
	//bot_npc_type->trap_template = 0;
	//bot_npc_type->light = 0;
	bot_npc_type->AC = 12;
	//bot_npc_type->Mana = 0;
	bot_npc_type->ATK = 75;
	bot_npc_type->STR = 75;
	bot_npc_type->STA = 75;
	bot_npc_type->DEX = 75;
	bot_npc_type->AGI = 75;
	bot_npc_type->INT = 75;
	bot_npc_type->WIS = 75;
	bot_npc_type->CHA = 75;
	bot_npc_type->MR = 25;
	bot_npc_type->FR = 25;
	bot_npc_type->CR = 25;
	bot_npc_type->PR = 15;
	bot_npc_type->DR = 15;
	bot_npc_type->Corrup = 15;
	//bot_npc_type->PhR = 0;
	//bot_npc_type->haircolor = 0;
	//bot_npc_type->beardcolor = 0;
	//bot_npc_type->eyecolor1 = 0;
	//bot_npc_type->eyecolor2 = 0;
	//bot_npc_type->hairstyle = 0;
	//bot_npc_type->luclinface = 0;
	//bot_npc_type->beard = 0;
	//bot_npc_type->drakkin_heritage = 0;
	//bot_npc_type->drakkin_tattoo = 0;
	//bot_npc_type->drakkin_details = 0;
	//bot_npc_type->armor_tint = { 0 };
	//bot_npc_type->min_dmg = 0;
	//bot_npc_type->max_dmg = 0;
	//bot_npc_type->charm_ac = 0;
	//bot_npc_type->charm_min_dmg = 0;
	//bot_npc_type->charm_max_dmg = 0;
	//bot_npc_type->charm_attack_delay = 0;
	//bot_npc_type->charm_accuracy_rating = 0;
	//bot_npc_type->charm_avoidance_rating = 0;
	//bot_npc_type->charm_atk = 0;
	//bot_npc_type->attack_count = 0;
	//*bot_npc_type->special_abilities = { 0 };
	//bot_npc_type->d_melee_texture1 = 0;
	//bot_npc_type->d_melee_texture2 = 0;
	//*bot_npc_type->ammo_idfile = { 0 };
	//bot_npc_type->prim_melee_type = 0;
	//bot_npc_type->sec_melee_type = 0;
	//bot_npc_type->ranged_type = 0;
	bot_npc_type->hp_regen = 1;
	bot_npc_type->mana_regen = 1;
	//bot_npc_type->aggroradius = 0;
	//bot_npc_type->assistradius = 0;
	//bot_npc_type->see_invis = 0;
	//bot_npc_type->see_invis_undead = false;
	//bot_npc_type->see_hide = false;
	//bot_npc_type->see_improved_hide = false;
	//bot_npc_type->qglobal = false;
	//bot_npc_type->npc_aggro = false;
	//bot_npc_type->spawn_limit = 0;
	//bot_npc_type->mount_color = 0;
	//bot_npc_type->attack_speed = 0.0f;
	//bot_npc_type->attack_delay = 0;
	//bot_npc_type->accuracy_rating = 0;
	//bot_npc_type->avoidance_rating = 0;
	//bot_npc_type->findable = false;
	bot_npc_type->trackable = true;
	//bot_npc_type->slow_mitigation = 0;
	bot_npc_type->maxlevel = botLevel;
	//bot_npc_type->scalerate = 0;
	//bot_npc_type->private_corpse = false;
	//bot_npc_type->unique_spawn_by_name = false;
	//bot_npc_type->underwater = false;
	//bot_npc_type->emoteid = 0;
	//bot_npc_type->spellscale = 0.0f;
	//bot_npc_type->healscale = 0.0f;
	//bot_npc_type->no_target_hotkey = false;
	//bot_npc_type->raid_target = false;
	//bot_npc_type->armtexture = 0;
	//bot_npc_type->bracertexture = 0;
	//bot_npc_type->handtexture = 0;
	//bot_npc_type->legtexture = 0;
	//bot_npc_type->feettexture = 0;
	//bot_npc_type->ignore_despawn = false;
	bot_npc_type->show_name = true;
	//bot_npc_type->untargetable = false;
	bot_npc_type->skip_global_loot = true;
	//bot_npc_type->rare_spawn = false;
	bot_npc_type->stuck_behavior = Ground;

	return bot_npc_type;
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
	switch(this->GetClass()) {
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

	switch(this->GetRace()) {
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

	this->STR = Strength;
	this->STA = Stamina;
	this->DEX = Dexterity;
	this->AGI = Agility;
	this->WIS = Wisdom;
	this->INT = Intelligence;
	this->CHA = Charisma;
	this->ATK = Attack;
	this->MR = MagicResist;
	this->FR = FireResist;
	this->DR = DiseaseResist;
	this->PR = PoisonResist;
	this->CR = ColdResist;
	this->PhR = 0;
	this->Corrup = CorruptionResist;
	SetBotSpellID(BotSpellID);
	this->size = BotSize;
	this->pAggroRange = 0;
	this->pAssistRange = 0;
	this->raid_target = false;
	this->deity = 396;
}

void Bot::GenerateAppearance() {
	// Randomize facial appearance
	int iFace = 0;
	if(this->GetRace() == 2) // Barbarian w/Tatoo
		iFace = zone->random.Int(0, 79);
	else
		iFace = zone->random.Int(0, 7);

	int iHair = 0;
	int iBeard = 0;
	int iBeardColor = 1;
	if(this->GetRace() == 522) {
		iHair = zone->random.Int(0, 8);
		iBeard = zone->random.Int(0, 11);
		iBeardColor = zone->random.Int(0, 3);
	} else if(this->GetGender()) {
		iHair = zone->random.Int(0, 2);
		if(this->GetRace() == 8) { // Dwarven Females can have a beard
			if(zone->random.Int(1, 100) < 50)
				iFace += 10;
		}
	} else {
		iHair = zone->random.Int(0, 3);
		iBeard = zone->random.Int(0, 5);
		iBeardColor = zone->random.Int(0, 19);
	}

	int iHairColor = 0;
	if(this->GetRace() == 522)
		iHairColor = zone->random.Int(0, 3);
	else
		iHairColor = zone->random.Int(0, 19);

	uint8 iEyeColor1 = (uint8)zone->random.Int(0, 9);
	uint8 iEyeColor2 = 0;
	if(this->GetRace() == 522)
		iEyeColor1 = iEyeColor2 = (uint8)zone->random.Int(0, 11);
	else if(zone->random.Int(1, 100) > 96)
		iEyeColor2 = zone->random.Int(0, 9);
	else
		iEyeColor2 = iEyeColor1;

	int iHeritage = 0;
	int iTattoo = 0;
	int iDetails = 0;
	if(this->GetRace() == 522) {
		iHeritage = zone->random.Int(0, 6);
		iTattoo = zone->random.Int(0, 7);
		iDetails = zone->random.Int(0, 7);
	}
	this->luclinface = iFace;
	this->hairstyle = iHair;
	this->beard = iBeard;
	this->beardcolor = iBeardColor;
	this->haircolor = iHairColor;
	this->eyecolor1 = iEyeColor1;
	this->eyecolor2 = iEyeColor2;
	this->drakkin_heritage = iHeritage;
	this->drakkin_tattoo = iTattoo;
	this->drakkin_details = iDetails;
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
#if EQDEBUG >= 11
	LogError("Error in Bot::acmod(): Agility: [{}], Level: [{}]",agility,level);
#endif
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
	return(database.GetSkillCap(class_, skillid, level));
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
	this->base_hp = new_base_hp;
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

bool Bot::IsValidRaceClassCombo(uint16 r, uint8 c)
{
	switch (r) {
	case HUMAN:
		switch (c) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case RANGER:
		case SHADOWKNIGHT:
		case DRUID:
		case MONK:
		case BARD:
		case ROGUE:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		}
		break;
	case BARBARIAN:
		switch (c) {
		case WARRIOR:
		case ROGUE:
		case SHAMAN:
		case BEASTLORD:
		case BERSERKER:
			return true;
		}
		break;
	case ERUDITE:
		switch (c) {
		case CLERIC:
		case PALADIN:
		case SHADOWKNIGHT:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		}
		break;
	case WOOD_ELF:
		switch (c) {
		case WARRIOR:
		case RANGER:
		case DRUID:
		case BARD:
		case ROGUE:
			return true;
		}
		break;
	case HIGH_ELF:
		switch (c) {
		case CLERIC:
		case PALADIN:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		}
		break;
	case DARK_ELF:
		switch (c) {
		case WARRIOR:
		case CLERIC:
		case SHADOWKNIGHT:
		case ROGUE:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		}
		break;
	case HALF_ELF:
		switch (c) {
		case WARRIOR:
		case PALADIN:
		case RANGER:
		case DRUID:
		case BARD:
		case ROGUE:
			return true;
		}
		break;
	case DWARF:
		switch (c) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case ROGUE:
		case BERSERKER:
			return true;
		}
		break;
	case TROLL:
		switch (c) {
		case WARRIOR:
		case SHADOWKNIGHT:
		case SHAMAN:
		case BEASTLORD:
		case BERSERKER:
			return true;
		}
		break;
	case OGRE:
		switch (c) {
		case WARRIOR:
		case SHADOWKNIGHT:
		case SHAMAN:
		case BEASTLORD:
		case BERSERKER:
			return true;
		}
		break;
	case HALFLING:
		switch (c) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case RANGER:
		case DRUID:
		case ROGUE:
			return true;
		}
		break;
	case GNOME:
		switch (c) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case SHADOWKNIGHT:
		case ROGUE:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		}
		break;
	case IKSAR:
		switch (c) {
		case WARRIOR:
		case SHADOWKNIGHT:
		case MONK:
		case SHAMAN:
		case NECROMANCER:
		case BEASTLORD:
			return true;
		}
		break;
	case VAHSHIR:
		switch (c) {
		case WARRIOR:
		case BARD:
		case ROGUE:
		case SHAMAN:
		case BEASTLORD:
		case BERSERKER:
			return true;
		}
		break;
	case FROGLOK:
		switch (c) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case SHADOWKNIGHT:
		case ROGUE:
		case SHAMAN:
		case NECROMANCER:
		case WIZARD:
			return true;
		}
		break;
	case DRAKKIN:
		switch (c) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case RANGER:
		case SHADOWKNIGHT:
		case DRUID:
		case MONK:
		case BARD:
		case ROGUE:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

bool Bot::IsValidName()
{
	std::string name = this->GetCleanName();
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
			bot_owner->Message(Chat::Red, "%s '%s'", BotDatabase::fail::SaveNewBot(), GetCleanName());
			return false;
		}
		SetBotID(bot_id);
	}
	else { // Update existing bot record
		if (!database.botdb.SaveBot(this)) {
			bot_owner->Message(Chat::Red, "%s '%s'", BotDatabase::fail::SaveBot(), GetCleanName());
			return false;
		}
	}
	
	// All of these continue to process if any fail
	if (!database.botdb.SaveBuffs(this))
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::SaveBuffs(), GetCleanName());
	if (!database.botdb.SaveTimers(this))
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::SaveTimers(), GetCleanName());
	if (!database.botdb.SaveStance(this))
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::SaveStance(), GetCleanName());
	
	if (!SavePet())
		bot_owner->Message(Chat::Red, "Failed to save pet for '%s'", GetCleanName());
	
	return true;
}

bool Bot::DeleteBot()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	if (!database.botdb.DeleteHealRotation(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s", BotDatabase::fail::DeleteHealRotation());
		return false;
	}

	std::string query = StringFormat("DELETE FROM `bot_heal_rotation_members` WHERE `bot_id` = '%u'", GetBotID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(Chat::Red, "Failed to delete heal rotation member '%s'", GetCleanName());
		return false;
	}

	query = StringFormat("DELETE FROM `bot_heal_rotation_targets` WHERE `target_name` LIKE '%s'", GetCleanName());
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(Chat::Red, "Failed to delete heal rotation target '%s'", GetCleanName());
		return false;
	}

	if (!DeletePet()) {
		bot_owner->Message(Chat::Red, "Failed to delete pet for '%s'", GetCleanName());
		return false;
	}

	if (GetGroup())
		RemoveBotFromGroup(this, GetGroup());

	std::string error_message;

	if (!database.botdb.RemoveMemberFromBotGroup(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s - '%s'", BotDatabase::fail::RemoveMemberFromBotGroup(), GetCleanName());
		return false;
	}

	if (!database.botdb.DeleteItems(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::DeleteItems(), GetCleanName());
		return false;
	}

	if (!database.botdb.DeleteTimers(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::DeleteTimers(), GetCleanName());
		return false;
	}

	if (!database.botdb.DeleteBuffs(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::DeleteBuffs(), GetCleanName());
		return false;
	}

	if (!database.botdb.DeleteStance(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::DeleteStance(), GetCleanName());
		return false;
	}

	if (!database.botdb.DeleteBot(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s '%s'", BotDatabase::fail::DeleteBot(), GetCleanName());
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
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::LoadPetIndex(), GetCleanName());
		return false;
	}
	if (!pet_index)
		return true;

	uint32 saved_pet_spell_id = 0;
	if (!database.botdb.LoadPetSpellID(GetBotID(), saved_pet_spell_id)) {
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::LoadPetSpellID(), GetCleanName());
	}
	if (!IsValidSpell(saved_pet_spell_id)) {
		bot_owner->Message(Chat::Red, "Invalid spell id for %s's pet", GetCleanName());
		DeletePet();
		return false;
	}

	std::string pet_name;
	uint32 pet_mana = 0;
	uint32 pet_hp = 0;
	uint32 pet_spell_id = 0;

	if (!database.botdb.LoadPetStats(GetBotID(), pet_name, pet_mana, pet_hp, pet_spell_id)) {
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::LoadPetStats(), GetCleanName());
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
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::LoadPetBuffs(), GetCleanName());

	uint32 pet_items[EQ::invslot::EQUIPMENT_COUNT];
	memset(pet_items, 0, (sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT));
	if (!database.botdb.LoadPetItems(GetBotID(), pet_items))
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::LoadPetItems(), GetCleanName());

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
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::SavePetStats(), GetCleanName());
		return false;
	}
	
	if (!database.botdb.SavePetBuffs(GetBotID(), pet_buffs))
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::SavePetBuffs(), GetCleanName());
	if (!database.botdb.SavePetItems(GetBotID(), pet_items))
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::SavePetItems(), GetCleanName());

	return true;
}

bool Bot::DeletePet()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;
	
	std::string error_message;

	if (!database.botdb.DeletePetItems(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::DeletePetItems(), GetCleanName());
		return false;
	}
	if (!database.botdb.DeletePetBuffs(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::DeletePetBuffs(), GetCleanName());
		return false;
	}
	if (!database.botdb.DeletePetStats(GetBotID())) {
		bot_owner->Message(Chat::Red, "%s for %s's pet", BotDatabase::fail::DeletePetStats(), GetCleanName());
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
		LogAIScanClose(
			"is_moving [{}] bot [{}] timer [{}]",
			moving ? "true" : "false",
			GetCleanName(),
			mob_close_scan_timer.GetDuration()
		);

		entity_list.ScanCloseClientMobs(close_mobs, this);
	}

	SpellProcess();

	if(tic_timer.Check()) {

		// 6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if(!IsMoving() && !IsEngaged()) {

			if(IsSitting()) {

				if (!rest_timer.Enabled()) {
					rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
				}
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
	if(isSitting) {
		if(GetManaRatio() < 99.0f || GetHPRatio() < 99.0f) {
			if (!IsEngaged() && !IsSitting())
				Sit();
		} else {
			if(IsSitting())
				Stand();
		}
	} else {
		if(IsSitting())
			Stand();
	}

	if(IsSitting()) {
		if(!rest_timer.Enabled())
			rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
	}
	else
		rest_timer.Disable();
}

void Bot::BotRangedAttack(Mob* other) {
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check())) {
		LogCombat("Bot Archery attack canceled. Timer not up. Attack [{}], ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
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

	LogCombat("Shooting [{}] with bow [{}] ([{}]) and arrow [{}] ([{}])", other->GetCleanName(), RangeWeapon->Name, RangeWeapon->ID, Ammo->Name, Ammo->ID);
	if(!IsAttackAllowed(other) || IsCasting() || DivineAura() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead))
		return;

	SendItemAnimation(other, Ammo, EQ::skills::SkillArchery);
	//DoArcheryAttackDmg(GetTarget(), rangedItem, ammoItem);
	DoArcheryAttackDmg(other, rangedItem, ammoItem); // watch

	//break invis when you attack
	if(invisible) {
		LogCombat("Removing invisibility due to melee attack");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}

	if(invisible_undead) {
		LogCombat("Removing invisibility vs. undead due to melee attack");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}

	if(invisible_animals) {
		LogCombat("Removing invisibility vs. animals due to melee attack");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if(hidden || improved_hidden){
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

void Bot::ApplySpecialAttackMod(EQ::skills::SkillType skill, int32 &dmg, int32 &mindmg) {
	int item_slot = -1;
	//1: Apply bonus from AC (BOOT/SHIELD/HANDS) est. 40AC=6dmg
	switch (skill) {
	case EQ::skills::SkillFlyingKick:
	case EQ::skills::SkillRoundKick:
	case EQ::skills::SkillKick:
		item_slot = EQ::invslot::slotFeet;
		break;
	case EQ::skills::SkillBash:
		item_slot = EQ::invslot::slotSecondary;
		break;
	case EQ::skills::SkillDragonPunch:
	case EQ::skills::SkillEagleStrike:
	case EQ::skills::SkillTigerClaw:
		item_slot = EQ::invslot::slotHands;
		break;
	}

	if (item_slot >= EQ::invslot::EQUIPMENT_BEGIN){
		const EQ::ItemInstance* inst = GetBotItem(item_slot);
		const EQ::ItemData* botweapon = nullptr;
		if(inst)
			botweapon = inst->GetItem();

		if(botweapon)
			dmg += botweapon->AC * (RuleI(Combat, SpecialAttackACBonus))/100;
	}
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

				Bot::BotGroupSay(this, "Pulling %s to the group..", pull_target->GetCleanName());
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

						SetTarget(assist_mob->GetTarget());
						if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

							// This artificially inflates pet's target aggro..but, less expensive than checking hate each AI process
							GetPet()->AddToHateList(assist_mob->GetTarget(), 1);
							GetPet()->SetTarget(assist_mob->GetTarget());
						}
					}

					find_target = false;
				}
				else if (assist_mob != this) {

					SetTarget(nullptr);
					if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

						GetPet()->WipeHateList();
						GetPet()->SetTarget(nullptr);
					}

					find_target = false;
				}
			}

			if (find_target) {

				if (IsRooted()) {
					SetTarget(hate_list.GetClosestEntOnHateList(this, true));
				}
				else {

					// This will keep bots on target for now..but, future updates will allow for rooting/stunning
					SetTarget(hate_list.GetEscapingEntOnHateList(leash_owner, leash_distance));
					if (!GetTarget()) {
						SetTarget(hate_list.GetEntWithMostHateOnList(this, nullptr, true));
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
			
			SetTarget(nullptr);
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

				SetBotArcher(true);
				changeWeapons = true;
			}
			else if (!atArcheryRange && IsBotArcher()) {

				SetBotArcher(false);
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
					TryWeaponProc(p_item, tar, EQ::invslot::slotPrimary);

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
					int32 ExtraAttackChanceBonus = (spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance + aabonuses.ExtraAttackChance);
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
								TryWeaponProc(s_item, tar, EQ::invslot::slotSecondary);

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

					LogAI("Pursuing [{}] while engaged", GetTarget()->GetCleanName());
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
	
	Mob* BotOwner = this->GetBotOwner();
	NPC* botPet = this->GetPet()->CastToNPC();
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
						LogAI("Pursuing [{}] while engaged", botPet->GetTarget()->GetCleanName());
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
	if(GetBotID() > 0 && _botOwnerCharacterID > 0 && botCharacterOwner && botCharacterOwner->CharacterID() == _botOwnerCharacterID) {
		// Rename the bot name to make sure that Mob::GetName() matches Mob::GetCleanName() so we dont have a bot named "Jesuschrist001"
		strcpy(name, GetCleanName());

		// Get the zone id this bot spawned in
		_lastZoneId = GetZoneID();

		// this change propagates to Bot::FillSpawnStruct()
		this->helmtexture = 0; //0xFF;
		this->texture = 0; //0xFF;

		if(this->Save())
			this->GetBotOwner()->CastToClient()->Message(Chat::White, "%s saved.", this->GetCleanName());
		else
			this->GetBotOwner()->CastToClient()->Message(Chat::Red, "%s save failed!", this->GetCleanName());

		// Spawn the bot at the bot owner's loc
		this->m_Position.x = botCharacterOwner->GetX();
		this->m_Position.y = botCharacterOwner->GetY();
		this->m_Position.z = botCharacterOwner->GetZ();

		// Make the bot look at the bot owner
		FaceTarget(botCharacterOwner);
		UpdateEquipmentLight();
		UpdateActiveLight();

		this->m_targetable = true;
		entity_list.AddBot(this, true, true);
		// Load pet
		LoadPet();
		SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);
		ping_timer.Start(8000);
		// there is something askew with spawn struct appearance fields...
		// I re-enabled this until I can sort it out
		uint32 itemID = 0;
		uint8 materialFromSlot = 0xFF;
		for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; ++i) {
			itemID = GetBotItemBySlot(i);
			if(itemID != 0) {
				materialFromSlot = EQ::InventoryProfile::CalcMaterialFromSlot(i);
				if(materialFromSlot != 0xFF)
					this->SendWearChange(materialFromSlot);
			}
		}

		return true;
	}

	return false;
}

// Deletes the inventory record for the specified item from the database for this bot.
void Bot::RemoveBotItemBySlot(uint32 slotID, std::string *errorMessage)
{
	if(!GetBotID())
        return;

    if(!database.botdb.DeleteItemBySlot(GetBotID(), slotID))
        *errorMessage = BotDatabase::fail::DeleteItemBySlot();

    m_inv.DeleteItem(slotID);
	UpdateEquipmentLight();
}

// Retrieves all the inventory records from the database for this bot.
void Bot::GetBotItems(EQ::InventoryProfile &inv, std::string* errorMessage)
{
	if(!GetBotID())
		return;

	if (!database.botdb.LoadItems(GetBotID(), inv)) {
		*errorMessage = BotDatabase::fail::LoadItems();
		return;
	}

	UpdateEquipmentLight();
}

// Returns the inventory record for this bot from the database for the specified equipment slot.
uint32 Bot::GetBotItemBySlot(uint32 slotID)
{
	uint32 item_id = 0;
	if(!GetBotID())
        return item_id;

	if (!database.botdb.LoadItemBySlot(GetBotID(), slotID, item_id)) {
		if (GetBotOwner() && GetBotOwner()->IsClient())
			GetBotOwner()->CastToClient()->Message(Chat::Red, "%s", BotDatabase::fail::LoadItemBySlot());
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
		spawnedbotid = this->GetBotID();
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
void Bot::LoadAndSpawnAllZonedBots(Client* botOwner) {
	if(botOwner) {
		if(botOwner->HasGroup()) {
			Group* g = botOwner->GetGroup();
			if(g) {
				uint32 TempGroupId = g->GetID();
				std::list<uint32> ActiveBots;
				// Modified LoadGroupedBotsByGroupID to require a CharacterID
				if (!database.botdb.LoadGroupedBotsByGroupID(botOwner->CharacterID(), TempGroupId, ActiveBots)) {
					botOwner->Message(Chat::Red, "%s", BotDatabase::fail::LoadGroupedBotsByGroupID());
					return;
				}
				
				if(!ActiveBots.empty()) {
					for(std::list<uint32>::iterator itr = ActiveBots.begin(); itr != ActiveBots.end(); ++itr) {
						Bot* activeBot = Bot::LoadBot(*itr);
						if (!activeBot)
							continue;

						if (!activeBot->Spawn(botOwner)) {
							safe_delete(activeBot);
							continue;
						}

						g->UpdatePlayer(activeBot);
						// follow the bot owner, not the group leader we just zoned with our owner.
						if (g->IsGroupMember(botOwner) && g->IsGroupMember(activeBot))
							activeBot->SetFollowID(botOwner->GetID());

						if(!botOwner->HasGroup())
							database.SetGroupID(activeBot->GetCleanName(), 0, activeBot->GetBotID());
					}
				}
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

uint32 Bot::SpawnedBotCount(uint32 botOwnerCharacterID) {
	uint32 Result = 0;
	if(botOwnerCharacterID > 0) {
		std::list<Bot*> SpawnedBots = entity_list.GetBotsByBotOwnerCharacterID(botOwnerCharacterID);
		Result = SpawnedBots.size();
	}
	return Result;
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

std::string Bot::ClassIdToString(uint16 classId) {
	std::string Result;

	if(classId > 0 && classId < 17) {
		switch(classId) {
			case 1:
				Result = std::string("Warrior");
				break;
			case 2:
				Result = std::string("Cleric");
				break;
			case 3:
				Result = std::string("Paladin");
				break;
			case 4:
				Result = std::string("Ranger");
				break;
			case 5:
				Result = std::string("Shadowknight");
				break;
			case 6:
				Result = std::string("Druid");
				break;
			case 7:
				Result = std::string("Monk");
				break;
			case 8:
				Result = std::string("Bard");
				break;
			case 9:
				Result = std::string("Rogue");
				break;
			case 10:
				Result = std::string("Shaman");
				break;
			case 11:
				Result = std::string("Necromancer");
				break;
			case 12:
				Result = std::string("Wizard");
				break;
			case 13:
				Result = std::string("Magician");
				break;
			case 14:
				Result = std::string("Enchanter");
				break;
			case 15:
				Result = std::string("Beastlord");
				break;
			case 16:
				Result = std::string("Berserker");
				break;
		}
	}

	return Result;
}

std::string Bot::RaceIdToString(uint16 raceId) {
	std::string Result;

	if(raceId > 0) {
		switch(raceId) {
			case 1:
				Result = std::string("Human");
				break;
			case 2:
				Result = std::string("Barbarian");
				break;
			case 3:
				Result = std::string("Erudite");
				break;
			case 4:
				Result = std::string("Wood Elf");
				break;
			case 5:
				Result = std::string("High Elf");
				break;
			case 6:
				Result = std::string("Dark Elf");
				break;
			case 7:
				Result = std::string("Half Elf");
				break;
			case 8:
				Result = std::string("Dwarf");
				break;
			case 9:
				Result = std::string("Troll");
				break;
			case 10:
				Result = std::string("Ogre");
				break;
			case 11:
				Result = std::string("Halfling");
				break;
			case 12:
				Result = std::string("Gnome");
				break;
			case 128:
				Result = std::string("Iksar");
				break;
			case 130:
				Result = std::string("Vah Shir");
				break;
			case 330:
				Result = std::string("Froglok");
				break;
			case 522:
				Result = std::string("Drakkin");
				break;
		}
	}

	return Result;
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
EQ::ItemInstance* Bot::GetBotItem(uint32 slotID) {
	EQ::ItemInstance* item = m_inv.GetItem(slotID);
	if(item)
		return item;

	return nullptr;
}

// Adds the specified item it bot to the NPC equipment array and to the bot inventory collection.
void Bot::BotAddEquipItem(int slot, uint32 id) {
	// this is being called before bot is assigned an entity id..
	// ..causing packets to be sent out to zone with an id of '0'
	if(slot > 0 && id > 0) {
		uint8 materialFromSlot = EQ::InventoryProfile::CalcMaterialFromSlot(slot);

		if (materialFromSlot != EQ::textures::materialInvalid) {
			equipment[slot] = id; // npc has more than just material slots. Valid material should mean valid inventory index
			if (GetID()) // temp hack fix
				SendWearChange(materialFromSlot);
		}

		UpdateEquipmentLight();
		if (UpdateActiveLight())
			if (GetID()) // temp hack fix
				SendAppearancePacket(AT_Light, GetActiveLightType());
	}
}

// Erases the specified item from bot the NPC equipment array and from the bot inventory collection.
void Bot::BotRemoveEquipItem(int16 slot)
{
	uint8 material_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot);

	if (material_slot != EQ::textures::materialInvalid) {
		equipment[slot] = 0; // npc has more than just material slots. Valid material should mean valid inventory index
		SendWearChange(material_slot);
		if (material_slot == EQ::textures::armorChest)
			SendWearChange(EQ::textures::armorArms);
	}

	UpdateEquipmentLight();
	if (UpdateActiveLight())
		SendAppearancePacket(AT_Light, GetActiveLightType());
}

void Bot::BotTradeSwapItem(Client* client, int16 lootSlot, const EQ::ItemInstance* inst, const EQ::ItemInstance* inst_swap, uint32 equipableSlots, std::string* errorMessage, bool swap) {

	if(!errorMessage->empty())
		return;

	client->PushItemOnCursor(*inst_swap, true);

	// Remove the item from the bot and from the bot's database records
	RemoveBotItemBySlot(lootSlot, errorMessage);

	if(!errorMessage->empty())
		return;

	this->BotRemoveEquipItem(lootSlot);

	if(swap) {
		BotTradeAddItem(inst->GetItem()->ID, inst, inst->GetCharges(), equipableSlots, lootSlot, errorMessage);

		if(!errorMessage->empty())
			return;
	}
}

void Bot::BotTradeAddItem(uint32 id, const EQ::ItemInstance* inst, int16 charges, uint32 equipableSlots, uint16 lootSlot, std::string* errorMessage, bool addToDb)
{
	if(addToDb) {
		if (!database.botdb.SaveItemBySlot(this, lootSlot, inst)) {
			*errorMessage = BotDatabase::fail::SaveItemBySlot();
			return;
		}

		m_inv.PutItem(lootSlot, *inst);
	}

	this->BotAddEquipItem(lootSlot, id);
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
void Bot::FinishTrade(Client* client, BotTradeType tradeType)
{
	if (!client || (GetOwner() != client) || client->GetTradeskillObject() || client->trade->state == Trading) {
		if (client)
			client->ResetTrade();
		return;
	}

	// these notes are not correct or obselete
	if (tradeType == BotTradeClientNormal) {
		// Items being traded are found in the normal trade window used to trade between a Client and a Client or NPC
		// Items in this mode are found in slot ids 3000 thru 3003 - thought bots used the full 8-slot window..?
		PerformTradeWithClient(EQ::invslot::TRADE_BEGIN, EQ::invslot::TRADE_END, client); // {3000..3007}
	}
	else if (tradeType == BotTradeClientNoDropNoTrade) {
		// Items being traded are found on the Client's cursor slot, slot id 30. This item can be either a single item or it can be a bag.
		// If it is a bag, then we have to search for items in slots 331 thru 340
		PerformTradeWithClient(EQ::invslot::slotCursor, EQ::invslot::slotCursor, client);

		// TODO: Add logic here to test if the item in SLOT_CURSOR is a container type, if it is then we need to call the following:
		// PerformTradeWithClient(331, 340, client);
	}
}

// Perfoms the actual trade action with a client bot owner
void Bot::PerformTradeWithClient(int16 beginSlotID, int16 endSlotID, Client* client)
{
	using namespace EQ;

	struct ClientTrade {
		const ItemInstance* tradeItemInstance;
		int16 fromClientSlot;
		int16 toBotSlot;
		int adjustStackSize;
		std::string acceptedItemName;
		
		ClientTrade(const ItemInstance* item, int16 from, const char* name = "") : tradeItemInstance(item), fromClientSlot(from), toBotSlot(invslot::SLOT_INVALID), adjustStackSize(0), acceptedItemName(name) { }
	};

	struct ClientReturn {
		const ItemInstance* returnItemInstance;
		int16 fromBotSlot;
		int16 toClientSlot;
		int adjustStackSize;
		std::string failedItemName;
		
		ClientReturn(const ItemInstance* item, int16 from, const char* name = "") : returnItemInstance(item), fromBotSlot(from), toClientSlot(invslot::SLOT_INVALID), adjustStackSize(0), failedItemName(name) { }
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
		client->Message(Chat::Red, "You are not the owner of this bot - Trade Canceled.");
		client->ResetTrade();
		return;
	}
	if ((beginSlotID != invslot::TRADE_BEGIN) && (beginSlotID != invslot::slotCursor)) {
		client->Message(Chat::Red, "Trade request processing from illegal 'begin' slot - Trade Canceled.");
		client->ResetTrade();
		return;
	}
	if ((endSlotID != invslot::TRADE_END) && (endSlotID != invslot::slotCursor)) {
		client->Message(Chat::Red, "Trade request processing from illegal 'end' slot - Trade Canceled.");
		client->ResetTrade();
		return;
	}
	if (((beginSlotID == invslot::slotCursor) && (endSlotID != invslot::slotCursor)) || ((beginSlotID != invslot::slotCursor) && (endSlotID == invslot::slotCursor))) {
		client->Message(Chat::Red, "Trade request processing illegal slot range - Trade Canceled.");
		client->ResetTrade();
		return;
	}
	if (endSlotID < beginSlotID) {
		client->Message(Chat::Red, "Trade request processing in reverse slot order - Trade Canceled.");
		client->ResetTrade();
		return;
	}
	if (client->IsEngaged() || IsEngaged()) {
		client->Message(Chat::Yellow, "You may not perform a trade while engaged - Trade Canceled!");
		client->ResetTrade();
		return;
	}

	std::list<ClientTrade> client_trade;
	std::list<ClientReturn> client_return;

	// pre-checks for incoming illegal transfers
	for (int16 trade_index = beginSlotID; trade_index <= endSlotID; ++trade_index) {
		auto trade_instance = client->GetInv()[trade_index];
		if (!trade_instance)
			continue;

		if (!trade_instance->GetItem()) {
			// TODO: add logging
			client->Message(Chat::Red, "A server error was encountered while processing client slot %i - Trade Canceled.", trade_index);
			client->ResetTrade();
			return;
		}
		if ((trade_index != invslot::slotCursor) && !trade_instance->IsDroppable()) {
			// TODO: add logging
			client->Message(Chat::Red, "Trade hack detected - Trade Canceled.");
			client->ResetTrade();
			return;
		}
		if (trade_instance->IsStackable() && (trade_instance->GetCharges() < trade_instance->GetItem()->StackSize)) { // temp until partial stacks are implemented
			client->Message(Chat::Yellow, "'%s' is only a partially stacked item - Trade Canceled!", trade_instance->GetItem()->Name);
			client->ResetTrade();
			return;
		}
		if (CheckLoreConflict(trade_instance->GetItem())) {
			client->Message(Chat::Yellow, "This bot already has lore equipment matching the item '%s' - Trade Canceled!", trade_instance->GetItem()->Name);
			client->ResetTrade();
			return;
		}

		if (!trade_instance->IsType(item::ItemClassCommon)) {
			client_return.push_back(ClientReturn(trade_instance, trade_index, trade_instance->GetItem()->Name));
			continue;
		}
		if (!trade_instance->IsEquipable(GetBaseRace(), GetClass()) || (GetLevel() < trade_instance->GetItem()->ReqLevel)) { // deity checks will be handled within IsEquipable()
			client_return.push_back(ClientReturn(trade_instance, trade_index, trade_instance->GetItem()->Name));
			continue;
		}

		client_trade.push_back(ClientTrade(trade_instance, trade_index, trade_instance->GetItem()->Name));
	}

	// check for incoming lore hacks
	for (auto& trade_iterator : client_trade) {
		if (!trade_iterator.tradeItemInstance->GetItem()->LoreFlag)
			continue;

		for (const auto& check_iterator : client_trade) {
			if (check_iterator.fromClientSlot == trade_iterator.fromClientSlot)
				continue;
			if (!check_iterator.tradeItemInstance->GetItem()->LoreFlag)
				continue;
			
			if ((trade_iterator.tradeItemInstance->GetItem()->LoreGroup == -1) && (check_iterator.tradeItemInstance->GetItem()->ID == trade_iterator.tradeItemInstance->GetItem()->ID)) {
				// TODO: add logging
				client->Message(Chat::Red, "Trade hack detected - Trade Canceled.");
				client->ResetTrade();
				return;
			}
			if ((trade_iterator.tradeItemInstance->GetItem()->LoreGroup > 0) && (check_iterator.tradeItemInstance->GetItem()->LoreGroup == trade_iterator.tradeItemInstance->GetItem()->LoreGroup)) {
				// TODO: add logging
				client->Message(Chat::Red, "Trade hack detected - Trade Canceled.");
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
			if (trade_iterator.toBotSlot != invslot::SLOT_INVALID)
				continue;

			auto trade_instance = trade_iterator.tradeItemInstance;
			//if ((stage_loop == stageStackable) && !trade_instance->IsStackable())
			//	continue;

			for (auto index : bot_equip_order) {
				if (!(trade_instance->GetItem()->Slots & (1 << index)))
					continue;

				//if (stage_loop == stageStackable) {
				//	// TODO: implement
				//	continue;
				//}

				if (stage_loop != stageReplaceable) {
					if (m_inv[index])
						continue;
				}

				bool slot_taken = false;
				for (const auto& check_iterator : client_trade) {
					if (check_iterator.fromClientSlot == trade_iterator.fromClientSlot)
						continue;

					if (check_iterator.toBotSlot == index) {
						slot_taken = true;
						break;
					}
				}
				if (slot_taken)
					continue;

				if (index == invslot::slotPrimary) {
					if (trade_instance->GetItem()->IsType2HWeapon()) {
						if (!melee_secondary) {
							melee_2h_weapon = true;

							auto equipped_secondary_weapon = m_inv[invslot::slotSecondary];
							if (equipped_secondary_weapon)
								client_return.push_back(ClientReturn(equipped_secondary_weapon, invslot::slotSecondary));
						}
						else {
							continue;
						}
					}
				}
				if (index == invslot::slotSecondary) {
					if (!melee_2h_weapon) {
						if ((can_dual_wield && trade_instance->GetItem()->IsType1HWeapon()) || trade_instance->GetItem()->IsTypeShield() || !trade_instance->IsWeapon()) {
							melee_secondary = true;

							auto equipped_primary_weapon = m_inv[invslot::slotPrimary];
							if (equipped_primary_weapon && equipped_primary_weapon->GetItem()->IsType2HWeapon())
								client_return.push_back(ClientReturn(equipped_primary_weapon, invslot::slotPrimary));
						}
						else {
							continue;
						}
					}
					else {
						continue;
					}
				}

				trade_iterator.toBotSlot = index;

				if (m_inv[index])
					client_return.push_back(ClientReturn(m_inv[index], index));

				break;
			}
		}
	}

	// move unassignable items from trade list to return list
	for (std::list<ClientTrade>::iterator trade_iterator = client_trade.begin(); trade_iterator != client_trade.end();) {
		if (trade_iterator->toBotSlot == invslot::SLOT_INVALID) {
			client_return.push_back(ClientReturn(trade_iterator->tradeItemInstance, trade_iterator->fromClientSlot, trade_iterator->tradeItemInstance->GetItem()->Name));
			trade_iterator = client_trade.erase(trade_iterator);
			continue;
		}
		++trade_iterator;
	}
	
	// out-going return checks for client
	for (auto& return_iterator : client_return) {
		auto return_instance = return_iterator.returnItemInstance;
		if (!return_instance)
			continue;

		if (!return_instance->GetItem()) {
			// TODO: add logging
			client->Message(Chat::Red, "A server error was encountered while processing bot slot %i - Trade Canceled.", return_iterator.fromBotSlot);
			client->ResetTrade();
			return;
		}
		// non-failing checks above are causing this to trigger (i.e., !ItemClassCommon and !IsEquipable{race, class, min_level})
		// this process is hindered by not having bots use the inventory trade method (TODO: implement bot inventory use)
		if (client->CheckLoreConflict(return_instance->GetItem())) {
			client->Message(Chat::Yellow, "You already have lore equipment matching the item '%s' - Trade Canceled!", return_instance->GetItem()->Name);
			client->ResetTrade();
			return;
		}

		if (return_iterator.fromBotSlot == invslot::slotCursor) {
			return_iterator.toClientSlot = invslot::slotCursor;
		}
		else {
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
					if (check_iterator.fromBotSlot == return_iterator.fromBotSlot)
						continue;

					if ((check_iterator.toClientSlot == client_test_slot) && (client_test_slot != invslot::slotCursor)) {
						slot_taken = true;
						break;
					}
				}
				if (slot_taken) {
					if ((client_test_slot >= invslot::GENERAL_BEGIN) && (client_test_slot <= invslot::GENERAL_END)) {
						++client_search_general;
						client_search_bag = invbag::SLOT_BEGIN;
					}
					else {
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

				return_iterator.toClientSlot = client_test_slot;
				run_search = false;
			}
		}

		if (return_iterator.toClientSlot == invslot::SLOT_INVALID) {
			client->Message(Chat::Yellow, "You do not have room to complete this trade - Trade Canceled!");
			client->ResetTrade();
			return;
		}
	}

	// perform actual trades
	// returns first since clients have trade slots and bots do not
	for (auto& return_iterator : client_return) {
		// TODO: code for stackables

		if (return_iterator.fromBotSlot == invslot::slotCursor) { // failed trade return
			// no movement action required
		}
		else if ((return_iterator.fromBotSlot >= invslot::TRADE_BEGIN) && (return_iterator.fromBotSlot <= invslot::TRADE_END)) { // failed trade returns
			client->PutItemInInventory(return_iterator.toClientSlot, *return_iterator.returnItemInstance);
			client->SendItemPacket(return_iterator.toClientSlot, return_iterator.returnItemInstance, ItemPacketTrade);
			client->DeleteItemInInventory(return_iterator.fromBotSlot);
		}
		else { // successful trade returns
			auto return_instance = m_inv.PopItem(return_iterator.fromBotSlot);
			//if (*return_instance != *return_iterator.returnItemInstance) {
			//	// TODO: add logging
			//}

			if (!database.botdb.DeleteItemBySlot(GetBotID(), return_iterator.fromBotSlot))
				client->Message(Chat::Red, "%s (slot: %i, name: '%s')", BotDatabase::fail::DeleteItemBySlot(), return_iterator.fromBotSlot, (return_instance ? return_instance->GetItem()->Name : "nullptr"));

			BotRemoveEquipItem(return_iterator.fromBotSlot);
			if (return_instance)
				client->PutItemInInventory(return_iterator.toClientSlot, *return_instance, true);
			InventoryProfile::MarkDirty(return_instance);
		}
		return_iterator.returnItemInstance = nullptr;
	}

	// trades can now go in as empty slot inserts
	for (auto& trade_iterator : client_trade) {
		// TODO: code for stackables

		if (!database.botdb.SaveItemBySlot(this, trade_iterator.toBotSlot, trade_iterator.tradeItemInstance))
			client->Message(Chat::Red, "%s (slot: %i, name: '%s')", BotDatabase::fail::SaveItemBySlot(), trade_iterator.toBotSlot, (trade_iterator.tradeItemInstance ? trade_iterator.tradeItemInstance->GetItem()->Name : "nullptr"));

		m_inv.PutItem(trade_iterator.toBotSlot, *trade_iterator.tradeItemInstance);
		this->BotAddEquipItem(trade_iterator.toBotSlot, (trade_iterator.tradeItemInstance ? trade_iterator.tradeItemInstance->GetID() : 0));
		trade_iterator.tradeItemInstance = nullptr; // actual deletion occurs in client delete below

		client->DeleteItemInInventory(trade_iterator.fromClientSlot, 0, (trade_iterator.fromClientSlot == EQ::invslot::slotCursor));

		// database currently has unattuned item saved in inventory..it will be attuned on next bot load
		// this prevents unattuned item returns in the mean time (TODO: re-work process)
		if (trade_iterator.toBotSlot >= invslot::EQUIPMENT_BEGIN && trade_iterator.toBotSlot <= invslot::EQUIPMENT_END) {
			auto attune_item = m_inv.GetItem(trade_iterator.toBotSlot);
			if (attune_item && attune_item->GetItem()->Attuneable)
				attune_item->SetAttuned(true);
		}
	}

	// trade messages
	for (const auto& return_iterator : client_return) {
		if (return_iterator.failedItemName.size())
			client->Message(Chat::Tell, "%s tells you, \"%s, I can't use this '%s.'\"", GetCleanName(), client->GetName(), return_iterator.failedItemName.c_str());
	}
	for (const auto& trade_iterator : client_trade) {
		if (trade_iterator.acceptedItemName.size())
			client->Message(Chat::Tell, "%s tells you, \"Thank you for the '%s,' %s!\"", GetCleanName(), trade_iterator.acceptedItemName.c_str(), client->GetName());
	}

	size_t accepted_count = client_trade.size();
	size_t returned_count = client_return.size();

	client->Message(Chat::Lime, "Trade with '%s' resulted in %i accepted item%s, %i returned item%s.", GetCleanName(), accepted_count, ((accepted_count == 1) ? "" : "s"), returned_count, ((returned_count == 1) ? "" : "s"));

	if (accepted_count)
		CalcBotStats(client->GetBotOption(Client::booStatsUpdate));
}

bool Bot::Death(Mob *killerMob, int32 damage, uint16 spell_id, EQ::skills::SkillType attack_skill) {
	if(!NPC::Death(killerMob, damage, spell_id, attack_skill))
		return false;

	Save();

	Mob *my_owner = GetBotOwner();
	if (my_owner && my_owner->IsClient() && my_owner->CastToClient()->GetBotOption(Client::booDeathMarquee)) {
		if (killerMob)
			my_owner->CastToClient()->SendMarqueeMessage(Chat::Red, 510, 0, 1000, 3000, StringFormat("%s has been slain by %s", GetCleanName(), killerMob->GetCleanName()));
		else
			my_owner->CastToClient()->SendMarqueeMessage(Chat::Red, 510, 0, 1000, 3000, StringFormat("%s has been slain", GetCleanName()));
	}

	Mob *give_exp = hate_list.GetDamageTopOnHateList(this);
	Client *give_exp_client = nullptr;
	if(give_exp && give_exp->IsClient())
		give_exp_client = give_exp->CastToClient();

	bool IsLdonTreasure = (this->GetClass() == LDON_TREASURE);
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

	entity_list.RemoveBot(this->GetID());
	return true;
}

void Bot::Damage(Mob *from, int32 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special) {
	if(spell_id == 0)
		spell_id = SPELL_UNKNOWN;

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if(attacked_timer.Check()) {
		LogCombat("Triggering EVENT_ATTACK due to attack by [{}]", from->GetName());
		parse->EventNPC(EVENT_ATTACK, this, from, "", 0);
	}

	attacked_timer.Start(CombatEventTimer_expire);
	// if spell is lifetap add hp to the caster
	if (spell_id != SPELL_UNKNOWN && IsLifetapSpell(spell_id)) {
		int healed = GetActSpellHealing(spell_id, damage);
		LogCombat("Applying lifetap heal of [{}] to [{}]", healed, GetCleanName());
		HealDamage(healed);
		entity_list.MessageClose(this, true, 300, Chat::Spells, "%s beams a smile at %s", GetCleanName(), from->GetCleanName() );
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

//void Bot::AddToHateList(Mob* other, uint32 hate = 0, int32 damage = 0, bool iYellForHelp = true, bool bFrenzy = false, bool iBuffTic = false)
void Bot::AddToHateList(Mob* other, uint32 hate, int32 damage, bool iYellForHelp, bool bFrenzy, bool iBuffTic, bool pet_command) {
	Mob::AddToHateList(other, hate, damage, iYellForHelp, bFrenzy, iBuffTic, pet_command);
}

bool Bot::Attack(Mob* other, int Hand, bool FromRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts) {	
	if (!other) {
		SetTarget(nullptr);
		LogError("A null Mob object was passed to Bot::Attack for evaluation!");
		return false;
	}

	if ((GetHP() <= 0) || (GetAppearance() == eaDead)) {
		SetTarget(nullptr);
		LogCombat("Attempted to attack [{}] while unconscious or, otherwise, appearing dead", other->GetCleanName());
		return false;
	}

	//if(!GetTarget() || GetTarget() != other) // NPC::Attack() doesn't do this
	//	SetTarget(other);

	// apparently, we always want our target to be 'other'..why not just set it?
	SetTarget(other);
	// takes more to compare a call result, load for a call, load a compare to address and compare, and finally
	// push a value to an address than to just load for a call and push a value to an address.
	
	LogCombat("Attacking [{}] with hand [{}] [{}]", other->GetCleanName(), Hand, (FromRiposte ? "(this is a riposte)" : ""));
	if ((IsCasting() && (GetClass() != BARD) && !IsFromSpell) || (!IsAttackAllowed(other))) {
		if(this->GetOwnerID())
			entity_list.MessageClose(this, 1, 200, 10, "%s says, '%s is not a legal target master.'", this->GetCleanName(), this->GetTarget()->GetCleanName());

		if(other) {
			RemoveFromHateList(other);
			LogCombat("I am not allowed to attack [{}]", other->GetCleanName());
		}
		return false;
	}

	if(DivineAura()) {//cant attack while invulnerable
		LogCombat("Attack canceled, Divine Aura is in effect");
		return false;
	}

	FaceTarget(GetTarget());
	EQ::ItemInstance* weapon = nullptr;
	if (Hand == EQ::invslot::slotPrimary) {
		weapon = GetBotItem(EQ::invslot::slotPrimary);
		OffHandAtk(false);
	}

	if (Hand == EQ::invslot::slotSecondary) {
		weapon = GetBotItem(EQ::invslot::slotSecondary);
		OffHandAtk(true);
	}

	if(weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			LogCombat("Attack canceled, Item [{}] ([{}]) is not a weapon", weapon->GetItem()->Name, weapon->GetID());
			return false;
		}
		LogCombat("Attacking with weapon: [{}] ([{}])", weapon->GetItem()->Name, weapon->GetID());
	}
	else
		LogCombat("Attacking without a weapon");

	// calculate attack_skill and skillinuse depending on hand and weapon
	// also send Packet to near clients
	DamageHitInfo my_hit;
	my_hit.skill = AttackAnimation(Hand, weapon);
	LogCombat("Attacking with [{}] in slot [{}] using skill [{}]", weapon?weapon->GetItem()->Name:"Fist", Hand, my_hit.skill);

	// Now figure out damage
	my_hit.damage_done = 1;
	my_hit.min_damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon)
		hate = (weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt);

	my_hit.base_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && my_hit.base_damage > 1)
		hate = my_hit.base_damage;

	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if (my_hit.base_damage > 0) {
		my_hit.min_damage = 0;

		// ***************************************************************
		// *** Calculate the damage bonus, if applicable, for this hit ***
		// ***************************************************************

#ifndef EQEMU_NO_WEAPON_DAMAGE_BONUS

		// If you include the preprocessor directive "#define EQEMU_NO_WEAPON_DAMAGE_BONUS", that indicates that you do not
		// want damage bonuses added to weapon damage at all. This feature was requested by ChaosSlayer on the EQEmu Forums.
		//
		// This is not recommended for normal usage, as the damage bonus represents a non-trivial component of the DPS output
		// of weapons wielded by higher-level melee characters (especially for two-handed weapons).
		int ucDamageBonus = 0;
		if (Hand == EQ::invslot::slotPrimary && GetLevel() >= 28 && IsWarriorClass()) {
			// Damage bonuses apply only to hits from the main hand (Hand == MainPrimary) by characters level 28 and above
			// who belong to a melee class. If we're here, then all of these conditions apply.
			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQ::ItemData*) nullptr);
			my_hit.min_damage = ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		//Live AA - Sinister Strikes *Adds weapon damage bonus to offhand weapon.
		if (Hand == EQ::invslot::slotSecondary) {
			if (aabonuses.SecondaryDmgInc || itembonuses.SecondaryDmgInc || spellbonuses.SecondaryDmgInc){
				ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQ::ItemData*) nullptr);
				my_hit.min_damage = ucDamageBonus;
				hate += ucDamageBonus;
			}
		}

		LogCombat("Damage calculated: base [{}] min damage [{}] skill [{}]", my_hit.base_damage, my_hit.min_damage, my_hit.skill);

		int hit_chance_bonus = 0;
		my_hit.offense = offense(my_hit.skill);
		my_hit.hand = Hand;

		if (opts) {
			my_hit.base_damage *= opts->damage_percent;
			my_hit.base_damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
			hit_chance_bonus += opts->hit_chance;
		}

		my_hit.tohit = GetTotalToHit(my_hit.skill, hit_chance_bonus);

		DoAttack(other, my_hit, opts);

		LogCombat("Final damage after all reductions: [{}]", my_hit.damage_done);
	} else {
		my_hit.damage_done = DMG_INVULNERABLE;
	}

	// Hate Generation is on a per swing basis, regardless of a hit, miss, or block, its always the same.
	// If we are this far, this means we are atleast making a swing.
	other->AddToHateList(this, hate);

	///////////////////////////////////////////////////////////
	////// Send Attack Damage
	///////////////////////////////////////////////////////////
	other->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill);

	if (GetHP() < 0)
		return false;

	MeleeLifeTap(my_hit.damage_done);

	if (my_hit.damage_done > 0)
		CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);

	CommonBreakInvisibleFromCombat();
	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if(GetTarget())
		TriggerDefensiveProcs(other, Hand, true, my_hit.damage_done);

	if (my_hit.damage_done > 0)
		return true;
	else
		return false;
}

int32 Bot::CalcBotAAFocus(focusType type, uint32 aa_ID, uint32 points, uint16 spell_id)
{
	const SPDat_Spell_Struct &spell = spells[spell_id];
	int32 value = 0;
	int lvlModifier = 100;
	int spell_level = 0;
	int lvldiff = 0;
	bool LimitSpellSkill = false;
	bool SpellSkill_Found = false;
	uint32 effect = 0;
	int32 base1 = 0;
	int32 base2 = 0;
	uint32 slot = 0;
	bool LimitFound = false;
	int FocusCount = 0;

	auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa_ID, points);
	auto ability = ability_rank.first;
	auto rank = ability_rank.second;

	if(!ability) {
		return 0;
	}

	for(auto &eff : rank->effects) {
		effect = eff.effect_id;
		base1 = eff.base1;
		base2 = eff.base2;
		slot = eff.slot;

		//AA Foci's can contain multiple focus effects within the same AA.
		//To handle this we will not automatically return zero if a limit is found.
		//Instead if limit is found and multiple effects, we will reset the limit check
		//when the next valid focus effect is found.
		if (IsFocusEffect(0, 0, true,effect) || (effect == SE_TriggerOnCast)) {
			FocusCount++;
			//If limit found on prior check next, else end loop.
			if (FocusCount > 1) {
				if (LimitFound) {
					value = 0;
					LimitFound = false;
				}
				else
					break;
			}
		}


		switch (effect) {
			case SE_Blank:
				break;
			case SE_LimitResist:
				if(base1) {
					if(spell.resisttype != base1)
						LimitFound = true;
				}
				break;
			case SE_LimitInstant:
				if(spell.buffduration)
					LimitFound = true;
				break;
			case SE_LimitMaxLevel:
				spell_level = spell.classes[(GetClass() % 17) - 1];
				lvldiff = spell_level - base1;
				//every level over cap reduces the effect by base2 percent unless from a clicky when ItemCastsUseFocus is true
				if(lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) || RuleB(Character, ItemCastsUseFocus) == false)) {
					if(base2 > 0) {
						lvlModifier -= (base2 * lvldiff);
						if(lvlModifier < 1)
							LimitFound = true;
					}
					else
						LimitFound = true;
				}
				break;
			case SE_LimitMinLevel:
				if((spell.classes[(GetClass() % 17) - 1]) < base1)
					LimitFound = true;
				break;
			case SE_LimitCastTimeMin:
				if (spell.cast_time < base1)
					LimitFound = true;
				break;
			case SE_LimitSpell:
				if(base1 < 0) {
					if (spell_id == (base1*-1))
						LimitFound = true;
				} else {
					if (spell_id != base1)
						LimitFound = true;
				}
				break;
			case SE_LimitMinDur:
				if (base1 > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
					LimitFound = true;
				break;
			case SE_LimitEffect:
				if(base1 < 0) {
					if(IsEffectInSpell(spell_id,(base1*-1)))
						LimitFound = true;
				} else {
					if(!IsEffectInSpell(spell_id,base1))
						LimitFound = true;
				}
				break;
			case SE_LimitSpellType:
				switch(base1) {
					case 0:
						if (!IsDetrimentalSpell(spell_id))
							LimitFound = true;
						break;
					case 1:
						if (!IsBeneficialSpell(spell_id))
							LimitFound = true;
						break;
				}
				break;

			case SE_LimitManaMin:
				if(spell.mana < base1)
					LimitFound = true;
				break;
			case SE_LimitTarget:
				if(base1 < 0) {
					if(-base1 == spell.targettype)
						LimitFound = true;
				} else {
					if(base1 != spell.targettype)
						LimitFound = true;
				}
				break;
			case SE_LimitCombatSkills:
				if((base1 == 1 && !IsDiscipline(spell_id)) || (base1 == 0 && IsDiscipline(spell_id)))
					LimitFound = true;
			break;
			case SE_LimitSpellGroup:
				if((base1 > 0 && base1 != spell.spellgroup) || (base1 < 0 && base1 == spell.spellgroup))
					LimitFound = true;
				break;
			case SE_LimitCastingSkill:
				LimitSpellSkill = true;
				if(base1 == spell.skill)
					SpellSkill_Found = true;
				break;
			case SE_LimitClass:
			//Do not use this limit more then once per spell. If multiple class, treat value like items would.
				if (!PassLimitClass(base1, GetClass()))
					LimitFound = true;
				break;
			//Handle Focus Effects
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
					if(value != 0) {
						if(value > 0) {
							if(base1 > value)
								value = base1;
						} else {
							if(base1 < value)
								value = base1;
						}
					}
					else
						value = base1;
				}
				break;

			case SE_ReduceReuseTimer: {
				if(type == focusReduceRecastTime)
					value = (base1 / 1000);
				break;
			}
			case SE_TriggerOnCast: {
				if(type == focusTriggerOnCast) {
					if(zone->random.Int(0, 100) <= base1)
						value = base2;
					else {
						value = 0;
						LimitFound = true;
					}
				}
				break;
			}
			case SE_FcSpellVulnerability: {
				if(type == focusSpellVulnerability)
					value = base1;
				break;
			}
			case SE_BlockNextSpellFocus: {
				if(type == focusBlockNextSpell) {
					if(zone->random.Int(1, 100) <= base1)
						value = 1;
				}
				break;
			}
			case SE_FcTwincast: {
				if(type == focusTwincast)
					value = base1;
				break;
			}
			//case SE_SympatheticProc:
			//{
			//	if(type == focusSympatheticProc)
			//	{
			//		float ProcChance, ProcBonus;
			//		int16 ProcRateMod = base1; //Baseline is 100 for most Sympathetic foci
			//		int32 cast_time = GetActSpellCasttime(spell_id, spells[spell_id].cast_time);
			//		GetSympatheticProcChances(ProcBonus, ProcChance, cast_time, ProcRateMod);

			//		if(zone->random.Real(0, 1) <= ProcChance)
			//			value = focus_id;

			//		else
			//			value = 0;
			//	}
			//	break;
			//}
			case SE_FcDamageAmt: {
				if(type == focusFcDamageAmt)
					value = base1;
				break;
			}
			case SE_FcDamageAmt2: {
				if(type == focusFcDamageAmt2)
					value = base1;
				break;
			}
			case SE_FcDamageAmtCrit: {
				if(type == focusFcDamageAmtCrit)
					value = base1;
				break;
			}
			case SE_FcDamageAmtIncoming: {
				if(type == focusFcDamageAmtIncoming)
					value = base1;
				break;
			}
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
			case SE_FcBaseEffects: {
				if (type == focusFcBaseEffects)
					value = base1;
				break;
			}
			case SE_FcDamagePctCrit: {
				if(type == focusFcDamagePctCrit)
					value = base1;
				break;
			}
			case SE_FcIncreaseNumHits: {
				if(type == focusIncreaseNumHits)
					value = base1;
				break;
			}

	//Check for spell skill limits.
			if ((LimitSpellSkill) && (!SpellSkill_Found))
				return 0;
		}
	}

	if (LimitFound)
		return 0;

	return (value * lvlModifier / 100);
}

int32 Bot::GetBotFocusEffect(focusType bottype, uint16 spell_id) {
	if (IsBardSong(spell_id) && bottype != focusFcBaseEffects)
		return 0;

	int32 realTotal = 0;
	int32 realTotal2 = 0;
	int32 realTotal3 = 0;
	bool rand_effectiveness = false;
	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if(RuleB(Spells, LiveLikeFocusEffects) && (bottype == focusManaCost || bottype == focusImprovedHeal || bottype == focusImprovedDamage || bottype == focusImprovedDamage2 || bottype == focusResistRate))
		rand_effectiveness = true;

	//Check if item focus effect exists for the client.
	if (itembonuses.FocusEffects[bottype]) {
		const EQ::ItemData* TempItem = nullptr;
		const EQ::ItemData* UsedItem = nullptr;
		const EQ::ItemInstance* TempInst = nullptr;
		uint16 UsedFocusID = 0;
		int32 Total = 0;
		int32 focus_max = 0;
		int32 focus_max_real = 0;
		//item focus
		// are focus effects the same as bonus? (slotAmmo-excluded)
		for (int x = EQ::invslot::EQUIPMENT_BEGIN; x <= EQ::invslot::EQUIPMENT_END; x++) {
			TempItem = nullptr;
			EQ::ItemInstance* ins = GetBotItem(x);
			if (!ins)
				continue;

			TempItem = ins->GetItem();
			if (TempItem && TempItem->Focus.Effect > 0 && TempItem->Focus.Effect != SPELL_UNKNOWN) {
				if(rand_effectiveness) {
					focus_max = CalcBotFocusEffect(bottype, TempItem->Focus.Effect, spell_id, true);
					if ((focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) || (focus_max < 0 && focus_max < focus_max_real)) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				} else {
					Total = CalcBotFocusEffect(bottype, TempItem->Focus.Effect, spell_id);
					if ((Total > 0 && realTotal >= 0 && Total > realTotal) || (Total < 0 && Total < realTotal)) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
			}

			for (int y = EQ::invaug::SOCKET_BEGIN; y <= EQ::invaug::SOCKET_END; ++y) {
				EQ::ItemInstance *aug = nullptr;
				aug = ins->GetAugment(y);
				if(aug) {
					const EQ::ItemData* TempItemAug = aug->GetItem();
					if (TempItemAug && TempItemAug->Focus.Effect > 0 && TempItemAug->Focus.Effect != SPELL_UNKNOWN) {
						if(rand_effectiveness) {
							focus_max = CalcBotFocusEffect(bottype, TempItemAug->Focus.Effect, spell_id, true);
							if ((focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) || (focus_max < 0 && focus_max < focus_max_real)) {
								focus_max_real = focus_max;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							}
						} else {
							Total = CalcBotFocusEffect(bottype, TempItemAug->Focus.Effect, spell_id);
							if ((Total > 0 && realTotal >= 0 && Total > realTotal) || (Total < 0 && Total < realTotal)) {
								realTotal = Total;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							}
						}
					}
				}
			}
		}

		if(UsedItem && rand_effectiveness && focus_max_real != 0)
			realTotal = CalcBotFocusEffect(bottype, UsedFocusID, spell_id);
	}

	//Check if spell focus effect exists for the client.
	if (spellbonuses.FocusEffects[bottype]) {
		//Spell Focus
		int32 Total2 = 0;
		int32 focus_max2 = 0;
		int32 focus_max_real2 = 0;
		int buff_tracker = -1;
		int buff_slot = 0;
		uint32 focusspellid = 0;
		uint32 focusspell_tracker = 0;
		uint32 buff_max = GetMaxTotalSlots();
		for (buff_slot = 0; buff_slot < buff_max; buff_slot++) {
			focusspellid = buffs[buff_slot].spellid;
			if (focusspellid == 0 || focusspellid >= SPDAT_RECORDS)
				continue;

			if(rand_effectiveness) {
				focus_max2 = CalcBotFocusEffect(bottype, focusspellid, spell_id, true);
				if ((focus_max2 > 0 && focus_max_real2 >= 0 && focus_max2 > focus_max_real2) || (focus_max2 < 0 && focus_max2 < focus_max_real2)) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			} else {
				Total2 = CalcBotFocusEffect(bottype, focusspellid, spell_id);
				if ((Total2 > 0 && realTotal2 >= 0 && Total2 > realTotal2) || (Total2 < 0 && Total2 < realTotal2)) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
		}

		if(focusspell_tracker && rand_effectiveness && focus_max_real2 != 0)
			realTotal2 = CalcBotFocusEffect(bottype, focusspell_tracker, spell_id);

		// For effects like gift of mana that only fire once, save the spellid into an array that consists of all available buff slots.
		if(buff_tracker >= 0 && buffs[buff_tracker].numhits > 0)
			m_spellHitsLeft[buff_tracker] = focusspell_tracker;
	}

	// AA Focus
	if (aabonuses.FocusEffects[bottype]) {
		int32 Total3 = 0;
		uint32 slots = 0;
		uint32 aa_AA = 0;
		uint32 aa_value = 0;

		for(auto &aa : aa_ranks) {
			auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
			auto ability = ability_rank.first;
			auto rank = ability_rank.second;

			if(!ability) {
				continue;
			}

			aa_AA = ability->id;
			aa_value = aa.second.first;
			if (aa_AA < 1 || aa_value < 1)
				continue;

			Total3 = CalcBotAAFocus(bottype, aa_AA, aa_value, spell_id);
			if (Total3 > 0 && realTotal3 >= 0 && Total3 > realTotal3) {
				realTotal3 = Total3;
			}
			else if (Total3 < 0 && Total3 < realTotal3) {
				realTotal3 = Total3;
			}
		}
	}

	if(bottype == focusReagentCost && IsSummonPetSpell(spell_id) && GetAA(aaElementalPact))
		return 100;

	if(bottype == focusReagentCost && (IsEffectInSpell(spell_id, SE_SummonItem) || IsSacrificeSpell(spell_id)))
		return 0;

	return (realTotal + realTotal2);
}

int32 Bot::CalcBotFocusEffect(focusType bottype, uint16 focus_id, uint16 spell_id, bool best_focus) {
	if(!IsValidSpell(focus_id) || !IsValidSpell(spell_id))
		return 0;

	const SPDat_Spell_Struct &focus_spell = spells[focus_id];
	const SPDat_Spell_Struct &spell = spells[spell_id];
	int32 value = 0;
	int lvlModifier = 100;
	int spell_level = 0;
	int lvldiff = 0;
	bool LimitSpellSkill = false;
	bool SpellSkill_Found = false;
	for (int i = 0; i < EFFECT_COUNT; i++) {
		switch (focus_spell.effectid[i]) {
			case SE_Blank:
				break;
			case SE_LimitResist:{
				if(focus_spell.base[i]) {
					if(spell.resisttype != focus_spell.base[i])
						return 0;
				}
				break;
			}
			case SE_LimitInstant: {
				if(spell.buffduration)
					return 0;
				break;
			}
			case SE_LimitMaxLevel:{
				if (IsNPC())
					break;
				spell_level = spell.classes[(GetClass() % 17) - 1];
				lvldiff = (spell_level - focus_spell.base[i]);
				if(lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) || RuleB(Character, ItemCastsUseFocus) == false)) {
					if(focus_spell.base2[i] > 0) {
						lvlModifier -= (focus_spell.base2[i] * lvldiff);
						if(lvlModifier < 1)
							return 0;
					}
					else
						return 0;
				}
				break;
			}
			case SE_LimitMinLevel:
				if (IsNPC())
					break;
				if (spell.classes[(GetClass() % 17) - 1] < focus_spell.base[i])
					return 0;
				break;

			case SE_LimitCastTimeMin:
				if (spells[spell_id].cast_time < (uint32)focus_spell.base[i])
					return 0;
				break;
			case SE_LimitSpell:
				if(focus_spell.base[i] < 0) {
					if (spell_id == (focus_spell.base[i] * -1))
						return 0;
				} else {
					if (spell_id != focus_spell.base[i])
						return 0;
				}
				break;
			case SE_LimitMinDur:
				if (focus_spell.base[i] > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
					return 0;
				break;
			case SE_LimitEffect:
				if(focus_spell.base[i] < 0) {
					if(IsEffectInSpell(spell_id,focus_spell.base[i]))
						return 0;
				} else {
					if(focus_spell.base[i] == SE_SummonPet) {
						if(!IsEffectInSpell(spell_id, SE_SummonPet) && !IsEffectInSpell(spell_id, SE_NecPet) && !IsEffectInSpell(spell_id, SE_SummonBSTPet)) {
							return 0;
						}
					} else if(!IsEffectInSpell(spell_id,focus_spell.base[i]))
						return 0;
				}
				break;


			case SE_LimitSpellType:
				switch(focus_spell.base[i]) {
					case 0:
						if (!IsDetrimentalSpell(spell_id))
							return 0;
						break;
					case 1:
						if (!IsBeneficialSpell(spell_id))
							return 0;
						break;
					default:
						LogInfo("CalcFocusEffect: unknown limit spelltype [{}]", focus_spell.base[i]);
				}
				break;

			case SE_LimitManaMin:
				if(spell.mana < focus_spell.base[i])
					return 0;
				break;
			case SE_LimitTarget:
				if((focus_spell.base[i] < 0) && -focus_spell.base[i] == spell.targettype)
					return 0;
				else if (focus_spell.base[i] > 0 && focus_spell.base[i] != spell.targettype)
					return 0;
				break;
			case SE_LimitCombatSkills:
				if(focus_spell.base[i] == 1 && !IsDiscipline(spell_id))
					return 0;
				else if(focus_spell.base[i] == 0 && IsDiscipline(spell_id))
					return 0;
				break;
			case SE_LimitSpellGroup:
				if(focus_spell.base[i] > 0 && focus_spell.base[i] != spell.spellgroup)
					return 0;
				else if(focus_spell.base[i] < 0 && focus_spell.base[i] == spell.spellgroup)
					return 0;
				break;
			case SE_LimitCastingSkill:
				LimitSpellSkill = true;
				if(focus_spell.base[i] == spell.skill)
					SpellSkill_Found = true;
				break;
			case SE_LimitClass:
				if (!PassLimitClass(focus_spell.base[i], GetClass()))
					return 0;
				break;
			case SE_ImprovedDamage:
				if (bottype == focusImprovedDamage) {
					if(best_focus) {
						if (focus_spell.base2[i] != 0)
							value = focus_spell.base2[i];
						else
							value = focus_spell.base[i];
					}
					else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i])
						value = focus_spell.base[i];
					else
						value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
				break;
			case SE_ImprovedDamage2:
				if (bottype == focusImprovedDamage2) {
					if(best_focus) {
						if (focus_spell.base2[i] != 0)
							value = focus_spell.base2[i];
						else
							value = focus_spell.base[i];
					}
					else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i])
						value = focus_spell.base[i];
					else
						value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
				break;
			case SE_ImprovedHeal:
				if (bottype == focusImprovedHeal) {
					if(best_focus) {
						if (focus_spell.base2[i] != 0)
							value = focus_spell.base2[i];
						else
							value = focus_spell.base[i];
					}
					else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i])
						value = focus_spell.base[i];
					else
						value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
				break;
			case SE_ReduceManaCost:
				if (bottype == focusManaCost) {
					if(best_focus) {
						if (focus_spell.base2[i] != 0)
							value = focus_spell.base2[i];
						else
							value = focus_spell.base[i];
					}
					else if (focus_spell.base2[i] == 0 || focus_spell.base[i] == focus_spell.base2[i])
						value = focus_spell.base[i];
					else
						value = zone->random.Int(focus_spell.base[i], focus_spell.base2[i]);
				}
				break;
			case SE_IncreaseSpellHaste:
				if (bottype == focusSpellHaste && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_IncreaseSpellDuration:
				if (bottype == focusSpellDuration && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SpellDurationIncByTic:
				if (bottype == focusSpellDurByTic && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SwarmPetDuration:
				if (bottype == focusSwarmPetDuration && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_IncreaseRange:
				if (bottype == focusRange && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_ReduceReagentCost:
				if (bottype == focusReagentCost && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_PetPowerIncrease:
				if (bottype == focusPetPower && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SpellResistReduction:
				if (bottype == focusResistRate && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SpellHateMod:
				if (bottype == focusSpellHateMod) {
					if(value != 0) {
						if(value > 0) {
							if(focus_spell.base[i] > value)
								value = focus_spell.base[i];
						}
						else {
							if(focus_spell.base[i] < value)
								value = focus_spell.base[i];
						}
					} else
						value = focus_spell.base[i];
				}
				break;
			case SE_ReduceReuseTimer: {
				if(bottype == focusReduceRecastTime)
					value = (focus_spell.base[i] / 1000);
				break;
			}
			case SE_TriggerOnCast: {
				if(bottype == focusTriggerOnCast) {
					if(zone->random.Int(0, 100) <= focus_spell.base[i])
						value = focus_spell.base2[i];
					else
						value = 0;
				}
				break;
			}
			case SE_FcSpellVulnerability: {
				if(bottype == focusSpellVulnerability)
					value = focus_spell.base[i];
				break;
			}
			case SE_BlockNextSpellFocus: {
				if(bottype == focusBlockNextSpell) {
					if(zone->random.Int(1, 100) <= focus_spell.base[i])
						value = 1;
				}
				break;
			}
			case SE_FcTwincast: {
				if(bottype == focusTwincast)
					value = focus_spell.base[i];
				break;
			}
			case SE_SympatheticProc: {
				if(bottype == focusSympatheticProc) {
					float ProcChance = GetSympatheticProcChances(spell_id, focus_spell.base[i]);
					if(zone->random.Real(0, 1) <= ProcChance)
						value = focus_id;
					else
						value = 0;
				}
				break;
			}
			case SE_FcDamageAmt: {
				if(bottype == focusFcDamageAmt)
					value = focus_spell.base[i];
				break;
			}
			case SE_FcDamageAmt2: {
				if(bottype == focusFcDamageAmt2)
					value = focus_spell.base[i];
				break;
			}
			case SE_FcDamageAmtCrit: {
				if(bottype == focusFcDamageAmtCrit)
					value = focus_spell.base[i];
				break;
			}
			case SE_FcHealAmtIncoming:
				if(bottype == focusFcHealAmtIncoming)
					value = focus_spell.base[i];
				break;
			case SE_FcHealPctCritIncoming:
				if (bottype == focusFcHealPctCritIncoming)
					value = focus_spell.base[i];
				break;
			case SE_FcHealAmtCrit:
				if(bottype == focusFcHealAmtCrit)
					value = focus_spell.base[i];
				break;
			case  SE_FcHealAmt:
				if(bottype == focusFcHealAmt)
					value = focus_spell.base[i];
				break;
			case SE_FcHealPctIncoming:
				if(bottype == focusFcHealPctIncoming)
					value = focus_spell.base[i];
				break;
			case SE_FcBaseEffects: {
				if (bottype == focusFcBaseEffects)
					value = focus_spell.base[i];

				break;
			}
			case SE_FcDamagePctCrit: {
				if(bottype == focusFcDamagePctCrit)
					value = focus_spell.base[i];

				break;
			}
			case SE_FcIncreaseNumHits: {
				if(bottype == focusIncreaseNumHits)
					value = focus_spell.base[i];

				break;
			}
			default:
				LogSpells("CalcFocusEffect: unknown effectid [{}]", focus_spell.effectid[i]);
				break;
		}
	}
	//Check for spell skill limits.
	if ((LimitSpellSkill) && (!SpellSkill_Found))
		return 0;

	return(value * lvlModifier / 100);
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

bool Bot::TryFinishingBlow(Mob *defender, int &damage)
{
	if (!defender)
		return false;

	if (aabonuses.FinishingBlow[1] && !defender->IsClient() && defender->GetHPRatio() < 10) {
		int chance = aabonuses.FinishingBlow[0];
		int fb_damage = aabonuses.FinishingBlow[1];
		int levelreq = aabonuses.FinishingBlowLvl[0];
		if (defender->GetLevel() <= levelreq && (chance >= zone->random.Int(1, 1000))) {
			LogCombat("Landed a finishing blow: levelreq at [{}], other level [{}]",
				levelreq, defender->GetLevel());
			entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, FINISHING_BLOW, GetName());
			damage = fb_damage;
			return true;
		} else {
			LogCombat("failed a finishing blow: levelreq at [{}], other level [{}]",
				levelreq, defender->GetLevel());
			return false;
		}
	}
	return false;
}

void Bot::DoRiposte(Mob* defender) {
	LogCombat("Preforming a riposte");
	if (!defender)
		return;

	defender->Attack(this, EQ::invslot::slotPrimary, true);
	int32 DoubleRipChance = (defender->GetAABonuses().GiveDoubleRiposte[0] + defender->GetSpellBonuses().GiveDoubleRiposte[0] + defender->GetItemBonuses().GiveDoubleRiposte[0]);
	if(DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		LogCombat("Preforming a double riposte ([{}] percent chance)", DoubleRipChance);
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

	uint32 hate = 0;

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

	if(ka_time){
		
		switch(GetClass()){
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
			BotGroupSay(this, "Taunting %s", target->GetCleanName());
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
	switch(GetClass()) {
		case WARRIOR:
			if(level >= RuleI(Combat, NPCBashKickLevel)){
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
			if(level >= RuleI(Combat, NPCBashKickLevel)){
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

	int dmg = GetBaseSkillDamage(static_cast<EQ::skills::SkillType>(skill_to_use), GetTarget());

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

int32 Bot::CheckAggroAmount(uint16 spellid) {
	int32 AggroAmount = Mob::CheckAggroAmount(spellid, nullptr);
	int32 focusAggro = GetBotFocusEffect(focusSpellHateMod, spellid);
	AggroAmount = (AggroAmount * (100 + focusAggro) / 100);
	return AggroAmount;
}

int32 Bot::CheckHealAggroAmount(uint16 spellid, Mob *target, uint32 heal_possible) {
	int32 AggroAmount = Mob::CheckHealAggroAmount(spellid, target, heal_possible);
	int32 focusAggro = GetBotFocusEffect(focusSpellHateMod, spellid);
	AggroAmount = (AggroAmount * (100 + focusAggro) / 100);
	return AggroAmount;
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
	if(this->GetBotOwner())
		Result = GetBotOwner();
	else
		Result = this;

	return Result;
}

Mob* Bot::GetOwner() {
	Mob* Result = nullptr;
	Result = GetBotOwner();
	if(!Result)
		this->SetBotOwner(0);

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

void Bot::EquipBot(std::string* errorMessage) {
	GetBotItems(m_inv, errorMessage);
	const EQ::ItemInstance* inst = nullptr;
	const EQ::ItemData* item = nullptr;
	for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; ++i) {
		inst = GetBotItem(i);
		if(inst) {
			item = inst->GetItem();
			BotTradeAddItem(inst->GetID(), inst, inst->GetCharges(), item->Slots, i, errorMessage, false);
			if(!errorMessage->empty())
				return;
		}
	}
	UpdateEquipmentLight();
}

void Bot::BotOrderCampAll(Client* c) {
	if(c) {
		std::list<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());
		for(std::list<Bot*>::iterator botListItr = BotList.begin(); botListItr != BotList.end(); ++botListItr)
			(*botListItr)->Camp();
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

void Bot::ProcessGuildInvite(Client* guildOfficer, Bot* botToGuild) {
	if(guildOfficer && botToGuild) {
		if(!botToGuild->IsInAGuild()) {
			if (!guild_mgr.CheckPermission(guildOfficer->GuildID(), guildOfficer->GuildRank(), GUILD_INVITE)) {
				guildOfficer->Message(Chat::Red, "You dont have permission to invite.");
				return;
			}

			if (!database.botdb.SaveGuildMembership(botToGuild->GetBotID(), guildOfficer->GuildID(), GUILD_MEMBER)) {
				guildOfficer->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::SaveGuildMembership(), botToGuild->GetCleanName());
				return;
			}
			
			ServerPacket* pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
			ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
			s->guild_id = guildOfficer->GuildID();
			s->old_guild_id = GUILD_NONE;
			s->char_id = botToGuild->GetBotID();
			worldserver.SendPacket(pack);

			safe_delete(pack);
		} else {
			guildOfficer->Message(Chat::Red, "Bot is in a guild.");
			return;
		}
	}
}

bool Bot::ProcessGuildRemoval(Client* guildOfficer, std::string botName) {
	bool Result = false;
	if(guildOfficer && !botName.empty()) {
		Bot* botToUnGuild = entity_list.GetBotByBotName(botName);
		if(botToUnGuild) {
			if (database.botdb.SaveGuildMembership(botToUnGuild->GetBotID(), 0, 0))
				Result = true;
		} else {
			uint32 ownerId = 0;
			if (!database.botdb.LoadOwnerID(botName, ownerId))
				guildOfficer->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::LoadOwnerID(), botName.c_str());
			uint32 botId = 0;
			if (!database.botdb.LoadBotID(ownerId, botName, botId))
				guildOfficer->Message(Chat::Red, "%s for '%s'", BotDatabase::fail::LoadBotID(), botName.c_str());
			if (botId && database.botdb.SaveGuildMembership(botId, 0, 0))
				Result = true;
		}

		if(Result) {
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildManageRemove, sizeof(GuildManageRemove_Struct));
			GuildManageRemove_Struct* gm = (GuildManageRemove_Struct*) outapp->pBuffer;
			gm->guildeqid = guildOfficer->GuildID();
			strcpy(gm->member, botName.c_str());
			guildOfficer->Message(Chat::White, "%s successfully removed from your guild.", botName.c_str());
			entity_list.QueueClientsGuild(guildOfficer, outapp, false, gm->guildeqid);
			safe_delete(outapp);
		}
	}
	return Result;
}

int32 Bot::CalcMaxMana() {
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

int32 Bot::GetActSpellDamage(uint16 spell_id, int32 value, Mob* target) {
	if (spells[spell_id].targettype == ST_Self)
		return value;

	bool Critical = false;
	int32 value_BaseEffect = 0;
	value_BaseEffect = (value + (value*GetBotFocusEffect(focusFcBaseEffects, spell_id) / 100));
	// Need to scale HT damage differently after level 40! It no longer scales by the constant value in the spell file. It scales differently, instead of 10 more damage per level, it does 30 more damage per level. So we multiply the level minus 40 times 20 if they are over level 40.
	if ( (spell_id == SPELL_HARM_TOUCH || spell_id == SPELL_HARM_TOUCH2 || spell_id == SPELL_IMP_HARM_TOUCH ) && GetLevel() > 40)
		value -= ((GetLevel() - 40) * 20);

	//This adds the extra damage from the AA Unholy Touch, 450 per level to the AA Improved Harm TOuch.
	if (spell_id == SPELL_IMP_HARM_TOUCH) //Improved Harm Touch
		value -= (GetAA(aaUnholyTouch) * 450); //Unholy Touch

	int chance = RuleI(Spells, BaseCritChance);
		chance += (itembonuses.CriticalSpellChance + spellbonuses.CriticalSpellChance + aabonuses.CriticalSpellChance);

	if (chance > 0) {
		int32 ratio = RuleI(Spells, BaseCritRatio);
		if (spell_id == SPELL_IMP_HARM_TOUCH && (GetAA(aaSpellCastingFury) > 0) && (GetAA(aaUnholyTouch) > 0))
			chance = 100;

		if (zone->random.Int(1, 100) <= chance){
			Critical = true;
			ratio += (itembonuses.SpellCritDmgIncrease + spellbonuses.SpellCritDmgIncrease + aabonuses.SpellCritDmgIncrease);
			ratio += (itembonuses.SpellCritDmgIncNoStack + spellbonuses.SpellCritDmgIncNoStack + aabonuses.SpellCritDmgIncNoStack);
		} else if (GetClass() == WIZARD && (GetLevel() >= RuleI(Spells, WizCritLevel)) && (zone->random.Int(1, 100) <= RuleI(Spells, WizCritChance))) {
			ratio = zone->random.Int(1, 100);
			Critical = true;
		}
		ratio += RuleI(Spells, WizCritRatio);
		if (Critical) {
			value = (value_BaseEffect * ratio / 100);
			value += (value_BaseEffect * GetBotFocusEffect(focusImprovedDamage, spell_id) / 100);
			value += (value_BaseEffect * GetBotFocusEffect(focusImprovedDamage2, spell_id) / 100);
			value += (int(value_BaseEffect * GetBotFocusEffect(focusFcDamagePctCrit, spell_id) / 100) * ratio / 100);
			if (target) {
				value += (int(value_BaseEffect * target->GetVulnerability(this, spell_id, 0) / 100) * ratio / 100);
				value -= target->GetFcDamageAmtIncoming(this, spell_id);
			}

			value -= (GetBotFocusEffect(focusFcDamageAmtCrit, spell_id) * ratio / 100);

			value -= GetBotFocusEffect(focusFcDamageAmt, spell_id);
			value -= GetBotFocusEffect(focusFcDamageAmt2, spell_id);

			if(itembonuses.SpellDmg && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5)
				value += (GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value) * ratio / 100);

			entity_list.MessageClose(this, false, 100, Chat::SpellCrit, "%s delivers a critical blast! (%d)", GetName(), -value);

			return value;
		}
	}

	value = value_BaseEffect;
	value += (value_BaseEffect * GetBotFocusEffect(focusImprovedDamage, spell_id) / 100);
	value += (value_BaseEffect * GetBotFocusEffect(focusImprovedDamage2, spell_id) / 100);
	value += (value_BaseEffect * GetBotFocusEffect(focusFcDamagePctCrit, spell_id) / 100);
	if (target) {
		value += (value_BaseEffect * target->GetVulnerability(this, spell_id, 0) / 100);
		value -= target->GetFcDamageAmtIncoming(this, spell_id);
	}

	value -= GetBotFocusEffect(focusFcDamageAmtCrit, spell_id);
	value -= GetBotFocusEffect(focusFcDamageAmt, spell_id);
	value -= GetBotFocusEffect(focusFcDamageAmt2, spell_id);
	if(itembonuses.SpellDmg && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5)
		value += GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value);

	return value;
 }

int32 Bot::GetActSpellHealing(uint16 spell_id, int32 value, Mob* target) {
	if (target == nullptr)
		target = this;

	int32 value_BaseEffect = 0;
	int32 chance = 0;
	int8 modifier = 1;
	bool Critical = false;
	value_BaseEffect = (value + (value*GetBotFocusEffect(focusFcBaseEffects, spell_id) / 100));
	value = value_BaseEffect;
	value += int(value_BaseEffect*GetBotFocusEffect(focusImprovedHeal, spell_id) / 100);
	if(spells[spell_id].buffduration < 1) {
		chance += (itembonuses.CriticalHealChance + spellbonuses.CriticalHealChance + aabonuses.CriticalHealChance);
		chance += target->GetFocusIncoming(focusFcHealPctCritIncoming, SE_FcHealPctCritIncoming, this, spell_id);
		if (spellbonuses.CriticalHealDecay)
			chance += GetDecayEffectValue(spell_id, SE_CriticalHealDecay);

		if(chance && (zone->random.Int(0, 99) < chance)) {
			Critical = true;
			modifier = 2;
		}

		value *= modifier;
		value += (GetBotFocusEffect(focusFcHealAmtCrit, spell_id) * modifier);
		value += GetBotFocusEffect(focusFcHealAmt, spell_id);
		value += target->GetFocusIncoming(focusFcHealAmtIncoming, SE_FcHealAmtIncoming, this, spell_id);

		if(itembonuses.HealAmt && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5)
			value += (GetExtraSpellAmt(spell_id, itembonuses.HealAmt, value) * modifier);

		value += (value * target->GetHealRate(spell_id, this) / 100);
		if (Critical)
			entity_list.MessageClose(this, false, 100, Chat::SpellCrit, "%s performs an exceptional heal! (%d)", GetName(), value);

		return value;
	} else {
		chance = (itembonuses.CriticalHealOverTime + spellbonuses.CriticalHealOverTime + aabonuses.CriticalHealOverTime);
		chance += target->GetFocusIncoming(focusFcHealPctCritIncoming, SE_FcHealPctCritIncoming, this, spell_id);
		if (spellbonuses.CriticalRegenDecay)
			chance += GetDecayEffectValue(spell_id, SE_CriticalRegenDecay);

		if(chance && (zone->random.Int(0,99) < chance))
			return (value * 2);
	}
	return value;
}

int32 Bot::GetActSpellCasttime(uint16 spell_id, int32 casttime) {
	int32 cast_reducer = 0;
	cast_reducer += GetBotFocusEffect(focusSpellHaste, spell_id);
	uint8 botlevel = GetLevel();
	uint8 botclass = GetClass();
	if (botlevel >= 51 && casttime >= 3000 && !BeneficialSpell(spell_id) && (botclass == SHADOWKNIGHT || botclass == RANGER || botclass == PALADIN || botclass == BEASTLORD ))
		cast_reducer += ((GetLevel() - 50) * 3);

	if((casttime >= 4000) && BeneficialSpell(spell_id) && IsBuffSpell(spell_id)) {
		switch (GetAA(aaSpellCastingDeftness)) {
			case 1:
				cast_reducer += 5;
				break;
			case 2:
				cast_reducer += 10;
				break;
			case 3:
				cast_reducer += 25;
				break;
		}

		switch (GetAA(aaQuickBuff)) {
			case 1:
				cast_reducer += 10;
				break;
			case 2:
				cast_reducer += 25;
				break;
			case 3:
				cast_reducer += 50;
				break;
		}
	}

	if(IsSummonSpell(spell_id)) {
		switch (GetAA(aaQuickSummoning)) {
			case 1:
				cast_reducer += 10;
				break;
			case 2:
				cast_reducer += 25;
				break;
			case 3:
				cast_reducer += 50;
				break;
		}
	}

	if(IsEvacSpell(spell_id)) {
		switch (GetAA(aaQuickEvacuation)) {
			case 1:
				cast_reducer += 10;
				break;
			case 2:
				cast_reducer += 25;
				break;
			case 3:
				cast_reducer += 50;
				break;
		}
	}

	if(IsDamageSpell(spell_id) && spells[spell_id].cast_time >= 4000) {
		switch (GetAA(aaQuickDamage)) {
			case 1:
				cast_reducer += 2;
				break;
			case 2:
				cast_reducer += 5;
				break;
			case 3:
				cast_reducer += 10;
				break;
		}
	}

	if (cast_reducer > RuleI(Spells, MaxCastTimeReduction))
		cast_reducer = RuleI(Spells, MaxCastTimeReduction);

	casttime = (casttime * (100 - cast_reducer) / 100);
	return casttime;
}

int32 Bot::GetActSpellCost(uint16 spell_id, int32 cost) {
	if(this->itembonuses.Clairvoyance && spells[spell_id].classes[(GetClass()%17) - 1] >= GetLevel() - 5) {
		int32 mana_back = (this->itembonuses.Clairvoyance * zone->random.Int(1, 100) / 100);
		if(mana_back > cost)
			mana_back = cost;

		cost -= mana_back;
	}

	float PercentManaReduction = 0;
	float SpecializeSkill = GetSpecializeSkillValue(spell_id);
	int SuccessChance = zone->random.Int(0, 100);
	float bonus = 1.0;
	switch(GetAA(aaSpellCastingMastery)) {
		case 1:
			bonus += 0.05;
			break;
		case 2:
			bonus += 0.15;
			break;
		case 3:
			bonus += 0.30;
			break;
	}

	bonus += (0.05 * GetAA(aaAdvancedSpellCastingMastery));

	if(SuccessChance <= (SpecializeSkill * 0.3 * bonus)) {
		PercentManaReduction = (1 + 0.05 * SpecializeSkill);
		switch(GetAA(aaSpellCastingMastery)) {
			case 1:
				PercentManaReduction += 2.5;
				break;
			case 2:
				PercentManaReduction += 5.0;
				break;
			case 3:
				PercentManaReduction += 10.0;
				break;
		}

		switch(GetAA(aaAdvancedSpellCastingMastery)) {
			case 1:
				PercentManaReduction += 2.5;
				break;
			case 2:
				PercentManaReduction += 5.0;
				break;
			case 3:
				PercentManaReduction += 10.0;
				break;
		}
	}

	int32 focus_redux = GetBotFocusEffect(focusManaCost, spell_id);

	if(focus_redux > 0)
		PercentManaReduction += zone->random.Real(1, (double)focus_redux);

	cost -= (cost * (PercentManaReduction / 100));
	if(focus_redux >= 100) {
		uint32 buff_max = GetMaxTotalSlots();
		for (int buffSlot = 0; buffSlot < buff_max; buffSlot++) {
			if (buffs[buffSlot].spellid == 0 || buffs[buffSlot].spellid >= SPDAT_RECORDS)
				continue;

			if(IsEffectInSpell(buffs[buffSlot].spellid, SE_ReduceManaCost)) {
				if(CalcFocusEffect(focusManaCost, buffs[buffSlot].spellid, spell_id) == 100)
					cost = 1;
			}
		}
	}

	if(cost < 0)
		cost = 0;

	return cost;
}

float Bot::GetActSpellRange(uint16 spell_id, float range) {
	float extrange = 100;
	extrange += GetBotFocusEffect(focusRange, spell_id);
	return ((range * extrange) / 100);
}

int32 Bot::GetActSpellDuration(uint16 spell_id, int32 duration) {
	int increase = 100;
	increase += GetBotFocusEffect(focusSpellDuration, spell_id);
	int tic_inc = 0;	tic_inc = GetBotFocusEffect(focusSpellDurByTic, spell_id);

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
	range = spells[spell_id].aoerange;
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

bool Bot::CastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot, int32 cast_time, int32 mana_cost,
					uint32* oSpellWillFinish, uint32 item_slot, int16 *resist_adjust, uint32 aa_id) {
	bool Result = false;
	if(zone && !zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))) {
		// LogSpells("CastSpell called for spell [{}] ([{}]) on entity [{}], slot [{}], time [{}], mana [{}], from item slot [{}]", spells[spell_id].name, spell_id, target_id, slot, cast_time, mana_cost, (item_slot==0xFFFFFFFF)?999:item_slot);

		if(casting_spell_id == spell_id)
			ZeroCastingVars();

		if(GetClass() != BARD) {
			if(!IsValidSpell(spell_id) || casting_spell_id || delaytimer || spellend_timer.Enabled() || IsStunned() || IsFeared() || IsMezzed() || (IsSilenced() && !IsDiscipline(spell_id)) || (IsAmnesiad() && IsDiscipline(spell_id))) {
				LogSpells("Spell casting canceled: not able to cast now. Valid? [{}], casting [{}], waiting? [{}], spellend? [{}], stunned? [{}], feared? [{}], mezed? [{}], silenced? [{}]", IsValidSpell(spell_id), casting_spell_id, delaytimer, spellend_timer.Enabled(), IsStunned(), IsFeared(), IsMezzed(), IsSilenced() );
				if(IsSilenced() && !IsDiscipline(spell_id))
					MessageString(Chat::Red, SILENCED_STRING);

				if(IsAmnesiad() && IsDiscipline(spell_id))

					MessageString(Chat::Red, MELEE_SILENCE);

				if(casting_spell_id)
					AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));

				return false;
			}
		}

		if(IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()){
			MessageString(Chat::Red, SPELL_WOULDNT_HOLD);
			if(casting_spell_id)
				AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));

			return false;
		}

		if(DivineAura()) {
			LogSpells("Spell casting canceled: cannot cast while Divine Aura is in effect");
			InterruptSpell(173, 0x121, false);
			return false;
		}

		if(slot < EQ::spells::CastingSlot::MaxGems && !CheckFizzle(spell_id)) {
			int fizzle_msg = IsBardSong(spell_id) ? MISS_NOTE : SPELL_FIZZLE;
			InterruptSpell(fizzle_msg, 0x121, spell_id);

			uint32 use_mana = ((spells[spell_id].mana) / 4);
			LogSpells("Spell casting canceled: fizzled. [{}] mana has been consumed", use_mana);
			SetMana(GetMana() - use_mana);
			return false;
		}

		if (HasActiveSong()) {
			LogSpells("Casting a new spell/song while singing a song. Killing old song [{}]", bardsong);
			bardsong = 0;
			bardsong_target_id = 0;
			bardsong_slot = EQ::spells::CastingSlot::Gem1;
			bardsong_timer.Disable();
		}

		Result = DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, aa_id);
	}
	return Result;
}

bool Bot::SpellOnTarget(uint16 spell_id, Mob* spelltar) {
	bool Result = false;
	if(!IsValidSpell(spell_id))
		return false;

	if(spelltar) {
		if(spelltar->IsBot() && (spells[spell_id].targettype == ST_GroupTeleport)) {
			switch(spell_id) {
				// Paladin
			case 3577: // Wave of Life
			case 4065: // Blessing of Austerity
			case 1455: // Wave of Healing
			case 2589: // Healing Wave of Prexus
			case 3427: // Wave of Marr
			case 3683: // Ethereal Cleansing
			case 1283: // Celestial Cleansing
			case 3485: // Supernal Cleansing
			case 5293: // Pious Cleansing
			case 4893: // Wave of Trushar
			case 5295: // Jeron's Mark
			case 5296: // Wave of Piety
				// Bard
			case 4085: // Forpar's Aria of Affliction
			case 4083: // Rizlona's Embers
			case 4086: // Forpar's Psalm of Pain
			case 4084: // Rizlona's Fire
			case 6734: // Song of the Storm
			case 3651: // Wind of Marr
			case 4087: // Forpar's Verse of Venom
			case 3362: // Rizlona's Call of Flame
			case 4112: // Call of the Muse
			case 4872: // Echo of the Trusik
			case 4873: // Dark Echo
			case 5377: // Cantata of Life
			case 5380: // Yelhun's Mystic Call
			case 5382: // Eriki's Psalm of Power
			case 6666: // Storm Blade
			case 5388: // Ancient Call of Power
				// Cleric
			case 134: // Word of Health
			case 136: // Word of Healing
			case 1520: // Word of Vigor
			case 1521: // Word of Restoration
			case 1523: // Word of Redemption
			case 3471: // Word of Replenishment
			case 5270: // Word of Vivification
			case 2502: // Celestial Remedy
			case 2175: // Celestial Health
			case 1444: // Celestial Healing
			case 1522: // Celestial Elixir
			case 2180: // Etherial Elixir
			case 3047: // Kazad's Mark
			case 3475: // Supernal Elixir
			case 4053: // Blessing of Temperance
			case 4108: // Aura of Reverence
			case 4882: // Holy Elixir
			case 5259: // Pious Elixir
			case 5272: // Aura of Devotion
			case 5277: // Balikor's Mark
				// Enchanter
			case 5517: // Circle of Alendar
			case 6671: // Rune of Rikkukin
			case 6739: // Rune of the Scale
				// Shaman
			case 2521: // Talisman of the Beast
			case 4055: // Pack Shrew
			case 3842: // Blood of Nadox
			case 5417: // Champion
				// Druid
			case 4058: // Feral Pack
			case 2520: // Natures Recovery
				break;
			default:
				return false;
			}
		}

		if(((IsDetrimentalSpell(spell_id) && spelltar->IsBot()) || (IsDetrimentalSpell(spell_id) && spelltar->IsClient())) && !IsResurrectionEffects(spell_id))
			return false;

		if(spelltar->IsPet()) {
			for(int  i= 0; i < EFFECT_COUNT; ++i) {
				if(spells[spell_id].effectid[i] == SE_Illusion)
					return false;
			}
		}
		Result = Mob::SpellOnTarget(spell_id, spelltar);
	}
	return Result;
}

bool Bot::IsImmuneToSpell(uint16 spell_id, Mob *caster) {
	bool Result = false;
	if(!caster)
		return false;

	if(!IsSacrificeSpell(spell_id) && !(zone->GetZoneID() == 202) && !(this == caster)) {
		Result = Mob::IsImmuneToSpell(spell_id, caster);
		if(!Result) {
			if(caster->IsBot()) {
				if(spells[spell_id].targettype == ST_Undead) {
					if((GetBodyType() != BT_SummonedUndead) && (GetBodyType() != BT_Undead) && (GetBodyType() != BT_Vampire)) {
						LogSpells("Bot's target is not an undead");
						return true;
					}
				}
				if(spells[spell_id].targettype == ST_Summoned) {
					if((GetBodyType() != BT_SummonedUndead) && (GetBodyType() != BT_Summoned) && (GetBodyType() != BT_Summoned2) && (GetBodyType() != BT_Summoned3)) {
						LogSpells("Bot's target is not a summoned creature");
						return true;
					}
				}
			}

			LogSpells("No bot immunities to spell [{}] found", spell_id);
		}
	}

	return Result;
}

bool Bot::DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, EQ::spells::CastingSlot slot) {
	bool Result = false;
	SpellTargetType targetType = spells[spell_id].targettype;
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
		if(!ApplyNextBardPulse(bardsong, this, bardsong_slot))
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
			for(int i = 0; i < AIspells.size(); i++) {
				int j = BotGetSpells(i);
				int spelltype = BotGetSpellType(i);
				bool spellequal = (j == thespell);
				bool spelltypeequal = ((spelltype == 2) || (spelltype == 16) || (spelltype == 32));
				bool spelltypetargetequal = ((spelltype == 8) && (spells[thespell].targettype == ST_Self));
				bool spelltypeclassequal = ((spelltype == 1024) && (GetClass() == SHAMAN));
				bool slotequal = (slot == EQ::spells::CastingSlot::Item);
				if(spellequal || slotequal) {
					if((spelltypeequal || spelltypetargetequal) || spelltypeclassequal || slotequal) {
						if(((spells[thespell].effectid[0] == 0) && (spells[thespell].base[0] < 0)) &&
							(spellTarget->GetHP() < ((spells[thespell].base[0] * (-1)) + 100))) {
							LogSpells("Bot::DoFinishedSpellSingleTarget - GroupBuffing failure");
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
		BotGroupSay(this, "MGB %s", spells[spell_id].name);
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

int32 Bot::CalcHPRegenCap(){
	int level = GetLevel();
	int32 hpregen_cap = 0;
	hpregen_cap = (RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA / 25);
	hpregen_cap += (aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap);
	return (hpregen_cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int32 Bot::CalcManaRegenCap(){
	int32 cap = RuleI(Character, ItemManaRegenCap) + aabonuses.ItemManaRegenCap;
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

	RestRegenHP = 6 * (GetMaxHP() / zone->newzone_data.FastRegenHP);
	RestRegenMana = 6 * (GetMaxMana() / zone->newzone_data.FastRegenMana);
	RestRegenEndurance = 6 * (GetMaxEndurance() / zone->newzone_data.FastRegenEndurance);
}

int32 Bot::LevelRegen() {
	int level = GetLevel();
	bool bonus = GetPlayerRaceBit(_baseRace) & RuleI(Character, BaseHPRegenBonusRaces);
	uint8 multiplier1 = bonus ? 2 : 1;
	int32 hp = 0;
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

int32 Bot::CalcHPRegen() {
	int32 regen = (LevelRegen() + itembonuses.HPRegen + spellbonuses.HPRegen);
	regen += (aabonuses.HPRegen + GroupLeadershipAAHealthRegeneration());
	regen = ((regen * RuleI(Character, HPRegenMultiplier)) / 100);
	return regen;
}

int32 Bot::CalcManaRegen() {
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

uint32 Bot::GetClassHPFactor() {
	uint32 factor;
	switch(GetClass()) {
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

int32 Bot::CalcMaxHP() {
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

int32 Bot::CalcMaxEndurance() {
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

int32 Bot::CalcBaseEndurance() {
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

int32 Bot::CalcEnduranceRegen() {
	int32 regen = (int32(GetLevel() * 4 / 10) + 2);
	regen += (spellbonuses.EnduranceRegen + itembonuses.EnduranceRegen);
	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int32 Bot::CalcEnduranceRegenCap() {
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
			int upkeep = spells[buffs[buffs_i].spellid].EndurUpkeep;
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

void Bot::Camp(bool databaseSave) {
	Sit();

	//auto group = GetGroup();
	if(GetGroup())
		RemoveBotFromGroup(this, GetGroup());

	// RemoveBotFromGroup() code is too complicated for this to work as-is (still needs to be addressed to prevent memory leaks)
	//if (group->GroupCount() < 2)
	//	group->DisbandGroup();

	LeaveHealRotationMemberPool();

	if(databaseSave)
		Save();

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

	if (item->SkillModValue != 0 && item->SkillModType <= EQ::skills::HIGHEST_SKILL){
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
		skills[sindex] = database.GetSkillCap(GetClass(), (EQ::skills::SkillType)sindex, GetLevel());
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

	//if(this->Save())
	//	this->GetBotOwner()->CastToClient()->Message(Chat::White, "%s saved.", this->GetCleanName());
	//else
	//	this->GetBotOwner()->CastToClient()->Message(Chat::Red, "%s save failed!", this->GetCleanName());

	CalcBonuses();

	AI_AddNPCSpells(this->GetBotSpellID());

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
		LogError("Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
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

	if( iSpellTypes == SpellType_Buff) {
		uint8 chanceToCast = caster->IsEngaged() ? caster->GetChanceToCastBySpellType(SpellType_Buff) : 100;
		if(botCasterClass == BARD) {
			if(caster->AICastSpell(caster, chanceToCast, SpellType_Buff))
				return true;
			else
				return false;
		}

		if(caster->HasGroup()) {
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

Client* EntityList::GetBotOwnerByBotEntityID(uint16 entityID) {
	Client* Result = nullptr;
	if (entityID > 0) {
		for (std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if (tempBot && tempBot->GetID() == entityID) {
				Result = tempBot->GetBotOwner()->CastToClient();
				break;
			}
		}
	}
	return Result;
}

void EntityList::AddBot(Bot *newBot, bool SendSpawnPacket, bool dontqueue) {
	if(newBot) {
		newBot->SetID(GetFreeID());
		newBot->SetSpawned();
		if(SendSpawnPacket) {
			if(dontqueue) {
				EQApplicationPacket* outapp = new EQApplicationPacket();
				newBot->CreateSpawnPacket(outapp);
				outapp->priority = 6;
				QueueClients(newBot, outapp, true);
				safe_delete(outapp);
			} else {
				NewSpawn_Struct* ns = new NewSpawn_Struct;
				memset(ns, 0, sizeof(NewSpawn_Struct));
				newBot->FillSpawnStruct(ns, newBot);
				AddToSpawnQueue(newBot->GetID(), &ns);
				safe_delete(ns);
			}
			parse->EventNPC(EVENT_SPAWN, newBot, nullptr, "", 0);
		}
		bot_list.push_back(newBot);
		mob_list.insert(std::pair<uint16, Mob*>(newBot->GetID(), newBot));
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

	LogAIScanClose("Close Client Mob List Size [{}] for mob [{}]", close_mobs.size(), scanning_mob->GetCleanName());
}

uint8 Bot::GetNumberNeedingHealedInGroup(uint8 hpr, bool includePets) {
	uint8 needHealed = 0;
	Group *g = nullptr;
	if(this->HasGroup()) {
		g = this->GetGroup();
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

	if(GetEndurance() > spell.EndurCost)
		SetEndurance(GetEndurance() - spell.EndurCost);
	else
		return false;

	if(spell.recast_time > 0) {
		if(CheckDisciplineRecastTimers(this, spells[spell_id].EndurTimerIndex)) {
			if(spells[spell_id].EndurTimerIndex > 0 && spells[spell_id].EndurTimerIndex < MAX_DISCIPLINE_TIMERS)
				SetDisciplineRecastTimer(spells[spell_id].EndurTimerIndex, spell.recast_time);
		} else {
			uint32 remain = (GetDisciplineRemainingTime(this, spells[spell_id].EndurTimerIndex) / 1000);
			GetOwner()->Message(Chat::White, "%s can use this discipline in %d minutes %d seconds.", GetCleanName(), (remain / 60), (remain % 60));
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
				GetBotOwner()->CastToClient()->Message(Chat::Red, "%s", BotDatabase::fail::SaveEquipmentColor());
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

uint8 Bot::spell_casting_chances[SPELL_TYPE_COUNT][PLAYER_CLASS_COUNT][EQ::constants::STANCE_TYPE_COUNT][cntHSND] = { 0 };

#endif
