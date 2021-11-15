#include "../client.h"

void command_details(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();
	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #details [number of drakkin detail]");
	}
	else if (!target) {
		c->Message(Chat::White, "Error: this command requires a target");
	}
	else {
		uint16 Race            = target->GetRace();
		uint8  Gender          = target->GetGender();
		uint8  Texture         = 0xFF;
		uint8  HelmTexture     = 0xFF;
		uint8  HairColor       = target->GetHairColor();
		uint8  BeardColor      = target->GetBeardColor();
		uint8  EyeColor1       = target->GetEyeColor1();
		uint8  EyeColor2       = target->GetEyeColor2();
		uint8  HairStyle       = target->GetHairStyle();
		uint8  LuclinFace      = target->GetLuclinFace();
		uint8  Beard           = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo   = target->GetDrakkinTattoo();
		uint32 DrakkinDetails  = atoi(sep->arg[1]);

		target->SendIllusionPacket(
			Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
			EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
			DrakkinHeritage, DrakkinTattoo, DrakkinDetails
		);

		c->Message(Chat::White, "Details = %i", atoi(sep->arg[1]));
	}
}

