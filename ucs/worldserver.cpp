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
			c->SendChannelMessageByNumber(Message.substr(1, std::string::npos));
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
