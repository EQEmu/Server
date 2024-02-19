#ifndef EQEMU_CONTENT_FILTER_CRITERIA_H
#define EQEMU_CONTENT_FILTER_CRITERIA_H

#include <string>
#include "../../content/world_content_service.h"
#include "../../strings.h"

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
				Strings::Implode("|", flags_enabled)
			);
		}
		if (!flags_disabled.empty()) {
			flags_in_filter_disabled = fmt::format(
				" OR CONCAT(',', {}content_flags_disabled, ',') REGEXP ',({}),' ",
				table_prefix,
				Strings::Implode("|", flags_disabled)
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
