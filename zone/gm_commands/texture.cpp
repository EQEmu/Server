#include "../client.h"

void command_texture(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #texture [Texture] [Helmet Texture]");
		return;
	}

	auto texture = static_cast<uint16>(std::min(std::stoul(sep->arg[1]), (unsigned long) 65535));
	auto helmet_texture = static_cast<uint8>(
		sep->IsNumber(2) ?
		std::min(std::stoul(sep->arg[2]), (unsigned long) 255) :
		0
	);

	Mob* target = c;
	if (c->GetTarget() && c->Admin() >= commandTextureOthers) {
		target = c->GetTarget();
	}

	if (Mob::IsPlayerRace(target->GetModel())) { // Player Races Wear Armor, so Wearchange is sent instead
		for (
			int texture_slot = EQ::textures::textureBegin;
			texture_slot <= EQ::textures::LastTintableTexture;
			texture_slot++
		) {
			target->SendTextureWC(texture_slot, texture);
		}
	} else { // Non-Player Races only need Illusion Packets to be sent for texture
		target->SendIllusionPacket(
			target->GetModel(),
			target->GetGender(),
			texture,
			helmet_texture
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Texture Changed for {} | Texture: {}{}",
			c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
			texture,
			(
				Mob::IsPlayerRace(target->GetModel()) ?
				"" :
				fmt::format(
					" Helmet Texture: {}",
					helmet_texture
				)
			)
		).c_str()
	);
}

