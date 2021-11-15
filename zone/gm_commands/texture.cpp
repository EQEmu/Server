#include "../client.h"

void command_texture(Client *c, const Seperator *sep)
{

	uint16 texture;

	if (sep->IsNumber(1) && atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 255) {
		texture = atoi(sep->arg[1]);
		uint8 helm = 0xFF;

		// Player Races Wear Armor, so Wearchange is sent instead
		int i;
		if (!c->GetTarget()) {
			for (i = EQ::textures::textureBegin; i <= EQ::textures::LastTintableTexture; i++) {
				c->SendTextureWC(i, texture);
			}
		}
		else if ((c->GetTarget()->GetModel() > 0 && c->GetTarget()->GetModel() <= 12) ||
				 c->GetTarget()->GetModel() == 128 || c->GetTarget()->GetModel() == 130 ||
				 c->GetTarget()->GetModel() == 330 || c->GetTarget()->GetModel() == 522) {
			for (i = EQ::textures::textureBegin; i <= EQ::textures::LastTintableTexture; i++) {
				c->GetTarget()->SendTextureWC(i, texture);
			}
		}
		else    // Non-Player Races only need Illusion Packets to be sent for texture
		{
			if (sep->IsNumber(2) && atoi(sep->arg[2]) >= 0 && atoi(sep->arg[2]) <= 255) {
				helm = atoi(sep->arg[2]);
			}
			else {
				helm = texture;
			}

			if (texture == 255) {
				texture = 0xFFFF;    // Should be pulling these from the database instead
				helm    = 0xFF;
			}

			if ((c->GetTarget()) && (c->Admin() >= commandTextureOthers)) {
				c->GetTarget()->SendIllusionPacket(c->GetTarget()->GetModel(), 0xFF, texture, helm);
			}
			else {
				c->SendIllusionPacket(c->GetRace(), 0xFF, texture, helm);
			}
		}
	}
	else {
		c->Message(Chat::White, "Usage: #texture [texture] [helmtexture] (0-255, 255 for show equipment)");
	}
}

