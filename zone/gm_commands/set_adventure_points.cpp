#include "../client.h"
#include "../../common/data_verification.h"

void command_set_adventure_points(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	
	if (
		!arguments ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2)
	) {
		c->Message(Chat::White, "Usage: #setadventurepoints [Theme] [Points]");
		c->Message(Chat::White, "Valid themes are as follows.");
		auto theme_map = EQ::constants::GetLDoNThemeMap();
		for (const auto& theme : theme_map) {
			c->Message(
				Chat::White,
				fmt::format(
					"Theme {} | {}",
					theme.first,
					theme.second
				).c_str()
			);
		}
		c->Message(Chat::White, "Note: Theme 0 splits the points evenly across all Themes.");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto theme_id = std::stoul(sep->arg[1]);
	if (!EQ::ValueWithin(theme_id, LDoNThemes::Unused, LDoNThemes::TAK)) {
		c->Message(Chat::White, "Valid themes are as follows.");
		auto theme_map = EQ::constants::GetLDoNThemeMap();
		for (const auto& theme : theme_map) {
			c->Message(
				Chat::White,
				fmt::format(
					"Theme {} | {}",
					theme.first,
					theme.second
				).c_str()
			);
		}		
		c->Message(Chat::White, "Note: Theme 0 splits the points evenly across all Themes.");
		return;
	}

	auto points = std::stoi(sep->arg[2]);

	c->Message(
		Chat::White,
		fmt::format(
			"{} for {}.",
			(
				theme_id == LDoNThemes::Unused ?
				fmt::format(
					"Splitting {} Points Evenly",
					points
				) :
				fmt::format(
					"Adding {} {} Points",
					points,
					EQ::constants::GetLDoNThemeName(theme_id)
				)
			),
			c->GetTargetDescription(target)
		).c_str()
	);

	target->UpdateLDoNPoints(theme_id, points);
}
