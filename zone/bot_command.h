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

class Client;
class Seperator;

#include "../common/types.h"
#include "bot.h"
#include "dialogue_window.h"

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

	typedef enum InvisType {
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

namespace
{
#define HP_RATIO_DELTA 5.0f

	enum { EffectIDFirst = 1, EffectIDLast = 12 };

#define VALIDATECLASSID(x) ((x >= Class::Warrior && x <= Class::Berserker) ? (x) : (0))
#define CLASSIDTOINDEX(x) ((x >= Class::Warrior && x <= Class::Berserker) ? (x - 1) : (0))
#define EFFECTIDTOINDEX(x) ((x >= EffectIDFirst && x <= EffectIDLast) ? (x - 1) : (0))
#define AILMENTIDTOINDEX(x) ((x >= BCEnum::AT_Blindness && x <= BCEnum::AT_Corruption) ? (x - 1) : (0))
#define RESISTANCEIDTOINDEX(x) ((x >= BCEnum::RT_Fire && x <= BCEnum::RT_Corruption) ? (x - 1) : (0))

	// ActionableTarget action_type
#define FRIENDLY true
#define ENEMY false
}

namespace MyBots
{
	static bool IsMyBot(Client *bot_owner, Mob *my_bot) {
		if (!bot_owner || !my_bot || !my_bot->IsBot()) {
			return false;
		}

		auto test_bot = my_bot->CastToBot();
		if (!test_bot->GetOwner() || !test_bot->GetOwner()->IsClient() || test_bot->GetOwner()->CastToClient() != bot_owner) {
			return false;
		}

		return true;
	}

	static bool IsMyBotInTargetsGroup(Client *bot_owner, Mob *grouped_bot) {
		if (!bot_owner || !grouped_bot || !grouped_bot->GetGroup() || !IsMyBot(bot_owner, grouped_bot)) {
			return false;
		}

		auto target_mob = bot_owner->GetTarget();
		if (!target_mob) {
			return false;
		}

		if (!target_mob->GetGroup() || (!target_mob->IsClient() && !target_mob->IsBot())) {
			return false;
		}

		return (grouped_bot->GetGroup() == target_mob->GetGroup());
	}

	static bool IsMyBotInPlayerGroup(Client *bot_owner, Mob *grouped_bot, Client *grouped_player) {
		if (!bot_owner || !grouped_player || !grouped_player->GetGroup() || !grouped_bot || !grouped_bot->GetGroup() || !IsMyBot(bot_owner, grouped_bot)) {
			return false;
		}

		return (grouped_player->GetGroup() == grouped_bot->GetGroup());
	}

	static void UniquifySBL(std::list<Bot*> &sbl) {
		sbl.remove(nullptr);
		sbl.sort();
		sbl.unique();
	}

	static void PopulateSBL_ByTargetedBot(Client *bot_owner, std::list<Bot*> &sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (IsMyBot(bot_owner, bot_owner->GetTarget())) {
			sbl.push_back(bot_owner->GetTarget()->CastToBot());
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByNamedBot(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !name) {
			return;
		}

		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (!strcasecmp(bot_iter->GetCleanName(), name)) {
				sbl.push_back(bot_iter);
				return;
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByMyGroupedBots(Client *bot_owner, std::list<Bot*> &sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return;
		}

		if (!bot_owner->GetGroup() && !bot_owner->GetRaid()) {
			return;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_owner->GetRaid();
			if (!raid) {
				return;
			}

			uint32 raid_group = raid->GetGroup(bot_owner);
			if (raid_group >= MAX_RAID_GROUPS) {
				return;
			}

			for (const auto& m : raid->members) {
				if (m.member && m.group_number == raid_group) {
					if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
						continue;
					}

					sbl.push_back(m.member->CastToBot());
				}
			}
		}
		else {
			std::list<Bot*> group_list;
			bot_owner->GetGroup()->GetBotList(group_list);
			for (auto member_iter : group_list) {
				if (IsMyBot(bot_owner, member_iter)) {
					sbl.push_back(member_iter);
				}
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByMyRaidBots(Client* bot_owner, std::list<Bot*>& sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return;
		}

		if (!bot_owner->GetRaid()) {
			return;
		}

		Raid* raid = bot_owner->GetRaid();
		if (!raid) {
			return;
		}

		for (const auto& m : raid->members) {
			if (m.member) {
				if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
					continue;
				}

				sbl.push_back(m.member->CastToBot());
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByTargetsGroupedBots(Client *bot_owner, std::list<Bot*> &sbl, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return;
		}

		auto target_mob = bot_owner->GetTarget();
		if (!target_mob || (!target_mob->GetGroup() && !target_mob->GetRaid()) || (!target_mob->IsClient() && !target_mob->IsBot())) {
			return;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_owner->GetRaid();
			if (!raid) {
				return;
			}

			if (MyBots::IsMyBot(bot_owner, target_mob)) {
				uint32 raid_group = raid->GetGroup(target_mob->CastToClient());
				if (raid_group >= MAX_RAID_GROUPS) {
					return;
				}

				for (const auto& m : raid->members) {
					if (m.member && m.group_number == raid_group) {
						if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
							continue;
						}

						sbl.push_back(m.member->CastToBot());
					}
				}
			}
		}
		else {
			std::list<Bot*> group_list;
			bot_owner->GetGroup()->GetBotList(group_list);
			for (auto member_iter : group_list) {
				if (IsMyBot(bot_owner, member_iter)) {
					sbl.push_back(member_iter);
				}
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByNamesGroupedBots(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !name) {
			return;
		}

		Mob* named_mob = nullptr;
		std::list<Mob*> l_mob_list;
		entity_list.GetMobList(l_mob_list);
		for (auto mob_iter : l_mob_list) {
			if (!strcasecmp(mob_iter->GetCleanName(), name)) {
				named_mob = mob_iter;
				break;
			}
		}

		if (!named_mob || !MyBots::IsMyBot(bot_owner, named_mob) || (!named_mob->GetGroup() && !named_mob->GetRaid()) || (!named_mob->IsClient() && !named_mob->IsBot())) {
			return;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_owner->GetRaid();
			if (!raid) {
				return;
			}

			uint32 raid_group = raid->GetGroup(named_mob->CastToClient());
			if (raid_group >= MAX_RAID_GROUPS) {
				return;
			}

			for (const auto& m : raid->members) {
				if (m.member && m.group_number == raid_group) {
					if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
						continue;
					}

					sbl.push_back(m.member->CastToBot());
				}
			}
		}
		else {
			std::list<Bot*> group_list;
			bot_owner->GetGroup()->GetBotList(group_list);
			for (auto member_iter : group_list) {
				if (IsMyBot(bot_owner, member_iter)) {
					sbl.push_back(member_iter);
				}
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByHealRotation(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || (!name && !bot_owner->GetTarget())) {
			return;
		}

		std::list<Bot*> selectable_bot_list;
		if (name) {
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		}
		else {
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);
		}

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember()) {
			return;
		}

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->MemberList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter)) {
				sbl.push_back(hrm_iter);
			}
		}

		auto hrt = (*selectable_bot_list.front()->MemberOfHealRotation())->TargetList();
		for (auto hrt_iter : *hrt) {
			if (IsMyBot(bot_owner, hrt_iter)) {
				sbl.push_back(hrt_iter->CastToBot());
			}
		}

		UniquifySBL(sbl);
	}

	static void PopulateSBL_ByHealRotationMembers(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || (!name && !bot_owner->GetTarget())) {
			return;
		}

		std::list<Bot*> selectable_bot_list;
		if (name) {
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		}
		else {
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);
		}

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember()) {
			return;
		}

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->MemberList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter)) {
				sbl.push_back(hrm_iter);
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_ByHealRotationTargets(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || (!name && !bot_owner->GetTarget())) {
			return;
		}

		std::list<Bot*> selectable_bot_list;
		if (name) {
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		}
		else {
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);
		}

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember()) {
			return;
		}

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->TargetList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter)) {
				sbl.push_back(static_cast<Bot*>(hrm_iter));
			}
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_BySpawnedBots(Client *bot_owner, std::list<Bot*> &sbl) { // should be used for most spell casting commands
		sbl.clear();
		if (!bot_owner) {
			return;
		}

		sbl = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		sbl.remove(nullptr);
	}

	static void PopulateSBL_BySpawnedBotsClass(Client * bot_owner, std::list<Bot*> &sbl, uint16 cls, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !cls) {
			return;
		}

		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (bot_iter->GetClass() != cls) {
				continue;
			}

			sbl.push_back(bot_iter);
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}

	static void PopulateSBL_BySpawnedBotsRace(Client* bot_owner, std::list<Bot*>& sbl, uint16 race, bool clear_list = true) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner || !race) {
			return;
		}

		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (bot_iter->GetBaseRace() != race) {
				continue;
			}

			sbl.push_back(bot_iter);
		}

		if (!clear_list) {
			UniquifySBL(sbl);
		}
	}
};

namespace ActionableTarget
{
	static bool AmIInPlayerGroup(Client *bot_owner, Client *grouped_player) {
		if (!bot_owner || !grouped_player || !bot_owner->GetGroup() || !grouped_player->GetGroup())
			return false;

		return (bot_owner->GetGroup() == grouped_player->GetGroup());
	}

	static bool IsAttackable(Client *bot_owner, Mob* target_mob) {
		if (!bot_owner || !target_mob || bot_owner == target_mob)
			return false;

		return bot_owner->IsAttackAllowed(target_mob);
	}

	static Client* AsSingle_ByPlayer(Client *bot_owner, bool return_me_on_null_target = true) {
		if (!bot_owner)
			return nullptr;

		if (!bot_owner->GetTarget()) {
			if (return_me_on_null_target)
				return bot_owner;
			else
				return nullptr;
		}

		if (!bot_owner->GetTarget()->IsClient())
			return nullptr;

		return bot_owner->GetTarget()->CastToClient();
	}

	static Client* AsGroupMember_ByPlayer(Client *bot_owner, bool return_me_on_null_target = true) {
		if (!bot_owner)
			return nullptr;

		if (!bot_owner->GetTarget()) {
			if (return_me_on_null_target)
				return bot_owner;
			else
				return nullptr;
		}

		if (!bot_owner->GetTarget()->IsClient() || !AmIInPlayerGroup(bot_owner, bot_owner->GetTarget()->CastToClient()))
			return nullptr;

		return bot_owner->GetTarget()->CastToClient();
	}

	static Corpse* AsCorpse_ByPlayer(Client *bot_owner) {
		if (!bot_owner || !bot_owner->GetTarget() || !bot_owner->GetTarget()->IsPlayerCorpse())
			return nullptr;

		return bot_owner->GetTarget()->CastToCorpse();
	}

	static Mob* AsSingle_ByAttackable(Client *bot_owner) {
		if (!IsAttackable(bot_owner, bot_owner->GetTarget()))
			return nullptr;

		return bot_owner->GetTarget();
	}

	static bool IsFriendlyAllowed(Mob* target_mob) {
		if (!target_mob || target_mob->IsClient() || target_mob->IsBot() || (target_mob->IsPet() && target_mob->GetOwner() && (target_mob->GetOwner()->IsClient() || target_mob->GetOwner()->IsBot())) || target_mob->IsPlayerCorpse())
			return true;

		return false;
	}

	static Mob* VerifyFriendly(Client* bot_owner, BCEnum::TType target_type, bool return_me_on_null_target = true) {
		if (IsAttackable(bot_owner, bot_owner->GetTarget()) || target_type == BCEnum::TT_None)
			return nullptr;

		auto target_mob = bot_owner->GetTarget();
		Mob* verified_friendly = nullptr;
		switch (target_type) {
			case BCEnum::TT_Single:
			case BCEnum::TT_GroupV1:
			case BCEnum::TT_GroupV2:
			case BCEnum::TT_AETarget:
				verified_friendly = target_mob;
				break;
			case BCEnum::TT_Animal:
				if (target_mob && target_mob->GetBodyType() == BodyType::Animal)
					verified_friendly = target_mob;
				break;
			case BCEnum::TT_Undead:
				if (target_mob && target_mob->GetBodyType() == BodyType::Undead)
					verified_friendly = target_mob;
				break;
			case BCEnum::TT_Summoned:
				if (target_mob && target_mob->GetBodyType() == BodyType::Summoned)
					verified_friendly = target_mob;
				break;
			case BCEnum::TT_Plant:
				if (target_mob && target_mob->GetBodyType() == BodyType::Plant)
					verified_friendly = target_mob;
				break;
			case BCEnum::TT_Corpse:
				if (target_mob && target_mob->IsCorpse())
					verified_friendly = target_mob;
				break;
			default:
				return nullptr;
		}

		if (return_me_on_null_target && !target_mob && !verified_friendly) {
			switch (target_type) {
				case BCEnum::TT_Single:
				case BCEnum::TT_GroupV1:
				case BCEnum::TT_GroupV2:
				case BCEnum::TT_AETarget:
					verified_friendly = bot_owner;
					break;
				default:
					break;
			}
		}

		return verified_friendly;
	}

	static Mob* VerifyEnemy(Client* bot_owner, BCEnum::TType target_type) {
		if (!IsAttackable(bot_owner, bot_owner->GetTarget()) || target_type == BCEnum::TT_None)
			return nullptr;

		auto target_mob = bot_owner->GetTarget();
		Mob* verified_enemy = nullptr;
		switch (target_type) {
			case BCEnum::TT_Animal:
				if (target_mob->GetBodyType() == BodyType::Animal)
					verified_enemy = target_mob;
				break;
			case BCEnum::TT_Undead:
				if (target_mob->GetBodyType() == BodyType::Undead)
					verified_enemy = target_mob;
				break;
			case BCEnum::TT_Summoned:
				if (target_mob->GetBodyType() == BodyType::Summoned)
					verified_enemy = target_mob;
				break;
			case BCEnum::TT_Plant:
				if (target_mob->GetBodyType() == BodyType::Plant)
					verified_enemy = target_mob;
				break;
			case BCEnum::TT_Single:
			case BCEnum::TT_GroupV1:
			case BCEnum::TT_GroupV2:
			case BCEnum::TT_AETarget:
				verified_enemy = target_mob;
				break;
			case BCEnum::TT_Corpse:
				if (target_mob->IsCorpse())
					verified_enemy = target_mob;
				break;
			default:
				return nullptr;
		}

		return verified_enemy;
	}

	class Types {
		Mob* target[BCEnum::TargetTypeCount];
		bool target_set[BCEnum::TargetTypeCount];

	public:
		Types() { Clear(); }

		void Clear() {
			for (int i = BCEnum::TT_None; i <= BCEnum::TargetTypeLast; ++i) {
				target[i] = nullptr;
				target_set[i] = false;
			}
			target_set[BCEnum::TT_None] = true;
		}

		Mob* Select(Client* bot_owner, BCEnum::TType target_type, bool action_type, bool return_me_on_null_target = true) {
			if (target_set[target_type])
				return target[target_type];

			if (action_type == FRIENDLY)
				target[target_type] = VerifyFriendly(bot_owner, target_type, return_me_on_null_target);
			else
				target[target_type] = VerifyEnemy(bot_owner, target_type);
			target_set[target_type] = true;

			return target[target_type];
		}
	};
}

namespace ActionableBots
{
	enum ABType {
		ABT_None = 0,
		ABT_Target,
		ABT_ByName,
		ABT_OwnerGroup,
		ABT_OwnerRaid,
		ABT_TargetGroup,
		ABT_NamesGroup,
		ABT_HealRotation,
		ABT_HealRotationMembers,
		ABT_HealRotationTargets,
		ABT_Class,
		ABT_Race,
		ABT_Spawned,
		ABT_All
	};

	enum ABMask {
		ABM_None = 0,
		ABM_Target = (1 << (ABT_Target - 1)),
		ABM_ByName = (1 << (ABT_ByName - 1)),
		ABM_OwnerGroup = (1 << (ABT_OwnerGroup - 1)),
		ABM_OwnerRaid = (1 << (ABT_OwnerRaid - 1)),
		ABM_TargetGroup = (1 << (ABT_TargetGroup - 1)),
		ABM_NamesGroup = (1 << (ABT_NamesGroup - 1)),
		ABM_HealRotation = (1 << (ABT_HealRotation - 1)),
		ABM_HealRotationMembers = (1 << (ABT_HealRotationMembers - 1)),
		ABM_HealRotationTargets = (1 << (ABT_HealRotationTargets - 1)),
		ABM_Class = (1 << (ABT_Class - 1)),
		ABM_Race = (1 << (ABT_Race - 1)),
		ABM_Spawned = (1 << (ABT_Spawned - 1)),
		ABM_All = (1 << (ABT_All - 1)),
		ABM_Spawned_All = (3 << (ABT_Spawned - 1)),
		ABM_NoFilter = ~0,
		// grouped values
		ABM_Type1 = (ABM_Target | ABM_ByName | ABM_OwnerGroup | ABM_OwnerRaid | ABM_TargetGroup | ABM_NamesGroup | ABM_HealRotationTargets | ABM_Spawned | ABM_Class | ABM_Race),
		ABM_Type2 = (ABM_ByName | ABM_OwnerGroup | ABM_OwnerRaid | ABM_NamesGroup | ABM_HealRotation | ABM_Spawned | ABM_Class | ABM_Race)
	};

	// Populates 'sbl'
	static ABType PopulateSBL(Client* bot_owner, std::string ab_type_arg, std::list<Bot*> &sbl, int ab_mask, const char* name = nullptr, uint16 classrace = 0, bool clear_list = true, bool suppress_message = false) {
		if (clear_list) {
			sbl.clear();
		}

		if (!bot_owner) {
			return ABT_None;
		}

		auto ab_type = ABT_None;
		if (!ab_type_arg.compare("target") || ab_type_arg.empty()) {
			ab_type = ABT_Target;
		}
		else if (!ab_type_arg.compare("byname")) {
			ab_type = ABT_ByName;
		}
		else if (!ab_type_arg.compare("ownergroup")) {
			ab_type = ABT_OwnerGroup;
		}
		else if (!ab_type_arg.compare("ownerraid")) {
			ab_type = ABT_OwnerRaid;
		}
		else if (!ab_type_arg.compare("targetgroup")) {
			ab_type = ABT_TargetGroup;
		}
		else if (!ab_type_arg.compare("namesgroup")) {
			ab_type = ABT_NamesGroup;
		}
		else if (!ab_type_arg.compare("healrotation")) {
			ab_type = ABT_HealRotation;
		}
		else if (!ab_type_arg.compare("healrotationmembers")) {
			ab_type = ABT_HealRotationMembers;
		}
		else if (!ab_type_arg.compare("healrotationtargets")) {
			ab_type = ABT_HealRotationTargets;
		}
		else if (!ab_type_arg.compare("byclass")) {
			ab_type = ABT_Class;
		}
		else if (!ab_type_arg.compare("byrace")) {
			ab_type = ABT_Race;
		}
		else if (!ab_type_arg.compare("spawned")) {
			ab_type = ABT_Spawned;
		}
		else if (!ab_type_arg.compare("all")) {
			ab_type = ABT_All;
		}

		if (ab_type_arg.empty()) {
			ab_type_arg = "target";
		}

		switch (ab_type) {
			case ABT_Target:
				if (ab_mask & ABM_Target) {
					MyBots::PopulateSBL_ByTargetedBot(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_ByName:
				if (ab_mask & ABM_ByName) {
					MyBots::PopulateSBL_ByNamedBot(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_OwnerGroup:
				if (ab_mask & ABM_OwnerGroup) {
					MyBots::PopulateSBL_ByMyGroupedBots(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_OwnerRaid:
				if (ab_mask & ABM_OwnerRaid) {
					MyBots::PopulateSBL_ByMyRaidBots(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_TargetGroup:
				if (ab_mask & ABM_TargetGroup) {
					MyBots::PopulateSBL_ByTargetsGroupedBots(bot_owner, sbl, clear_list);
				}
				break;
			case ABT_NamesGroup:
				if (ab_mask & ABM_NamesGroup) {
					MyBots::PopulateSBL_ByNamesGroupedBots(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_HealRotation:
				if (ab_mask & ABM_HealRotation) {
					MyBots::PopulateSBL_ByHealRotation(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_HealRotationMembers:
				if (ab_mask & ABM_HealRotationMembers) {
					MyBots::PopulateSBL_ByHealRotationMembers(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_HealRotationTargets:
				if (ab_mask & ABM_HealRotationTargets) {
					MyBots::PopulateSBL_ByHealRotationTargets(bot_owner, sbl, name, clear_list);
				}
				break;
			case ABT_Class:
				if (ab_mask & ABM_Class) {
					MyBots::PopulateSBL_BySpawnedBotsClass(bot_owner, sbl, classrace, clear_list);
				}
				break;
			case ABT_Race:
				if (ab_mask & ABM_Race) {
					MyBots::PopulateSBL_BySpawnedBotsRace(bot_owner, sbl, classrace, clear_list);
				}
				break;
			case ABT_Spawned:
			case ABT_All:
				if (ab_mask & ABM_Spawned_All) {
					MyBots::PopulateSBL_BySpawnedBots(bot_owner, sbl);
				}
				break;
			default:
				break;
		}
		if (sbl.empty() && ab_type != ABT_All) {
			if (suppress_message) {
				return ABT_None;
			}

			if (!ab_mask) {
				bot_owner->Message(Chat::White, "Command passed null 'ActionableBot' criteria");
			}
			else if (ab_mask & ab_type) {
				if (classrace) {
					bot_owner->Message(Chat::White, "You have no spawned bots meeting this criteria - type: '%s', %s: '%i'", ab_type_arg.c_str(), ab_mask & ABM_Class ? "class" : ab_mask & ABM_Race ? "race" : "", classrace);
				}
				else {
					bot_owner->Message(Chat::White, "You have no spawned bots meeting this criteria - type: '%s', name: '%s'", ab_type_arg.c_str(), ((name) ? (name) : ("")));
				}
			}
			else {
				bot_owner->Message(Chat::White, "This command does not allow 'ActionableBot' criteria '%s'", ab_type_arg.c_str());
			}

			return ABT_None;
		}

		return ab_type;
	}

	static Bot* AsGroupMember_ByClass(Client *bot_owner, Client *bot_grouped_player, uint8 cls, bool petless = false) {
		if (!bot_owner || !bot_grouped_player) {
			return nullptr;
		}

		if (!bot_grouped_player->GetGroup() && !bot_grouped_player->GetRaid()) {
			return nullptr;
		}

		if (bot_owner->IsRaidGrouped()) {
			Raid* raid = bot_grouped_player->GetRaid();
			if (!raid) {
				return nullptr;
			}

			uint32 raid_group = raid->GetGroup(bot_grouped_player);
			if (raid_group >= MAX_RAID_GROUPS) {
				return nullptr;
			}

			for (const auto& m : raid->members) {
				if (m.member && m.group_number == raid_group) {
					if (!MyBots::IsMyBot(bot_owner, m.member->CastToBot())) {
						continue;
					}

					if (m.member->GetClass() != cls) {
						continue;
					}

					if (petless && m.member->GetPet()) {
						continue;
					}

					return static_cast<Bot*>(m.member->CastToBot());
				}
			}
		}
		else {
			std::list<Mob*> group_list;
			bot_grouped_player->GetGroup()->GetMemberList(group_list);
			for (auto member_iter : group_list) {
				if (!MyBots::IsMyBot(bot_owner, member_iter)) {
					continue;
				}

				if (member_iter->GetClass() != cls) {
					continue;
				}

				if (petless && member_iter->GetPet()) {
					continue;
				}

				return static_cast<Bot*>(member_iter);
			}
		}

		return nullptr;
	}

	static Bot* AsGroupMember_ByMinLevelAndClass(Client *bot_owner, Client *bot_grouped_player, uint8 minlvl, uint8 cls, bool petless = false) {
		// This function can be nixed if we can enforce bot level as owner level..and the level check can then be moved to the spell loop in the command function
		if (!bot_owner || !bot_grouped_player)
			return nullptr;
		if (!bot_grouped_player->GetGroup())
			return nullptr;

		std::list<Mob*> group_list;
		bot_grouped_player->GetGroup()->GetMemberList(group_list);
		for (auto member_iter : group_list) {
			if (!MyBots::IsMyBot(bot_owner, member_iter))
				continue;
			if (member_iter->GetLevel() < minlvl || member_iter->GetClass() != cls)
				continue;
			if (petless && member_iter->GetPet())
				continue;

			return static_cast<Bot*>(member_iter);
		}

		return nullptr;
	}

	static Bot* AsSpawned_ByClass(Client *bot_owner, std::list<Bot*> &sbl, uint8 cls, bool petless = false) {
		if (!bot_owner) {
			return nullptr;
		}

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter)) {
				continue;
			}

			if (bot_iter->GetClass() != cls) {
				continue;
			}

			if (petless && bot_iter->GetPet()) {
				continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* AsSpawned_ByMinLevelAndClass(Client *bot_owner, std::list<Bot*> &sbl, uint8 minlvl, uint8 cls, bool petless = false) {
		// This function can be nixed if we can enforce bot level as owner level..and the level check can then be moved to the spell loop in the command function
		if (!bot_owner)
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetLevel() < minlvl || bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* AsTarget_ByBot(Client *bot_owner) {
		if (!bot_owner || !MyBots::IsMyBot(bot_owner, bot_owner->GetTarget()))
			return nullptr;

		return bot_owner->GetTarget()->CastToBot();
	}

	static Bot* AsNamed_ByBot(Client *bot_owner, std::string bot_name) {
		if (!bot_owner || bot_name.empty())
			return nullptr;

		std::list<Bot*> selectable_bot_list;
		MyBots::PopulateSBL_BySpawnedBots(bot_owner, selectable_bot_list);
		for (auto bot_iter : selectable_bot_list) {
			if (!bot_name.compare(bot_iter->GetCleanName()))
				return bot_iter;
		}

		return nullptr;
	}

	static Bot* Select_ByClass(Client* bot_owner, BCEnum::TType target_type, std::list<Bot*>& sbl, uint8 cls, Mob* target_mob = nullptr, bool petless = false) {
		if (!bot_owner || sbl.empty())
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;
			if (target_type == BCEnum::TT_GroupV1) {
				if (!target_mob)
					return nullptr;
				else if (bot_iter->GetGroup() != target_mob->GetGroup())
					continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* Select_ByMinLevelAndClass(Client* bot_owner, BCEnum::TType target_type, std::list<Bot*>& sbl, uint8 minlvl, uint8 cls, Mob* target_mob = nullptr, bool petless = false) {
		if (!bot_owner || sbl.empty())
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetLevel() < minlvl || bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;
			if (target_type == BCEnum::TT_GroupV1) {
				if (!target_mob)
					return nullptr;
				else if (bot_iter->GetGroup() != target_mob->GetGroup())
					continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	// Filters actual 'sbl' list
	static void Filter_ByClasses(Client* bot_owner, std::list<Bot*>& sbl, uint16 class_mask) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([class_mask](const Bot* l) { return (GetPlayerClassBit(l->GetClass()) & (~class_mask)); });
	}

	static void Filter_ByMinLevel(Client* bot_owner, std::list<Bot*>& sbl, uint8 min_level) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([min_level](const Bot* l) { return (l->GetLevel() < min_level); });
	}

	static void Filter_ByArcher(Client* bot_owner, std::list<Bot*>& sbl) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([bot_owner](Bot* l) { return (!l->IsBotArcher()); });
	}

	static void Filter_ByHighestSkill(Client* bot_owner, std::list<Bot*>& sbl, EQ::skills::SkillType skill_type, float& skill_value) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		skill_value = 0.0f;

		float mod_skill_value = 0.0f;
		const Bot* skilled_bot = nullptr;
		for (auto bot_iter : sbl) {
			float base_skill_value = bot_iter->GetSkill(skill_type);
			if (base_skill_value == 0.0f)
				continue;

			mod_skill_value = base_skill_value;
			for (int16 index = EQ::invslot::EQUIPMENT_BEGIN; index <= EQ::invslot::EQUIPMENT_END; ++index) {
				const EQ::ItemInstance* indexed_item = bot_iter->GetBotItem(index);
				if (indexed_item && indexed_item->GetItem()->SkillModType == skill_type)
					mod_skill_value += (base_skill_value * (((float)indexed_item->GetItem()->SkillModValue) / 100.0f));
			}

			if (!skilled_bot) {
				skill_value = mod_skill_value;
				skilled_bot = bot_iter;
			}
			else if (mod_skill_value > skill_value) {
				skill_value = mod_skill_value;
				skilled_bot = bot_iter;
			}
		}

		sbl.remove_if([skilled_bot](const Bot* l) { return (l != skilled_bot); });
	}

	static void Filter_ByHighestPickLock(Client* bot_owner, std::list<Bot*>& sbl, float& pick_lock_value) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([bot_owner](const Bot* l) { return (l->GetClass() != Class::Rogue && l->GetClass() != Class::Bard); });
		sbl.remove_if([bot_owner](const Bot* l) { return (l->GetClass() == Class::Rogue && l->GetLevel() < 5); });
		sbl.remove_if([bot_owner](const Bot* l) { return (l->GetClass() == Class::Bard && l->GetLevel() < 40); });

		ActionableBots::Filter_ByHighestSkill(bot_owner, sbl, EQ::skills::SkillPickLock, pick_lock_value);
	}
}


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

// Bot Command System:
int bot_command_init(void);
void bot_command_deinit(void);
int bot_command_add(std::string bot_command_name, const char *desc, int access, BotCmdFuncPtr function);
int bot_command_not_avail(Client *c, const char *message);
int bot_command_real_dispatch(Client *c, char const *message);

// Bot Commands
void bot_command_actionable(Client *c, const Seperator *sep);
void bot_command_aggressive(Client *c, const Seperator *sep);
void bot_command_apply_poison(Client *c, const Seperator *sep);
void bot_command_apply_potion(Client* c, const Seperator* sep);
void bot_command_attack(Client *c, const Seperator *sep);
void bot_command_bind_affinity(Client *c, const Seperator *sep);
void bot_command_bot(Client *c, const Seperator *sep);
void bot_command_caster_range(Client* c, const Seperator* sep);
void bot_command_charm(Client *c, const Seperator *sep);
void bot_command_click_item(Client* c, const Seperator* sep);
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
void bot_command_item_use(Client *c, const Seperator *sep);
void bot_command_levitation(Client *c, const Seperator *sep);
void bot_command_lull(Client *c, const Seperator *sep);
void bot_command_mesmerize(Client *c, const Seperator *sep);
void bot_command_movement_speed(Client *c, const Seperator *sep);
void bot_command_owner_option(Client *c, const Seperator *sep);
void bot_command_pet(Client *c, const Seperator *sep);
void bot_command_pick_lock(Client *c, const Seperator *sep);
void bot_command_pickpocket(Client* c, const Seperator* sep);
void bot_command_precombat(Client* c, const Seperator* sep);
void bot_command_pull(Client *c, const Seperator *sep);
void bot_command_release(Client *c, const Seperator *sep);
void bot_command_resistance(Client *c, const Seperator *sep);
void bot_command_resurrect(Client *c, const Seperator *sep);
void bot_command_rune(Client *c, const Seperator *sep);
void bot_command_send_home(Client *c, const Seperator *sep);
void bot_command_size(Client *c, const Seperator *sep);
void bot_command_spell_list(Client* c, const Seperator *sep);
void bot_command_spell_settings_add(Client* c, const Seperator *sep);
void bot_command_spell_settings_delete(Client* c, const Seperator *sep);
void bot_command_spell_settings_list(Client* c, const Seperator *sep);
void bot_command_spell_settings_toggle(Client* c, const Seperator *sep);
void bot_command_spell_settings_update(Client* c, const Seperator *sep);
void bot_spell_info_dialogue_window(Client* c, const Seperator *sep);
void bot_command_enforce_spell_list(Client* c, const Seperator* sep);
void bot_command_summon_corpse(Client *c, const Seperator *sep);
void bot_command_suspend(Client *c, const Seperator *sep);
void bot_command_taunt(Client *c, const Seperator *sep);
void bot_command_timer(Client* c, const Seperator* sep);
void bot_command_track(Client *c, const Seperator *sep);
void bot_command_view_combos(Client *c, const Seperator *sep);
void bot_command_water_breathing(Client *c, const Seperator *sep);

// Bot Subcommands
void bot_command_appearance(Client *c, const Seperator *sep);
void bot_command_beard_color(Client *c, const Seperator *sep);
void bot_command_beard_style(Client *c, const Seperator *sep);
void bot_command_camp(Client *c, const Seperator *sep);
void bot_command_clone(Client *c, const Seperator *sep);
void bot_command_create(Client *c, const Seperator *sep);
void bot_command_delete(Client *c, const Seperator *sep);
void bot_command_details(Client *c, const Seperator *sep);
void bot_command_dye_armor(Client *c, const Seperator *sep);
void bot_command_eyes(Client *c, const Seperator *sep);
void bot_command_face(Client *c, const Seperator *sep);
void bot_command_follow_distance(Client *c, const Seperator *sep);
void bot_command_hair_color(Client *c, const Seperator *sep);
void bot_command_hairstyle(Client *c, const Seperator *sep);
void bot_command_heritage(Client *c, const Seperator *sep);
void bot_command_inspect_message(Client *c, const Seperator *sep);
void bot_command_list_bots(Client *c, const Seperator *sep);
void bot_command_out_of_combat(Client *c, const Seperator *sep);
void bot_command_report(Client *c, const Seperator *sep);
void bot_command_spawn(Client *c, const Seperator *sep);
void bot_command_stance(Client *c, const Seperator *sep);
void bot_command_stop_melee_level(Client *c, const Seperator *sep);
void bot_command_suffix(Client *c, const Seperator *sep);
void bot_command_summon(Client *c, const Seperator *sep);
void bot_command_surname(Client *c, const Seperator *sep);
void bot_command_tattoo(Client *c, const Seperator *sep);
void bot_command_title(Client *c, const Seperator *sep);
void bot_command_toggle_archer(Client *c, const Seperator *sep);
void bot_command_toggle_helm(Client *c, const Seperator *sep);
void bot_command_update(Client *c, const Seperator *sep);
void bot_command_woad(Client *c, const Seperator *sep);

void bot_command_circle(Client *c, const Seperator *sep);
void bot_command_heal_rotation_adaptive_targeting(Client *c, const Seperator *sep);
void bot_command_heal_rotation_add_member(Client *c, const Seperator *sep);
void bot_command_heal_rotation_add_target(Client *c, const Seperator *sep);
void bot_command_heal_rotation_adjust_critical(Client *c, const Seperator *sep);
void bot_command_heal_rotation_adjust_safe(Client *c, const Seperator *sep);
void bot_command_heal_rotation_casting_override(Client *c, const Seperator *sep);
void bot_command_heal_rotation_change_interval(Client *c, const Seperator *sep);
void bot_command_heal_rotation_clear_hot(Client *c, const Seperator *sep);
void bot_command_heal_rotation_clear_targets(Client *c, const Seperator *sep);
void bot_command_heal_rotation_create(Client *c, const Seperator *sep);
void bot_command_heal_rotation_delete(Client *c, const Seperator *sep);
void bot_command_heal_rotation_fast_heals(Client *c, const Seperator *sep);
void bot_command_heal_rotation_list(Client *c, const Seperator *sep);
void bot_command_heal_rotation_remove_member(Client *c, const Seperator *sep);
void bot_command_heal_rotation_remove_target(Client *c, const Seperator *sep);
void bot_command_heal_rotation_reset_limits(Client *c, const Seperator *sep);
void bot_command_heal_rotation_save(Client *c, const Seperator *sep);
void bot_command_heal_rotation_set_hot(Client *c, const Seperator *sep);
void bot_command_heal_rotation_start(Client *c, const Seperator *sep);
void bot_command_heal_rotation_stop(Client *c, const Seperator *sep);
void bot_command_inventory_give(Client *c, const Seperator *sep);
void bot_command_inventory_list(Client *c, const Seperator *sep);
void bot_command_inventory_remove(Client *c, const Seperator *sep);
void bot_command_inventory_window(Client *c, const Seperator *sep);
void bot_command_pet_get_lost(Client *c, const Seperator *sep);
void bot_command_pet_remove(Client *c, const Seperator *sep);
void bot_command_pet_set_type(Client *c, const Seperator *sep);
void bot_command_portal(Client *c, const Seperator *sep);


// bot command helpers
bool helper_bot_appearance_fail(Client *bot_owner, Bot *my_bot, BCEnum::AFType fail_type, const char* type_desc);
void helper_bot_appearance_form_final(Client *bot_owner, Bot *my_bot);
void helper_bot_appearance_form_update(Bot *my_bot);
uint32 helper_bot_create(Client *bot_owner, std::string bot_name, uint8 bot_class, uint16 bot_race, uint8 bot_gender);
void helper_bot_out_of_combat(Client *bot_owner, Bot *my_bot);
int helper_bot_follow_option_chain(Client *bot_owner);
bool helper_cast_standard_spell(Bot* casting_bot, Mob* target_mob, int spell_id, bool annouce_cast = true, uint32* dont_root_before = nullptr);
bool helper_command_disabled(Client *bot_owner, bool rule_value, const char *command);
bool helper_command_alias_fail(Client *bot_owner, const char* command_handler, const char *alias, const char *command);
void helper_command_depart_list(Client* bot_owner, Bot* druid_bot, Bot* wizard_bot, bcst_list* local_list, bool single_flag = false);
bool helper_is_help_or_usage(const char* arg);
bool helper_no_available_bots(Client *bot_owner, Bot *my_bot = nullptr);
void helper_send_available_subcommands(Client *bot_owner, const char* command_simile, const std::list<const char*>& subcommand_list);
void helper_send_usage_required_bots(Client *bot_owner, BCEnum::SpType spell_type, uint8 bot_class = Class::None);
bool helper_spell_check_fail(STBaseEntry* local_entry);
bool helper_spell_list_fail(Client *bot_owner, bcst_list* spell_list, BCEnum::SpType spell_type);
#endif
