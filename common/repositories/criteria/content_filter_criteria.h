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
	static std::string apply()
	{
		std::string criteria;

		criteria += fmt::format(
			" AND (min_expansion <= {} OR min_expansion = 0)",
			content_service.GetCurrentExpansion()
		);

		criteria += fmt::format(
			" AND (max_expansion >= {} OR max_expansion = 0)",
			content_service.GetCurrentExpansion()
		);

		std::vector<std::string> flags = content_service.GetContentFlags();
		for (auto                &flag: flags) {
			flag = "'" + flag + "'";
		}

		std::string flags_in_filter;
		if (!flags.empty()) {
			flags_in_filter = fmt::format(" OR content_flags IN ({})", implode(", ", flags));
		}

		criteria += fmt::format(
			" AND (content_flags IS NULL{})",
			flags_in_filter
		);

		return std::string(criteria);
	};
}

#endif //EQEMU_CONTENT_FILTER_CRITERIA_H
