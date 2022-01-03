/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
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

#ifndef EQEMU_CONTENT_FILTER_CRITERIA_H
#define EQEMU_CONTENT_FILTER_CRITERIA_H

#include <string>
#include "../../content/world_content_service.h"
#include "../../string_util.h"

namespace ContentFilterCriteria {
	static std::string apply(std::string table_prefix = "")
	{
		std::string criteria;

		if (!table_prefix.empty()) {
			table_prefix = table_prefix + ".";
		}

		int current_expansion_filter_criteria = content_service.GetCurrentExpansion();
		if (current_expansion_filter_criteria == Expansion::EXPANSION_ALL) {
			current_expansion_filter_criteria = Expansion::EXPANSION_FILTER_MAX;
		}

		criteria += fmt::format(
			" AND ({}min_expansion <= {} OR {}min_expansion = -1)",
			table_prefix,
			current_expansion_filter_criteria,
			table_prefix
		);

		criteria += fmt::format(
			" AND ({}max_expansion >= {} OR {}max_expansion = -1)",
			table_prefix,
			current_expansion_filter_criteria,
			table_prefix
		);

		std::vector<std::string> flags_disabled = content_service.GetContentFlagsDisabled();
		std::vector<std::string> flags_enabled  = content_service.GetContentFlagsEnabled();
		std::string              flags_in_filter_enabled;
		std::string              flags_in_filter_disabled;
		if (!flags_enabled.empty()) {
			flags_in_filter_enabled = fmt::format(
				" OR CONCAT(',', {}content_flags, ',') REGEXP ',({}),' ",
				table_prefix,
				implode("|", flags_enabled)
			);
		}
		if (!flags_disabled.empty()) {
			flags_in_filter_disabled = fmt::format(
				" OR CONCAT(',', {}content_flags_disabled, ',') REGEXP ',({}),' ",
				table_prefix,
				implode("|", flags_disabled)
			);
		}

		criteria += fmt::format(
			" AND (({}content_flags IS NULL OR {}content_flags = ''){}) ",
			table_prefix,
			table_prefix,
			flags_in_filter_enabled
		);

		criteria += fmt::format(
			" AND (({}content_flags_disabled IS NULL OR {}content_flags_disabled = ''){}) ",
			table_prefix,
			table_prefix,
			flags_in_filter_disabled
		);

		return std::string(criteria);
	};
}

#endif //EQEMU_CONTENT_FILTER_CRITERIA_H
