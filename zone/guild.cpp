/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
#include "masterentity.h"
#include "worldserver.h"
#include "net.h"
#include "../common/database.h"
#include "../common/spdat.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/serverinfo.h"
#include "../common/ZoneNumbers.h"
#include "../common/moremath.h"
#include "../common/guilds.h"
#include "../common/StringUtil.h"
#include "guild_mgr.h"
#include "StringIDs.h"
#include "NpcAI.h"

extern WorldServer worldserver;

void Client::SendGuildMOTD(bool GetGuildMOTDReply) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildMOTD, sizeof(GuildMOTD_Struct));

	// When the Client gets an OP_GuildMOTD, it compares the text to the version it has previously stored.
	// If the text in the OP_GuildMOTD packet is the same, it does nothing. If not the same, it displays
	// the new MOTD and then stores the new text.
	//
	// When the Client receives an OP_GetGuildMOTDReply, it displays the text in the packet.
	//
	// So OP_GuildMOTD should be sent on zone entry and when an Officer changes the MOTD, and OP_GetGuildMOTDReply
	// should be sent when the client issues the /getguildmotd command.
	//
	if(GetGuildMOTDReply)
		outapp->SetOpcode(OP_GetGuildMOTDReply);

	GuildMOTD_Struct *motd = (GuildMOTD_Struct *) outapp->pBuffer;
	motd->unknown0 = 0;
	strn0cpy(motd->name, m_pp.name, 64);

	if(IsInAGuild()) {
		if(!guild_mgr.GetGuildMOTD(GuildID(), motd->motd, motd->setby_name)) {
			motd->setby_name[0] = '\0';
			strcpy(motd->motd, "ERROR GETTING MOTD!");
		}
	} else {
		//we have to send them an empty MOTD anywyas.
		motd->motd[0] = '\0';	//just to be sure
		motd->setby_name[0] = '\0';	//just to be sure

	}

	mlog(GUILDS__OUT_PACKETS, "Sending OP_GuildMOTD of length %d", outapp->size);
	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);
}

void Client::SendGuildURL()
{
	if(GetClientVersion() < EQClientSoF)
		return;

	if(IsInAGuild())
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateURLAndChannel_Struct));

		GuildUpdateURLAndChannel_Struct *guuacs = (GuildUpdateURLAndChannel_Struct*) outapp->pBuffer;

		if(guild_mgr.GetGuildURL(GuildID(), guuacs->Text))
		{
			guuacs->Action = 0;
			FastQueuePacket(&outapp);
		}
		else
			safe_delete(outapp);
	}
}

void Client::SendGuildChannel()
{
	if(GetClientVersion() < EQClientSoF)
		return;

	if(IsInAGuild())
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateURLAndChannel_Struct));

		GuildUpdateURLAndChannel_Struct *guuacs = (GuildUpdateURLAndChannel_Struct*) outapp->pBuffer;

		if(guild_mgr.GetGuildChannel(GuildID(), guuacs->Text))
		{
			guuacs->Action = 1;

			FastQueuePacket(&outapp);
		}
		else
			safe_delete(outapp);
	}
}

void Client::SendGuildSpawnAppearance() {
	if (!IsInAGuild()) {
		// clear guildtag
		SendAppearancePacket(AT_GuildID, GUILD_NONE);
		mlog(GUILDS__OUT_PACKETS, "Sending spawn appearance for no guild tag.");
	} else {
		uint8 rank = guild_mgr.GetDisplayedRank(GuildID(), GuildRank(), CharacterID());
		mlog(GUILDS__OUT_PACKETS, "Sending spawn appearance for guild %d at rank %d", GuildID(), rank);
		SendAppearancePacket(AT_GuildID, GuildID());
		SendAppearancePacket(AT_GuildRank, rank);
	}

	UpdateWho();
}

void Client::SendGuildList() {
	EQApplicationPacket *outapp;
//	outapp = new EQApplicationPacket(OP_ZoneGuildList);
	outapp = new EQApplicationPacket(OP_GuildsList);

	//ask the guild manager to build us a nice guild list packet
	outapp->pBuffer = guild_mgr.MakeGuildList(/*GetName()*/"", outapp->size);
	if(outapp->pBuffer == nullptr) {
		mlog(GUILDS__ERROR, "Unable to make guild list!");
		return;
	}

	mlog(GUILDS__OUT_PACKETS, "Sending OP_ZoneGuildList of length %d", outapp->size);
//	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);
}


void Client::SendGuildMembers() {
	uint32 len;
	uint8 *data = guild_mgr.MakeGuildMembers(GuildID(), GetName(), len);
	if(data == nullptr)
		return;	//invalid guild, shouldent happen.

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildMemberList);
	outapp->size = len;
	outapp->pBuffer = data;
	data = nullptr;

	mlog(GUILDS__OUT_PACKETS, "Sending OP_GuildMemberList of length %d", outapp->size);
	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);

	ServerPacket* pack = new ServerPacket(ServerOP_RequestOnlineGuildMembers, sizeof(ServerRequestOnlineGuildMembers_Struct));

	ServerRequestOnlineGuildMembers_Struct *srogms = (ServerRequestOnlineGuildMembers_Struct*)pack->pBuffer;

	srogms->FromID = CharacterID();
	srogms->GuildID = GuildID();

	worldserver.SendPacket(pack);

	safe_delete(pack);

	// We need to send the Guild URL and Channel name again, as sending OP_GuildMemberList appears to clear this information out.
	SendGuildURL();
	SendGuildChannel();
}

void Client::RefreshGuildInfo()
{
	uint32 OldGuildID = guild_id;

	guildrank = GUILD_RANK_NONE;
	guild_id = GUILD_NONE;

	bool WasBanker = GuildBanker;

	CharGuildInfo info;
	if(!guild_mgr.GetCharInfo(CharacterID(), info)) {
		mlog(GUILDS__ERROR, "Unable to obtain guild char info for %s (%d)", GetName(), CharacterID());
		return;
	}

	guildrank = info.rank;
	guild_id = info.guild_id;
	GuildBanker = info.banker || guild_mgr.IsGuildLeader(GuildID(), CharacterID());

	if(((int)zone->GetZoneID() == RuleI(World, GuildBankZoneID)))
	{
		if(WasBanker != GuildBanker)
		{
			EQApplicationPacket *outapp = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));

			GuildSetRank_Struct *gsrs = (GuildSetRank_Struct*)outapp->pBuffer;

			gsrs->Rank = guildrank;
			strn0cpy(gsrs->MemberName, GetName(), sizeof(gsrs->MemberName));
			gsrs->Banker = GuildBanker;

			FastQueuePacket(&outapp);
		}

		if((guild_id != OldGuildID) && GuildBanks)
		{
			ClearGuildBank();

			if(guild_id != GUILD_NONE)
				GuildBanks->SendGuildBank(this);
		}
	}

	SendGuildSpawnAppearance();
}

void EntityList::SendGuildMOTD(uint32 guild_id) {
	if(guild_id == GUILD_NONE)
		return;
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client* client = iterator.GetData();
		if (client->GuildID() == guild_id) {
			client->SendGuildMOTD();
			client->SendGuildURL();
			client->SendGuildChannel();
		}
		iterator.Advance();
	}
}

void EntityList::SendGuildSpawnAppearance(uint32 guild_id) {
	if(guild_id == GUILD_NONE)
		return;
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client* client = iterator.GetData();
		if (client->GuildID() == guild_id) {
			client->SendGuildSpawnAppearance();
		}
		iterator.Advance();
	}
}

void EntityList::RefreshAllGuildInfo(uint32 guild_id) {
	if(guild_id == GUILD_NONE)
		return;
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client* client = iterator.GetData();
		if (client->GuildID() == guild_id) {
			client->RefreshGuildInfo();
		}
		iterator.Advance();
	}
}

void EntityList::SendGuildMembers(uint32 guild_id) {
	if(guild_id == GUILD_NONE)
		return;

	//this could be optimized a bit to only build the member's packet once
	//and then keep swapping out the name in the packet on each send.

	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client* client = iterator.GetData();
		if (client->GuildID() == guild_id) {
			client->SendGuildMembers();
		}
		iterator.Advance();
	}
}

void EntityList::SendGuildList() {
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client* client = iterator.GetData();
		client->SendGuildList();
		iterator.Advance();
	}
}

void Client::SendGuildJoin(GuildJoin_Struct* gj){
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildManageAdd, sizeof(GuildJoin_Struct));
	GuildJoin_Struct* outgj=(GuildJoin_Struct*)outapp->pBuffer;
	outgj->class_ = gj->class_;
	outgj->guild_id = gj->guild_id;
	outgj->level = gj->level;
	strcpy(outgj->name, gj->name);
	outgj->rank = gj->rank;
	outgj->zoneid = gj->zoneid;

	mlog(GUILDS__OUT_PACKETS, "Sending OP_GuildManageAdd for join of length %d", outapp->size);
	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);

//	SendGuildMembers(gj->guild_id, true);
}

/*
void EntityList::SendGuildJoin(GuildJoin_Struct* gj){
	LinkedListIterator<Client*> iterator(client_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData()->CastToClient();
		if (client->IsInGuild(gj->guild_id))
			client->SendGuildJoin(gj);
		iterator.Advance();
	}
}*/



/*bool Client::SetGuild(uint32 in_guild_id, uint8 in_rank) {
	if (in_guild_id == 0) {
		// update DB
		if (!guild_mgr.SetGuild(character_id, 0, GUILD_MEMBER))
			return false;
		// clear guildtag
		guild_id = GUILD_NONE;
		SendAppearancePacket(AT_GuildID, GUILD_NONE);
		SendAppearancePacket(AT_GuildRank, GUILD_RANK_NONE);
		UpdateWho();
		return true;
	} else {
		if (!guild_mgr.SetGuild(character_id, in_guild_id, in_rank))
			return false;
		guildrank = in_rank;
		if (guild_id != in_guild_id) {
			guild_id = in_guild_id;
			SendAppearancePacket(AT_GuildID, in_guild_id);
		}
		SendAppearancePacket(AT_GuildRank, in_rank);
		UpdateWho();
		return true;
	}
	UpdateWho();
	return false;
}*/

/*
void Client::GuildChangeRank(uint32 guild_id, uint32 oldrank, uint32 newrank){
	GuildChangeRank(GetName(), guild_id, oldrank, newrank);
}

void Client::GuildChangeRank(const char* name, uint32 guild_id, uint32 oldrank, uint32 newrank) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildManageStatus, sizeof(GuildManageStatus_Struct));
	GuildManageStatus_Struct* gms = (GuildManageStatus_Struct*) outapp->pBuffer;
	gms->guild_id = guild_id;
	strcpy(gms->name, name);
	gms->newrank = newrank;
	gms->oldrank = oldrank;
	entity_list.QueueClientsGuild(this, outapp, false, guild_id);
	safe_delete(outapp);
	SendGuildMembers(guild_id, true);
}*/


bool ZoneDatabase::CheckGuildDoor(uint8 doorid,uint16 guild_id,const char* zone) {
	MYSQL_ROW row;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	if (!RunQuery(query, MakeAnyLenString(&query,
		"SELECT guild FROM doors where doorid=%i AND zone='%s'",
		doorid-128, zone), errbuf, &result))
	{
		LogFile->write(EQEMuLog::Error, "Error in CheckGuildDoor query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	} else {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if (atoi(row[0]) == guild_id)
			{
				mysql_free_result(result);
				return true;
			}
			else
			{
				mysql_free_result(result);
				return false;
			}

			// code below will never be reached
			mysql_free_result(result);
			return false;
		}
	}
	return false;
}

bool ZoneDatabase::SetGuildDoor(uint8 doorid,uint16 guild_id, const char* zone) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32	affected_rows = 0;
	if (doorid > 127)
		doorid = doorid - 128;
	if (!RunQuery(query, MakeAnyLenString(&query,
		"UPDATE doors SET guild = %i WHERE (doorid=%i) AND (zone='%s')",
		guild_id, doorid, zone), errbuf, 0,&affected_rows))
	{
		LogFile->write(EQEMuLog::Error, "Error in SetGuildDoor query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);

	return(affected_rows > 0);
}

