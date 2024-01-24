#include "../client.h"
#include "../bot_command.h"

void bot_command_appearance(Client *c, const Seperator *sep)
{

	std::list<const char*> subcommand_list;
	subcommand_list.push_back("botbeardcolor");
	subcommand_list.push_back("botbeardstyle");
	subcommand_list.push_back("botdetails");
	subcommand_list.push_back("boteyes");
	subcommand_list.push_back("botface");
	subcommand_list.push_back("bothaircolor");
	subcommand_list.push_back("bothairstyle");
	subcommand_list.push_back("botheritage");
	subcommand_list.push_back("bottattoo");
	subcommand_list.push_back("botwoad");

	if (helper_command_alias_fail(c, "bot_command_appearance", sep->arg[0], "botappearance"))
		return;

	helper_send_available_subcommands(c, "bot appearance", subcommand_list);
}

void bot_command_beard_color(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_beard_color", sep->arg[0], "botbeardcolor"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Dwarves or male bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetGender() != Gender::Male && my_bot->GetRace() != DWARF)
		fail_type = BCEnum::AFT_GenderRace;
	else if (!PlayerAppearance::IsValidBeardColor(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetBeardColor(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "beard color"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_beard_style(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_beard_style", sep->arg[0], "botbeardstyle"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Dwarves or male bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetGender() != Gender::Male && my_bot->GetRace() != DWARF)
		fail_type = BCEnum::AFT_GenderRace;
	else if (!PlayerAppearance::IsValidBeard(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetBeard(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "beard style"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_details(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	if (helper_command_alias_fail(c, "bot_command_details", sep->arg[0], "botdetails"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != DRAKKIN)
		fail_type = BCEnum::AFT_Race;
	else if (!PlayerAppearance::IsValidDetail(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetDrakkinDetails(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "details"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_eyes(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	// not sure if left/right bias is allowed in pc-type entities (something is keeping them from being different)
	if (helper_command_alias_fail(c, "bot_command_eyes", sep->arg[0], "boteyes"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		//c->Message(Chat::White, "usage: <target_bot> %s [value:0-n] ([option: left | right])", sep->arg[0]);
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	//uint8 eye_bias = 0;
	//std::string arg2 = sep->arg[2];
	//if (!arg2.compare("left"))
	//	eye_bias = 1;
	//else if (!arg2.compare("right"))
	//	eye_bias = 2;

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidEyeColor(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = BCEnum::AFT_Value;
	}
	else {
		//if (eye_bias == 1) {
		//	my_bot->SetEyeColor1(uvalue);
		//}
		//else if (eye_bias == 2) {
		//	my_bot->SetEyeColor2(uvalue);
		//}
		//else {
		my_bot->SetEyeColor1(uvalue);
		my_bot->SetEyeColor2(uvalue);
		//}
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "eyes"))
		return;

	c->Message(Chat::White, "This feature will update the next time your bot is spawned");
	//helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_face(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_face", sep->arg[0], "botface"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidFace(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = BCEnum::AFT_Value;
	}
	else {
		uint8 old_woad = 0;
		if (my_bot->GetRace() == BARBARIAN)
			old_woad = ((my_bot->GetLuclinFace() / 10) * 10);
		my_bot->SetLuclinFace((old_woad + uvalue));
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "face"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_hair_color(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_hair_color", sep->arg[0], "bothaircolor"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidHairColor(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetHairColor(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "hair color"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_hair_style(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_hair_style", sep->arg[0], "bothairstyle"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidHair(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetHairStyle(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "hair style"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_heritage(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	if (helper_command_alias_fail(c, "bot_command_heritage", sep->arg[0], "botheritage"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != DRAKKIN)
		fail_type = BCEnum::AFT_Race;
	else if (!PlayerAppearance::IsValidHeritage(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetDrakkinHeritage(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "heritage"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_tattoo(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	if (helper_command_alias_fail(c, "bot_command_tattoo", sep->arg[0], "bottattoo"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != DRAKKIN)
		fail_type = BCEnum::AFT_Race;
	else if (!PlayerAppearance::IsValidTattoo(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetDrakkinTattoo(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "tattoo"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_command_woad(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_woad", sep->arg[0], "botwoad"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [value: 0-n] (Barbarian bots only)", sep->arg[0]);
		c->Message(Chat::White, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = Strings::ToInt(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != BARBARIAN) {
		fail_type = BCEnum::AFT_Race;
	}
	else if (!PlayerAppearance::IsValidWoad(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = BCEnum::AFT_Value;
	}
	else {
		uint8 old_face = (my_bot->GetLuclinFace() % 10);
		my_bot->SetLuclinFace(((uvalue * 10) + old_face));
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "woad"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}
