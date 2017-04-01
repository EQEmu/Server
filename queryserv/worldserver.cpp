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
#include "../common/md5.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "../common/servertalk.h"

#include "database.h"
#include "lfguild.h"
#include "queryservconfig.h"
#include "worldserver.h"
#include <iomanip>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern WorldServer worldserver;
extern const queryservconfig *Config;
extern Database database;
extern LFGuildManager lfguildmanager;

WorldServer::WorldServer()
: WorldConnection(EmuTCPConnection::packetModeQueryServ, Config->SharedKey.c_str())
{
	pTryReconnect = true;
}

WorldServer::~WorldServer()
{
}

void WorldServer::OnConnected()
{
	Log(Logs::Detail, Logs::QS_Server, "Connected to World.");
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
		Log(Logs::Detail, Logs::QS_Server, "Received Opcode: %4X", pack->opcode); 
		switch(pack->opcode) {
			case 0: {
				break;
			}
			case ServerOP_KeepAlive: {
				break;
			}
			case ServerOP_Speech: {
				Server_Speech_Struct *SSS = (Server_Speech_Struct*)pack->pBuffer; 
				std::string tmp1 = SSS->from;
				std::string tmp2 = SSS->to; 
				database.AddSpeech(tmp1.c_str(), tmp2.c_str(), SSS->message, SSS->minstatus, SSS->guilddbid, SSS->type);
				break;
			}
			case ServerOP_QSPlayerLogTrades: {
				QSPlayerLogTrade_Struct *QS = (QSPlayerLogTrade_Struct*)pack->pBuffer;
				database.LogPlayerTrade(QS, QS->_detail_count);
				break;
			}
			case ServerOP_QSPlayerLogHandins: {
				QSPlayerLogHandin_Struct *QS = (QSPlayerLogHandin_Struct*)pack->pBuffer;
				database.LogPlayerHandin(QS, QS->_detail_count);
				break;
			}
			case ServerOP_QSPlayerLogNPCKills: {
				QSPlayerLogNPCKill_Struct *QS = (QSPlayerLogNPCKill_Struct*)pack->pBuffer;
				uint32 Members = pack->size - sizeof(QSPlayerLogNPCKill_Struct);
				if (Members > 0) Members = Members / sizeof(QSPlayerLogNPCKillsPlayers_Struct);
				database.LogPlayerNPCKill(QS, Members);
				break;
			}
			case ServerOP_QSPlayerLogDeletes: {
				QSPlayerLogDelete_Struct *QS = (QSPlayerLogDelete_Struct*)pack->pBuffer;
				uint32 Items = QS->char_count;
				database.LogPlayerDelete(QS, Items);
				break;
			}
			case ServerOP_QSPlayerLogMoves: {
				QSPlayerLogMove_Struct *QS = (QSPlayerLogMove_Struct*)pack->pBuffer;
				uint32 Items = QS->char_count;
				database.LogPlayerMove(QS, Items);
				break;
			}
			case ServerOP_QSPlayerLogMerchantTransactions: {
				QSMerchantLogTransaction_Struct *QS = (QSMerchantLogTransaction_Struct*)pack->pBuffer;
				uint32 Items = QS->char_count + QS->merchant_count;
				database.LogMerchantTransaction(QS, Items);
				break; 
			}
			case ServerOP_QueryServGeneric: {
				/* 
					The purpose of ServerOP_QueryServerGeneric is so that we don't have to add code to world just to relay packets
					each time we add functionality to queryserv.
				
					A ServerOP_QueryServGeneric packet has the following format:
				
					uint32 SourceZoneID
					uint32 SourceInstanceID
					char OriginatingCharacterName[0] 
						- Null terminated name of the character this packet came from. This could be just
						- an empty string if it has no meaning in the context of a particular packet.
					uint32 Type
				
					The 'Type' field is a 'sub-opcode'. A value of 0 is used for the LFGuild packets. The next feature to be added
					to queryserv would use 1, etc.
				
					Obviously, any fields in the packet following the 'Type' will be unique to the particular type of packet. The
					'Generic' in the name of this ServerOP code relates to the four header fields.
				*/

				char From[64];
				pack->SetReadPosition(8);
				pack->ReadString(From);
				uint32 Type = pack->ReadUInt32();

				switch(Type) {
					case QSG_LFGuild:{
						lfguildmanager.HandlePacket(pack); 
						break;
					}
					default:
						Log(Logs::Detail, Logs::QS_Server, "Received unhandled ServerOP_QueryServGeneric", Type);
						break;
				}
				break;
			}
			case ServerOP_QSSendQuery: {
				/* Process all packets here */
				database.GeneralQueryReceive(pack);  
				break;
			}
		}
	} 
	safe_delete(pack);
	return;
}

