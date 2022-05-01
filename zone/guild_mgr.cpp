/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/servertalk.h"
#include "../common/string_util.h"

#include "client.h"
#include "entity.h"
#include "guild_mgr.h"
#include "worldserver.h"
#include "zonedb.h"
#include "zone_store.h"

ZoneGuildManager guild_mgr;
GuildBankManager *GuildBanks;

extern WorldServer worldserver;
extern volatile bool is_zone_loaded;

void ZoneGuildManager::SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) {
	LogGuilds("Sending guild refresh for [{}] to world, changes: name=[{}], motd=[{}], rank=d, relation=[{}]", guild_id, name, motd, rank, relation);
	auto pack = new ServerPacket(ServerOP_RefreshGuild, sizeof(ServerGuildRefresh_Struct));
	ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->name_change = name;
	s->motd_change = motd;
	s->rank_change = rank;
	s->relation_change = relation;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid) {
	if(guild_id == 0) {
		LogGuilds("Guild lookup for char [{}] when sending char refresh", charid);

		CharGuildInfo gci;
		if(!GetCharInfo(charid, gci)) {
			guild_id = GUILD_NONE;
		} else {
			guild_id = gci.guild_id;
		}
	}

	LogGuilds("Sending char refresh for [{}] from guild [{}] to world", charid, guild_id);

	auto pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
	ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->old_guild_id = old_guild_id;
	s->char_id = charid;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::SendRankUpdate(uint32 CharID)
{
	CharGuildInfo gci;

	if(!GetCharInfo(CharID, gci))
		return;

	auto pack = new ServerPacket(ServerOP_GuildRankUpdate, sizeof(ServerGuildRankUpdate_Struct));

	ServerGuildRankUpdate_Struct *sgrus = (ServerGuildRankUpdate_Struct*)pack->pBuffer;

	sgrus->GuildID = gci.guild_id;
	strn0cpy(sgrus->MemberName, gci.char_name.c_str(), sizeof(sgrus->MemberName));
	sgrus->Rank = gci.rank;
	sgrus->Banker = gci.banker + (gci.alt * 2);

	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::SendGuildDelete(uint32 guild_id) {
	LogGuilds("Sending guild delete for guild [{}] to world", guild_id);
	auto pack = new ServerPacket(ServerOP_DeleteGuild, sizeof(ServerGuildID_Struct));
	ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

//makes a guild member list packet (internal format), returns ownership of the buffer.
uint8 *ZoneGuildManager::MakeGuildMembers(uint32 guild_id, const char *prefix_name, uint32 &length) {
	uint8 *retbuffer;

	//hack because we dont have the "remove from guild" packet right now.
	if(guild_id == GUILD_NONE) {
		length = sizeof(Internal_GuildMembers_Struct);
		retbuffer = new uint8[length];
		Internal_GuildMembers_Struct *gms = (Internal_GuildMembers_Struct *) retbuffer;
		strcpy(gms->player_name, prefix_name);
		gms->count = 0;
		gms->name_length = 0;
		gms->note_length = 0;
		return(retbuffer);
	}

	std::vector<CharGuildInfo *> members;
	if(!GetEntireGuild(guild_id, members))
		return(nullptr);

	//figure out the actual packet length.
	uint32 fixed_length = sizeof(Internal_GuildMembers_Struct) + members.size()*sizeof(Internal_GuildMemberEntry_Struct);
	std::vector<CharGuildInfo *>::iterator cur, end;
	CharGuildInfo *ci;
	cur = members.begin();
	end = members.end();
	uint32 name_len = 0;
	uint32 note_len = 0;
	for(; cur != end; ++cur) {
		ci = *cur;
		name_len += ci->char_name.length();
		note_len += ci->public_note.length();
	}

	//calc total length.
	length = fixed_length + name_len + note_len + members.size()*2;	//string data + null terminators

	//make our nice buffer
	retbuffer = new uint8[length];

	Internal_GuildMembers_Struct *gms = (Internal_GuildMembers_Struct *) retbuffer;

	//fill in the global header
	strcpy(gms->player_name, prefix_name);
	gms->count = members.size();
	gms->name_length = name_len;
	gms->note_length = note_len;

	char *name_buf = (char *) ( retbuffer + fixed_length );
	char *note_buf = (char *) ( name_buf + name_len + members.size() );

	//fill in each member's entry.
	Internal_GuildMemberEntry_Struct *e = gms->member;

	cur = members.begin();
	end = members.end();
	for(; cur != end; ++cur) {
		ci = *cur;

		//the order we set things here must match the struct

//nice helper macro
#define SlideStructString(field, str) \
		strcpy(field, str.c_str()); \
		field += str.length() + 1
#define PutField(field) \
		e->field = ci->field

		SlideStructString( name_buf, ci->char_name );
		PutField(level);
		e->banker = ci->banker + (ci->alt * 2);	// low bit is banker flag, next bit is 'alt' flag.
		PutField(class_);
		PutField(rank);
		PutField(time_last_on);
		PutField(tribute_enable);
		PutField(total_tribute);
		PutField(last_tribute);
		SlideStructString( note_buf, ci->public_note );
		e->zoneinstance = 0;
		e->zone_id = 0;	// Flag them as offline (zoneid 0) as world will update us with their online status afterwards.
#undef SlideStructString
#undef PutFieldN

		delete *cur;

		e++;
	}

	return(retbuffer);
}

void ZoneGuildManager::ListGuilds(Client *c) const {
	if (m_guilds.size()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Listing {} Guild{}.",
				m_guilds.size(),
				m_guilds.size() != 1 ? "s" : ""
			).c_str()
		);

		for (const auto& guild : m_guilds) {
			auto leader_name = database.GetCharNameByID(guild.second->leader_char_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Guild {} | {}Name: {}",
					guild.first,
					(
						!leader_name.empty() ?
						fmt::format(
							"Leader: {} ({}) ",
							leader_name,
							guild.second->leader_char_id
						) :
						""
					),
					guild.second->name
				).c_str()
			);
		}
	} else {
		c->Message(Chat::White, "There are no Guilds to list.");
	}
}


void ZoneGuildManager::DescribeGuild(Client *c, uint32 guild_id) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if (res == m_guilds.end()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Guild ID {} could not be found.",
				guild_id
			).c_str()
		);
		return;
	}

	const GuildInfo *info = res->second;

	auto leader_name = database.GetCharNameByID(info->leader_char_id);
	std::string popup_text = "<table>";
	popup_text += fmt::format(
		"<tr><td>Name</td><td>{}</td><td>Guild ID</td><td>{}</td></tr>",
		info->name,
		guild_id
	);
	popup_text += fmt::format(
		"<tr><td>Leader</td><td>{}</td><td>Character ID</td><td>{}</td></tr>",
		leader_name,
		info->leader_char_id
	);
	popup_text += "<br><br>";
	popup_text += "<tr>";
	popup_text += "<td>Rank</td>";
	popup_text += "<td>Demote</td>";
	popup_text += "<td>Hear Guild Chat</td>";
	popup_text += "<td>Invite</td>";
	popup_text += "<td>Promote</td>";
	popup_text += "<td>Remove</td>";
	popup_text += "<td>Set MOTD</td>";
	popup_text += "<td>Speak Guild Chat</td>";
	popup_text += "<td>War/Peace</td>";
	popup_text += "</tr>";

	for (uint8 guild_rank = 0; guild_rank <= GUILD_MAX_RANK; guild_rank++) {
		auto can_hear_guild_chat = info->ranks[guild_rank].permissions[GUILD_HEAR] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		auto can_speak_guild_chat = info->ranks[guild_rank].permissions[GUILD_SPEAK] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		auto can_invite = info->ranks[guild_rank].permissions[GUILD_INVITE] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		auto can_remove = info->ranks[guild_rank].permissions[GUILD_REMOVE] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		auto can_promote = info->ranks[guild_rank].permissions[GUILD_PROMOTE] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		auto can_demote = info->ranks[guild_rank].permissions[GUILD_DEMOTE] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		auto can_set_motd = info->ranks[guild_rank].permissions[GUILD_MOTD] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		auto can_war_peace = info->ranks[guild_rank].permissions[GUILD_WARPEACE] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>";
		popup_text += fmt::format(
			"<tr>"
			"<td>{} ({})</td>"
			"<td>{}</td>"
			"<td>{}</td>"
			"<td>{}</td>"
			"<td>{}</td>"
			"<td>{}</td>"
			"<td>{}</td>"
			"<td>{}</td>"
			"<td>{}</td>"
			"</tr>",
			!info->ranks[guild_rank].name.empty() ? info->ranks[guild_rank].name : "Nameless",
			guild_rank,
			can_demote,
			can_hear_guild_chat,
			can_invite,
			can_promote,
			can_remove,
			can_set_motd,
			can_speak_guild_chat,
			can_war_peace
		);
	}

	popup_text += "</table>";

	c->SendPopupToClient(
		"Guild Information",
		popup_text.c_str()
	);
}

//in theory, we could get a pile of unused entries in this array, but only if
//we had a malicious client sending controlled packets, plus its like 10 bytes per entry.
void ZoneGuildManager::RecordInvite(uint32 char_id, uint32 guild_id, uint8 rank) {
	m_inviteQueue[char_id] = std::pair<uint32, uint8>(guild_id, rank);
}

bool ZoneGuildManager::VerifyAndClearInvite(uint32 char_id, uint32 guild_id, uint8 rank) {
	std::map<uint32, std::pair<uint32, uint8> >::iterator res;
	res = m_inviteQueue.find(char_id);
	if(res == m_inviteQueue.end())
		return(false);	//no entry...
	bool valid = false;
	if(res->second.first == guild_id && res->second.second == rank) {
		valid = true;
	}
	m_inviteQueue.erase(res);
	return(valid);
}

void ZoneGuildManager::ProcessWorldPacket(ServerPacket *pack) {
	switch(pack->opcode) {
	case ServerOP_RefreshGuild: {
		if(pack->size != sizeof(ServerGuildRefresh_Struct)) {
			LogError("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildRefresh_Struct));
			return;
		}
		ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;

		LogGuilds("Received guild refresh from world for [{}], changes: name=[{}], motd=[{}], rank=[{}], relation=[{}]", s->guild_id, s->name_change, s->motd_change, s->rank_change, s->relation_change);

		//reload all the guild details from the database.
		RefreshGuild(s->guild_id);

		if(s->motd_change) {
			//resend guild MOTD to all guild members in this zone.
			entity_list.SendGuildMOTD(s->guild_id);
		}

		if(s->name_change) {
			//until we figure out the guild update packet, we resend the whole guild list.
			entity_list.SendGuildList();
		}

		if(s->rank_change) {
			//we need to send spawn appearance packets for all members of this guild in the zone, to everybody.
			entity_list.SendGuildSpawnAppearance(s->guild_id);
		}

		if(s->relation_change) {
			//unknown until we implement guild relations.
		}

		break;
	}

	case ServerOP_GuildCharRefresh: {
		if(pack->size != sizeof(ServerGuildCharRefresh_Struct)) {
			LogError("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildCharRefresh_Struct));
			return;
		}
		ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;

		LogGuilds("Received guild member refresh from world for char [{}] from guild [{}]", s->char_id, s->guild_id);

		Client *c = entity_list.GetClientByCharID(s->char_id);

		if(c != nullptr) {
			//this reloads the char's guild info from the database and sends appearance updates
			c->RefreshGuildInfo();
		}

		//it would be nice if we had the packet to send just a one-person update
		if(s->guild_id == GUILD_NONE) {
			if(c != nullptr)
				c->SendGuildMembers();	//only need to update this player's list (trying to clear it)
		} else {
			entity_list.SendGuildMembers(s->guild_id);		//even send GUILD_NONE (empty)
		}

		if(s->old_guild_id != 0 && s->old_guild_id != GUILD_NONE && s->old_guild_id != s->guild_id)
			entity_list.SendGuildMembers(s->old_guild_id);
		else if(c != nullptr && s->guild_id != GUILD_NONE) {
			//char is in zone, and has changed into a new guild, send MOTD.
			c->SendGuildMOTD();
			if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF)
			{
				c->SendGuildRanks();
			}
		}


		break;
	}

	case ServerOP_GuildRankUpdate:
	{
		if(is_zone_loaded)
		{
			if(pack->size != sizeof(ServerGuildRankUpdate_Struct))
			{
				LogError("Received ServerOP_RankUpdate of incorrect size [{}], expected [{}]",
					pack->size, sizeof(ServerGuildRankUpdate_Struct));

				return;
			}

			ServerGuildRankUpdate_Struct *sgrus = (ServerGuildRankUpdate_Struct*)pack->pBuffer;

			auto outapp = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));

			GuildSetRank_Struct *gsrs = (GuildSetRank_Struct*)outapp->pBuffer;

			gsrs->Rank = sgrus->Rank;
			strn0cpy(gsrs->MemberName, sgrus->MemberName, sizeof(gsrs->MemberName));
			gsrs->Banker = sgrus->Banker;

			entity_list.QueueClientsGuild(nullptr, outapp, false, sgrus->GuildID);

			safe_delete(outapp);
		}

		break;
	}

	case ServerOP_DeleteGuild: {
		if(pack->size != sizeof(ServerGuildID_Struct)) {
			LogError("Received ServerOP_DeleteGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildID_Struct));
			return;
		}
		ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;

		LogGuilds("Received guild delete from world for guild [{}]", s->guild_id);

		//clear all the guild tags.
		entity_list.RefreshAllGuildInfo(s->guild_id);

		//remove the guild data from the local guild manager
		guild_mgr.LocalDeleteGuild(s->guild_id);

		//if we stop forcing guild list to send on guild create, we need to do this:
		//in the case that we delete a guild and add a new one.
		//entity_list.SendGuildList();

		break;
	}

	case ServerOP_GuildMemberUpdate:
	{
		ServerGuildMemberUpdate_Struct *sgmus = (ServerGuildMemberUpdate_Struct*)pack->pBuffer;

		if(is_zone_loaded)
		{
			auto outapp = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(GuildMemberUpdate_Struct));

			GuildMemberUpdate_Struct *gmus = (GuildMemberUpdate_Struct*)outapp->pBuffer;

			gmus->GuildID = sgmus->GuildID;
			strn0cpy(gmus->MemberName, sgmus->MemberName, sizeof(gmus->MemberName));
			gmus->ZoneID = sgmus->ZoneID;
			gmus->InstanceID = 0;	// I don't think we care what Instance they are in, for the Guild Management Window.
			gmus->LastSeen = sgmus->LastSeen;

			entity_list.QueueClientsGuild(nullptr, outapp, false, sgmus->GuildID);

			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_OnlineGuildMembersResponse:
		if (is_zone_loaded)
		{
			char *Buffer = (char *)pack->pBuffer;

			uint32 FromID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
			uint32 Count = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
			Client *c = entity_list.GetClientByCharID(FromID);

			if (!c || !c->IsInAGuild())
			{
				LogGuilds("Invalid Client or not in guild. ID=[{}]", FromID);
				break;
			}
			LogGuilds("Processing ServerOP_OnlineGuildMembersResponse");
			auto outapp = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(GuildMemberUpdate_Struct));
			GuildMemberUpdate_Struct *gmus = (GuildMemberUpdate_Struct*)outapp->pBuffer;
			char Name[64];
			gmus->LastSeen = time(nullptr);
			gmus->InstanceID = 0;
			gmus->GuildID = c->GuildID();
			for (int i = 0; i < Count; i++) {
				// Just make the packet once and swap out name/zone and send
				VARSTRUCT_DECODE_STRING(Name, Buffer);
				strn0cpy(gmus->MemberName, Name, sizeof(gmus->MemberName));
				gmus->ZoneID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
				LogGuilds("Sending OP_GuildMemberUpdate to [{}]. Name=[{}] ZoneID=[{}]", FromID, Name, gmus->ZoneID);
				c->QueuePacket(outapp);
			}
			safe_delete(outapp);

		}
		break;

	case ServerOP_LFGuildUpdate:
	{
		if(is_zone_loaded)
		{
			char GuildName[33];
			char Comments[257];
			uint32 FromLevel, ToLevel, Classes, AACount, TimeZone, TimePosted, Toggle;

			pack->ReadString(GuildName);
			pack->ReadString(Comments);
			FromLevel = pack->ReadUInt32();
			ToLevel = pack->ReadUInt32();
			Classes = pack->ReadUInt32();
			AACount = pack->ReadUInt32();
			TimeZone = pack->ReadUInt32();
			TimePosted = pack->ReadUInt32();
			Toggle = pack->ReadUInt32();

			uint32 GuildID = GetGuildIDByName(GuildName);

			if(GuildID == GUILD_NONE)
				break;

			auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_GuildToggle_Struct));

			LFGuild_GuildToggle_Struct *gts = (LFGuild_GuildToggle_Struct *)outapp->pBuffer;
			gts->Command = 1;
			strcpy(gts->Comment, Comments);
			gts->FromLevel = FromLevel;
			gts->ToLevel = ToLevel;
			gts->Classes = Classes;
			gts->AACount = AACount;
			gts->TimeZone = TimeZone;
			gts->Toggle = Toggle;
			gts->TimePosted = TimePosted;
			gts->Name[0] = 0;
			entity_list.QueueClientsGuild(nullptr, outapp, false, GuildID);
			safe_delete(outapp);
			break;
		}
	}
	}
}

void ZoneGuildManager::SendGuildMemberUpdateToWorld(const char *MemberName, uint32 GuildID, uint16 ZoneID, uint32 LastSeen)
{
	auto pack = new ServerPacket(ServerOP_GuildMemberUpdate, sizeof(ServerGuildMemberUpdate_Struct));

	ServerGuildMemberUpdate_Struct *sgmus = (ServerGuildMemberUpdate_Struct*)pack->pBuffer;
	sgmus->GuildID = GuildID;
	strn0cpy(sgmus->MemberName, MemberName, sizeof(sgmus->MemberName));
	sgmus->ZoneID = ZoneID;
	sgmus->LastSeen = LastSeen;
	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::RequestOnlineGuildMembers(uint32 FromID, uint32 GuildID)
{
	auto pack =
	    new ServerPacket(ServerOP_RequestOnlineGuildMembers, sizeof(ServerRequestOnlineGuildMembers_Struct));
	ServerRequestOnlineGuildMembers_Struct *srogm = (ServerRequestOnlineGuildMembers_Struct*)pack->pBuffer;

	srogm->FromID = FromID;
	srogm->GuildID = GuildID;
	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::ProcessApproval()
{
	LinkedListIterator<GuildApproval*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(!iterator.GetData()->ProcessApproval())
			iterator.RemoveCurrent();
		iterator.Advance();
	}
}

void ZoneGuildManager::AddGuildApproval(const char* guildname,Client* owner)
{
	auto tmp = new GuildApproval(guildname, owner, GetFreeID());
	list.Insert(tmp);
}

void ZoneGuildManager::AddMemberApproval(uint32 refid,Client* name)
{
	GuildApproval* tmp = FindGuildByIDApproval(refid);
	if(tmp != 0)
	{
		if(!tmp->AddMemberApproval(name))
			name->Message(Chat::White,"Unable to add to list.");
		else
		{
			name->Message(Chat::White,"Added to list.");
		}
	}
	else
		name->Message(Chat::White,"Unable to find guild reference id.");
}

ZoneGuildManager::~ZoneGuildManager()
{
	ClearGuilds();
}

void ZoneGuildManager::ClearGuildsApproval()
{
	list.Clear();
}

GuildApproval* ZoneGuildManager::FindGuildByIDApproval(uint32 refid)
{
	LinkedListIterator<GuildApproval*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID() == refid)
			return iterator.GetData();
		iterator.Advance();
	}
	return 0;
}

GuildApproval* ZoneGuildManager::FindGuildByOwnerApproval(Client* owner)
{
	LinkedListIterator<GuildApproval*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetOwner() == owner)
			return iterator.GetData();
		iterator.Advance();
	}
	return 0;
}

GuildBankManager::~GuildBankManager()
{
	auto Iterator = Banks.begin();

	while(Iterator != Banks.end())
	{
		safe_delete(*Iterator);

		++Iterator;
	}
}

bool GuildBankManager::Load(uint32 guildID)
{

	std::string query = StringFormat("SELECT `area`, `slot`, `itemid`, `qty`, `donator`, `permissions`, `whofor` "
                                    "FROM `guild_bank` WHERE `guildid` = %i", guildID);
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		return false;
	}

	auto bank = new GuildBank;

	bank->GuildID = guildID;

	for (int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
		bank->Items.MainArea[i].ItemID = 0;

	for (int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
		bank->Items.DepositArea[i].ItemID = 0;

	char donator[64], whoFor[64];

	for (auto row = results.begin(); row != results.end(); ++row) {
		int area = atoi(row[0]);
		int slot = atoi(row[1]);
		int itemID = atoi(row[2]);
		int qty = atoi(row[3]);

		if (row[4])
			strn0cpy(donator, row[4], sizeof(donator));
		else
			donator[0] = '\0';

		int permissions = atoi(row[5]);

		if (row[6])
			strn0cpy(whoFor, row[6], sizeof(whoFor));
		else
			whoFor[0] = '\0';

		if (slot < 0)
			continue;

		GuildBankItem *itemSection = nullptr;

		if (area == GuildBankMainArea && slot < GUILD_BANK_MAIN_AREA_SIZE)
			itemSection = bank->Items.MainArea;
		else if (area != GuildBankMainArea && slot < GUILD_BANK_DEPOSIT_AREA_SIZE)
			itemSection = bank->Items.DepositArea;
		else
			continue;

		itemSection[slot].ItemID = itemID;
		itemSection[slot].Quantity = qty;

		strn0cpy(itemSection[slot].Donator, donator, sizeof(donator));

		itemSection[slot].Permissions = permissions;

		strn0cpy(itemSection[slot].WhoFor, whoFor, sizeof(whoFor));
    }

    Banks.push_back(bank);

	return true;
}

bool GuildBankManager::IsLoaded(uint32 GuildID)
{
	auto Iterator = GetGuildBank(GuildID);

	return (Iterator != Banks.end());
}

void GuildBankManager::SendGuildBank(Client *c)
{
	if(!c || !c->IsInAGuild())
		return;

	if(!IsLoaded(c->GuildID()))
		Load(c->GuildID());

	auto Iterator = GetGuildBank(c->GuildID());

	if(Iterator == Banks.end())
	{
		LogError("Unable to find guild bank for guild ID [{}]", c->GuildID());

		return;
	}

	auto &guild_bank = *Iterator;

	// RoF+ uses a bulk list packet -- This is also how the Action 0 of older clients basically works
	if (c->ClientVersionBit() & EQ::versions::maskRoFAndLater) {
		auto outapp = new EQApplicationPacket(OP_GuildBankItemList, sizeof(GuildBankItemListEntry_Struct) * 240);
		for (int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i) {
			const EQ::ItemData *Item = database.GetItem(guild_bank->Items.DepositArea[i].ItemID);
			if (Item) {
				outapp->WriteUInt8(1);
				outapp->WriteUInt32(guild_bank->Items.DepositArea[i].Permissions);
				outapp->WriteString(guild_bank->Items.DepositArea[i].WhoFor);
				outapp->WriteString(guild_bank->Items.DepositArea[i].Donator);
				outapp->WriteUInt32(Item->ID);
				outapp->WriteUInt32(Item->Icon);
				if (Item->Stackable) {
					outapp->WriteUInt32(guild_bank->Items.DepositArea[i].Quantity);
					outapp->WriteUInt8(Item->StackSize == guild_bank->Items.DepositArea[i].Quantity ? 0 : 1);
				} else {
					outapp->WriteUInt32(1);
					outapp->WriteUInt8(0);
				}
				outapp->WriteUInt8(Item->IsEquipable(c->GetBaseRace(), c->GetBaseClass()) ? 1 : 0);
				outapp->WriteString(Item->Name);
			} else {
				outapp->WriteUInt8(0); // empty
			}
		}
		outapp->SetWritePosition(outapp->GetWritePosition() + 20); // newer clients have 40 deposit slots, keep them 0 for now

		for (int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i) {
			const EQ::ItemData *Item = database.GetItem(guild_bank->Items.MainArea[i].ItemID);
			if (Item) {
				outapp->WriteUInt8(1);
				outapp->WriteUInt32(guild_bank->Items.MainArea[i].Permissions);
				outapp->WriteString(guild_bank->Items.MainArea[i].WhoFor);
				outapp->WriteString(guild_bank->Items.MainArea[i].Donator);
				outapp->WriteUInt32(Item->ID);
				outapp->WriteUInt32(Item->Icon);
				if (Item->Stackable) {
					outapp->WriteUInt32(guild_bank->Items.MainArea[i].Quantity);
					outapp->WriteUInt8(Item->StackSize == guild_bank->Items.MainArea[i].Quantity ? 0 : 1);
				} else {
					outapp->WriteUInt32(1);
					outapp->WriteUInt8(0);
				}
				outapp->WriteUInt8(Item->IsEquipable(c->GetBaseRace(), c->GetBaseClass()) ? 1 : 0);
				outapp->WriteString(Item->Name);
			} else {
				outapp->WriteUInt8(0); // empty
			}
		}

		outapp->size = outapp->GetWritePosition(); // truncate to used size
		c->FastQueuePacket(&outapp);
		return;
	}

	for(int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
	{
		if(guild_bank->Items.DepositArea[i].ItemID > 0)
		{
			const EQ::ItemData *Item = database.GetItem(guild_bank->Items.DepositArea[i].ItemID);

			if(!Item)
				continue;

			auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

			GuildBankItemUpdate_Struct *gbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

			if(!Item->Stackable)
				gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon, 1,
						guild_bank->Items.DepositArea[i].Permissions, 0, 0);
			else
			{
				if(guild_bank->Items.DepositArea[i].Quantity == Item->StackSize)
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon,
							guild_bank->Items.DepositArea[i].Quantity, guild_bank->Items.DepositArea[i].Permissions, 0, 0);
				else
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon,
							guild_bank->Items.DepositArea[i].Quantity, guild_bank->Items.DepositArea[i].Permissions, 1, 0);
			}

			strn0cpy(gbius->ItemName, Item->Name, sizeof(gbius->ItemName));

			strn0cpy(gbius->Donator, guild_bank->Items.DepositArea[i].Donator, sizeof(gbius->Donator));

			strn0cpy(gbius->WhoFor, guild_bank->Items.DepositArea[i].WhoFor, sizeof(gbius->WhoFor));

			c->FastQueuePacket(&outapp);
		}
	}

	for(int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
	{
		if(guild_bank->Items.MainArea[i].ItemID > 0)
		{
			const EQ::ItemData *Item = database.GetItem(guild_bank->Items.MainArea[i].ItemID);

			if(!Item)
				continue;

			bool Useable = Item->IsEquipable(c->GetBaseRace(), c->GetBaseClass());

			auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

			GuildBankItemUpdate_Struct *gbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

			if(!Item->Stackable)
				gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon, 1,
						guild_bank->Items.MainArea[i].Permissions, 0, Useable);
			else
			{
				if(guild_bank->Items.MainArea[i].Quantity == Item->StackSize)
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon,
							guild_bank->Items.MainArea[i].Quantity, guild_bank->Items.MainArea[i].Permissions, 0, Useable);
				else
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon,
							guild_bank->Items.MainArea[i].Quantity, guild_bank->Items.MainArea[i].Permissions, 1, Useable);
			}

			strn0cpy(gbius->ItemName, Item->Name, sizeof(gbius->ItemName));

			strn0cpy(gbius->Donator, guild_bank->Items.MainArea[i].Donator, sizeof(gbius->Donator));

			strn0cpy(gbius->WhoFor, guild_bank->Items.MainArea[i].WhoFor, sizeof(gbius->WhoFor));

			c->FastQueuePacket(&outapp);
		}
	}
}
bool GuildBankManager::IsAreaFull(uint32 GuildID, uint16 Area)
{
	auto Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return true;

	GuildBankItem* BankArea = nullptr;

	int AreaSize = 0;

	if(Area == GuildBankMainArea)
	{
		BankArea = &(*Iterator)->Items.MainArea[0];

		AreaSize = GUILD_BANK_MAIN_AREA_SIZE;
	}
	else
	{
		BankArea = &(*Iterator)->Items.DepositArea[0];

		AreaSize = GUILD_BANK_DEPOSIT_AREA_SIZE;
	}

	for(int i = 0; i < AreaSize; ++i)
		if(BankArea[i].ItemID == 0)
			return false;

	return true;
}

bool GuildBankManager::AddItem(uint32 GuildID, uint8 Area, uint32 ItemID, int32 QtyOrCharges, const char *Donator, uint8 Permissions, const char *WhoFor)
{
	auto Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
	{
		LogError("Unable to find guild bank for guild ID [{}]", GuildID);

		return false;
	}

	GuildBankItem* BankArea = nullptr;

	int AreaSize = 0;

	if(Area == GuildBankMainArea)
	{
		BankArea = &(*Iterator)->Items.MainArea[0];

		AreaSize = GUILD_BANK_MAIN_AREA_SIZE;
	}
	else
	{
		BankArea = &(*Iterator)->Items.DepositArea[0];

		AreaSize = GUILD_BANK_DEPOSIT_AREA_SIZE;
	}

	int Slot = -1;

	for(int i = 0; i < AreaSize; ++i)
	{
		if(BankArea[i].ItemID == 0)
		{
			BankArea[i].ItemID = ItemID;

			BankArea[i].Quantity = QtyOrCharges;

			strn0cpy(BankArea[i].Donator, Donator, sizeof(BankArea[i].Donator));

			BankArea[i].Permissions = Permissions;

			strn0cpy(BankArea[i].WhoFor, WhoFor, sizeof(BankArea[i].WhoFor));

			Slot = i;

			break;
		}
	}

	if(Slot < 0)
	{
		LogError("No space to add item to the guild bank");

		return false;
	}

	std::string query = StringFormat("INSERT INTO `guild_bank` "
                                    "(`guildid`, `area`, `slot`, `itemid`, `qty`, `donator`, `permissions`, `WhoFor`) "
                                    "VALUES (%i, %i, %i, %i, %i, '%s', %i, '%s')",
                                    GuildID, Area, Slot, ItemID, QtyOrCharges, Donator, Permissions, WhoFor);
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		return false;
	}

	const EQ::ItemData *Item = database.GetItem(ItemID);

	GuildBankItemUpdate_Struct gbius;

	if(!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, Slot, Area, 1, ItemID, Item->Icon, Item->Stackable ? QtyOrCharges : 1, Permissions, 0, 0);
	else
	{
		if(QtyOrCharges == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, Slot, Area, 1, ItemID, Item->Icon, Item->Stackable ? QtyOrCharges : 1, Permissions, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, Slot, Area, 1, ItemID, Item->Icon, Item->Stackable ? QtyOrCharges : 1, Permissions, 1, 0);
	}

	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	strn0cpy(gbius.Donator, Donator, sizeof(gbius.Donator));

	strn0cpy(gbius.WhoFor, WhoFor, sizeof(gbius.WhoFor));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, GuildID);

	return true;
}

int GuildBankManager::Promote(uint32 guildID, int slotID)
{
	if((slotID < 0) || (slotID > (GUILD_BANK_DEPOSIT_AREA_SIZE - 1)))
		return -1;

	auto iter = GetGuildBank(guildID);

	if(iter == Banks.end())
		return -1;

	if((*iter)->Items.DepositArea[slotID].ItemID == 0)
		return -1;

	int mainSlot = -1;

	for(int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
		if((*iter)->Items.MainArea[i].ItemID == 0) {
			mainSlot = i;
			break;
		}

	if(mainSlot == -1)
		return -1;

	(*iter)->Items.MainArea[mainSlot].ItemID = (*iter)->Items.DepositArea[slotID].ItemID;
	(*iter)->Items.MainArea[mainSlot].Quantity = (*iter)->Items.DepositArea[slotID].Quantity;
    (*iter)->Items.MainArea[mainSlot].Permissions = (*iter)->Items.DepositArea[slotID].Permissions;

	strn0cpy((*iter)->Items.MainArea[mainSlot].Donator, (*iter)->Items.DepositArea[slotID].Donator, sizeof((*iter)->Items.MainArea[mainSlot].Donator));
	strn0cpy((*iter)->Items.MainArea[mainSlot].WhoFor, (*iter)->Items.DepositArea[slotID].WhoFor, sizeof((*iter)->Items.MainArea[mainSlot].WhoFor));

	std::string query = StringFormat("UPDATE `guild_bank` SET `area` = 1, `slot` = %i "
                                    "WHERE `guildid` = %i AND `area` = 0 AND `slot` = %i "
                                    "LIMIT 1", mainSlot, guildID, slotID);
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
		return -1;
	}

	(*iter)->Items.DepositArea[slotID].ItemID = 0;

	const EQ::ItemData *Item = database.GetItem((*iter)->Items.MainArea[mainSlot].ItemID);

	GuildBankItemUpdate_Struct gbius;

	if(!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, mainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon, 1, 0, 0, 0);
	else
	{
		if((*iter)->Items.MainArea[mainSlot].Quantity == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, mainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*iter)->Items.MainArea[mainSlot].Quantity, 0, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, mainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*iter)->Items.MainArea[mainSlot].Quantity, 0, 1, 0);
	}

	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, guildID);

	gbius.Init(GuildBankItemUpdate, 1, slotID, GuildBankDepositArea, 0, 0, 0, 0, 0, 0, 0);

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, guildID);

	return mainSlot;
}

void GuildBankManager::SetPermissions(uint32 guildID, uint16 slotID, uint32 permissions, const char *memberName)
{
	if((slotID > (GUILD_BANK_MAIN_AREA_SIZE - 1)))
		return;

	auto iter = GetGuildBank(guildID);

	if(iter == Banks.end())
		return;

	if((*iter)->Items.MainArea[slotID].ItemID == 0)
		return;

	std::string query = StringFormat("UPDATE `guild_bank` SET `permissions` = %i, `whofor` = '%s' "
                                "WHERE `guildid` = %i AND `area` = 1 AND `slot` = %i LIMIT 1",
                                permissions, memberName, guildID, slotID);
    auto results = database.QueryDatabase(query);
	if(!results.Success())
	{
		return;
	}

	(*iter)->Items.MainArea[slotID].Permissions = permissions;

	if(permissions == GuildBankSingleMember)
		strn0cpy((*iter)->Items.MainArea[slotID].WhoFor, memberName, sizeof((*iter)->Items.MainArea[slotID].WhoFor));
	else
		(*iter)->Items.MainArea[slotID].WhoFor[0] = '\0';

	const EQ::ItemData *Item = database.GetItem((*iter)->Items.MainArea[slotID].ItemID);

	GuildBankItemUpdate_Struct gbius;

	if(!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, slotID, GuildBankMainArea, 1, Item->ID, Item->Icon, 1, (*iter)->Items.MainArea[slotID].Permissions, 0, 0);
	else
	{
		if((*iter)->Items.MainArea[slotID].Quantity == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, slotID, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*iter)->Items.MainArea[slotID].Quantity, (*iter)->Items.MainArea[slotID].Permissions, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, slotID, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*iter)->Items.MainArea[slotID].Quantity, (*iter)->Items.MainArea[slotID].Permissions, 1, 0);
	}


	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	strn0cpy(gbius.WhoFor, (*iter)->Items.MainArea[slotID].WhoFor, sizeof(gbius.WhoFor));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, guildID);
}

EQ::ItemInstance* GuildBankManager::GetItem(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity)
{
	auto Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return nullptr;

	GuildBankItem* BankArea = nullptr;

	EQ::ItemInstance* inst = nullptr;

	if(Area == GuildBankDepositArea)
	{
		if((SlotID > (GUILD_BANK_DEPOSIT_AREA_SIZE - 1)))
			return nullptr;

		inst = database.CreateItem((*Iterator)->Items.DepositArea[SlotID].ItemID);

		if(!inst)
			return nullptr;

		BankArea = &(*Iterator)->Items.DepositArea[0];
	}
	else
	{

		if((SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1)))
			return nullptr;

		inst = database.CreateItem((*Iterator)->Items.MainArea[SlotID].ItemID);

		if(!inst)
			return nullptr;

		BankArea = &(*Iterator)->Items.MainArea[0];
	}

	if(!inst->IsStackable())
		inst->SetCharges(BankArea[SlotID].Quantity);
	else
	{
		if(Quantity <= BankArea[SlotID].Quantity)
			inst->SetCharges(Quantity);
		else
			inst->SetCharges(BankArea[SlotID].Quantity);
	}

	return inst;
}

bool GuildBankManager::HasItem(uint32 GuildID, uint32 ItemID)
{
	auto Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return false;

	for(int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
		if((*Iterator)->Items.MainArea[i].ItemID == ItemID)
			return true;

	for(int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
		if((*Iterator)->Items.DepositArea[i].ItemID == ItemID)
			return true;

	return false;
}

std::list<GuildBank*>::iterator GuildBankManager::GetGuildBank(uint32 GuildID)
{
	auto Iterator = Banks.begin();

	while(Iterator != Banks.end())
	{
		if((*Iterator)->GuildID == GuildID)
			break;

		++Iterator;
	}

	return Iterator;
}

bool GuildBankManager::DeleteItem(uint32 guildID, uint16 area, uint16 slotID, uint32 quantity)
{
	auto iter = GetGuildBank(guildID);

	if(iter == Banks.end())
		return false;

	GuildBankItem* BankArea = nullptr;

	if(area == GuildBankMainArea)
	{
		if(slotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
			return false;

		BankArea = &(*iter)->Items.MainArea[0];
	} else {
		if(slotID > (GUILD_BANK_DEPOSIT_AREA_SIZE - 1))
			return false;

		BankArea = &(*iter)->Items.DepositArea[0];
	}

	bool deleted = true;

	const EQ::ItemData *Item = database.GetItem(BankArea[slotID].ItemID);

	if(!Item->Stackable || (quantity >= BankArea[slotID].Quantity)) {
        std::string query = StringFormat("DELETE FROM `guild_bank` WHERE `guildid` = %i "
                                        "AND `area` = %i AND `slot` = %i LIMIT 1",
                                        guildID, area, slotID);
        auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			return false;
		}

		BankArea[slotID].ItemID = 0;

	} else {
		std::string query = StringFormat("UPDATE `guild_bank` SET `qty` = %i WHERE `guildid` = %i "
                                        "AND `area` = %i AND `slot` = %i LIMIT 1",
                                        BankArea[slotID].Quantity - quantity, guildID, area, slotID);
        auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			return false;
		}

		BankArea[slotID].Quantity -= quantity;

		deleted = false;
	}

	GuildBankItemUpdate_Struct gbius;

	if(!deleted)
	{
		gbius.Init(GuildBankItemUpdate, 1, slotID, area, 1, Item->ID, Item->Icon, BankArea[slotID].Quantity, BankArea[slotID].Permissions, 1, 0);

		strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

		strn0cpy(gbius.WhoFor, BankArea[slotID].WhoFor, sizeof(gbius.WhoFor));
	}
	else
		gbius.Init(GuildBankItemUpdate, 1, slotID, area, 0, 0, 0, 0, 0, 0, 0);

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, guildID);

	return true;

}

bool GuildBankManager::MergeStacks(uint32 GuildID, uint16 SlotID)
{
	if(SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	auto Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return false;

	GuildBankItem* BankArea = &(*Iterator)->Items.MainArea[0];

	if(BankArea[SlotID].ItemID == 0)
		return false;

	const EQ::ItemData *Item = database.GetItem(BankArea[SlotID].ItemID);

	if(!Item->Stackable)
		return false;

	uint32 ItemID = BankArea[SlotID].ItemID;

	for(int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE - 1; ++i)
	{
		if(BankArea[i].ItemID != ItemID)
			continue;

		if(BankArea[i].Quantity == Item->StackSize)
			continue;

		bool Merged = false;

		for(int j = i + 1; j < GUILD_BANK_MAIN_AREA_SIZE; ++j)
		{
			if(BankArea[j].ItemID != ItemID)
				continue;

			if(BankArea[j].Permissions != BankArea[i].Permissions)
				continue;

			if(BankArea[i].Permissions == 1)
				if(strncmp(BankArea[i].WhoFor, BankArea[j].WhoFor, sizeof(BankArea[i].WhoFor)))
					continue;

			if((BankArea[i].Quantity + BankArea[j].Quantity) <= Item->StackSize)
			{
				BankArea[i].Quantity += BankArea[j].Quantity;

				DeleteItem(GuildID, GuildBankMainArea, j, BankArea[j].Quantity);

				Merged = true;

				if(BankArea[i].Quantity == Item->StackSize)
					break;
			}
			else
			{
				uint32 QuantityToMove = Item->StackSize - BankArea[i].Quantity;

				DeleteItem(GuildID, GuildBankMainArea, j, QuantityToMove);

				BankArea[i].Quantity = Item->StackSize;

				Merged = true;

				break;
			}
		}

		if(Merged)
		{
			UpdateItemQuantity(GuildID, GuildBankMainArea, i, BankArea[i].Quantity);

			GuildBankItemUpdate_Struct gbius;

			if(BankArea[i].Quantity == Item->StackSize)
				gbius.Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, ItemID, Item->Icon, BankArea[i].Quantity, BankArea[i].Permissions, 0, 0);
			else
				gbius.Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, ItemID, Item->Icon, BankArea[i].Quantity, BankArea[i].Permissions, 1, 0);

			strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

			strn0cpy(gbius.WhoFor, BankArea[i].WhoFor, sizeof(gbius.WhoFor));

			entity_list.QueueClientsGuildBankItemUpdate(&gbius, GuildID);
		}

	}

	return true;
}

bool GuildBankManager::SplitStack(uint32 GuildID, uint16 SlotID, uint32 Quantity)
{
	if(SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	auto Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return false;

	if(IsAreaFull(GuildID, GuildBankMainArea))
		return false;

	GuildBankItem* BankArea = &(*Iterator)->Items.MainArea[0];

	if(BankArea[SlotID].ItemID == 0)
		return false;

	if(BankArea[SlotID].Quantity <= Quantity || Quantity == 0)
		return false;

	const EQ::ItemData *Item = database.GetItem(BankArea[SlotID].ItemID);

	if(!Item->Stackable)
		return false;

	AddItem(GuildID, GuildBankMainArea, BankArea[SlotID].ItemID, Quantity, "", BankArea[SlotID].Permissions, BankArea[SlotID].WhoFor);

	DeleteItem(GuildID, GuildBankMainArea, SlotID, Quantity);

	return true;
}

void GuildBankManager::UpdateItemQuantity(uint32 guildID, uint16 area, uint16 slotID, uint32 quantity)
{
	// Helper method for MergeStacks. Assuming all passed parameters are valid.
	//
	std::string query = StringFormat("UPDATE `guild_bank` SET `qty` = %i "
                                    "WHERE `guildid` = %i AND `area` = %i "
                                    "AND `slot` = %i LIMIT 1",
                                    quantity, guildID, area, slotID);
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		return;
	}

}

bool GuildBankManager::AllowedToWithdraw(uint32 GuildID, uint16 Area, uint16 SlotID, const char *Name)
{
	// Is a none-Guild Banker allowed to withdraw the item at this slot ?
	// This is really here for anti-hacking measures, as the client should not request an item it does not have permission to withdraw.
	//
	if(SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	auto Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return false;

	if(Area != GuildBankMainArea)
		return false;

	uint8 Permissions = (*Iterator)->Items.MainArea[SlotID].Permissions;

	if(Permissions == GuildBankBankerOnly)
		return false;

	if(Permissions != GuildBankSingleMember)	// Public or Public-If-Useable (should really check if item is useable)
		return true;

	if(!strncmp((*Iterator)->Items.MainArea[SlotID].WhoFor, Name, sizeof((*Iterator)->Items.MainArea[SlotID].WhoFor)))
		return true;

	return false;
}

/*================== GUILD APPROVAL ========================*/

bool GuildApproval::ProcessApproval()
{
	if(owner && owner->GuildID() != 0)
	{
		owner->Message(Chat::NPCQuestSay,"You are already in a guild! Guild request deleted.");
		return false;
	}
	if(deletion_timer->Check() || !owner)
	{
		if(owner)
			owner->Message(Chat::White,"You took too long! Your guild request has been deleted.");
		return false;
	}

	return true;
}

GuildApproval::GuildApproval(const char* guildname, Client* owner,uint32 id)
{
	std::string founders;
	database.GetVariable("GuildCreation", founders);
	uint8 tmp = atoi(founders.c_str());
	deletion_timer = new Timer(1800000);
	strcpy(guild,guildname);
	owner = owner;
	refid = id;
	if(owner)
		owner->Message(Chat::White,"You can now start getting your guild approved, tell your %i members to #guildapprove %i, you have 30 minutes to create your guild.",tmp,GetID());
	for(int i=0;i<tmp;i++)
		members[i] = 0;
}

GuildApproval::~GuildApproval()
{
	safe_delete(deletion_timer);
}

bool GuildApproval::AddMemberApproval(Client* addition)
{
	std::string founders;
	database.GetVariable("GuildCreation", founders);
	uint8 tmp = atoi(founders.c_str());
	for(int i=0;i<tmp;i++)
	{
		if(members[i] && members[i] == addition)
			return false;
	}

	for(int i=0;i<tmp;i++)
	{
		if(!members[i])
		{
			members[i] = addition;
			int z=0;
			for(int i=0;i<tmp;i++)
			{
				if(members[i])
					z++;
			}
			if(z==tmp)
				GuildApproved();

			return true;
		}
	}
	return false;
}

void GuildApproval::ApprovedMembers(Client* requestee)
{
	std::string founders;
	database.GetVariable("GuildCreation", founders);
	uint8 tmp = atoi(founders.c_str());
	for(int i=0;i<tmp;i++)
	{
		if(members[i])
			requestee->Message(Chat::White,"%i: %s",i,members[i]->GetName());
	}
}

void GuildApproval::GuildApproved()
{
	char petitext[PBUFFER] = "A new guild was founded! Guildname: ";
	char gmembers[MBUFFER] = " ";

	if(!owner)
		return;
	std::string founders;
	database.GetVariable("GuildCreation", founders);
	uint8 tmp = atoi(founders.c_str());
	uint32 tmpeq = guild_mgr.CreateGuild(guild, owner->CharacterID());
	guild_mgr.SetGuild(owner->CharacterID(),tmpeq,2);
	owner->SendAppearancePacket(AT_GuildID,true,false);
	for(int i=0;i<tmp;i++)
	{
		if(members[i])
			{
			owner->Message(Chat::White, "%s",members[i]->GetName());
			owner->Message(Chat::White, "%i",members[i]->CharacterID());
			guild_mgr.SetGuild(members[i]->CharacterID(),tmpeq,0);
			size_t len = MBUFFER - strlen(gmembers)+1;
			strncat(gmembers," ",len);
			strncat(gmembers,members[i]->GetName(),len);
			}
	}
	size_t len = PBUFFER - strlen(petitext)+1;
	strncat(petitext,guild,len);
	strncat(petitext," Leader: ",len);
	strncat(petitext,owner->CastToClient()->GetName(),len);
	strncat(petitext," Members:",len);
	strncat(petitext,gmembers,len);
	auto pet = new Petition(owner->CastToClient()->CharacterID());
	pet->SetAName(owner->CastToClient()->AccountName());
	pet->SetClass(owner->CastToClient()->GetClass());
	pet->SetLevel(owner->CastToClient()->GetLevel());
	pet->SetCName(owner->CastToClient()->GetName());
	pet->SetRace(owner->CastToClient()->GetRace());
	pet->SetLastGM("");
	pet->SetCName(owner->CastToClient()->GetName()); //aza77 is this really 2 times needed ??
	pet->SetPetitionText(petitext);
	pet->SetZone(zone->GetZoneID());
	pet->SetUrgency(0);
	petition_list.AddPetition(pet);
	database.InsertPetitionToDB(pet);
	petition_list.UpdateGMQueue();
	petition_list.UpdateZoneListQueue();
	worldserver.SendEmoteMessage(
		0,
		0,
		AccountStatus::QuestTroupe,
		Chat::Yellow,
		fmt::format(
			"{} has made a petition. ID: {}",
			owner->CastToClient()->GetName(),
			pet->GetID()
		).c_str()
	);
	auto pack = new ServerPacket;
	pack->opcode = ServerOP_RefreshGuild;
	pack->size = tmp;
	pack->pBuffer = new uchar[pack->size];
	memcpy(pack->pBuffer, &tmpeq, 4);
	worldserver.SendPacket(pack);
	safe_delete(pack);
	owner->Message(Chat::White, "Your guild was created.");
	owner = 0;
}

