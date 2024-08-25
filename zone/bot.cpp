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
#include "raids.h"
#include "doors.h"
#include "quest_parser_collection.h"
#include "lua_parser.h"
#include "../common/repositories/bot_inventories_repository.h"
#include "../common/repositories/bot_spell_settings_repository.h"
#include "../common/repositories/bot_starting_items_repository.h"
#include "../common/data_verification.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/skill_caps.h"

// This constructor is used during the bot create command
Bot::Bot(NPCType *npcTypeData, Client* botOwner) : NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), ping_timer(1) {
	GiveNPCTypeData(npcTypeData);

	if (botOwner) {
		SetBotOwner(botOwner);
		_botOwnerCharacterID = botOwner->CharacterID();
	} else {
		SetBotOwner(nullptr);
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
	m_enforce_spell_settings = false;
	m_bot_archery_setting = false;
	m_expansion_bitmask = -1;
	m_bot_caster_range = 0;
	SetBotID(0);
	SetBotSpellID(0);
	SetSpawnStatus(false);
	SetBotCharmer(false);
	SetPetChooser(false);
	SetRangerAutoWeaponSelect(false);
	SetTaunting(HasClass(Class::Warrior) || HasClass(Class::Paladin) || HasClass(Class::ShadowKnight));
	SetDefaultBotStance();

	SetAltOutOfCombatBehavior(HasClass(Class::Bard)); // will need to be updated if more classes make use of this flag
	SetShowHelm(true);
	SetPauseAI(false);

	m_auto_defend_timer.Disable();
	SetGuardFlag(false);
	SetHoldFlag(false);
	SetAttackFlag(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	SetIsUsingItemClick(false);
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
	bot_timers.clear();

	// Calculate HitPoints Last As It Uses Base Stats
	current_hp = GenerateBaseHitPoints();
	current_mana = GenerateBaseManaPoints();
	cur_end = CalcBaseEndurance();
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();

	SetExtraHaste(database.botdb.GetBotExtraHasteByID(GetBotID()), false);

	strcpy(name, GetCleanName());
	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
}

// This constructor is used when the bot is loaded out of the database
Bot::Bot(
	uint32 botID,
	uint32 botOwnerCharacterID,
	uint32 botSpellsID,
	double totalPlayTime,
	uint32 lastZoneId,
	NPCType *npcTypeData,
	int32 expansion_bitmask
)
	: NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), ping_timer(1)
{
	GiveNPCTypeData(npcTypeData);

	_botOwnerCharacterID = botOwnerCharacterID;
	if (_botOwnerCharacterID > 0)
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
	m_expansion_bitmask = expansion_bitmask;
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

	SetTaunting((HasClass(Class::Warrior) || HasClass(Class::Paladin) || HasClass(Class::ShadowKnight)) && (GetBotStance() == Stance::Aggressive));
	SetPauseAI(false);

	m_auto_defend_timer.Disable();
	SetGuardFlag(false);
	SetHoldFlag(false);
	SetAttackFlag(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	SetIsUsingItemClick(false);
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

	database.botdb.LoadInspectMessage(GetBotID(), _botInspectMessage);

	EquipBot();

	if (HasClass(Class::Rogue)) {
		m_evade_timer.Start();
	}

	m_CastingRoles.GroupHealer = false;
	m_CastingRoles.GroupSlower = false;
	m_CastingRoles.GroupNuker = false;
	m_CastingRoles.GroupDoter = false;

	GenerateBaseStats();

	bot_timers.clear();

	database.botdb.LoadTimers(this);

	LoadAAs();

	if (database.botdb.LoadBuffs(this)) {
		//reapply some buffs
		uint32 buff_count = GetMaxBuffSlots();
		for (uint32 j1 = 0; j1 < buff_count; j1++) {
			if (!IsValidSpell(buffs[j1].spellid)) {
				continue;
			}

			const SPDat_Spell_Struct& spell = spells[buffs[j1].spellid];

			if (int NimbusEffect = GetSpellNimbusEffect(buffs[j1].spellid); NimbusEffect && !IsNimbusEffectActive(NimbusEffect)) {
				SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
			}

			for (int x1 = 0; x1 < EFFECT_COUNT; x1++) {
				switch (spell.effect_id[x1]) {
				case SE_IllusionCopy:
				case SE_Illusion: {
					if (spell.base_value[x1] == -1) {
						if (gender == Gender::Female) {
							gender = Gender::Male;
						} else if (gender == Gender::Male) {
							gender = Gender::Female;
						}

						SendIllusionPacket(
							AppearanceStruct{
								.gender_id = gender,
								.race_id = GetRace(),
							}
						);
					} else if (spell.base_value[x1] == -2) // WTF IS THIS
					{
						if (GetRace() == IKSAR || GetRace() == VAHSHIR || GetRace() <= GNOME) {
							SendIllusionPacket(
								AppearanceStruct{
									.gender_id = GetGender(),
									.helmet_texture = static_cast<uint8>(spell.max_value[x1]),
									.race_id = GetRace(),
									.texture = static_cast<uint8>(spell.limit_value[x1]),
								}
							);
						}
					} else if (spell.max_value[x1] > 0) {
						SendIllusionPacket(
							AppearanceStruct{
								.helmet_texture = static_cast<uint8>(spell.max_value[x1]),
								.race_id = static_cast<uint16>(spell.base_value[x1]),
								.texture = static_cast<uint8>(spell.limit_value[x1]),
							}
						);
					} else {
						SendIllusionPacket(
							AppearanceStruct{
								.helmet_texture = static_cast<uint8>(spell.max_value[x1]),
								.race_id = static_cast<uint16>(spell.base_value[x1]),
								.texture = static_cast<uint8>(spell.limit_value[x1]),
							}
						);
					}

					switch (spell.base_value[x1]) {
					case OGRE:
						SendAppearancePacket(AppearanceType::Size, 9);
						break;
					case TROLL:
						SendAppearancePacket(AppearanceType::Size, 8);
						break;
					case VAHSHIR:
					case BARBARIAN:
						SendAppearancePacket(AppearanceType::Size, 7);
						break;
					case HALF_ELF:
					case WOOD_ELF:
					case DARK_ELF:
					case FROGLOK:
						SendAppearancePacket(AppearanceType::Size, 5);
						break;
					case DWARF:
						SendAppearancePacket(AppearanceType::Size, 4);
						break;
					case HALFLING:
					case GNOME:
						SendAppearancePacket(AppearanceType::Size, 3);
						break;
					default:
						SendAppearancePacket(AppearanceType::Size, 6);
						break;
					}
					break;
				}
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
					SendAppearancePacket(AppearanceType::Invisibility, 1);
					break;
				}
				case SE_Levitate:
				{
					if (!zone->CanLevitate())
					{
							SendAppearancePacket(AppearanceType::FlyMode, 0);
							BuffFadeByEffect(SE_Levitate);
					}
					else {
						SendAppearancePacket(AppearanceType::FlyMode, 2);
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

	CalcBotStats(false);
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
	if (current_hp > max_hp)
		current_hp = max_hp;

	if (current_hp <= 0) {
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
			SetHP(GetMaxHP() / 20);
			SetMana(GetMaxMana() / 20);
			SetEndurance(GetMaxEndurance() / 20);
		}
	}

	if (current_mana > max_mana) {
		current_mana = max_mana;
	}

	cur_end = max_end;

	SetExtraHaste(database.botdb.GetBotExtraHasteByID(GetBotID()), false);
}

Bot::~Bot() {
	AI_Stop();
	LeaveHealRotationMemberPool();
	DataBucket::DeleteCachedBuckets(DataBucketLoadType::Bot, GetBotID());

	if (HasPet()) {
		GetPet()->Depop();
	}

	entity_list.RemoveBot(GetID());

	if (GetGroup()) {
		GetGroup()->MemberZoned(this);
	}

	if (GetRaid()) {
		GetRaid()->MemberZoned(CastToClient());
	}
}

void Bot::SetBotID(uint32 botID) {
	_botID = botID;
	npctype_id = botID;
}

void Bot::SetBotSpellID(uint32 newSpellID) {
	npc_spells_id = newSpellID;
}

void Bot::SetSurname(std::string_view bot_surname) {

	_surname = bot_surname.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
		auto gmn = (GMLastName_Struct*)outapp->pBuffer;

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

void Bot::SetTitle(std::string_view bot_title) {

	_title = bot_title.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
		auto strs = (SetTitleReply_Struct*)outapp->pBuffer;

		strs->is_suffix = 0;
		strn0cpy(strs->title, _title.c_str(), sizeof(strs->title));
		strs->entity_id = GetID();

		entity_list.QueueClients(this, outapp, false);
		safe_delete(outapp);
	}
}

void  Bot::SetSuffix(std::string_view bot_suffix) {

	_suffix = bot_suffix.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
		auto strs = (SetTitleReply_Struct*)outapp->pBuffer;

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
	if ((GetClass()==Class::Warrior) || (GetClass()==Class::Paladin) || (GetClass()==Class::Ranger) || (GetClass()==Class::ShadowKnight) || (GetClass()==Class::Rogue)) {
		if (!isArcher) {
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
	if (IsMoving()) {
		moved = false;
		StopNavigation();
	}

	SetAppearance(eaSitting);
}

void Bot::Stand() {
	SetAppearance(eaStanding);
}

bool Bot::IsSitting() const {
	bool result = false;
	if (GetAppearance() == eaSitting && !IsMoving())
		result = true;

	return result;
}

bool Bot::IsStanding() {
	bool result = false;
	if (GetAppearance() == eaStanding)
		result = true;

	return result;
}

NPCType *Bot::FillNPCTypeStruct(
	uint32 botSpellsID,
	const std::string& botName,
	const std::string& botLastName,
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
	n->runspeed = 1.25f;
	n->gender = gender;
	n->race = botRace;
	n->class_ = botClass;
	n->bodytype = 1;
	n->deity = Deity::Agnostic1;
	n->level = botLevel;
	n->npc_spells_id = botSpellsID;
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
	n->hp_regen = 1;
	n->mana_regen = 1;
	n->trackable = true;
	n->maxlevel = botLevel;
	n->show_name = true;
	n->skip_global_loot = true;
	n->stuck_behavior = Ground;
	n->skip_auto_scale = true;

	return n;
}

NPCType *Bot::CreateDefaultNPCTypeStructForBot(
	const std::string& botName,
	const std::string& botLastName,
	uint8 botLevel,
	uint16 botRace,
	uint8 botClass,
	uint8 gender
) {
	auto n = new NPCType{ 0 };

	strn0cpy(n->name, botName.c_str(), sizeof(n->name));
	strn0cpy(n->lastname, botLastName.c_str(), sizeof(n->lastname));

	n->size = 6.0f;
	n->runspeed = 0.7f;
	n->gender = gender;
	n->race = botRace;
	n->class_ = botClass;
	n->bodytype = 1;
	n->deity = Deity::Agnostic1;
	n->level = botLevel;
	n->AC = 12;
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
	n->hp_regen = 1;
	n->mana_regen = 1;
	n->trackable = true;
	n->maxlevel = botLevel;
	n->show_name = true;
	n->skip_global_loot = true;
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
		case Class::Warrior:
			BotSpellID = 3001;
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case Class::Cleric:
			BotSpellID = 3002;
			Strength += 5;
			Stamina += 5;
			Agility += 10;
			Wisdom += 30;
			Attack += 8;
			break;
		case Class::Paladin:
			BotSpellID = 3003;
			Strength += 15;
			Stamina += 5;
			Wisdom += 15;
			Charisma += 10;
			Dexterity += 5;
			Attack += 17;
			break;
		case Class::Ranger:
			BotSpellID = 3004;
			Strength += 15;
			Stamina += 10;
			Agility += 10;
			Wisdom += 15;
			Attack += 17;
			break;
		case Class::ShadowKnight:
			BotSpellID = 3005;
			Strength += 10;
			Stamina += 15;
			Intelligence += 20;
			Charisma += 5;
			Attack += 17;
			break;
		case Class::Druid:
			BotSpellID = 3006;
			Stamina += 15;
			Wisdom += 35;
			Attack += 5;
			break;
		case Class::Monk:
			BotSpellID = 3007;
			Strength += 5;
			Stamina += 15;
			Agility += 15;
			Dexterity += 15;
			Attack += 17;
			break;
		case Class::Bard:
			BotSpellID = 3008;
			Strength += 15;
			Dexterity += 10;
			Charisma += 15;
			Intelligence += 10;
			Attack += 17;
			break;
		case Class::Rogue:
			BotSpellID = 3009;
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case Class::Shaman:
			BotSpellID = 3010;
			Stamina += 10;
			Wisdom += 30;
			Charisma += 10;
			Attack += 28;
			break;
		case Class::Necromancer:
			BotSpellID = 3011;
			Dexterity += 10;
			Agility += 10;
			Intelligence += 30;
			Attack += 5;
			break;
		case Class::Wizard:
			BotSpellID = 3012;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case Class::Magician:
			BotSpellID = 3013;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case Class::Enchanter:
			BotSpellID = 3014;
			Intelligence += 25;
			Charisma += 25;
			Attack += 5;
			break;
		case Class::Beastlord:
			BotSpellID = 3015;
			Stamina += 10;
			Agility += 10;
			Dexterity += 5;
			Wisdom += 20;
			Charisma += 5;
			Attack += 31;
			break;
		case Class::Berserker:
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
		if (GetRace() == DWARF && zone->random.Int(1, 100) < 50) {
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

	auto iEyeColor1 = (uint8)zone->random.Int(0, 9);
	uint8 iEyeColor2 = 0;
	if (GetRace() == DRAKKIN) {
		iEyeColor1 = iEyeColor2 = (uint8)zone->random.Int(0, 11);
	} else if (zone->random.Int(1, 100) > 96) {
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

uint16 Bot::GetPrimarySkillValue() {
	EQ::skills::SkillType skill = EQ::skills::HIGHEST_SKILL; //because nullptr == 0, which is 1H Slashing, & we want it to return 0 from GetSkill
	if (bool equipped = m_inv.GetItem(EQ::invslot::slotPrimary); !equipped) {
		skill = EQ::skills::SkillHandtoHand;
	} else {
		uint8 type = m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->ItemType; //is this the best way to do this?
		switch (type) {
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
	return skill_caps.GetSkillCap(class_, skillid, level).cap;
}

uint32 Bot::GetTotalATK() {
	uint32 AttackRating = 0;
	uint32 WornCap = itembonuses.ATK;
	if (IsBot()) {
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
	if (IsBot()) {
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
		if (((NormalSTA - 255) / 2) > 0)
			SoDPost255 = ((static_cast<float>(NormalSTA) - 255.0f) / 2.0f);
		else
			SoDPost255 = 0.0f;

		int hp_factor = GetClassHPFactor();

		if (level < 41)
			new_base_hp = (5 + (GetLevel() * hp_factor / 12) + ((NormalSTA - SoDPost255) * GetLevel() * hp_factor / 3600));
		else if (level < 81)
			new_base_hp = (5 + (40 * hp_factor / 12) + ((GetLevel() - 40) * hp_factor / 6) + ((NormalSTA - SoDPost255) * hp_factor / 90) + ((NormalSTA - SoDPost255) * (GetLevel() - 40) * hp_factor / 1800));
		else
			new_base_hp = (5 + (80 * hp_factor / 8) + ((GetLevel() - 80) * hp_factor / 10) + ((NormalSTA - SoDPost255) * hp_factor / 90) + ((NormalSTA - SoDPost255) * hp_factor / 45));
	} else {
		if (((NormalSTA - 255) / 2) > 0)
			Post255 = ((NormalSTA - 255) / 2);
		else
			Post255 = 0;

		new_base_hp = 5 + (GetLevel() * lm / 10) + ((NormalSTA - Post255) * GetLevel() * lm / 3000) + ((Post255 * 1) * lm / 6000);
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
		if (!ability->first || ability->charges > 0) {
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
			if (!CanUseAlternateAdvancementRank(current)) {
				current = nullptr;
			} else {
				current = current->next;
				points++;
			}
		}

		if (points > 0) {
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
	if (auto bot_class_bitmask = GetPlayerClassBit(bot_class); classes & bot_class_bitmask) {
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

	for (char c : name.substr(1)) {
		if (!RuleB(Bots, AllowCamelCaseNames) && !islower(c)) {
			return false;
		}
		if (isdigit(c) || ispunct(c)) {
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

	if (!GetBotID()) { // New bot record
		uint32 bot_id = 0;
		if (!database.botdb.SaveNewBot(this, bot_id) || !bot_id) {
			return false;
		}
		SetBotID(bot_id);
	}
	else { // Update existing bot record
		if (!database.botdb.SaveBot(this)) {
			return false;
		}
	}

	// All of these continue to process if any fail
	database.botdb.SaveBuffs(this);
	database.botdb.SaveTimers(this);
	database.botdb.SaveStance(this);

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

	if (GetGroup()) {
		RemoveBotFromGroup(this, GetGroup());
	}

	if (GetRaid()) {
		RemoveBotFromRaid(this);
	}

	if (!database.botdb.DeleteItems(GetBotID())) {
		return false;
	}

	if (!database.botdb.DeleteTimers(GetBotID())) {
		return false;
	}

	if (!database.botdb.DeleteBuffs(GetBotID())) {
		return false;
	}

	if (!database.botdb.DeleteStance(GetBotID())) {
		return false;
	}

	if (!database.botdb.DeleteBot(GetBotID())) {
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

	if (HasClass(Class::Wizard)) {
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

	uint32 pet_index = 0;
	if (!database.botdb.LoadPetIndex(GetBotID(), pet_index)) {
		return false;
	}
	if (!pet_index)
		return true;

	uint32 saved_pet_spell_id = 0;
	database.botdb.LoadPetSpellID(GetBotID(), saved_pet_spell_id);

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
		return false;
	}

	MakePet(pet_spell_id, spells[pet_spell_id].teleport_zone, pet_name.c_str());
	if (!GetPet()->IsNPC()) {
		DeletePet();
		return false;
	}

	NPC *pet_inst = GetPet()->CastToNPC();

	SpellBuff_Struct pet_buffs[PET_BUFF_COUNT];
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * PET_BUFF_COUNT));
	database.botdb.LoadPetBuffs(GetBotID(), pet_buffs);

	uint32 pet_items[EQ::invslot::EQUIPMENT_COUNT];
	memset(pet_items, 0, (sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT));
	database.botdb.LoadPetItems(GetBotID(), pet_items);

	pet_inst->SetPetState(pet_buffs, pet_items);
	pet_inst->CalcBonuses();
	pet_inst->SetHP(pet_hp);
	pet_inst->SetMana(pet_mana);

	return true;
}

bool Bot::SavePet()
{
	if (!GetPet() || GetPet()->IsFamiliar()) { // dead?
		return true;
	}

	NPC *pet_inst = GetPet()->CastToNPC();
	if (!pet_inst->GetPetSpellID() || !IsValidSpell(pet_inst->GetPetSpellID())) {
		return false;
	}

	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return false;
	}

	auto pet_name = new char[64];
	SpellBuff_Struct pet_buffs[PET_BUFF_COUNT];
	uint32 pet_items[EQ::invslot::EQUIPMENT_COUNT];

	memset(pet_name, 0, 64);
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * PET_BUFF_COUNT));
	memset(pet_items, 0, (sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT));

	pet_inst->GetPetState(pet_buffs, pet_items, pet_name);

	std::string pet_name_str = pet_name;
	safe_delete_array(pet_name)

	if (!database.botdb.SavePetStats(GetBotID(), pet_name_str, pet_inst->GetMana(), pet_inst->GetHP(), pet_inst->GetPetSpellID())) {
		return false;
	}

	database.botdb.SavePetBuffs(GetBotID(), pet_buffs);
	database.botdb.SavePetItems(GetBotID(), pet_items);

	return true;
}

bool Bot::DeletePet()
{
	return false;
	// NUKED BY CATAPULTAM
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

		_botOwner = nullptr;
		_botOwnerCharacterID = 0;

		return false;
	}

	if (mob_close_scan_timer.Check()) {
		LogAIScanCloseDetail(
			"is_moving [{}] bot [{}] timer [{}]",
			moving ? "true" : "false",
			GetCleanName(),
			mob_close_scan_timer.GetDuration()
		);

		entity_list.ScanCloseMobs(close_mobs, this, IsMoving());
	}

	SpellProcess();

	if (tic_timer.Check()) {

		// 6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if (!IsEngaged() && !rest_timer.Enabled()) {
			rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
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

	if (auto_save_timer.Check()) {
		clock_t t = std::clock(); /* Function timer start */
		Save();
		LogDebug(
			"ZoneDatabase::SaveBotData [{}], done Took [{}] seconds",
			GetBotID(),
			((float)(std::clock() - t)) / CLOCKS_PER_SEC
		);
		auto_save_timer.Start(RuleI(Bots, AutosaveIntervalSeconds) * 1000);
	}

	if (IsStunned() || IsMezzed()) {
		return true;
	}

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

	m_roambox.max_x		 = 0;
	m_roambox.max_y		 = 0;
	m_roambox.min_x		 = 0;
	m_roambox.min_y		 = 0;
	m_roambox.distance  = 0;
	m_roambox.dest_x		= 0;
	m_roambox.dest_y		= 0;
	m_roambox.dest_z		= 0;
	m_roambox.delay		 = 2500;
	m_roambox.min_delay = 2500;
}

void Bot::SpellProcess() {
	if (spellend_timer.Check(false))	{
		NPC::SpellProcess();
		if (HasClass(Class::Bard) && casting_spell_id != 0) casting_spell_id = 0;
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
	if ((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check())) {
		LogCombatDetail("Bot Archery attack canceled. Timer not up. Attack [{}] ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	const auto rangedItem = GetBotItem(EQ::invslot::slotRange);
	const EQ::ItemData* RangeWeapon = nullptr;
	if (rangedItem)
		RangeWeapon = rangedItem->GetItem();

	const auto ammoItem = GetBotItem(EQ::invslot::slotAmmo);
	const EQ::ItemData* Ammo = nullptr;
	if (ammoItem)
		Ammo = ammoItem->GetItem();

	if (!RangeWeapon || !Ammo)
		return;

	LogCombatDetail("Shooting [{}] with bow [{}] ([{}]) and arrow [{}] ([{}])", other->GetCleanName(), RangeWeapon->Name, RangeWeapon->ID, Ammo->Name, Ammo->ID);
	if (!IsAttackAllowed(other) || IsCasting() || DivineAura() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead))
		return;

	SendItemAnimation(other, Ammo, EQ::skills::SkillArchery);
	DoArcheryAttackDmg(other, rangedItem, ammoItem); // watch

	//break invis when you attack
	if (invisible) {
		LogCombatDetail("Removing invisibility due to melee attack");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}

	if (invisible_undead) {
		LogCombatDetail("Removing invisibility vs. undead due to melee attack");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}

	if (invisible_animals) {
		LogCombatDetail("Removing invisibility vs. animals due to melee attack");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if (hidden || improved_hidden) {
		hidden = false;
		improved_hidden = false;
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		auto sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
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
	if (!GetSkill(EQ::skills::SkillDoubleAttack) && !(HasClass(Class::Bard) || HasClass(Class::Beastlord)))
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
	if (tripleAttack) {
		// Only some Double Attack classes get Triple Attack [This is already checked in client_processes.cpp]
		int32 triple_bonus = (spellbonuses.TripleAttackChance + itembonuses.TripleAttackChance);
		chance *= 0.2f; //Baseline chance is 20% of your double attack chance.
		chance *= (float(100.0f + triple_bonus) / 100.0f); //Apply modifiers.
	}

	if (zone->random.Real(0, 1) < chance)
		return true;

	return false;
}

void Bot::SetTarget(Mob *mob)
{
	if (mob != this) {
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

	if (HasPet() && (GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {
		GetPet()->StopMoving();
	}
}

void Bot::SetHoldMode() {

	SetHoldFlag();
}

// AI Processing for the Bot object

constexpr float MAX_CASTER_DISTANCE[Class::PLAYER_CLASS_COUNT] = {
	0,
	(34 * 34),
	(24 * 24),
	(28 * 28),
	(26 * 26),
	(42 * 42),
	0,
	(30 * 30),
	0,
	(38 * 38),
	(54 * 54),
	(48 * 48),
	(52 * 52),
	(50 * 50),
	(32 * 32),
	0
};

void Bot::AI_Process()
{

#define PULLING_BOT (GetPullingFlag() || GetReturningFlag())
#define NOT_PULLING_BOT (!GetPullingFlag() && !GetReturningFlag())
#define GUARDING (GetGuardFlag())
#define NOT_GUARDING (!GetGuardFlag())
#define HOLDING (GetHoldFlag())
#define NOT_HOLDING (!GetHoldFlag())
#define PASSIVE (GetBotStance() == Stance::Passive)
#define NOT_PASSIVE (GetBotStance() != Stance::Passive)

	Client* bot_owner = (GetBotOwner() && GetBotOwner()->IsClient() ? GetBotOwner()->CastToClient() : nullptr);

	if (!bot_owner) {
		return;
	}

	auto raid = entity_list.GetRaidByBotName(GetName());
	uint32 r_group = RAID_GROUPLESS;
	if (raid) {
		raid->VerifyRaid();
		r_group = raid->GetGroup(GetName());

		if (mana_timer.Check(false)) {
			raid->SendHPManaEndPacketsFrom(this);
		}

		if (send_hp_update_timer.Check(false)) {
			raid->SendHPManaEndPacketsFrom(this);
		}
	}

	auto bot_group = GetGroup();

	// Primary reasons for not processing AI
	if (!IsAIProcessValid(bot_owner, bot_group, raid)) {
		return;
	}

	auto leash_owner = SetLeashOwner(bot_owner, bot_group, raid, r_group);

	if (!leash_owner) {
		return;
	}

	SetFollowID(leash_owner->GetID());

	auto follow_mob = SetFollowMob(leash_owner);

	SetBerserkState();

	// Secondary reasons for not processing AI
	if (CheckIfIncapacitated()) {
		return;
	}

	float fm_distance = DistanceSquared(m_Position, follow_mob->GetPosition());
	float lo_distance = DistanceSquared(m_Position, leash_owner->GetPosition());
	float leash_distance = RuleR(Bots, LeashDistance);

// CURRENTLY CASTING CHECKS

	if (CheckIfCasting(fm_distance)) {
		return;
	}

// HEAL ROTATION CASTING CHECKS

	HealRotationChecks();

	if (GetAttackFlag()) { // Push owner's target onto our hate list
		SetOwnerTarget(bot_owner);
	}
	else if (GetPullFlag()) { // Push owner's target onto our hate list and set flags so other bots do not aggro
		BotPullerProcess(bot_owner, raid);
	}

//ALT COMBAT (ACQUIRE HATE)

	glm::vec3 Goal(0, 0, 0);

	// We have aggro to choose from
	if (IsEngaged()) {

		if (rest_timer.Enabled()) {
			rest_timer.Disable();
		}

// PULLING FLAG (TARGET VALIDATION)

		if (GetPullingFlag()) {
			if (!PullingFlagChecks(bot_owner)) {
				return;
			}
		}

// RETURNING FLAG

		else if (GetReturningFlag()) {
			if (!ReturningFlagChecks(bot_owner, fm_distance)) {
				return;
			}
		}

// DEFAULT (ACQUIRE TARGET)

// VERIFY TARGET AND STANCE

		auto tar = GetBotTarget(bot_owner);
		if (!tar) {
			return;
		}

// ATTACKING FLAG (HATE VALIDATION)

		if (GetAttackingFlag() && tar->CheckAggro(this)) {
			SetAttackingFlag(false);
		}

		float tar_distance = DistanceSquared(m_Position, tar->GetPosition());

// TARGET VALIDATION

		if (!IsValidTarget(bot_owner, leash_owner, lo_distance, leash_distance, tar, tar_distance)) {
			return;
		}

		// This causes conflicts with default pet handler (bounces between targets)
		if (NOT_PULLING_BOT && HasPet() && (GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

			// We don't add to hate list here because it's assumed to already be on the list
			GetPet()->SetTarget(tar);
		}

		if (DivineAura()) {
			return;
		}

		if (!(m_PlayerState & static_cast<uint32>(PlayerState::Aggressive))) {
			SendAddPlayerState(PlayerState::Aggressive);
		}

// PULLING FLAG (ACTIONABLE RANGE)

		if (GetPullingFlag()) {

			constexpr size_t PULL_AGGRO = 5225; // spells[5225]: 'Throw Stone' - 0 cast time

			if (tar_distance <= (spells[PULL_AGGRO].range * spells[PULL_AGGRO].range)) {

				StopMoving();
				CastSpell(PULL_AGGRO, tar->GetID());
				return;
			}
		}

// COMBAT RANGE CALCS

		bool atCombatRange;
		const EQ::ItemInstance* p_item;
		const EQ::ItemInstance* s_item;
		CheckCombatRange(tar, tar_distance, atCombatRange, p_item, s_item);

// ENGAGED AT COMBAT RANGE

		// We can fight
		if (atCombatRange) {

			if (IsMoving()) {
				StopMoving(CalculateHeadingToTarget(tar->GetX(), tar->GetY()));
				return;
			}

			if (AI_movement_timer->Check() && (!spellend_timer.Enabled() || HasClass(Class::Bard))) {

				if (TryEvade(tar)) {
					return;
				}

				if (TryFacingTarget(tar)) {
					return;
				}
			}

			if (!IsBotNonSpellFighter() && AI_EngagedCastCheck()) {
				return;
			}

			// Up to this point, GetTarget() has been safe to dereference since the initial
			// if (!GetTarget() || GetAppearance() == eaDead) { return false; } call. Due to the chance of the target dying and our pointer
			// being nullified, we need to test it before dereferencing to avoid crashes

			if (IsBotArcher() && TryRangedAttack(tar)) {
				return;
			}

			if (!IsBotArcher() && GetLevel() < GetStopMeleeLevel()) {
				if (!TryClassAttacks(tar)) {
					return;
				}

				if (!TryPrimaryWeaponAttacks(tar, p_item)) {
					return;
				}

				if (!TrySecondaryWeaponAttacks(tar, s_item)) {
					return;
				}
			}

			if (GetAppearance() == eaDead) {
				return;
			}
		}

// ENGAGED NOT AT COMBAT RANGE

		else if (!TryPursueTarget(leash_distance, Goal)) {
			return;
		}

// End not in combat range

		if (TryMeditate()) {
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

// AUTO DEFEND

		if (TryAutoDefend(bot_owner, leash_distance) ) {
			return;
		}

		SetTarget(nullptr);

		if (HasPet() && (GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

			GetPet()->WipeHateList();
			GetPet()->SetTarget(nullptr);
		}

		if (m_PlayerState & static_cast<uint32>(PlayerState::Aggressive)) {
			SendRemovePlayerState(PlayerState::Aggressive);
		}

// OK TO IDLE

		// Ok to idle
		if (TryNonCombatMovementChecks(bot_owner, follow_mob, Goal)) {
			return;
		}
		if (TryIdleChecks(fm_distance)) {
			return;
		}
		if (TryBardMovementCasts()) {
			return;
		}
	}
}

bool Bot::TryBardMovementCasts() {// Basically, bard bots get a chance to cast idle spells while moving

	if (HasClass(Class::Bard) && IsMoving() && NOT_PASSIVE && !spellend_timer.Enabled() && AI_think_timer->Check()) {

		AI_IdleCastCheck();
		return true;
	}
	return false;
}

bool Bot::TryNonCombatMovementChecks(Client* bot_owner, const Mob* follow_mob, glm::vec3& Goal) {// Non-engaged movement checks

	if (AI_movement_timer->Check() && (!IsCasting() || HasClass(Class::Bard))) {
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

				RunTo(Goal.x, Goal.y, Goal.z);

				return true;
			}
		}
		else {

			if (IsMoving()) {

				StopMoving();
				return true;
			}
		}
	}
	return false;
}

bool Bot::TryIdleChecks(float fm_distance) {

	if (
		(
			(NOT_GUARDING && fm_distance <= GetFollowDistance()) ||
			(GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) <= GetFollowDistance())) &&
			!IsMoving() &&
			AI_think_timer->Check() &&
			!spellend_timer.Enabled()
	) {

		if (NOT_PASSIVE) {

			if (!AI_IdleCastCheck() && !IsCasting() && GetClass() != Class::Bard) {
				BotMeditate(true);
			}

		} else {
			if (GetClass() != Class::Bard) {
				BotMeditate(true);
			}

		}
		return true;
	}
	return false;
}

// This is as close as I could get without modifying the aggro mechanics and making it an expensive process...
// 'class Client' doesn't make use of hate_list
bool Bot::TryAutoDefend(Client* bot_owner, float leash_distance) {

	if (RuleB(Bots, AllowOwnerOptionAutoDefend) && bot_owner->GetBotOption(Client::booAutoDefend)) {

		if (!m_auto_defend_timer.Enabled()) {

			m_auto_defend_timer.Start(zone->random.Int(250, 1250)); // random timer to simulate 'awareness' (cuts down on scanning overhead)
			return true;
		}

		if (
			m_auto_defend_timer.Check() &&
			bot_owner->GetAggroCount() &&
			NOT_HOLDING &&
			NOT_PASSIVE
		) {
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
					if (hater && hater->CastToNPC()->IsOnHatelist(bot_owner) && !hater->IsMezzed() && DistanceSquared(hater->GetPosition(), bot_owner->GetPosition()) <= leash_distance) {
						// This is roughly equivilent to npc attacking a client pet owner
						AddToHateList(hater, 1);
						SetTarget(hater);
						SetAttackingFlag();

						if (HasPet() && (GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {
							GetPet()->AddToHateList(hater, 1);
							GetPet()->SetTarget(hater);
						}

						m_auto_defend_timer.Disable();

						return true;
					}
				}
			}
		}
	}
	return false;
}

bool Bot::TryMeditate() {

	if (!IsMoving() && !spellend_timer.Enabled()) {
		if (GetTarget() && AI_EngagedCastCheck()) {
			BotMeditate(false);
		} else if (GetArchetype() == Archetype::Caster) {
			BotMeditate(true);
		}

		if (!(GetPlayerState() & static_cast<uint32>(PlayerState::Aggressive))) {
			SendAddPlayerState(PlayerState::Aggressive);
		}
		return true;
	}
	return false;
}

// This code actually gets processed when we are too far away from target and have not engaged yet
bool Bot::TryPursueTarget(float leash_distance, glm::vec3& Goal) {

	if (AI_movement_timer->Check() && (!spellend_timer.Enabled() || HasClass(Class::Bard))) {
		if (GetTarget() && !IsRooted()) {
			LogAIDetail("Pursuing [{}] while engaged", GetTarget()->GetCleanName());
			Goal = GetTarget()->GetPosition();
			if (DistanceSquared(m_Position, Goal) <= leash_distance) {
				RunTo(Goal.x, Goal.y, Goal.z);

			} else {
				WipeHateList();
				SetTarget(nullptr);

				if (HasPet() && (GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {
					GetPet()->WipeHateList();
					GetPet()->SetTarget(nullptr);
				}
			}
			return true;

		} else {
			if (IsMoving()) {
				StopMoving();
			}

			return false;
		}
	}

	if (GetTarget() && GetTarget()->IsFeared() && !spellend_timer.Enabled() && AI_think_timer->Check()) {
		if (!IsFacingMob(GetTarget())) {
			FaceTarget(GetTarget());
		}

		// This is a mob that is fleeing either because it has been feared or is low on hitpoints
		AI_PursueCastCheck();

		return true;
	}
	return false;
}

bool Bot::TrySecondaryWeaponAttacks(Mob* tar, const EQ::ItemInstance* s_item) {

	if (!GetTarget() || GetAppearance() == eaDead) { return false; }
	if (attack_dw_timer.Check() && CanThisClassDualWield()) {
		const EQ::ItemData* s_itemdata = nullptr;

		// Can only dual wield without a weapon if you're a monk
		if (s_item || (HasClass(Class::Monk))) {

			if (s_item) {
				s_itemdata = s_item->GetItem();
			}

			if (!s_itemdata) {
				return false;
			}

			bool use_fist = true;
			if (s_itemdata) {
				use_fist = false;
			}

			if (use_fist || !s_itemdata->IsType2HWeapon()) {

				float DualWieldProbability = 0.0f;

				int32 Ambidexterity = (aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity);
				DualWieldProbability = ((GetSkill(EQ::skills::SkillDualWield) + GetLevel() + Ambidexterity) / 400.0f); // 78.0 max chance

				int32 DWBonus = (spellbonuses.DualWieldChance + itembonuses.DualWieldChance);
				DualWieldProbability += (DualWieldProbability * float(DWBonus) / 100.0f);

				float random = zone->random.Real(0, 1);
				if (random < DualWieldProbability) { // Max 78% for DW chance
					Attack(tar, EQ::invslot::slotSecondary);	// Single attack with offhand

					if (GetAppearance() == eaDead) { return false; }
					TryCombatProcs(s_item, tar, EQ::invslot::slotSecondary);

					if (GetAppearance() == eaDead) { return false; }
					if (CanThisClassDoubleAttack() && CheckBotDoubleAttack() && tar->GetHP() > -10) {
						Attack(tar, EQ::invslot::slotSecondary);	// Single attack with offhand
					}
				}
			}
		}
	}
	return true;
}

bool Bot::TryPrimaryWeaponAttacks(Mob* tar, const EQ::ItemInstance* p_item) {

	if (!GetTarget() || GetAppearance() == eaDead) { return false; }
	if (attack_timer.Check()) { // Process primary weapon attacks

		Attack(tar, EQ::invslot::slotPrimary);

		if (GetAppearance() == eaDead) { return false; }
		TriggerDefensiveProcs(tar, EQ::invslot::slotPrimary, false);

		if (GetAppearance() == eaDead) { return false; }
		TryCombatProcs(p_item, tar, EQ::invslot::slotPrimary);

		if (GetAppearance() == eaDead) { return false; }
		if (CanThisClassDoubleAttack()) {

			if (CheckBotDoubleAttack()) {
				Attack(tar, EQ::invslot::slotPrimary, true);
			}

			if (GetAppearance() == eaDead) { return false; }
			if (GetSpecialAbility(SpecialAbility::TripleAttack) && CheckBotDoubleAttack(true)) {

				Attack(tar, EQ::invslot::slotPrimary, true);
			}

			if (GetAppearance() == eaDead) { return false; }
			// quad attack, does this belong here??
			if (GetSpecialAbility(SpecialAbility::QuadrupleAttack) && CheckBotDoubleAttack(true)) {
				Attack(tar, EQ::invslot::slotPrimary, true);
			}
		}

		if (GetAppearance() == eaDead) { return false; }
		// Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
		if (int32 flurrychance = (aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance)) {

			if (zone->random.Int(0, 100) < flurrychance) {

				MessageString(Chat::NPCFlurry, YOU_FLURRY);
				Attack(tar, EQ::invslot::slotPrimary, false);

				if (GetAppearance() == eaDead) { return false; }
				Attack(tar, EQ::invslot::slotPrimary, false);
			}
		}

		if (GetAppearance() == eaDead) { return false; }
		auto ExtraAttackChanceBonus =
			(spellbonuses.ExtraAttackChance[0] + itembonuses.ExtraAttackChance[0] +
			 aabonuses.ExtraAttackChance[0]);

		if (
			ExtraAttackChanceBonus &&
			p_item &&
			p_item->GetItem()->IsType2HWeapon() &&
			zone->random.Int(0, 100) < ExtraAttackChanceBonus
		) {
			Attack(tar, EQ::invslot::slotPrimary, false);
		}
	}
	return true;
}

// We can't fight if we don't have a target, are stun/mezzed or dead..
bool Bot::TryClassAttacks(Mob* tar) {

// Stop attacking if the target is enraged
	if (!GetTarget() || GetAppearance() == eaDead) { return false; }
	if (tar->IsEnraged() && !BehindMob(tar, GetX(), GetY())) {
		return false;
	}

	// First, special attack per class (kick, backstab etc..)
	DoClassAttacks(tar);
	return true;
}

bool Bot::TryRangedAttack(Mob* tar) {

	if (IsBotArcher() && ranged_timer.Check(false)) {

		if (!GetTarget() || GetAppearance() == eaDead) { return false; }
		if (GetTarget()->GetHPRatio() <= 99.0f) {
			BotRangedAttack(tar);
		}
		return true;
	}
	return false;
}

bool Bot::TryFacingTarget(Mob* tar) {

	if (!IsSitting() && !IsFacingMob(tar)) {
		FaceTarget(tar);
		return true;
	}
	return false;
}


bool Bot::TryEvade(Mob* tar) {

	if (
		!IsRooted() &&
		HasTargetReflection() &&
		!tar->IsFeared() &&
		!tar->IsStunned() &&
		HasClass(Class::Rogue) &&
		m_evade_timer.Check(false)
	) {
		int timer_duration = (HideReuseTime - GetSkillReuseTime(EQ::skills::SkillHide)) * 1000;

		if (timer_duration < 0) {
			timer_duration = 0;
		}

		m_evade_timer.Start(timer_duration);
		if (zone->random.Int(0, 260) < (int) GetSkill(EQ::skills::SkillHide)) {
			RogueEvade(tar);
		}
		return true;
	}

	return false;
}

void Bot::CheckCombatRange(Mob* tar, float tar_distance, bool& atCombatRange, const EQ::ItemInstance*& p_item, const EQ::ItemInstance*& s_item) {

	atCombatRange= false;
	p_item= GetBotItem(EQ::invslot::slotPrimary);
	s_item= GetBotItem(EQ::invslot::slotSecondary);
	bool behind_mob = false;
	bool backstab_weapon = false;
	if (HasClass(Class::Rogue)) {

		behind_mob = BehindMob(tar, GetX(), GetY()); // Can be separated for other future use
		backstab_weapon = p_item && p_item->GetItemBackstabDamage();
	}

	// Calculate melee distances
	float melee_distance_max = 0.0f;
	float melee_distance = 0.0f;

	CalcMeleeDistances(tar, p_item, s_item, behind_mob, backstab_weapon, melee_distance_max, melee_distance);

	float caster_distance_max = GetBotCasterMaxRange(melee_distance_max);

	bool atArcheryRange = IsArcheryRange(tar);

	SetRangerCombatWeapon(atArcheryRange);

	bool stop_melee_level = GetLevel() >= GetStopMeleeLevel();
	if (IsBotArcher() && atArcheryRange) {
		atCombatRange = true;
	}
	else if (caster_distance_max && tar_distance <= caster_distance_max && stop_melee_level) {
		atCombatRange = true;
	}
	else if (tar_distance <= melee_distance) {
		atCombatRange = true;
	}
}

void Bot::SetRangerCombatWeapon(bool atArcheryRange) {

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
}

void Bot::CalcMeleeDistances(const Mob* tar, const EQ::ItemInstance* const& p_item, const EQ::ItemInstance* const& s_item, bool behind_mob, bool backstab_weapon, float& melee_distance_max, float& melee_distance) const {

	float size_mod = GetSize();
	float other_size_mod = tar->GetSize();

	// For races with a fixed size
	if (GetRace() == Race::LavaDragon || GetRace() == Race::Wurm || GetRace() == Race::GhostDragon) {
		// size_mod = 60.0f;
	}

	else if (size_mod < 6.0f) {
		size_mod = 8.0f;
	}

	// For races with a fixed size
	if (tar->GetRace() == Race::LavaDragon || tar->GetRace() == Race::Wurm || tar->GetRace() == Race::GhostDragon) {
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
	case Class::Warrior:
	case Class::Paladin:
	case Class::ShadowKnight:
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
	case Class::Necromancer:
	case Class::Wizard:
	case Class::Magician:
	case Class::Enchanter:
		if (p_item && p_item->GetItem()->IsType2HWeapon()) {
			melee_distance = melee_distance_max * 0.95f;
		}
		else {
			melee_distance = melee_distance_max * 0.75f;
		}
		break;
	case Class::Rogue:
		if (behind_mob && backstab_weapon) {
			if (p_item->GetItem()->IsType2HWeapon()) {
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

bool Bot::IsValidTarget(
	Client* bot_owner,
	Client* leash_owner,
	float lo_distance,
	float leash_distance,
	Mob* tar,
	float tar_distance
)
{
	if (!tar || !bot_owner || !leash_owner) {
		return false;
	}

	bool invalid_target_state = false;
	if (HOLDING ||
		!tar->IsNPC() ||
		tar->IsMezzed() ||
		lo_distance > leash_distance ||
		tar_distance > leash_distance ||
		(!GetAttackingFlag() && !CheckLosFN(tar) && !leash_owner->CheckLosFN(tar)) ||
		!IsAttackAllowed(tar)
	) {
		invalid_target_state = true;
	}

	if (invalid_target_state) {
		// Normally, we wouldn't want to do this without class checks..but, too many issues can arise if we let enchanter animation pets run rampant
		if (HasPet()) {
			GetPet()->RemoveFromHateList(tar);
			GetPet()->RemoveFromRampageList(tar);
			GetPet()->SetTarget(nullptr);
		}

		RemoveFromHateList(tar);
		RemoveFromRampageList(tar);
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

		return false;
	}

	return true;
}

Mob* Bot::GetBotTarget(Client* bot_owner)
{
	Mob* t = GetTarget();

	if (!t || PASSIVE) {
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

		if (GetArchetype() == Archetype::Caster) {
			BotMeditate(true);
		}
	}

	return t;
}

bool Bot::ReturningFlagChecks(Client* bot_owner, float fm_distance) {// Need to make it back to group before clearing return flag

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
		return false;
	}
	return true;
}

bool Bot::PullingFlagChecks(Client* bot_owner) {

	if (!GetTarget()) {

		WipeHateList();
		SetTarget(nullptr);
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (GetPet()) {
			GetPet()->SetPetOrder(m_previous_pet_order);
		}

		return false;
	}
	else if (GetTarget()->GetHateList().size()) {

		WipeHateList();
		SetTarget(nullptr);
		SetPullingFlag(false);
		SetReturningFlag();

		if (HasPet() &&
			(GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

			GetPet()->WipeHateList();
			GetPet()->SetTarget(nullptr);
		}

		if (GetPlayerState() & static_cast<uint32>(PlayerState::Aggressive)) {
			SendRemovePlayerState(PlayerState::Aggressive);
		}
	}

	return true;
}

void Bot::HealRotationChecks() {

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
}

bool Bot::IsAIProcessValid(const Client* bot_owner, const Group* bot_group, const Raid* raid) {

	if (!bot_owner || (!bot_group && !raid) || !IsAIControlled()) {
		return false;
	}

	if (bot_owner->IsDead()) {
		SetTarget(nullptr);
		SetBotOwner(nullptr);
		return false;
	}
	return true;
}

bool Bot::CheckIfCasting(float fm_distance) {

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
			return true;
		}
		else if (GetClass() != Class::Bard) {

			if (IsEngaged()) {
				return true;
			}

			if (
				(NOT_GUARDING && fm_distance > GetFollowDistance()) || // Cancel out-of-combat casting if movement to follow mob is required
				(GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) > GetFollowDistance()) // Cancel out-of-combat casting if movement to guard point is required
			) {
				InterruptSpell();
			}

			return true;
		}
	}
	else if (IsHealRotationMember()) {
		m_member_of_heal_rotation->SetMemberIsCasting(this, false);
	}
	return false;
}

bool Bot::CheckIfIncapacitated() {

	if (GetPauseAI() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead)) {

		if (IsCasting()) {
			InterruptSpell();
		}

		if (IsMyHealRotationSet() || (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this)) {
			AdvanceHealRotation(false);
			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
		}
		return true;
	}

	if (IsRooted() && IsMoving()) {
		StopMoving();
		return true;
	}

	return false;
}

void Bot::SetBerserkState() {// Berserk updates should occur if primary AI criteria are met
	if (HasClass(Class::Warrior) || HasClass(Class::Berserker)) {

		if (!berserk && GetHP() > 0 && GetHPRatio() < 30.0f) {
			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_START, GetName());
			berserk = true;
		}

		if (berserk && GetHPRatio() >= 30.0f) {
			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_END, GetName());
			berserk = false;
		}
	}
}

Mob* Bot::SetFollowMob(Client* leash_owner) {
	Mob* follow_mob = entity_list.GetMob(GetFollowID());
	if (!follow_mob) {

		follow_mob = leash_owner;
		SetFollowID(leash_owner->GetID());
	}
	return follow_mob;
}

Client* Bot::SetLeashOwner(Client* bot_owner, Group* bot_group, Raid* raid, uint32 r_group) const {

	Client* leash_owner = nullptr;
	if (raid && r_group < MAX_RAID_GROUPS && raid->GetGroupLeader(r_group)) {
		leash_owner =
			raid->GetGroupLeader(r_group) &&
			raid->GetGroupLeader(r_group)->IsClient() ?
				raid->GetGroupLeader(r_group)->CastToClient() : bot_owner;

	} else if (bot_group) {
		leash_owner = (bot_group->GetLeader() && bot_group->GetLeader()->IsClient() ? bot_group->GetLeader()->CastToClient() : bot_owner);

	} else {
		leash_owner = bot_owner;
	}
	return leash_owner;
}

void Bot::SetOwnerTarget(Client* bot_owner) {
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
			if (GetPet() && (GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {
				GetPet()->WipeHateList();
				GetPet()->AddToHateList(attack_target, 1);
				GetPet()->SetTarget(attack_target);
			}
		}
	}
}

void Bot::BotPullerProcess(Client* bot_owner, Raid* raid) {
	SetAttackFlag(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	bot_owner->SetBotPulling(false);

	if (NOT_HOLDING && NOT_PASSIVE) {
		auto pull_target = bot_owner->GetTarget();
		if (pull_target) {
			if (raid) {
				const auto msg = fmt::format("Pulling {} to the group..", pull_target->GetCleanName());
				raid->RaidSay(msg.c_str(), GetCleanName(), 0, 100);
			} else {
				BotGroupSay(
					this,
					fmt::format(
						"Pulling {}.",
						pull_target->GetCleanName()
					).c_str()
				);
			}

			InterruptSpell();
			WipeHateList();
			AddToHateList(pull_target, 1);
			SetTarget(pull_target);
			SetPullingFlag();
			bot_owner->SetBotPulling();
			if (HasPet() && (GetClass() != Class::Enchanter || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

				GetPet()->WipeHateList();
				GetPet()->SetTarget(nullptr);
				m_previous_pet_order = GetPet()->GetPetOrder();
				GetPet()->SetPetOrder(SPO_Guard);
			}
		}
	}
}

void Bot::Depop() {
	WipeHateList();
	entity_list.RemoveFromHateLists(this);

	if (HasPet())
		GetPet()->Depop();

	_botOwner = nullptr;
	_botOwnerCharacterID = 0;

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

		DataBucket::GetDataBuckets(this);
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
		auto_save_timer.Start(RuleI(Bots, AutosaveIntervalSeconds) * 1000);
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

		if (auto raid = entity_list.GetRaidByBotName(GetName())) {
			// Safety Check to confirm we have a valid raid
			auto owner = GetBotOwner();
			if (owner && !raid->IsRaidMember(owner->GetCleanName())) {
				Bot::RemoveBotFromRaid(this);
			} else {
				SetRaidGrouped(true);
				raid->LearnMembers();
				raid->VerifyRaid();
			}
		}
		else if (auto group = entity_list.GetGroupByMobName(GetName())) {
			// Safety Check to confirm we have a valid group
			auto owner = GetBotOwner();
			if (owner && !group->IsGroupMember(owner->GetCleanName())) {
				Bot::RemoveBotFromGroup(this, group);
			} else {
				SetGrouped(true);
				group->LearnMembers();
				group->VerifyGroup();
			}
		}

		return true;
	}

	return false;
}

// Deletes the inventory record for the specified item from the database for this bot.
void Bot::RemoveBotItemBySlot(uint16 slot_id)
{
	if (!GetBotID()) {
		return;
	}

	database.botdb.DeleteItemBySlot(GetBotID(), slot_id);

	m_inv.DeleteItem(slot_id);
	UpdateEquipmentLight();
}

// Retrieves all the inventory records from the database for this bot.
void Bot::GetBotItems(EQ::InventoryProfile &inv)
{
	if (!GetBotID()) {
		return;
	}

	if (!database.botdb.LoadItems(GetBotID(), inv)) {
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

	if (!database.botdb.LoadItemBySlot(GetBotID(), slot_id, item_id) && GetBotOwner() && GetBotOwner()->IsClient()) {
		GetBotOwner()->CastToClient()->Message(
			Chat::White,
			fmt::format(
				"Failed to load slot ID {} for {}.",
				slot_id,
				GetCleanName()
			).c_str()
		);
	}

	return item_id;
}

void Bot::SetLevel(uint8 in_level, bool command) {
	if (in_level > 0)
		Mob::SetLevel(in_level, command);
}

void Bot::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	if (ns) {
		Mob::FillSpawnStruct(ns, ForWho);
		ns->spawn.afk = 0;
		ns->spawn.lfg = 0;
		ns->spawn.anon = 0;
		ns->spawn.gm = 0;
		if (IsInAGuild())
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
		for (int i = EQ::textures::textureBegin; i < EQ::textures::weaponPrimary; i++) {
			inst = GetBotItem(i);
			if (inst) {
				item = inst->GetItem();
				if (item != nullptr) {
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
		if (inst) {
			item = inst->GetItem();

			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment.Primary.Material = Strings::ToUnsignedInt(&item->IDFile[2]);


				ns->spawn.equipment_tint.Primary.Color = GetEquipmentColor(EQ::textures::weaponPrimary);
			}
		}

		inst = GetBotItem(EQ::invslot::slotSecondary);
		if (inst) {
			item = inst->GetItem();

			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment.Secondary.Material = Strings::ToUnsignedInt(&item->IDFile[2]);

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
	if (bot_owner && bot_owner->HasGroup()) {
		std::vector<int> bot_class_spawn_limits;
		std::vector<int> bot_class_spawned_count = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		for (uint8 class_id = Class::Warrior; class_id <= Class::Berserker; class_id++) {
			auto bot_class_limit = bot_owner->GetBotSpawnLimit(class_id);
			bot_class_spawn_limits.push_back(bot_class_limit);
		}

		auto* g = bot_owner->GetGroup();
		if (g) {
			uint32 group_id = g->GetID();
			std::list<uint32> active_bots;

			auto spawned_bots_count = 0;
			auto bot_spawn_limit = bot_owner->GetBotSpawnLimit();

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
						Group::RemoveFromGroup(b);
						g->UpdatePlayer(bot_owner);
						continue;
					}

					auto spawned_bot_count_class = bot_class_spawned_count[b->GetClass() - 1];

					if (
						auto bot_spawn_limit_class = bot_class_spawn_limits[b->GetClass() - 1];
						bot_spawn_limit_class >= 0 &&
						spawned_bot_count_class >= bot_spawn_limit_class
					) {
						Group::RemoveFromGroup(b);
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
						Group::RemoveFromGroup(b);
					}
				}
			}
		}
	}
}

// Returns TRUE if there is atleast 1 bot in the specified group
bool Bot::GroupHasBot(Group* group) {
	bool Result = false;
	if (group) {
		for (int Counter = 0; Counter < MAX_GROUP_MEMBERS; Counter++) {
			if (group->members[Counter] == nullptr)
				continue;

			if (group->members[Counter]->IsBot()) {
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

void Bot::LevelBotWithClient(Client* c, uint8 new_level, bool send_appearance) {
	// This essentially performs a '#bot update,' with appearance packets, based on the current methods.
	// This should not be called outside of Client::SetEXP() due to its lack of rule checks.

	if (c) {
		const auto &l = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());

		for (const auto &e : l) {
			if (e && (e->GetLevel() != c->GetLevel())) {
				int levels_change = (new_level - e->GetLevel());

				if (levels_change < 0) {
					parse->EventBot(EVENT_LEVEL_DOWN, e, nullptr, std::to_string(std::abs(levels_change)), 0);
				} else {
					parse->EventBot(EVENT_LEVEL_UP, e, nullptr, std::to_string(levels_change), 0);
				}

				e->SetPetChooser(false); // not sure what this does, but was in bot 'update' code
				e->CalcBotStats(c->GetBotOption(Client::booStatsUpdate));

				if (send_appearance) {
					e->SendLevelAppearance();
				}

				if (!RuleB(Bots, BotHealOnLevel)) {
					const int64 max_hp = e->CalcMaxHP();
					if (e->GetHP() > max_hp) {
						e->SetHP(max_hp);
					}
				} else {
					e->SetHP(e->CalcMaxHP());
					e->SetMana(e->CalcMaxMana());
				}

				e->SendHPUpdate();
				e->SendAppearancePacket(AppearanceType::WhoLevel, new_level, true, true); // who level change
				e->AI_AddBotSpells(e->GetBotSpellID());
			}
		}
	}
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

		if (uint8 material_from_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id); material_from_slot != EQ::textures::materialInvalid) {
			equipment[slot_id] = item_id; // npc has more than just material slots. Valid material should mean valid inventory index
			if (GetID()) { // temp hack fix
				SendWearChange(material_from_slot);
			}
		}

		UpdateEquipmentLight();
		if (UpdateActiveLight() && GetID()) { // temp hack fix
			SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
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
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}
}

void Bot::BotTradeAddItem(const EQ::ItemInstance* inst, uint16 slot_id, bool save_to_database)
{
	if (!inst) {
		return;
	}

	if (save_to_database) {
		if (!database.botdb.SaveItemBySlot(this, slot_id, inst)) {
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

int16 Bot::HasBotItem(uint32 item_id) {
	EQ::ItemInstance const *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}
		if (inst->GetID() == item_id) {
			return slot_id;
		}
	}
	return INVALID_INDEX;
}

void Bot::RemoveBotItem(uint32 item_id) {
	EQ::ItemInstance *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}


		if (inst->GetID() == item_id) {
			RemoveBotItemBySlot(slot_id);
			BotRemoveEquipItem(slot_id);
			CalcBotStats(GetOwner()->CastToClient()->GetBotOption(Client::booStatsUpdate));
			return;
		}
	}
}

bool Bot::RemoveBotFromGroup(Bot* bot, Group* group) {
	bool Result = false;
	if (bot && group && bot->HasGroup())  {
		if (!group->IsLeader(bot)) {
			bot->SetFollowID(0);
			if (group->DelMember(bot)) {
				group->DelMemberOOZ(bot->GetName());
				Group::RemoveFromGroup(bot);
				if (group->GroupCount() < 2) {
					group->DisbandGroup();
				}
			}
		} else {
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (!group->members[i]) {
					continue;
				}

				group->members[i]->SetFollowID(0);
			}
			group->DisbandGroup();
			Group::RemoveFromGroup(bot);
		}
		Result = true;
	}
	return Result;
}

bool Bot::AddBotToGroup(Bot* bot, Group* group) {
	bool result = false;
	if (!group || group->GroupCount() >= MAX_GROUP_MEMBERS) {
		return result;
	}

	if (bot && group->AddMember(bot)) {
		if (group->GetLeader()) {
			bot->SetFollowID(group->GetLeader()->GetID());
			// Need to send this only once when a group is formed with a bot so the client knows it is also the group leader
			if (group->GroupCount() == 2 && group->GetLeader()->IsClient()) {
				group->UpdateGroupAAs();
				Mob *TempLeader = group->GetLeader();
				group->SendUpdate(groupActUpdate, TempLeader);
			}
		}
		group->VerifyGroup();
		group->SendGroupJoinOOZ(bot);
		result = true;
	}
	return result;
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
		int16 to_bot_slot = invslot::SLOT_INVALID;

		ClientTrade(ItemInstance* item, int16 from) : trade_item_instance(item), from_client_slot(from) { }
	};

	struct ClientReturn {
		ItemInstance* return_item_instance;
		int16 from_bot_slot;
		int16 to_client_slot = invslot::SLOT_INVALID;

		ClientReturn(ItemInstance* item, int16 from) : return_item_instance(item), from_bot_slot(from) { }
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

		for (int m = EQ::invaug::SOCKET_BEGIN; m <= EQ::invaug::SOCKET_END; ++m) {
			const auto augment = trade_instance->GetAugment(m);
			if (!augment) {
				continue;
			}

			if (!CheckLoreConflict(augment->GetItem())) {
				continue;
			}

			linker.SetLinkType(EQ::saylink::SayLinkItemInst);
			linker.SetItemInst(augment);

			item_link = linker.GenerateLink();

			client->Message(
					Chat::Yellow,
					fmt::format(
							"{} already has {}, the trade has been cancelled!",
							GetCleanName(),
							item_link
					).c_str()
			);
			client->ResetTrade();
			return;
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

				if (trade_instance->GetItem()->ItemType == EQ::item::ItemTypeAugmentation) {
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
							auto equipped_secondary_weapon = GetBotItem(invslot::slotSecondary);
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
							auto equipped_primary_weapon = GetBotItem(invslot::slotPrimary);
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
					client_return.push_back(ClientReturn(GetBotItem(index), index));
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

			//	// TODO: add logging

			if (!database.botdb.DeleteItemBySlot(GetBotID(), return_iterator.from_bot_slot)) {
				OwnerMessage(
					fmt::format(
						"Failed to delete item by slot from slot {}.",
						return_iterator.from_bot_slot
					).c_str()
				);
			}

			BotRemoveEquipItem(return_iterator.from_bot_slot);

			if (parse->BotHasQuestSub(EVENT_UNEQUIP_ITEM_BOT)) {
				const auto& export_string = fmt::format(
					"{} {}",
					return_iterator.return_item_instance->IsStackable() ? return_iterator.return_item_instance->GetCharges() : 1,
					return_iterator.from_bot_slot
				);

				std::vector<std::any> args = { return_iterator.return_item_instance };

				parse->EventBot(
					EVENT_UNEQUIP_ITEM_BOT,
					this,
					nullptr,
					export_string,
					return_iterator.return_item_instance->GetID(),
					&args
				);
			}

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

		if (!trade_iterator.trade_item_instance) {
			continue;
		}

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

		if (parse->BotHasQuestSub(EVENT_EQUIP_ITEM_BOT)) {
			const auto& export_string = fmt::format(
				"{} {}",
				trade_iterator.trade_item_instance->IsStackable() ? trade_iterator.trade_item_instance->GetCharges() : 1,
				trade_iterator.to_bot_slot
			);

			std::vector<std::any> args = { trade_iterator.trade_item_instance };

			parse->EventBot(
				EVENT_EQUIP_ITEM_BOT,
				this,
				nullptr,
				export_string,
				trade_iterator.trade_item_instance->GetID(),
				&args
			);
		}

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
			user_inv = client->GetInv();
			insts[0] = user_inv.GetItem(invslot::slotCursor);
			client->DeleteItemInInventory(invslot::slotCursor);

			// copy to be filtered by task updates, null trade slots preserved for quest event arg
			std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

			// Check if EVENT_TRADE accepts any items
			if (parse->BotHasQuestSub(EVENT_TRADE)) {
				std::vector<std::any> item_list(items.begin(), items.end());
				parse->EventBot(EVENT_TRADE, this, client, "", 0, &item_list);
			}

			CalcBotStats(false);

		} else {
			EQ::ItemInstance* insts[8] = { 0 };
			user_inv = client->GetInv();
			for (int i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_END; ++i) {
				insts[i - EQ::invslot::TRADE_BEGIN] = user_inv.GetItem(i);
				client->DeleteItemInInventory(i);
			}

			// copy to be filtered by task updates, null trade slots preserved for quest event arg
			std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

			// Check if EVENT_TRADE accepts any items
			if (parse->BotHasQuestSub(EVENT_TRADE)) {
				std::vector<std::any> item_list(items.begin(), items.end());
				parse->EventBot(EVENT_TRADE, this, client, "", 0, &item_list);
			}

			CalcBotStats(false);
		}
	}
}

bool Bot::Death(Mob *killer_mob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, KilledByTypes killed_by, bool is_buff_tic)
{
	if (!NPC::Death(killer_mob, damage, spell_id, attack_skill)) {
		return false;
	}

	Mob *my_owner = GetBotOwner();

	if (my_owner && my_owner->IsClient() && my_owner->CastToClient()->GetBotOption(Client::booDeathMarquee)) {
		if (killer_mob) {
			my_owner->CastToClient()->SendMarqueeMessage(Chat::White, 510, 0, 1000, 3000, StringFormat("%s has been slain by %s", GetCleanName(), killer_mob->GetCleanName()));
		} else {
			my_owner->CastToClient()->SendMarqueeMessage(Chat::White, 510, 0, 1000, 3000, StringFormat("%s has been slain", GetCleanName()));
		}
	}

	const auto c = entity_list.GetCorpseByID(GetID());

	if (c) {
		c->Depop();
	}

	LeaveHealRotationMemberPool();

	if ((GetPullingFlag() || GetReturningFlag()) && my_owner && my_owner->IsClient()) {
		my_owner->CastToClient()->SetBotPulling(false);
	}

	if (parse->BotHasQuestSub(EVENT_DEATH_COMPLETE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {}",
			killer_mob ? killer_mob->GetID() : 0,
			damage,
			spell_id,
			static_cast<int>(attack_skill)
		);

		parse->EventBot(EVENT_DEATH_COMPLETE, this, killer_mob, export_string, 0);
	}

	Zone();
	entity_list.RemoveBot(GetID());

	return true;
}

void Bot::Damage(Mob *from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special) {
	if (!from) {
		return;
	}

	if (spell_id == 0) {
		spell_id = SPELL_UNKNOWN;
	}

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if (attacked_timer.Check()) {
		if (parse->BotHasQuestSub(EVENT_ATTACK)) {
			LogCombat("Triggering EVENT_ATTACK due to attack by [{}]", from->GetName());

			parse->EventBot(EVENT_ATTACK, this, from, "", 0);
		}
	}

	attacked_timer.Start(CombatEventTimer_expire);
	// if spell is lifetap add hp to the caster
	if (IsValidSpell(spell_id) && IsLifetapSpell(spell_id)) {
		int64 healed = GetActSpellHealing(spell_id, damage);
		LogCombatDetail("Applying lifetap heal of [{}] to [{}]", healed, GetCleanName());
		HealDamage(healed);
		if (from) {
			entity_list.FilteredMessageClose(this, true, RuleI(Range, SpellMessages), Chat::Emote, FilterSocials, "%s beams a smile at %s", GetCleanName(), from->GetCleanName());
		}
	}

	CommonDamage(from, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);
	if (GetHP() < 0) {
		if (IsCasting())
			InterruptSpell();
		SetAppearance(eaDead);
	}

	SendHPUpdate();
	if (this == from)
		return;

	// Aggro the bot's group members
	if (IsGrouped()) {
		Group *g = GetGroup();
		if (g) {
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (g->members[i] && g->members[i]->IsBot() && from && !g->members[i]->CheckAggro(from) && g->members[i]->IsAttackAllowed(from))
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
				4, 4, 4, 4, 5, 5, 5, 5, 5, 6,		   // 1-10
				6, 6, 6, 6, 7, 7, 7, 7, 7, 8,		   // 11-20
				8, 8, 8, 8, 9, 9, 9, 9, 9, 10,		  // 21-30
				10, 10, 10, 10, 11, 11, 11, 11, 11, 12, // 31-40
				12, 12, 12, 12, 13, 13, 13, 13, 13, 14, // 41-50
				14, 14, 14, 14, 14, 14, 14, 14, 14, 14, // 51-60
				14, 14};								// 61-62
	static uint8 bst_dmg[] = {99,
				4, 4, 4, 4, 4, 5, 5, 5, 5, 5,		// 1-10
				5, 6, 6, 6, 6, 6, 6, 7, 7, 7,		// 11-20
				7, 7, 7, 8, 8, 8, 8, 8, 8, 9,		// 21-30
				9, 9, 9, 9, 9, 10, 10, 10, 10, 10,   // 31-40
				10, 11, 11, 11, 11, 11, 11, 12, 12}; // 41-49
	if (HasClass(Class::Monk)) {
		if (CastToNPC()->GetEquippedItemFromTextureSlot(EQ::textures::armorHands) == 10652 && GetLevel() > 50)
			return 9;
		if (level > 62)
			return 15;
		return mnk_dmg[level];
	} else if (HasClass(Class::Beastlord)) {
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
	if (DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		LogCombatDetail("Preforming a double riposte ([{}] percent chance)", DoubleRipChance);
		defender->Attack(this, EQ::invslot::slotPrimary, true);
	}

	DoubleRipChance = defender->GetAABonuses().GiveDoubleRiposte[1];
	if (DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		if (defender->HasClass(Class::Monk))
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
		if (HasShieldEquipped())
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
	if (hate_override > -1)
		hate = hate_override;

	if (skill == EQ::skills::SkillBash) {
		const EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotSecondary);
		const EQ::ItemData* botweapon = nullptr;
		if (inst)
			botweapon = inst->GetItem();

		if (botweapon) {
			if (botweapon->ItemType == EQ::item::ItemTypeShield)
				hate += botweapon->AC;

			hate = (hate * (100 + GetSpellFuriousBash(botweapon->Focus.Effect)) / 100);
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

	if (!GetTarget() || HasDied())
		return;

	if (my_hit.damage_done > 0)
		CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);

	if (HasSkillProcs())
		TrySkillProc(who, skill, (ReuseTime * 1000));

	if (my_hit.damage_done > 0 && HasSkillProcSuccess())
		TrySkillProc(who, skill, (ReuseTime * 1000), true);
}

void Bot::TryBackstab(Mob *other, int ReuseTime) {
	if (!other)
		return;

	bool bIsBehind = false;
	bool bCanFrontalBS = false;
	const EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotPrimary);
	const EQ::ItemData* botpiercer = nullptr;
	if (inst)
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
		if (level >= 60 && other->GetLevel() <= 45 && !other->CastToNPC()->IsEngaged() && other->GetHP()<= 32000 && other->IsNPC() && zone->random.Real(0, 99) < chance) {
			entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, ASSASSINATES, GetName());
			RogueAssassinate(other);
		} else {
			RogueBackstab(other);
			if (level > 54) {
				float DoubleAttackProbability = ((GetSkill(EQ::skills::SkillDoubleAttack) + GetLevel()) / 500.0f);
				if (zone->random.Real(0, 1) < DoubleAttackProbability) {
					if (other->GetHP() > 0)
						RogueBackstab(other,false,ReuseTime);

					if (tripleChance && other->GetHP() > 0 && tripleChance > zone->random.Int(0, 100))
						RogueBackstab(other,false,ReuseTime);
				}
			}
		}
	} else if (aabonuses.FrontalBackstabMinDmg || itembonuses.FrontalBackstabMinDmg || spellbonuses.FrontalBackstabMinDmg) {
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
	if (botweaponInst) {
		if (GetWeaponDamage(other, botweaponInst))
			other->Damage(this, 32000, SPELL_UNKNOWN, EQ::skills::SkillBackstab);
		else
			other->Damage(this, -5, SPELL_UNKNOWN, EQ::skills::SkillBackstab);
	}

	DoAnim(anim1HPiercing);
}

void Bot::DoClassAttacks(Mob *target, bool IsRiposte) {
	if (!target || spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || GetHP() < 0 || !IsAttackAllowed(target))
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
			case Class::ShadowKnight: {
				CastSpell(SPELL_NPC_HARM_TOUCH, target->GetID());
				knightattack_timer.Start(HarmTouchReuseTime * 1000);

				break;
			}
			case Class::Paladin: {
				if (GetHPRatio() < 20) {
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

	if (taunting && target->IsNPC() && taunt_time) {
		if (GetTarget() && GetTarget()->GetHateTop() && GetTarget()->GetHateTop() != this) {
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
		case Class::Monk: {
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
			break;
		}
	}

	if (!ca_time) {
		return;
	}

	float HasteModifier = (GetHaste() * 0.01f);
	uint16 skill_to_use = -1;
	int bot_level = GetLevel();
	int reuse = (TauntReuseTime * 1000); // Same as Bash and Kick
	bool did_attack = false;
	switch (GetClass()) {
		case Class::Warrior:
			if (bot_level >= RuleI(Combat, NPCBashKickLevel)) {
				bool canBash = false;
				if ((GetRace() == OGRE || GetRace() == TROLL || GetRace() == BARBARIAN) || (m_inv.GetItem(EQ::invslot::slotSecondary) && m_inv.GetItem(EQ::invslot::slotSecondary)->GetItem()->ItemType == EQ::item::ItemTypeShield) || (m_inv.GetItem(EQ::invslot::slotPrimary) && m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->IsType2HWeapon() && GetAA(aa2HandBash) >= 1))
					canBash = true;

				if (!canBash || zone->random.Int(0, 100) > 25)
					skill_to_use = EQ::skills::SkillKick;
				else
					skill_to_use = EQ::skills::SkillBash;
			}
		case Class::Ranger:
		case Class::Beastlord:
			skill_to_use = EQ::skills::SkillKick;
			break;
		case Class::Berserker:
			skill_to_use = EQ::skills::SkillFrenzy;
			break;
		case Class::Cleric:
		case Class::ShadowKnight:
		case Class::Paladin:
			if (bot_level >= RuleI(Combat, NPCBashKickLevel)) {
				if ((GetRace() == OGRE || GetRace() == TROLL || GetRace() == BARBARIAN) || (m_inv.GetItem(EQ::invslot::slotSecondary) && m_inv.GetItem(EQ::invslot::slotSecondary)->GetItem()->ItemType == EQ::item::ItemTypeShield) || (m_inv.GetItem(EQ::invslot::slotPrimary) && m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->IsType2HWeapon() && GetAA(aa2HandBash) >= 1))
					skill_to_use = EQ::skills::SkillBash;
			}
			break;
		case Class::Monk:
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
		case Class::Rogue:
			skill_to_use = EQ::skills::SkillBackstab;
			break;
	}

	int64 dmg = GetBaseSkillDamage(static_cast<EQ::skills::SkillType>(skill_to_use), GetTarget());

	if (skill_to_use == EQ::skills::SkillBash) {
		if (target != this) {
			DoAnim(animTailRake);
			if (GetWeaponDamage(target, GetBotItem(EQ::invslot::slotSecondary)) <= 0 && GetWeaponDamage(target, GetBotItem(EQ::invslot::slotShoulders)) <= 0)
				dmg = DMG_INVULNERABLE;

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
		if (target != this) {
			DoAnim(animKick);
			if (GetWeaponDamage(target, GetBotItem(EQ::invslot::slotFeet)) <= 0)
				dmg = DMG_INVULNERABLE;

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
	if (iOther->IsBot())
		return FACTION_ALLY;

	return NPC::GetReverseFactionCon(iOther);
}

Mob* Bot::GetOwnerOrSelf() {
	Mob* Result = nullptr;
	if (GetBotOwner())
		Result = GetBotOwner();
	else
		Result = this;

	return Result;
}

Mob* Bot::GetOwner() {
	Mob* Result = nullptr;
	Result = GetBotOwner();
	if (!Result)
		SetBotOwner(0);

	return Result;
}

bool Bot::IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined) {
	bool Result = false;
	if (attacker && target) {
		if (attacker == target) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsClient() && target->IsBot() && attacker->CastToClient()->GetPVP() && target->CastToBot()->GetBotOwner()->CastToClient()->GetPVP()) {
			hasRuleDefined = true;
			Result = true;
		} else if (attacker->IsClient() && target->IsBot()) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsBot() && target->IsNPC()) {
			hasRuleDefined = true;
			Result = true;
		} else if (attacker->IsBot() && !target->IsNPC()) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsPet() && attacker->IsFamiliar()) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsBot() && attacker->CastToBot()->GetBotOwner() && attacker->CastToBot()->GetBotOwner()->CastToClient()->GetPVP()) {
			if (target->IsBot() && target->GetOwner() && target->GetOwner()->CastToClient()->GetPVP()) {
				hasRuleDefined = true;
				if (target->GetOwner() == attacker->GetOwner())
					Result = false;
				else
					Result = true;
			} else if (target->IsClient() && target->CastToClient()->GetPVP()) {
				hasRuleDefined = true;
				if (target == attacker->GetOwner())
					Result = false;
				else
					Result = true;
			} else if (target->IsNPC()) {
				hasRuleDefined = true;
				Result = true;
			} else if (!target->IsNPC()) {
				hasRuleDefined = true;
				Result = false;
			}
		}
	}
	return Result;
}

void Bot::EquipBot() {
	GetBotItems(m_inv);
	const EQ::ItemInstance* inst = nullptr;
	const EQ::ItemData* item = nullptr;
	for (int slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (inst) {
			item = inst->GetItem();
			BotTradeAddItem(inst, slot_id, false);
		}
	}
	UpdateEquipmentLight();
}

void Bot::BotOrderCampAll(Client* c, uint8 class_id) {
	if (c) {
		const auto& l = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());
		for (const auto& b : l) {
			if (!class_id || b->GetClass() == class_id) {
				b->Camp(true);
			}
		}
	}
}

void Bot::ProcessBotOwnerRefDelete(Mob* botOwner) {
	if (botOwner && botOwner->IsClient()) {
		std::list<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(botOwner->CastToClient()->CharacterID());
		if (!BotList.empty()) {
			for (std::list<Bot*>::iterator botListItr = BotList.begin(); botListItr != BotList.end(); ++botListItr) {
				Bot* tempBot = *botListItr;
				if (tempBot) {
					tempBot->SetTarget(nullptr);
					tempBot->SetBotOwner(nullptr);
					tempBot->Zone();
				}
			}
		}
	}
}

int64 Bot::CalcMaxMana()
{
	if (IsIntelligenceCasterClass() || IsWisdomCasterClass()) {
		max_mana = (
			GenerateBaseManaPoints() +
			itembonuses.Mana +
			spellbonuses.Mana +
			aabonuses.Mana +
			GroupLeadershipAAManaEnhancement()
		);
		max_mana += itembonuses.heroic_max_mana;
	} else {
		max_mana = 0;
	}

	if (current_mana > max_mana) {
		current_mana = max_mana;
	} else if (max_mana < 0) {
		max_mana = 0;
	}

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

	if (IsBeneficialSpell(spell_id)) {
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

	if (IsMesmerizeSpell(spell_id))
		tic_inc += GetAA(aaMesmerizationMastery);

	return (((duration * increase) / 100) + tic_inc);
}

float Bot::GetAOERange(uint16 spell_id) {
	float range;
	range = spells[spell_id].aoe_range;
	if (range == 0)
		range = spells[spell_id].range;

	if (range == 0)
		range = 10;

	if (IsBardSong(spell_id) && IsBeneficialSpell(spell_id)) {
		float song_bonus = (aabonuses.SongRange + spellbonuses.SongRange + itembonuses.SongRange);
		range += (range * song_bonus / 100.0f);
	}
	range = GetActSpellRange(spell_id, range);
	return range;
}

bool Bot::SpellEffect(Mob* caster, uint16 spell_id, float partial) {
	bool Result = false;
	Result = Mob::SpellEffect(caster, spell_id, partial);
	if (IsGrouped()) {
		Group *g = GetGroup();
		if (g) {
			EQApplicationPacket hp_app;
			CreateHPPacket(&hp_app);
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (g->members[i] && g->members[i]->IsClient())
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

		if (GetClass() != Class::Bard) {
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

bool Bot::SpellOnTarget(
		uint16 spell_id,
		Mob *spelltar,
		int reflect_effectiveness,
		bool use_resist_adjust,
		int16 resist_adjust,
		bool isproc,
		int level_override,
		int duration_override,
		bool disable_buff_overwrite
) {
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
	if (!caster)
		return false;

	if (!IsSacrificeSpell(spell_id) && zone->GetZoneID() != Zones::POKNOWLEDGE && this != caster) {
		Result = Mob::IsImmuneToSpell(spell_id, caster);
		if (!Result) {
			if (caster->IsBot()) {
				if (spells[spell_id].target_type == ST_Undead) {
					if ((GetBodyType() != BodyType::SummonedUndead) && (GetBodyType() != BodyType::Undead) && (GetBodyType() != BodyType::Vampire)) {
						LogSpellsDetail("Bot's target is not an undead");
						return true;
					}
				}
				if (spells[spell_id].target_type == ST_Summoned) {
					if ((GetBodyType() != BodyType::SummonedUndead) && (GetBodyType() != BodyType::Summoned) && (GetBodyType() != BodyType::Summoned2) && (GetBodyType() != BodyType::Summoned3)) {
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
	if (targetType == ST_GroupClientAndPet) {
		if ((spell_id == 1768 && zone->GetZoneID() == 202) || (!IsDetrimentalSpell(spell_id))) {
			CastAction = SingleTarget;
			return true;
		}
	}
	Result = Mob::DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction, slot);
	return Result;
}

bool Bot::DoCastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot, int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish, uint32 item_slot, uint32 aa_id) {
	bool Result = false;
	if (HasClass(Class::Bard))
		cast_time = 0;

	Result = Mob::DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, aa_id);

	if (oSpellWillFinish) {
		const SPDat_Spell_Struct &spell = spells[spell_id];
		*oSpellWillFinish = Timer::GetCurrentTime() + ((spell.recast_time > 20000) ? 10000 : spell.recast_time);
	}
	return Result;
}

int32 Bot::GenerateBaseManaPoints()
{
	int32 bot_mana        = 0;
	int32 WisInt          = 0;
	int32 MindLesserFactor, MindFactor;
	int   wisint_mana     = 0;
	int   base_mana       = 0;
	int   ConvertedWisInt = 0;

	if (IsIntelligenceCasterClass()) {
		WisInt = INT;

		if (
			GetOwner() &&
			GetOwner()->CastToClient() &&
			GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD &&
			RuleB(Character, SoDClientUseSoDHPManaEnd)
		) {
			if (WisInt > 100) {
				ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
				if (WisInt > 201) {
					ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
				}
			} else {
				ConvertedWisInt = WisInt;
			}

			if (GetLevel() < 41) {
				wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
				base_mana   = (GetLevel() * 15);
			} else if (GetLevel() < 81) {
				wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
				base_mana   = (600 + ((GetLevel() - 40) * 30));
			} else {
				wisint_mana = (9 * ConvertedWisInt);
				base_mana   = (1800 + ((GetLevel() - 80) * 18));
			}

			bot_mana = (base_mana + wisint_mana);
		} else {
			if (((WisInt - 199) / 2) > 0) {
				MindLesserFactor = ((WisInt - 199) / 2);
			} else {
				MindLesserFactor = 0;
			}

			MindFactor = WisInt - MindLesserFactor;
			if (WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			} else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
		}
	} else if (IsWisdomCasterClass()) {
		WisInt = WIS;

		if (
			GetOwner() &&
			GetOwner()->CastToClient() &&
			GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD &&
			RuleB(Character, SoDClientUseSoDHPManaEnd)
		) {
			if (WisInt > 100) {
				ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
				if (WisInt > 201) {
					ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
				}
			} else {
				ConvertedWisInt = WisInt;
			}

			if (GetLevel() < 41) {
				wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
				base_mana   = (GetLevel() * 15);
			} else if (GetLevel() < 81) {
				wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
				base_mana   = (600 + ((GetLevel() - 40) * 30));
			} else {
				wisint_mana = (9 * ConvertedWisInt);
				base_mana   = (1800 + ((GetLevel() - 80) * 18));
			}

			bot_mana = (base_mana + wisint_mana);
		} else {
			if (((WisInt - 199) / 2) > 0) {
				MindLesserFactor = ((WisInt - 199) / 2);
			} else {
				MindLesserFactor = 0;
			}

			MindFactor = (WisInt - MindLesserFactor);
			if (WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			} else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
		}
	} else {
		bot_mana = 0;
	}

	max_mana = bot_mana;
	return bot_mana;
}

void Bot::GenerateSpecialAttacks() {
	if (((HasClass(Class::Monk)) || (HasClass(Class::Warrior)) || (HasClass(Class::Ranger)) || (HasClass(Class::Berserker)))	&& (GetLevel() >= 60))
		SetSpecialAbility(SpecialAbility::TripleAttack, 1);
}

bool Bot::DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool& stopLogic) {

	if (
		spellTarget &&
		IsGrouped() &&
		(
			spellTarget->IsBot() ||
			spellTarget->IsClient()
		) &&
		RuleB(Bots, GroupBuffing)
	) {
		bool noGroupSpell = false;
		uint16 thespell = spell_id;
		for (int i = 0; i < AIBot_spells.size(); i++) {
			int j = BotGetSpells(i);
			int spelltype = BotGetSpellType(i);
			bool spellequal = (j == thespell);
			bool spelltypeequal = ((spelltype == 2) || (spelltype == 16) || (spelltype == 32));
			bool spelltypetargetequal = ((spelltype == 8) && (spells[thespell].target_type == ST_Self));
			bool spelltypeclassequal = ((spelltype == 1024) && (HasClass(Class::Shaman)));
			bool slotequal = (slot == EQ::spells::CastingSlot::Item);
			if (spellequal || slotequal) {
				if ((spelltypeequal || spelltypetargetequal) || spelltypeclassequal || slotequal) {
					if (((spells[thespell].effect_id[0] == 0) && (spells[thespell].base_value[0] < 0)) &&
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

		if (!noGroupSpell) {
			Group *g = GetGroup();
			if (g) {
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (g->members[i]) {
						if ((g->members[i]->HasClass(Class::Necromancer)) && (IsEffectInSpell(thespell, SE_AbsorbMagicAtt) || IsEffectInSpell(thespell, SE_Rune))) {
						}
						else
							SpellOnTarget(thespell, g->members[i]);

						if (g->members[i] && g->members[i]->GetPetID())
							SpellOnTarget(thespell, g->members[i]->GetPet());
					}
				}
				SetMana(GetMana() - (GetActSpellCost(thespell, spells[thespell].mana) * (g->GroupCount() - 1)));
			}
		}
		stopLogic = true;
	}
	return true;
}

bool Bot::DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool& stopLogic) {
	bool isMainGroupMGB = false;
	Raid* raid = entity_list.GetRaidByBotName(GetName());

	if (isMainGroupMGB && (GetClass() != Class::Bard)) {
		BotGroupSay(
			this,
			fmt::format(
				"Casting {} as a Mass Group Buff.",
				spells[spell_id].name
			).c_str()
		);
		SpellOnTarget(spell_id, this);
		entity_list.AESpell(this, this, spell_id, true);
	}
	else if (raid)
	{
		std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(raid->GetGroup(GetName()));
		for (auto iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
			if (iter->member) {
				SpellOnTarget(spell_id, iter->member);
				if (iter->member && iter->member->GetPetID())
					SpellOnTarget(spell_id, iter->member ->GetPet());
			}
		}
	}
	else
	{
		Group *g = GetGroup();
		if (g) {
			for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if (g->members[i]) {
					SpellOnTarget(spell_id, g->members[i]);
					if (g->members[i] && g->members[i]->GetPetID())
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
	CalcHeroicBonuses(&itembonuses);
	CalcSpellBonuses(&spellbonuses);
	CalcAABonuses(&aabonuses);
	SetAttackTimer();
	CalcSeeInvisibleLevel();
	CalcInvisibleLevel();
	ProcessItemCaps();
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
	int64 hpregen_cap = 0;
	hpregen_cap = (RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA / 25);
	hpregen_cap += (aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap);
	return (hpregen_cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int64 Bot::CalcManaRegenCap() {
	int64 cap = RuleI(Character, ItemManaRegenCap) + aabonuses.ItemManaRegenCap + itembonuses.heroic_mana_regen;
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
	if (level > 60)
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
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	STR = (val + mod);
	if (STR < 1)
		STR = 1;

	int m = GetMaxSTR();
	if (STR > m)
		STR = m;

	return STR;
}

int32 Bot::CalcSTA() {
	int32 val = (STA + itembonuses.STA + spellbonuses.STA);
	int32 mod = aabonuses.STA;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	STA = (val + mod);
	if (STA < 1)
		STA = 1;

	int m = GetMaxSTA();
	if (STA > m)
		STA = m;

	return STA;
}

int32 Bot::CalcAGI() {
	int32 val = (AGI + itembonuses.AGI + spellbonuses.AGI);
	int32 mod = aabonuses.AGI;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	AGI = (val + mod);
	if (AGI < 1)
		AGI = 1;

	int m = GetMaxAGI();
	if (AGI > m)
		AGI = m;

	return AGI;
}

int32 Bot::CalcDEX() {
	int32 val = (DEX + itembonuses.DEX + spellbonuses.DEX);
	int32 mod = aabonuses.DEX;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	DEX = (val + mod);
	if (DEX < 1)
		DEX = 1;

	int m = GetMaxDEX();
	if (DEX > m)
		DEX = m;

	return DEX;
}

int32 Bot::CalcINT() {
	int32 val = (INT + itembonuses.INT + spellbonuses.INT);
	int32 mod = aabonuses.INT;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	INT = (val + mod);

	if (INT < 1)
		INT = 1;

	int m = GetMaxINT();
	if (INT > m)
		INT = m;

	return INT;
}

int32 Bot::CalcWIS() {
	int32 val = (WIS + itembonuses.WIS + spellbonuses.WIS);
	int32 mod = aabonuses.WIS;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	WIS = (val + mod);

	if (WIS < 1)
		WIS = 1;

	int m = GetMaxWIS();
	if (WIS > m)
		WIS = m;

	return WIS;
}

int32 Bot::CalcCHA() {
	int32 val = (CHA + itembonuses.CHA + spellbonuses.CHA);
	int32 mod = aabonuses.CHA;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	CHA = (val + mod);

	if (CHA < 1)
		CHA = 1;

	int m = GetMaxCHA();
	if (CHA > m)
		CHA = m;

	return CHA;
}

int32 Bot::CalcMR() {
	MR += (itembonuses.MR + spellbonuses.MR + aabonuses.MR);
	if (HasClass(Class::Warrior))
		MR += (GetLevel() / 2);

	if (MR < 1)
		MR = 1;

	if (MR > GetMaxMR())
		MR = GetMaxMR();

	return MR;
}

int32 Bot::CalcFR() {
	int c = GetClass();
	if (c == Class::Ranger) {
		FR += 4;
		int l = GetLevel();
		if (l > 49)
			FR += (l - 49);
	}

	FR += (itembonuses.FR + spellbonuses.FR + aabonuses.FR);

	if (FR < 1)
		FR = 1;

	if (FR > GetMaxFR())
		FR = GetMaxFR();

	return FR;
}

int32 Bot::CalcDR() {
	int c = GetClass();
	if (c == Class::Paladin) {
		DR += 8;
		int l = GetLevel();
		if (l > 49)
			DR += (l - 49);
	} else if (c == Class::ShadowKnight) {
		DR += 4;
		int l = GetLevel();
		if (l > 49)
			DR += (l - 49);
	}

	DR += (itembonuses.DR + spellbonuses.DR + aabonuses.DR);
	if (DR < 1)
		DR = 1;

	if (DR > GetMaxDR())
		DR = GetMaxDR();

	return DR;
}

int32 Bot::CalcPR() {
	int c = GetClass();
	if (c == Class::Rogue) {
		PR += 8;
		int l = GetLevel();
		if (l > 49)
			PR += (l - 49);
	} else if (c == Class::ShadowKnight) {
		PR += 4;
		int l = GetLevel();
		if (l > 49)
			PR += (l - 49);
	}

	PR += (itembonuses.PR + spellbonuses.PR + aabonuses.PR);

	if (PR < 1)
		PR = 1;

	if (PR > GetMaxPR())
		PR = GetMaxPR();

	return PR;
}

int32 Bot::CalcCR() {
	int c = GetClass();
	if (c == Class::Ranger) {
		CR += 4;
		int l = GetLevel();
		if (l > 49)
			CR += (l - 49);
	}

	CR += (itembonuses.CR + spellbonuses.CR + aabonuses.CR);

	if (CR < 1)
		CR = 1;

	if (CR > GetMaxCR())
		CR = GetMaxCR();

	return CR;
}

int32 Bot::CalcCorrup() {
	Corrup = (Corrup + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup);
	if (Corrup > GetMaxCorrup())
		Corrup = GetMaxCorrup();

	return Corrup;
}

int32 Bot::CalcATK() {
	ATK = (itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement());
	return ATK;
}

void Bot::CalcRestState() {
	if (!RuleB(Character, RestRegenEnabled))
		return;

	RestRegenHP = RestRegenMana = RestRegenEndurance = 0;
	if (IsEngaged() || !IsSitting() || !rest_timer.Check(false))
		return;

	uint32 buff_count = GetMaxTotalSlots();
	for (unsigned int j = 0; j < buff_count; j++) {
		if (IsValidSpell(buffs[j].spellid)) {
			if (IsDetrimentalSpell(buffs[j].spellid) && (buffs[j].ticsremaining > 0))
				if (!IsRestAllowedSpell(buffs[j].spellid))
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
	regen += itembonuses.heroic_hp_regen;
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
		if (botclass != Class::Warrior && botclass != Class::Monk && botclass != Class::Rogue && botclass != Class::Berserker) {
			regen = ((((GetSkill(EQ::skills::SkillMeditate) / 10) + (level - (level / 4))) / 4) + 4);
			regen += (spellbonuses.ManaRegen + itembonuses.ManaRegen);
		} else
			regen = (2 + spellbonuses.ManaRegen + itembonuses.ManaRegen);
	} else
		regen = (2 + spellbonuses.ManaRegen + itembonuses.ManaRegen);

	regen += aabonuses.ManaRegen + itembonuses.heroic_mana_regen;

	regen = ((regen * RuleI(Character, ManaRegenMultiplier)) / 100);
	float mana_regen_rate = RuleR(Bots, ManaRegen);
	if (mana_regen_rate < 0.0f)
		mana_regen_rate = 0.0f;

	regen = (regen * mana_regen_rate);
	return regen;
}

uint64 Bot::GetClassHPFactor() {
	uint32 factor;
	switch (GetClass()) {
		case Class::Beastlord:
		case Class::Berserker:
		case Class::Monk:
		case Class::Rogue:
		case Class::Shaman:
			factor = 255;
			break;
		case Class::Bard:
		case Class::Cleric:
			factor = 264;
			break;
		case Class::ShadowKnight:
		case Class::Paladin:
			factor = 288;
			break;
		case Class::Ranger:
			factor = 276;
			break;
		case Class::Warrior:
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
	bot_hp += itembonuses.heroic_max_hp;
	nd += aabonuses.PercentMaxHPChange + spellbonuses.PercentMaxHPChange + itembonuses.PercentMaxHPChange;
	bot_hp = ((float)bot_hp * (float)nd / (float)10000);
	bot_hp += (spellbonuses.FlatMaxHPChange + aabonuses.FlatMaxHPChange + itembonuses.FlatMaxHPChange);
	bot_hp += GroupLeadershipAAHealthEnhancement();
	max_hp = bot_hp;
	if (current_hp > max_hp)
		current_hp = max_hp;

	int hp_perc_cap = spellbonuses.HPPercCap[0];
	if (hp_perc_cap) {
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
	if (end_perc_cap) {
		int curEnd_cap = ((max_end * end_perc_cap) / 100);
		if (cur_end > curEnd_cap || (spellbonuses.EndPercCap[1] && cur_end > spellbonuses.EndPercCap[1]))
			cur_end = curEnd_cap;
	}

	return max_end;
}

int64 Bot::CalcBaseEndurance() {
	int32 base_end = 0;
	int32 base_endurance = 0;
	int32 converted_stats = 0;
	int32 sta_end = 0;
	int stats = 0;
	if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		stats = ((GetSTR() + GetSTA() + GetDEX() + GetAGI()) / 4);

		if (stats > 100) {
			converted_stats = (((stats - 100) * 5 / 2) + 100);
			if (stats > 201) {
				converted_stats -= ((stats - 201) * 5 / 4);
			}
		} else {
			converted_stats = stats;
		}

		if (GetLevel() < 41) {
			sta_end = (GetLevel() * 75 * converted_stats / 1000);
			base_endurance = (GetLevel() * 15);
		} else if (GetLevel() < 81) {
			sta_end = ((3 * converted_stats) + ((GetLevel() - 40) * 15 * converted_stats / 100));
			base_endurance = (600 + ((GetLevel() - 40) * 30));
		} else {
			sta_end = (9 * converted_stats);
			base_endurance = (1800 + ((GetLevel() - 80) * 18));
		}
		base_end = base_endurance + sta_end + itembonuses.heroic_max_end;
	} else {

		stats = (GetSTR() + GetSTA() + GetDEX() + GetAGI());
		int level_base = (GetLevel() * 15);
		int at_most_800 = stats;
		if(at_most_800 > 800)
			at_most_800 = 800;

		int Bonus400to800 = 0;
		int HalfBonus400to800 = 0;
		int Bonus800plus = 0;
		int HalfBonus800plus = 0;

		auto BonusUpto800 = int(at_most_800 / 4) ;

		if(stats > 400) {
			Bonus400to800 = int((at_most_800 - 400) / 4);
			HalfBonus400to800 = int(std::max((at_most_800 - 400), 0) / 8);
			if(stats > 800) {
				Bonus800plus = (int((stats - 800) / 8) * 2);
				HalfBonus800plus = int((stats - 800) / 16);
			}
		}
		int bonus_sum = (BonusUpto800 + Bonus400to800 + HalfBonus400to800 + Bonus800plus + HalfBonus800plus);
		base_end = level_base;
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
	if (newEnd < 0)
		newEnd = 0;
	else if (newEnd > GetMaxEndurance())
		newEnd = GetMaxEndurance();

	cur_end = newEnd;
}

void Bot::DoEnduranceUpkeep() {
	int upkeep_sum = 0;
	int cost_redux = (spellbonuses.EnduranceReduction + itembonuses.EnduranceReduction);
	uint32 buffs_i;
	uint32 buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if (IsValidSpell(buffs[buffs_i].spellid)) {
			int upkeep = spells[buffs[buffs_i].spellid].endurance_upkeep;
			if (upkeep > 0) {
				if (cost_redux > 0) {
					if (upkeep <= cost_redux)
						continue;

					upkeep -= cost_redux;
				}

				if ((upkeep+upkeep_sum) > GetEndurance())
					BuffFadeBySlot(buffs_i);
				else
					upkeep_sum += upkeep;
			}
		}
	}

	if (upkeep_sum != 0)
		SetEndurance(GetEndurance() - upkeep_sum);
}

void Bot::Camp(bool save_to_database) {

	if (IsEngaged() || GetBotOwner()->IsEngaged()) {
		GetBotOwner()->Message(
			Chat::White,
			fmt::format(
				"You cannot camp your bots while in combat"
			).c_str()
		);
		return;
	}

	Sit();

	LeaveHealRotationMemberPool();

	if (save_to_database) {
		Save();
	}

	if (HasGroup() || HasRaid()) {
		Zone();
	} else {
		Depop();
	}
}

void Bot::Zone() {
	if (auto raid = entity_list.GetRaidByBotName(GetName())) {
		raid->MemberZoned(CastToClient());
	}
	else if (HasGroup()) {
		GetGroup()->MemberZoned(this);
	}

	Save();
	Depop();
}

bool Bot::IsArcheryRange(Mob *target) {
	bool result = false;
	if (target) {
		float range = (GetBotArcheryRange() + 5.0);
		range *= range;
		float targetDistance = DistanceSquaredNoZ(m_Position, target->GetPosition());
		float minRuleDistance = (RuleI(Combat, MinRangedAttackDist) * RuleI(Combat, MinRangedAttackDist));
		if ((targetDistance > range) || (targetDistance < minRuleDistance))
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

	for (auto iter : group->members) {
		if (!iter)
			continue;

		// GroupHealer
		switch (iter->GetClass()) {
		case Class::Cleric:
			if (!healer)
				healer = iter;
			else
				switch (healer->GetClass()) {
				case Class::Cleric:
					break;
				default:
					healer = iter;
				}

			break;
		case Class::Druid:
			if (!healer)
				healer = iter;
			else
				switch (healer->GetClass()) {
				case Class::Cleric:
				case Class::Druid:
					break;
				default:
					healer = iter;
				}
			break;
		case Class::Shaman:
			if (!healer)
				healer = iter;
			else
				switch (healer->GetClass()) {
				case Class::Cleric:
				case Class::Druid:
				case Class::Shaman:
					break;
				default:
					healer = iter;
				}
			break;
		case Class::Paladin:
		case Class::Ranger:
		case Class::Beastlord:
			if (!healer)
				healer = iter;
			break;
		default:
			break;
		}

		// GroupSlower
		switch (iter->GetClass()) {
		case Class::Shaman:
			if (!slower)
				slower = iter;
			else
				switch (slower->GetClass()) {
				case Class::Shaman:
					break;
				default:
					slower = iter;
				}
			break;
		case Class::Enchanter:
			if (!slower)
				slower = iter;
			else
				switch (slower->GetClass()) {
				case Class::Shaman:
				case Class::Enchanter:
					break;
				default:
					slower = iter;
				}
			break;
		case Class::Beastlord:
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
}

Bot* Bot::GetBotByBotClientOwnerAndBotName(Client* c, const std::string& botName) {
	Bot* Result = nullptr;
	if (c) {
		std::list<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());
		if (!BotList.empty()) {
			for (auto botListItr = BotList.begin(); botListItr != BotList.end(); ++botListItr) {
				if (std::string((*botListItr)->GetCleanName()) == botName) {
					Result = (*botListItr);
					break;
				}
			}
		}
	}
	return Result;
}

void Bot::ProcessBotGroupInvite(Client* c, std::string const& botName) {
	if (c && !c->HasRaid()) {
		Bot* invitedBot = GetBotByBotClientOwnerAndBotName(c, botName);
		if (!invitedBot) {
			return;
		}

		if (!invitedBot->HasGroup() && !invitedBot->HasRaid()) {
			if (!c->IsGrouped()) {
				auto g = new Group(c);
				if (AddBotToGroup(invitedBot, g)) {
					entity_list.AddGroup(g);
					database.SetGroupLeaderName(g->GetID(), c->GetName());
					g->SaveGroupLeaderAA();
					g->AddToGroup(c);
					g->AddToGroup(invitedBot);
				} else {
					delete g;
				}
			} else {
				if (AddBotToGroup(invitedBot, c->GetGroup())) {
					c->GetGroup()->AddToGroup(invitedBot);
				}
			}
		} else if (invitedBot->HasGroup()) {
			c->MessageString(Chat::LightGray, TARGET_ALREADY_IN_GROUP, invitedBot->GetCleanName());
		}
	}
}

// Processes a group disband request from a Client for a Bot.
void Bot::ProcessBotGroupDisband(Client* c, const std::string& botName) {
	if (c) {
		Bot* tempBot = nullptr;

		if (botName.empty())
			tempBot = GetFirstBotInGroup(c->GetGroup());
		else
			tempBot = GetBotByBotClientOwnerAndBotName(c, botName);

		RemoveBotFromGroup(tempBot, c->GetGroup());
	}
}

// Processes a raid disband request from a Client for a Bot.
void Bot::RemoveBotFromRaid(Bot* bot) {

	Raid* bot_raid = entity_list.GetRaidByBotName(bot->GetName());
	if (bot_raid) {
		uint32 gid = bot_raid->GetGroup(bot->GetName());
		bot_raid->SendRaidGroupRemove(bot->GetName(), gid);
		bot_raid->RemoveMember(bot->GetName());
		bot_raid->GroupUpdate(gid);
		if (!bot_raid->RaidCount()) {
			bot_raid->DisbandRaid();
		}
	}
}

// Handles all client zone change event
void Bot::ProcessClientZoneChange(Client* botOwner) {
	if (botOwner) {
		std::list<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(botOwner->CharacterID());

		for (std::list<Bot*>::iterator itr = BotList.begin(); itr != BotList.end(); ++itr) {
			Bot* tempBot = *itr;

			if (tempBot) {
				Raid* raid = entity_list.GetRaidByBotName(tempBot->GetName());
				if (raid) {
					tempBot->Zone();
				}
				else if (tempBot->HasGroup()) {
					Group* g = tempBot->GetGroup();
					if (g && g->IsGroupMember(botOwner)) {
						if (botOwner->IsClient()) {
							// Modified to not only zone bots if you're the leader.
							// Also zone bots of the non-leader when they change zone.
							if (tempBot->GetBotOwnerCharacterID() == botOwner->CharacterID() && g->IsGroupMember(botOwner))
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

	if (group) {
		for (int Counter = 0; Counter < MAX_GROUP_MEMBERS; Counter++) {
			if (group->members[Counter] == nullptr) {
				continue;
			}

			if (group->members[Counter]->IsBot()) {
				Result = group->members[Counter]->CastToBot();
				break;
			}
		}
	}

	return Result;
}

// Processes a client request to inspect a bot's equipment.
void Bot::ProcessBotInspectionRequest(Bot* inspectedBot, Client* client) {
	if (inspectedBot && client) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_InspectAnswer, sizeof(InspectResponse_Struct));
		InspectResponse_Struct* insr = (InspectResponse_Struct*) outapp->pBuffer;
		insr->TargetID = inspectedBot->GetNPCTypeID();
		insr->playerid = inspectedBot->GetID();

		const EQ::ItemData* item = nullptr;
		const EQ::ItemInstance* inst = nullptr;

		for (int16 L = EQ::invslot::EQUIPMENT_BEGIN; L <= EQ::invslot::EQUIPMENT_END; L++) {
			inst = inspectedBot->GetBotItem(L);

			if (inst) {
				item = inst->GetItem();
				if (item) {
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

// This method is intended to call all necessary methods to do all bot stat calculations, including spell buffs, equipment, AA bonsues, etc.
void Bot::CalcBotStats(bool showtext) {
	if (!GetBotOwner())
		return;

	if (showtext) {
		GetBotOwner()->Message(Chat::Yellow, "Updating %s...", GetCleanName());
	}

	// this code is annoying since many classes change their name and illusions change the race id
	/*if (!IsValidRaceClassCombo()) {
		GetBotOwner()->Message(Chat::Yellow, "A %s - %s bot was detected. Is this Race/Class combination allowed?.", GetRaceIDName(GetRace()), GetClassIDName(GetClass(), GetLevel()));
		GetBotOwner()->Message(Chat::Yellow, "Previous Bots Code releases did not check Race/Class combinations during create.");
		GetBotOwner()->Message(Chat::Yellow, "Unless you are experiencing heavy lag, you should delete and remake this bot.");
	}*/

	if (GetBotOwner()->GetLevel() != GetLevel())
		SetLevel(GetBotOwner()->GetLevel());

	for (int sindex = 0; sindex <= EQ::skills::HIGHEST_SKILL; ++sindex) {
		skills[sindex] = skill_caps.GetSkillCap(GetClass(), (EQ::skills::SkillType)sindex, GetLevel()).cap;
	}

	taunt_timer.Start(1000);

	if (HasClass(Class::Monk) && GetLevel() >= 10) {
		monkattack_timer.Start(1000);
	}

	LoadAAs();
	GenerateSpecialAttacks();

	if (showtext) {
		GetBotOwner()->Message(Chat::Yellow, "Base stats:");
		GetBotOwner()->Message(Chat::Yellow, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), base_hp, AC, max_mana, STR, STA, DEX, AGI, INT, WIS, CHA);
		GetBotOwner()->Message(Chat::Yellow, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",MR,PR,FR,CR,DR,Corrup);
		// Test Code
		if (HasClass(Class::Bard))
			GetBotOwner()->Message(Chat::Yellow, "Bard Skills-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i",
			GetSkill(EQ::skills::SkillBrassInstruments), GetSkill(EQ::skills::SkillPercussionInstruments), GetSkill(EQ::skills::SkillSinging), GetSkill(EQ::skills::SkillStringedInstruments), GetSkill(EQ::skills::SkillWindInstruments));
	}

	//if (Save())
	//	GetBotOwner()->CastToClient()->Message(Chat::White, "%s saved.", GetCleanName());
	//else
	//	GetBotOwner()->CastToClient()->Message(Chat::White, "%s save failed!", GetCleanName());

	CalcBonuses();

	if (showtext) {
		GetBotOwner()->Message(Chat::Yellow, "%s has been updated.", GetCleanName());
		GetBotOwner()->Message(Chat::Yellow, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), max_hp, GetAC(), max_mana, GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA());
		GetBotOwner()->Message(Chat::Yellow, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",GetMR(),GetPR(),GetFR(),GetCR(),GetDR(),GetCorrup());
		// Test Code
		if (HasClass(Class::Bard)) {
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

	if ((iSpellTypes & SPELL_TYPES_DETRIMENTAL) != 0) {
		LogError("[EntityList::Bot_AICheckCloseBeneficialSpells] detrimental spells requested");
		return false;
	}

	if (!caster || !caster->AI_HasSpells()) {
		return false;
	}

	if (iChance < 100) {
		uint8 tmp = zone->random.Int(1, 100);
		if (tmp > iChance)
			return false;
	}

	uint8 botCasterClass = caster->GetClass();

	if (iSpellTypes == SpellType_Heal)	{
		if (botCasterClass == Class::Cleric || botCasterClass == Class::Druid || botCasterClass == Class::Shaman) {
			if (caster->IsRaidGrouped()) {
				Raid* raid = entity_list.GetRaidByBotName(caster->GetName());
				uint32 gid = raid->GetGroup(caster->GetName());
				if (gid < MAX_RAID_GROUPS) {
					std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(gid);
					for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
						if (iter->member && !iter->member->qglobal) {
							if (iter->member->IsClient() && iter->member->GetHPRatio() < 90) {
								if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
									return true;
							}
							else if ((iter->member->HasClass(Class::Warrior) || iter->member->HasClass(Class::Paladin) || iter->member->HasClass(Class::ShadowKnight)) && iter->member->GetHPRatio() < 95) {
								if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
									return true;
							}
							else if (iter->member->HasClass(Class::Enchanter) && iter->member->GetHPRatio() < 80) {
								if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
									return true;
							}
							else if (iter->member->GetHPRatio() < 70) {
								if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
									return true;
							}

						}

						if (iter->member && !iter->member->qglobal && iter->member->HasPet() && iter->member->GetPet()->GetHPRatio() < 50) {
							if (iter->member->GetPet()->GetOwner() != caster && caster->IsEngaged() && iter->member->IsCasting() && iter->member->GetClass() != Class::Enchanter)
								continue;

							if (caster->AICastSpell(iter->member->GetPet(), 100, SpellType_Heal))
								return true;
						}
					}
				}
			}

			else if (caster->HasGroup()) {
				Group *g = caster->GetGroup();
				if (g) {
					for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
						if (g->members[i] && !g->members[i]->qglobal) {
							if (g->members[i]->IsClient() && g->members[i]->GetHPRatio() < 90) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if ((g->members[i]->HasClass(Class::Warrior) || g->members[i]->HasClass(Class::Paladin) || g->members[i]->HasClass(Class::ShadowKnight)) && g->members[i]->GetHPRatio() < 95) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if (g->members[i]->HasClass(Class::Enchanter) && g->members[i]->GetHPRatio() < 80) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if (g->members[i]->GetHPRatio() < 70) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							}
						}

						if (g->members[i] && !g->members[i]->qglobal && g->members[i]->HasPet() && g->members[i]->GetPet()->GetHPRatio() < 50) {
							if (g->members[i]->GetPet()->GetOwner() != caster && caster->IsEngaged() && g->members[i]->IsCasting() && g->members[i]->GetClass() != Class::Enchanter )
								continue;

							if (caster->AICastSpell(g->members[i]->GetPet(), 100, SpellType_Heal))
								return true;
						}
					}
				}
			}
		}

		if ((botCasterClass == Class::Paladin || botCasterClass == Class::Beastlord || botCasterClass == Class::Ranger) && (caster->HasGroup() || caster->IsRaidGrouped())) {
			float hpRatioToHeal = 25.0f;
			switch(caster->GetBotStance()) {
			case Stance::Reactive:
			case Stance::Balanced:
				hpRatioToHeal = 50.0f;
				break;
			case Stance::Burn:
			case Stance::AEBurn:
				hpRatioToHeal = 20.0f;
				break;
			case Stance::Aggressive:
			case Stance::Efficient:
			default:
				hpRatioToHeal = 25.0f;
				break;
			}
			if (caster->IsRaidGrouped()) {
				if (auto raid = entity_list.GetRaidByBotName(caster->GetName())) {
					uint32 gid = raid->GetGroup(caster->GetName());
					if (gid < MAX_RAID_GROUPS) {
						std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(gid);
						for (std::vector<RaidMember>::iterator iter = raid_group_members.begin();
							 iter != raid_group_members.end(); ++iter) {
							if (iter->member && !iter->member->qglobal) {
								if (iter->member->IsClient() && iter->member->GetHPRatio() < hpRatioToHeal) {
									if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
										return true;
								} else if (
									(iter->member->HasClass(Class::Warrior) || iter->member->HasClass(Class::Paladin) ||
									 iter->member->HasClass(Class::ShadowKnight)) &&
									iter->member->GetHPRatio() < hpRatioToHeal) {
									if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
										return true;
								} else if (iter->member->HasClass(Class::Enchanter) &&
										   iter->member->GetHPRatio() < hpRatioToHeal) {
									if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
										return true;
								} else if (iter->member->GetHPRatio() < hpRatioToHeal / 2) {
									if (caster->AICastSpell(iter->member, 100, SpellType_Heal))
										return true;
								}
							}

							if (iter->member && !iter->member->qglobal && iter->member->HasPet() &&
								iter->member->GetPet()->GetHPRatio() < 25) {
								if (iter->member->GetPet()->GetOwner() != caster && caster->IsEngaged() &&
									iter->member->IsCasting() && iter->member->GetClass() != Class::Enchanter)
									continue;

								if (caster->AICastSpell(iter->member->GetPet(), 100, SpellType_Heal))
									return true;
							}
						}
					}
				}
			}
			else if (caster->HasGroup()) {
				if (auto g = caster->GetGroup()) {
					for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
						if (g->members[i] && !g->members[i]->qglobal) {
							if (g->members[i]->IsClient() && g->members[i]->GetHPRatio() < hpRatioToHeal) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if ((g->members[i]->HasClass(Class::Warrior) || g->members[i]->HasClass(Class::Paladin) ||
										g->members[i]->HasClass(Class::ShadowKnight)) &&
									   g->members[i]->GetHPRatio() < hpRatioToHeal) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if (g->members[i]->HasClass(Class::Enchanter) &&
									   g->members[i]->GetHPRatio() < hpRatioToHeal) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							} else if (g->members[i]->GetHPRatio() < hpRatioToHeal / 2) {
								if (caster->AICastSpell(g->members[i], 100, SpellType_Heal))
									return true;
							}
						}

						if (g->members[i] && !g->members[i]->qglobal && g->members[i]->HasPet() &&
							g->members[i]->GetPet()->GetHPRatio() < 25) {
							if (g->members[i]->GetPet()->GetOwner() != caster && caster->IsEngaged() &&
								g->members[i]->IsCasting() && g->members[i]->GetClass() != Class::Enchanter)
								continue;

							if (caster->AICastSpell(g->members[i]->GetPet(), 100, SpellType_Heal))
								return true;
						}
					}
				}
			}
		}
	}

	if (iSpellTypes == SpellType_Buff) {
		uint8 chanceToCast = caster->IsEngaged() ? caster->GetChanceToCastBySpellType(SpellType_Buff) : 100;
		if (botCasterClass == Class::Bard) {
			if (caster->AICastSpell(caster, chanceToCast, SpellType_Buff)) {
				return true;
			} else
				return false;
		}

		if (caster->IsRaidGrouped()) {
			Raid* raid = entity_list.GetRaidByBotName(caster->GetName());
			uint32 g = raid->GetGroup(caster->GetName());
			if (g < MAX_RAID_GROUPS) {
				std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(g);
				for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
					if (iter->member) {
						if (caster->AICastSpell(iter->member, chanceToCast, SpellType_Buff) || caster->AICastSpell(iter->member->GetPet(), chanceToCast, SpellType_Buff))
							return true;
					}
				}
			}
		}
		if (caster->HasGroup()) {
			Group *g = caster->GetGroup();
			if (g) {
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (g->members[i]) {
						if (caster->AICastSpell(g->members[i], chanceToCast, SpellType_Buff) || caster->AICastSpell(g->members[i]->GetPet(), chanceToCast, SpellType_Buff))
							return true;
					}
				}
			}
		}
	}

	if (iSpellTypes == SpellType_Cure) {
		if (caster->IsRaidGrouped()) {
			Raid* raid = entity_list.GetRaidByBotName(caster->GetName());
			uint32 gid = raid->GetGroup(caster->GetName());
			if (gid < MAX_RAID_GROUPS) {
				std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(gid);
				for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
					if (iter->member && caster->GetNeedsCured(iter->member)) {
						if (caster->AICastSpell(iter->member, caster->GetChanceToCastBySpellType(SpellType_Cure), SpellType_Cure))
							return true;
						else if (botCasterClass == Class::Bard) {
							return false;
						}
					}

					if (iter->member && iter->member->GetPet() && caster->GetNeedsCured(iter->member->GetPet())) {
						if (caster->AICastSpell(iter->member->GetPet(), (int)caster->GetChanceToCastBySpellType(SpellType_Cure) / 4, SpellType_Cure))
							return true;
					}
				}
			}
		}
		else if (caster->HasGroup()) {
			Group *g = caster->GetGroup();
			if (g) {
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (g->members[i] && caster->GetNeedsCured(g->members[i])) {
						if (caster->AICastSpell(g->members[i], caster->GetChanceToCastBySpellType(SpellType_Cure), SpellType_Cure))
							return true;
						else if (botCasterClass == Class::Bard)
							return false;
					}

					if (g->members[i] && g->members[i]->GetPet() && caster->GetNeedsCured(g->members[i]->GetPet())) {
						if (caster->AICastSpell(g->members[i]->GetPet(), (int)caster->GetChanceToCastBySpellType(SpellType_Cure)/4, SpellType_Cure))
							return true;
					}
				}
			}
		}
	}

	if (iSpellTypes == SpellType_HateRedux) {
		if (!caster->IsEngaged())
			return false;

		if (caster->IsRaidGrouped()) {
			Raid* raid = entity_list.GetRaidByBotName(caster->GetName());
			uint32 gid = raid->GetGroup(caster->GetName());
			if (gid < MAX_RAID_GROUPS) {
				std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(gid);
				for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
					if (iter->member && caster->GetNeedsHateRedux(iter->member)) {
						if (caster->AICastSpell(iter->member, caster->GetChanceToCastBySpellType(SpellType_HateRedux), SpellType_HateRedux))
							return true;
					}
				}
			}
		}
		else if (caster->HasGroup()) {
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
		if (botCasterClass == Class::Bard || caster->IsEngaged())
			return false;

		//added raid check
		if (caster->IsRaidGrouped()) {
			Raid* raid = entity_list.GetRaidByBotName(caster->GetName());
			uint32 g = raid->GetGroup(caster->GetName());
			if (g < MAX_RAID_GROUPS) {
				std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(g);
				for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
					if (iter->member &&
						(caster->AICastSpell(iter->member, iChance, SpellType_PreCombatBuff) ||
						caster->AICastSpell(iter->member->GetPet(), iChance, SpellType_PreCombatBuff))
					) {
						return true;
					}
				}
			}
		} else if (caster->HasGroup()) {
			const auto g = caster->GetGroup();
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
		if (botCasterClass == Class::Bard) {
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
	if (botID > 0) {
		for (const auto& m: mob_list) {
			if (!m.second)
				continue;

			if (m.second->IsBot() && m.second->CastToBot()->GetBotID() == botID) {
				Result = m.second;
				break;
			}
		}
	}
	return Result;
}

Bot* EntityList::GetBotByBotID(uint32 botID) {
	Bot* Result = nullptr;
	if (botID > 0) {
		for (std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if (tempBot && tempBot->GetBotID() == botID) {
				Result = tempBot;
				break;
			}
		}
	}
	return Result;
}

Bot* EntityList::GetBotByBotName(std::string_view botName) {
	Bot* Result = nullptr;
	if (!botName.empty()) {
		for (const auto b : bot_list) {
			if (b && std::string_view(b->GetName()) == botName) {
				Result = b;
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

		if (parse->BotHasQuestSub(EVENT_SPAWN)) {
			parse->EventBot(EVENT_SPAWN, new_bot, nullptr, "", 0);
		}

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

		if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_SPAWN_ZONE)) {
			new_bot->DispatchZoneControllerEvent(EVENT_SPAWN_ZONE, new_bot, "", 0, nullptr);
		}
	}
}

std::list<Bot*> EntityList::GetBotsByBotOwnerCharacterID(uint32 botOwnerCharacterID) {
	std::list<Bot*> Result;
	if (botOwnerCharacterID > 0) {
		for (std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if (tempBot && tempBot->GetBotOwnerCharacterID() == botOwnerCharacterID)
				Result.push_back(tempBot);
		}
	}
	return Result;
}

bool EntityList::RemoveBot(uint16 entityID) {
	bool Result = false;
	if (entityID > 0) {
		for (std::list<Bot*>::iterator botListItr = bot_list.begin(); botListItr != bot_list.end(); ++botListItr) {
			Bot* tempBot = *botListItr;
			if (tempBot && tempBot->GetID() == entityID) {
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

	for (const auto& m : mob_list) {
	curMob = m.second;
		if (curMob && DistanceNoZ(curMob->GetPosition(), client->GetPosition()) <= Distance) {
			if (curMob->IsTrackable()) {
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
				}
				if (NamedOnly) {
					bool ContinueFlag = false;
					const char *CurEntityName = cur_entity->GetName();
					for (int Index = 0; Index < MyArraySize; Index++) {
						if (!strncasecmp(CurEntityName, MyArray[Index], strlen(MyArray[Index])) || (Extras)) {
							ContinueFlag = true;
							break;
						}
					}
					if (ContinueFlag)
						continue;
				}

				CurrentCon = client->GetLevelCon(cur_entity->GetLevel());
				if (CurrentCon != LastCon) {
					if (LastCon != -1)
						WindowText += "</c>";

					LastCon = CurrentCon;
					switch(CurrentCon) {
						case ConsiderColor::Green: {
							WindowText += "<c \"#00FF00\">";
							break;
						}
						case ConsiderColor::LightBlue: {
							WindowText += "<c \"#8080FF\">";
							break;
						}
						case ConsiderColor::DarkBlue: {
							WindowText += "<c \"#2020FF\">";
							break;
						}
						case ConsiderColor::Yellow: {
							WindowText += "<c \"#FFFF00\">";
							break;
						}
						case ConsiderColor::Red: {
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
				if (strlen(WindowText.c_str()) > 4000) {
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

uint8 Bot::GetNumberNeedingHealedInGroup(uint8 hpr, bool includePets, Raid* raid) {

	uint8 need_healed = 0;
	if (HasGroup()) {

		auto group_members = GetGroup();
		if (group_members) {

			for (auto member : group_members->members) {
				if (member && !member->qglobal) {

					if (member->GetHPRatio() <= hpr) {
						need_healed++;
					}

					if (includePets && member->GetPet() && member->GetPet()->GetHPRatio() <= hpr) {
						need_healed++;
					}
				}
			}
		}
	}
	return GetNumberNeedingHealedInRaidGroup(need_healed, hpr, includePets, raid);
}

int Bot::GetRawACNoShield(int &shield_ac) {
	int ac = itembonuses.AC + spellbonuses.AC;
	shield_ac = 0;
	EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotSecondary);
	if (inst) {
		if (inst->GetItem()->ItemType == EQ::item::ItemTypeShield) {
			ac -= inst->GetItem()->AC;
			shield_ac = inst->GetItem()->AC;
			for (uint8 i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (inst->GetAugment(i)) {
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
		if (inst) {
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

	if (!g || (g->GroupCount() < 3))
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
	if (!g || (g->GroupCount() < 3))
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
	if (!g || (g->GroupCount() < 3))
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

	if (!g || (g->GroupCount() < 3))
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
	if (tar) {
		if (tar->FindType(SE_PoisonCounter) || tar->FindType(SE_DiseaseCounter) || tar->FindType(SE_CurseCounter) || tar->FindType(SE_CorruptionCounter)) {
			uint32 buff_count = tar->GetMaxTotalSlots();
			int buffsWithCounters = 0;
			needCured = true;
			for (unsigned int j = 0; j < buff_count; j++) {
				if (IsValidSpell(tar->GetBuffs()[j].spellid)) {
					if (CalculateCounters(tar->GetBuffs()[j].spellid) > 0) {
						buffsWithCounters++;
						if (buffsWithCounters == 1 && (tar->GetBuffs()[j].ticsremaining < 2 || (int32)((tar->GetBuffs()[j].ticsremaining * 6) / tar->GetBuffs()[j].counters) < 2)) {
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

	if (tar->IsBot()) {
		switch (tar->GetClass()) {
		case Class::Rogue:
			if (tar->CanFacestab() || tar->CastToBot()->m_evade_timer.Check(false))
				return false;
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			return true;
		default:
			return false;
		}
	}

	return false;
}

bool Bot::HasOrMayGetAggro() {
	bool mayGetAggro = false;
	if (GetTarget() && GetTarget()->GetHateTop()) {
		Mob *topHate = GetTarget()->GetHateTop();
		if (topHate == this)
			mayGetAggro = true;
		else {
			uint32 myHateAmt = GetTarget()->GetHateAmount(this);
			uint32 topHateAmt = GetTarget()->GetHateAmount(topHate);

			if (myHateAmt > 0 && topHateAmt > 0 && (uint8)((myHateAmt / topHateAmt) * 100) > 90)
				mayGetAggro = true;
		}
	}
	return mayGetAggro;
}

void Bot::SetDefaultBotStance() {
	_botStance = HasClass(Class::Warrior) ? Stance::Aggressive : Stance::Balanced;
}

void Bot::BotGroupSay(Mob* speaker, const char* msg, ...) {
	char buf[1000];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, 1000, msg, ap);
	va_end(ap);

	if (speaker->IsRaidGrouped()) {
		Raid* r = entity_list.GetRaidByBotName(speaker->GetName());
		if (r) {
			for (const auto& m : r->members) {
				if (m.member && !m.is_bot) {
					m.member->FilteredMessageString(
						speaker,
						Chat::PetResponse,
						FilterSocials,
						GENERIC_SAY,
						speaker->GetCleanName(),
						buf
					);
				}
			}
		}
	}
	else if (speaker->HasGroup()) {
		Group* g = speaker->GetGroup();
		if (g) {
			for (auto& m : g->members) {
				if (m && !m->IsBot()) {
					m->FilteredMessageString(
						speaker,
						Chat::PetResponse,
						FilterSocials,
						GENERIC_SAY,
						speaker->GetCleanName(),
						buf
					);
				}
			}
		}
	}
	else {
		//speaker->Say("%s", buf);
		speaker->GetOwner()->FilteredMessageString(
			speaker,
			Chat::PetResponse,
			FilterSocials,
			GENERIC_SAY,
			speaker->GetCleanName(),
			buf
		);
	}
}

bool Bot::UseDiscipline(uint32 spell_id, uint32 target) {
	if (!IsValidSpell(spell_id)) {
		BotGroupSay(this, "Not a valid spell.");
		return false;
	}

	const SPDat_Spell_Struct &spell = spells[spell_id];
	uint8 level_to_use = spell.classes[GetClass() - 1];
	if (level_to_use == 255 || level_to_use > GetLevel()) {
		return false;
	}

	if (GetEndurance() > spell.endurance_cost)
		SetEndurance(GetEndurance() - spell.endurance_cost);
	else
		return false;

	if (spell.recast_time > 0) {
		if (CheckDisciplineReuseTimer(spell_id)) {
			if (spells[spell_id].timer_id > 0) {
				SetDisciplineReuseTimer(spell_id);
			}
		} else {
			uint32 remaining_time = (GetDisciplineReuseRemainingTime(spell_id) / 1000);
			OwnerMessage(
				fmt::format(
					"I can use this discipline in {}.",
					Strings::SecondsToTime(remaining_time)
				)
			);
			return false;
		}
	}

	if (IsCasting())
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
			return false;
		}
	}

	return true;
}

void Bot::Signal(int signal_id)
{
	if (parse->BotHasQuestSub(EVENT_SIGNAL)) {
		parse->EventBot(EVENT_SIGNAL, this, nullptr, std::to_string(signal_id), 0);
	}
}

void Bot::SendPayload(int payload_id, std::string payload_value)
{
	if (parse->BotHasQuestSub(EVENT_PAYLOAD)) {
		const auto& export_string = fmt::format("{} {}", payload_id, payload_value);

		parse->EventBot(EVENT_PAYLOAD, this, nullptr, export_string, 0);
	}
}

void Bot::OwnerMessage(const std::string& message)
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

bool Bot::CheckDataBucket(std::string bucket_name, const std::string& bucket_value, uint8 bucket_comparison)
{
	if (!bucket_name.empty() && !bucket_value.empty()) {
		// try to fetch from bot first
		DataBucketKey k = GetScopedBucketKeys();
		k.key = bucket_name;

		auto b = DataBucket::GetData(k);
		if (b.value.empty() && GetBotOwner()) {
			// fetch from owner
			k = GetBotOwner()->GetScopedBucketKeys();
			k.key = bucket_name;

			b = DataBucket::GetData(k);
			if (b.value.empty()) {
				return false;
			}
		}

		if (zone->CompareDataBucket(bucket_comparison, bucket_value, b.value)) {
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

	for (const auto& s : (GetBotEnforceSpellSetting()) ? AIBot_spells_enforced : AIBot_spells) {
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
		if (r) {
			auto group_id = r->GetGroup(GetCleanName());
			if (EQ::ValueWithin(group_id, 0, (MAX_RAID_GROUPS - 1))) {
				for (const auto& m: r->members) {
					if (m.is_bot) {
						continue;
					}
					if (m.member && m.member->IsClient() &&
						(!is_raid_group_only || r->GetGroup(m.member) == group_id)) {
						l.push_back(m.member);

						if (allow_pets && m.member->HasPet()) {
							l.push_back(m.member->GetPet());
						}

						const auto& sbl = entity_list.GetBotListByCharacterID(m.member->CharacterID());
						for (const auto& b: sbl) {
							l.push_back(b);
						}
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
	int level,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only);

	for (const auto& m : l) {
		m->ApplySpellBuff(spell_id, duration, level);
	}
}

void Bot::SetSpellDuration(
	int spell_id,
	int duration,
	int level,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only);

	for (const auto& m : l) {
		m->SetBuffDuration(spell_id, duration, level);
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

float Bot::GetBotCasterMaxRange(float melee_distance_max) {// Calculate caster distances
	float caster_distance_max = 0.0f;
	float caster_distance_min = 0.0f;
	float caster_distance = 0.0f;

	caster_distance_max = GetBotCasterRange() * GetBotCasterRange();
	if (!GetBotCasterRange() && GetLevel() >= GetStopMeleeLevel() && GetClass() >= Class::Warrior && GetClass() <= Class::Berserker) {
		caster_distance_max = MAX_CASTER_DISTANCE[GetClass() - 1];
	}
	if (caster_distance_max) {
		caster_distance_min = melee_distance_max;
		if (caster_distance_max <= caster_distance_min) {
			caster_distance_max = caster_distance_min * 1.25f;
		}
	}
	return caster_distance_max;
}


int32 Bot::CalcItemATKCap()
{
	return RuleI(Character, ItemATKCap) + itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap;
}

bool Bot::CheckSpawnConditions(Client* c) {

	if (c->GetFeigned()) {
		c->Message(Chat::White, "You cannot spawn a bot-group while feigned.");
		return false;
	}

	Raid* raid = entity_list.GetRaidByClient(c);
	if (raid && raid->IsEngaged()) {
		c->Message(Chat::White, "You cannot spawn bots while your raid is engaged.");
		return false;
	}

	auto* owner_group = c->GetGroup();
	if (owner_group) {
		std::list<Client*> member_list;
		owner_group->GetClientList(member_list);
		member_list.remove(nullptr);

		for (auto member_iter : member_list) {
			if (member_iter->IsEngaged() || member_iter->GetAggroCount() > 0) {
				c->Message(Chat::White, "You cannot spawn bots while your group is engaged,");
				return false;
			}
		}
	} else {
		if (c->GetAggroCount() > 0) {
			c->Message(Chat::White, "You cannot spawn bots while you are engaged,");
			return false;
		}
	}

	return true;
}

void Bot::AddBotStartingItems(uint16 race_id, uint8 class_id)
{
	if (!IsPlayerRace(race_id) || !IsPlayerClass(class_id)) {
		return;
	}

	const uint16 race_bitmask  = GetPlayerRaceBit(race_id);
	const uint16 class_bitmask = GetPlayerClassBit(class_id);

	const auto& l = BotStartingItemsRepository::GetWhere(
		content_db,
		fmt::format(
			"(races & {} OR races = 0) AND "
			"(classes & {} OR classes = 0) {}",
			race_bitmask,
			class_bitmask,
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return;
	}

	std::vector<BotInventoriesRepository::BotInventories> v;

	for (const auto& e : l) {
		if (
			CanClassEquipItem(e.item_id) &&
			(CanRaceEquipItem(e.item_id) || RuleB(Bots, AllowBotEquipAnyRaceGear))
		) {
			auto i = BotInventoriesRepository::NewEntity();

			i.bot_id       = GetBotID();
			i.slot_id      = e.slot_id;
			i.item_id      = e.item_id;
			i.inst_charges = e.item_charges;
			i.augment_1    = e.augment_one;
			i.augment_2    = e.augment_two;
			i.augment_3    = e.augment_three;
			i.augment_4    = e.augment_four;
			i.augment_5    = e.augment_five;
			i.augment_6    = e.augment_six;

			v.emplace_back(i);
		}
	}

	if (!v.empty()) {
		BotInventoriesRepository::InsertMany(content_db, v);
	}
}

void Bot::SetSpellRecastTimer(uint16 spell_id, int32 recast_delay) {
	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to set spell recast timer.");
		return;
	}

	if (!recast_delay) {
		recast_delay = CalcSpellRecastTimer(spell_id);
	}

	if (CheckSpellRecastTimer(spell_id)) {
		BotTimer_Struct t;

		t.timer_id    = spells[ spell_id ].timer_id;
		t.timer_value = (Timer::GetCurrentTime() + recast_delay);
		t.recast_time = recast_delay;
		t.is_spell    = true;
		t.is_disc     = false;
		t.spell_id    = spells[ spell_id ].id;
		t.is_item     = false;
		t.item_id     = 0;

		bot_timers.push_back(t);
	} else {
		if (!bot_timers.empty()) {
			for (int i = 0; i < bot_timers.size(); i++) {
				if (
					bot_timers[i].is_spell &&
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[ i ].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = (Timer::GetCurrentTime() + recast_delay);
					bot_timers[i].recast_time = recast_delay;
					break;
				}
			}
		}
	}
}

uint32 Bot::GetSpellRecastTimer(uint16 spell_id)
{
	uint32 result = 0;

	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage("Failed to get spell recast timer.");
		return result;
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_spell &&
				(
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				)
			) {
				result = bot_timers[i].timer_value;
				break;
			}
		}
	}

	return result;
}

uint32 Bot::GetSpellRecastRemainingTime(uint16 spell_id)
{
	uint32 result = 0;

	if (GetSpellRecastTimer(spell_id) > Timer::GetCurrentTime()) {
		result = (GetSpellRecastTimer(spell_id) - Timer::GetCurrentTime());
	}

	return result;
}

bool Bot::CheckSpellRecastTimer(uint16 spell_id)
{
	ClearExpiredTimers();

	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage("Failed to check spell recast timer.");
		return false;
	}

	if (GetSpellRecastTimer(spell_id) < Timer::GetCurrentTime()) {
		return true;
	}

	return false;
}

void Bot::SetDisciplineReuseTimer(uint16 spell_id, int32 reuse_timer)
{
	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to set discipline reuse timer.");
		return;
	}

	if (!reuse_timer) {
		reuse_timer = CalcSpellRecastTimer(spell_id);
	}

	if (CheckDisciplineReuseTimer(spell_id)) {
		BotTimer_Struct t;

		t.timer_id    = spells[ spell_id ].timer_id;
		t.timer_value = (Timer::GetCurrentTime() + reuse_timer);
		t.recast_time = reuse_timer;
		t.is_spell    = false;
		t.is_disc     = true;
		t.spell_id    = spells[ spell_id ].id;
		t.is_item     = false;
		t.item_id     = 0;

		bot_timers.push_back(t);
	} else {
		if (!bot_timers.empty()) {
			for (int i = 0; i < bot_timers.size(); i++) {
				if (
					bot_timers[i].is_disc &&
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = (Timer::GetCurrentTime() + reuse_timer);
					bot_timers[i].recast_time = reuse_timer;
					break;
				}
			}
		}
	}
}

uint32 Bot::GetDisciplineReuseTimer(uint16 spell_id)
{
	uint32 result = 0;

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_disc &&
				(
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				)
			) {
				result = bot_timers[i].timer_value;
				break;
			}
		}
	}

	return result;
}

uint32 Bot::GetDisciplineReuseRemainingTime(uint16 spell_id) {
	uint32 result = 0;

	if (GetDisciplineReuseTimer(spell_id) > Timer::GetCurrentTime()) {
		result = (GetDisciplineReuseTimer(spell_id) - Timer::GetCurrentTime());
	}

	return result;
}

bool Bot::CheckDisciplineReuseTimer(uint16 spell_id)
{
	ClearExpiredTimers();

	if (GetDisciplineReuseTimer(spell_id) < Timer::GetCurrentTime()) { //checks for spells on the same timer
		return true; //can cast spell
	}

	return false;
}

void Bot::SetItemReuseTimer(uint32 item_id, uint32 reuse_timer)
{
	const auto *item = database.GetItem(item_id);

	if (!item) {
		OwnerMessage("Failed to set item reuse timer.");
		return;
	}

	if (item->RecastDelay <= 0) {
		return;
	}

	if (CheckItemReuseTimer(item_id)) {
		BotTimer_Struct t;

		t.timer_id    = (item->RecastType == NegativeItemReuse ? item->ID : item->RecastType);
		t.timer_value = (
			reuse_timer != 0 ?
			(Timer::GetCurrentTime() + reuse_timer) :
			(Timer::GetCurrentTime() + (item->RecastDelay * 1000))
		);
		t.recast_time = (reuse_timer != 0 ? reuse_timer : (item->RecastDelay * 1000));
		t.is_spell    = false;
		t.is_disc     = false;
		t.spell_id    = 0;
		t.is_item     = true;
		t.item_id     = item->ID;

		bot_timers.push_back(t);
	}
	else {
		if (!bot_timers.empty()) {
			for (int i = 0; i < bot_timers.size(); i++) {
				if (
					bot_timers[i].is_item &&
					(
						(
							item->RecastType != 0 &&
							item->RecastType == bot_timers[i].timer_id
						) ||
						bot_timers[i].item_id == item_id
					)
				) {
					bot_timers[i].timer_value = (
						reuse_timer != 0 ?
						(Timer::GetCurrentTime() + reuse_timer) :
						(Timer::GetCurrentTime() + (item->RecastDelay * 1000))
					);
					bot_timers[i].recast_time = (
						reuse_timer != 0 ?
						reuse_timer :
						(item->RecastDelay * 1000)
					);
					break;
				}
			}
		}
	}
}

uint32 Bot::GetItemReuseTimer(uint32 item_id)
{
	uint32 result = 0;
	const EQ::ItemData* item;

	if (item_id) {
		item = database.GetItem(item_id);

		if (!item) {
			OwnerMessage("Failed to get item reuse timer.");
			return result;
		}
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_item &&
				(
					!item_id ||
					(
						(
							item->RecastType != 0 &&
							item->RecastType == bot_timers[i].timer_id
						) ||
						bot_timers[i].item_id == item_id
					)
				)
			) {
				result = bot_timers[i].timer_value;
				break;
			}
		}
	}

	ClearExpiredTimers();

	return result;
}

bool Bot::CheckItemReuseTimer(uint32 item_id)
{
	ClearExpiredTimers();

	if (GetItemReuseTimer(item_id) < Timer::GetCurrentTime()) {
		return true;
	}

	return false;
}

uint32 Bot::GetItemReuseRemainingTime(uint32 item_id)
{
	uint32 result = 0;

	if (GetItemReuseTimer(item_id) > Timer::GetCurrentTime()) {
		result = (GetItemReuseTimer(item_id) - Timer::GetCurrentTime());
	}

	return result;
}

uint32 Bot::CalcSpellRecastTimer(uint16 spell_id)
{
	uint32 result = 0;

	if (spells[spell_id].recast_time == 0 && spells[spell_id].recovery_time == 0) {
		return result;
	} else {
		if (spells[spell_id].recovery_time > spells[spell_id].recast_time) {
			result = spells[spell_id].recovery_time;
		} else {
			result = spells[spell_id].recast_time;
		}
	}

	return result;
}

void Bot::ClearDisciplineReuseTimer(uint16 spell_id)
{
	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage(
			fmt::format(
				"{} is not a valid spell ID.'",
				spell_id
			)
		);
		return;
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_disc &&
				bot_timers[i].timer_value >= Timer::GetCurrentTime()
			) {
				if (
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = 0;
				}
			}
		}
	}

	ClearExpiredTimers();
}

void Bot::ClearItemReuseTimer(uint32 item_id)
{
	const EQ::ItemData* item;

	if (item_id) {
		item = database.GetItem(item_id);

		if (!item) {
			OwnerMessage(
				fmt::format(
					"{} is not a valid item ID.",
					item_id
				)
			);
			return;
		}
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (bot_timers[i].is_item && bot_timers[i].timer_value >= Timer::GetCurrentTime()) {
				if (
					!item_id ||
					(
						(
							item->RecastType != 0 &&
							item->RecastType == bot_timers[i].timer_id
						) ||
						bot_timers[i].item_id == item_id
					)
				) {
					bot_timers[i].timer_value = 0;
				}
			}
		}
	}

	ClearExpiredTimers();
}

void Bot::ClearSpellRecastTimer(uint16 spell_id)
{
	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage(
			fmt::format(
				"{} is not a valid spell ID.",
				spell_id
			)
		);
		return;
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (bot_timers[i].is_spell && bot_timers[i].timer_value >= Timer::GetCurrentTime()) {
				if (
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = 0;
				}
			}
		}
	}

	ClearExpiredTimers();
}


void Bot::ClearExpiredTimers()
{
	if (!bot_timers.empty()) {
		int current = 0;
		int end = bot_timers.size();

		while (current < end) {
			if (bot_timers[current].timer_value < Timer::GetCurrentTime()) {
				bot_timers.erase(bot_timers.begin() + current);
			} else {
				current++;
			}

			end = bot_timers.size();
		}
	}
}

void Bot::TryItemClick(uint16 slot_id)
{
	if (!GetOwner()) {
		return;
	}

	const auto *inst = GetClickItem(slot_id);

	if (!inst) {
		return;
	}

	const auto *item = inst->GetItem();

	if (!item) {
		return;
	}

	if (!CheckItemReuseTimer(item->ID)) {
		uint32 remaining_time = (GetItemReuseRemainingTime(item->ID) / 1000);
		OwnerMessage(
			fmt::format(
				"I can use this item in {}.",
				Strings::SecondsToTime(remaining_time)
			)
		);
		return;
	}

	DoItemClick(item, slot_id);
}

EQ::ItemInstance *Bot::GetClickItem(uint16 slot_id)
{
	EQ::ItemInstance* inst = nullptr;
	const EQ::ItemData* item = nullptr;

	inst = GetBotItem(slot_id);

	if (!inst || !inst->GetItem()) {
		return nullptr;
	}

	item = inst->GetItem();

	if (item->ID == MAG_EPIC_1_0 && !RuleB(Bots, CanClickMageEpicV1)) {
		OwnerMessage(
			fmt::format(
				"{} is currently disabled for bots to click.",
				item->Name
			)
		);
		return nullptr;
	}

	if (item->Click.Effect <= 0) {
		OwnerMessage(
			fmt::format(
				"{} does not have a clickable effect.",
				item->Name
			)
		);
		return nullptr;
	}

	if (!IsValidSpell(item->Click.Effect)) {
		OwnerMessage(
			fmt::format(
				"{} does not have a valid clickable effect.",
				item->Name
			)
		);
		return nullptr;
	}

	if (item->ReqLevel > GetLevel()) {
		OwnerMessage(
			fmt::format(
				"I am below the level requirement of {} for {}.",
				item->ReqLevel,
				item->Name
			)
		);
		return nullptr;
	}

	if (item->Click.Level2 > GetLevel()) {
		OwnerMessage(
			fmt::format(
				"I must be level {} to use {}.",
				item->Click.Level2,
				item->Name
			)
		);
		return nullptr;
	}

	if (inst->GetCharges() == 0) {
		OwnerMessage(
			fmt::format(
				"{} is out of charges.",
				item->Name
			)
		);
		return nullptr;
	}

	return inst;
}

void Bot::DoItemClick(const EQ::ItemData *item, uint16 slot_id)
{
	bool is_casting_bard_song = false;
	Mob* tar = (GetOwner()->GetTarget() ? GetOwner()->GetTarget() : this);

	if (IsCasting()) {
		InterruptSpell();
	}

	SetIsUsingItemClick(true);

	BotGroupSay(
		this,
		fmt::format(
			"Attempting to cast [{}] on {}.",
			spells[item->Click.Effect].name,
			tar->GetCleanName()
		).c_str()
	);

	if (!IsCastWhileInvisibleSpell(item->Click.Effect)) {
		CommonBreakInvisible();
	}

	if (HasClass(Class::Bard) && IsCasting() && casting_spell_slot < EQ::spells::CastingSlot::MaxGems) {
		is_casting_bard_song = true;
	}

	if (HasClass(Class::Bard)) {
		DoBardCastingFromItemClick(is_casting_bard_song, item->CastTime, item->Click.Effect, tar->GetID(), EQ::spells::CastingSlot::Item, slot_id, item->RecastType, item->RecastDelay);
	} else {
		if (!CastSpell(item->Click.Effect, tar->GetID(), EQ::spells::CastingSlot::Item, item->CastTime, 0, 0, slot_id)) {
			OwnerMessage(
				fmt::format(
					"Casting failed for {}. This could be due to zone restrictions, target restrictions or other limiting factors.",
					item->Name
				)
			);
		}
	}

}

uint8 Bot::spell_casting_chances[SPELL_TYPE_COUNT][Class::PLAYER_CLASS_COUNT][Stance::AEBurn][cntHSND] = { 0 };
