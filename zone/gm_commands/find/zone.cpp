#include "../../client.h"
#include "../../common/content/world_content_service.h"

void FindZone(Client *c, const Seperator *sep)
{
	std::string query = "SELECT zoneidnumber, short_name, long_name, version FROM zone WHERE ";

	const auto is_expansion_search  = !strcasecmp(sep->arg[2], "expansion");
	const auto is_id_search         = Strings::IsNumber(sep->arg[2]);
	const auto is_short_name_search = !is_expansion_search && !is_id_search;

	std::string search_string;
	std::string search_type;

	if (is_expansion_search) {
		query += fmt::format(
			"expansion = {}",
			Strings::ToInt(sep->arg[3])
		);

		search_string = Expansion::ExpansionName[Strings::ToInt(sep->arg[3])];
		search_type   = "Expansion";
	} else if (is_id_search) {
		query += fmt::format(
			"zoneidnumber = {}",
			Strings::ToUnsignedInt(sep->arg[2])
		);

		search_string = sep->arg[2];
		search_type   = "Expansion";
	} else if (is_short_name_search) {
		query += fmt::format(
			"LOWER(`long_name`) LIKE '%%{}%%' OR LOWER(`short_name`) LIKE '%%{}%%'",
			Strings::Escape(Strings::ToLower(sep->argplus[2])),
			Strings::Escape(Strings::ToLower(sep->argplus[2]))
		);

		search_string = sep->argplus[2];
		search_type   = "Expansion";
	}

	query += " ORDER BY `zoneidnumber` ASC LIMIT 50";

	auto results = content_db.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		c->Message(Chat::White, "No zones were found matching your search criteria.");
		c->Message(Chat::White, query.c_str());
		return;
	}

	auto found_count = 0;

	for (auto row : results) {
		const auto zone_id            = Strings::ToUnsignedInt(row[0]);
		const std::string& short_name = row[1];
		const std::string& long_name  = row[2];
		const auto version            = Strings::ToInt(row[3]);

		c->Message(
			Chat::White,
			fmt::format(
				"{}{} {} ({}) (ID {}){}",
				(
					version == 0 ?
					fmt::format(
						"{} | ",
						Saylink::Silent(
							fmt::format(
								"#zone {}",
								short_name
							),
							"Zone"
						)
					) :
					""
				),
				fmt::format(
					"{} |",
					Saylink::Silent(
						fmt::format(
							"#gmzone {} {}",
							short_name,
							version
						),
						"GM Zone"
					)
				),
				long_name,
				short_name,
				zone_id,
				(
					version != 0 ?
					fmt::format(
						" (Version {})",
						version
					) :
					""
				)
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Zones found matching '{}' of '{}'.",
				search_type,
				search_string
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Zone{} found matching '{}' of '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			search_type,
			search_string
		).c_str()
	);
}
