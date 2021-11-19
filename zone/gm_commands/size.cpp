#include "../client.h"

void command_size(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();
	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #size [0 - 255] (Decimal increments are allowed)");
	}
	else {
		float newsize = atof(sep->arg[1]);
		if (newsize > 255) {
			c->Message(Chat::White, "Error: #size: Size can not be greater than 255.");
		}
		else if (newsize < 0) {
			c->Message(Chat::White, "Error: #size: Size can not be less than 0.");
		}
		else if (!target) {
			c->Message(Chat::White, "Error: this command requires a target");
		}
		else {
			uint16 Race            = target->GetModel();
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
			uint32 DrakkinDetails  = target->GetDrakkinDetails();

			target->SendIllusionPacket(
				Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
				EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
				DrakkinHeritage, DrakkinTattoo, DrakkinDetails, newsize
			);

			c->Message(Chat::White, "Size = %f", atof(sep->arg[1]));
		}
	}
}

