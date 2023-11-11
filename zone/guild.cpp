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

#include "../common/database.h"
#include "../common/guilds.h"
#include "../common/strings.h"

#include "guild_mgr.h"
#include "worldserver.h"

extern WorldServer worldserver;

void Client::SendGuildMOTD(bool GetGuildMOTDReply) {
	auto outapp = new EQApplicationPacket(OP_GuildMOTD, sizeof(GuildMOTD_Struct));

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

	LogGuilds("Sending OP_GuildMOTD of length [{}]", outapp->size);

	FastQueuePacket(&outapp);
}

void Client::SendGuildURL()
{
	if (ClientVersion() < EQ::versions::ClientVersion::SoF)
		return;

	if(IsInAGuild())
	{
		auto outapp =
		    new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateURLAndChannel_Struct));

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
	if (ClientVersion() < EQ::versions::ClientVersion::SoF)
		return;

	if(IsInAGuild())
	{
		auto outapp =
		    new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateURLAndChannel_Struct));

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

void Client::SendGuildRanks()
{
	if (ClientVersion() < EQ::versions::ClientVersion::RoF)
		return;

	int permissions = 30 + 1; //Static number of permissions in all EQ clients as of May 2014
	int ranks = 8 + 1; // Static number of RoF+ ranks as of May 2014
	int j = 1;
	int i = 1;
	if(IsInAGuild())
	{
		while(j < ranks)
		{
			while(i < permissions)
			{
					auto outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel,
						sizeof(GuildUpdateRanks_Struct));
					GuildUpdateRanks_Struct* guuacs = (GuildUpdateRanks_Struct*)outapp->pBuffer;
					//guuacs->Unknown0008 = GuildID();
					strncpy(guuacs->Unknown0012, GetCleanName(), 64);
					guuacs->Action = 5;
					guuacs->RankID = j;
					guuacs->GuildID = GuildID();
					guuacs->PermissionID = i;
					//				guuacs->PermissionVal = 1;
					guuacs->PermissionVal = guild_mgr.CheckPermission(GuildID(), j, (GuildAction)i);
					guuacs->Unknown0089[0] = 0x2c;
					guuacs->Unknown0089[1] = 0x01;
					guuacs->Unknown0089[2] = 0x00;
					FastQueuePacket(&outapp);
					i++;
			}
			j++;
			i = 1;
		}
	}
}

void Client::SendGuildRankNames() 
{
	if (IsInAGuild() && (ClientVersion() >= EQ::versions::ClientVersion::RoF))
	{
		auto guild = guild_mgr.GetGuildByGuildID(GuildID());
		for (int i = 1; i <= 8; i++)
		{
			auto outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateUCP));
			GuildUpdateUCP* gucp = (GuildUpdateUCP*)outapp->pBuffer;

			gucp->payload.rank_name.rank = i;
			strcpy(gucp->payload.rank_name.rank_name, guild->rank_names[i].c_str());
			gucp->action = 4;

			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void Client::SendGuildSpawnAppearance() {
	if (!IsInAGuild()) {
		// clear guildtag
		SendAppearancePacket(AT_GuildID, GUILD_NONE);
		LogGuilds("Sending spawn appearance for no guild tag");
	} else {
		uint8 rank = guild_mgr.GetDisplayedRank(GuildID(), GuildRank(), CharacterID());
		LogGuilds("Sending spawn appearance for guild [{}] at rank [{}]", GuildID(), rank);
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
		LogGuilds("Unable to make guild list!");
		safe_delete(outapp);
		return;
	}

	LogGuilds("Sending OP_ZoneGuildList of length [{}]", outapp->size);

	FastQueuePacket(&outapp);
}


void Client::SendGuildMembers() {
	uint32 len;
	uint8 *data = guild_mgr.MakeGuildMembers(GuildID(), GetName(), len);
	if(data == nullptr)
		return;	//invalid guild, shouldent happen.

	auto outapp = new EQApplicationPacket(OP_GuildMemberList);
	outapp->size = len;
	outapp->pBuffer = data;
	data = nullptr;

	LogGuilds("Sending OP_GuildMemberList of length [{}]", outapp->size);

	FastQueuePacket(&outapp);

	auto pack =
	    new ServerPacket(ServerOP_RequestOnlineGuildMembers, sizeof(ServerRequestOnlineGuildMembers_Struct));

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
		LogGuilds("Unable to obtain guild char info for [{}] ([{}])", GetName(), CharacterID());
		return;
	}

	guildrank = info.rank;
	guild_id = info.guild_id;
	GuildBanker = info.banker ||
		guild_mgr.IsGuildLeader(GuildID(), CharacterID()) ||
		guild_mgr.GetBankerFlag(CharacterID()) ||
		ClientVersion() >= EQ::versions::ClientVersion::RoF ? true : false;
	
	if(zone->GetZoneID() == Zones::GUILDHALL)
	{
		if(WasBanker != GuildBanker)
		{
			auto outapp = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));

			GuildSetRank_Struct *gsrs = (GuildSetRank_Struct*)outapp->pBuffer;

			gsrs->Rank = guildrank;
			strn0cpy(gsrs->MemberName, GetName(), sizeof(gsrs->MemberName));
			gsrs->Banker = GuildBanker;

			FastQueuePacket(&outapp);
		}

		if((guild_id != OldGuildID) && GuildBanks)
		{
			// Unsure about this for RoF+ ... But they don't have that action anymore so fuck it
			if (ClientVersion() < EQ::versions::ClientVersion::RoF)
				ClearGuildBank();

			if (guild_id != GUILD_NONE)
				GuildBanks->SendGuildBank(this);
		}
	}

	SendGuildSpawnAppearance();
}

void EntityList::SendGuildMOTD(uint32 guild_id) {
	if (guild_id == GUILD_NONE)
		return;
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client* client = it->second;
		if (client->GuildID() == guild_id) {
			client->SendGuildMOTD();
			client->SendGuildURL();
			client->SendGuildChannel();
		}
		++it;
	}
}

void EntityList::SendGuildSpawnAppearance(uint32 guild_id) {
	if (guild_id == GUILD_NONE)
		return;
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client* client = it->second;
		if (client->GuildID() == guild_id) {
			client->SendGuildSpawnAppearance();
		}
		++it;
	}
}

void EntityList::RefreshAllGuildInfo(uint32 guild_id) {
	if (guild_id == GUILD_NONE)
		return;
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client* client = it->second;
		if (client->GuildID() == guild_id) {
			client->RefreshGuildInfo();
		}
		++it;
	}
}

void EntityList::SendGuildMembers(uint32 guild_id) {
	if (guild_id == GUILD_NONE)
		return;

	//this could be optimized a bit to only build the member's packet once
	//and then keep swapping out the name in the packet on each send.

	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client* client = it->second;
		if (client->GuildID() == guild_id) {
			client->SendGuildMembers();
		}
		++it;
	}
}

void EntityList::SendGuildList() {
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client* client = it->second;
		client->SendGuildList();
		++it;
	}
}

void Client::SendGuildJoin(GuildJoin_Struct* gj) {
	auto outapp = new EQApplicationPacket(OP_GuildManageAdd, sizeof(GuildJoin_Struct));
	GuildJoin_Struct* outgj = (GuildJoin_Struct*)outapp->pBuffer;
	outgj->class_ = gj->class_;
	outgj->guild_id = gj->guild_id;
	outgj->level = gj->level;
	strcpy(outgj->name, gj->name);
	outgj->rank = gj->rank;
	outgj->zoneid = gj->zoneid;

	LogGuilds("Sending OP_GuildManageAdd for join of length [{}]", outapp->size);

	FastQueuePacket(&outapp);

}

void EntityList::GuildSetPreRoFBankerFlag(uint32 guild_id, uint32 guild_rank, bool bank_status)
{
	auto g_members = [&]() -> std::vector<Client*> {
		std::vector<Client*> members = {};
		auto client_list = entity_list.GetClientList();
		for (auto const& c : client_list) {
			CharGuildInfo cgi;
			guild_mgr.GetCharInfo(c.second->CharacterID(), cgi);
			if (c.second->GuildID() == guild_id && c.second->GuildRank() == guild_rank) {
				members.push_back(c.second);
			}
		}
		return members;
	};
	
//	auto guild_bank_status = guild_mgr.GetGuildBankerStatus(guild_id, guild_rank);
	for (auto const& c : g_members()) {
		CharGuildInfo cgi;
		guild_mgr.GetCharInfo(c->CharacterID(), cgi);

		auto outapp = new ServerPacket(ServerOP_GuildRankUpdate, sizeof(ServerGuildRankUpdate_Struct));
		ServerGuildRankUpdate_Struct* sgrus = (ServerGuildRankUpdate_Struct*)outapp->pBuffer;
		sgrus->GuildID = guild_id;
		sgrus->Rank = guild_rank;
		strcpy(sgrus->MemberName, c->GetCleanName());
		guild_mgr.DBSetBankerFlag(c->CharacterID(), bank_status);
		sgrus->Banker = (bank_status ? 1 : 0) + (cgi.alt ? 2 : 0);
		sgrus->no_update = true;
		worldserver.SendPacket(outapp);
		safe_delete(outapp);
	}
	//	auto outapp = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));
	//	GuildSetRank_Struct* gsrs = (GuildSetRank_Struct*)outapp->pBuffer;
	//	gsrs->Rank = c->GuildRank();
	//	strn0cpy(gsrs->MemberName, c->GetCleanName(), sizeof(c->GetCleanName()));
	//	if (guild_bank_status && !cgi.banker) {
	//		guild_mgr.DBSetBankerFlag(c->CharacterID(), true);
	//		gsrs->Banker = 1 + (cgi.alt ? 2 : 0);
	//	}
	//	else if (!guild_bank_status && cgi.banker) {
	//		guild_mgr.DBSetBankerFlag(c->CharacterID(), true);
	//		gsrs->Banker = 0 + (cgi.alt ? 2 : 0);
	//	}
	//	c->QueuePacket(outapp);
	//	safe_delete(outapp);
	//}
}

void Client::SetGuildRank(uint32 rank) 
{
	guildrank = rank;
}

void Client::SetGuildID(uint32 guild_id) 
{
	guild_id = guild_id;
}

void EntityList::UpdateGuildTributes(uint32 guild_id) {
	for (auto const& c : entity_list.GetClientList()) {
		if (c.second->GuildID() == guild_id) {
			c.second->DoGuildTributeUpdate();
		}
	}
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

