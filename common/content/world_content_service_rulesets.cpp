#include "world_content_service.h"
#include "../repositories/rule_sets_repository.h"
#include "../repositories/rule_values_repository.h"

// RuleSet is a struct that contains a ruleset and its associated rules.
struct RuleSet {
	RuleSetsRepository::RuleSets                  rule_set;
	std::vector<RuleValuesRepository::RuleValues> rules;
};

inline RuleSet GetClassicRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 100,
			.name = "Original (Classic)",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::Classic,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "Classic client-based ruleset",
		},
		.rules = {
			{
				.rule_name = "Character:EnableXTargetting",
				.rule_value = "false",
				.notes = "Added in Call of the Forsaken"
			},
			{
				.rule_name = "Character:LeaveCorpses",
				.rule_value = "true",
				.notes = "Leave corpses behind"
			},
			{
				.rule_name = "Character:LeaveNakedCorpses",
				.rule_value = "false",
				.notes = "Gear left on corpses until SoD"
			},
			{
				.rule_name = "Character:MaxExpLevel",
				.rule_value = "50",
				.notes = "Level 50 cap until Kunark"
			},
			{
				.rule_name = "Character:MaxLevel",
				.rule_value = "50",
				.notes = "Level 50 cap until Kunark"
			},
			{
				.rule_name = "Character:RestRegenEnabled",
				.rule_value = "false",
				.notes = "OoC regen not added until TSS"
			},
			{
				.rule_name = "Character:SharedBankPlat",
				.rule_value = "false",
				.notes = "No shared bank until LoY"
			},
			{
				.rule_name = "Character:StatCap",
				.rule_value = "255",
				.notes = "Classic stat cap is 255"
			},
			{
				.rule_name = "Character:UseOldBindWound",
				.rule_value = "true",
				.notes = ""
			},
			{
				.rule_name = "Character:UseOldClassExpPenalties",
				.rule_value = "true",
				.notes = "Exp penalties removed in SoF"
			},
			{
				.rule_name = "Character:UseOldConSystem",
				.rule_value = "true",
				.notes = "Con system changed in SoF"
			},
			{
				.rule_name = "Character:UseOldRaceExpPenalties",
				.rule_value = "true",
				.notes = "Exp penalties removed in SoF"
			},
			{
				.rule_name = "Character:UseOldRaceRezEffects",
				.rule_value = "true",
				.notes = "May need testing to ensure it can't be dispelled"
			},
			{
				.rule_name = "Character:UseRaceClassExpBonuses",
				.rule_value = "true",
				.notes = ""
			},
			{
				.rule_name = "Chat:EnableVoiceMacros",
				.rule_value = "false",
				.notes = "Introduced in GoD"
			},
			{
				.rule_name = "Chat:ServerWideAuction",
				.rule_value = "false",
				.notes = "Disable Server-Wide Auction Chat"
			},
			{
				.rule_name = "Chat:ServerWideOOC",
				.rule_value = "false",
				.notes = "Disable Server-Wide OOC Chat"
			},
			{
				.rule_name = "Combat:ClassicNPCBackstab",
				.rule_value = "true",
				.notes = "Disables front backstab"
			},
			{
				.rule_name = "Mail:EnableMailSystem",
				.rule_value = "false",
				.notes = "Mail system added in DoN"
			},
			{
				.rule_name = "Spells:PreNerfBardAEDoT",
				.rule_value = "true",
				.notes = "Bard AE nerf not added until OoW"
			},
			{
				.rule_name = "Spells:WizCritLevel",
				.rule_value = "65",
				.notes = "Wiz non-AA crits not until Luclin"
			},
			{
				.rule_name = "TaskSystem:EnableTaskSystem",
				.rule_value = "false",
				.notes = "Task system added in OoW"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "0",
				.notes = "Classic Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:EnableReturnHomeButton",
				.rule_value = "false",
				.notes = "Return Home added in DoN"
			},
			{
				.rule_name = "World:EnableTutorialButton",
				.rule_value = "false",
				.notes = "Tutorial added in DoN"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "0",
				.notes = "Classic Client-Based Expansion Setting"
			},
		}
	};
}

inline RuleSet GetKunarkRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 101,
			.name = "Ruins of Kunark",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::TheRuinsOfKunark,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "Kunark client-based ruleset. Level 60 cap until PoP.",
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "1",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "1",
				.notes = "Kunark Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "1",
				.notes = "Kunark Client-Based Expansion Setting"
			},
			{
				.rule_name = "Character:MaxExpLevel",
				.rule_value = "60",
				.notes = "Level 60 cap until PoP"
			},
			{
				.rule_name = "Character:MaxLevel",
				.rule_value = "60",
				.notes = "Level 60 cap until PoP"
			},
		}
	};
}

inline RuleSet GetVeliousRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 102,
			.name = "Scars of Velious",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::TheScarsOfVelious,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "Velious client-based ruleset. Level 60 cap until PoP."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "2",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "3",
				.notes = "Velious Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "3",
				.notes = "Velious Client-Based Expansion Setting"
			},
			{
				.rule_name = "Character:MaxExpLevel",
				.rule_value = "60",
				.notes = "Level 60 cap until PoP"
			},
			{
				.rule_name = "Character:MaxLevel",
				.rule_value = "60",
				.notes = "Level 60 cap until PoP"
			},
		}
	};
}

inline RuleSet GetLuclinRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 103,
			.name = "Shadows of Luclin",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::TheShadowsOfLuclin,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "Luclin client-based ruleset. Level 60 cap. Added Wiz crits."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "3",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "7",
				.notes = "Luclin Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "7",
				.notes = "Luclin Client-Based Expansion Setting"
			},
			{
				.rule_name = "Spells:WizCritLevel",
				.rule_value = "12",
				.notes = "Wizard non-AA Criticals Not added until Luclin"
			},
		}
	};
}

inline RuleSet GetPlanesRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 104,
			.name = "Planes of Power",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::ThePlanesOfPower,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "PoP client-based ruleset. Level 65 cap. Stat cap increased. Bind Wound changed."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "4",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "15",
				.notes = "Planes of Power Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "15",
				.notes = "Planes of Power Client-Based Expansion Setting"
			},
			{
				.rule_name = "Character:MaxExpLevel",
				.rule_value = "65",
				.notes = "Level 65 cap until Omens of War"
			},
			{
				.rule_name = "Character:MaxLevel",
				.rule_value = "65",
				.notes = "Level 65 cap until Omens of War"
			},
			{
				.rule_name = "Character:StatCap",
				.rule_value = "305",
				.notes = "Stat cap raised from 255 to 305 with PoP"
			},
			{
				.rule_name = "Character:UseOldBindWound",
				.rule_value = "false",
				.notes = "PoP introduced a more effective bind wound system"
			},
		}
	};
}

// Returns a RuleSet for the Legacy of Ykesha ruleset.
inline RuleSet GetLegacyOfYkeshaRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 105,
			.name = "Legacy of Ykesha",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::TheLegacyOfYkesha,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "LoY client-based ruleset. Shared bank introduced. Stat cap raised to 350."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "5",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "31",
				.notes = "Legacy of Ykesha Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "31",
				.notes = "Legacy of Ykesha Client-Based Expansion Setting"
			},
			{
				.rule_name = "Character:MaxExpLevel",
				.rule_value = "65",
				.notes = "Level 65 cap until Omens of War"
			},
			{
				.rule_name = "Character:MaxLevel",
				.rule_value = "65",
				.notes = "Level 65 cap until Omens of War"
			},
			{
				.rule_name = "Character:SharedBankPlat",
				.rule_value = "true",
				.notes = "Shared bank enabled in Legacy of Ykesha"
			},
			{
				.rule_name = "Character:StatCap",
				.rule_value = "350",
				.notes = "Stat cap increased to 350 with LoY (was 305 in PoP)"
			},
		}
	};
}

inline RuleSet GetLDoNRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 106,
			.name = "Lost Dungeons of Norrath",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::LostDungeonsOfNorrath,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "LDoN client-based ruleset. Stat cap raised to 400."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "6",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "63",
				.notes = "Lost Dungeons of Norrath Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "63",
				.notes = "Lost Dungeons of Norrath Client-Based Expansion Setting"
			},
			{
				.rule_name = "Character:StatCap",
				.rule_value = "400",
				.notes = "Stat cap increased to 400 with LDoN (was 350 in LoY)"
			},
		}
	};
}

inline RuleSet GetGatesOfDiscordRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 107,
			.name = "Gates of Discord",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::GatesOfDiscord,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "GoD client-based ruleset. Voice macros introduced."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "7",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "127",
				.notes = "Gates of Discord Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "127",
				.notes = "Gates of Discord Client-Based Expansion Setting"
			},
			{
				.rule_name = "Chat:EnableVoiceMacros",
				.rule_value = "true",
				.notes = "Enable Voice Macros - Introduced in Gates of Discord"
			},
		}
	};
}

inline RuleSet GetOmensOfWarRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 108,
			.name = "Omens of War",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::OmensOfWar,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "OoW client-based ruleset. Level cap raised to 70. Bard AE nerfed. Task system enabled."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "8",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "255",
				.notes = "Omens of War Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "255",
				.notes = "Omens of War Client-Based Expansion Setting"
			},
			{
				.rule_name = "Spells:PreNerfBardAEDoT",
				.rule_value = "false",
				.notes = "Bard AE Nerf"
			},
			{
				.rule_name = "TaskSystem:EnableTaskSystem",
				.rule_value = "true",
				.notes = "Task system was introduced in Omens of War"
			},
			{
				.rule_name = "Character:MaxExpLevel",
				.rule_value = "70",
				.notes = "Level 70 cap until The Serpent's Spine"
			},
			{
				.rule_name = "Character:MaxLevel",
				.rule_value = "70",
				.notes = "Level 70 cap until The Serpent's Spine"
			},
		}
	};
}

inline RuleSet GetDragonsOfNorrathRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 109,
			.name = "Dragons of Norrath",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::DragonsOfNorrath,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "DoN client-based ruleset. Mail system introduced. Accursed Nest unlockable."
		},
		.rules = {
			{
				.rule_name = "Expansion:CurrentExpansion",
				.rule_value = "9",
				.notes = "Current Expansion"
			},
			{
				.rule_name = "World:CharacterSelectExpansionSettings",
				.rule_value = "511",
				.notes = "Dragons of Norrath Client-Based Expansion Setting"
			},
			{
				.rule_name = "World:ExpansionSettings",
				.rule_value = "511",
				.notes = "Dragons of Norrath Client-Based Expansion Setting"
			},
			{
				.rule_name = "Mail:EnableMailSystem",
				.rule_value = "true",
				.notes = "Mail System not added until Dragons of Norrath"
			},
		}
	};
}

inline RuleSet GetEraOverridesRuleset()
{
	return {
		.rule_set = {
			.ruleset_id = 200,
			.name = "Era Overrides",
			.zone_ids = "",
			.instance_versions = "",
			.content_flags = "",
			.content_flags_disabled = "",
			.min_expansion = Expansion::EXPANSION_ALL,
			.max_expansion = Expansion::EXPANSION_MAX,
			.notes = "If you want to override any rules for era specific rulesets, use this ruleset. It will not be used by default.",
		},
		.rules = {
			// users will add their own dynamically
		}
	};
}

inline std::vector<RuleSet> GetCustomRulesets()
{
	return {
		{
			.rule_set = {
				.ruleset_id = 300,
				.name = "Double Experience",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = Expansion::EXPANSION_ZERO_VALUE,
				.max_expansion = Expansion::EXPANSION_ZERO_VALUE,
				.notes = "Doubles EXP globally",
			},
			.rules = {
				{
					.rule_name = "Character:FinalExpMultiplier",
					.rule_value = "2"
				}
			}
		},
		{
			.rule_set = {
				.ruleset_id = 1000,
				.name = "Custom Boundary (Put your custom rulesets after here)",
				.zone_ids = "",
				.instance_versions = "",
				.content_flags = "",
				.content_flags_disabled = "",
				.min_expansion = Expansion::EXPANSION_ZERO_VALUE,
				.max_expansion = Expansion::EXPANSION_ZERO_VALUE,
				.notes = "You may add your own rulesets above 1000+"
			},
		},
	};
}

inline std::vector<RuleSet> GetDefaultRulesets()
{
	std::vector<RuleSet> sets = {
		GetClassicRuleset(),
		GetKunarkRuleset(),
		GetVeliousRuleset(),
		GetLuclinRuleset(),
		GetPlanesRuleset(),
		GetLegacyOfYkeshaRuleset(),
		GetLDoNRuleset(),
		GetGatesOfDiscordRuleset(),
		GetOmensOfWarRuleset(),
		GetDragonsOfNorrathRuleset(),
		GetEraOverridesRuleset()
	};

	// Append custom rulesets
	const auto custom = GetCustomRulesets();
	sets.insert(sets.end(), custom.begin(), custom.end());

	return sets;
}

void WorldContentService::SeedDefaultRulesets() const
{
	LogInfo("Seeding default rulesets");

	// Load existing rule_sets into a map for fast lookup
	std::unordered_map<uint32_t, RuleSetsRepository::RuleSets> existing_rulesets_map;
	for (const auto& r : RuleSetsRepository::All(*m_database)) {
		existing_rulesets_map[r.ruleset_id] = r;
	}

	// Load all existing rule_values once
	std::unordered_set<std::string> existing_rule_keys;
	for (const auto& r : RuleValuesRepository::All(*m_database)) {
		existing_rule_keys.insert(fmt::format("{}|{}", r.ruleset_id, r.rule_name));
	}

	for (const auto& entry : GetDefaultRulesets()) {
		const auto& new_ruleset = entry.rule_set;

		bool should_insert = !existing_rulesets_map.count(new_ruleset.ruleset_id);
		bool should_update = false;

		if (!should_insert) {
			const auto& existing = existing_rulesets_map[new_ruleset.ruleset_id];

			// Compare metadata fields
			should_update = (
				new_ruleset.name != existing.name ||
				new_ruleset.zone_ids != existing.zone_ids ||
				new_ruleset.instance_versions != existing.instance_versions ||
				new_ruleset.content_flags != existing.content_flags ||
				new_ruleset.content_flags_disabled != existing.content_flags_disabled ||
				new_ruleset.min_expansion != existing.min_expansion ||
				new_ruleset.max_expansion != existing.max_expansion ||
				new_ruleset.notes != existing.notes
			);
		}

		if (should_insert) {
			RuleSetsRepository::InsertOne(*m_database, new_ruleset);
			LogInfo("Inserted ruleset [{}] {}", new_ruleset.ruleset_id, new_ruleset.name);
		}
		else if (should_update) {
			RuleSetsRepository::UpdateOne(*m_database, new_ruleset);
			LogInfo("Updated ruleset metadata [{}] {}", new_ruleset.ruleset_id, new_ruleset.name);
		}

		if (!entry.rules.empty()) {
			std::vector<RuleValuesRepository::RuleValues> to_insert;

			for (auto rule : entry.rules) {
				rule.ruleset_id = new_ruleset.ruleset_id;

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
				LogInfo("Inserted [{}] rule(s) into ruleset [{}]", to_insert.size(), new_ruleset.ruleset_id);
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

	auto rules = RuleValuesRepository::GetWhere(*m_database, "TRUE ORDER BY ruleset_id, rule_name");
	auto sets  = RuleSetsRepository::GetWhere(*m_database, "TRUE ORDER BY ruleset_id");
	for (auto& e : sets) {
		bool has_filters =
			!e.zone_ids.empty() ||
			!e.instance_versions.empty() ||
			!e.content_flags.empty() ||
			!e.content_flags_disabled.empty() ||
			e.min_expansion != Expansion::EXPANSION_ZERO_VALUE ||
			e.max_expansion != Expansion::EXPANSION_ZERO_VALUE;
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

		for (auto& r : rules) {
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
