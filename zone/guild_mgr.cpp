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
#include "../common/debug.h"
#include "guild_mgr.h"
#include "zonedb.h"
#include "worldserver.h"
#include "../common/servertalk.h"
#include "../common/StringUtil.h"
#include "client.h"
#include "entity.h"

/*

CREATE TABLE guilds (
	id MEDIUMINT UNSIGNED NOT nullptr,
	name VARCHAR(32) NOT nullptr,
	leader int NOT nullptr,
	minstatus SMALLINT NOT nullptr,
	tribute INT UNSIGNED NOT nullptr,
	motd TEXT NOT nullptr DEFAULT '',
	PRIMARY KEY(id),
	UNIQUE KEY(name),
	UNIQUE KEY(leader)
);

CREATE TABLE guild_ranks (
	guild_id MEDIUMINT UNSIGNED NOT nullptr,
	rank TINYINT UNSIGNED NOT nullptr,
	title VARCHAR(128) NOT nullptr,
	can_hear TINYINT UNSIGNED NOT nullptr,
	can_speak TINYINT UNSIGNED NOT nullptr,
	can_invite TINYINT UNSIGNED NOT nullptr,
	can_remove TINYINT UNSIGNED NOT nullptr,
	can_promote TINYINT UNSIGNED NOT nullptr,
	can_demote TINYINT UNSIGNED NOT nullptr,
	can_motd TINYINT UNSIGNED NOT nullptr,
	can_warpeace TINYINT UNSIGNED NOT nullptr,
	PRIMARY KEY(guild_id,rank)
);

# guild1 < guild2 by definition.
CREATE TABLE guild_relations (
	guild1 MEDIUMINT UNSIGNED NOT nullptr,
	guild2 MEDIUMINT UNSIGNED NOT nullptr,
	relation TINYINT NOT nullptr,
	PRIMARY KEY(guild1, guild1)
);

CREATE TABLE guild_members (
	char_id INT NOT nullptr,
	guild_id MEDIUMINT UNSIGNED NOT nullptr,
	rank TINYINT UNSIGNED NOT nullptr,
	tribute_enable TINYINT UNSIGNED NOT nullptr DEFAULT 0,
	total_tribute INT UNSIGNED NOT nullptr DEFAULT 0,
	last_tribute INT UNSIGNED NOT nullptr DEFAULT 0,
	banker TINYINT UNSIGNED NOT nullptr DEFAULT 0,
	public_note TEXT NOT nullptr DEFAULT '',
	PRIMARY KEY(char_id)
);


*/


ZoneGuildManager guild_mgr;
GuildBankManager *GuildBanks;

extern WorldServer worldserver;
extern volatile bool ZoneLoaded;

void ZoneGuildManager::SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) {
	_log(GUILDS__REFRESH, "Sending guild refresh for %d to world, changes: name=%d, motd=%d, rank=d, relation=%d", guild_id, name, motd, rank, relation);
	ServerPacket* pack = new ServerPacket(ServerOP_RefreshGuild, sizeof(ServerGuildRefresh_Struct));
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
		_log(GUILDS__REFRESH, "Guild lookup for char %d when sending char refresh.", charid);

		CharGuildInfo gci;
		if(!GetCharInfo(charid, gci)) {
			guild_id = GUILD_NONE;
		} else {
			guild_id = gci.guild_id;
		}
	}

	_log(GUILDS__REFRESH, "Sending char refresh for %d from guild %d to world", charid, guild_id);

	ServerPacket* pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
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

	ServerPacket* pack = new ServerPacket(ServerOP_GuildRankUpdate, sizeof(ServerGuildRankUpdate_Struct));

	ServerGuildRankUpdate_Struct *sgrus = (ServerGuildRankUpdate_Struct*)pack->pBuffer;

	sgrus->GuildID = gci.guild_id;
	strn0cpy(sgrus->MemberName, gci.char_name.c_str(), sizeof(sgrus->MemberName));
	sgrus->Rank = gci.rank;
	sgrus->Banker = gci.banker + (gci.alt * 2);

	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::SendGuildDelete(uint32 guild_id) {
	_log(GUILDS__REFRESH, "Sending guild delete for guild %d to world", guild_id);
	ServerPacket* pack = new ServerPacket(ServerOP_DeleteGuild, sizeof(ServerGuildID_Struct));
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
	for(; cur != end; cur++) {
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
	for(; cur != end; cur++) {
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
	c->Message(0, "Listing guilds on the server:");
	char leadername[64];
	std::map<uint32, GuildInfo *>::const_iterator cur, end;
	cur = m_guilds.begin();
	end = m_guilds.end();
	int r = 0;
	for(; cur != end; cur++) {
		leadername[0] = '\0';
		database.GetCharName(cur->second->leader_char_id, leadername);
		if (leadername[0] == '\0')
			c->Message(0, "  Guild #%i <%s>", cur->first, cur->second->name.c_str());
		else
			c->Message(0, "  Guild #%i <%s> Leader: %s", cur->first, cur->second->name.c_str(), leadername);
		r++;
	}
	c->Message(0, "%i guilds listed.", r);
}


void ZoneGuildManager::DescribeGuild(Client *c, uint32 guild_id) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		c->Message(0, "Guild %d not found.", guild_id);
		return;
	}

	const GuildInfo *info = res->second;

	c->Message(0, "Guild info DB# %i <%s>", guild_id, info->name.c_str());

	char leadername[64];
	database.GetCharName(info->leader_char_id, leadername);
	c->Message(0, "Guild Leader: %s", leadername);

	char permbuffer[256];
	uint8 i;
	for (i = 0; i <= GUILD_MAX_RANK; i++) {
		char *permptr = permbuffer;
		uint8 r;
		for(r = 0; r < _MaxGuildAction; r++)
			permptr += sprintf(permptr, "  %s: %c", GuildActionNames[r], info->ranks[i].permissions[r]?'Y':'N');

		c->Message(0, "Rank %i: %s", i, info->ranks[i].name.c_str());
		c->Message(0, "Permissions: %s", permbuffer);
	}

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
			_log(GUILDS__ERROR, "Received ServerOP_RefreshGuild of incorrect size %d, expected %d", pack->size, sizeof(ServerGuildRefresh_Struct));
			return;
		}
		ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;

		_log(GUILDS__REFRESH, "Received guild refresh from world for %d, changes: name=%d, motd=%d, rank=%d, relation=%d", s->guild_id, s->name_change, s->motd_change, s->rank_change, s->relation_change);

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
			_log(GUILDS__ERROR, "Received ServerOP_RefreshGuild of incorrect size %d, expected %d", pack->size, sizeof(ServerGuildCharRefresh_Struct));
			return;
		}
		ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;

		_log(GUILDS__REFRESH, "Received guild member refresh from world for char %d from guild %d", s->char_id, s->guild_id);

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
		}


		break;
	}

	case ServerOP_GuildRankUpdate:
	{
		if(ZoneLoaded)
		{
			if(pack->size != sizeof(ServerGuildRankUpdate_Struct))
			{
				_log(GUILDS__ERROR, "Received ServerOP_RankUpdate of incorrect size %d, expected %d",
					pack->size, sizeof(ServerGuildRankUpdate_Struct));

				return;
			}

			ServerGuildRankUpdate_Struct *sgrus = (ServerGuildRankUpdate_Struct*)pack->pBuffer;

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));

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
			_log(GUILDS__ERROR, "Received ServerOP_DeleteGuild of incorrect size %d, expected %d", pack->size, sizeof(ServerGuildID_Struct));
			return;
		}
		ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;

		_log(GUILDS__REFRESH, "Received guild delete from world for guild %d", s->guild_id);

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

		if(ZoneLoaded)
		{
			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(GuildMemberUpdate_Struct));

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
		if (ZoneLoaded)
		{
			char *Buffer = (char *)pack->pBuffer;

			uint32 FromID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
			uint32 Count = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
			Client *c = entity_list.GetClientByCharID(FromID);

			if (!c || !c->IsInAGuild())
			{
				_log(GUILDS__ERROR,"Invalid Client or not in guild. ID=%i", FromID);
				break;
			}
			_log(GUILDS__IN_PACKETS,"Processing ServerOP_OnlineGuildMembersResponse");
			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(GuildMemberUpdate_Struct));
			GuildMemberUpdate_Struct *gmus = (GuildMemberUpdate_Struct*)outapp->pBuffer;
			char Name[64];
			gmus->LastSeen = time(nullptr);
			gmus->InstanceID = 0;
			gmus->GuildID = c->GuildID();
			for (int i=0;i<Count;i++)
			{
				// Just make the packet once and swap out name/zone and send
				VARSTRUCT_DECODE_STRING(Name, Buffer);
				strn0cpy(gmus->MemberName, Name, sizeof(gmus->MemberName));
				gmus->ZoneID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
				_log(GUILDS__OUT_PACKETS,"Sending OP_GuildMemberUpdate to %i. Name=%s ZoneID=%i",FromID,Name,gmus->ZoneID);
				c->QueuePacket(outapp);
			}
			safe_delete(outapp);

		}
		break;

	case ServerOP_LFGuildUpdate:
	{
		if(ZoneLoaded)
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

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_GuildToggle_Struct));

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
	ServerPacket* pack = new ServerPacket(ServerOP_GuildMemberUpdate, sizeof(ServerGuildMemberUpdate_Struct));

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
	ServerPacket* pack = new ServerPacket(ServerOP_RequestOnlineGuildMembers, sizeof(ServerRequestOnlineGuildMembers_Struct));
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
	GuildApproval* tmp = new GuildApproval(guildname,owner,GetFreeID());
	list.Insert(tmp);
}

void ZoneGuildManager::AddMemberApproval(uint32 refid,Client* name)
{
	GuildApproval* tmp = FindGuildByIDApproval(refid);
	if(tmp != 0)
	{
		if(!tmp->AddMemberApproval(name))
			name->Message(0,"Unable to add to list.");
		else
		{
			name->Message(0,"Added to list.");
		}
	}
	else
		name->Message(0,"Unable to find guild reference id.");
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
	std::list<GuildBank*>::iterator Iterator = Banks.begin();

	while(Iterator != Banks.end())
	{
		safe_delete(*Iterator);

		++Iterator;
	}
}

bool GuildBankManager::Load(uint32 GuildID)
{
	const char *LoadQuery = "SELECT `area`, `slot`, `itemid`, `qty`, `donator`, `permissions`, `whofor` from `guild_bank` "
				"WHERE `guildid` = %i";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	MYSQL_RES *result;

	MYSQL_ROW row;

	if(database.RunQuery(query, MakeAnyLenString(&query, LoadQuery, GuildID), errbuf, &result))
	{
		GuildBank *Bank = new GuildBank;

		Bank->GuildID = GuildID;

		for(int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
			Bank->Items.MainArea[i].ItemID = 0;

		for(int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
			Bank->Items.DepositArea[i].ItemID = 0;

		char Donator[64], WhoFor[64];

		while((row = mysql_fetch_row(result)))
		{
			int Area = atoi(row[0]);

			int Slot = atoi(row[1]);

			int ItemID = atoi(row[2]);

			int Qty = atoi(row[3]);

			if(row[4])
				strn0cpy(Donator, row[4], sizeof(Donator));
			else
				Donator[0] = '\0';

			int Permissions = atoi(row[5]);

			if(row[6])
				strn0cpy(WhoFor, row[6], sizeof(WhoFor));
			else
				WhoFor[0] = '\0';

			if(Area == GuildBankMainArea)
			{
				if((Slot >= 0) && (Slot < GUILD_BANK_MAIN_AREA_SIZE))
				{
					Bank->Items.MainArea[Slot].ItemID = ItemID;

					Bank->Items.MainArea[Slot].Quantity = Qty;

					strn0cpy(Bank->Items.MainArea[Slot].Donator, Donator, sizeof(Donator));

					Bank->Items.MainArea[Slot].Permissions = Permissions;

					strn0cpy(Bank->Items.MainArea[Slot].WhoFor, WhoFor, sizeof(WhoFor));
				}
			}
			else
			{
				if((Slot >= 0 ) && (Slot < GUILD_BANK_DEPOSIT_AREA_SIZE))
				{
					Bank->Items.DepositArea[Slot].ItemID = ItemID;

					Bank->Items.DepositArea[Slot].Quantity = Qty;

					strn0cpy(Bank->Items.DepositArea[Slot].Donator, Donator, sizeof(Donator));

					Bank->Items.DepositArea[Slot].Permissions = Permissions;

					strn0cpy(Bank->Items.DepositArea[Slot].WhoFor, WhoFor, sizeof(WhoFor));
				}
			}

		}
		mysql_free_result(result);

		safe_delete_array(query);

		Banks.push_back(Bank);
	}
	else
	{
		_log(GUILDS__BANK_ERROR, "Error Loading guild bank: %s, %s", query, errbuf);

		safe_delete_array(query);

		return false;
	}

	return true;

}

bool GuildBankManager::IsLoaded(uint32 GuildID)
{
	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	return (Iterator != Banks.end());
}

void GuildBankManager::SendGuildBank(Client *c)
{
	if(!c || !c->IsInAGuild())
		return;

	if(!IsLoaded(c->GuildID()))
		Load(c->GuildID());

	std::list<GuildBank*>::iterator Iterator = GetGuildBank(c->GuildID());

	if(Iterator == Banks.end())
	{
		_log(GUILDS__BANK_ERROR, "Unable to find guild bank for guild ID %i", c->GuildID());

		return;
	}

	for(int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
	{
		if((*Iterator)->Items.DepositArea[i].ItemID > 0)
		{
			const Item_Struct *Item = database.GetItem((*Iterator)->Items.DepositArea[i].ItemID);

			if(!Item)
				continue;

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

			GuildBankItemUpdate_Struct *gbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

			if(!Item->Stackable)
				gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon, 1,
						(*Iterator)->Items.DepositArea[i].Permissions, 0, 0);
			else
			{
				if((*Iterator)->Items.DepositArea[i].Quantity == Item->StackSize)
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon,
							(*Iterator)->Items.DepositArea[i].Quantity, (*Iterator)->Items.DepositArea[i].Permissions, 0, 0);
				else
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon,
							(*Iterator)->Items.DepositArea[i].Quantity, (*Iterator)->Items.DepositArea[i].Permissions, 1, 0);
			}

			strn0cpy(gbius->ItemName, Item->Name, sizeof(gbius->ItemName));

			strn0cpy(gbius->Donator, (*Iterator)->Items.DepositArea[i].Donator, sizeof(gbius->Donator));

			strn0cpy(gbius->WhoFor, (*Iterator)->Items.DepositArea[i].WhoFor, sizeof(gbius->WhoFor));

			c->FastQueuePacket(&outapp);
		}
	}

	for(int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
	{
		if((*Iterator)->Items.MainArea[i].ItemID > 0)
		{
			const Item_Struct *Item = database.GetItem((*Iterator)->Items.MainArea[i].ItemID);

			if(!Item)
				continue;

			bool Useable = Item->IsEquipable(c->GetBaseRace(), c->GetBaseClass());

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

			GuildBankItemUpdate_Struct *gbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

			if(!Item->Stackable)
				gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon, 1,
						(*Iterator)->Items.MainArea[i].Permissions, 0, Useable);
			else
			{
				if((*Iterator)->Items.MainArea[i].Quantity == Item->StackSize)
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon,
							(*Iterator)->Items.MainArea[i].Quantity, (*Iterator)->Items.MainArea[i].Permissions, 0, Useable);
				else
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon,
							(*Iterator)->Items.MainArea[i].Quantity, (*Iterator)->Items.MainArea[i].Permissions, 1, Useable);
			}

			strn0cpy(gbius->ItemName, Item->Name, sizeof(gbius->ItemName));

			strn0cpy(gbius->Donator, (*Iterator)->Items.MainArea[i].Donator, sizeof(gbius->Donator));

			strn0cpy(gbius->WhoFor, (*Iterator)->Items.MainArea[i].WhoFor, sizeof(gbius->WhoFor));

			c->FastQueuePacket(&outapp);
		}
	}
}
bool GuildBankManager::IsAreaFull(uint32 GuildID, uint16 Area)
{
	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

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
	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
	{
		_log(GUILDS__BANK_ERROR, "Unable to find guild bank for guild ID %i", GuildID);

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
		_log(GUILDS__BANK_ERROR, "No space to add item to the guild bank.");

		return false;
	}

	const char *Query="INSERT INTO `guild_bank` (`guildid`, `area`, `slot`, `itemid`, `qty`, `donator`, `permissions`, `WhoFor`) "
				"VALUES (%i, %i, %i, %i, %i, '%s', %i, '%s')";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, Query, GuildID, Area, Slot, ItemID, QtyOrCharges, Donator, Permissions, WhoFor), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "Insert Error: %s : %s", query, errbuf);

		safe_delete_array(query);

		return false;
	}

	safe_delete_array(query);

	const Item_Struct *Item = database.GetItem(ItemID);

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

int GuildBankManager::Promote(uint32 GuildID, int SlotID)
{
	if((SlotID < 0) || (SlotID > (GUILD_BANK_DEPOSIT_AREA_SIZE - 1)))
		return -1;

	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
	{
		return -1;
	}

	if((*Iterator)->Items.DepositArea[SlotID].ItemID == 0)
	{
		return -1;
	}

	int MainSlot = -1;

	for(int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
		if((*Iterator)->Items.MainArea[i].ItemID == 0)
		{
			MainSlot = i;

			break;
		}

	if(MainSlot == -1)
		return -1;


	(*Iterator)->Items.MainArea[MainSlot].ItemID = (*Iterator)->Items.DepositArea[SlotID].ItemID;

	(*Iterator)->Items.MainArea[MainSlot].Quantity = (*Iterator)->Items.DepositArea[SlotID].Quantity;

	strn0cpy((*Iterator)->Items.MainArea[MainSlot].Donator, (*Iterator)->Items.DepositArea[SlotID].Donator, sizeof((*Iterator)->Items.MainArea[MainSlot].Donator));
	(*Iterator)->Items.MainArea[MainSlot].Permissions = (*Iterator)->Items.DepositArea[SlotID].Permissions;

	strn0cpy((*Iterator)->Items.MainArea[MainSlot].WhoFor, (*Iterator)->Items.DepositArea[SlotID].WhoFor, sizeof((*Iterator)->Items.MainArea[MainSlot].WhoFor));

	const char *Query="UPDATE `guild_bank` SET `area` = 1, `slot` = %i WHERE `guildid` = %i AND `area` = 0 AND `slot` = %i LIMIT 1";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, Query, MainSlot, GuildID, SlotID), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "error promoting item: %s : %s", query, errbuf);

		safe_delete_array(query);

		return -1;
	}

	safe_delete_array(query);

	(*Iterator)->Items.DepositArea[SlotID].ItemID = 0;

	const Item_Struct *Item = database.GetItem((*Iterator)->Items.MainArea[MainSlot].ItemID);

	GuildBankItemUpdate_Struct gbius;

	if(!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, MainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon, 1, 0, 0, 0);
	else
	{
		if((*Iterator)->Items.MainArea[MainSlot].Quantity == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, MainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*Iterator)->Items.MainArea[MainSlot].Quantity, 0, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, MainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*Iterator)->Items.MainArea[MainSlot].Quantity, 0, 1, 0);
	}

	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, GuildID);

	gbius.Init(GuildBankItemUpdate, 1, SlotID, GuildBankDepositArea, 0, 0, 0, 0, 0, 0, 0);

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, GuildID);

	return MainSlot;
}

void GuildBankManager::SetPermissions(uint32 GuildID, uint16 SlotID, uint32 Permissions, const char *MemberName)
{
	if((SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1)))
		return;

	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
	{
		return;
	}

	if((*Iterator)->Items.MainArea[SlotID].ItemID == 0)
	{
		return;
	}

	const char *Query="UPDATE `guild_bank` SET `permissions` = %i, `whofor` = '%s' WHERE `guildid` = %i AND `area` = 1 AND `slot` = %i LIMIT 1";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, Query, Permissions, MemberName, GuildID, SlotID), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "error changing permissions: %s : %s", query, errbuf);

		safe_delete_array(query);

		return;
	}

	safe_delete_array(query);

	(*Iterator)->Items.MainArea[SlotID].Permissions = Permissions;

	if(Permissions == GuildBankSingleMember)
		strn0cpy((*Iterator)->Items.MainArea[SlotID].WhoFor, MemberName, sizeof((*Iterator)->Items.MainArea[SlotID].WhoFor));
	else
		(*Iterator)->Items.MainArea[SlotID].WhoFor[0] = '\0';


	const Item_Struct *Item = database.GetItem((*Iterator)->Items.MainArea[SlotID].ItemID);

	GuildBankItemUpdate_Struct gbius;

	if(!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, SlotID, GuildBankMainArea, 1, Item->ID, Item->Icon, 1, (*Iterator)->Items.MainArea[SlotID].Permissions, 0, 0);
	else
	{
		if((*Iterator)->Items.MainArea[SlotID].Quantity == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, SlotID, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*Iterator)->Items.MainArea[SlotID].Quantity, (*Iterator)->Items.MainArea[SlotID].Permissions, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, SlotID, GuildBankMainArea, 1, Item->ID, Item->Icon,
					(*Iterator)->Items.MainArea[SlotID].Quantity, (*Iterator)->Items.MainArea[SlotID].Permissions, 1, 0);
	}


	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	strn0cpy(gbius.WhoFor, (*Iterator)->Items.MainArea[SlotID].WhoFor, sizeof(gbius.WhoFor));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, GuildID);
}

ItemInst* GuildBankManager::GetItem(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity)
{
	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return nullptr;

	GuildBankItem* BankArea = nullptr;

	ItemInst* inst = nullptr;

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
	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

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
	std::list<GuildBank*>::iterator Iterator = Banks.begin();

	while(Iterator != Banks.end())
	{
		if((*Iterator)->GuildID == GuildID)
			break;

		++Iterator;
	}

	return Iterator;
}

bool GuildBankManager::DeleteItem(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity)
{
	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return false;

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	GuildBankItem* BankArea = nullptr;

	if(Area == GuildBankMainArea)
	{
		if(SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
			return false;

		BankArea = &(*Iterator)->Items.MainArea[0];
	}
	else
	{
		if(SlotID > (GUILD_BANK_DEPOSIT_AREA_SIZE - 1))
			return false;

		BankArea = &(*Iterator)->Items.DepositArea[0];
	}


	bool Deleted = true;

	const Item_Struct *Item = database.GetItem(BankArea[SlotID].ItemID);

	if(!Item->Stackable || (Quantity >= BankArea[SlotID].Quantity))
	{
		const char *Query = "DELETE from `guild_bank` where `guildid` = %i AND `area` = %i AND `slot` = %i LIMIT 1";

		if(!database.RunQuery(query, MakeAnyLenString(&query, Query, GuildID, Area, SlotID), errbuf))
		{
			_log(GUILDS__BANK_ERROR, "Delete item failed. %s : %s", query, errbuf);

			safe_delete_array(query);

			return false;
		}

		safe_delete_array(query);

		BankArea[SlotID].ItemID = 0;
	}
	else
	{
		const char *Query = "UPDATE `guild_bank` SET `qty` = %i where `guildid` = %i AND `area` = %i AND `slot` = %i LIMIT 1";

		if(!database.RunQuery(query, MakeAnyLenString(&query, Query, BankArea[SlotID].Quantity - Quantity,
				GuildID, Area, SlotID), errbuf))
		{
			_log(GUILDS__BANK_ERROR, "Update item failed. %s : %s", query, errbuf);

			safe_delete_array(query);

			return false;
		}

		safe_delete_array(query);

		BankArea[SlotID].Quantity -= Quantity;

		Deleted = false;
	}
	GuildBankItemUpdate_Struct gbius;

	if(!Deleted)
	{
		gbius.Init(GuildBankItemUpdate, 1, SlotID, Area, 1, Item->ID, Item->Icon, BankArea[SlotID].Quantity, BankArea[SlotID].Permissions, 1, 0);

		strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

		strn0cpy(gbius.WhoFor, BankArea[SlotID].WhoFor, sizeof(gbius.WhoFor));
	}
	else
		gbius.Init(GuildBankItemUpdate, 1, SlotID, Area, 0, 0, 0, 0, 0, 0, 0);

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, GuildID);

	return true;

}

bool GuildBankManager::MergeStacks(uint32 GuildID, uint16 SlotID)
{
	if(SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return false;

	GuildBankItem* BankArea = &(*Iterator)->Items.MainArea[0];

	if(BankArea[SlotID].ItemID == 0)
		return false;

	const Item_Struct *Item = database.GetItem(BankArea[SlotID].ItemID);

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

	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

	if(Iterator == Banks.end())
		return false;

	if(IsAreaFull(GuildID, GuildBankMainArea))
		return false;

	GuildBankItem* BankArea = &(*Iterator)->Items.MainArea[0];

	if(BankArea[SlotID].ItemID == 0)
		return false;

	if(BankArea[SlotID].Quantity <= Quantity || Quantity == 0)
		return false;

	const Item_Struct *Item = database.GetItem(BankArea[SlotID].ItemID);

	if(!Item->Stackable)
		return false;

	AddItem(GuildID, GuildBankMainArea, BankArea[SlotID].ItemID, Quantity, "", BankArea[SlotID].Permissions, BankArea[SlotID].WhoFor);

	DeleteItem(GuildID, GuildBankMainArea, SlotID, Quantity);

	return true;
}

void GuildBankManager::UpdateItemQuantity(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity)
{
	// Helper method for MergeStacks. Assuming all passed parameters are valid.
	//
	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	const char *Query = "UPDATE `guild_bank` SET `qty` = %i where `guildid` = %i AND `area` = %i AND `slot` = %i LIMIT 1";

	if(!database.RunQuery(query, MakeAnyLenString(&query, Query, Quantity, GuildID, Area, SlotID), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "Update item quantity failed. %s : %s", query, errbuf);

		safe_delete_array(query);

		return;
	}

	safe_delete_array(query);
}

bool GuildBankManager::AllowedToWithdraw(uint32 GuildID, uint16 Area, uint16 SlotID, const char *Name)
{
	// Is a none-Guild Banker allowed to withdraw the item at this slot ?
	// This is really here for anti-hacking measures, as the client should not request an item it does not have permission to withdraw.
	//
	if(SlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	std::list<GuildBank*>::iterator Iterator = GetGuildBank(GuildID);

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
		owner->Message(10,"You are already in a guild! Guild request deleted.");
		return false;
	}
	if(deletion_timer->Check() || !owner)
	{
		if(owner)
			owner->Message(0,"You took too long! Your guild request has been deleted.");
		return false;
	}

	return true;
}

GuildApproval::GuildApproval(const char* guildname, Client* owner,uint32 id)
{
	database.GetVariable("GuildCreation", founders, 3);
	uint8 tmp = atoi(founders);
	deletion_timer = new Timer(1800000);
	strcpy(guild,guildname);
	this->owner = owner;
	this->refid = id;
	if(owner)
		owner->Message(0,"You can now start getting your guild approved, tell your %i members to #guildapprove %i, you have 30 minutes to create your guild.",tmp,GetID());
	for(int i=0;i<tmp;i++)
		members[i] = 0;
}

GuildApproval::~GuildApproval()
{
	safe_delete(deletion_timer);
}

bool GuildApproval::AddMemberApproval(Client* addition)
{
	database.GetVariable("GuildCreation", founders, 3);
	uint8 tmp = atoi(founders);
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
	database.GetVariable("GuildCreation", founders, 3);
	uint8 tmp = atoi(founders);
	for(int i=0;i<tmp;i++)
	{
		if(members[i])
			requestee->Message(0,"%i: %s",i,members[i]->GetName());
	}
}

void GuildApproval::GuildApproved()
{
	char petitext[PBUFFER] = "A new guild was founded! Guildname: ";
	char gmembers[MBUFFER] = " ";

	if(!owner)
		return;
	database.GetVariable("GuildCreation", founders, 3);
	uint8 tmp = atoi(founders);
	uint32 tmpeq = guild_mgr.CreateGuild(guild, owner->CharacterID());
	guild_mgr.SetGuild(owner->CharacterID(),tmpeq,2);
	owner->SendAppearancePacket(AT_GuildID,true,false);
	for(int i=0;i<tmp;i++)
	{
		if(members[i])
			{
			owner->Message(0, "%s",members[i]->GetName());
			owner->Message(0, "%i",members[i]->CharacterID());
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
	Petition* pet = new Petition(owner->CastToClient()->CharacterID());
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
	worldserver.SendEmoteMessage(0, 0, 80, 15, "%s has made a petition. #%i", owner->CastToClient()->GetName(), pet->GetID());
	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_RefreshGuild;
	pack->size = tmp;
	pack->pBuffer = new uchar[pack->size];
	memcpy(pack->pBuffer, &tmpeq, 4);
	worldserver.SendPacket(pack);
	safe_delete(pack);
	owner->Message(0, "Your guild was created.");
	owner = 0;
}

