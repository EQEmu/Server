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
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../common/servertalk.h"
#include "worldserver.h"
#include "clientlist.h"
#include "ucsconfig.h"
#include "database.h"
#include "../common/packet_functions.h"
#include "../common/md5.h"

extern WorldServer worldserver;
extern Clientlist *g_Clientlist;
extern const ucsconfig *Config;
extern Database database;

void ProcessMailTo(Client *c, std::string from, std::string subject, std::string message);

WorldServer::WorldServer()
: WorldConnection(EmuTCPConnection::packetModeUCS, Config->SharedKey.c_str())
{
	pTryReconnect = true;
}

WorldServer::~WorldServer()
{
}

void WorldServer::OnConnected()
{
	Log.Out(Logs::Detail, Logs::UCS_Server, "Connected to World.");
	WorldConnection::OnConnected();
}

void WorldServer::Process()
{
	WorldConnection::Process();

	if (!Connected())
		return;

	ServerPacket *pack = nullptr;

	while((pack = tcpc.PopPacket()))
	{
		Log.Out(Logs::Detail, Logs::UCS_Server, "Received Opcode: %4X", pack->opcode);

		switch(pack->opcode)
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

				Log.Out(Logs::Detail, Logs::UCS_Server, "Player: %s, Sent Message: %s", From, Message.c_str());

				Client *c = g_Clientlist->FindCharacter(From);

				safe_delete_array(From);

				if(Message.length() < 2)
					break;

				if(!c)
				{
					Log.Out(Logs::Detail, Logs::UCS_Server, "Client not found.");
					break;
				}

				if(Message[0] == ';')
				{
					c->SendChannelMessageByNumber(Message.substr(1, std::string::npos));
				}
				else if(Message[0] == '[')
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

	safe_delete(pack);
	return;
}

