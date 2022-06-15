/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/servertalk.h"
#include "../common/misc_functions.h"
#include "../common/packet_functions.h"
#include "../common/md5.h"
#include "../common/string_util.h"
#include "worldserver.h"
#include "clientlist.h"
#include "ucsconfig.h"
#include "database.h"
#include "../common/discord_manager.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

extern WorldServer     worldserver;
extern Clientlist      *g_Clientlist;
extern const ucsconfig *Config;
extern Database        database;
extern DiscordManager  discord_manager;

void ProcessMailTo(Client *c, std::string from, std::string subject, std::string message);

void Client45ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink);
void Client50ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink);
void Client55ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink);

WorldServer::WorldServer()
{
	m_connection = std::make_unique<EQ::Net::ServertalkClient>(Config->WorldIP, Config->WorldTCPPort, false, "UCS", Config->SharedKey);
	m_connection->OnMessage(std::bind(&WorldServer::ProcessMessage, this, std::placeholders::_1, std::placeholders::_2));
}

WorldServer::~WorldServer()
{
}

void WorldServer::ProcessMessage(uint16 opcode, EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	LogNetcode("Received Opcode: {:#04x}", opcode);

	switch (opcode)
	{
	case 0: {
		break;
	}
	case ServerOP_KeepAlive:
	{
		break;
	}
	case ServerOP_DiscordWebhookMessage: {
		auto *q = (DiscordWebhookMessage_Struct *) p.Data();

		discord_manager.QueueWebhookMessage(
			q->webhook_id,
			q->message
		);

		break;
	}
	case ServerOP_UCSMessage:
	{
		char *Buffer = (char *)pack->pBuffer;

		auto From = new char[strlen(Buffer) + 1];

		VARSTRUCT_DECODE_STRING(From, Buffer);

		std::string Message = Buffer;

		LogInfo("Player: [{}], Sent Message: [{}]", From, Message.c_str());

		Client *c = g_Clientlist->FindCharacter(From);

		safe_delete_array(From);

		if (Message.length() < 2)
			break;

		if (!c)
		{
			LogInfo("Client not found");
			break;
		}

		if (Message[0] == ';')
		{
			std::string new_message;
			switch (c->GetClientVersion()) {
			case EQ::versions::ClientVersion::Titanium:
				Client45ToServerSayLink(new_message, Message.substr(1, std::string::npos));
				break;
			case EQ::versions::ClientVersion::SoF:
			case EQ::versions::ClientVersion::SoD:
			case EQ::versions::ClientVersion::UF:
				Client50ToServerSayLink(new_message, Message.substr(1, std::string::npos));
				break;
			case EQ::versions::ClientVersion::RoF:
				Client55ToServerSayLink(new_message, Message.substr(1, std::string::npos));
				break;
			case EQ::versions::ClientVersion::RoF2:
			default:
				new_message = Message.substr(1, std::string::npos);
				break;
			}

			c->SendChannelMessageByNumber(new_message);
		}
		else if (Message[0] == '[')
		{
			g_Clientlist->ProcessOPMailCommand(c, Message.substr(1, std::string::npos));
		}

		break;
	}

	case ServerOP_UCSMailMessage:
	{
		ServerMailMessageHeader_Struct *mail = (ServerMailMessageHeader_Struct*)pack->pBuffer;
		database.SendMail(std::string("SOE.EQ.") + Config->ShortName + std::string(".") + std::string(mail->to),
			std::string(mail->from),
			mail->subject,
			mail->message,
			std::string());
		break;
	}
	}
}

void Client45ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink) {
	if (clientSayLink.find('\x12') == std::string::npos) {
		serverSayLink = clientSayLink;
		return;
	}

	auto segments = SplitString(clientSayLink, '\x12');

	for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
		if (segment_iter & 1) {
			if (segments[segment_iter].length() <= 45) {
				serverSayLink.append(segments[segment_iter]);
				// TODO: log size mismatch error
				continue;
			}

			// Idx:  0 1     6     11    16    21    26          31 32    36       37       (Source)
			// 6.2:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X        XXXXXXXX (45)
			// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX  XXXXX XXXXXXXX (56)
			// Diff:                                       ^^^^^         ^   ^^^^^

			serverSayLink.push_back('\x12');
			serverSayLink.append(segments[segment_iter].substr(0, 31));
			serverSayLink.append("00000");
			serverSayLink.append(segments[segment_iter].substr(31, 5));
			serverSayLink.push_back('0');
			serverSayLink.push_back(segments[segment_iter][36]);
			serverSayLink.append("00000");
			serverSayLink.append(segments[segment_iter].substr(37));
			serverSayLink.push_back('\x12');
		}
		else {
			serverSayLink.append(segments[segment_iter]);
		}
	}
}

void Client50ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink) {
	if (clientSayLink.find('\x12') == std::string::npos) {
		serverSayLink = clientSayLink;
		return;
	}

	auto segments = SplitString(clientSayLink, '\x12');

	for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
		if (segment_iter & 1) {
			if (segments[segment_iter].length() <= 50) {
				serverSayLink.append(segments[segment_iter]);
				// TODO: log size mismatch error
				continue;
			}

			// Idx:  0 1     6     11    16    21    26          31 32    36 37    42       (Source)
			// SoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X  XXXXX XXXXXXXX (50)
			// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX  XXXXX XXXXXXXX (56)
			// Diff:                                       ^^^^^         ^

			serverSayLink.push_back('\x12');
			serverSayLink.append(segments[segment_iter].substr(0, 31));
			serverSayLink.append("00000");
			serverSayLink.append(segments[segment_iter].substr(31, 5));
			serverSayLink.push_back('0');
			serverSayLink.append(segments[segment_iter].substr(36));
			serverSayLink.push_back('\x12');
		}
		else {
			serverSayLink.append(segments[segment_iter]);
		}
	}
}

void Client55ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink) {
	if (clientSayLink.find('\x12') == std::string::npos) {
		serverSayLink = clientSayLink;
		return;
	}

	auto segments = SplitString(clientSayLink, '\x12');

	for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
		if (segment_iter & 1) {
			if (segments[segment_iter].length() <= 55) {
				serverSayLink.append(segments[segment_iter]);
				// TODO: log size mismatch error
				continue;
			}

			// Idx:  0 1     6     11    16    21    26    31    36 37    41 42    47       (Source)
			// RoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX  X  XXXXX XXXXXXXX (55)
			// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX  XXXXX XXXXXXXX (56)
			// Diff:                                                     ^

			serverSayLink.push_back('\x12');
			serverSayLink.append(segments[segment_iter].substr(0, 41));
			serverSayLink.push_back('0');
			serverSayLink.append(segments[segment_iter].substr(41));
			serverSayLink.push_back('\x12');
		}
		else {
			serverSayLink.append(segments[segment_iter]);
		}
	}
}
