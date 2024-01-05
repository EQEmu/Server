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
			guuacs->Action = GuildUpdateURL;
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
			guuacs->Action = GuildUpdateChannel;

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

	int permissions = GUILD_MAX_FUNCTIONS + 1; //Static number of permissions in all EQ clients as of May 2014
	int ranks = GUILD_RECRUIT + 1; // Static number of RoF+ ranks as of May 2014
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
					guuacs->Action = GuildUpdatePermissions;
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
		for (int i = 1; i <= GUILD_MAX_RANK; i++)
		{
			auto outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateUCPStruct));
			GuildUpdateUCPStruct* gucp = (GuildUpdateUCPStruct*)outapp->pBuffer;

			gucp->payload.rank_name.rank = i;
			strcpy(gucp->payload.rank_name.rank_name, guild->rank_names[i].c_str());
			gucp->action = GuildUpdateRanks;

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

			gsrs->rank = guildrank;
			strn0cpy(gsrs->member_name, GetName(), sizeof(gsrs->member_name));
			gsrs->banker = GuildBanker;

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

void EntityList::SendGuildMOTD(uint32 guild_id)
{
	if (guild_id == GUILD_NONE) {
		return;
	}

	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			c.second->SendGuildMOTD();
		}
	}
}

void EntityList::SendGuildChannel(uint32 guild_id)
{
	if (guild_id == GUILD_NONE) {
		return;
	}

	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			c.second->SendGuildChannel();
		}
	}
}

void EntityList::SendGuildURL(uint32 guild_id)
{
	if (guild_id == GUILD_NONE) {
		return;
	}

	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			c.second->SendGuildURL();
		}
	}
}

void EntityList::SendGuildSpawnAppearance(uint32 guild_id)
{
	if (guild_id == GUILD_NONE) {
		return;
	}
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if (client->GuildID() == guild_id) {
			client->SendGuildSpawnAppearance();
		}
		++it;
	}
}

void EntityList::RefreshAllGuildInfo(uint32 guild_id)
{
	if (guild_id == GUILD_NONE) {
		return;
	}
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if (client->GuildID() == guild_id) {
			client->RefreshGuildInfo();
		}
		++it;
	}
}

void EntityList::SendGuildMembers(uint32 guild_id)
{
	if (guild_id == GUILD_NONE) {
		return;
	}

	//this could be optimized a bit to only build the member's packet once
	//and then keep swapping out the name in the packet on each send.

	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if (client->GuildID() == guild_id) {
			//client->SendGuildMembers();
		}
		++it;
	}
}

void EntityList::SendGuildList()
{
	for (auto const& c : client_list) {
		c.second->SendGuildList();
	}
}

void Client::SendGuildJoin(GuildJoin_Struct *gj)
{
	auto outapp = new EQApplicationPacket(OP_GuildManageAdd, sizeof(GuildJoin_Struct));
	auto outgj  = (GuildJoin_Struct *) outapp->pBuffer;
	outgj->class_   = gj->class_;
	outgj->guild_id = gj->guild_id;
	outgj->level    = gj->level;
	outgj->rank     = gj->rank;
	outgj->zoneid   = gj->zoneid;
	strcpy(outgj->name, gj->name);

	LogGuilds("Sending OP_GuildManageAdd for join of length [{}]", outapp->size);

	FastQueuePacket(&outapp);
}

void EntityList::GuildSetPreRoFBankerFlag(uint32 guild_id, uint32 guild_rank, bool bank_status)
{
	auto g_members = [&]() -> std::vector<Client *> {
		std::vector<Client *> members     = {};
		auto                  client_list = entity_list.GetClientList();
		for (auto const       &c: client_list) {
			CharGuildInfo cgi;
			guild_mgr.GetCharInfo(c.second->CharacterID(), cgi);
			if (c.second->GuildID() == guild_id && c.second->GuildRank() == guild_rank) {
				members.push_back(c.second);
			}
		}
		return members;
	};

	for (auto const &c: g_members()) {
		CharGuildInfo cgi;
		guild_mgr.GetCharInfo(c->CharacterID(), cgi);

		auto outapp = new ServerPacket(
			ServerOP_GuildRankUpdate,
			sizeof(ServerGuildRankUpdate_Struct)
		);

		ServerGuildRankUpdate_Struct *sgrus = (ServerGuildRankUpdate_Struct *) outapp->pBuffer;
		sgrus->guild_id = guild_id;
		sgrus->rank    = guild_rank;
		strcpy(sgrus->member_name, c->GetCleanName());
		guild_mgr.UpdateDbBankerFlag(c->CharacterID(), bank_status);
		sgrus->banker    = (bank_status ? 1 : 0) + (cgi.alt ? 2 : 0);
		sgrus->no_update = true;
		worldserver.SendPacket(outapp);
		safe_delete(outapp);
	}
}

void Client::SetGuildRank(uint32 rank)
{
	guildrank = rank;
}

void Client::SetGuildTributeOptIn(bool state)
{
	guild_tribute_opt_in = state;
}

void Client::SetGuildID(uint32 g_id)
{
	guild_id = g_id;
}

void EntityList::UpdateGuildTributes(uint32 guild_id)
{
	for (auto const &c: entity_list.GetClientList()) {
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

void Client::SendGuildMembersList()
{
	auto   guild_name = guild_mgr.GetGuildName(GuildID());
	uint32 len;
	uint8 *data = guild_mgr.MakeGuildMembers(GuildID(), guild_name, len);
	if (data == nullptr) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_GuildMemberList, len);
	outapp->pBuffer = data;
	data = nullptr;

	LogGuilds("Sending OP_GuildMemberList of length [{}]", outapp->size);

	FastQueuePacket(&outapp);

	SendGuildMOTD();
	SendGuildChannel();
	SendGuildURL();

	SetGuildListDirty(false);
}

void Client::SendGuildMemberAdd(
	uint32 guild_id,
	uint32 level,
	uint32 _class,
	uint32 rank,
	uint32 spirit,
	uint32 zone_id,
	std::string player_name
)
{
//correct
	struct GuildMemberAdd_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ uint32 unknown04;
		/*008*/ uint32 unknown08;
		/*012*/ uint32 unknown12;
		/*016*/ uint32 level;
		/*020*/ uint32 _class;
		/*024*/ uint32 rank;
		/*028*/ uint32 spirt; //not confirmed single byte 0x000000FF
		/*032*/ uint32 zone_id;
		/*036*/ uint32 last_on;
		/*040*/ char   player_name[64];
	};

	if (GetGuildListDirty()) {
		SendGuildMembersList();
		return;
	}

	auto outapp = new EQApplicationPacket(OP_GuildMemberAdd, sizeof(GuildMemberAdd_Struct));
	GuildMemberAdd_Struct *out = (GuildMemberAdd_Struct *) outapp->pBuffer;

	out->guild_id = guild_id;
	out->last_on  = time(nullptr);
	out->level    = level;
	out->zone_id  = zone_id;
	out->rank     = rank;
	out->spirt    = 0;
	out->_class   = _class;
	strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::SendGuildMemberRename(uint32 guild_id, std::string player_name, std::string new_player_name)
{
	struct GuildMemberRename_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
		/*068*/ char   new_player_name[64];
	};

	auto outapp = new EQApplicationPacket(OP_GuildMemberRename, sizeof(GuildMemberRename_Struct));
	GuildMemberRename_Struct *out = (GuildMemberRename_Struct *) outapp->pBuffer;

	out->guild_id = guild_id;
	strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));
	strn0cpy(out->new_player_name, new_player_name.c_str(), sizeof(out->new_player_name));

	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::SendGuildMemberDelete(uint32 guild_id, std::string player_name)
{
//correct
	struct GuildMemberDelete_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
	};

	auto outapp = new EQApplicationPacket(OP_GuildMemberDelete, sizeof(GuildMemberDelete_Struct));
	GuildMemberDelete_Struct *out = (GuildMemberDelete_Struct *) outapp->pBuffer;

	out->guild_id = guild_id;
	strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

	QueuePacket(outapp);
	safe_delete(outapp);

	SetGuildListDirty(true);

}

void Client::SendGuildMemberLevel(uint32 guild_id, uint32 level, std::string player_name)
{
//correct
	struct GuildMemberLevel_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
		/*068*/ uint32 level;
	};

	auto outapp = new EQApplicationPacket(OP_GuildMemberLevel, sizeof(GuildMemberLevel_Struct));
	GuildMemberLevel_Struct *out = (GuildMemberLevel_Struct *) outapp->pBuffer;

	out->guild_id = guild_id;
	out->level    = level;
	strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::SendGuildMemberRankAltBanker(uint32 guild_id, uint32 rank, std::string player_name, bool alt, bool banker)
{
//correct
	struct GuildMemberRank_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ uint32 rank;
		/*008*/ char   player_name[64];
		/*072*/ uint32 alt_banker; //Banker/Alt bit 00 - none 10 - Alt 11 - Alt and Banker 01 - Banker.  Banker not functional for RoF2+
	};

	auto outapp = new EQApplicationPacket(OP_GuildMemberRankAltBanker, sizeof(GuildMemberRank_Struct));
	GuildMemberRank_Struct *out = (GuildMemberRank_Struct *) outapp->pBuffer;

	out->guild_id   = guild_id;
	out->rank       = rank;
	out->alt_banker = (alt << 1) | banker;
	strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::SendGuildMemberPublicNote(uint32 guild_id, std::string player_name, std::string public_note)
{
	//correct
	struct GuildMemberPublicNote_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
		/*068*/ char   public_note[256]; //RoF2 256
	};

	auto outapp = new EQApplicationPacket(OP_GuildMemberPublicNote, sizeof(GuildMemberPublicNote_Struct));
	GuildMemberPublicNote_Struct *out = (GuildMemberPublicNote_Struct *) outapp->pBuffer;

	out->guild_id = guild_id;
	strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));
	strn0cpy(out->public_note, public_note.c_str(), sizeof(out->public_note));

	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::SendGuildMemberDetails(uint32 guild_id, uint32 zone_id, uint32 offline_mode, std::string player_name)
{
//correct
	struct GuildMemberDetails_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
		/*068*/ uint32 zone_id;
		/*072*/ uint32 last_on;
		/*076*/ uint32 offline_mode; //1 Offline
	};

	auto outapp = new EQApplicationPacket(OP_GuildMemberDetails, sizeof(GuildMemberDetails_Struct));
	GuildMemberDetails_Struct *out = (GuildMemberDetails_Struct *) outapp->pBuffer;

	out->guild_id     = guild_id;
	out->zone_id      = zone_id;
	out->last_on      = time(nullptr);
	out->offline_mode = offline_mode;
	strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::SendGuildRenameGuild(uint32 guild_id, std::string new_guild_name)
{
	//correct
	struct GuildRenameGuild_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   new_guild_name[64];
	};

	auto outapp = new EQApplicationPacket(OP_GuildRenameGuild, sizeof(GuildRenameGuild_Struct));
	GuildRenameGuild_Struct *out = (GuildRenameGuild_Struct *) outapp->pBuffer;

	out->guild_id = guild_id;
	strn0cpy(out->new_guild_name, new_guild_name.c_str(), sizeof(out->new_guild_name));

	QueuePacket(outapp);
	safe_delete(outapp);

}

void EntityList::SendGuildMembersList(uint32 guild_id)
{
	for (auto const& c : client_list) {
		if (c.second->GuildID() == guild_id) {
			c.second->SendGuildMembersList();
			c.second->SetGuildListDirty(false);
		}
	}
}

void EntityList::SendGuildMemberAdd(
	uint32 guild_id,
	uint32 level,
	uint32 _class,
	uint32 rank,
	uint32 spirit,
	uint32 zone_id,
	std::string player_name
)
{
	for (auto &c: client_list) {
		if (c.second->GuildID() == guild_id && c.second->GetGuildListDirty()) {
			c.second->SendGuildMembersList();
		}
		else if (c.second->GuildID() == guild_id && !c.second->GetGuildListDirty()) {
			auto outapp = new EQApplicationPacket(OP_GuildMemberAdd, sizeof(GuildMemberAdd_Struct));
			auto out = (GuildMemberAdd_Struct *) outapp->pBuffer;

			out->guild_id = guild_id;
			out->last_on  = time(nullptr);
			out->level    = level;
			out->zone_id  = zone_id;
			out->rank     = rank;
			out->spirt    = spirit;
			out->_class   = _class;
			strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);
		}

		if (player_name.compare(c.second->GetCleanName()) == 0) {
			c.second->SetGuildID(guild_id);
			c.second->SetGuildRank(rank);
			c.second->SendGuildList();
			c.second->SetGuildTributeOptIn(false);
			c.second->SendGuildMembersList();
			if (c.second->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
				c.second->SendGuildRanks();
				c.second->SendGuildRankNames();
			}

			if (zone->GetZoneID() == Zones::GUILDHALL && GuildBanks) {
				GuildBanks->SendGuildBank(c.second);
			}

			c.second->SendGuildActiveTributes(guild_id);
			c.second->SendAppearancePacket(AT_GuildID, guild_id, true, false, c.second, false);
			c.second->SendAppearancePacket(AT_GuildRank, rank, true, false, c.second, false);
			c.second->DoGuildTributeUpdate();
		}
	}
}

void EntityList::SendGuildMemberRename(uint32 guild_id, std::string player_name, std::string new_player_name)
{
	struct GuildMemberRename_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
		/*068*/ char   new_player_name[64];
	};

	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			auto outapp = new EQApplicationPacket(OP_GuildMemberRename, sizeof(GuildMemberRename_Struct));
			GuildMemberRename_Struct *out = (GuildMemberRename_Struct *) outapp->pBuffer;

			out->guild_id = guild_id;
			strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));
			strn0cpy(out->new_player_name, new_player_name.c_str(), sizeof(out->new_player_name));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void EntityList::SendGuildMemberRemove(uint32 guild_id, std::string player_name)
{
	struct GuildMemberDelete_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
	};

	for (auto &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			auto outapp = new EQApplicationPacket(OP_GuildMemberDelete, sizeof(GuildMemberDelete_Struct));
			GuildMemberDelete_Struct *out = (GuildMemberDelete_Struct *) outapp->pBuffer;

			out->guild_id = guild_id;
			strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);

			c.second->SetGuildListDirty(true);

			if (player_name.compare(c.second->GetCleanName()) == 0) {
				c.second->SetGuildID(GUILD_NONE);
				c.second->SetGuildRank(GUILD_RANK_NONE);
				c.second->SetGuildTributeOptIn(false);
				c.second->SendGuildActiveTributes(0);
				c.second->SetGuildListDirty(false);
				c.second->SendGuildList();
				c.second->SendAppearancePacket(AT_GuildID, GUILD_NONE, true);
				c.second->SendAppearancePacket(AT_GuildRank, GUILD_RANK_NONE, true);
			}
		}
	}
}

void EntityList::SendGuildMemberLevel(uint32 guild_id, uint32 level, std::string player_name)
{
	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			auto outapp = new EQApplicationPacket(OP_GuildMemberLevel, sizeof(GuildMemberLevel_Struct));
			GuildMemberLevel_Struct *out = (GuildMemberLevel_Struct *) outapp->pBuffer;

			out->guild_id = guild_id;
			out->level    = level;
			strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void EntityList::SendGuildMemberRankAltBanker(uint32 guild_id, uint32 rank, std::string player_name, bool alt, bool banker)
{
	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			auto outapp = new EQApplicationPacket(OP_GuildMemberRankAltBanker, sizeof(GuildMemberRank_Struct));
			auto out = (GuildMemberRank_Struct *) outapp->pBuffer;

			out->guild_id   = guild_id;
			out->rank       = rank;
			out->alt_banker = (alt << 1) | banker;
			strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);
		}

		if (player_name.compare(c.second->GetName()) == 0) {
			c.second->SetGuildRank(rank);
			c.second->SendAppearancePacket(AT_GuildRank, rank, false);
		}
	}
}

void EntityList::SendGuildMemberPublicNote(uint32 guild_id, std::string player_name, std::string public_note)
{
	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			auto outapp = new EQApplicationPacket(OP_GuildMemberPublicNote, sizeof(GuildMemberPublicNote_Struct));
			auto out = (GuildMemberPublicNote_Struct *) outapp->pBuffer;

			out->guild_id = guild_id;
			strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));
			strn0cpy(out->public_note, public_note.c_str(), sizeof(out->public_note));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void EntityList::SendGuildMemberDetails(uint32 guild_id, uint32 zone_id, uint32 offline_mode, std::string player_name)
{
	struct GuildMemberDetails_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   player_name[64];
		/*068*/ uint32 zone_id;
		/*072*/ uint32 last_on;
		/*076*/ uint32 offline_mode; //1 Offline
	};

	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			auto outapp = new EQApplicationPacket(OP_GuildMemberDetails, sizeof(GuildMemberDetails_Struct));
			GuildMemberDetails_Struct *out = (GuildMemberDetails_Struct *) outapp->pBuffer;

			out->guild_id     = guild_id;
			out->zone_id      = zone_id;
			out->last_on      = time(nullptr);
			out->offline_mode = offline_mode;
			strn0cpy(out->player_name, player_name.c_str(), sizeof(out->player_name));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void EntityList::SendGuildRenameGuild(uint32 guild_id, std::string new_guild_name)
{
	struct GuildRenameGuild_Struct {
		/*000*/ uint32 guild_id;
		/*004*/ char   new_guild_name[64];
	};

	for (auto const &c: client_list) {
		if (c.second->GuildID() == guild_id) {
			auto outapp = new EQApplicationPacket(OP_GuildRenameGuild, sizeof(GuildRenameGuild_Struct));
			GuildRenameGuild_Struct *out = (GuildRenameGuild_Struct *) outapp->pBuffer;

			out->guild_id = guild_id;
			strn0cpy(out->new_guild_name, new_guild_name.c_str(), sizeof(out->new_guild_name));

			c.second->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void Client::SendGuildDeletePacket(uint32 guild_id)
{
	auto outapp = new EQApplicationPacket(OP_GuildDeleteGuild, sizeof(GuildDelete_Struct));
	auto data = (GuildDelete_Struct*)outapp->pBuffer;

	data->guild_id = guild_id;
	FastQueuePacket(&outapp);
}