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

void WorldContentService::SetExpansionContext()
{
	int expansion = RuleI(Expansion, CurrentExpansion);
	if (expansion >= Expansion::Classic && expansion <= Expansion::MaxId) {
		content_service.SetCurrentExpansion(expansion);
	}

	LogInfo(
		"Current expansion is [{}] ({})",
		GetCurrentExpansion(),
		GetCurrentExpansionName()
	);
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
const std::vector<std::string> &WorldContentService::GetContentFlags() const
{
	return content_flags;
}

/**
 * @param content_flags
 */
void WorldContentService::SetContentFlags(std::vector<std::string> content_flags)
{
	WorldContentService::content_flags = content_flags;
}

/**
 * @param content_flag
 * @return
 */
bool WorldContentService::IsContentFlagEnabled(const std::string& content_flag)
{
	for (auto &flag : GetContentFlags()) {
		if (flag == content_flag) {
			return true;
		}
	}

	return false;
}

void WorldContentService::ReloadContentFlags(Database &db)
{
	std::vector<std::string> set_content_flags;
	auto                     content_flags = ContentFlagsRepository::GetWhere(db, "enabled = 1");

	set_content_flags.reserve(content_flags.size());
	for (auto &flags: content_flags) {
		set_content_flags.push_back(flags.flag_name);
	}

	LogInfo(
		"Enabled content flags [{}]",
		implode(", ", set_content_flags)
	);

	SetContentFlags(set_content_flags);
}
