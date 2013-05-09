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
#include "../common/debug.h"
#include <iostream>
using namespace std;
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
extern Clientlist *CL;
extern const ucsconfig *Config;
extern Database database;

void ProcessMailTo(Client *c, string from, string subject, string message);

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
	_log(UCS__INIT, "Connected to World.");
	WorldConnection::OnConnected();
}

void WorldServer::Process()
{
	WorldConnection::Process();

	if (!Connected())
		return;

	ServerPacket *pack = 0;

	while((pack = tcpc.PopPacket()))
	{
		_log(UCS__TRACE, "Received Opcode: %4X", pack->opcode);

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

				char *From = new char[strlen(Buffer) + 1];

				VARSTRUCT_DECODE_STRING(From, Buffer);

				string Message = Buffer;

				_log(UCS__TRACE, "Player: %s, Sent Message: %s", From, Message.c_str());

				Client *c = CL->FindCharacter(From);

				safe_delete_array(From);

				if(Message.length() < 2)
					break;

				if(!c)
				{
					_log(UCS__TRACE, "Client not found.");
					break;
				}

				if(Message[0] == ';')
				{
					c->SendChannelMessageByNumber(Message.substr(1, string::npos));
				}
				else if(Message[0] == '[')
				{
					CL->ProcessOPMailCommand(c, Message.substr(1, string::npos));
				}

				break;
			}

			case ServerOP_UCSMailMessage:
			{
				ServerMailMessageHeader_Struct *mail = (ServerMailMessageHeader_Struct*)pack->pBuffer;
				database.SendMail(string("SOE.EQ.") + Config->ShortName + string(".") + string(mail->to),
					string(mail->from),
					mail->subject,
					mail->message,
					string());
				break;
			}
		}
	}

	safe_delete(pack);
	return;
}

