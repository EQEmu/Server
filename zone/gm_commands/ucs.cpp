#include "../client.h"

void command_ucs(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	LogInfo("Character [{}] attempting ucs reconnect while ucs server is [{}] available",
			c->GetName(), (zone->IsUCSServerAvailable() ? "" : "un"));

	if (zone->IsUCSServerAvailable()) {
		EQApplicationPacket *outapp = nullptr;
		std::string         buffer;

		std::string              MailKey        = database.GetMailKey(c->CharacterID(), true);
		EQ::versions::UCSVersion ConnectionType = EQ::versions::ucsUnknown;

		// chat server packet
		switch (c->ClientVersion()) {
			case EQ::versions::ClientVersion::Titanium:
				ConnectionType = EQ::versions::ucsTitaniumChat;
				break;
			case EQ::versions::ClientVersion::SoF:
				ConnectionType = EQ::versions::ucsSoFCombined;
				break;
			case EQ::versions::ClientVersion::SoD:
				ConnectionType = EQ::versions::ucsSoDCombined;
				break;
			case EQ::versions::ClientVersion::UF:
				ConnectionType = EQ::versions::ucsUFCombined;
				break;
			case EQ::versions::ClientVersion::RoF:
				ConnectionType = EQ::versions::ucsRoFCombined;
				break;
			case EQ::versions::ClientVersion::RoF2:
				ConnectionType = EQ::versions::ucsRoF2Combined;
				break;
			default:
				ConnectionType = EQ::versions::ucsUnknown;
				break;
		}

		buffer = StringFormat(
			"%s,%i,%s.%s,%c%s",
			Config->ChatHost.c_str(),
			Config->ChatPort,
			Config->ShortName.c_str(),
			c->GetName(),
			ConnectionType,
			MailKey.c_str()
		);

		outapp = new EQApplicationPacket(OP_SetChatServer, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		c->QueuePacket(outapp);
		safe_delete(outapp);

		// mail server packet
		switch (c->ClientVersion()) {
			case EQ::versions::ClientVersion::Titanium:
				ConnectionType = EQ::versions::ucsTitaniumMail;
				break;
			default:
				// retain value from previous switch
				break;
		}

		buffer = StringFormat(
			"%s,%i,%s.%s,%c%s",
			Config->MailHost.c_str(),
			Config->MailPort,
			Config->ShortName.c_str(),
			c->GetName(),
			ConnectionType,
			MailKey.c_str()
		);

		outapp = new EQApplicationPacket(OP_SetChatServer2, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

