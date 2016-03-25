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


#ifndef BOT_COMMAND_H
#define BOT_COMMAND_H

#ifdef BOTS

class Client;
class Seperator;

#include "../common/types.h"
#include "bot.h"


class BCEnum
{
public:
	typedef enum SpellType {
		SpT_None = 0,
		SpT_BindAffinity,
		SpT_Charm,
		SpT_Cure,
		SpT_Depart,
		SpT_Escape,
		SpT_Identify,
		SpT_Invisibility,
		SpT_Levitation,
		SpT_Lull,
		SpT_Mesmerize,
		SpT_MovementSpeed,
		SpT_Resistance,
		SpT_Resurrect,
		SpT_Rune,
		SpT_SendHome,
		SpT_Size,
		SpT_Stance,
		SpT_SummonCorpse,
		SpT_WaterBreathing
	} SpType;
	static const int SpellTypeFirst = SpT_BindAffinity;
	static const int SpellTypeLast = SpT_WaterBreathing;

	typedef enum TargetType {
		TT_None = 0,
		TT_Corpse,
		TT_Self,
		TT_Animal,
		TT_Undead,
		TT_Summoned,
		TT_Plant,
		TT_Single,
		TT_GroupV1,
		TT_GroupV2,
		TT_AECaster,
		TT_AEBard,
		TT_AETarget
	} TType;
	static const int TargetTypeFirst = TT_Corpse;
	static const int TargetTypeLast = TT_AETarget;
	static const int TargetTypeCount = 13;

	typedef enum TargetMask {
		TM_None = 0,
		TM_Corpse = 1,
		TM_Self = 2,
		TM_Animal = 4,
		TM_Undead = 8,
		TM_Summoned = 16,
		TM_Plant = 32,
		TM_Single = 124, // currently, 2^6 + 2^{2..5}) -or- (64+32+16+8+4)
		TM_GroupV1 = 128,
		TM_GroupV2 = 256,
		TM_AECaster = 512,
		TM_AEBard = 1024,
		TM_AETarget = 2048
	} TMask;

	typedef enum AppearanceFailType {
		AFT_None = 0,
		AFT_Value,
		AFT_GenderRace,
		AFT_Race
	} AFType;

	typedef enum AilmentType {
		AT_None = 0,
		AT_Blindness,	// SE: 20
		AT_Disease,		// SE: 35
		AT_Poison,		// SE: 36
		AT_Curse,		// SE: 116
		AT_Corruption	// SE: 369
	} AType;
	static const int AilmentTypeCount = 5;

	typedef enum InvisibilityType {
		IT_None = 0,
		IT_Animal,
		IT_Undead,
		IT_Living,
		IT_See
	} IType;

	typedef enum ResistanceType {
		RT_None = 0,
		RT_Fire,		// SE: 46
		RT_Cold,		// SE: 47
		RT_Poison,		// SE: 48
		RT_Disease,		// SE: 49
		RT_Magic,		// SE: 50
		RT_Corruption	// SE: 370
	} RType;
	static const int ResistanceTypeCount = 6;

	typedef enum SizeType {
		SzT_None = 0,
		SzT_Enlarge,
		SzT_Reduce
	} SzType;

	typedef enum StanceType {
		StT_None = 0,
		StT_Aggressive,
		StT_Defensive
	} StType;

	static std::string SpellTypeEnumToString(BCEnum::SpType spell_type) {
		switch (spell_type) {
		case SpT_BindAffinity:
			return "SpT_BindAffinity";
		case SpT_Charm:
			return "SpT_Charm";
		case SpT_Cure:
			return "SpT_Cure";
		case SpT_Depart:
			return "SpT_Depart";
		case SpT_Escape:
			return "SpT_Escape";
		case SpT_Identify:
			return "SpT_Identify";
		case SpT_Invisibility:
			return "SpT_Invisibility";
		case SpT_Levitation:
			return "SpT_Levitation";
		case SpT_Lull:
			return "SpT_Lull";
		case SpT_Mesmerize:
			return "SpT_Mesmerize";
		case SpT_MovementSpeed:
			return "SpT_MovementSpeed";
		case SpT_Resistance:
			return "SpT_Resistance";
		case SpT_Resurrect:
			return "SpT_Resurrect";
		case SpT_Rune:
			return "SpT_Rune";
		case SpT_SendHome:
			return "SpT_SendHome";
		case SpT_Size:
			return "SpT_Size";
		case SpT_Stance:
			return "SpT_Stance";
		case SpT_SummonCorpse:
			return "SpT_SummonCorpse";
		case SpT_WaterBreathing:
			return "SpT_WaterBreathing";
		default:
			return "SpT_None";
		}
	}

	static std::string TargetTypeEnumToString(BCEnum::TType target_type) {
		switch (target_type) {
		case TT_Self:
			return "TT_Self";
		case TT_Animal:
			return "TT_Animal";
		case TT_Undead:
			return "TT_Undead";
		case TT_Summoned:
			return "TT_Summoned";
		case TT_Plant:
			return "TT_Plant";
		case TT_Single:
			return "TT_Single";
		case TT_GroupV1:
			return "TT_GroupV1";
		case TT_GroupV2:
			return "TT_GroupV2";
		case TT_AECaster:
			return "TT_AECaster";
		case TT_AEBard:
			return "TT_AEBard";
		case TT_AETarget:
			return "TT_AETarget";
		case TT_Corpse:
			return "TT_Corpse";
		default:
			return "TT_None";
		}
	}
};


class STBaseEntry;
class STCharmEntry;
class STCureEntry;
class STDepartEntry;
class STEscapeEntry;
class STInvisibilityEntry;
class STMovementSpeedEntry;
class STResistanceEntry;
class STResurrectEntry;
class STSendHomeEntry;
class STSizeEntry;
class STStanceEntry;

class STBaseEntry
{
protected:
	BCEnum::SpType m_bcst;

public:
	int spell_id;
	uint8 spell_level;
	uint8 caster_class;
	BCEnum::TType target_type;

	// A non-polymorphic constructor requires an appropriate, non-'ST_None' BCEnum::SType
	STBaseEntry(BCEnum::SpType init_bcst = BCEnum::SpT_None) {
		spell_id = 0;
		spell_level = 255;
		caster_class = 255;
		target_type = BCEnum::TT_None;
		m_bcst = init_bcst;
	}
	STBaseEntry(STBaseEntry* prototype) {
		spell_id = prototype->spell_id;
		spell_level = 255;
		caster_class = 255;
		target_type = prototype->target_type;
		m_bcst = prototype->BCST();
	}
	virtual ~STBaseEntry() { return; };

	BCEnum::SpType BCST() { return m_bcst; }

	virtual bool IsDerived() { return false; }

	bool IsCharm() const { return (m_bcst == BCEnum::SpT_Charm); }
	bool IsCure() const { return (m_bcst == BCEnum::SpT_Cure); }
	bool IsDepart() const { return (m_bcst == BCEnum::SpT_Depart); }
	bool IsEscape() const { return (m_bcst == BCEnum::SpT_Escape); }
	bool IsInvisibility() const { return (m_bcst == BCEnum::SpT_Invisibility); }
	bool IsMovementSpeed() const { return (m_bcst == BCEnum::SpT_MovementSpeed); }
	bool IsResistance() const { return (m_bcst == BCEnum::SpT_Resistance); }
	bool IsResurrect() const { return (m_bcst == BCEnum::SpT_Resurrect); }
	bool IsSendHome() const { return (m_bcst == BCEnum::SpT_SendHome); }
	bool IsSize() const { return (m_bcst == BCEnum::SpT_Size); }
	bool IsStance() const { return (m_bcst == BCEnum::SpT_Stance); }

	virtual STCharmEntry* SafeCastToCharm() { return nullptr; }
	virtual STCureEntry* SafeCastToCure() { return nullptr; }
	virtual STDepartEntry* SafeCastToDepart() { return nullptr; }
	virtual STEscapeEntry* SafeCastToEscape() { return nullptr; }
	virtual STInvisibilityEntry* SafeCastToInvisibility() { return nullptr; }
	virtual STMovementSpeedEntry* SafeCastToMovementSpeed() { return nullptr; }
	virtual STResistanceEntry* SafeCastToResistance() { return nullptr; }
	virtual STResurrectEntry* SafeCastToResurrect() { return nullptr; }
	virtual STSendHomeEntry* SafeCastToSendHome() { return nullptr; }
	virtual STSizeEntry* SafeCastToSize() { return nullptr; }
	virtual STStanceEntry* SafeCastToStance() { return nullptr; }
};

class STCharmEntry : public STBaseEntry
{
public:
	bool dire;

	STCharmEntry() {
		m_bcst = BCEnum::SpT_Charm;
		dire = false;
	}
	STCharmEntry(STCharmEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Charm;
		dire = prototype->dire;
	}
	virtual ~STCharmEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STCharmEntry* SafeCastToCharm() { return ((m_bcst == BCEnum::SpT_Charm) ? (static_cast<STCharmEntry*>(this)) : (nullptr)); }
};

class STCureEntry : public STBaseEntry
{
public:
	int cure_value[BCEnum::AilmentTypeCount];
	int cure_total;

	STCureEntry() {
		m_bcst = BCEnum::SpT_Cure;
		memset(&cure_value, 0, (sizeof(int) * BCEnum::AilmentTypeCount));
		cure_total = 0;
	}
	STCureEntry(STCureEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Cure;
		memcpy(&cure_value, prototype->cure_value, (sizeof(int) * BCEnum::AilmentTypeCount));
		cure_total = prototype->cure_total;
	}
	virtual ~STCureEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STCureEntry* SafeCastToCure() { return ((m_bcst == BCEnum::SpT_Cure) ? (static_cast<STCureEntry*>(this)) : (nullptr)); }
};

class STDepartEntry : public STBaseEntry
{
public:
	bool single;
	std::string long_name;

	STDepartEntry() {
		m_bcst = BCEnum::SpT_Depart;
		single = false;
		long_name.clear();
	}
	STDepartEntry(STDepartEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Depart;
		single = prototype->single;
		long_name = prototype->long_name;
	}
	virtual ~STDepartEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STDepartEntry* SafeCastToDepart() { return ((m_bcst == BCEnum::SpT_Depart) ? (static_cast<STDepartEntry*>(this)) : (nullptr)); }
};

class STEscapeEntry : public STBaseEntry
{
public:
	bool lesser;

	STEscapeEntry() {
		m_bcst = BCEnum::SpT_Escape;
		lesser = false;
	}
	STEscapeEntry(STEscapeEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Escape;
		lesser = prototype->lesser;
	}
	virtual ~STEscapeEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STEscapeEntry* SafeCastToEscape() { return ((m_bcst == BCEnum::SpT_Escape) ? (static_cast<STEscapeEntry*>(this)) : (nullptr)); }
};

class STInvisibilityEntry : public STBaseEntry
{
public:
	BCEnum::IType invis_type;

	STInvisibilityEntry() {
		m_bcst = BCEnum::SpT_Invisibility;
		invis_type = BCEnum::IT_None;
	}
	STInvisibilityEntry(STInvisibilityEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Invisibility;
		invis_type = prototype->invis_type;
	}
	virtual ~STInvisibilityEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STInvisibilityEntry* SafeCastToInvisibility() { return ((m_bcst == BCEnum::SpT_Invisibility) ? (static_cast<STInvisibilityEntry*>(this)) : (nullptr)); }
};

class STMovementSpeedEntry : public STBaseEntry
{
public:
	bool group;

	STMovementSpeedEntry() {
		m_bcst = BCEnum::SpT_MovementSpeed;
		group = false;
	}
	STMovementSpeedEntry(STMovementSpeedEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_MovementSpeed;
		group = prototype->group;
	}
	virtual ~STMovementSpeedEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STMovementSpeedEntry* SafeCastToMovementSpeed() { return ((m_bcst == BCEnum::SpT_MovementSpeed) ? (static_cast<STMovementSpeedEntry*>(this)) : (nullptr)); }
};

class STResistanceEntry : public STBaseEntry
{
public:
	int resist_value[BCEnum::ResistanceTypeCount];
	int resist_total;

	STResistanceEntry() {
		m_bcst = BCEnum::SpT_Resistance;
		memset(&resist_value, 0, (sizeof(int) * BCEnum::ResistanceTypeCount));
		resist_total = 0;
	}
	STResistanceEntry(STResistanceEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Resistance;
		memcpy(&resist_value, prototype->resist_value, (sizeof(int) * BCEnum::ResistanceTypeCount));
		resist_total = prototype->resist_total;
	}
	virtual ~STResistanceEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STResistanceEntry* SafeCastToResistance() { return ((m_bcst == BCEnum::SpT_Resistance) ? (static_cast<STResistanceEntry*>(this)) : (nullptr)); }
};

class STResurrectEntry : public STBaseEntry
{
public:
	bool aoe;

	STResurrectEntry() {
		m_bcst = BCEnum::SpT_Resurrect;
		aoe = false;
	}
	STResurrectEntry(STResurrectEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Resurrect;
		aoe = prototype->aoe;
	}
	virtual ~STResurrectEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STResurrectEntry* SafeCastToResurrect() { return ((m_bcst == BCEnum::SpT_Resurrect) ? (static_cast<STResurrectEntry*>(this)) : (nullptr)); }
};

class STSendHomeEntry : public STBaseEntry
{
public:
	bool group;

	STSendHomeEntry() {
		m_bcst = BCEnum::SpT_SendHome;
		group = false;
	}
	STSendHomeEntry(STSendHomeEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_SendHome;
		group = prototype->group;
	}
	virtual ~STSendHomeEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STSendHomeEntry* SafeCastToSendHome() { return ((m_bcst == BCEnum::SpT_SendHome) ? (static_cast<STSendHomeEntry*>(this)) : (nullptr)); }
};

class STSizeEntry : public STBaseEntry
{
public:
	BCEnum::SzType size_type;

	STSizeEntry() {
		m_bcst = BCEnum::SpT_Size;
		size_type = BCEnum::SzT_None;
	}
	STSizeEntry(STSizeEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Size;
		size_type = prototype->size_type;
	}
	virtual ~STSizeEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STSizeEntry* SafeCastToSize() { return ((m_bcst == BCEnum::SpT_Size) ? (static_cast<STSizeEntry*>(this)) : (nullptr)); }
};

class STStanceEntry : public STBaseEntry {
public:
	BCEnum::StType stance_type;

	STStanceEntry() {
		m_bcst = BCEnum::SpT_Stance;
		stance_type = BCEnum::StT_None;
	}
	STStanceEntry(STStanceEntry* prototype) : STBaseEntry(prototype) {
		m_bcst = BCEnum::SpT_Stance;
		stance_type = prototype->stance_type;
	}
	virtual ~STStanceEntry() { return; };

	virtual bool IsDerived() { return true; }

	virtual STStanceEntry* SafeCastToStance() { return ((m_bcst == BCEnum::SpT_Stance) ? (static_cast<STStanceEntry*>(this)) : (nullptr)); }
};


typedef std::list<STBaseEntry*> bcst_list;
typedef std::map<BCEnum::SpType, bcst_list> bcst_map;

typedef std::map<BCEnum::SpType, std::string> bcst_required_bot_classes_map;
typedef std::map<BCEnum::SpType, std::map<uint8, std::string>> bcst_required_bot_classes_map_by_class;

typedef std::map<uint8, uint8> bcst_levels;
typedef std::map<BCEnum::SpType, bcst_levels> bcst_levels_map;


#define	BOT_COMMAND_CHAR '^'

typedef void (*BotCmdFuncPtr)(Client *,const Seperator *);

typedef struct {
	int access;
	const char *desc;			// description of bot command
	BotCmdFuncPtr function;		// null means perl function
} BotCommandRecord;

extern int (*bot_command_dispatch)(Client *,char const*);
extern int bot_command_count;	// number of bot commands loaded


// the bot command system:
int bot_command_init(void);
void bot_command_deinit(void);
int bot_command_add(std::string bot_command_name, const char *desc, int access, BotCmdFuncPtr function);
int bot_command_not_avail(Client *c, const char *message);
int bot_command_real_dispatch(Client *c, char const *message);
void bot_command_log_command(Client *c, const char *message);


// bot commands
void bot_command_actionable(Client *c, const Seperator *sep);
void bot_command_aggressive(Client *c, const Seperator *sep);
void bot_command_attack(Client *c, const Seperator *sep);
void bot_command_bind_affinity(Client *c, const Seperator *sep);
void bot_command_bot(Client *c, const Seperator *sep);
void bot_command_botgroup(Client *c, const Seperator *sep);
void bot_command_charm(Client *c, const Seperator *sep);
void bot_command_cure(Client *c, const Seperator *sep);
void bot_command_defensive(Client *c, const Seperator *sep);
void bot_command_depart(Client *c, const Seperator *sep);
void bot_command_escape(Client *c, const Seperator *sep);
void bot_command_find_aliases(Client *c, const Seperator *sep);
void bot_command_follow(Client *c, const Seperator *sep);
void bot_command_guard(Client *c, const Seperator *sep);
void bot_command_heal_rotation(Client *c, const Seperator *sep);
void bot_command_help(Client *c, const Seperator *sep);
void bot_command_hold(Client *c, const Seperator *sep);
void bot_command_identify(Client *c, const Seperator *sep);
void bot_command_inventory(Client *c, const Seperator *sep);
void bot_command_invisibility(Client *c, const Seperator *sep);
void bot_command_levitation(Client *c, const Seperator *sep);
void bot_command_lull(Client *c, const Seperator *sep);
void bot_command_mesmerize(Client *c, const Seperator *sep);
void bot_command_movement_speed(Client *c, const Seperator *sep);
void bot_command_pet(Client *c, const Seperator *sep);
void bot_command_pick_lock(Client *c, const Seperator *sep);
void bot_command_pull(Client *c, const Seperator *sep);
void bot_command_release(Client *c, const Seperator *sep);
void bot_command_resistance(Client *c, const Seperator *sep);
void bot_command_resurrect(Client *c, const Seperator *sep);
void bot_command_rune(Client *c, const Seperator *sep);
void bot_command_send_home(Client *c, const Seperator *sep);
void bot_command_size(Client *c, const Seperator *sep);
void bot_command_summon_corpse(Client *c, const Seperator *sep);
void bot_command_taunt(Client *c, const Seperator *sep);
void bot_command_track(Client *c, const Seperator *sep);
void bot_command_water_breathing(Client *c, const Seperator *sep);


// bot subcommands
void bot_subcommand_bot_appearance(Client *c, const Seperator *sep);
void bot_subcommand_bot_beard_color(Client *c, const Seperator *sep);
void bot_subcommand_bot_beard_style(Client *c, const Seperator *sep);
void bot_subcommand_bot_camp(Client *c, const Seperator *sep);
void bot_subcommand_bot_clone(Client *c, const Seperator *sep);
void bot_subcommand_bot_create(Client *c, const Seperator *sep);
void bot_subcommand_bot_delete(Client *c, const Seperator *sep);
void bot_subcommand_bot_details(Client *c, const Seperator *sep);
void bot_subcommand_bot_dye_armor(Client *c, const Seperator *sep);
void bot_subcommand_bot_eyes(Client *c, const Seperator *sep);
void bot_subcommand_bot_face(Client *c, const Seperator *sep);
void bot_subcommand_bot_follow_distance(Client *c, const Seperator *sep);
void bot_subcommand_bot_hair_color(Client *c, const Seperator *sep);
void bot_subcommand_bot_hairstyle(Client *c, const Seperator *sep);
void bot_subcommand_bot_heritage(Client *c, const Seperator *sep);
void bot_subcommand_bot_inspect_message(Client *c, const Seperator *sep);
void bot_subcommand_bot_list(Client *c, const Seperator *sep);
void bot_subcommand_bot_out_of_combat(Client *c, const Seperator *sep);
void bot_subcommand_bot_report(Client *c, const Seperator *sep);
void bot_subcommand_bot_spawn(Client *c, const Seperator *sep);
void bot_subcommand_bot_stance(Client *c, const Seperator *sep);
void bot_subcommand_bot_summon(Client *c, const Seperator *sep);
void bot_subcommand_bot_tattoo(Client *c, const Seperator *sep);
void bot_subcommand_bot_toggle_archer(Client *c, const Seperator *sep);
void bot_subcommand_bot_toggle_helm(Client *c, const Seperator *sep);
void bot_subcommand_bot_update(Client *c, const Seperator *sep);
void bot_subcommand_bot_woad(Client *c, const Seperator *sep);
void bot_subcommand_botgroup_add_member(Client *c, const Seperator *sep);
void bot_subcommand_botgroup_create(Client *c, const Seperator *sep);
void bot_subcommand_botgroup_delete(Client *c, const Seperator *sep);
void bot_subcommand_botgroup_list(Client *c, const Seperator *sep);
void bot_subcommand_botgroup_load(Client *c, const Seperator *sep);
void bot_subcommand_botgroup_remove_member(Client *c, const Seperator *sep);
void bot_subcommand_circle(Client *c, const Seperator *sep);
void bot_subcommand_evacuate(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_adaptive_targeting(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_add_member(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_add_target(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_adjust_critical(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_adjust_safe(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_casting_override(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_change_interval(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_clear_targets(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_create(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_fast_heals(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_list(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_remove_member(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_remove_target(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_reset_limits(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_start(Client *c, const Seperator *sep);
void bot_subcommand_heal_rotation_stop(Client *c, const Seperator *sep);
void bot_subcommand_inventory_give(Client *c, const Seperator *sep);
void bot_subcommand_inventory_list(Client *c, const Seperator *sep);
void bot_subcommand_inventory_remove(Client *c, const Seperator *sep);
void bot_subcommand_pet_remove(Client *c, const Seperator *sep);
void bot_subcommand_pet_set_type(Client *c, const Seperator *sep);
void bot_subcommand_portal(Client *c, const Seperator *sep);
void bot_subcommand_succor(Client *c, const Seperator *sep);


// bot command helpers
bool helper_bot_appearance_fail(Client *bot_owner, Bot *my_bot, BCEnum::AFType fail_type, const char* type_desc);
void helper_bot_appearance_form_final(Client *bot_owner, Bot *my_bot);
void helper_bot_appearance_form_update(Bot *my_bot);
uint32 helper_bot_create(Client *bot_owner, std::string bot_name, uint8 bot_class, uint16 bot_race, uint8 bot_gender);
void helper_bot_out_of_combat(Client *bot_owner, Bot *my_bot);
bool helper_cast_standard_spell(Bot* casting_bot, Mob* target_mob, int spell_id, bool annouce_cast = true, uint32* dont_root_before = nullptr);
bool helper_command_alias_fail(Client *bot_owner, const char* command_handler, const char *alias, const char *command);
void helper_command_depart_list(Client* bot_owner, Bot* druid_bot, Bot* wizard_bot, bcst_list* local_list, bool single_flag = false);
bool helper_is_help_or_usage(const char* arg);
bool helper_no_available_bots(Client *bot_owner, Bot *my_bot = nullptr);
void helper_send_available_subcommands(Client *bot_owner, const char* command_simile, const std::list<const char*>& subcommand_list);
void helper_send_usage_required_bots(Client *bot_owner, BCEnum::SpType spell_type, uint8 bot_class = 0);
bool helper_spell_check_fail(STBaseEntry* local_entry);
bool helper_spell_list_fail(Client *bot_owner, bcst_list* spell_list, BCEnum::SpType spell_type);
#endif

#endif // BOTS
