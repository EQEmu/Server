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
#include "../common/string_util.h"

extern volatile bool is_zone_loaded;

// This constructor is used during the bot create command
Bot::Bot(NPCType npcTypeData, Client* botOwner) : NPC(&npcTypeData, nullptr, glm::vec4(), 0, false), rest_timer(1) {
	if(botOwner) {
		this->SetBotOwner(botOwner);
		this->_botOwnerCharacterID = botOwner->CharacterID();
	} else {
		this->SetBotOwner(0);
		this->_botOwnerCharacterID = 0;
	}

	_guildRank = 0;
	_guildId = 0;
	_lastTotalPlayTime = 0;
	_startTotalPlayTime = time(&_startTotalPlayTime);
	_lastZoneId = 0;
	_baseMR = npcTypeData.MR;
	_baseCR = npcTypeData.CR;
	_baseDR = npcTypeData.DR;
	_baseFR = npcTypeData.FR;
	_basePR = npcTypeData.PR;
	_baseCorrup = npcTypeData.Corrup;
	_baseAC = npcTypeData.AC;
	_baseSTR = npcTypeData.STR;
	_baseSTA = npcTypeData.STA;
	_baseDEX = npcTypeData.DEX;
	_baseAGI = npcTypeData.AGI;
	_baseINT = npcTypeData.INT;
	_baseWIS = npcTypeData.WIS;
	_baseCHA = npcTypeData.CHA;
	_baseATK = npcTypeData.ATK;
	_baseRace = npcTypeData.race;
	_baseGender = npcTypeData.gender;
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
	SetHasBeenSummoned(false);
	SetTaunting(GetClass() == WARRIOR);
	SetDefaultBotStance();

	SetAltOutOfCombatBehavior(GetClass() == BARD); // will need to be updated if more classes make use of this flag
	SetShowHelm(true);
	SetPauseAI(false);
	CalcChanceToCast();
	rest_timer.Disable();
	SetFollowDistance(BOT_DEFAULT_FOLLOW_DISTANCE);
	// Do this once and only in this constructor
	GenerateAppearance();
	GenerateBaseStats();
	GenerateArmorClass();
	// Calculate HitPoints Last As It Uses Base Stats
	cur_hp = GenerateBaseHitPoints();
	cur_mana = GenerateBaseManaPoints();
	cur_end = CalcBaseEndurance();
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
	for (int i = 0; i < MaxTimer; i++)
		timers[i] = 0;

	strcpy(this->name, this->GetCleanName());
	memset(&m_Light, 0, sizeof(EQEmu::lightsource::LightSourceProfile));
	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
}

// This constructor is used when the bot is loaded out of the database
Bot::Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, double totalPlayTime, uint32 lastZoneId, NPCType npcTypeData) : NPC(&npcTypeData, nullptr, glm::vec4(), 0, false), rest_timer(1)
{
	this->_botOwnerCharacterID = botOwnerCharacterID;
	if(this->_botOwnerCharacterID > 0)
		this->SetBotOwner(entity_list.GetClientByCharID(this->_botOwnerCharacterID));

	auto bot_owner = GetBotOwner();

	_guildRank = 0;
	_guildId = 0;
	_lastTotalPlayTime = totalPlayTime;
	_startTotalPlayTime = time(&_startTotalPlayTime);
	_lastZoneId = lastZoneId;
	berserk = false;
	_baseMR = npcTypeData.MR;
	_baseCR = npcTypeData.CR;
	_baseDR = npcTypeData.DR;
	_baseFR = npcTypeData.FR;
	_basePR = npcTypeData.PR;
	_baseCorrup = npcTypeData.Corrup;
	_baseAC = npcTypeData.AC;
	_baseSTR = npcTypeData.STR;
	_baseSTA = npcTypeData.STA;
	_baseDEX = npcTypeData.DEX;
	_baseAGI = npcTypeData.AGI;
	_baseINT = npcTypeData.INT;
	_baseWIS = npcTypeData.WIS;
	_baseCHA = npcTypeData.CHA;
	_baseATK = npcTypeData.ATK;
	_baseRace = npcTypeData.race;
	_baseGender = npcTypeData.gender;
	cur_hp = npcTypeData.cur_hp;
	cur_mana = npcTypeData.Mana;
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
	SetHasBeenSummoned(false);

	bool stance_flag = false;
	if (!botdb.LoadStance(this, stance_flag) && bot_owner)
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::LoadStance(), GetCleanName());
	if (!stance_flag && bot_owner)
		bot_owner->Message(13, "Could not locate stance for '%s'", GetCleanName());

	SetTaunting((GetClass() == WARRIOR || GetClass() == PALADIN || GetClass() == SHADOWKNIGHT) && (GetBotStance() == BotStanceAggressive));
	SetPauseAI(false);

	CalcChanceToCast();
	rest_timer.Disable();
	SetFollowDistance(BOT_DEFAULT_FOLLOW_DISTANCE);
	strcpy(this->name, this->GetCleanName());

	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
	if (!botdb.LoadInspectMessage(GetBotID(), _botInspectMessage) && bot_owner)
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::LoadInspectMessage(), GetCleanName());

	if (!botdb.LoadGuildMembership(GetBotID(), _guildId, _guildRank, _guildName) && bot_owner)
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::LoadGuildMembership(), GetCleanName());
	
	std::string error_message;

	EquipBot(&error_message);
	if(!error_message.empty()) {
		if(bot_owner)
			bot_owner->Message(13, error_message.c_str());
		error_message.clear();
	}

	for (int i = 0; i < MaxTimer; i++)
		timers[i] = 0;

	GenerateBaseStats();

	if (!botdb.LoadTimers(this) && bot_owner)
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::LoadTimers(), GetCleanName());

	LoadAAs();

	if (!botdb.LoadBuffs(this) && bot_owner)
		bot_owner->Message(13, "&s for '%s'", BotDatabase::fail::LoadBuffs(), GetCleanName());

	CalcBotStats(false);
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
	if(cur_hp > max_hp)
		cur_hp = max_hp;

	if(cur_hp <= 0) {
		SetHP(max_hp/5);
		SetMana(0);
		BuffFadeAll();
		SpellOnTarget(756, this); // Rezz effects
	}

	if(cur_mana > max_mana)
		cur_mana = max_mana;

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

uint32 Bot::GetBotArcheryRange() {
	const ItemInst *range_inst = GetBotItem(EQEmu::legacy::SlotRange);
	const ItemInst *ammo_inst = GetBotItem(EQEmu::legacy::SlotAmmo);
	if (!range_inst || !ammo_inst)
		return 0;

	const Item_Struct *range_item = range_inst->GetItem();
	const Item_Struct *ammo_item = ammo_inst->GetItem();
	if (!range_item || !ammo_item || range_item->ItemType != ItemTypeBow || ammo_item->ItemType != ItemTypeArrow)
		return 0;

	// everything is good!
	return (range_item->Range + ammo_item->Range);
}

void Bot::ChangeBotArcherWeapons(bool isArcher) {
	if((GetClass()==WARRIOR) || (GetClass()==PALADIN) || (GetClass()==RANGER) || (GetClass()==SHADOWKNIGHT) || (GetClass()==ROGUE)) {
		if(!isArcher) {
			BotAddEquipItem(EQEmu::legacy::SlotPrimary, GetBotItemBySlot(EQEmu::legacy::SlotPrimary));
			BotAddEquipItem(EQEmu::legacy::SlotSecondary, GetBotItemBySlot(EQEmu::legacy::SlotSecondary));
			SetAttackTimer();
			BotGroupSay(this, "My blade is ready");
		} else {
			BotRemoveEquipItem(EQEmu::legacy::SlotPrimary);
			BotRemoveEquipItem(EQEmu::legacy::SlotSecondary);
			BotAddEquipItem(EQEmu::legacy::SlotAmmo, GetBotItemBySlot(EQEmu::legacy::SlotAmmo));
			BotAddEquipItem(EQEmu::legacy::SlotSecondary, GetBotItemBySlot(EQEmu::legacy::SlotRange));
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
		SetCurrentSpeed(0);
		tar_ndx = 0;
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

NPCType Bot::FillNPCTypeStruct(uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender, float size, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, int32 hp, int32 mana, int32 mr, int32 cr, int32 dr, int32 fr, int32 pr, int32 corrup, int32 ac, uint32 str, uint32 sta, uint32 dex, uint32 agi, uint32 _int, uint32 wis, uint32 cha, uint32 attack) {
	NPCType BotNPCType;
	int CopyLength = 0;
	CopyLength = botName.copy(BotNPCType.name, 63);
	BotNPCType.name[CopyLength] = '\0';
	CopyLength = 0;
	CopyLength = botLastName.copy(BotNPCType.lastname, 69);
	BotNPCType.lastname[CopyLength] = '\0';
	CopyLength = 0;
	BotNPCType.npc_spells_id = botSpellsID;
	BotNPCType.level = botLevel;
	BotNPCType.race = botRace;
	BotNPCType.class_ = botClass;
	BotNPCType.gender = gender;
	BotNPCType.size = size;
	BotNPCType.luclinface = face;
	BotNPCType.hairstyle = hairStyle;
	BotNPCType.haircolor = hairColor;
	BotNPCType.eyecolor1 = eyeColor;
	BotNPCType.eyecolor2 = eyeColor2;
	BotNPCType.beardcolor = beardColor;
	BotNPCType.beard = beard;
	BotNPCType.drakkin_heritage = drakkinHeritage;
	BotNPCType.drakkin_tattoo = drakkinTattoo;
	BotNPCType.drakkin_details = drakkinDetails;
	BotNPCType.cur_hp = hp;
	BotNPCType.Mana = mana;
	BotNPCType.MR = mr;
	BotNPCType.CR = cr;
	BotNPCType.DR = dr;
	BotNPCType.FR = fr;
	BotNPCType.PR = pr;
	BotNPCType.Corrup = corrup;
	BotNPCType.AC = ac;
	BotNPCType.STR = str;
	BotNPCType.STA = sta;
	BotNPCType.DEX = dex;
	BotNPCType.AGI = agi;
	BotNPCType.INT = _int;
	BotNPCType.WIS = wis;
	BotNPCType.CHA = cha;
	BotNPCType.ATK = attack;
	BotNPCType.npc_id = 0;
	BotNPCType.texture = 0;
	BotNPCType.d_melee_texture1 = 0;
	BotNPCType.d_melee_texture2 = 0;
	BotNPCType.qglobal = false;
	BotNPCType.attack_speed = 0;
	BotNPCType.runspeed = 0.7f;
	BotNPCType.bodytype = 1;
	BotNPCType.findable = 0;
	BotNPCType.hp_regen = 1;
	BotNPCType.mana_regen = 1;
	BotNPCType.maxlevel = botLevel;
	BotNPCType.light = NOT_USED; // due to the way that bots are coded..this is sent post-spawn
	return BotNPCType;
}

NPCType Bot::CreateDefaultNPCTypeStructForBot(std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender) {
	NPCType Result;
	int CopyLength = 0;
	CopyLength = botName.copy(Result.name, 63);
	Result.name[CopyLength] = '\0';
	CopyLength = 0;
	CopyLength = botLastName.copy(Result.lastname, 69);
	Result.lastname[CopyLength] = '\0';
	CopyLength = 0;
	Result.level = botLevel;
	Result.race = botRace;
	Result.class_ = botClass;
	Result.gender = gender;
	// default values
	Result.maxlevel = botLevel;
	Result.size = 6.0;
	Result.npc_id = 0;
	Result.cur_hp = 0;
	Result.drakkin_details = 0;
	Result.drakkin_heritage = 0;
	Result.drakkin_tattoo = 0;
	Result.runspeed = 0.7f;
	Result.bodytype = 1;
	Result.findable = 0;
	Result.hp_regen = 1;
	Result.mana_regen = 1;
	Result.texture = 0;
	Result.d_melee_texture1 = 0;
	Result.d_melee_texture2 = 0;
	Result.qglobal = false;
	Result.npc_spells_id = 0;
	Result.attack_speed = 0;
	Result.STR = 75;
	Result.STA = 75;
	Result.DEX = 75;
	Result.AGI = 75;
	Result.WIS = 75;
	Result.INT = 75;
	Result.CHA = 75;
	Result.ATK = 75;
	Result.MR = 25;
	Result.FR = 25;
	Result.DR = 15;
	Result.PR = 15;
	Result.CR = 25;
	Result.Corrup = 15;
	Result.AC = 12;
	return Result;
}

void Bot::GenerateBaseStats() {
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
	switch(this->GetClass()) {
		case 1: // Warrior (why not just use 'case WARRIOR:'?)
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case 2: // Cleric
			BotSpellID = 701;
			Strength += 5;
			Stamina += 5;
			Agility += 10;
			Wisdom += 30;
			Attack += 8;
			break;
		case 3: // Paladin
			BotSpellID = 708;
			Strength += 15;
			Stamina += 5;
			Wisdom += 15;
			Charisma += 10;
			Dexterity += 5;
			Attack += 17;
			break;
		case 4: // Ranger
			BotSpellID = 710;
			Strength += 15;
			Stamina += 10;
			Agility += 10;
			Wisdom += 15;
			Attack += 17;
			break;
		case 5: // Shadowknight
			BotSpellID = 709;
			Strength += 10;
			Stamina += 15;
			Intelligence += 20;
			Charisma += 5;
			Attack += 17;
			break;
		case 6: // Druid
			BotSpellID = 707;
			Stamina += 15;
			Wisdom += 35;
			Attack += 5;
			break;
		case 7: // Monk
			Strength += 5;
			Stamina += 15;
			Agility += 15;
			Dexterity += 15;
			Attack += 17;
			break;
		case 8: // Bard
			BotSpellID = 711;
			Strength += 15;
			Dexterity += 10;
			Charisma += 15;
			Intelligence += 10;
			Attack += 17;
			break;
		case 9: // Rogue
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case 10: // Shaman
			BotSpellID = 706;
			Stamina += 10;
			Wisdom += 30;
			Charisma += 10;
			Attack += 28;
			break;
		case 11: // Necromancer
			BotSpellID = 703;
			Dexterity += 10;
			Agility += 10;
			Intelligence += 30;
			Attack += 5;
			break;
		case 12: // Wizard
			BotSpellID = 702;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case 13: // Magician
			BotSpellID = 704;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case 14: // Enchanter
			BotSpellID = 705;
			Intelligence += 25;
			Charisma += 25;
			Attack += 5;
			break;
		case 15: // Beastlord
			BotSpellID = 712;
			Stamina += 10;
			Agility += 10;
			Dexterity += 5;
			Wisdom += 20;
			Charisma += 5;
			Attack += 31;
			break;
		case 16: // Berserker
			Strength += 10;
			Stamina += 15;
			Dexterity += 15;
			Agility += 10;
			Attack += 25;
			break;
	}

	float BotSize = GetSize();

	switch(this->GetRace()) {
		case 1: // Humans have no race bonus
			break;
		case 2: // Barbarian
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
		case 3: // Erudite
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
		case 4: // Wood Elf
			Strength -= 10;
			Stamina -= 10;
			Agility += 20;
			Dexterity += 5;
			Wisdom += 5;
			BotSize = 5.0;
			break;
		case 5: // High Elf
			Strength -= 20;
			Stamina -= 10;
			Agility += 10;
			Dexterity -= 5;
			Wisdom += 20;
			Intelligence += 12;
			Charisma += 5;
			break;
		case 6: // Dark Elf
			Strength -= 15;
			Stamina -= 10;
			Agility += 15;
			Wisdom += 8;
			Intelligence += 24;
			Charisma -= 15;
			BotSize = 5.0;
			break;
		case 7: // Half Elf
			Strength -= 5;
			Stamina -= 5;
			Agility += 15;
			Dexterity += 10;
			Wisdom -= 15;
			BotSize = 5.5;
			break;
		case 8: // Dwarf
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
		case 9: // Troll
			Strength += 33;
			Stamina += 34;
			Agility += 8;
			Wisdom -= 15;
			Intelligence -= 23;
			Charisma -= 35;
			BotSize = 8.0;
			FireResist -= 20;
			break;
		case 10: // Ogre
			Strength += 55;
			Stamina += 77;
			Agility -= 5;
			Dexterity -= 5;
			Wisdom -= 8;
			Intelligence -= 15;
			Charisma -= 38;
			BotSize = 9.0;
			break;
		case 11: // Halfling
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
		case 12: // Gnome
			Strength -= 15;
			Stamina -= 5;
			Agility += 10;
			Dexterity += 10;
			Wisdom -= 8;
			Intelligence += 23;
			Charisma -= 15;
			BotSize = 3.0;
			break;
		case 128: // Iksar
			Strength -= 5;
			Stamina -= 5;
			Agility += 15;
			Dexterity += 10;
			Wisdom += 5;
			Charisma -= 20;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case 130: // Vah Shir
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
		case 330: // Froglok
			Strength -= 5;
			Stamina += 5;
			Agility += 25;
			Dexterity += 25;
			Charisma -= 25;
			BotSize = 5.0;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case 522: // Drakkin
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
	Log.Out(Logs::General, Logs::Error, "Error in Bot::acmod(): Agility: %i, Level: %i",agility,level);
#endif
	return 0;
}

void Bot::GenerateArmorClass() {
	/// new formula
	int avoidance = 0;
	avoidance = (acmod() + ((GetSkill(SkillDefense) * 16) / 9));
	if(avoidance < 0)
		avoidance = 0;

	int mitigation = 0;
	if(GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == NECROMANCER || GetClass() == ENCHANTER) {
		mitigation = (GetSkill(SkillDefense) / 4 + (itembonuses.AC + 1));
		mitigation -= 4;
	} else {
		mitigation = (GetSkill(SkillDefense) / 3 + ((itembonuses.AC * 4) / 3));
		if(GetClass() == MONK)
			mitigation += (GetLevel() * 13 / 10);	//the 13/10 might be wrong, but it is close...
	}
	int displayed = 0;
	displayed += (((avoidance + mitigation) * 1000) / 847);	//natural AC

	//Iksar AC, untested
	if(GetRace() == IKSAR) {
		displayed += 12;
		int iksarlevel = GetLevel();
		iksarlevel -= 10;
		if(iksarlevel > 25)
			iksarlevel = 25;

		if(iksarlevel > 0)
			displayed += (iksarlevel * 12 / 10);
	}

	//spell AC bonuses are added directly to natural total
	displayed += spellbonuses.AC;
	this->AC = displayed;
}

uint16 Bot::GetPrimarySkillValue() {
	SkillUseTypes skill = HIGHEST_SKILL; //because nullptr == 0, which is 1H Slashing, & we want it to return 0 from GetSkill
	bool equiped = m_inv.GetItem(EQEmu::legacy::SlotPrimary);
	if(!equiped)
		skill = SkillHandtoHand;
	else {
		uint8 type = m_inv.GetItem(EQEmu::legacy::SlotPrimary)->GetItem()->ItemType; //is this the best way to do this?
		switch(type) {
			case ItemType1HSlash: {
				skill = Skill1HSlashing;
				break;
			}
			case ItemType2HSlash: {
				skill = Skill2HSlashing;
				break;
			}
			case ItemType1HPiercing: {
				skill = Skill1HPiercing;
				break;
			}
			case ItemType1HBlunt: {
				skill = Skill1HBlunt;
				break;
			}
			case ItemType2HBlunt: {
				skill = Skill2HBlunt;
				break;
			}
			case ItemType2HPiercing: {
				skill = Skill2HPiercing;
				break;
			}
			case ItemTypeMartial: {
				skill = SkillHandtoHand;
				break;
			}
			default: {
				skill = SkillHandtoHand;
				break;
			}
		}
	}

	return GetSkill(skill);
}

uint16 Bot::MaxSkill(SkillUseTypes skillid, uint16 class_, uint16 level) const {
	return(database.GetSkillCap(class_, skillid, level));
}

uint32 Bot::GetTotalATK() {
	uint32 AttackRating = 0;
	uint32 WornCap = itembonuses.ATK;
	if(IsBot()) {
		AttackRating = ((WornCap * 1.342) + (GetSkill(SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69));
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
		AttackRating = (GetSkill(SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69);
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
	if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
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
	int maxAAExpansion = RuleI(Bots, AAExpansion); //get expansion to get AAs up to
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
		if (!islower(name[i]) && name[i] != '_') {
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
		if (!botdb.SaveNewBot(this, bot_id) || !bot_id) {
			bot_owner->Message(13, "%s '%s'", BotDatabase::fail::SaveNewBot(), GetCleanName());
			return false;
		}
		SetBotID(bot_id);
	}
	else { // Update existing bot record
		if (!botdb.SaveBot(this)) {
			bot_owner->Message(13, "%s '%s'", BotDatabase::fail::SaveBot(), GetCleanName());
			return false;
		}
	}
	
	// All of these continue to process if any fail
	if (!botdb.SaveBuffs(this))
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::SaveBuffs(), GetCleanName());
	if (!botdb.SaveTimers(this))
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::SaveTimers(), GetCleanName());
	if (!botdb.SaveStance(this))
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::SaveStance(), GetCleanName());
	
	if (!SavePet())
		bot_owner->Message(13, "Failed to save pet for '%s'", GetCleanName());
	
	return true;
}

bool Bot::DeleteBot()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	if (!botdb.DeleteHealRotation(GetBotID())) {
		bot_owner->Message(13, "%s", BotDatabase::fail::DeleteHealRotation());
		return false;
	}

	std::string query = StringFormat("DELETE FROM `bot_heal_rotation_members` WHERE `bot_id` = '%u'", GetBotID());
	auto results = botdb.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(13, "Failed to delete heal rotation member '%s'", GetCleanName());
		return false;
	}

	query = StringFormat("DELETE FROM `bot_heal_rotation_targets` WHERE `target_name` LIKE '%s'", GetCleanName());
	results = botdb.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(13, "Failed to delete heal rotation target '%s'", GetCleanName());
		return false;
	}

	if (!DeletePet()) {
		bot_owner->Message(13, "Failed to delete pet for '%s'", GetCleanName());
		return false;
	}

	if (GetGroup())
		RemoveBotFromGroup(this, GetGroup());

	std::string error_message;

	if (!botdb.RemoveMemberFromBotGroup(GetBotID())) {
		bot_owner->Message(13, "%s - '%s'", BotDatabase::fail::RemoveMemberFromBotGroup(), GetCleanName());
		return false;
	}

	if (!botdb.DeleteItems(GetBotID())) {
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::DeleteItems(), GetCleanName());
		return false;
	}

	if (!botdb.DeleteTimers(GetBotID())) {
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::DeleteTimers(), GetCleanName());
		return false;
	}

	if (!botdb.DeleteBuffs(GetBotID())) {
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::DeleteBuffs(), GetCleanName());
		return false;
	}

	if (!botdb.DeleteStance(GetBotID())) {
		bot_owner->Message(13, "%s for '%s'", BotDatabase::fail::DeleteStance(), GetCleanName());
		return false;
	}

	if (!botdb.DeleteBot(GetBotID())) {
		bot_owner->Message(13, "%s '%s'", BotDatabase::fail::DeleteBot(), GetCleanName());
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
	
	std::string error_message;

	uint32 pet_index = 0;
	if (!botdb.LoadPetIndex(GetBotID(), pet_index)) {
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::LoadPetIndex(), GetCleanName());		
		return false;
	}
	if (!pet_index)
		return true;

	uint32 saved_pet_spell_id = 0;
	if (!botdb.LoadPetSpellID(GetBotID(), saved_pet_spell_id)) {
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::LoadPetSpellID(), GetCleanName());
	}
	if (!saved_pet_spell_id || saved_pet_spell_id > SPDAT_RECORDS) {
		bot_owner->Message(13, "Invalid spell id for %s's pet", GetCleanName());
		DeletePet();
		return false;
	}

	std::string pet_name;
	uint32 pet_mana = 0;
	uint32 pet_hp = 0;
	uint32 pet_spell_id = 0;

	if (!botdb.LoadPetStats(GetBotID(), pet_name, pet_mana, pet_hp, pet_spell_id)) {
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::LoadPetStats(), GetCleanName());
		return false;
	}

	MakePet(pet_spell_id, spells[pet_spell_id].teleport_zone, pet_name.c_str());
	if (!GetPet() || !GetPet()->IsNPC()) {
		DeletePet();
		return false;
	}

	NPC *pet_inst = GetPet()->CastToNPC();

	SpellBuff_Struct pet_buffs[BUFF_COUNT];
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * BUFF_COUNT));
	if (!botdb.LoadPetBuffs(GetBotID(), pet_buffs))
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::LoadPetBuffs(), GetCleanName());

	uint32 pet_items[EQEmu::legacy::EQUIPMENT_SIZE];
	memset(pet_items, 0, (sizeof(uint32) * EQEmu::legacy::EQUIPMENT_SIZE));
	if (!botdb.LoadPetItems(GetBotID(), pet_items))
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::LoadPetItems(), GetCleanName());

	pet_inst->SetPetState(pet_buffs, pet_items);
	pet_inst->CalcBonuses();
	pet_inst->SetHP(pet_hp);
	pet_inst->SetMana(pet_mana);

	return true;
}

bool Bot::SavePet()
{
	if (!GetPet() /*|| dead*/)
		return true;
	
	NPC *pet_inst = GetPet()->CastToNPC();
	if (pet_inst->IsFamiliar() || !pet_inst->GetPetSpellID() || pet_inst->GetPetSpellID() > SPDAT_RECORDS)
		return false;

	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	char* pet_name = new char[64];
	SpellBuff_Struct pet_buffs[BUFF_COUNT];
	uint32 pet_items[EQEmu::legacy::EQUIPMENT_SIZE];

	memset(pet_name, 0, 64);
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * BUFF_COUNT));
	memset(pet_items, 0, (sizeof(uint32) * EQEmu::legacy::EQUIPMENT_SIZE));
	
	pet_inst->GetPetState(pet_buffs, pet_items, pet_name);
	
	std::string pet_name_str = pet_name;
	safe_delete_array(pet_name);

	std::string error_message;

	if (!botdb.SavePetStats(GetBotID(), pet_name_str, pet_inst->GetMana(), pet_inst->GetHP(), pet_inst->GetPetSpellID())) {
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::SavePetStats(), GetCleanName());
		return false;
	}
	
	if (!botdb.SavePetBuffs(GetBotID(), pet_buffs))
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::SavePetBuffs(), GetCleanName());
	if (!botdb.SavePetItems(GetBotID(), pet_items))
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::SavePetItems(), GetCleanName());

	return true;
}

bool Bot::DeletePet()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;
	
	std::string error_message;

	if (!botdb.DeletePetItems(GetBotID())) {
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::DeletePetItems(), GetCleanName());
		return false;
	}
	if (!botdb.DeletePetBuffs(GetBotID())) {
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::DeletePetBuffs(), GetCleanName());
		return false;
	}
	if (!botdb.DeletePetStats(GetBotID())) {
		bot_owner->Message(13, "%s for %s's pet", BotDatabase::fail::DeletePetStats(), GetCleanName());
		return false;
	}

	if (!GetPet() || !GetPet()->IsNPC())
		return true;

	NPC* pet_inst = GetPet()->CastToNPC();
	pet_inst->SetOwnerID(0);

	return true;
}

bool Bot::Process() {
	if(IsStunned() && stunned_timer.Check())
		Mob::UnStun();

	if(!GetBotOwner())
		return false;

	if (GetDepop()) {
		_botOwner = 0;
		_botOwnerCharacterID = 0;
		_previousTarget = 0;
		return false;
	}

	SpellProcess();

	if(tic_timer.Check()) {
		//6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if(!IsMoving() && !IsEngaged()) {
			SendPosition();
			if(IsSitting()) {
				if(!rest_timer.Enabled())
					rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
			}
		}

		BuffProcess();
		CalcRestState();
		if(currently_fleeing)
			ProcessFlee();

		if(GetHP() < GetMaxHP())
			SetHP(GetHP() + CalcHPRegen() + RestRegenHP);

		if(GetMana() < GetMaxMana())
			SetMana(GetMana() + CalcManaRegen() + RestRegenMana);

		CalcATK();
		if(GetEndurance() < GetMaxEndurance())
			SetEndurance(GetEndurance() + CalcEnduranceRegen() + RestRegenEndurance);
	}

	if (sendhpupdate_timer.Check(false)) {
		SendHPUpdate();
		if(HasPet())
			GetPet()->SendHPUpdate();
	}

	if(GetAppearance() == eaDead && GetHP() > 0)
		SetAppearance(eaStanding);

	if (IsStunned() || IsMezzed())
		return true;

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
		Log.Out(Logs::Detail, Logs::Combat, "Bot Archery attack canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	ItemInst* rangedItem = GetBotItem(EQEmu::legacy::SlotRange);
	const Item_Struct* RangeWeapon = 0;
	if(rangedItem)
		RangeWeapon = rangedItem->GetItem();

	ItemInst* ammoItem = GetBotItem(EQEmu::legacy::SlotAmmo);
	const Item_Struct* Ammo = 0;
	if(ammoItem)
		Ammo = ammoItem->GetItem();

	if(!RangeWeapon || !Ammo)
		return;

	Log.Out(Logs::Detail, Logs::Combat, "Shooting %s with bow %s (%d) and arrow %s (%d)", other->GetCleanName(), RangeWeapon->Name, RangeWeapon->ID, Ammo->Name, Ammo->ID);
	if(!IsAttackAllowed(other) || IsCasting() || DivineAura() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead))
		return;

	SendItemAnimation(other, Ammo, SkillArchery);
	//DoArcheryAttackDmg(GetTarget(), rangedItem, ammoItem);
	DoArcheryAttackDmg(other, rangedItem, ammoItem); // watch

	//break invis when you attack
	if(invisible) {
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}

	if(invisible_undead) {
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}

	if(invisible_animals) {
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility vs. animals due to melee attack.");
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
	if(!GetSkill(SkillDoubleAttack) && !(GetClass() == BARD || GetClass() == BEASTLORD))
		return false;

	// You start with no chance of double attacking
	float chance = 0.0f;
	uint16 skill = GetSkill(SkillDoubleAttack);
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

void Bot::DoMeleeSkillAttackDmg(Mob* other, uint16 weapon_damage, SkillUseTypes skillinuse, int16 chance_mod, int16 focus, bool CanRiposte, int ReuseTime) {
	if (!CanDoSpecialAttack(other))
		return;

	//For spells using skill value 98 (feral swipe ect) server sets this to 67 automatically.
	if (skillinuse == SkillBegging)
		skillinuse = SkillOffense;

	int damage = 0;
	uint32 hate = 0;
	int Hand = EQEmu::legacy::SlotPrimary;
	if (hate == 0 && weapon_damage > 1)
		hate = weapon_damage;

	if(weapon_damage > 0) {
		if(GetClass() == BERSERKER) {
			int bonus = (3 + GetLevel( )/ 10);
			weapon_damage = (weapon_damage * (100 + bonus) / 100);
		}

		int32 min_hit = 1;
		int32 max_hit = ((2 * weapon_damage * GetDamageTable(skillinuse)) / 100);
		if(GetLevel() >= 28 && IsWarriorClass()) {
			int ucDamageBonus = GetWeaponDamageBonus((const Item_Struct*) nullptr);
			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}

		ApplySpecialAttackMod(skillinuse, max_hit, min_hit);
		min_hit += (min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100);
		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = zone->random.Int(min_hit, max_hit);

		if (other->AvoidDamage(this, damage, CanRiposte ? EQEmu::legacy::SlotRange : EQEmu::legacy::SlotPrimary)) { // MainRange excludes ripo, primary doesn't have any extra behavior
			if (damage == -3) {
				DoRiposte(other);
				if (HasDied())
					return;
			}
		} else {
			if (other->CheckHitChance(this, skillinuse, Hand, chance_mod)) {
				other->MeleeMitigation(this, damage, min_hit);
				if (damage > 0) {
					damage += damage*focus/100;
					ApplyMeleeDamageBonus(skillinuse, damage);
					damage += other->GetFcDamageAmtIncoming(this, 0, true, skillinuse);
					damage += ((itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse) / 100) + GetSkillDmgAmt(skillinuse));
					TryCriticalHit(other, skillinuse, damage, nullptr);
				}
			} else {
				damage = 0;
			}
		}
	}
	else
		damage = -5;

	if(skillinuse == SkillBash){
		const ItemInst* inst = GetBotItem(EQEmu::legacy::SlotSecondary);
		const Item_Struct* botweapon = 0;
		if(inst)
			botweapon = inst->GetItem();

		if(botweapon) {
			if(botweapon->ItemType == ItemTypeShield)
				hate += botweapon->AC;

			hate = (hate * (100 + GetFuriousBash(botweapon->Focus.Effect)) / 100);
		}
	}

	other->AddToHateList(this, hate);

	bool CanSkillProc = true;
	if (skillinuse == SkillOffense){ //Hack to allow damage to display.
		skillinuse = SkillTigerClaw; //'strike' your opponent - Arbitrary choice for message.
		CanSkillProc = false; //Disable skill procs
	}

	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);
	if (HasDied())
		return;

	if (damage > 0)
		CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);

	if((skillinuse == SkillDragonPunch) && GetAA(aaDragonPunch) && zone->random.Int(0, 99) < 25){
		SpellFinished(904, other, 10, 0, -1, spells[904].ResistDiff);
		other->Stun(100);
	}

	if (CanSkillProc && HasSkillProcs())
		TrySkillProc(other, skillinuse, ReuseTime);

	if (CanSkillProc && (damage > 0) && HasSkillProcSuccess())
		TrySkillProc(other, skillinuse, ReuseTime, true);
}

void Bot::ApplySpecialAttackMod(SkillUseTypes skill, int32 &dmg, int32 &mindmg) {
	int item_slot = -1;
	//1: Apply bonus from AC (BOOT/SHIELD/HANDS) est. 40AC=6dmg
	switch (skill) {
		case SkillFlyingKick:
		case SkillRoundKick:
		case SkillKick:
			item_slot = EQEmu::legacy::SlotFeet;
			break;
		case SkillBash:
			item_slot = EQEmu::legacy::SlotSecondary;
			break;
		case SkillDragonPunch:
		case SkillEagleStrike:
		case SkillTigerClaw:
			item_slot = EQEmu::legacy::SlotHands;
			break;
	}

	if (item_slot >= EQEmu::legacy::EQUIPMENT_BEGIN){
		const ItemInst* inst = GetBotItem(item_slot);
		const Item_Struct* botweapon = 0;
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

float Bot::GetMaxMeleeRangeToTarget(Mob* target) {
	float result = 0;
	if(target) {
		float size_mod = GetSize();
		float other_size_mod = target->GetSize();

		if(GetRace() == 49 || GetRace() == 158 || GetRace() == 196) //For races with a fixed size
			size_mod = 60.0f;
		else if (size_mod < 6.0)
			size_mod = 8.0f;

		if(target->GetRace() == 49 || target->GetRace() == 158 || target->GetRace() == 196) //For races with a fixed size
			other_size_mod = 60.0f;
		else if (other_size_mod < 6.0)
			other_size_mod = 8.0f;

		if (other_size_mod > size_mod)
			size_mod = other_size_mod;

		if (size_mod > 29)
			size_mod *= size_mod;
		else if (size_mod > 19)
			size_mod *= (size_mod * 2);
		else
			size_mod *= (size_mod * 4);

		// prevention of ridiculously sized hit boxes
		if (size_mod > 10000)
			size_mod = (size_mod / 7);

		result = size_mod;
	}

	return result;
}

// AI Processing for the Bot object
void Bot::AI_Process() {
	if (!IsAIControlled())
		return;
	if (GetPauseAI())
		return;

	uint8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	if (IsCasting()) {
		if (
			IsHealRotationMember() &&
			m_member_of_heal_rotation->CastingOverride() &&
			m_member_of_heal_rotation->CastingTarget() != nullptr &&
			m_member_of_heal_rotation->CastingReady() &&
			m_member_of_heal_rotation->CastingMember() == this &&
			!m_member_of_heal_rotation->MemberIsCasting(this)
		) {
			InterruptSpell();
		}
		else if (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this) {
			AdvanceHealRotation(false);
			return;
		}
		else if (botClass != BARD) {
			return;
		}
	}
	else if (IsHealRotationMember()) {
		m_member_of_heal_rotation->SetMemberIsCasting(this, false);
	}

	// A bot wont start its AI if not grouped
	if(!GetBotOwner() || !IsGrouped() || GetAppearance() == eaDead)
		return;

	Mob* BotOwner = GetBotOwner();
	if(!BotOwner)
		return;

	try {
		if(BotOwner->CastToClient()->IsDead()) {
			SetTarget(0);
			SetBotOwner(0);
			return;
		}
	}
	catch(...) {
		SetTarget(0);
		SetBotOwner(0);
		return;
	}

	if(IsMyHealRotationSet()) {
		Mob* delete_me = HealRotationTarget();
		if (AIHealRotation(HealRotationTarget(), UseHealRotationFastHeals())) {
#if (EQDEBUG >= 12)
			Log.Out(Logs::General, Logs::Error, "Bot::AI_Process() - Casting succeeded (m: %s, t: %s) : AdvHR(true)", GetCleanName(), ((delete_me) ? (delete_me->GetCleanName()) : ("nullptr")));
#endif
			m_member_of_heal_rotation->SetMemberIsCasting(this);
			m_member_of_heal_rotation->UpdateTargetHealingStats(HealRotationTarget());
			AdvanceHealRotation();
		}
		else {
#if (EQDEBUG >= 12)
			Log.Out(Logs::General, Logs::Error, "Bot::AI_Process() - Casting failed (m: %s, t: %s) : AdvHR(false)", GetCleanName(), ((delete_me) ? (delete_me->GetCleanName()) : ("nullptr")));
#endif
			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
			AdvanceHealRotation(false);
		}
	}

	if(GetHasBeenSummoned()) {
		if(IsBotCaster() || IsBotArcher()) {
			if (AI_movement_timer->Check()) {
				if(!GetTarget() || (IsBotCaster() && !IsBotCasterCombatRange(GetTarget())) || (IsBotArcher() && IsArcheryRange(GetTarget())) || (DistanceSquaredNoZ(static_cast<glm::vec3>(m_Position), m_PreSummonLocation) < 10)) {
					if(GetTarget())
						FaceTarget(GetTarget());

					SetHasBeenSummoned(false);
				} else if(!IsRooted()) {
					if(GetTarget() && GetTarget()->GetHateTop() && GetTarget()->GetHateTop() != this) {
						Log.Out(Logs::Detail, Logs::AI, "Returning to location prior to being summoned.");
						CalculateNewPosition2(m_PreSummonLocation.x, m_PreSummonLocation.y, m_PreSummonLocation.z, GetRunspeed());
						SetHeading(CalculateHeadingToTarget(m_PreSummonLocation.x, m_PreSummonLocation.y));
						return;
					}
				}

				if(IsMoving())
					SendPosUpdate();
				else
					SendPosition();
			}
		} else {
			if(GetTarget())
				FaceTarget(GetTarget());

			SetHasBeenSummoned(false);
		}
		return;
	}

	if(!IsEngaged()) {
		if(GetFollowID()) {
			if(BotOwner && BotOwner->GetTarget() && BotOwner->GetTarget()->IsNPC() && (BotOwner->GetTarget()->GetHateAmount(BotOwner) || BotOwner->CastToClient()->AutoAttackEnabled()) && IsAttackAllowed(BotOwner->GetTarget())) {
					AddToHateList(BotOwner->GetTarget(), 1);
					if(HasPet())
						GetPet()->AddToHateList(BotOwner->GetTarget(), 1);
			} else {
				Group* g = GetGroup();
				if(g) {
					for(int counter = 0; counter < g->GroupCount(); counter++) {
						if(g->members[counter]) {
							Mob* tar = g->members[counter]->GetTarget();
							if(tar && tar->IsNPC() && tar->GetHateAmount(g->members[counter]) && IsAttackAllowed(g->members[counter]->GetTarget())) {
								AddToHateList(tar, 1);
								if(HasPet())
									GetPet()->AddToHateList(tar, 1);

								break;
							}
						}
					}
				}
			}
		}
	}

	if(IsEngaged()) {
		if(rest_timer.Enabled())
			rest_timer.Disable();

		if(IsRooted())
			SetTarget(hate_list.GetClosestEntOnHateList(this));
		else
			SetTarget(hate_list.GetEntWithMostHateOnList(this));

		if(!GetTarget())
			return;

		if(HasPet())
			GetPet()->SetTarget(GetTarget());

		if(!IsSitting())
			FaceTarget(GetTarget());

		if(DivineAura())
			return;

		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// Else, it was causing the bot to aggro behind wall etc... causing massive trains.
		if(!CheckLosFN(GetTarget()) || GetTarget()->IsMezzed() || !IsAttackAllowed(GetTarget())) {
			WipeHateList();
			if(IsMoving()) {
				SetHeading(0);
				SetRunAnimSpeed(0);
				SetCurrentSpeed(GetRunspeed());
				if(moved)
					SetCurrentSpeed(0);
			}
			return;
		}

		if (!(m_PlayerState & static_cast<uint32>(PlayerState::Aggressive)))
			SendAddPlayerState(PlayerState::Aggressive);

		bool atCombatRange = false;
		float meleeDistance = GetMaxMeleeRangeToTarget(GetTarget());
		if(botClass == SHADOWKNIGHT || botClass == PALADIN || botClass == WARRIOR)
			meleeDistance = (meleeDistance * .30);
		else
			meleeDistance *= (float)zone->random.Real(.50, .85);

		bool atArcheryRange = IsArcheryRange(GetTarget());

		if(GetRangerAutoWeaponSelect()) {
			bool changeWeapons = false;

			if(atArcheryRange && !IsBotArcher()) {
				SetBotArcher(true);
				changeWeapons = true;
			} else if(!atArcheryRange && IsBotArcher()) {
				SetBotArcher(false);
				changeWeapons = true;
			}

			if(changeWeapons)
				ChangeBotArcherWeapons(IsBotArcher());
		}

		if(IsBotArcher() && atArcheryRange) {
			if(IsMoving()) {
				SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
				SetRunAnimSpeed(0);
				SetCurrentSpeed(0);
				if(moved) {
					moved = false;
					SetCurrentSpeed(0);
				}
			}
			atCombatRange = true;
		} else if(IsBotCaster() && GetLevel() > 12) {
			if(IsBotCasterCombatRange(GetTarget()))
				atCombatRange = true;
		}
		else if(DistanceSquared(m_Position, GetTarget()->GetPosition())  <= meleeDistance)
			atCombatRange = true;

		if(atCombatRange) {
			if(IsMoving()) {
				SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
				SetCurrentSpeed(0);
				if(moved) {
					moved = false;
					SetCurrentSpeed(0);
				}
			}

			if(AI_movement_timer->Check()) {
				if(!IsMoving() && GetClass() == ROGUE && !BehindMob(GetTarget(), GetX(), GetY())) {
					// Move the rogue to behind the mob
					float newX = 0;
					float newY = 0;
					float newZ = 0;
					if(PlotPositionAroundTarget(GetTarget(), newX, newY, newZ)) {
						CalculateNewPosition2(newX, newY, newZ, GetRunspeed());
						return;
					}
				}
				else if(!IsMoving() && GetClass() != ROGUE && (DistanceSquaredNoZ(m_Position, GetTarget()->GetPosition()) < GetTarget()->GetSize())) {
					// If we are not a rogue trying to backstab, let's try to adjust our melee range so we don't appear to be bunched up
					float newX = 0;
					float newY = 0;
					float newZ = 0;
					if(PlotPositionAroundTarget(GetTarget(), newX, newY, newZ, false) && GetArchetype() != ARCHETYPE_CASTER) {
						CalculateNewPosition2(newX, newY, newZ, GetRunspeed());
						return;
					}
				}

				if(IsMoving())
					SendPosUpdate();
				else
					SendPosition();
			}

			if(IsBotArcher() && ranged_timer.Check(false)) {
				if(GetTarget()->GetHPRatio() <= 99.0f)
					BotRangedAttack(GetTarget());
			}
			else if(!IsBotArcher() && (!(IsBotCaster() && GetLevel() > 12)) && GetTarget() && !IsStunned() && !IsMezzed() && (GetAppearance() != eaDead)) {
				// we can't fight if we don't have a target, are stun/mezzed or dead..
				// Stop attacking if the target is enraged
				if((IsEngaged() && !BehindMob(GetTarget(), GetX(), GetY()) && GetTarget()->IsEnraged()) || GetBotStance() == BotStancePassive)
					return;

				// First, special attack per class (kick, backstab etc..)
				DoClassAttacks(GetTarget());
				if(attack_timer.Check()) {
					Attack(GetTarget(), EQEmu::legacy::SlotPrimary);
					TriggerDefensiveProcs(GetTarget(), EQEmu::legacy::SlotPrimary, false);
					ItemInst *wpn = GetBotItem(EQEmu::legacy::SlotPrimary);
					TryWeaponProc(wpn, GetTarget(), EQEmu::legacy::SlotPrimary);
					bool tripleSuccess = false;
					if(BotOwner && GetTarget() && CanThisClassDoubleAttack()) {
						if(BotOwner && CheckBotDoubleAttack())
							Attack(GetTarget(), EQEmu::legacy::SlotPrimary, true);

						if(BotOwner && GetTarget() && GetSpecialAbility(SPECATK_TRIPLE) && CheckBotDoubleAttack(true)) {
							tripleSuccess = true;
							Attack(GetTarget(), EQEmu::legacy::SlotPrimary, true);
						}

						//quad attack, does this belong here??
						if(BotOwner && GetTarget() && GetSpecialAbility(SPECATK_QUAD) && CheckBotDoubleAttack(true))
							Attack(GetTarget(), EQEmu::legacy::SlotPrimary, true);
					}

					//Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
					int32 flurrychance = (aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance);
					if (GetTarget() && flurrychance) {
						if(zone->random.Int(0, 100) < flurrychance) {
							Message_StringID(MT_NPCFlurry, YOU_FLURRY);
							Attack(GetTarget(), EQEmu::legacy::SlotPrimary, false);
							Attack(GetTarget(), EQEmu::legacy::SlotPrimary, false);
						}
					}

					int32 ExtraAttackChanceBonus = (spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance + aabonuses.ExtraAttackChance);
					if (GetTarget() && ExtraAttackChanceBonus) {
						ItemInst *wpn = GetBotItem(EQEmu::legacy::SlotPrimary);
						if(wpn) {
							if(wpn->GetItem()->ItemType == ItemType2HSlash || wpn->GetItem()->ItemType == ItemType2HBlunt || wpn->GetItem()->ItemType == ItemType2HPiercing) {
								if(zone->random.Int(0, 100) < ExtraAttackChanceBonus)
									Attack(GetTarget(), EQEmu::legacy::SlotPrimary, false);
							}
						}
					}
				}

				if (GetClass() == WARRIOR || GetClass() == BERSERKER) {
					if(GetHP() > 0 && !berserk && this->GetHPRatio() < 30) {
						entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_START, GetName());
						this->berserk = true;
					}

					if (berserk && this->GetHPRatio() > 30) {
						entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_END, GetName());
						this->berserk = false;
					}
				}

				//now off hand
				if(GetTarget() && attack_dw_timer.Check() && CanThisClassDualWield()) {
					const ItemInst* instweapon = GetBotItem(EQEmu::legacy::SlotSecondary);
					const Item_Struct* weapon = 0;
					//can only dual wield without a weapon if you're a monk
					if(instweapon || (botClass == MONK)) {
						if(instweapon)
							weapon = instweapon->GetItem();

						int weapontype = 0; // No weapon type.
						bool bIsFist = true;
						if(weapon) {
							weapontype = weapon->ItemType;
							bIsFist = false;
						}

						if(bIsFist || ((weapontype != ItemType2HSlash) && (weapontype != ItemType2HPiercing) && (weapontype != ItemType2HBlunt))) {
							float DualWieldProbability = 0.0f;
							int32 Ambidexterity = (aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity);
							DualWieldProbability = ((GetSkill(SkillDualWield) + GetLevel() + Ambidexterity) / 400.0f); // 78.0 max
							int32 DWBonus = (spellbonuses.DualWieldChance + itembonuses.DualWieldChance);
							DualWieldProbability += (DualWieldProbability * float(DWBonus) / 100.0f);
							float random = zone->random.Real(0, 1);
							if (random < DualWieldProbability){ // Max 78% of DW
								Attack(GetTarget(), EQEmu::legacy::SlotSecondary);	// Single attack with offhand
								ItemInst *wpn = GetBotItem(EQEmu::legacy::SlotSecondary);
								TryWeaponProc(wpn, GetTarget(), EQEmu::legacy::SlotSecondary);
								if( CanThisClassDoubleAttack() && CheckBotDoubleAttack()) {
									if(GetTarget() && GetTarget()->GetHP() > -10)
										Attack(GetTarget(), EQEmu::legacy::SlotSecondary);	// Single attack with offhand
								}
							}
						}
					}
				}
			}
		} else {
			if(GetTarget()->IsFeared() && !spellend_timer.Enabled()){
				// This is a mob that is fleeing either because it has been feared or is low on hitpoints
				if(GetBotStance() != BotStancePassive)
					AI_PursueCastCheck();
			}

			if (AI_movement_timer->Check()) {
				if(!IsRooted()) {
					Log.Out(Logs::Detail, Logs::AI, "Pursuing %s while engaged.", GetTarget()->GetCleanName());
					CalculateNewPosition2(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetRunspeed());
					return;
				}

				if(IsMoving())
					SendPosUpdate();
				else
					SendPosition();
			}
		} // end not in combat range

		if(!IsMoving() && !spellend_timer.Enabled()) {
			if(GetBotStance() == BotStancePassive)
				return;

			if(AI_EngagedCastCheck())
				BotMeditate(false);
			else if(GetArchetype() == ARCHETYPE_CASTER)
				BotMeditate(true);
		}
	} else {
		SetTarget(0);
		if (m_PlayerState & static_cast<uint32>(PlayerState::Aggressive))
			SendRemovePlayerState(PlayerState::Aggressive);

		if(!IsMoving() && AI_think_timer->Check() && !spellend_timer.Enabled()) {
			if(GetBotStance() != BotStancePassive) {
				if(!AI_IdleCastCheck() && !IsCasting())
					BotMeditate(true);
			}
			else
				BotMeditate(true);
		}

		if(AI_movement_timer->Check()) {
			if(GetFollowID()) {
				Mob* follow = entity_list.GetMob(GetFollowID());
				if(follow) {
					float dist = DistanceSquared(m_Position, follow->GetPosition());
					int speed = follow->GetRunspeed();
					if(dist < GetFollowDistance() + 1000)
						speed = follow->GetWalkspeed();

					if(dist > GetFollowDistance()) {
						CalculateNewPosition2(follow->GetX(), follow->GetY(), follow->GetZ(), speed);
						if(rest_timer.Enabled())
							rest_timer.Disable();
						return;
					} else {
						if(moved) {
							moved = false;
							SetCurrentSpeed(0);
						}
					}
				}
			}
		}
	}
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
						botPet->CalculateNewPosition2(newX, newY, newZ, botPet->GetRunspeed());
						return;
					}
				}
				else if(GetTarget() == botPet->GetTarget() && !petHasAggro && !botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY())) {
					// If the bot owner and the bot are fighting the same mob, then move the pet to the rear arc of the mob
					if(botPet->PlotPositionAroundTarget(botPet->GetTarget(), newX, newY, newZ)) {
						botPet->CalculateNewPosition2(newX, newY, newZ, botPet->GetRunspeed());
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
						botPet->CalculateNewPosition2(newX, newY, newZ, botPet->GetRunspeed());
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

					if (botPet->Attack(GetTarget(), EQEmu::legacy::SlotPrimary))	// try the main hand
						if (botPet->GetTarget()) {
							// We're a pet so we re able to dual attack
							int32 RandRoll = zone->random.Int(0, 99);
							if (botPet->CanThisClassDoubleAttack() && (RandRoll < (botPet->GetLevel() + NPCDualAttackModifier))) {
								if (botPet->Attack(botPet->GetTarget(), EQEmu::legacy::SlotPrimary)) {}
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
								float DualWieldProbability = ((botPet->GetSkill(SkillDualWield) + botPet->GetLevel()) / 400.0f);
								DualWieldProbability -= zone->random.Real(0, 1);
								if(DualWieldProbability < 0) {
									botPet->Attack(botPet->GetTarget(), EQEmu::legacy::SlotSecondary);
									if (botPet->CanThisClassDoubleAttack()) {
										int32 RandRoll = zone->random.Int(0, 99);
										if (RandRoll < (botPet->GetLevel() + 20))
											botPet->Attack(botPet->GetTarget(), EQEmu::legacy::SlotSecondary);
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
						Log.Out(Logs::Detail, Logs::AI, "Pursuing %s while engaged.", botPet->GetTarget()->GetCleanName());
						botPet->CalculateNewPosition2(botPet->GetTarget()->GetX(), botPet->GetTarget()->GetY(), botPet->GetTarget()->GetZ(), botPet->GetOwner()->GetRunspeed());
						return;
					} else {
						botPet->SetHeading(botPet->GetTarget()->GetHeading());
						if(moved) {
							moved = false;
							SetCurrentSpeed(0);
							botPet->SendPosition();
							botPet->SetMoving(false);
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
						botPet->CalculateNewPosition2(botPet->GetTarget()->GetX(), botPet->GetTarget()->GetY(), botPet->GetTarget()->GetZ(), botPet->GetTarget()->GetRunspeed());
						return;
					} else {
						botPet->SetHeading(botPet->GetTarget()->GetHeading());
						if(moved) {
							moved = false;
							SetCurrentSpeed(0);
							botPet->SendPosition();
							botPet->SetMoving(false);
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

void Bot::Spawn(Client* botCharacterOwner) {
	if(GetBotID() > 0 && _botOwnerCharacterID > 0 && botCharacterOwner && botCharacterOwner->CharacterID() == _botOwnerCharacterID) {
		// Rename the bot name to make sure that Mob::GetName() matches Mob::GetCleanName() so we dont have a bot named "Jesuschrist001"
		strcpy(name, GetCleanName());

		// Get the zone id this bot spawned in
		_lastZoneId = GetZoneID();

		// this change propagates to Bot::FillSpawnStruct()
		this->helmtexture = 0; //0xFF;
		this->texture = 0; //0xFF;

		if(this->Save())
			this->GetBotOwner()->CastToClient()->Message(0, "%s saved.", this->GetCleanName());
		else
			this->GetBotOwner()->CastToClient()->Message(13, "%s save failed!", this->GetCleanName());

		// Spawn the bot at the bow owner's loc
		this->m_Position.x = botCharacterOwner->GetX();
		this->m_Position.y = botCharacterOwner->GetY();
		this->m_Position.z = botCharacterOwner->GetZ();

		// Make the bot look at the bot owner
		FaceTarget(botCharacterOwner);
		UpdateEquipmentLight();
		UpdateActiveLight();
		entity_list.AddBot(this, true, true);
		// Load pet
		LoadPet();
		this->SendPosition();
		// there is something askew with spawn struct appearance fields...
		// I re-enabled this until I can sort it out
		uint32 itemID = 0;
		uint8 materialFromSlot = 0xFF;
		for (int i = EQEmu::legacy::EQUIPMENT_BEGIN; i <= EQEmu::legacy::EQUIPMENT_END; ++i) {
			itemID = GetBotItemBySlot(i);
			if(itemID != 0) {
				materialFromSlot = Inventory::CalcMaterialFromSlot(i);
				if(materialFromSlot != 0xFF)
					this->SendWearChange(materialFromSlot);
			}
		}
	}
}

// Deletes the inventory record for the specified item from the database for this bot.
void Bot::RemoveBotItemBySlot(uint32 slotID, std::string *errorMessage)
{
	if(!GetBotID())
        return;

    if(!botdb.DeleteItemBySlot(GetBotID(), slotID))
        *errorMessage = BotDatabase::fail::DeleteItemBySlot();

    m_inv.DeleteItem(slotID);
	UpdateEquipmentLight();
}

// Retrieves all the inventory records from the database for this bot.
void Bot::GetBotItems(Inventory &inv, std::string* errorMessage)
{
	if(!GetBotID())
		return;

	if (!botdb.LoadItems(GetBotID(), inv)) {
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

	if (!botdb.LoadItemBySlot(GetBotID(), slotID, item_id)) {
		if (GetBotOwner() && GetBotOwner()->IsClient())
			GetBotOwner()->CastToClient()->Message(13, "%s", BotDatabase::fail::LoadItemBySlot());
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
		ns->spawn.light = m_Light.Type.Active;
		ns->spawn.helm = helmtexture; //(GetShowHelm() ? helmtexture : 0); //0xFF;
		ns->spawn.equip_chest2 = texture; //0xFF;
		const Item_Struct* item = 0;
		const ItemInst* inst = 0;
		uint32 spawnedbotid = 0;
		spawnedbotid = this->GetBotID();
		for (int i = 0; i < EQEmu::legacy::MaterialPrimary; i++) {
			inst = GetBotItem(i);
			if (inst) {
				item = inst->GetItem();
				if (item != 0) {
					ns->spawn.equipment[i].Material = item->Material;
					ns->spawn.equipment[i].EliteMaterial = item->EliteMaterial;
					ns->spawn.equipment[i].HeroForgeModel = item->HerosForgeModel;
					if (armor_tint[i])
						ns->spawn.colors[i].Color = armor_tint[i];
					else
						ns->spawn.colors[i].Color = item->Color;
				} else {
					if (armor_tint[i])
						ns->spawn.colors[i].Color = armor_tint[i];
				}
			}
		}

		inst = GetBotItem(EQEmu::legacy::SlotPrimary);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment[EQEmu::legacy::MaterialPrimary].Material = atoi(&item->IDFile[2]);

				ns->spawn.colors[EQEmu::legacy::MaterialPrimary].Color = GetEquipmentColor(EQEmu::legacy::MaterialPrimary);
			}
		}

		inst = GetBotItem(EQEmu::legacy::SlotSecondary);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment[EQEmu::legacy::MaterialSecondary].Material = atoi(&item->IDFile[2]);

				ns->spawn.colors[EQEmu::legacy::MaterialSecondary].Color = GetEquipmentColor(EQEmu::legacy::MaterialSecondary);
			}
		}
	}
}

Bot* Bot::LoadBot(uint32 botID)
{
	Bot* loaded_bot = nullptr;
	if (!botID)
		return loaded_bot;
	
	if (!botdb.LoadBot(botID, loaded_bot)) // TODO: Consider update to message handler
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
				if (!botdb.LoadGroupedBotsByGroupID(TempGroupId, ActiveBots)) {
					botOwner->Message(13, "%s", BotDatabase::fail::LoadGroupedBotsByGroupID());
					return;
				}
				
				if(!ActiveBots.empty()) {
					for(std::list<uint32>::iterator itr = ActiveBots.begin(); itr != ActiveBots.end(); ++itr) {
						Bot* activeBot = Bot::LoadBot(*itr);

						if(activeBot) {
							activeBot->Spawn(botOwner);
							g->UpdatePlayer(activeBot);
							if(g->GetLeader())
								activeBot->SetFollowID(g->GetLeader()->GetID());
						}

						if(activeBot && !botOwner->HasGroup())
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
				bot->CalcBotStats(false); // TODO: look at this and see if 'true' should be passed...
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
ItemInst* Bot::GetBotItem(uint32 slotID) {
	ItemInst* item = m_inv.GetItem(slotID);
	if(item)
		return item;

	return nullptr;
}

// Adds the specified item it bot to the NPC equipment array and to the bot inventory collection.
void Bot::BotAddEquipItem(int slot, uint32 id) {
	if(slot > 0 && id > 0) {
		uint8 materialFromSlot = Inventory::CalcMaterialFromSlot(slot);

		if (materialFromSlot != EQEmu::legacy::MaterialInvalid) {
			equipment[slot] = id; // npc has more than just material slots. Valid material should mean valid inventory index
			SendWearChange(materialFromSlot);
		}

		UpdateEquipmentLight();
		if (UpdateActiveLight())
			SendAppearancePacket(AT_Light, GetActiveLightType());
	}
}

// Erases the specified item from bot the NPC equipment array and from the bot inventory collection.
void Bot::BotRemoveEquipItem(int slot) {
	if(slot > 0) {
		uint8 materialFromSlot = Inventory::CalcMaterialFromSlot(slot);

		if (materialFromSlot != EQEmu::legacy::MaterialInvalid) {
			equipment[slot] = 0; // npc has more than just material slots. Valid material should mean valid inventory index
			SendWearChange(materialFromSlot);
			if (materialFromSlot == EQEmu::legacy::MaterialChest)
				SendWearChange(EQEmu::legacy::MaterialArms);
		}

		UpdateEquipmentLight();
		if (UpdateActiveLight())
			SendAppearancePacket(AT_Light, GetActiveLightType());
	}
}

void Bot::BotTradeSwapItem(Client* client, int16 lootSlot, const ItemInst* inst, const ItemInst* inst_swap, uint32 equipableSlots, std::string* errorMessage, bool swap) {

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

void Bot::BotTradeAddItem(uint32 id, const ItemInst* inst, int16 charges, uint32 equipableSlots, uint16 lootSlot, std::string* errorMessage, bool addToDb)
{
	if(addToDb) {
		if (!botdb.SaveItemBySlot(this, lootSlot, inst)) {
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
void Bot::FinishTrade(Client* client, BotTradeType tradeType) {
	if(client && !client->GetTradeskillObject() && (client->trade->state != Trading)) {
		if(tradeType == BotTradeClientNormal) {
			// Items being traded are found in the normal trade window used to trade between a Client and a Client or NPC
			// Items in this mode are found in slot ids 3000 thru 3003 - thought bots used the full 8-slot window..?
			PerformTradeWithClient(EQEmu::legacy::TRADE_BEGIN, EQEmu::legacy::TRADE_END, client); // {3000..3007}
		}
		else if(tradeType == BotTradeClientNoDropNoTrade) {
			// Items being traded are found on the Client's cursor slot, slot id 30. This item can be either a single item or it can be a bag.
			// If it is a bag, then we have to search for items in slots 331 thru 340
			PerformTradeWithClient(EQEmu::legacy::SlotCursor, EQEmu::legacy::SlotCursor, client);

			// TODO: Add logic here to test if the item in SLOT_CURSOR is a container type, if it is then we need to call the following:
			// PerformTradeWithClient(331, 340, client);
		}
	}
}

// Perfoms the actual trade action with a client bot owner
void Bot::PerformTradeWithClient(int16 beginSlotID, int16 endSlotID, Client* client) {
	if(client) {
		// TODO: Figure out what the actual max slot id is
		const int MAX_SLOT_ID = EQEmu::legacy::TRADE_BAGS_END; // was the old incorrect 3179..
		uint32 items[MAX_SLOT_ID] = {0};
		uint8 charges[MAX_SLOT_ID] = {0};
		bool botCanWear[MAX_SLOT_ID] = {0};

		for(int16 i = beginSlotID; i <= endSlotID; ++i) {
			bool BotCanWear = false;
			bool UpdateClient = false;
			bool already_returned = false;

			Inventory& clientInventory = client->GetInv();
			const ItemInst* inst = clientInventory[i];
			if(inst) {
				items[i] = inst->GetItem()->ID;
				charges[i] = inst->GetCharges();
			}

			if (i == EQEmu::legacy::SlotCursor)
				UpdateClient = true;

			//EQoffline: will give the items to the bots and change the bot stats
			if(inst && (GetBotOwner() == client->CastToMob()) && !IsEngaged()) {
				std::string TempErrorMessage;
				const Item_Struct* mWeaponItem = inst->GetItem();
				bool failedLoreCheck = false;
				for (int m = AUG_INDEX_BEGIN; m < EQEmu::legacy::ITEM_COMMON_SIZE; ++m) {
					ItemInst *itm = inst->GetAugment(m);
					if(itm)
					{
						if(CheckLoreConflict(itm->GetItem())) {
							failedLoreCheck = true;
						}
					}
				}
				if(CheckLoreConflict(mWeaponItem)) {
					failedLoreCheck = true;
				}
				if(failedLoreCheck) {
					Message_StringID(0, DUP_LORE);
				}
				if(!failedLoreCheck && mWeaponItem && inst->IsEquipable(GetBaseRace(), GetClass()) && (GetLevel() >= mWeaponItem->ReqLevel)) {
					BotCanWear = true;
					botCanWear[i] = BotCanWear;
					ItemInst* swap_item = nullptr;

					const char* equipped[EQEmu::legacy::EQUIPMENT_SIZE + 1] = { "Charm", "Left Ear", "Head", "Face", "Right Ear", "Neck", "Shoulders", "Arms", "Back",
												"Left Wrist", "Right Wrist", "Range", "Hands", "Primary Hand", "Secondary Hand",
												"Left Finger", "Right Finger", "Chest", "Legs", "Feet", "Waist", "Ammo", "Powersource" };
					bool success = false;
					int how_many_slots = 0;
					for (int j = EQEmu::legacy::EQUIPMENT_BEGIN; j <= (EQEmu::legacy::EQUIPMENT_END + 1); ++j) {
						if((mWeaponItem->Slots & (1 << j))) {
							if (j == 22)
								j = 9999;

							how_many_slots++;
							if(!GetBotItem(j)) {
								if (j == EQEmu::legacy::SlotPrimary) {
									if((mWeaponItem->ItemType == ItemType2HSlash) || (mWeaponItem->ItemType == ItemType2HBlunt) || (mWeaponItem->ItemType == ItemType2HPiercing)) {
										if (GetBotItem(EQEmu::legacy::SlotSecondary)) {
											if(mWeaponItem && (mWeaponItem->ItemType == ItemType2HSlash) || (mWeaponItem->ItemType == ItemType2HBlunt) || (mWeaponItem->ItemType == ItemType2HPiercing)) {
												if (client->CheckLoreConflict(GetBotItem(EQEmu::legacy::SlotSecondary)->GetItem())) {
													failedLoreCheck = true;
												}
											}
											else {
												ItemInst* remove_item = GetBotItem(EQEmu::legacy::SlotSecondary);
												BotTradeSwapItem(client, EQEmu::legacy::SlotSecondary, 0, remove_item, remove_item->GetItem()->Slots, &TempErrorMessage, false);
											}
										}
									}
									if(!failedLoreCheck) {
										BotTradeAddItem(mWeaponItem->ID, inst, inst->GetCharges(), mWeaponItem->Slots, j, &TempErrorMessage);
										success = true;
									}
									break;
								}
								else if (j == EQEmu::legacy::SlotSecondary) {
									if(inst->IsWeapon()) {
										if(CanThisClassDualWield()) {
											BotTradeAddItem(mWeaponItem->ID, inst, inst->GetCharges(), mWeaponItem->Slots, j, &TempErrorMessage);
											success = true;
										}
										else {
											BotGroupSay(this, "I can't Dual Wield yet.");
											--how_many_slots;
										}
									}
									else {
										BotTradeAddItem(mWeaponItem->ID, inst, inst->GetCharges(), mWeaponItem->Slots, j, &TempErrorMessage);
										success = true;
									}
									if(success) {
										if (GetBotItem(EQEmu::legacy::SlotPrimary)) {
											ItemInst* remove_item = GetBotItem(EQEmu::legacy::SlotPrimary);
											if((remove_item->GetItem()->ItemType == ItemType2HSlash) || (remove_item->GetItem()->ItemType == ItemType2HBlunt) || (remove_item->GetItem()->ItemType == ItemType2HPiercing)) {
												BotTradeSwapItem(client, EQEmu::legacy::SlotPrimary, 0, remove_item, remove_item->GetItem()->Slots, &TempErrorMessage, false);
											}
										}
										break;
									}
								}
								else {
									BotTradeAddItem(mWeaponItem->ID, inst, inst->GetCharges(), mWeaponItem->Slots, j, &TempErrorMessage);
									success = true;
									break;
								}
							}
						}
					}
					if(!success) {
						for (int j = EQEmu::legacy::EQUIPMENT_BEGIN; j <= (EQEmu::legacy::EQUIPMENT_END + 1); ++j) {
							if((mWeaponItem->Slots & (1 << j))) {
								if (j == 22)
									j = 9999;

								swap_item = GetBotItem(j);
								failedLoreCheck = false;
								for (int k = AUG_INDEX_BEGIN; k < EQEmu::legacy::ITEM_COMMON_SIZE; ++k) {
									ItemInst *itm = swap_item->GetAugment(k);
									if(itm)
									{
										if(client->CheckLoreConflict(itm->GetItem())) {
											failedLoreCheck = true;
										}
									}
								}
								if(client->CheckLoreConflict(swap_item->GetItem())) {
									failedLoreCheck = true;
								}
								if(!failedLoreCheck) {
									if (j == EQEmu::legacy::SlotPrimary) {
										if((mWeaponItem->ItemType == ItemType2HSlash) || (mWeaponItem->ItemType == ItemType2HBlunt) || (mWeaponItem->ItemType == ItemType2HPiercing)) {
											if (GetBotItem(EQEmu::legacy::SlotSecondary)) {
												if (client->CheckLoreConflict(GetBotItem(EQEmu::legacy::SlotSecondary)->GetItem())) {
													failedLoreCheck = true;
												}
												else {
													ItemInst* remove_item = GetBotItem(EQEmu::legacy::SlotSecondary);
													BotTradeSwapItem(client, EQEmu::legacy::SlotSecondary, 0, remove_item, remove_item->GetItem()->Slots, &TempErrorMessage, false);
												}
											}
										}
										if(!failedLoreCheck) {
											BotTradeSwapItem(client, EQEmu::legacy::SlotPrimary, inst, swap_item, mWeaponItem->Slots, &TempErrorMessage);
											success = true;
										}
										break;
									}
									else if (j == EQEmu::legacy::SlotSecondary) {
										if(inst->IsWeapon()) {
											if(CanThisClassDualWield()) {
												BotTradeSwapItem(client, EQEmu::legacy::SlotSecondary, inst, swap_item, mWeaponItem->Slots, &TempErrorMessage);
												success = true;
											}
											else {
												botCanWear[i] = false;
												BotGroupSay(this, "I can't Dual Wield yet.");
											}
										}
										else {
											BotTradeSwapItem(client, EQEmu::legacy::SlotSecondary, inst, swap_item, mWeaponItem->Slots, &TempErrorMessage);
											success = true;
										}
										if (success && GetBotItem(EQEmu::legacy::SlotPrimary)) {
											ItemInst* remove_item = GetBotItem(EQEmu::legacy::SlotPrimary);
											if((remove_item->GetItem()->ItemType == ItemType2HSlash) || (remove_item->GetItem()->ItemType == ItemType2HBlunt) || (remove_item->GetItem()->ItemType == ItemType2HPiercing)) {
												BotTradeSwapItem(client, EQEmu::legacy::SlotPrimary, 0, remove_item, remove_item->GetItem()->Slots, &TempErrorMessage, false);
											}
										}
										break;
									}
									else {
										BotTradeSwapItem(client, j, inst, swap_item, mWeaponItem->Slots, &TempErrorMessage);
										success = true;
										break;
									}
								}
								else {
									botCanWear[i] = false;
									Message_StringID(0, PICK_LORE);
									break;
								}
							}
						}
					}
					if(success) {
						if(how_many_slots > 1) {
							Message(300, "If you want this item in a different slot, use #bot inventory remove <slot_id> to clear the spot.");
						}
						CalcBotStats();
					}
				}
			}
			if(inst) {
				client->DeleteItemInInventory(i, 0, UpdateClient);
				if(!botCanWear[i]) {
					client->PushItemOnCursor(*inst, true);
				}
			}
		}

		const Item_Struct* item2 = 0;
		for(int y = beginSlotID; y <= endSlotID; ++y) {
			item2 = database.GetItem(items[y]);
			if(item2) {
				if(botCanWear[y]) {
					BotGroupSay(this, "Thank you for the %s, %s!", item2->Name, client->GetName());
				}
				else {
					BotGroupSay(this, "I can't use this %s!", item2->Name);
				}
			}
		}
	}
}

bool Bot::Death(Mob *killerMob, int32 damage, uint16 spell_id, SkillUseTypes attack_skill) {
	if(!NPC::Death(killerMob, damage, spell_id, attack_skill))
		return false;

	Save();
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

	entity_list.RemoveBot(this->GetID());
	return true;
}

void Bot::Damage(Mob *from, int32 damage, uint16 spell_id, SkillUseTypes attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, int special) {
	if(spell_id == 0)
		spell_id = SPELL_UNKNOWN;

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if(attacked_timer.Check()) {
		Log.Out(Logs::Detail, Logs::Combat, "Triggering EVENT_ATTACK due to attack by %s", from->GetName());
		parse->EventNPC(EVENT_ATTACK, this, from, "", 0);
	}

	attacked_timer.Start(CombatEventTimer_expire);
	// if spell is lifetap add hp to the caster
	if (spell_id != SPELL_UNKNOWN && IsLifetapSpell(spell_id)) {
		int healed = GetActSpellHealing(spell_id, damage);
		Log.Out(Logs::Detail, Logs::Combat, "Applying lifetap heal of %d to %s", healed, GetCleanName());
		HealDamage(healed);
		entity_list.MessageClose(this, true, 300, MT_Spells, "%s beams a smile at %s", GetCleanName(), from->GetCleanName() );
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

void Bot::AddToHateList(Mob* other, uint32 hate /*= 0*/, int32 damage /*= 0*/, bool iYellForHelp /*= true*/, bool bFrenzy /*= false*/, bool iBuffTic /*= false*/) {
	Mob::AddToHateList(other, hate, damage, iYellForHelp, bFrenzy, iBuffTic);
}

bool Bot::Attack(Mob* other, int Hand, bool FromRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, int special) {
	if (!other) {
		SetTarget(nullptr);
		Log.Out(Logs::General, Logs::Error, "A null Mob object was passed to Bot::Attack for evaluation!");
		return false;
	}

	if(!GetTarget() || GetTarget() != other)
		SetTarget(other);

	Log.Out(Logs::Detail, Logs::Combat, "Attacking %s with hand %d %s", other?other->GetCleanName():"(nullptr)", Hand, FromRiposte?"(this is a riposte)":"");
	if ((IsCasting() && (GetClass() != BARD) && !IsFromSpell) || other == nullptr || (GetHP() < 0) || (GetAppearance() == eaDead) || (!IsAttackAllowed(other))) {
		if(this->GetOwnerID())
			entity_list.MessageClose(this, 1, 200, 10, "%s says, '%s is not a legal target master.'", this->GetCleanName(), this->GetTarget()->GetCleanName());

		if(other) {
			RemoveFromHateList(other);
			Log.Out(Logs::Detail, Logs::Combat, "I am not allowed to attack %s", other->GetCleanName());
		}
		return false;
	}

	if(DivineAura()) {//cant attack while invulnerable
		Log.Out(Logs::Detail, Logs::Combat, "Attack canceled, Divine Aura is in effect.");
		return false;
	}

	FaceTarget(GetTarget());
	ItemInst* weapon = nullptr;
	if (Hand == EQEmu::legacy::SlotPrimary) {
		weapon = GetBotItem(EQEmu::legacy::SlotPrimary);
		OffHandAtk(false);
	}

	if (Hand == EQEmu::legacy::SlotSecondary) {
		weapon = GetBotItem(EQEmu::legacy::SlotSecondary);
		OffHandAtk(true);
	}

	if(weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			Log.Out(Logs::Detail, Logs::Combat, "Attack canceled, Item %s (%d) is not a weapon.", weapon->GetItem()->Name, weapon->GetID());
			return false;
		}
		Log.Out(Logs::Detail, Logs::Combat, "Attacking with weapon: %s (%d)", weapon->GetItem()->Name, weapon->GetID());
	}
	else
		Log.Out(Logs::Detail, Logs::Combat, "Attacking without a weapon.");

	// calculate attack_skill and skillinuse depending on hand and weapon
	// also send Packet to near clients
	SkillUseTypes skillinuse;
	AttackAnimation(skillinuse, Hand, weapon);
	Log.Out(Logs::Detail, Logs::Combat, "Attacking with %s in slot %d using skill %d", weapon?weapon->GetItem()->Name:"Fist", Hand, skillinuse);
	/// Now figure out damage
	int damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon)
		hate = (weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt);

	int weapon_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && weapon_damage > 1)
		hate = weapon_damage;

	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if(weapon_damage > 0) {
		//Berserker Berserk damage bonus
		if(berserk && (GetClass() == BERSERKER)){
			int bonus = (3 + GetLevel() / 10);		//unverified
			weapon_damage = (weapon_damage * (100 + bonus) / 100);
			Log.Out(Logs::Detail, Logs::Combat, "Berserker damage bonus increases DMG to %d", weapon_damage);
		}

		//try a finishing blow.. if successful end the attack
		if(TryFinishingBlow(other, skillinuse))
			return true;

		//damage formula needs some work
		int min_hit = 1;
		int max_hit = ((2 * weapon_damage * GetDamageTable(skillinuse)) / 100);

		if(GetLevel() < 10 && max_hit > RuleI(Combat, HitCapPre10))
			max_hit = (RuleI(Combat, HitCapPre10));
		else if(GetLevel() < 20 && max_hit > RuleI(Combat, HitCapPre20))
			max_hit = (RuleI(Combat, HitCapPre20));

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
		if (Hand == EQEmu::legacy::SlotPrimary && GetLevel() >= 28 && IsWarriorClass()) {
			// Damage bonuses apply only to hits from the main hand (Hand == MainPrimary) by characters level 28 and above
			// who belong to a melee class. If we're here, then all of these conditions apply.
			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const Item_Struct*) nullptr);
			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		//Live AA - Sinister Strikes *Adds weapon damage bonus to offhand weapon.
		if (Hand == EQEmu::legacy::SlotSecondary) {
			if (aabonuses.SecondaryDmgInc || itembonuses.SecondaryDmgInc || spellbonuses.SecondaryDmgInc){
				ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const Item_Struct*) nullptr);
				min_hit += (int) ucDamageBonus;
				max_hit += (int) ucDamageBonus;
				hate += ucDamageBonus;
			}
		}

		min_hit = (min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100);

		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = zone->random.Int(min_hit, max_hit);

		Log.Out(Logs::Detail, Logs::Combat, "Damage calculated to %d (min %d, max %d, str %d, skill %d, DMG %d, lv %d)",
			damage, min_hit, max_hit, GetSTR(), GetSkill(skillinuse), weapon_damage, GetLevel());

		if(opts) {
			damage *= opts->damage_percent;
			damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
		}

		//check to see if we hit..
		if (other->AvoidDamage(this, damage, Hand)) {
			if (!FromRiposte && !IsStrikethrough) {
				int strike_through = itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough;
				if(strike_through && zone->random.Roll(strike_through)) {
					Message_StringID(MT_StrikeThrough, STRIKETHROUGH_STRING); // You strike through your opponents defenses!
					Attack(other, Hand, false, true); // Strikethrough only gives another attempted hit
					return false;
				}
				if (damage == -3 && !FromRiposte) {
					DoRiposte(other);
					if (HasDied())
						return false;
				}
			}
		} else {
			if (other->CheckHitChance(this, skillinuse, Hand)) {
				other->MeleeMitigation(this, damage, min_hit, opts);
				ApplyMeleeDamageBonus(skillinuse, damage);
				damage += ((itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse) / 100) + GetSkillDmgAmt(skillinuse));
				TryCriticalHit(other, skillinuse, damage, opts);
			} else {
				damage = 0;
			}
		}
		Log.Out(Logs::Detail, Logs::Combat, "Final damage after all reductions: %d", damage);
	}
	else
		damage = -5;

	// Hate Generation is on a per swing basis, regardless of a hit, miss, or block, its always the same.
	// If we are this far, this means we are atleast making a swing.
	other->AddToHateList(this, hate);

	///////////////////////////////////////////////////////////
	////// Send Attack Damage
	///////////////////////////////////////////////////////////
	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);

	if (GetHP() < 0)
		return false;

	MeleeLifeTap(damage);

	if (damage > 0)
		CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);

	//break invis when you attack
	if(invisible) {
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}

	if(invisible_undead) {
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}

	if(invisible_animals){
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility vs. animals due to melee attack.");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

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

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if(GetTarget())
		TriggerDefensiveProcs(other, Hand, true, damage);

	if (damage > 0)
		return true;
	else
		return false;
}

int32 Bot::CalcBotAAFocus(BotfocusType type, uint32 aa_ID, uint32 points, uint16 spell_id)
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
			/*
			case SE_SympatheticProc:
			{
				if(type == focusSympatheticProc)
				{
					float ProcChance, ProcBonus;
					int16 ProcRateMod = base1; //Baseline is 100 for most Sympathetic foci
					int32 cast_time = GetActSpellCasttime(spell_id, spells[spell_id].cast_time);
					GetSympatheticProcChances(ProcBonus, ProcChance, cast_time, ProcRateMod);

					if(zone->random.Real(0, 1) <= ProcChance)
						value = focus_id;

					else
						value = 0;
				}
				break;
			}*/
			case SE_FcDamageAmt: {
				if(type == focusFcDamageAmt)
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

int32 Bot::GetBotFocusEffect(BotfocusType bottype, uint16 spell_id) {
	if (IsBardSong(spell_id) && bottype != BotfocusFcBaseEffects)
		return 0;

	int32 realTotal = 0;
	int32 realTotal2 = 0;
	int32 realTotal3 = 0;
	bool rand_effectiveness = false;
	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if((bottype == BotfocusManaCost || bottype == BotfocusImprovedHeal || bottype == BotfocusImprovedDamage) && RuleB(Spells, LiveLikeFocusEffects))
		rand_effectiveness = true;

	//Check if item focus effect exists for the client.
	if (itembonuses.FocusEffects[bottype]) {
		const Item_Struct* TempItem = 0;
		const Item_Struct* UsedItem = 0;
		const ItemInst* TempInst = 0;
		uint16 UsedFocusID = 0;
		int32 Total = 0;
		int32 focus_max = 0;
		int32 focus_max_real = 0;
		//item focus
		for (int x = EQEmu::legacy::EQUIPMENT_BEGIN; x <= EQEmu::legacy::EQUIPMENT_END; x++) {
			TempItem = nullptr;
			ItemInst* ins = GetBotItem(x);
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

			for (int y = AUG_INDEX_BEGIN; y < EQEmu::legacy::ITEM_COMMON_SIZE; ++y) {
				ItemInst *aug = nullptr;
				aug = ins->GetAugment(y);
				if(aug) {
					const Item_Struct* TempItemAug = aug->GetItem();
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

	if(bottype == BotfocusReagentCost && IsSummonPetSpell(spell_id) && GetAA(aaElementalPact))
		return 100;

	if(bottype == BotfocusReagentCost && (IsEffectInSpell(spell_id, SE_SummonItem) || IsSacrificeSpell(spell_id)))
		return 0;

	return (realTotal + realTotal2);
}

int32 Bot::CalcBotFocusEffect(BotfocusType bottype, uint16 focus_id, uint16 spell_id, bool best_focus) {
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
						Log.Out(Logs::General, Logs::Normal, "CalcFocusEffect: unknown limit spelltype %d", focus_spell.base[i]);
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
				if (bottype == BotfocusImprovedDamage) {
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
				if (bottype == BotfocusImprovedHeal) {
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
				if (bottype == BotfocusManaCost) {
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
				if (bottype == BotfocusSpellHaste && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_IncreaseSpellDuration:
				if (bottype == BotfocusSpellDuration && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SpellDurationIncByTic:
				if (bottype == BotfocusSpellDurByTic && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SwarmPetDuration:
				if (bottype == BotfocusSwarmPetDuration && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_IncreaseRange:
				if (bottype == BotfocusRange && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_ReduceReagentCost:
				if (bottype == BotfocusReagentCost && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_PetPowerIncrease:
				if (bottype == BotfocusPetPower && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SpellResistReduction:
				if (bottype == BotfocusResistRate && focus_spell.base[i] > value)
					value = focus_spell.base[i];
				break;
			case SE_SpellHateMod:
				if (bottype == BotfocusSpellHateMod) {
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
				if(bottype == BotfocusReduceRecastTime)
					value = (focus_spell.base[i] / 1000);
				break;
			}
			case SE_TriggerOnCast: {
				if(bottype == BotfocusTriggerOnCast) {
					if(zone->random.Int(0, 100) <= focus_spell.base[i])
						value = focus_spell.base2[i];
					else
						value = 0;
				}
				break;
			}
			case SE_FcSpellVulnerability: {
				if(bottype == BotfocusSpellVulnerability)
					value = focus_spell.base[i];
				break;
			}
			case SE_BlockNextSpellFocus: {
				if(bottype == BotfocusBlockNextSpell) {
					if(zone->random.Int(1, 100) <= focus_spell.base[i])
						value = 1;
				}
				break;
			}
			case SE_FcTwincast: {
				if(bottype == BotfocusTwincast)
					value = focus_spell.base[i];
				break;
			}
			case SE_SympatheticProc: {
				if(bottype == BotfocusSympatheticProc) {
					float ProcChance = GetSympatheticProcChances(spell_id, focus_spell.base[i]);
					if(zone->random.Real(0, 1) <= ProcChance)
						value = focus_id;
					else
						value = 0;
				}
				break;
			}
			case SE_FcDamageAmt: {
				if(bottype == BotfocusFcDamageAmt)
					value = focus_spell.base[i];
				break;
			}
			case SE_FcDamageAmtCrit: {
				if(bottype == BotfocusFcDamageAmtCrit)
					value = focus_spell.base[i];
				break;
			}
			case SE_FcHealAmtIncoming:
				if(bottype == BotfocusFcHealAmtIncoming)
					value = focus_spell.base[i];
				break;
			case SE_FcHealPctCritIncoming:
				if (bottype == BotfocusFcHealPctCritIncoming)
					value = focus_spell.base[i];
				break;
			case SE_FcHealAmtCrit:
				if(bottype == BotfocusFcHealAmtCrit)
					value = focus_spell.base[i];
				break;
			case  SE_FcHealAmt:
				if(bottype == BotfocusFcHealAmt)
					value = focus_spell.base[i];
				break;
			case SE_FcHealPctIncoming:
				if(bottype == BotfocusFcHealPctIncoming)
					value = focus_spell.base[i];
				break;
			case SE_FcBaseEffects: {
				if (bottype == BotfocusFcBaseEffects)
					value = focus_spell.base[i];

				break;
			}
			case SE_FcDamagePctCrit: {
				if(bottype == BotfocusFcDamagePctCrit)
					value = focus_spell.base[i];

				break;
			}
			case SE_FcIncreaseNumHits: {
				if(bottype == BotfocusIncreaseNumHits)
					value = focus_spell.base[i];

				break;
			}
			default:
				Log.Out(Logs::General, Logs::Spells, "CalcFocusEffect: unknown effectid %d", focus_spell.effectid[i]);
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
	case EQEmu::legacy::SlotPrimary:
		weapon_speed = attack_timer.GetDuration();
		break;
	case EQEmu::legacy::SlotSecondary:
		weapon_speed = attack_dw_timer.GetDuration();
		break;
	case EQEmu::legacy::SlotRange:
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

	Log.Out(Logs::Detail, Logs::Combat, "Proc chance %.2f (%.2f from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

int Bot::GetHandToHandDamage(void) {
	if (RuleB(Combat, UseRevampHandToHand)) {
		// everyone uses this in the revamp!
		int skill = GetSkill(SkillHandtoHand);
		int epic = 0;
		if (CastToNPC()->GetEquipment(EQEmu::legacy::MaterialHands) == 10652 && GetLevel() > 46)
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
		if (CastToNPC()->GetEquipment(EQEmu::legacy::MaterialHands) == 10652 && GetLevel() > 50)
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

bool Bot::TryFinishingBlow(Mob *defender, SkillUseTypes skillinuse) {
	if (!defender)
		return false;

	if (aabonuses.FinishingBlow[1] && !defender->IsClient() && defender->GetHPRatio() < 10) {
		uint32 chance = (aabonuses.FinishingBlow[0] / 10);
		uint32 damage = aabonuses.FinishingBlow[1];
		uint16 levelreq = aabonuses.FinishingBlowLvl[0];
		if(defender->GetLevel() <= levelreq && (chance >= zone->random.Int(0, 1000))){
			Log.Out(Logs::Detail, Logs::Combat, "Landed a finishing blow: levelreq at %d, other level %d", levelreq , defender->GetLevel());
			entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, FINISHING_BLOW, GetName());
			defender->Damage(this, damage, SPELL_UNKNOWN, skillinuse);
			return true;
		} else {
			Log.Out(Logs::Detail, Logs::Combat, "FAILED a finishing blow: levelreq at %d, other level %d", levelreq , defender->GetLevel());
			return false;
		}
	}
	return false;
}

void Bot::DoRiposte(Mob* defender) {
	Log.Out(Logs::Detail, Logs::Combat, "Preforming a riposte");
	if (!defender)
		return;

	defender->Attack(this, EQEmu::legacy::SlotPrimary, true);
	int32 DoubleRipChance = (defender->GetAABonuses().GiveDoubleRiposte[0] + defender->GetSpellBonuses().GiveDoubleRiposte[0] + defender->GetItemBonuses().GiveDoubleRiposte[0]);
	if(DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		Log.Out(Logs::Detail, Logs::Combat, "Preforming a double riposte (%d percent chance)", DoubleRipChance);
		defender->Attack(this, EQEmu::legacy::SlotPrimary, true);
	}

	DoubleRipChance = defender->GetAABonuses().GiveDoubleRiposte[1];
	if(DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		if (defender->GetClass() == MONK)
			defender->MonkSpecialAttack(this, defender->GetAABonuses().GiveDoubleRiposte[2]);
		else if (defender->IsBot())
			defender->CastToClient()->DoClassAttacks(this,defender->GetAABonuses().GiveDoubleRiposte[2], true);
	}
}

void Bot::DoSpecialAttackDamage(Mob *who, SkillUseTypes skill, int32 max_damage, int32 min_damage, int32 hate_override,int ReuseTime, bool HitChance) {
	int32 hate = max_damage;
	if(hate_override > -1)
		hate = hate_override;

	if(skill == SkillBash) {
		const ItemInst* inst = GetBotItem(EQEmu::legacy::SlotSecondary);
		const Item_Struct* botweapon = 0;
		if(inst)
			botweapon = inst->GetItem();

		if(botweapon) {
			if(botweapon->ItemType == ItemTypeShield)
				hate += botweapon->AC;

			hate = (hate * (100 + GetFuriousBash(botweapon->Focus.Effect)) / 100);
		}
	}

	min_damage += (min_damage * GetMeleeMinDamageMod_SE(skill) / 100);
	int hand = EQEmu::legacy::SlotPrimary;
	if (skill == SkillThrowing || skill == SkillArchery)
		hand = EQEmu::legacy::SlotRange;
	if (who->AvoidDamage(this, max_damage, hand)) {
		if (max_damage == -3)
			DoRiposte(who);
	} else {
		if (HitChance || who->CheckHitChance(this, skill, EQEmu::legacy::SlotPrimary)) {
			who->MeleeMitigation(this, max_damage, min_damage);
			ApplyMeleeDamageBonus(skill, max_damage);
			max_damage += who->GetFcDamageAmtIncoming(this, 0, true, skill);
			max_damage += ((itembonuses.HeroicSTR / 10) + (max_damage * who->GetSkillDmgTaken(skill) / 100) + GetSkillDmgAmt(skill));
			TryCriticalHit(who, skill, max_damage);
		} else {
			max_damage = 0;
		}
	}

	who->AddToHateList(this, hate);

	who->Damage(this, max_damage, SPELL_UNKNOWN, skill, false);

	if(!GetTarget() || HasDied())
		return;

	if (max_damage > 0)
		CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);

	//[AA Dragon Punch] value[0] = 100 for 25%, chance value[1] = skill
/* 	if(aabonuses.SpecialAttackKBProc[0] && aabonuses.SpecialAttackKBProc[1] == skill){
		int kb_chance = 25;
		kb_chance += (kb_chance * (100 - aabonuses.SpecialAttackKBProc[0]) / 100);

		if (zone->random.Int(0, 99) < kb_chance)
			SpellFinished(904, who, 10, 0, -1, spells[904].ResistDiff);
			//who->Stun(100); Kayen: This effect does not stun on live, it only moves the NPC.
	}*/

	if (HasSkillProcs())
		TrySkillProc(who, skill, (ReuseTime * 1000));

	if (max_damage > 0 && HasSkillProcSuccess())
		TrySkillProc(who, skill, (ReuseTime * 1000), true);
}

void Bot::TryBackstab(Mob *other, int ReuseTime) {
	if(!other)
		return;

	bool bIsBehind = false;
	bool bCanFrontalBS = false;
	const ItemInst* inst = GetBotItem(EQEmu::legacy::SlotPrimary);
	const Item_Struct* botpiercer = nullptr;
	if(inst)
		botpiercer = inst->GetItem();

	if(!botpiercer || (botpiercer->ItemType != ItemType1HPiercing)) {
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
			entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, ASSASSINATES, GetName());
			RogueAssassinate(other);
		} else {
			RogueBackstab(other);
			if (level > 54) {
				float DoubleAttackProbability = ((GetSkill(SkillDoubleAttack) + GetLevel()) / 500.0f);
				if(zone->random.Real(0, 1) < DoubleAttackProbability) {
					if(other->GetHP() > 0)
						RogueBackstab(other,false,ReuseTime);

					if (tripleChance && other->GetHP() > 0 && tripleChance > zone->random.Int(0, 100))
						RogueBackstab(other,false,ReuseTime);
				}
			}
		}
	} else if(aabonuses.FrontalBackstabMinDmg || itembonuses.FrontalBackstabMinDmg || spellbonuses.FrontalBackstabMinDmg) {
		RogueBackstab(other, true);
		if (level > 54) {
			float DoubleAttackProbability = ((GetSkill(SkillDoubleAttack) + GetLevel()) / 500.0f);
			if(zone->random.Real(0, 1) < DoubleAttackProbability)
				if(other->GetHP() > 0)
					RogueBackstab(other,true, ReuseTime);

			if (tripleChance && other->GetHP() > 0 && tripleChance > zone->random.Int(0, 100))
				RogueBackstab(other,false,ReuseTime);
		}
	}
	else
		Attack(other, EQEmu::legacy::SlotPrimary);
}

void Bot::RogueBackstab(Mob* other, bool min_damage, int ReuseTime) {
	int32 ndamage = 0;
	int32 max_hit = 0;
	int32 min_hit = 0;
	int32 hate = 0;
	int32 primaryweapondamage = 0;
	int32 backstab_dmg = 0;
	ItemInst* botweaponInst = GetBotItem(EQEmu::legacy::SlotPrimary);
	if(botweaponInst) {
		primaryweapondamage = GetWeaponDamage(other, botweaponInst);
		backstab_dmg = botweaponInst->GetItem()->BackstabDmg;
		for (int i = AUG_INDEX_BEGIN; i < EQEmu::legacy::ITEM_COMMON_SIZE; ++i) {
			ItemInst *aug = botweaponInst->GetAugment(i);
			if(aug)
				backstab_dmg += aug->GetItem()->BackstabDmg;
		}
	} else {
		primaryweapondamage = ((GetLevel() / 7) + 1);
		backstab_dmg = primaryweapondamage;
	}

	if(primaryweapondamage > 0) {
		if(level > 25) {
			max_hit = (((((2 * backstab_dmg) * GetDamageTable(SkillBackstab) / 100) * 10 * GetSkill(SkillBackstab) / 355) + ((level - 25) / 3) + 1) * ((100 + RuleI(Combat, BackstabBonus)) / 100));
			hate = (20 * backstab_dmg * GetSkill(SkillBackstab) / 355);
		} else {
			max_hit = (((((2 * backstab_dmg) * GetDamageTable(SkillBackstab) / 100) * 10 * GetSkill(SkillBackstab) / 355) + 1) * ((100 + RuleI(Combat, BackstabBonus)) / 100));
			hate = (20 * backstab_dmg * GetSkill(SkillBackstab) / 355);
		}

		if (level < 51)
			min_hit = (level * 15 / 10);
		else
			min_hit = ((level * ( level * 5 - 105)) / 100);

		if(!other->CheckHitChance(this, SkillBackstab, 0))
			ndamage = 0;
		else {
			if (min_damage) {
				ndamage = min_hit;
			} else {
				if (max_hit < min_hit)
					max_hit = min_hit;

				ndamage = (RuleB(Combat, UseIntervalAC) ? max_hit : zone->random.Int(min_hit, max_hit));
			}
		}
	} else
		ndamage = -5;

	DoSpecialAttackDamage(other, SkillBackstab, ndamage, min_hit, hate, ReuseTime);
	DoAnim(anim1HPiercing);
}

void Bot::RogueAssassinate(Mob* other) {
	ItemInst* botweaponInst = GetBotItem(EQEmu::legacy::SlotPrimary);
	if(botweaponInst) {
		if(GetWeaponDamage(other, botweaponInst))
			other->Damage(this, 32000, SPELL_UNKNOWN, SkillBackstab);
		else
			other->Damage(this, -5, SPELL_UNKNOWN, SkillBackstab);
	}

	DoAnim(anim1HPiercing);
}

void Bot::DoClassAttacks(Mob *target, bool IsRiposte) {
	if(!target || spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || GetHP() < 0 || !IsAttackAllowed(target))
		return;

	bool taunt_time = taunt_timer.Check();
	bool ca_time = classattack_timer.Check(false);
	bool ka_time = knightattack_timer.Check(false);
	if((taunt_time || ca_time || ka_time) && !IsAttackAllowed(target))
		return;

	if(ka_time){
		int knightreuse = 1000;
		switch(GetClass()){
			case SHADOWKNIGHT:
			case SHADOWKNIGHTGM: {
				CastSpell(SPELL_NPC_HARM_TOUCH, target->GetID());
				knightreuse = (HarmTouchReuseTime * 1000);
				break;
			}
			case PALADIN:
			case PALADINGM: {
				if(GetHPRatio() < 20) {
					CastSpell(SPELL_LAY_ON_HANDS, GetID());
					knightreuse = (LayOnHandsReuseTime * 1000);
				}
				else
					knightreuse = 2000;

				break;
			}
		}
		knightattack_timer.Start(knightreuse);
	}

	if(taunting && target && target->IsNPC() && taunt_time) {
		if(GetTarget() && GetTarget()->GetHateTop() && GetTarget()->GetHateTop() != this) {
			BotGroupSay(this, "Taunting %s", target->GetCleanName());
			Taunt(target->CastToNPC(), false);
			taunt_timer.Start(TauntReuseTime * 1000);
		}
	}

	if(!ca_time)
		return;

	float HasteModifier = (GetHaste() * 0.01f);
	int32 dmg = 0;
	uint16 skill_to_use = -1;
	int level = GetLevel();
	int reuse = (TauntReuseTime * 1000);
	bool did_attack = false;
	switch(GetClass()) {
		case WARRIOR:
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				bool canBash = false;
				if ((GetRace() == OGRE || GetRace() == TROLL || GetRace() == BARBARIAN) || (m_inv.GetItem(EQEmu::legacy::SlotSecondary) && m_inv.GetItem(EQEmu::legacy::SlotSecondary)->GetItem()->ItemType == ItemTypeShield) || (m_inv.GetItem(EQEmu::legacy::SlotPrimary) && (m_inv.GetItem(EQEmu::legacy::SlotPrimary)->GetItem()->ItemType == ItemType2HSlash || m_inv.GetItem(EQEmu::legacy::SlotPrimary)->GetItem()->ItemType == ItemType2HBlunt || m_inv.GetItem(EQEmu::legacy::SlotPrimary)->GetItem()->ItemType == ItemType2HPiercing) && GetAA(aa2HandBash) >= 1))
					canBash = true;

				if(!canBash || zone->random.Int(0, 100) > 25)
					skill_to_use = SkillKick;
				else
					skill_to_use = SkillBash;
			}
		case RANGER:
		case BEASTLORD:
			skill_to_use = SkillKick;
			break;
		case BERSERKER:
			skill_to_use = SkillFrenzy;
			break;
		case CLERIC:
		case SHADOWKNIGHT:
		case PALADIN:
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				if ((GetRace() == OGRE || GetRace() == TROLL || GetRace() == BARBARIAN) || (m_inv.GetItem(EQEmu::legacy::SlotSecondary) && m_inv.GetItem(EQEmu::legacy::SlotSecondary)->GetItem()->ItemType == ItemTypeShield) || (m_inv.GetItem(EQEmu::legacy::SlotPrimary) && (m_inv.GetItem(EQEmu::legacy::SlotPrimary)->GetItem()->ItemType == ItemType2HSlash || m_inv.GetItem(EQEmu::legacy::SlotPrimary)->GetItem()->ItemType == ItemType2HBlunt || m_inv.GetItem(EQEmu::legacy::SlotPrimary)->GetItem()->ItemType == ItemType2HPiercing) && GetAA(aa2HandBash) >= 1))
					skill_to_use = SkillBash;
			}
			break;
		case MONK:
			if(GetLevel() >= 30)
				skill_to_use = SkillFlyingKick;
			else if(GetLevel() >= 25)
				skill_to_use = SkillDragonPunch;
			else if(GetLevel() >= 20)
				skill_to_use = SkillEagleStrike;
			else if(GetLevel() >= 10)
				skill_to_use = SkillTigerClaw;
			else if(GetLevel() >= 5)
				skill_to_use = SkillRoundKick;
			else
				skill_to_use = SkillKick;
			break;
		case ROGUE:
			skill_to_use = SkillBackstab;
			break;
	}

	if(skill_to_use == -1)
		return;

	if(skill_to_use == SkillBash) {
		if (target != this) {
			DoAnim(animTailRake);
			if (GetWeaponDamage(target, GetBotItem(EQEmu::legacy::SlotSecondary)) <= 0 && GetWeaponDamage(target, GetBotItem(EQEmu::legacy::SlotShoulders)) <= 0)
				dmg = -5;
			else {
				if(!target->CheckHitChance(this, SkillBash, 0))
					dmg = 0;
				else {
					if(RuleB(Combat, UseIntervalAC))
						dmg = GetBashDamage();
					else
						dmg = zone->random.Int(1, GetBashDamage());
				}
			}
			reuse = (BashReuseTime * 1000);
			DoSpecialAttackDamage(target, SkillBash, dmg, 1, -1,reuse);
			did_attack = true;
		}
	}

	if(skill_to_use == SkillFrenzy) {
		int AtkRounds = 3;
		int skillmod = 0;
		if(MaxSkill(SkillFrenzy) > 0)
			skillmod = (100 * GetSkill(SkillFrenzy)/MaxSkill(SkillFrenzy));

		int32 max_dmg = (26 + ((((GetLevel() - 6) * 2) * skillmod) / 100)) * ((100 + RuleI(Combat, FrenzyBonus)) / 100);
		int32 min_dmg = 0;
		DoAnim(anim2HSlashing);

		if (GetLevel() < 51)
			min_dmg = 1;
		else
			min_dmg = (GetLevel() * 8 / 10);

		if (min_dmg > max_dmg)
			max_dmg = min_dmg;

		reuse = (FrenzyReuseTime * 1000);
		did_attack = true;
		while(AtkRounds > 0) {
			if (GetTarget() && (AtkRounds == 1 || zone->random.Int(0, 100) < 75)) {
				DoSpecialAttackDamage(GetTarget(), SkillFrenzy, max_dmg, min_dmg, max_dmg, reuse, true);
			}

			AtkRounds--;
		}
	}

	if(skill_to_use == SkillKick) {
		if(target != this) {
			DoAnim(animKick);
			if (GetWeaponDamage(target, GetBotItem(EQEmu::legacy::SlotFeet)) <= 0)
				dmg = -5;
			else {
				if(!target->CheckHitChance(this, SkillKick, 0))
					dmg = 0;
				else {
					if(RuleB(Combat, UseIntervalAC))
						dmg = GetKickDamage();
					else
						dmg = zone->random.Int(1, GetKickDamage());
				}
			}
			reuse = (KickReuseTime * 1000);
			DoSpecialAttackDamage(target, SkillKick, dmg, 1, -1, reuse);
			did_attack = true;
		}
	}

	if(skill_to_use == SkillFlyingKick || skill_to_use == SkillDragonPunch || skill_to_use == SkillEagleStrike || skill_to_use == SkillTigerClaw || skill_to_use == SkillRoundKick) {
		reuse = (MonkSpecialAttack(target, skill_to_use) - 1);
		MonkSpecialAttack(target, skill_to_use);
		uint32 bDoubleSpecialAttack = (itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack);
		if(bDoubleSpecialAttack && (bDoubleSpecialAttack >= 100 || bDoubleSpecialAttack > zone->random.Int(0, 100))) {
			int MonkSPA [5] = { SkillFlyingKick, SkillDragonPunch, SkillEagleStrike, SkillTigerClaw, SkillRoundKick };
			MonkSpecialAttack(target, MonkSPA[zone->random.Int(0, 4)]);
			int TripleChance = 25;
			if (bDoubleSpecialAttack > 100)
				TripleChance += (TripleChance * (100 - bDoubleSpecialAttack) / 100);

			if(TripleChance > zone->random.Int(0,100))
				MonkSpecialAttack(target, MonkSPA[zone->random.Int(0, 4)]);
		}

		reuse *= 1000;
		did_attack = true;
	}

	if(skill_to_use == SkillBackstab) {
		reuse = (BackstabReuseTime * 1000);
		did_attack = true;
		if (IsRiposte)
			reuse = 0;

		TryBackstab(target,reuse);
	}
	classattack_timer.Start(reuse / HasteModifier);
}

bool Bot::TryHeadShot(Mob* defender, SkillUseTypes skillInUse) {
	bool Result = false;
	if(defender && (defender->GetBodyType() == BT_Humanoid) && (skillInUse == SkillArchery) && (GetClass() == RANGER) && (GetLevel() >= 62)) {
		int defenderLevel = defender->GetLevel();
		int rangerLevel = GetLevel();
		if(GetAA(aaHeadshot) && ((defenderLevel - 46) <= GetAA(aaHeadshot) * 2)) {
			float AttackerChance = 0.20f + ((float)(rangerLevel - 51) * 0.005f);
			float DefenderChance = (float)zone->random.Real(0.00f, 1.00f);
			if(AttackerChance > DefenderChance) {
				Log.Out(Logs::Detail, Logs::Combat, "Landed a headshot: Attacker chance was %f and Defender chance was %f.", AttackerChance, DefenderChance);
				entity_list.MessageClose(this, false, 200, MT_CritMelee, "%s has scored a leathal HEADSHOT!", GetName());
				defender->Damage(this, (defender->GetMaxHP()+50), SPELL_UNKNOWN, skillInUse);
				Result = true;
			} else
				Log.Out(Logs::Detail, Logs::Combat, "FAILED a headshot: Attacker chance was %f and Defender chance was %f.", AttackerChance, DefenderChance);
		}
	}
	return Result;
}

int32 Bot::CheckAggroAmount(uint16 spellid) {
	int32 AggroAmount = Mob::CheckAggroAmount(spellid, nullptr);
	int32 focusAggro = GetBotFocusEffect(BotfocusSpellHateMod, spellid);
	AggroAmount = (AggroAmount * (100 + focusAggro) / 100);
	return AggroAmount;
}

int32 Bot::CheckHealAggroAmount(uint16 spellid, Mob *target, uint32 heal_possible) {
	int32 AggroAmount = Mob::CheckHealAggroAmount(spellid, target, heal_possible);
	int32 focusAggro = GetBotFocusEffect(BotfocusSpellHateMod, spellid);
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
	Mob* Result = 0;
	if(this->GetBotOwner())
		Result = GetBotOwner();
	else
		Result = this;

	return Result;
}

Mob* Bot::GetOwner() {
	Mob* Result = 0;
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
	const ItemInst* inst = 0;
	const Item_Struct* item = 0;
	for (int i = EQEmu::legacy::EQUIPMENT_BEGIN; i <= EQEmu::legacy::EQUIPMENT_END; ++i) {
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
				guildOfficer->Message(13, "You dont have permission to invite.");
				return;
			}

			if (!botdb.SaveGuildMembership(botToGuild->GetBotID(), guildOfficer->GuildID(), GUILD_MEMBER)) {
				guildOfficer->Message(13, "%s for '%s'", BotDatabase::fail::SaveGuildMembership(), botToGuild->GetCleanName());
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
			guildOfficer->Message(13, "Bot is in a guild.");
			return;
		}
	}
}

bool Bot::ProcessGuildRemoval(Client* guildOfficer, std::string botName) {
	bool Result = false;
	if(guildOfficer && !botName.empty()) {
		Bot* botToUnGuild = entity_list.GetBotByBotName(botName);
		if(botToUnGuild) {
			if (botdb.SaveGuildMembership(botToUnGuild->GetBotID(), 0, 0))
				Result = true;
		} else {
			uint32 ownerId = 0;
			if (!botdb.LoadOwnerID(botName, ownerId))
				guildOfficer->Message(13, "%s for '%s'", BotDatabase::fail::LoadOwnerID(), botName.c_str());
			uint32 botId = 0;
			if (!botdb.LoadBotID(ownerId, botName, botId))
				guildOfficer->Message(13, "%s for '%s'", BotDatabase::fail::LoadBotID(), botName.c_str());
			if (botId && botdb.SaveGuildMembership(botId, 0, 0))
				Result = true;
		}

		if(Result) {
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildManageRemove, sizeof(GuildManageRemove_Struct));
			GuildManageRemove_Struct* gm = (GuildManageRemove_Struct*) outapp->pBuffer;
			gm->guildeqid = guildOfficer->GuildID();
			strcpy(gm->member, botName.c_str());
			guildOfficer->Message(0, "%s successfully removed from your guild.", botName.c_str());
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
			Log.Out(Logs::General, Logs::None, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
			max_mana = 0;
			break;
		}
	}

	if(cur_mana > max_mana)
		cur_mana = max_mana;
	else if(max_mana < 0)
		max_mana = 0;

	return max_mana;
}

void Bot::SetAttackTimer() {
	float haste_mod = (GetHaste() * 0.01f);
	attack_timer.SetAtTrigger(4000, true);
	Timer* TimerToUse = nullptr;
	const Item_Struct* PrimaryWeapon = nullptr;
	for (int i = EQEmu::legacy::SlotRange; i <= EQEmu::legacy::SlotSecondary; i++) {
		if (i == EQEmu::legacy::SlotPrimary)
			TimerToUse = &attack_timer;
		else if (i == EQEmu::legacy::SlotRange)
			TimerToUse = &ranged_timer;
		else if (i == EQEmu::legacy::SlotSecondary)
			TimerToUse = &attack_dw_timer;
		else
			continue;

		const Item_Struct* ItemToUse = nullptr;
		ItemInst* ci = GetBotItem(i);
		if (ci)
			ItemToUse = ci->GetItem();

		if (i == EQEmu::legacy::SlotSecondary) {
			if (PrimaryWeapon != nullptr) {
				if (PrimaryWeapon->ItemClass == ItemClassCommon && (PrimaryWeapon->ItemType == ItemType2HSlash || PrimaryWeapon->ItemType == ItemType2HBlunt || PrimaryWeapon->ItemType == ItemType2HPiercing)) {
					attack_dw_timer.Disable();
					continue;
				}
			}

			if (!GetSkill(SkillDualWield)) {
				attack_dw_timer.Disable();
				continue;
			}
		}

		if (ItemToUse != nullptr) {
			if (ItemToUse->ItemClass != ItemClassCommon || ItemToUse->Damage == 0 || ItemToUse->Delay == 0 || ((ItemToUse->ItemType > ItemTypeLargeThrowing) && (ItemToUse->ItemType != ItemTypeMartial) && (ItemToUse->ItemType != ItemType2HPiercing)))
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
		TimerToUse->SetAtTrigger(std::max(RuleI(Combat, MinHastedDelay), speed), true);

		if (i == EQEmu::legacy::SlotPrimary)
			PrimaryWeapon = ItemToUse;
	}
}

int32 Bot::GetActSpellDamage(uint16 spell_id, int32 value, Mob* target) {
	if (spells[spell_id].targettype == ST_Self)
		return value;

	bool Critical = false;
	int32 value_BaseEffect = 0;
	value_BaseEffect = (value + (value*GetBotFocusEffect(BotfocusFcBaseEffects, spell_id) / 100));
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
			value += (value_BaseEffect * GetBotFocusEffect(BotfocusImprovedDamage, spell_id) / 100);
			value += (int(value_BaseEffect * GetBotFocusEffect(BotfocusFcDamagePctCrit, spell_id) / 100) * ratio / 100);
			if (target) {
				value += (int(value_BaseEffect * target->GetVulnerability(this, spell_id, 0) / 100) * ratio / 100);
				value -= target->GetFcDamageAmtIncoming(this, spell_id);
			}

			value -= (GetBotFocusEffect(BotfocusFcDamageAmtCrit, spell_id) * ratio / 100);

			value -= GetBotFocusEffect(BotfocusFcDamageAmt, spell_id);

			if(itembonuses.SpellDmg && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5)
				value += (GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value) * ratio / 100);

			entity_list.MessageClose(this, false, 100, MT_SpellCrits, "%s delivers a critical blast! (%d)", GetName(), -value);

			return value;
		}
	}

	value = value_BaseEffect;
	value += (value_BaseEffect * GetBotFocusEffect(BotfocusImprovedDamage, spell_id) / 100);
	value += (value_BaseEffect * GetBotFocusEffect(BotfocusFcDamagePctCrit, spell_id) / 100);
	if (target) {
		value += (value_BaseEffect * target->GetVulnerability(this, spell_id, 0) / 100);
		value -= target->GetFcDamageAmtIncoming(this, spell_id);
	}

	value -= GetBotFocusEffect(BotfocusFcDamageAmtCrit, spell_id);
	value -= GetBotFocusEffect(BotfocusFcDamageAmt, spell_id);
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
	value_BaseEffect = (value + (value*GetBotFocusEffect(BotfocusFcBaseEffects, spell_id) / 100));
	value = value_BaseEffect;
	value += int(value_BaseEffect*GetBotFocusEffect(BotfocusImprovedHeal, spell_id) / 100);
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
		value += (GetBotFocusEffect(BotfocusFcHealAmtCrit, spell_id) * modifier);
		value += GetBotFocusEffect(BotfocusFcHealAmt, spell_id);
		value += target->GetFocusIncoming(focusFcHealAmtIncoming, SE_FcHealAmtIncoming, this, spell_id);

		if(itembonuses.HealAmt && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5)
			value += (GetExtraSpellAmt(spell_id, itembonuses.HealAmt, value) * modifier);

		value += (value * target->GetHealRate(spell_id, this) / 100);
		if (Critical)
			entity_list.MessageClose(this, false, 100, MT_SpellCrits, "%s performs an exceptional heal! (%d)", GetName(), value);

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
	cast_reducer += GetBotFocusEffect(BotfocusSpellHaste, spell_id);
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

	int32 focus_redux = GetBotFocusEffect(BotfocusManaCost, spell_id);

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
	extrange += GetBotFocusEffect(BotfocusRange, spell_id);
	return ((range * extrange) / 100);
}

int32 Bot::GetActSpellDuration(uint16 spell_id, int32 duration) {
	int increase = 100;
	increase += GetBotFocusEffect(BotfocusSpellDuration, spell_id);
	int tic_inc = 0;	tic_inc = GetBotFocusEffect(BotfocusSpellDurByTic, spell_id);

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

bool Bot::CastSpell(uint16 spell_id, uint16 target_id, uint16 slot, int32 cast_time, int32 mana_cost,
					uint32* oSpellWillFinish, uint32 item_slot, int16 *resist_adjust, uint32 aa_id) {
	bool Result = false;
	if(zone && !zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))) {
		Log.Out(Logs::Detail, Logs::Spells, "CastSpell called for spell %s (%d) on entity %d, slot %d, time %d, mana %d, from item slot %d", spells[spell_id].name, spell_id, target_id, slot, cast_time, mana_cost, (item_slot==0xFFFFFFFF)?999:item_slot);

		if(casting_spell_id == spell_id)
			ZeroCastingVars();

		if(GetClass() != BARD) {
			if(!IsValidSpell(spell_id) || casting_spell_id || delaytimer || spellend_timer.Enabled() || IsStunned() || IsFeared() || IsMezzed() || (IsSilenced() && !IsDiscipline(spell_id)) || (IsAmnesiad() && IsDiscipline(spell_id))) {
				Log.Out(Logs::Detail, Logs::Spells, "Spell casting canceled: not able to cast now. Valid? %d, casting %d, waiting? %d, spellend? %d, stunned? %d, feared? %d, mezed? %d, silenced? %d", IsValidSpell(spell_id), casting_spell_id, delaytimer, spellend_timer.Enabled(), IsStunned(), IsFeared(), IsMezzed(), IsSilenced() );
				if(IsSilenced() && !IsDiscipline(spell_id))
					Message_StringID(13, SILENCED_STRING);

				if(IsAmnesiad() && IsDiscipline(spell_id))

					Message_StringID(13, MELEE_SILENCE);

				if(casting_spell_id)
					AI_Event_SpellCastFinished(false, casting_spell_slot);

				return false;
			}
		}

		if(IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()){
			Message_StringID(13, SPELL_WOULDNT_HOLD);
			if(casting_spell_id)
				AI_Event_SpellCastFinished(false, casting_spell_slot);

			return false;
		}

		if(DivineAura()) {
			Log.Out(Logs::Detail, Logs::Spells, "Spell casting canceled: cannot cast while Divine Aura is in effect.");
			InterruptSpell(173, 0x121, false);
			return false;
		}

		if(slot < MAX_PP_MEMSPELL && !CheckFizzle(spell_id)) {
			int fizzle_msg = IsBardSong(spell_id) ? MISS_NOTE : SPELL_FIZZLE;
			InterruptSpell(fizzle_msg, 0x121, spell_id);

			uint32 use_mana = ((spells[spell_id].mana) / 4);
			Log.Out(Logs::Detail, Logs::Spells, "Spell casting canceled: fizzled. %d mana has been consumed", use_mana);
			SetMana(GetMana() - use_mana);
			return false;
		}

		if (HasActiveSong()) {
			Log.Out(Logs::Detail, Logs::Spells, "Casting a new spell/song while singing a song. Killing old song %d.", bardsong);
			bardsong = 0;
			bardsong_target_id = 0;
			bardsong_slot = 0;
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
						Log.Out(Logs::Detail, Logs::Spells, "Bot's target is not an undead.");
						return true;
					}
				}
				if(spells[spell_id].targettype == ST_Summoned) {
					if((GetBodyType() != BT_SummonedUndead) && (GetBodyType() != BT_Summoned) && (GetBodyType() != BT_Summoned2) && (GetBodyType() != BT_Summoned3)) {
						Log.Out(Logs::Detail, Logs::Spells, "Bot's target is not a summoned creature.");
						return true;
					}
				}
			}

			Log.Out(Logs::Detail, Logs::Spells, "No bot immunities to spell %d found.", spell_id);
		}
	}

	return Result;
}

bool Bot::DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction) {
	bool Result = false;
	SpellTargetType targetType = spells[spell_id].targettype;
	if(targetType == ST_GroupClientAndPet) {
		if((spell_id == 1768 && zone->GetZoneID() == 202) || (!IsDetrimentalSpell(spell_id))) {
			CastAction = SingleTarget;
			return true;
		}
	}
	Result = Mob::DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction);
	return Result;
}

bool Bot::DoCastSpell(uint16 spell_id, uint16 target_id, uint16 slot, int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish, uint32 item_slot, uint32 aa_id) {
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
			if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
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
			if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
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

bool Bot::DoFinishedSpellAETarget(uint16 spell_id, Mob* spellTarget, uint16 slot, bool& stopLogic) {
	if(GetClass() == BARD) {
		if(!ApplyNextBardPulse(bardsong, this, bardsong_slot))
			InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);

		stopLogic = true;
	}
	return true;
}

bool Bot::DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, uint16 slot, bool& stopLogic) {
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
				bool slotequal = (slot == USE_ITEM_SPELL_SLOT);
				if(spellequal || slotequal) {
					if((spelltypeequal || spelltypetargetequal) || spelltypeclassequal || slotequal) {
						if(((spells[thespell].effectid[0] == 0) && (spells[thespell].base[0] < 0)) &&
							(spellTarget->GetHP() < ((spells[thespell].base[0] * (-1)) + 100))) {
							Log.Out(Logs::General, Logs::Spells, "Bot::DoFinishedSpellSingleTarget - GroupBuffing failure");
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

bool Bot::DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, uint16 slot, bool& stopLogic) {
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
	GenerateArmorClass();
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
	else if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoF)
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
	if(!RuleI(Character, RestRegenPercent))
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

	RestRegenHP = (GetMaxHP() * RuleI(Character, RestRegenPercent) / 100);
	RestRegenMana = (GetMaxMana() * RuleI(Character, RestRegenPercent) / 100);
	if(RuleB(Character, RestRegenEndurance))
		RestRegenEndurance = (GetMaxEndurance() * RuleI(Character, RestRegenPercent) / 100);
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
			regen = ((((GetSkill(SkillMeditate) / 10) + (level - (level / 4))) / 4) + 4);
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
	if (cur_hp > max_hp)
		cur_hp = max_hp;

	int hp_perc_cap = spellbonuses.HPPercCap[0];
	if(hp_perc_cap) {
		int curHP_cap = ((max_hp * hp_perc_cap) / 100);
		if (cur_hp > curHP_cap || (spellbonuses.HPPercCap[1] && cur_hp > spellbonuses.HPPercCap[1]))
			cur_hp = curHP_cap;
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
	if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
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

	if(IsGrouped())
		RemoveBotFromGroup(this, GetGroup());

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

bool Bot::IsBotCasterCombatRange(Mob *target) {
	bool result = false;
	if(target) {
		float range = BotAISpellRange;
		range *= range;
		range *= .5;
		float targetDistance = DistanceSquaredNoZ(m_Position, target->GetPosition());
		if(targetDistance > range)
			result = false;
		else
			result = true;
	}

	return result;
}

bool Bot::IsGroupPrimaryHealer() {
	bool result = false;
	uint8 botclass = GetClass();
	if(HasGroup()) {
		Group *g = GetGroup();
		switch(botclass) {
			case CLERIC: {
				result = true;
				break;
			}
			case DRUID: {
				result = GroupHasClericClass(g) ? false : true;
				break;
			}
			case SHAMAN: {
				result = (GroupHasClericClass(g) || GroupHasDruidClass(g)) ? false : true;
				break;
			}
			case PALADIN:
			case RANGER:
			case BEASTLORD: {
				result = GroupHasPriestClass(g) ? false : true;
				break;
			}
			default: {
				result = false;
				break;
			}
		}
	}

	return result;
}

bool Bot::IsGroupPrimarySlower() {
	bool result = false;
	uint8 botclass = GetClass();
	if(HasGroup()) {
		Group *g = GetGroup();
		switch(botclass) {
			case SHAMAN: {
				result = true;
				break;
			}
			case ENCHANTER: {
				result = GroupHasShamanClass(g) ? false : true;
				break;
			}
			case BEASTLORD: {
				result = (GroupHasShamanClass(g) || GroupHasEnchanterClass(g)) ? false : true;
				break;
			}
			default: {
				result = false;
				break;
			}
		}
	}

	return result;
}

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

bool Bot::CalculateNewPosition2(float x, float y, float z, float speed, bool checkZ) {
	return MakeNewPositionAndSendUpdate(x, y, z, speed, checkZ);
}

Bot* Bot::GetBotByBotClientOwnerAndBotName(Client* c, std::string botName) {
	Bot* Result = 0;
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
		Bot* tempBot = 0;

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
					if(g && g->GetLeader()) {
						Mob* tempGroupLeader = tempBot->GetGroup()->GetLeader();
						if(tempGroupLeader && tempGroupLeader->IsClient()) {
							if(tempBot->GetBotOwnerCharacterID() == tempGroupLeader->CastToClient()->CharacterID())
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
	Bot* Result = 0;

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

		const Item_Struct* item = 0;
		const ItemInst* inst = 0;

		// Modded to display power source items (will only show up on SoF+ client inspect windows though.)
		// I don't think bots are currently coded to use them..but, you'll have to use '#bot inventory list'
		// to see them on a Titanium client when/if they are activated.
		for (int16 L = EQEmu::legacy::EQUIPMENT_BEGIN; L <= EQEmu::legacy::SlotWaist; L++) {
			inst = inspectedBot->GetBotItem(L);

			if(inst) {
				item = inst->GetItem();
				if(item) {
					strcpy(insr->itemnames[L], item->Name);
					insr->itemicons[L] = item->Icon;
				}
				else
					insr->itemicons[L] = 0xFFFFFFFF;
			}
		}

		inst = inspectedBot->GetBotItem(EQEmu::legacy::SlotPowerSource);

		if(inst) {
			item = inst->GetItem();
			if(item) {
				strcpy(insr->itemnames[SoF::inventory::SlotPowerSource], item->Name);
				insr->itemicons[SoF::inventory::SlotPowerSource] = item->Icon;
			}
			else
				insr->itemicons[SoF::inventory::SlotPowerSource] = 0xFFFFFFFF;
		}

		inst = inspectedBot->GetBotItem(EQEmu::legacy::SlotAmmo);

		if(inst) {
			item = inst->GetItem();
			if(item) {
				strcpy(insr->itemnames[SoF::inventory::SlotAmmo], item->Name);
				insr->itemicons[SoF::inventory::SlotAmmo] = item->Icon;
			}
			else
				insr->itemicons[SoF::inventory::SlotAmmo] = 0xFFFFFFFF;
		}

		strcpy(insr->text, inspectedBot->GetInspectMessage().text);

		client->QueuePacket(outapp); // Send answer to requester
	}
}

void Bot::CalcItemBonuses(StatBonuses* newbon)
{
	const Item_Struct* itemtmp = 0;

	for (int i = EQEmu::legacy::EQUIPMENT_BEGIN; i <= (EQEmu::legacy::EQUIPMENT_END + 1); ++i) {
		const ItemInst* item = GetBotItem((i == 22 ? 9999 : i));
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

void Bot::AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug, bool isTribute, int rec_override) {
	if(!inst || !inst->IsType(ItemClassCommon))
	{
		return;
	}

	if(inst->GetAugmentType()==0 && isAug == true)
	{
		return;
	}

	const Item_Struct *item = inst->GetItem();

	if(!isTribute && !inst->IsEquipable(GetBaseRace(),GetClass()))
	{
		if(item->ItemType != ItemTypeFood && item->ItemType != ItemTypeDrink)
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
	if (item->Worn.Effect > 0 && item->Worn.Type == ET_WornEffect) {// latent effects
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, item->Worn.Type);
	}

	if (item->Focus.Effect>0 && (item->Focus.Type == ET_Focus)) { // focus effects
		ApplySpellsBonuses(item->Focus.Effect, item->Focus.Level, newbon, 0);
	}

	switch(item->BardType)
	{
	case 51: /* All (e.g. Singing Short Sword) */
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
	case 50: /* Singing */
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			break;
		}
	case 23: /* Wind */
		{
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case 24: /* stringed */
		{
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			break;
		}
	case 25: /* brass */
		{
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			break;
		}
	case 26: /* Percussion */
		{
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			break;
		}
	}

	if (item->SkillModValue != 0 && item->SkillModType <= HIGHEST_SKILL){
		if ((item->SkillModValue > 0 && newbon->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && newbon->skillmod[item->SkillModType] > item->SkillModValue))
		{
			newbon->skillmod[item->SkillModType] = item->SkillModValue;
		}
	}

	if (item->ExtraDmgSkill != 0 && item->ExtraDmgSkill <= HIGHEST_SKILL) {
		if((newbon->SkillDamageAmount[item->ExtraDmgSkill] + item->ExtraDmgAmt) > RuleI(Character, ItemExtraDmgCap))
			newbon->SkillDamageAmount[item->ExtraDmgSkill] = RuleI(Character, ItemExtraDmgCap);
		else
			newbon->SkillDamageAmount[item->ExtraDmgSkill] += item->ExtraDmgAmt;
	}

	if (!isAug)
	{
		for (int i = 0; i < EQEmu::legacy::ITEM_COMMON_SIZE; i++)
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
		GetBotOwner()->Message(15, "Updating %s...", GetCleanName());
	}

	if(!IsValidRaceClassCombo()) {
		GetBotOwner()->Message(15, "A %s - %s bot was detected. Is this Race/Class combination allowed?.", GetRaceIDName(GetRace()), GetClassIDName(GetClass(), GetLevel()));
		GetBotOwner()->Message(15, "Previous Bots Code releases did not check Race/Class combinations during create.");
		GetBotOwner()->Message(15, "Unless you are experiencing heavy lag, you should delete and remake this bot.");
	}

	if(GetBotOwner()->GetLevel() != GetLevel())
		SetLevel(GetBotOwner()->GetLevel());

	GenerateSpecialAttacks();

	if(showtext) {
		GetBotOwner()->Message(15, "Base stats:");
		GetBotOwner()->Message(15, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), base_hp, AC, max_mana, STR, STA, DEX, AGI, INT, WIS, CHA);
		GetBotOwner()->Message(15, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",MR,PR,FR,CR,DR,Corrup);
		// Test Code
		if(GetClass() == BARD)
			GetBotOwner()->Message(15, "Bard Skills-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i",
				GetSkill(SkillBrassInstruments), GetSkill(SkillPercussionInstruments), GetSkill(SkillSinging), GetSkill(SkillStringedInstruments), GetSkill(SkillWindInstruments));
	}

	/*if(this->Save())
		this->GetBotOwner()->CastToClient()->Message(0, "%s saved.", this->GetCleanName());
	else
		this->GetBotOwner()->CastToClient()->Message(13, "%s save failed!", this->GetCleanName());*/

	CalcBonuses();

	AI_AddNPCSpells(this->GetBotSpellID());

	if(showtext) {
		GetBotOwner()->Message(15, "%s has been updated.", GetCleanName());
		GetBotOwner()->Message(15, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), max_hp, GetAC(), max_mana, GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA());
		GetBotOwner()->Message(15, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",GetMR(),GetPR(),GetFR(),GetCR(),GetDR(),GetCorrup());
		// Test Code
		if(GetClass() == BARD) {
			GetBotOwner()->Message(15, "Bard Skills-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i",
				GetSkill(SkillBrassInstruments) + GetBrassMod(),
				GetSkill(SkillPercussionInstruments) + GetPercMod(),
				GetSkill(SkillSinging) + GetSingMod(),
				GetSkill(SkillStringedInstruments) + GetStringMod(),
				GetSkill(SkillWindInstruments) + GetWindMod());
			GetBotOwner()->Message(15, "Bard Skill Mods-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i", GetBrassMod(), GetPercMod(), GetSingMod(), GetStringMod(), GetWindMod());
		}
	}
}

bool Bot::CheckLoreConflict(const Item_Struct* item) {
	if (!item || !(item->LoreFlag))
		return false;

	if (item->LoreGroup == -1)	// Standard lore items; look everywhere except the shared bank, return the result
		return (m_inv.HasItem(item->ID, 0, invWhereWorn) != INVALID_INDEX);

	//If the item has a lore group, we check for other items with the same group and return the result
	return (m_inv.HasItemByLoreGroup(item->LoreGroup, invWhereWorn) != INVALID_INDEX);
}

bool Bot::GroupHasClass(Group* group, uint8 classId) {
	bool result = false;

	if(group) {
		for(int counter = 0; counter < MAX_GROUP_MEMBERS; counter++) {
			if(group->members[counter] && group->members[counter]->GetClass() & classId) {
				result = true;
				break;
			}
		}
	}

	return result;
}

bool EntityList::Bot_AICheckCloseBeneficialSpells(Bot* caster, uint8 iChance, float iRange, uint16 iSpellTypes) {
	if((iSpellTypes&SpellTypes_Detrimental) != 0) {
		Log.Out(Logs::General, Logs::Error, "Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
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
					case BotStanceReactive:
					case BotStanceBalanced:
						hpRatioToHeal = 50.0f;
						break;
					case BotStanceBurn:
					case BotStanceBurnAE:
						hpRatioToHeal = 20.0f;
						break;
					case BotStanceAggressive:
					case BotStanceEfficient:
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
	return false;
}


Mob* EntityList::GetMobByBotID(uint32 botID) {
	Mob* Result = 0;
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
	Bot* Result = 0;
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
	Bot* Result = 0;
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

uint8 Bot::GetNumberNeedingHealedInGroup(uint8 hpr, bool includePets) {
	uint8 needHealed = 0;
	Group *g;
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
	ItemInst* inst = GetBotItem(EQEmu::legacy::SlotSecondary);
	if(inst) {
		if(inst->GetItem()->ItemType == ItemTypeShield) {
			ac -= inst->GetItem()->AC;
			shield_ac = inst->GetItem()->AC;
			for (uint8 i = AUG_INDEX_BEGIN; i < EQEmu::legacy::ITEM_COMMON_SIZE; i++) {
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
	const Item_Struct* TempItem = 0;
	ItemInst* inst;
	uint32 Total = 0;
	for (int i = EQEmu::legacy::EQUIPMENT_BEGIN; i <= EQEmu::legacy::EQUIPMENT_END; ++i) {
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
			uint32 buff_count = GetMaxTotalSlots();
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

void Bot::SetHasBeenSummoned(bool wasSummoned) {
	_hasBeenSummoned = wasSummoned;
	if(!wasSummoned)
        m_PreSummonLocation = glm::vec3();
}

void Bot::SetDefaultBotStance() {
	BotStanceType defaultStance = BotStanceBalanced;
	if (GetClass() == WARRIOR)
		defaultStance = BotStanceAggressive;

	_baseBotStance = BotStancePassive;
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
			GetOwner()->Message(0, "%s can use this discipline in %d minutes %d seconds.", GetCleanName(), (remain / 60), (remain % 60));
			return false;
		}
	}

	if(IsCasting())
		InterruptSpell();

	CastSpell(spell_id, target, DISCIPLINE_SPELL_SLOT);
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

		for (uint8 i = 0; i < EQEmu::legacy::MaterialPrimary; ++i) {
			uint8 inv_slot = Inventory::CalcSlotFromMaterial(i);
			ItemInst* inst = m_inv.GetItem(inv_slot);
			if (!inst)
				continue;

			inst->SetColor(rgb);
			SendWearChange(i);
		}
	}
	else {
		uint8 mat_slot = Inventory::CalcMaterialFromSlot(slot_id);
		if (mat_slot == EQEmu::legacy::MaterialInvalid || mat_slot >= EQEmu::legacy::MaterialPrimary)
			return false;

		ItemInst* inst = m_inv.GetItem(slot_id);
		if (!inst)
			return false;

		inst->SetColor(rgb);
		SendWearChange(mat_slot);
	}

	if (save_flag) {
		int16 save_slot = slot_id;
		if (all_flag)
			save_slot = -2;

		if (!botdb.SaveEquipmentColor(GetBotID(), save_slot, rgb)) {
			if (GetBotOwner() && GetBotOwner()->IsClient())
				GetBotOwner()->CastToClient()->Message(13, "%s", BotDatabase::fail::SaveEquipmentColor());
			return false;
		}
	}

	return true;
}

std::string Bot::CreateSayLink(Client* c, const char* message, const char* name)
{
	int saylink_size = strlen(message);
	char* escaped_string = new char[saylink_size * 2];

	database.DoEscapeString(escaped_string, message, saylink_size);

	uint32 saylink_id = database.LoadSaylinkID(escaped_string);
	safe_delete_array(escaped_string);

	EQEmu::saylink::SayLinkEngine linker;
	linker.SetLinkType(linker.SayLinkItemData);
	linker.SetProxyItemID(SAYLINK_ITEM_ID);
	linker.SetProxyAugment1ID(saylink_id);
	linker.SetProxyText(name);

	auto saylink = linker.GenerateLink();
	return saylink;
}

#endif
