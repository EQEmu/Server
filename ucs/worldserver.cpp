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

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

extern WorldServer worldserver;
extern Clientlist *g_Clientlist;
extern const ucsconfig *Config;
extern Database database;

void ProcessMailTo(Client *c, std::string from, std::string subject, std::string message);

void Client45ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink);
void Client50ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink);
void Client55ToServerSayLink(std::string& serverSayLink, const std::string& clientSayLink);

WorldServer::WorldServer()
{
	m_connection.reset(new EQ::Net::ServertalkClient(Config->WorldIP, Config->WorldTCPPort, false, "UCS", Config->SharedKey));
	m_connection->OnMessage(std::bind(&WorldServer::ProcessMessage, this, std::placeholders::_1, std::placeholders::_2));

	m_bsr_timer = nullptr;
}

WorldServer::~WorldServer()
{
}

void WorldServer::ProcessMessage(uint16 opcode, EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	Log(Logs::Detail, Logs::UCS_Server, "Received Opcode: %4X", opcode);

	switch (opcode)
	{
	case 0: {
		break;
	}
	case ServerOP_KeepAlive:
	{
		break;
	}
	case ServerOP_UCSMessage:
	{
		char *Buffer = (char *)pack->pBuffer;

		auto From = new char[strlen(Buffer) + 1];

		VARSTRUCT_DECODE_STRING(From, Buffer);

		std::string Message = Buffer;

		Log(Logs::Detail, Logs::UCS_Server, "Player: %s, Sent Message: %s", From, Message.c_str());

		Client *c = g_Clientlist->FindCharacter(From);

		safe_delete_array(From);

		if (Message.length() < 2)
			break;

		if (!c)
		{
			Log(Logs::Detail, Logs::UCS_Server, "Client not found.");
			break;
		}

		if (Message[0] == ';')
		{
			std::string new_message;
			switch (c->GetClientVersion()) {
			case EQEmu::versions::ClientVersion::Titanium:
				Client45ToServerSayLink(new_message, Message.substr(1, std::string::npos));
				break;
			case EQEmu::versions::ClientVersion::SoF:
			case EQEmu::versions::ClientVersion::SoD:
			case EQEmu::versions::ClientVersion::UF:
				Client50ToServerSayLink(new_message, Message.substr(1, std::string::npos));
				break;
			case EQEmu::versions::ClientVersion::RoF:
				Client55ToServerSayLink(new_message, Message.substr(1, std::string::npos));
				break;
			case EQEmu::versions::ClientVersion::RoF2:
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

	case ServerOP_UCSClientVersionReply:
	{
		UCSClientVersionReply_Struct* cvr = (UCSClientVersionReply_Struct*)pack->pBuffer;
		g_Clientlist->QueueClientVersionReply(cvr->character_id, cvr->client_version);
		break;
	}
	}
}

void WorldServer::ProcessClientVersionRequests(std::list<uint32>& id_list) {
	UCSClientVersionRequest_Struct cvr;
	EQ::Net::DynamicPacket dp_cvr;
	for (auto iter : id_list) {
		cvr.character_id = iter;
		dp_cvr.PutData(0, &cvr, sizeof(cvr));
		m_connection->Send(ServerOP_UCSClientVersionRequest, dp_cvr);
	}
	id_list.clear();
}

void WorldServer::ProcessBroadcastServerReady() {
	if (m_bsr_timer && (*m_bsr_timer) <= Timer::GetCurrentTime()) {
		UCSBroadcastServerReady_Struct bsr;
		memset(&bsr, 0, sizeof(UCSBroadcastServerReady_Struct));

		sprintf(bsr.chat_prefix, "%s,%i,%s.",
			Config->ChatHost.c_str(),
			Config->ChatPort,
			Config->ShortName.c_str()
		);
		sprintf(bsr.mail_prefix, "%s,%i,%s.",
			Config->ChatHost.c_str(),
			Config->MailPort,
			Config->ShortName.c_str()
		);

		EQ::Net::DynamicPacket dp_bsr;
		dp_bsr.PutData(0, &bsr, sizeof(UCSBroadcastServerReady_Struct));
		m_connection->Send(ServerOP_UCSBroadcastServerReady, dp_bsr);

		safe_delete(m_bsr_timer);
	}
}

void WorldServer::ActivateBroadcastServerReadyTimer() {
	safe_delete(m_bsr_timer);
	m_bsr_timer = new uint32;

	// clients do not drop their connection to ucs immediately...
	// it can take upwards of 60 seconds to process the drop
	// and clients will not re-connect to ucs until that occurs
	*m_bsr_timer = (Timer::GetCurrentTime() + (RuleI(Chat, UCSBroadcastServerReadyDelay) * 1000));
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
