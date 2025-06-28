#include "world_content_service.h"

#include <glm/vec3.hpp>
#include "../database.h"
#include "../rulesys.h"
#include "../eqemu_logsys.h"
#include "../repositories/instance_list_repository.h"
#include "../repositories/rule_sets_repository.h"
#include "../repositories/rule_values_repository.h"
#include "../zone_store.h"

int WorldContentService::GetCurrentExpansion() const
{
	return current_expansion;
}

WorldContentService *WorldContentService::SetExpansionContext()
{
	// do a rule manager reload until where we store expansion is changed to somewhere else
	RuleManager::Instance()->LoadRules(GetDatabase(), "default", true);

	// pull expansion from rules
	int expansion = RuleI(Expansion, CurrentExpansion);
	if (expansion >= Expansion::Classic && expansion <= Expansion::MaxId) {
		SetCurrentExpansion(expansion);
	}

	LogInfo(
		"Current expansion is [{}] ({})",
		GetCurrentExpansion(),
		GetCurrentExpansionName()
	);

	return this;
}

std::string WorldContentService::GetCurrentExpansionName()
{
	if (GetCurrentExpansion() == Expansion::EXPANSION_ALL) {
		return "All Expansions";
	}

	if (current_expansion >= Expansion::Classic && current_expansion <= Expansion::MaxId) {
		return Expansion::ExpansionName[GetCurrentExpansion()];
	}

	return "Unknown Expansion";
}

void WorldContentService::SetCurrentExpansion(int current_expansion)
{
	WorldContentService::current_expansion = current_expansion;
}

const std::vector<ContentFlagsRepository::ContentFlags> &WorldContentService::GetContentFlags() const
{
	return content_flags;
}

std::vector<std::string> WorldContentService::GetContentFlagsEnabled()
{
	std::vector<std::string> enabled_flags;

	for (auto &f: GetContentFlags()) {
		if (f.enabled) {
			enabled_flags.emplace_back(f.flag_name);
		}
	}

	return enabled_flags;
}

std::vector<std::string> WorldContentService::GetContentFlagsDisabled()
{
	std::vector<std::string> disabled_flags;

	for (auto &f: GetContentFlags()) {
		if (!f.enabled) {
			disabled_flags.emplace_back(f.flag_name);
		}
	}

	return disabled_flags;
}

void WorldContentService::SetContentFlags(const std::vector<ContentFlagsRepository::ContentFlags> &content_flags)
{
	WorldContentService::content_flags = content_flags;
}

bool WorldContentService::IsContentFlagEnabled(const std::string &content_flag)
{
	for (auto &f: GetContentFlags()) {
		if (f.flag_name == content_flag && f.enabled == true) {
			return true;
		}
	}

	return false;
}

bool WorldContentService::IsContentFlagDisabled(const std::string &content_flag)
{
	for (auto &f: GetContentFlags()) {
		if (f.flag_name == content_flag && f.enabled == false) {
			return true;
		}
	}

	return false;
}

bool WorldContentService::DoesPassContentFiltering(const ContentFlags &f)
{
	// if we're not set to (-1 All) then fail when we aren't within minimum expansion
	if (f.min_expansion > Expansion::EXPANSION_ALL && current_expansion < f.min_expansion && current_expansion != -1) {
		return false;
	}

	// if we're not set to (-1 All) then fail when we aren't within max expansion
	if (f.max_expansion > Expansion::EXPANSION_ALL && current_expansion > f.max_expansion && current_expansion != -1) {
		return false;
	}

	// if we don't have any enabled flag in enabled flags, we fail
	for (const auto &flag: Strings::Split(f.content_flags)) {
		if (!Strings::Contains(GetContentFlagsEnabled(), flag)) {
			return false;
		}
	}

	// if we don't have any disabled flag in disabled flags, we fail
	for (const auto &flag: Strings::Split(f.content_flags_disabled)) {
		if (!Strings::Contains(GetContentFlagsDisabled(), flag)) {
			return false;
		}
	}

	return true;
}

void WorldContentService::ReloadContentFlags()
{
	std::vector<ContentFlagsRepository::ContentFlags> set_content_flags;
	auto                                              flags = ContentFlagsRepository::All(*GetDatabase());

	set_content_flags.reserve(flags.size());
	for (auto &f: flags) {
		set_content_flags.push_back(f);

		LogInfo(
			"Loaded content flag [{}] [{}]",
			f.flag_name,
			(f.enabled ? "enabled" : "disabled")
		);
	}

	SetContentFlags(set_content_flags);
	LoadStaticGlobalZoneInstances();
	ZoneStore::Instance()->LoadZones(*m_content_database);
}

Database *WorldContentService::GetDatabase() const
{
	return m_database;
}

WorldContentService *WorldContentService::SetDatabase(Database *database)
{
	m_database = database;

	return this;
}

Database *WorldContentService::GetContentDatabase() const
{
	return m_content_database;
}

WorldContentService *WorldContentService::SetContentDatabase(Database *database)
{
	WorldContentService::m_content_database = database;

	return this;
}

void WorldContentService::SetContentFlag(const std::string &content_flag_name, bool enabled)
{
	auto flags = ContentFlagsRepository::GetWhere(
		*GetDatabase(),
		fmt::format("flag_name = '{}'", content_flag_name)
	);

	auto f = ContentFlagsRepository::NewEntity();
	if (!flags.empty()) {
		f = flags.front();
	}

	f.enabled   = enabled ? 1 : 0;
	f.flag_name = content_flag_name;

	if (!flags.empty()) {
		ContentFlagsRepository::UpdateOne(*GetDatabase(), f);
	}
	else {
		ContentFlagsRepository::InsertOne(*GetDatabase(), f);
	}

	ReloadContentFlags();
}

void WorldContentService::HandleZoneRoutingMiddleware(ZoneChange_Struct *zc)
{
	auto r = FindZone(zc->zoneID, zc->instanceID);
	if (r.zone_id == 0) {
		return;
	}

	zc->instanceID = r.instance.id;
}

// LoadStaticGlobalZoneInstances loads all static global zone instances
// these are zones that are never set to expire and are global
// these are used commonly in v1/v2/v3 versions of the same zone for expansion routing
WorldContentService *WorldContentService::LoadStaticGlobalZoneInstances()
{
	m_zone_static_instances = InstanceListRepository::GetWhere(
		*GetDatabase(),
		fmt::format("never_expires = 1 AND is_global = 1")
	);

	LogInfo("Loaded [{}] zone_instances", m_zone_static_instances.size());

	return this;
}

// FindZone handles content and context aware zone routing (middleware)
//
// this is a middleware function that is meant to be used in the zone change process
// this hooks all core zone changes within the server and routes the player to the correct zone
// returning a zone_id of non-zero means the middleware will route the player
// returning a zone_id of 0 means the middleware will not route the player
// this is useful for handling multiple versions of the same zone
//
// implementation >
// the zoning and process spawning logic already is handled by two keys "zone_id" and "instance_id"
// we leverage static, never expires instances to handle this and client still sees it as a normal zone
//
// content awareness >
// simply use the zone_id, server content settings and the middleware will handle the rest
// you don't have to think about instances in any data tables (use instance_id 0)
// you don't have to keep track of instance ids in scripts (use instance_id 0)
// the versions of zones are represented by two zone entries that have potentially different min/max expansion and/or different content flags
// we decide to route the client to the correct version of the zone based on the current server side expansion
//
// example >
// we want to route players to the correct version of lavastorm based on the current server side expansion (DoesZonePassContentFiltering)
// lavastorm (pre-don) version 0 (classic)
//   zone table entry for version = 0, min_expansion = 0, max_expansion = 8
//   instance_list table entry for lavastorm has version = 0, is_global = 1, never_expires = 1
// lavastorm (don) version 1
//   zone table entry for version = 1, min_expansion = 9, max_expansion = 99
//   instance_list table entry for lavastorm has version = 1, is_global = 1, never_expires = 1
WorldContentService::FindZoneResult WorldContentService::FindZone(uint32 zone_id, uint32 instance_id)
{
	for (const auto &z: ZoneStore::Instance()->GetZones()) {
		for (auto &i: m_zone_static_instances) {
			if (
				z.zoneidnumber == zone_id &&
				DoesZonePassContentFiltering(z) &&
				i.zone == zone_id &&
				i.version == z.version) {

				if (instance_id > 0 && i.id != instance_id) {
					continue;
				}

				LogInfo(
					"Routed player to public static instance [{}] of zone [{}] ({}) version [{}] long_name [{}] notes [{}]",
					i.id,
					z.short_name,
					z.zoneidnumber,
					z.version,
					z.long_name,
					i.notes
				);

				return FindZoneResult{
					.zone_id = static_cast<uint32>(z.zoneidnumber),
					.instance = i,
					.zone = z
				};
			}
		}
	}

	return FindZoneResult{.zone_id = 0};
}

bool WorldContentService::IsInPublicStaticInstance(uint32 instance_id)
{
	for (auto &i: m_zone_static_instances) {
		if (i.id == instance_id) {
			return true;
		}
	}

	return false;
}

bool WorldContentService::DoesZonePassContentFiltering(const ZoneRepository::Zone &z)
{
	auto f = ContentFlags{
		.min_expansion = z.min_expansion,
		.max_expansion = z.max_expansion,
		.content_flags = z.content_flags,
		.content_flags_disabled = z.content_flags_disabled
	};

	return DoesPassContentFiltering(f);
}

void WorldContentService::SeedDefaultRulesets()
{
	LogInfo("Seeding default rulesets");

	struct RuleSet {
		RuleSetsRepository::RuleSets rule_set;
		std::vector<RuleValuesRepository::RuleValues> rules;
	};

	std::vector<RuleSet> rulesets = {
		// era based rulesets
		{
			.rule_set = {
				.ruleset_id = 100,
				.name = "Original (Classic)",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 0,
				.max_expansion = 1,
				.notes = "Classic client-based ruleset",
			},
			.rules = {
				{ .rule_name = "Character:EnableXTargetting", .rule_value = "false", .notes = "Added in Call of the Forsaken" },
				{ .rule_name = "Character:LeaveCorpses", .rule_value = "true", .notes = "Leave corpses behind" },
				{ .rule_name = "Character:LeaveNakedCorpses", .rule_value = "false", .notes = "Gear left on corpses until SoD" },
				{ .rule_name = "Character:MaxExpLevel", .rule_value = "50", .notes = "Level 50 cap until Kunark" },
				{ .rule_name = "Character:MaxLevel", .rule_value = "50", .notes = "Level 50 cap until Kunark" },
				{ .rule_name = "Character:RestRegenEnabled", .rule_value = "false", .notes = "OoC regen not added until TSS" },
				{ .rule_name = "Character:SharedBankPlat", .rule_value = "false", .notes = "No shared bank until LoY" },
				{ .rule_name = "Character:StatCap", .rule_value = "255", .notes = "Classic stat cap is 255" },
				{ .rule_name = "Character:UseOldBindWound", .rule_value = "true", .notes = "" },
				{ .rule_name = "Character:UseOldClassExpPenalties", .rule_value = "true", .notes = "Exp penalties removed in SoF" },
				{ .rule_name = "Character:UseOldConSystem", .rule_value = "true", .notes = "Con system changed in SoF" },
				{ .rule_name = "Character:UseOldRaceExpPenalties", .rule_value = "true", .notes = "Exp penalties removed in SoF" },
				{ .rule_name = "Character:UseOldRaceRezEffects", .rule_value = "true", .notes = "May need testing to ensure it can't be dispelled" },
				{ .rule_name = "Character:UseRaceClassExpBonuses", .rule_value = "true", .notes = "" },
				{ .rule_name = "Chat:EnableVoiceMacros", .rule_value = "false", .notes = "Introduced in GoD" },
				{ .rule_name = "Chat:ServerWideAuction", .rule_value = "false", .notes = "Disable Server-Wide Auction Chat" },
				{ .rule_name = "Chat:ServerWideOOC", .rule_value = "false", .notes = "Disable Server-Wide OOC Chat" },
				{ .rule_name = "Combat:ClassicNPCBackstab", .rule_value = "true", .notes = "Disables front backstab" },
				{ .rule_name = "Mail:EnableMailSystem", .rule_value = "false", .notes = "Mail system added in DoN" },
				{ .rule_name = "Spells:PreNerfBardAEDoT", .rule_value = "true", .notes = "Bard AE nerf not added until OoW" },
				{ .rule_name = "Spells:WizCritLevel", .rule_value = "65", .notes = "Wiz non-AA crits not until Luclin" },
				{ .rule_name = "TaskSystem:EnableTaskSystem", .rule_value = "false", .notes = "Task system added in OoW" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "0", .notes = "Classic Client-Based Expansion Setting" },
				{ .rule_name = "World:EnableReturnHomeButton", .rule_value = "false", .notes = "Return Home added in DoN" },
				{ .rule_name = "World:EnableTutorialButton", .rule_value = "false", .notes = "Tutorial added in DoN" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "0", .notes = "Classic Client-Based Expansion Setting" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 101,
				.name = "Ruins of Kunark",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 1,
				.max_expansion = 2,
				.notes = "Kunark client-based ruleset. Level 60 cap until PoP.",
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "1", .notes = "Current Expansion" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "1", .notes = "Kunark Client-Based Expansion Setting" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "1", .notes = "Kunark Client-Based Expansion Setting" },
				{ .rule_name = "Character:MaxExpLevel", .rule_value = "60", .notes = "Level 60 cap until PoP" },
				{ .rule_name = "Character:MaxLevel", .rule_value = "60", .notes = "Level 60 cap until PoP" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 102,
				.name = "Scars of Velious",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 2,
				.max_expansion = 3,
				.notes = "Velious client-based ruleset. Level 60 cap until PoP."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "2", .notes = "Current Expansion" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "3", .notes = "Velious Client-Based Expansion Setting" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "3", .notes = "Velious Client-Based Expansion Setting" },
				{ .rule_name = "Character:MaxExpLevel", .rule_value = "60", .notes = "Level 60 cap until PoP" },
				{ .rule_name = "Character:MaxLevel", .rule_value = "60", .notes = "Level 60 cap until PoP" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 103,
				.name = "Shadows of Luclin",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 3,
				.max_expansion = 4,
				.notes = "Luclin client-based ruleset. Level 60 cap. Added Wiz crits."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "3", .notes = "Current Expansion" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "7", .notes = "Luclin Client-Based Expansion Setting" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "7", .notes = "Luclin Client-Based Expansion Setting" },
				{ .rule_name = "Spells:WizCritLevel", .rule_value = "12", .notes = "Wizard non-AA Criticals Not added until Luclin" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 104,
				.name = "Planes of Power",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 4,
				.max_expansion = 5,
				.notes = "PoP client-based ruleset. Level 65 cap. Stat cap increased. Bind Wound changed."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "4", .notes = "Current Expansion" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "15", .notes = "Planes of Power Client-Based Expansion Setting" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "15", .notes = "Planes of Power Client-Based Expansion Setting" },
				{ .rule_name = "Character:MaxExpLevel", .rule_value = "65", .notes = "Level 65 cap until Omens of War" },
				{ .rule_name = "Character:MaxLevel", .rule_value = "65", .notes = "Level 65 cap until Omens of War" },
				{ .rule_name = "Character:StatCap", .rule_value = "305", .notes = "Stat cap raised from 255 to 305 with PoP" },
				{ .rule_name = "Character:UseOldBindWound", .rule_value = "false", .notes = "PoP introduced a more effective bind wound system" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 105,
				.name = "Legacy of Ykesha",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 5,
				.max_expansion = 6,
				.notes = "LoY client-based ruleset. Shared bank introduced. Stat cap raised to 350."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "5", .notes = "Current Expansion" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "31", .notes = "Legacy of Ykesha Client-Based Expansion Setting" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "31", .notes = "Legacy of Ykesha Client-Based Expansion Setting" },
				{ .rule_name = "Character:MaxExpLevel", .rule_value = "65", .notes = "Level 65 cap until Omens of War" },
				{ .rule_name = "Character:MaxLevel", .rule_value = "65", .notes = "Level 65 cap until Omens of War" },
				{ .rule_name = "Character:SharedBankPlat", .rule_value = "true", .notes = "Shared bank enabled in Legacy of Ykesha" },
				{ .rule_name = "Character:StatCap", .rule_value = "350", .notes = "Stat cap increased to 350 with LoY (was 305 in PoP)" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 106,
				.name = "Lost Dungeons of Norrath",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 6,
				.max_expansion = 7,
				.notes = "LDoN client-based ruleset. Stat cap raised to 400."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "6", .notes = "Current Expansion" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "63", .notes = "Lost Dungeons of Norrath Client-Based Expansion Setting" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "63", .notes = "Lost Dungeons of Norrath Client-Based Expansion Setting" },
				{ .rule_name = "Character:StatCap", .rule_value = "400", .notes = "Stat cap increased to 400 with LDoN (was 350 in LoY)" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 107,
				.name = "Gates of Discord",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 7,
				.max_expansion = 8,
				.notes = "GoD client-based ruleset. Voice macros introduced."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "7", .notes = "Current Expansion" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "127", .notes = "Gates of Discord Client-Based Expansion Setting" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "127", .notes = "Gates of Discord Client-Based Expansion Setting" },
				{ .rule_name = "Chat:EnableVoiceMacros", .rule_value = "true", .notes = "Enable Voice Macros - Introduced in Gates of Discord" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 108,
				.name = "Omens of War",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = 8,
				.max_expansion = 9,
				.notes = "OoW client-based ruleset. Level cap raised to 70. Bard AE nerfed. Task system enabled."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "8", .notes = "Current Expansion" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "255", .notes = "Omens of War Client-Based Expansion Setting" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "255", .notes = "Omens of War Client-Based Expansion Setting" },
				{ .rule_name = "Spells:PreNerfBardAEDoT", .rule_value = "false", .notes = "Bard AE Nerf" },
				{ .rule_name = "TaskSystem:EnableTaskSystem", .rule_value = "true", .notes = "Task system was introduced in Omens of War" },
				{ .rule_name = "Character:MaxExpLevel", .rule_value = "70", .notes = "Level 70 cap until The Serpent's Spine" },
				{ .rule_name = "Character:MaxLevel", .rule_value = "70", .notes = "Level 70 cap until The Serpent's Spine" },
			}
		},
		{
			.rule_set = {
				.ruleset_id = 109,
				.name = "Dragons of Norrath",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "don_nest",
				.content_flags_disabled = "",
				.min_expansion = 9,
				.max_expansion = 10,
				.notes = "DoN client-based ruleset. Mail system introduced. Accursed Nest unlockable."
			},
			.rules = {
				{ .rule_name = "Expansion:CurrentExpansion", .rule_value = "9", .notes = "Current Expansion" },
				{ .rule_name = "World:CharacterSelectExpansionSettings", .rule_value = "511", .notes = "Dragons of Norrath Client-Based Expansion Setting" },
				{ .rule_name = "World:ExpansionSettings", .rule_value = "511", .notes = "Dragons of Norrath Client-Based Expansion Setting" },
				{ .rule_name = "Mail:EnableMailSystem", .rule_value = "true", .notes = "Mail System not added until Dragons of Norrath" },
			}
		},

		{
			.rule_set = {
				.ruleset_id = 1000,
				.name = "Custom Boundary (Put your custom rulesets after here)",
				.notes = "You may add your own rulesets above 1000+"
			},
		},

		// non-era rulesets
		{
			.rule_set = {
				.ruleset_id = 100,
				.name = "Double Experience",
				.notes = "Doubles EXP globally"
			},
			.rules = {
				{.rule_name = "Character:FinalExpMultiplier", .rule_value = "2"}
			}
		},
	};

	// Load all existing rule_sets once
	std::unordered_set<uint8_t> existing_ruleset_ids;
	for (const auto& r : RuleSetsRepository::All(*m_database)) {
		existing_ruleset_ids.insert(r.ruleset_id);
	}

	// Load all existing rule_values once
	std::unordered_set<std::string> existing_rule_keys;
	for (const auto& r : RuleValuesRepository::All(*m_database)) {
		existing_rule_keys.insert(fmt::format("{}|{}", r.ruleset_id, r.rule_name));
	}

	// Process in-memory
	for (const auto& entry : rulesets) {
		const auto& ruleset = entry.rule_set;

		if (!existing_ruleset_ids.count(ruleset.ruleset_id)) {
			RuleSetsRepository::InsertOne(*m_database, ruleset);
			LogInfo("Inserted ruleset [{}] - {}", ruleset.ruleset_id, ruleset.name);
		}

		if (!entry.rules.empty()) {
			std::vector<RuleValuesRepository::RuleValues> to_insert;

			for (auto rule : entry.rules) {
				rule.ruleset_id = ruleset.ruleset_id;

				if (rule.notes.empty()) {
					rule.notes = m_rule_manager->GetRuleNotesByName(rule.rule_name);
				}

				auto key = fmt::format("{}|{}", rule.ruleset_id, rule.rule_name);
				if (!existing_rule_keys.count(key)) {
					to_insert.push_back(rule);
				}
			}

			if (!to_insert.empty()) {
				RuleValuesRepository::InsertMany(*m_database, to_insert);
				LogInfo("Inserted [{}] rule(s) into ruleset [{}]", to_insert.size(), ruleset.ruleset_id);
			}
		}
	}
}

void WorldContentService::LoadTargetedRulesets()
{
	if (!m_zone_id) {
		LogError("Zone ID is not set. Cannot load targeted rulesets.");
		return;
	}

	SeedDefaultRulesets();

	LogInfo("Zone ID [{}] Instance Version [{}] - Loading targeted rulesets", m_zone_id, m_instance_version);

	constexpr int8 EXPANSION_ZERO_VALUE = -2;

	auto rules = RuleValuesRepository::GetWhere(*m_database, "TRUE ORDER BY ruleset_id, rule_name");
	auto sets  = RuleSetsRepository::GetWhere(*m_database, "TRUE ORDER BY ruleset_id");
	for (auto& e : sets) {
		bool has_filters =
			!e.zone_ids.empty() ||
			!e.instance_versions.empty() ||
			!e.content_flags.empty() ||
			!e.content_flags_disabled.empty() ||
			e.min_expansion != EXPANSION_ZERO_VALUE ||
			e.max_expansion != EXPANSION_ZERO_VALUE;
		if (!has_filters) {
			continue; // not a targeted ruleset
		}

		auto zone_id = std::to_string(m_zone_id);
		if (!e.zone_ids.empty() && !Strings::Contains(e.zone_ids, zone_id)) {
			continue;
		}

		auto instance_id = std::to_string(m_instance_version);
		if (!e.instance_versions.empty() && !Strings::Contains(e.instance_versions, instance_id)) {
			continue;
		}

		if (!DoesPassContentFiltering(
			ContentFlags{
				.min_expansion = e.min_expansion,
				.max_expansion = e.max_expansion,
				.content_flags = e.content_flags,
				.content_flags_disabled = e.content_flags_disabled
			}
		)) {
			continue;
		}

		for (auto &r : rules) {
			if (r.ruleset_id != e.ruleset_id) {
				continue;
			}

			m_rule_manager->SetRule(r.rule_name, r.rule_value);

			LogInfo(
				"Loading targeted rule from ruleset [{}] ruleset_name [{}] rule_name [{}] rule_value [{}]",
				e.ruleset_id,
				e.name,
				r.rule_name,
				r.rule_value
			);
		}
	}
}

