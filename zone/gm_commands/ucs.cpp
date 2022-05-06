#include "../client.h"

void command_ucs(Client *c, const Seperator *sep)
{
	LogInfo(
		"Character [{}] attempting UCS reconnect while UCS server is {}available.",
		c->GetName(),
		zone->IsUCSServerAvailable() ? "" : "un"
	);

	if (!zone->IsUCSServerAvailable()) {
		c->Message(Chat::White, "The UCS is currently unavailable.");
		return;
	}

	EQApplicationPacket *outapp = nullptr;
	std::string buffer;

	std::string mail_key = database.GetMailKey(c->CharacterID(), true);
	EQ::versions::UCSVersion connection_type = EQ::versions::ucsUnknown;

	switch (c->ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			connection_type = EQ::versions::ucsTitaniumChat;
			break;
		case EQ::versions::ClientVersion::SoF:
			connection_type = EQ::versions::ucsSoFCombined;
			break;
		case EQ::versions::ClientVersion::SoD:
			connection_type = EQ::versions::ucsSoDCombined;
			break;
		case EQ::versions::ClientVersion::UF:
			connection_type = EQ::versions::ucsUFCombined;
			break;
		case EQ::versions::ClientVersion::RoF:
			connection_type = EQ::versions::ucsRoFCombined;
			break;
		case EQ::versions::ClientVersion::RoF2:
			connection_type = EQ::versions::ucsRoF2Combined;
			break;
		default:
			connection_type = EQ::versions::ucsUnknown;
			break;
	}

	buffer = fmt::format(
		"{},{},{}.{},{}{}",
		Config->ChatHost,
		Config->ChatPort,
		Config->ShortName,
		c->GetName(),
		connection_type,
		mail_key
	);

	outapp = new EQApplicationPacket(OP_SetChatServer, (buffer.length() + 1));
	memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
	outapp->pBuffer[buffer.length()] = '\0';

	c->QueuePacket(outapp);
	safe_delete(outapp);

	switch (c->ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			connection_type = EQ::versions::ucsTitaniumMail;
			break;
		default:
			break;
	}

	buffer = StringFormat(
		"{},{},{}.{},{}{}",
		Config->MailHost,
		Config->MailPort,
		Config->ShortName,
		c->GetName(),
		connection_type,
		mail_key
	);

	outapp = new EQApplicationPacket(OP_SetChatServer2, (buffer.length() + 1));
	memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
	outapp->pBuffer[buffer.length()] = '\0';

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

