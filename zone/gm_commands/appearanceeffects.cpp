#include "../client.h"

void command_appearanceeffects(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #appearanceeffects help - Display appearance effects help menu");
		c->Message(Chat::White, "Usage: #appearanceeffects remove - Remove all appearance effects saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects set [Effect ID] [Slot ID] - Set an appearance effect saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects view - Display all appearance effects saved to your target");
		return;
	}

	const bool is_help   = !strcasecmp(sep->arg[1], "help");
	const bool is_remove = !strcasecmp(sep->arg[1], "remove");
	const bool is_set    = !strcasecmp(sep->arg[1], "set");
	const bool is_view   = !strcasecmp(sep->arg[1], "view");

	if (is_help) {
		c->Message(Chat::White, "Usage: #appearanceeffects help - Display appearance effects help menu");
		c->Message(Chat::White, "Usage: #appearanceeffects remove - Remove all appearance effects saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects set [Effect ID] [Slot ID] - Set an appearance effect saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects view - Display all appearance effects saved to your target");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (is_set) {
		if (arguments != 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
			c->Message(Chat::White, "Usage: #appearanceeffects set [Effect ID] [Slot ID] - Set an appearance effect saved to your target");
			return;
		}

		const auto effect_id = Strings::ToUnsignedInt(sep->arg[2]);
		const auto slot_id   = Strings::ToUnsignedInt(sep->arg[3]);

		t->SendAppearanceEffect(
			effect_id,
			0,
			0,
			0,
			0,
			nullptr,
			slot_id,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Appearance Effect ID {} in slot ID {} has been set for {}.",
				effect_id,
				slot_id,
				c->GetTargetDescription(t)
			).c_str()
		);
	} else if (is_remove) {
		t->SendIllusionPacket(
			t->GetRace(),
			t->GetGender(),
			t->GetTexture(),
			t->GetHelmTexture(),
			t->GetHairColor(),
			t->GetBeardColor(),
			t->GetEyeColor1(),
			t->GetEyeColor2(),
			t->GetHairStyle(),
			t->GetLuclinFace(),
			t->GetBeard(),
			0xFF,
			t->GetDrakkinHeritage(),
			t->GetDrakkinTattoo(),
			t->GetDrakkinDetails(),
			t->GetSize(),
			false
		);
		t->ClearAppearenceEffects();
		c->Message(
			Chat::White,
			fmt::format(
				"Appearance Effects have been removed for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);
	} else if (is_view) {
		t->ListAppearanceEffects(c);
	}
}
