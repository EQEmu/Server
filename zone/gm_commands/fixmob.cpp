#include "../client.h"

void command_fixmob(Client *c, const Seperator *sep)
{
	Mob        *target = c->GetTarget();
	const char *Usage  = "Usage: #fixmob [race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev]";

	if (!sep->arg[1]) {
		c->Message(Chat::White, Usage);
	}
	else if (!target) {
		c->Message(Chat::White, "Error: this command requires a target");
	}
	else {

		uint32 Adjustment = 1;    // Previous or Next
		char   codeMove   = 0;

		if (sep->arg[2]) {
			char *command2 = sep->arg[2];
			codeMove = (command2[0] | 0x20); // First character, lower-cased
			if (codeMove == 'n') {
				Adjustment = 1;
			}
			else if (codeMove == 'p') {
				Adjustment = -1;
			}
		}

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
		uint32 DrakkinDetails  = target->GetDrakkinDetails();

		const char *ChangeType = nullptr; // If it's still nullptr after processing, they didn't send a valid command
		uint32     ChangeSetting;
		char       *command    = sep->arg[1];

		if (strcasecmp(command, "race") == 0) {
			if (Race == 1 && codeMove == 'p') {
				Race = RuleI(NPC, MaxRaceID);
			}
			else if (Race >= RuleI(NPC, MaxRaceID) && codeMove != 'p') {
				Race = 1;
			}
			else {
				Race += Adjustment;
			}
			ChangeType    = "Race";
			ChangeSetting = Race;
		}
		else if (strcasecmp(command, "gender") == 0) {
			if (Gender == 0 && codeMove == 'p') {
				Gender = 2;
			}
			else if (Gender >= 2 && codeMove != 'p') {
				Gender = 0;
			}
			else {
				Gender += Adjustment;
			}
			ChangeType    = "Gender";
			ChangeSetting = Gender;
		}
		else if (strcasecmp(command, "texture") == 0) {
			Texture       = target->GetTexture();

			if (Texture == 0 && codeMove == 'p') {
				Texture = 25;
			}
			else if (Texture >= 25 && codeMove != 'p') {
				Texture = 0;
			}
			else {
				Texture += Adjustment;
			}
			ChangeType    = "Texture";
			ChangeSetting = Texture;
		}
		else if (strcasecmp(command, "helm") == 0) {
			HelmTexture   = target->GetHelmTexture();
			if (HelmTexture == 0 && codeMove == 'p') {
				HelmTexture = 25;
			}
			else if (HelmTexture >= 25 && codeMove != 'p') {
				HelmTexture = 0;
			}
			else {
				HelmTexture += Adjustment;
			}
			ChangeType    = "HelmTexture";
			ChangeSetting = HelmTexture;
		}
		else if (strcasecmp(command, "face") == 0) {
			if (LuclinFace == 0 && codeMove == 'p') {
				LuclinFace = 87;
			}
			else if (LuclinFace >= 87 && codeMove != 'p') {
				LuclinFace = 0;
			}
			else {
				LuclinFace += Adjustment;
			}
			ChangeType    = "LuclinFace";
			ChangeSetting = LuclinFace;
		}
		else if (strcasecmp(command, "hair") == 0) {
			if (HairStyle == 0 && codeMove == 'p') {
				HairStyle = 8;
			}
			else if (HairStyle >= 8 && codeMove != 'p') {
				HairStyle = 0;
			}
			else {
				HairStyle += Adjustment;
			}
			ChangeType    = "HairStyle";
			ChangeSetting = HairStyle;
		}
		else if (strcasecmp(command, "haircolor") == 0) {
			if (HairColor == 0 && codeMove == 'p') {
				HairColor = 24;
			}
			else if (HairColor >= 24 && codeMove != 'p') {
				HairColor = 0;
			}
			else {
				HairColor += Adjustment;
			}
			ChangeType    = "HairColor";
			ChangeSetting = HairColor;
		}
		else if (strcasecmp(command, "beard") == 0) {
			if (Beard == 0 && codeMove == 'p') {
				Beard = 11;
			}
			else if (Beard >= 11 && codeMove != 'p') {
				Beard = 0;
			}
			else {
				Beard += Adjustment;
			}
			ChangeType    = "Beard";
			ChangeSetting = Beard;
		}
		else if (strcasecmp(command, "beardcolor") == 0) {
			if (BeardColor == 0 && codeMove == 'p') {
				BeardColor = 24;
			}
			else if (BeardColor >= 24 && codeMove != 'p') {
				BeardColor = 0;
			}
			else {
				BeardColor += Adjustment;
			}
			ChangeType    = "BeardColor";
			ChangeSetting = BeardColor;
		}
		else if (strcasecmp(command, "heritage") == 0) {
			if (DrakkinHeritage == 0 && codeMove == 'p') {
				DrakkinHeritage = 6;
			}
			else if (DrakkinHeritage >= 6 && codeMove != 'p') {
				DrakkinHeritage = 0;
			}
			else {
				DrakkinHeritage += Adjustment;
			}
			ChangeType    = "DrakkinHeritage";
			ChangeSetting = DrakkinHeritage;
		}
		else if (strcasecmp(command, "tattoo") == 0) {
			if (DrakkinTattoo == 0 && codeMove == 'p') {
				DrakkinTattoo = 8;
			}
			else if (DrakkinTattoo >= 8 && codeMove != 'p') {
				DrakkinTattoo = 0;
			}
			else {
				DrakkinTattoo += Adjustment;
			}
			ChangeType    = "DrakkinTattoo";
			ChangeSetting = DrakkinTattoo;
		}
		else if (strcasecmp(command, "detail") == 0) {
			if (DrakkinDetails == 0 && codeMove == 'p') {
				DrakkinDetails = 7;
			}
			else if (DrakkinDetails >= 7 && codeMove != 'p') {
				DrakkinDetails = 0;
			}
			else {
				DrakkinDetails += Adjustment;
			}
			ChangeType    = "DrakkinDetails";
			ChangeSetting = DrakkinDetails;
		}

		// Hack to fix some races that base features from face
		switch (Race) {
			case 2:    // Barbarian
				if (LuclinFace > 10) {
					LuclinFace -= ((DrakkinTattoo - 1) * 10);
				}
				LuclinFace += (DrakkinTattoo * 10);
				break;
			case 3: // Erudite
				if (LuclinFace > 10) {
					LuclinFace -= ((HairStyle - 1) * 10);
				}
				LuclinFace += (HairStyle * 10);
				break;
			case 5: // HighElf
			case 6: // DarkElf
			case 7: // HalfElf
				if (LuclinFace > 10) {
					LuclinFace -= ((Beard - 1) * 10);
				}
				LuclinFace += (Beard * 10);
				break;
			default:
				break;
		}


		if (ChangeType == nullptr) {
			c->Message(Chat::White, Usage);
		}
		else {
			target->SendIllusionPacket(
				Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
				EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
				DrakkinHeritage, DrakkinTattoo, DrakkinDetails
			);

			c->Message(Chat::White, "%s=%i", ChangeType, ChangeSetting);
		}
	}
}

