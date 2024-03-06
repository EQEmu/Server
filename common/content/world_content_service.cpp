#include "world_content_service.h"

#include <utility>
#include <glm/vec3.hpp>
#include "../database.h"
#include "../rulesys.h"
#include "../eqemu_logsys.h"
#include "../repositories/instance_list_repository.h"


WorldContentService::WorldContentService()
{
	SetCurrentExpansion(Expansion::EXPANSION_ALL);
}

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
		content_service.SetCurrentExpansion(expansion);
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
	if (content_service.GetCurrentExpansion() == Expansion::EXPANSION_ALL) {
		return "All Expansions";
	}

	if (current_expansion >= Expansion::Classic && current_expansion <= Expansion::MaxId) {
		return Expansion::ExpansionName[content_service.GetCurrentExpansion()];
	}

	return "Unknown Expansion";
}

/**
 * @param current_expansion
 */
void WorldContentService::SetCurrentExpansion(int current_expansion)
{
	WorldContentService::current_expansion = current_expansion;
}

/**
 * @return
 */
const std::vector<ContentFlagsRepository::ContentFlags> &WorldContentService::GetContentFlags() const
{
	return content_flags;
}

/**
 * @return
 */
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

/**
 * @return
 */
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

/**
 * @param content_flags
 */
void WorldContentService::SetContentFlags(const std::vector<ContentFlagsRepository::ContentFlags> &content_flags)
{
	WorldContentService::content_flags = content_flags;
}

/**
 * @param content_flag
 * @return
 */
bool WorldContentService::IsContentFlagEnabled(const std::string &content_flag)
{
	for (auto &f: GetContentFlags()) {
		if (f.flag_name == content_flag && f.enabled == true) {
			return true;
		}
	}

	return false;
}

/**
 * @param content_flag
 * @return
 */
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
	LoadZones();
	LoadStaticGlobalZoneInstances();
}

Database *WorldContentService::GetDatabase() const
{
	return m_database;
}

WorldContentService *WorldContentService::SetDatabase(Database *database)
{
	WorldContentService::m_database = database;

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

// HandleZoneRoutingMiddleware is meant to handle content and context aware zone routing
//
// example # 1
// lavastorm (pre-don) version 0 (classic)
// lavastorm (don) version 1
// we want to route players to the correct version of lavastorm based on the current server side expansion
// in order to do that the simplest and cleanest way we intercept the zoning process and route players to an "instance" of the zone
// the reason why we're doing this is because all of the zoning logic already is handled by two keys "zone_id" and "instance_id"
// we can leverage static, never expires instances to handle this but to the client they don't see it any other way than a public normal zone
// scripts handle all the same way, you don't have to think about instances, the middleware will handle the magic
// the versions of zones are represented by two zone entries that have potentially different min/max expansion and/or different content flags
// we decide to route the client to the correct version of the zone based on the current server side expansion
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
WorldContentService * WorldContentService::LoadStaticGlobalZoneInstances()
{
	m_zone_instances = InstanceListRepository::GetWhere(*GetDatabase(), fmt::format("never_expires = 1 AND is_global = 1"));

	LogInfo("Loaded [{}] zone_instances", m_zone_instances.size());

	return this;
}

// LoadZones sets the zones for the world content service
// this is used for zone routing middleware
// we pull the zone list from the zone repository and feed from the zone store for now
// we're holding a copy in the content service - but we're talking 250kb of data in memory to handle routing of zoning
WorldContentService * WorldContentService::LoadZones()
{
	m_zones = ZoneRepository::All(*GetContentDatabase());

	LogInfo("Loaded [{}] zones", m_zones.size());

	return this;
}

// FindZone is critical to the zone routing middleware and any logic that needs to route players to the correct zone
// era contextual routing, multiple version of zones, etc
WorldContentService::FindZoneResult WorldContentService::FindZone(uint32 zone_id, uint32 instance_id)
{
	// if there's an active dynamic instance, we don't need to route
	if (instance_id > 0) {
		auto inst = InstanceListRepository::FindOne(*GetDatabase(), instance_id);
		if (inst.id != 0 && !inst.is_global && !inst.never_expires) {
			return WorldContentService::FindZoneResult{
				.zone_id = 0,
			};
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			auto f = ContentFlags{
				.min_expansion = z.min_expansion,
				.max_expansion = z.max_expansion,
				.content_flags = z.content_flags,
				.content_flags_disabled = z.content_flags_disabled
			};

			if (DoesPassContentFiltering(f)) {
				LogInfo(
					"Attempting to route player to zone [{}] ({}) version [{}] long_name [{}]",
					z.short_name,
					z.zoneidnumber,
					z.version,
					z.long_name
				);

				// first pass, explicit match on public static global zone instances
				for (auto &i: m_zone_instances) {
					if (i.zone == zone_id && i.version == z.version) {
						LogInfo(
							"Routed player to instance [{}] of zone [{}] ({}) version [{}] long_name [{}] notes [{}]",
							i.id,
							z.short_name,
							z.zoneidnumber,
							z.version,
							z.long_name,
							i.notes
						);

						return WorldContentService::FindZoneResult{
							.zone_id = static_cast<uint32>(z.zoneidnumber),
							.instance = i,
							.zone = z
						};
					}
				}

				LogInfo(
					"Routed player to non-instance zone [{}] ({}) version [{}] long_name [{}] notes [{}]",
					z.short_name,
					z.zoneidnumber,
					z.version,
					z.long_name,
					z.note
				);

				return WorldContentService::FindZoneResult{
					.zone_id = static_cast<uint32>(z.zoneidnumber),
					.instance = InstanceListRepository::NewEntity(),
					.zone = z
				};
			}
		}
	}

	return WorldContentService::FindZoneResult{.zone_id = 0};
}

