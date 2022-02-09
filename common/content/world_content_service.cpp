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

#include "world_content_service.h"
#include "../database.h"
#include "../rulesys.h"
#include "../eqemu_logsys.h"
#include "../repositories/content_flags_repository.h"


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
void WorldContentService::SetContentFlags(std::vector<ContentFlagsRepository::ContentFlags> content_flags)
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
			(f.enabled ? "Enabled" : "Disabled")
		);
	}

	SetContentFlags(set_content_flags);
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
