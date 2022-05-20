#include "../client.h"

void command_feature(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		auto feature_save_link = EQ::SayLinkEngine::GenerateQuestSaylink(
			"#npcedit featuresave",
			false,
			"#npcedit featuresave"
		);

		c->Message(Chat::White, "Usage: #feature beard [Beard] - Change your or your target's Beard");
		c->Message(Chat::White, "Usage: #feature beardcolor [Beard Color] - Change your or your target's Beard Color");
		c->Message(Chat::White, "Usage: #feature details [Details] - Change your or your target's Drakkin Details");
		c->Message(Chat::White, "Usage: #feature eyes [Eye Color] - Change your or your target's Eyes");
		c->Message(Chat::White, "Usage: #feature face [Face] - Change your or your target's Face");
		c->Message(Chat::White, "Usage: #feature gender [Gender] - Change your or your target's Gender");
		c->Message(Chat::White, "Usage: #feature hair [Hair] - Change your or your target's Hair");
		c->Message(Chat::White, "Usage: #feature haircolor [Hair Color] - Change your or your target's Hair Color");
		c->Message(Chat::White, "Usage: #feature helm [Helmet Texture] - Change your or your target's Helmet Texture");
		c->Message(Chat::White, "Usage: #feature heritage [Heritage] - Change your or your target's Drakkin Heritage");
		c->Message(Chat::White, "Usage: #feature race [Race ID] - Change your or your target's Race");
		c->Message(Chat::White, "Usage: #feature size [Size] - Change your or your target's Size (Valid values are 0 to 255, decimal increments are allowed.)");
		c->Message(Chat::White, "Usage: #feature tattoo [Tattoo] - Change your or your target's Drakkin Tattoos");
		c->Message(Chat::White, "Usage: #feature texture [Texture] - Change your or your target's Texture");
		c->Message(
			Chat::White,
			fmt::format(
				"Note: All features are temporary. If your target is an NPC, you can save these features to the database using {}.",
				feature_save_link
			).c_str()
		);
		return;
	}

	Mob* target = c->GetTarget() ? c->GetTarget() : c;

	bool is_beard = !strcasecmp(sep->arg[1], "beard");
	bool is_beard_color = !strcasecmp(sep->arg[1], "beardcolor");
	bool is_details = !strcasecmp(sep->arg[1], "details");
	bool is_eyes = !strcasecmp(sep->arg[1], "eyes");
	bool is_face = !strcasecmp(sep->arg[1], "face");
	bool is_gender = !strcasecmp(sep->arg[1], "gender");
	bool is_hair = !strcasecmp(sep->arg[1], "hair");
	bool is_hair_color = !strcasecmp(sep->arg[1], "haircolor");
	bool is_helm = !strcasecmp(sep->arg[1], "helm");
	bool is_heritage = !strcasecmp(sep->arg[1], "heritage");
	bool is_race = !strcasecmp(sep->arg[1], "race");
	bool is_size = !strcasecmp(sep->arg[1], "size");
	bool is_tattoo = !strcasecmp(sep->arg[1], "tattoo");
	bool is_texture = !strcasecmp(sep->arg[1], "texture");

	if (
		!is_beard &&
		!is_beard_color &&
		!is_details &&
		!is_eyes &&
		!is_face &&
		!is_gender &&
		!is_hair &&
		!is_hair_color &&
		!is_helm &&
		!is_heritage &&
		!is_race &&
		!is_size &&
		!is_tattoo &&
		!is_texture
	) {
		auto feature_save_link = EQ::SayLinkEngine::GenerateQuestSaylink(
			"#npcedit featuresave",
			false,
			"#npcedit featuresave"
		);

		c->Message(Chat::White, "Usage: #feature beard [Beard] - Change your or your target's Beard");
		c->Message(Chat::White, "Usage: #feature beardcolor [Beard Color] - Change your or your target's Beard Color");
		c->Message(Chat::White, "Usage: #feature details [Details] - Change your or your target's Drakkin Details");
		c->Message(Chat::White, "Usage: #feature eyes [Eye Color] - Change your or your target's Eyes");
		c->Message(Chat::White, "Usage: #feature face [Face] - Change your or your target's Face");
		c->Message(Chat::White, "Usage: #feature gender [Gender] - Change your or your target's Gender");
		c->Message(Chat::White, "Usage: #feature hair [Hair] - Change your or your target's Hair");
		c->Message(Chat::White, "Usage: #feature haircolor [Hair Color] - Change your or your target's Hair Color");
		c->Message(Chat::White, "Usage: #feature helm [Helmet Texture] - Change your or your target's Helmet Texture");
		c->Message(Chat::White, "Usage: #feature heritage [Heritage] - Change your or your target's Drakkin Heritage");
		c->Message(Chat::White, "Usage: #feature race [Race ID] - Change your or your target's Race");
		c->Message(Chat::White, "Usage: #feature size [Size] - Change your or your target's Size (Valid values are 0 to 255, decimal increments are allowed.)");
		c->Message(Chat::White, "Usage: #feature tattoo [Tattoo] - Change your or your target's Drakkin Tattoos");
		c->Message(Chat::White, "Usage: #feature texture [Texture] - Change your or your target's Texture");
		c->Message(
			Chat::White,
			fmt::format(
				"Note: All features are temporary. If your target is an NPC, you can save these features to the database using {}.",
				feature_save_link
			).c_str()
		);
		return;
	}

	FaceChange_Struct face{};
	face.haircolor        = target->GetHairColor();
	face.beardcolor       = target->GetBeardColor();
	face.eyecolor1        = target->GetEyeColor1();
	face.eyecolor2        = target->GetEyeColor2();
	face.hairstyle        = target->GetHairStyle();
	face.face             = target->GetLuclinFace();
	face.beard            = target->GetBeard();
	face.drakkin_heritage = target->GetDrakkinHeritage();
	face.drakkin_tattoo   = target->GetDrakkinTattoo();
	face.drakkin_details  = target->GetDrakkinDetails();

	auto gender = target->GetGender();
	auto helm_texture = target->GetHelmTexture();
	auto race = target->GetModel();
	auto size = target->GetSize();
	auto texture = target->GetTexture();

	std::string feature_changed;
	float value_changed = 0.0f;

	if (is_beard) {
		face.beard = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Beard";
		value_changed = face.beard;
	} else if (is_beard_color) {
		face.beardcolor = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Beard Color";
		value_changed = face.beardcolor;
	} else if (is_details) {
		if (target->GetRace() != DRAKKIN) {
			c->Message(Chat::White, "You must target a Drakkin to use this command.");
			return;
		}

		face.drakkin_details = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Drakkin Details";
		value_changed = static_cast<float>(face.drakkin_details);
	} else if (is_eyes) {
		face.eyecolor1 = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Eyes";
		value_changed = face.eyecolor1; // eyecolor2 isn't used
	} else if (is_face) {
		face.face = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Face";
		value_changed = face.face;
	} else if (is_gender) {
		gender = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Gender";
		value_changed = gender;
	} else if (is_hair) {
		face.hairstyle = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Hair";
		value_changed = face.hairstyle;
	} else if (is_hair_color) {
		face.haircolor = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Hair Color";
		value_changed = face.haircolor;
	} else if (is_helm) {
		helm_texture = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Helmet Texture";
		value_changed = helm_texture;
	} else if (is_heritage) {
		if (target->GetRace() != DRAKKIN) {
			c->Message(Chat::White, "You must target a Drakkin to use this command.");
			return;
		}

		face.drakkin_heritage = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Drakkin Heritage";
		value_changed = static_cast<float>(face.drakkin_heritage);
	} else if (is_race) {
		race = static_cast<uint16>(std::stoul(sep->arg[2]));
		feature_changed = "Race";
		value_changed = race;
	} else if (is_size) {
		size = std::stof(sep->arg[2]);

		if (size < 0 || size > 255) {
			c->Message(Chat::White, "Usage: #feature size [Size] - Change your or your target's Size temporarily (Valid values are 0 to 255, decimal increments are allowed.)");
			return;
		}

		feature_changed = "Size";
		value_changed = size;
	} else if (is_tattoo) {
		if (target->GetRace() != DRAKKIN) {
			c->Message(Chat::White, "You must target a Drakkin to use this command.");
			return;
		}

		face.drakkin_tattoo = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Drakkin Tattoos";
		value_changed = static_cast<float>(face.drakkin_tattoo);
	} else if (is_texture) {
		texture = static_cast<uint8>(std::stoul(sep->arg[2]));
		feature_changed = "Texture";
		value_changed = texture;
	}

	// For now face number is not set through SetFace. This is because the
	// client may not update face features after being set to an invalid face
	// until a specific valid face number is re-sent (needs more research)
	if (!is_gender && !is_helm && !is_race && !is_size && !is_texture && !is_face)
	{
		target->SetFaceAppearance(face);
	}
	else
	{
		target->SendIllusionPacket(
			race,
			gender,
			texture,
			helm_texture,
			face.haircolor,
			face.beardcolor,
			target->GetEyeColor1(),
			target->GetEyeColor2(),
			face.hairstyle,
			face.face,
			face.beard,
			0xFF,
			face.drakkin_heritage,
			face.drakkin_tattoo,
			face.drakkin_details,
			size
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} set for {} to {}.",
			feature_changed,
			c->GetTargetDescription(target),
			(
				is_size ?
				fmt::format(
					"{:.2f}",
					value_changed
				) :
				fmt::format(
					"{}",
					static_cast<uint8>(value_changed)
				)
			)
		).c_str()
	);
}

