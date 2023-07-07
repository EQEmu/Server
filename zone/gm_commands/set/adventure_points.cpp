#include "../../client.h"
#include "../../../common/data_verification.h"

void SetAdventurePoints(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set adventure_points [Theme] [Points]");

		c->Message(Chat::White, "Valid themes are as follows:");

		for (const auto& e : EQ::constants::GetLDoNThemeMap()) {
			if (e.first != LDoNThemes::Unused) {
				c->Message(
					Chat::White,
					fmt::format(
						"Theme {} | {}",
						e.first,
						e.second
					).c_str()
				);
			}
		}

		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 theme_id = Strings::ToUnsignedInt(sep->arg[2]);
	const uint32 points = Strings::ToUnsignedInt(sep->arg[3]);

	if (!EQ::ValueWithin(theme_id, LDoNThemes::GUK, LDoNThemes::TAK)) {
		c->Message(Chat::White, "Valid themes are as follows:");

		for (const auto& e : EQ::constants::GetLDoNThemeMap()) {
			if (e.first != LDoNThemes::Unused) {
				c->Message(
					Chat::White,
					fmt::format(
						"Theme {} | {}",
						e.first,
						e.second
					).c_str()
				);
			}
		}

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} Points to {} for {}.",
			EQ::constants::GetLDoNThemeName(theme_id),
			Strings::Commify(points),
			c->GetTargetDescription(t)
		).c_str()
	);

	t->SetLDoNPoints(theme_id, points);
}
