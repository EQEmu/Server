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

#include "guild_base.h"
#include "database.h"

//#include "misc_functions.h"
#include "string_util.h"
#include <cstdlib>
#include <cstring>

//until we move MAX_NUMBER_GUILDS
#include "eq_packet_structs.h"

const char *const BaseGuildManager::GuildActionNames[_MaxGuildAction] =
{ "HearGuildChat", "SpeakGuildChat", "Invite", "Remove", "Promote", "Demote", "Set_MOTD", "War/Peace" };

BaseGuildManager::BaseGuildManager()
: m_db(nullptr)
{
}

BaseGuildManager::~BaseGuildManager() {
	ClearGuilds();
}



bool BaseGuildManager::LoadGuilds() {

	ClearGuilds();

	if(m_db == nullptr) {
		LogGuilds("Requested to load guilds when we have no database object");
		return(false);
	}

	std::string query("SELECT id, name, leader, minstatus, motd, motd_setter,channel,url FROM guilds");
	std::map<uint32, GuildInfo *>::iterator res;

	auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		return false;
	}

	for (auto row=results.begin();row!=results.end();++row)
		_CreateGuild(atoi(row[0]), row[1], atoi(row[2]), atoi(row[3]), row[4], row[5], row[6], row[7]);

    query = "SELECT guild_id,rank,title,can_hear,can_speak,can_invite,can_remove,can_promote,can_demote,can_motd,can_warpeace FROM guild_ranks";
	results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		return false;
	}

	for (auto row=results.begin();row!=results.end();++row)
	{
		uint32 guild_id = atoi(row[0]);
		uint8 rankn = atoi(row[1]);

		if(rankn > GUILD_MAX_RANK) {
			LogGuilds("Found invalid (too high) rank [{}] for guild [{}], skipping", rankn, guild_id);
			continue;
		}

		res = m_guilds.find(guild_id);
		if(res == m_guilds.end()) {
			LogGuilds("Found rank [{}] for non-existent guild [{}], skipping", rankn, guild_id);
			continue;
		}

		RankInfo &rank = res->second->ranks[rankn];

		rank.name = row[2];
		rank.permissions[GUILD_HEAR] = (row[3][0] == '1')?true:false;
		rank.permissions[GUILD_SPEAK] = (row[4][0] == '1')?true:false;
		rank.permissions[GUILD_INVITE] = (row[5][0] == '1')?true:false;
		rank.permissions[GUILD_REMOVE] = (row[6][0] == '1')?true:false;
		rank.permissions[GUILD_PROMOTE] = (row[7][0] == '1')?true:false;
		rank.permissions[GUILD_DEMOTE] = (row[8][0] == '1')?true:false;
		rank.permissions[GUILD_MOTD] = (row[9][0] == '1')?true:false;
		rank.permissions[GUILD_WARPEACE] = (row[10][0] == '1')?true:false;
	}

	return true;
}

bool BaseGuildManager::RefreshGuild(uint32 guild_id) {
	if(m_db == nullptr) {
		LogGuilds("Requested to refresh guild [{}] when we have no database object", guild_id);
		return(false);
	}

	std::string query = StringFormat("SELECT name, leader, minstatus, motd, motd_setter, channel,url FROM guilds WHERE id=%lu", (unsigned long)guild_id);
	std::map<uint32, GuildInfo *>::iterator res;
	GuildInfo *info;

	// load up all the guilds
	auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		return false;
	}

	if (results.RowCount() == 0)
	{
		LogGuilds("Unable to find guild [{}] in the database", guild_id);
		return false;
	}

	auto row = results.begin();

	info = _CreateGuild(guild_id, row[0], atoi(row[1]), atoi(row[2]), row[3], row[4], row[5], row[6]);

    query = StringFormat("SELECT guild_id, rank, title, can_hear, can_speak, can_invite, can_remove, can_promote, can_demote, can_motd, can_warpeace "
                        "FROM guild_ranks WHERE guild_id=%lu", (unsigned long)guild_id);
	results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		return false;
	}

	for (auto row=results.begin();row!=results.end();++row)
	{
		uint8 rankn = atoi(row[1]);

		if(rankn > GUILD_MAX_RANK) {
			LogGuilds("Found invalid (too high) rank [{}] for guild [{}], skipping", rankn, guild_id);
			continue;
		}

		RankInfo &rank = info->ranks[rankn];

		rank.name = row[2];
		rank.permissions[GUILD_HEAR] = (row[3][0] == '1') ? true: false;
		rank.permissions[GUILD_SPEAK] = (row[4][0] == '1') ? true: false;
		rank.permissions[GUILD_INVITE] = (row[5][0] == '1') ? true: false;
		rank.permissions[GUILD_REMOVE] = (row[6][0] == '1') ? true: false;
		rank.permissions[GUILD_PROMOTE] = (row[7][0] == '1') ? true: false;
		rank.permissions[GUILD_DEMOTE] = (row[8][0] == '1') ? true: false;
		rank.permissions[GUILD_MOTD] = (row[9][0] == '1') ? true: false;
		rank.permissions[GUILD_WARPEACE] = (row[10][0] == '1') ? true: false;
	}

	LogGuilds("Successfully refreshed guild [{}] from the database", guild_id);

	return true;
}

BaseGuildManager::GuildInfo *BaseGuildManager::_CreateGuild(uint32 guild_id, const char *guild_name, uint32 leader_char_id, uint8 minstatus, const char *guild_motd, const char *motd_setter, const char *Channel, const char *URL)
{
	std::map<uint32, GuildInfo *>::iterator res;

	//remove any old entry.
	res = m_guilds.find(guild_id);
	if(res != m_guilds.end()) {
		delete res->second;
		m_guilds.erase(res);
	}

	//make the new entry and store it into the map.
	auto info = new GuildInfo;
	info->name = guild_name;
	info->leader_char_id = leader_char_id;
	info->minstatus = minstatus;
	info->motd = guild_motd;
	info->motd_setter = motd_setter;
	info->url = URL;
	info->channel = Channel;
	m_guilds[guild_id] = info;

	//now setup default ranks (everything defaults to false)
	info->ranks[0].name = "Member";
	info->ranks[0].permissions[GUILD_HEAR] = true;
	info->ranks[0].permissions[GUILD_SPEAK] = true;
	info->ranks[1].name = "Officer";
	info->ranks[1].permissions[GUILD_HEAR] = true;
	info->ranks[1].permissions[GUILD_SPEAK] = true;
	info->ranks[1].permissions[GUILD_INVITE] = true;
	info->ranks[1].permissions[GUILD_REMOVE] = true;
	info->ranks[1].permissions[GUILD_MOTD] = true;
	info->ranks[2].name = "Leader";
	info->ranks[2].permissions[GUILD_HEAR] = true;
	info->ranks[2].permissions[GUILD_SPEAK] = true;
	info->ranks[2].permissions[GUILD_INVITE] = true;
	info->ranks[2].permissions[GUILD_REMOVE] = true;
	info->ranks[2].permissions[GUILD_PROMOTE] = true;
	info->ranks[2].permissions[GUILD_DEMOTE] = true;
	info->ranks[2].permissions[GUILD_MOTD] = true;
	info->ranks[2].permissions[GUILD_WARPEACE] = true;

	return(info);
}

bool BaseGuildManager::_StoreGuildDB(uint32 guild_id) {
	if(m_db == nullptr) {
		LogGuilds("Requested to store guild [{}] when we have no database object", guild_id);
		return(false);
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		LogGuilds("Requested to store non-existent guild [{}]", guild_id);
		return(false);
	}
	GuildInfo *info = res->second;

	std::string query = StringFormat("DELETE FROM guilds WHERE id=%lu", (unsigned long)guild_id);

	//clear out old `guilds` entry
	auto results = m_db->QueryDatabase(query);

	//clear out old `guild_ranks` entries
	query = StringFormat("DELETE FROM guild_ranks WHERE guild_id=%lu", (unsigned long)guild_id);
	results = m_db->QueryDatabase(query);

	//escape our strings.
	auto name_esc = new char[info->name.length() * 2 + 1];
	auto motd_esc = new char[info->motd.length() * 2 + 1];
	auto motd_set_esc = new char[info->motd_setter.length() * 2 + 1];
	m_db->DoEscapeString(name_esc, info->name.c_str(), info->name.length());
	m_db->DoEscapeString(motd_esc, info->motd.c_str(), info->motd.length());
	m_db->DoEscapeString(motd_set_esc, info->motd_setter.c_str(), info->motd_setter.length());

	//insert the new `guilds` entry
	query = StringFormat("INSERT INTO guilds (id,name,leader,minstatus,motd,motd_setter) VALUES(%lu,'%s',%lu,%d,'%s', '%s')",
		(unsigned long)guild_id, name_esc, (unsigned long)info->leader_char_id, info->minstatus, motd_esc, motd_set_esc);
	results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		safe_delete_array(name_esc);
		safe_delete_array(motd_esc);
		safe_delete_array(motd_set_esc);
		return false;
	}
	safe_delete_array(name_esc);
	safe_delete_array(motd_esc);
	safe_delete_array(motd_set_esc);

	//now insert the new ranks
	uint8 rank;
	for(rank = 0; rank <= GUILD_MAX_RANK; rank++) {
		const RankInfo &rankInfo = info->ranks[rank];

		auto title_esc = new char[rankInfo.name.length() * 2 + 1];
		m_db->DoEscapeString(title_esc, rankInfo.name.c_str(), rankInfo.name.length());

        query = StringFormat("INSERT INTO guild_ranks "
        "(guild_id,rank,title,can_hear,can_speak,can_invite,can_remove,can_promote,can_demote,can_motd,can_warpeace)"
		" VALUES(%d,%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d)",
			guild_id, rank, title_esc,
			rankInfo.permissions[GUILD_HEAR],
			rankInfo.permissions[GUILD_SPEAK],
			rankInfo.permissions[GUILD_INVITE],
			rankInfo.permissions[GUILD_REMOVE],
			rankInfo.permissions[GUILD_PROMOTE],
			rankInfo.permissions[GUILD_DEMOTE],
			rankInfo.permissions[GUILD_MOTD],
			rankInfo.permissions[GUILD_WARPEACE]);
		results = m_db->QueryDatabase(query);

		if (!results.Success())
		{
			safe_delete_array(title_esc);
			return false;
		}
		safe_delete_array(title_esc);
	}

	LogGuilds("Stored guild [{}] in the database", guild_id);

	return true;
}

uint32 BaseGuildManager::_GetFreeGuildID() {
	if(m_db == nullptr) {
		LogGuilds("Requested find a free guild ID when we have no database object");
		return(GUILD_NONE);
	}

	std::string query;
	//this has got to be one of the more retarded things I have seen.
	//none the less, im too lazy to rewrite it right now.
	//possibly:
	//
	// SELECT t1.id + 1
	// FROM guilds t1
	// WHERE NOT EXISTS (
    //	SELECT *
    //	FROM guilds t2
    //	WHERE t2.id = t1.id + 1
	// )
	// LIMIT 1
	//
	// Seems likely what we should be doing is auto incrementing the guild table
	// inserting, then getting the id. NOT getting a free id then inserting.
	// could be a race condition.

	for (auto index = 1; index < MAX_NUMBER_GUILDS; ++index)
	{
        query = StringFormat("SELECT id FROM guilds where id=%i;", index);
		auto results = m_db->QueryDatabase(query);

		if (!results.Success())
		{
			continue;
		}

		if (results.RowCount() == 0)
		{
			LogGuilds("Located free guild ID [{}] in the database", index);
			return index;
		}
	}

	LogGuilds("Unable to find a free guild ID when requested");
	return GUILD_NONE;
}



uint32 BaseGuildManager::CreateGuild(const char* name, uint32 leader_char_id) {
	uint32 gid = DBCreateGuild(name, leader_char_id);
	if(gid == GUILD_NONE)
		return(GUILD_NONE);

	SendGuildRefresh(gid, true, false, false, false);
	SendCharRefresh(GUILD_NONE, gid, leader_char_id);

	return(gid);
}

bool BaseGuildManager::DeleteGuild(uint32 guild_id) {
	if(!DBDeleteGuild(guild_id))
		return(false);

	SendGuildDelete(guild_id);

	return(true);
}

bool BaseGuildManager::RenameGuild(uint32 guild_id, const char* name) {
	if(!DBRenameGuild(guild_id, name))
		return(false);

	SendGuildRefresh(guild_id, true, false, false, false);

	return(true);
}

bool BaseGuildManager::SetGuildLeader(uint32 guild_id, uint32 leader_char_id) {
	//get old leader first.
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return(false);
	GuildInfo *info = res->second;
	uint32 old_leader = info->leader_char_id;

	if(!DBSetGuildLeader(guild_id, leader_char_id))
		return(false);

	SendGuildRefresh(guild_id, false, false, false, false);
	SendCharRefresh(GUILD_NONE, guild_id, old_leader);
	SendCharRefresh(GUILD_NONE, guild_id, leader_char_id);

	return(true);
}

bool BaseGuildManager::SetGuildMOTD(uint32 guild_id, const char* motd, const char *setter) {
	if(!DBSetGuildMOTD(guild_id, motd, setter))
		return(false);

	SendGuildRefresh(guild_id, false, true, false, false);

	return(true);
}

bool BaseGuildManager::SetGuildURL(uint32 GuildID, const char* URL)
{
	if(!DBSetGuildURL(GuildID, URL))
		return(false);

	SendGuildRefresh(GuildID, false, true, false, false);

	return(true);
}

bool BaseGuildManager::SetGuildChannel(uint32 GuildID, const char* Channel)
{
	if(!DBSetGuildChannel(GuildID, Channel))
		return(false);

	SendGuildRefresh(GuildID, false, true, false, false);

	return(true);
}

bool BaseGuildManager::SetGuild(uint32 charid, uint32 guild_id, uint8 rank) {
	if(rank > GUILD_MAX_RANK && guild_id != GUILD_NONE)
		return(false);

	//lookup their old guild, if they had one.
	uint32 old_guild = GUILD_NONE;
	CharGuildInfo gci;
	if(GetCharInfo(charid, gci)) {
		old_guild = gci.guild_id;
	}

	if(!DBSetGuild(charid, guild_id, rank))
		return(false);

	SendCharRefresh(old_guild, guild_id, charid);

	return(true);
}

//changes rank, but not guild.
bool BaseGuildManager::SetGuildRank(uint32 charid, uint8 rank) {
	if(rank > GUILD_MAX_RANK)
		return(false);

	if(!DBSetGuildRank(charid, rank))
		return(false);

	SendCharRefresh(GUILD_NONE, 0, charid);

	return(true);
}


bool BaseGuildManager::SetBankerFlag(uint32 charid, bool is_banker) {
	if(!DBSetBankerFlag(charid, is_banker))
		return(false);

	SendRankUpdate(charid);

	return(true);
}

bool BaseGuildManager::ForceRankUpdate(uint32 charid) {
	SendRankUpdate(charid);
	return(true);
}

bool BaseGuildManager::SetAltFlag(uint32 charid, bool is_alt)
{
	if(!DBSetAltFlag(charid, is_alt))
		return(false);

	SendRankUpdate(charid);

	return(true);
}

bool BaseGuildManager::SetTributeFlag(uint32 charid, bool enabled) {
	if(!DBSetTributeFlag(charid, enabled))
		return(false);

	SendCharRefresh(GUILD_NONE, 0, charid);

	return(true);
}

bool BaseGuildManager::SetPublicNote(uint32 charid, const char *note) {
	if(!DBSetPublicNote(charid, note))
		return(false);

	SendCharRefresh(GUILD_NONE, 0, charid);

	return(true);
}

uint32 BaseGuildManager::DBCreateGuild(const char* name, uint32 leader) {
	//first try to find a free ID.
	uint32 new_id = _GetFreeGuildID();
	if(new_id == GUILD_NONE)
		return(GUILD_NONE);

	//now make the guild record in our local manager.
	//this also sets up the default ranks for us.
	_CreateGuild(new_id, name, leader, 0, "", "", "", "");

	//now store the resulting guild setup into the DB.
	if(!_StoreGuildDB(new_id)) {
		LogGuilds("Error storing new guild. It may have been partially created which may need manual removal");
		return(GUILD_NONE);
	}

	LogGuilds("Created guild [{}] in the database", new_id);

	return(new_id);
}

bool BaseGuildManager::DBDeleteGuild(uint32 guild_id) {

	//remove the local entry
	std::map<uint32, GuildInfo *>::iterator res;
	res = m_guilds.find(guild_id);
	if(res != m_guilds.end()) {
		delete res->second;
		m_guilds.erase(res);
	}

	if(m_db == nullptr) {
		LogGuilds("Requested to delete guild [{}] when we have no database object", guild_id);
		return(false);
	}

	//clear out old `guilds` entry
	std::string query = StringFormat("DELETE FROM guilds WHERE id=%lu", (unsigned long)guild_id);
	QueryWithLogging(query, "clearing old guild record");

	//clear out old `guild_ranks` entries
	query = StringFormat("DELETE FROM guild_ranks WHERE guild_id=%lu", (unsigned long)guild_id);
	QueryWithLogging(query, "clearing old guild_ranks records");

	//clear out people belonging to this guild.
	query = StringFormat("DELETE FROM guild_members WHERE guild_id=%lu", (unsigned long)guild_id);
	QueryWithLogging(query, "clearing chars in guild");

	// Delete the guild bank
	query = StringFormat("DELETE FROM guild_bank WHERE guildid=%lu", (unsigned long)guild_id);
	QueryWithLogging(query, "deleting guild bank");

	LogGuilds("Deleted guild [{}] from the database", guild_id);

	return(true);
}

bool BaseGuildManager::DBRenameGuild(uint32 guild_id, const char* name) {
	if(m_db == nullptr) {
		LogGuilds("Requested to rename guild [{}] when we have no database object", guild_id);
		return false;
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return false;
	GuildInfo *info = res->second;

	//escape our strings.
	uint32 len = strlen(name);
	auto esc = new char[len * 2 + 1];
	m_db->DoEscapeString(esc, name, len);

	//insert the new `guilds` entry
	std::string query = StringFormat("UPDATE guilds SET name='%s' WHERE id=%d", esc, guild_id);
	auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		LogGuilds("Error renaming guild [{}] [{}]: [{}]", guild_id, query.c_str(), results.Success());
		safe_delete_array(esc);
		return false;
	}
	safe_delete_array(esc);

	LogGuilds("Renamed guild [{}] ([{}]) to [{}] in database", info->name.c_str(), guild_id, name);

	info->name = name;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildLeader(uint32 guild_id, uint32 leader) {
	if(m_db == nullptr) {
		LogGuilds("Requested to set the leader for guild [{}] when we have no database object", guild_id);
		return false;
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return false;
	GuildInfo *info = res->second;

	//insert the new `guilds` entry
    std::string query = StringFormat("UPDATE guilds SET leader='%d' WHERE id=%d",leader, guild_id);
	auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		return false;
	}

	//set the old leader to officer
	if(!DBSetGuildRank(info->leader_char_id, GUILD_OFFICER))
		return false;
	//set the new leader to leader
	if(!DBSetGuildRank(leader, GUILD_LEADER))
		return false;

	LogGuilds("Set guild leader for guild [{}] to [{}] in the database", guild_id, leader);

	info->leader_char_id = leader;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildMOTD(uint32 guild_id, const char* motd, const char *setter) {
	if(m_db == nullptr) {
		LogGuilds("Requested to set the MOTD for guild [{}] when we have no database object", guild_id);
		return(false);
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return(false);
	GuildInfo *info = res->second;

	//escape our strings.
	uint32 len = strlen(motd);
	uint32 len2 = strlen(setter);
	auto esc = new char[len * 2 + 1];
	auto esc_set = new char[len2 * 2 + 1];
	m_db->DoEscapeString(esc, motd, len);
	m_db->DoEscapeString(esc_set, setter, len2);

	//insert the new `guilds` entry
	std::string query = StringFormat("UPDATE guilds SET motd='%s',motd_setter='%s' WHERE id=%d", esc, esc_set, guild_id);
	auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		safe_delete_array(esc);
		safe_delete_array(esc_set);
		return false;
	}
	safe_delete_array(esc);
	safe_delete_array(esc_set);

	LogGuilds("Set MOTD for guild [{}] in the database", guild_id);

	info->motd = motd;	//update our local record.
	info->motd_setter = setter;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildURL(uint32 GuildID, const char* URL)
{
	if(m_db == nullptr)
		return false;

	auto res = m_guilds.find(GuildID);
	if(res == m_guilds.end())
		return false;

	GuildInfo *info = res->second;

	//escape our strings.
	uint32 len = strlen(URL);
	auto esc = new char[len * 2 + 1];
	m_db->DoEscapeString(esc, URL, len);

    std::string query = StringFormat("UPDATE guilds SET url='%s' WHERE id=%d", esc, GuildID);
	auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		safe_delete_array(esc);
		return(false);
	}
	safe_delete_array(esc);

	LogGuilds("Set URL for guild [{}] in the database", GuildID);

	info->url = URL;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildChannel(uint32 GuildID, const char* Channel)
{
	if(m_db == nullptr)
		return(false);

	auto res = m_guilds.find(GuildID);

	if(res == m_guilds.end())
		return(false);

	GuildInfo *info = res->second;

	//escape our strings.
	uint32 len = strlen(Channel);
	auto esc = new char[len * 2 + 1];
	m_db->DoEscapeString(esc, Channel, len);

    std::string query = StringFormat("UPDATE guilds SET channel='%s' WHERE id=%d", esc, GuildID);
    auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		safe_delete_array(esc);
		return(false);
	}
	safe_delete_array(esc);

	LogGuilds("Set Channel for guild [{}] in the database", GuildID);

	info->channel = Channel;	//update our local record.

	return(true);
}

bool BaseGuildManager::DBSetGuild(uint32 charid, uint32 guild_id, uint8 rank) {
	if(m_db == nullptr) {
		LogGuilds("Requested to set char to guild [{}] when we have no database object", guild_id);
		return(false);
	}

	std::string query;

	if(guild_id != GUILD_NONE) {
        query = StringFormat("REPLACE INTO guild_members (char_id,guild_id,rank,public_note) VALUES(%d,%d,%d,'')", charid, guild_id, rank);
        auto results = m_db->QueryDatabase(query);

		if (!results.Success()) {
			return false;
		}

	} else {
        query = StringFormat("DELETE FROM guild_members WHERE char_id=%d", charid);
        auto results = m_db->QueryDatabase(query);
        if (!results.Success())
		{
			return false;
		}
    }
	LogGuilds("Set char [{}] to guild [{}] and rank [{}] in the database", charid, guild_id, rank);
	return true;
}

bool BaseGuildManager::DBSetGuildRank(uint32 charid, uint8 rank) {
	std::string query = StringFormat("UPDATE guild_members SET rank=%d WHERE char_id=%d", rank, charid);
	return(QueryWithLogging(query, "setting a guild member's rank"));
}

bool BaseGuildManager::DBSetBankerFlag(uint32 charid, bool is_banker) {
	std::string query = StringFormat("UPDATE guild_members SET banker=%d WHERE char_id=%d",
		is_banker? 1: 0, charid);
	return(QueryWithLogging(query, "setting a guild member's banker flag"));
}

bool BaseGuildManager::GetBankerFlag(uint32 CharID)
{
	if(!m_db)
		return false;

    std::string query = StringFormat("select `banker` from `guild_members` where char_id=%i LIMIT 1", CharID);
    auto results = m_db->QueryDatabase(query);
	if(!results.Success())
	{
		return false;
	}

	if(results.RowCount() != 1)
		return false;

	auto row = results.begin();

	bool IsBanker = atoi(row[0]);

	return IsBanker;
}

bool BaseGuildManager::DBSetAltFlag(uint32 charid, bool is_alt)
{
	std::string query = StringFormat("UPDATE guild_members SET alt=%d WHERE char_id=%d",
		is_alt ? 1: 0, charid);

	return(QueryWithLogging(query, "setting a guild member's alt flag"));
}

bool BaseGuildManager::GetAltFlag(uint32 CharID)
{
    if(!m_db)
		return false;

    std::string query = StringFormat("SELECT `alt` FROM `guild_members` WHERE char_id=%i LIMIT 1", CharID);
    auto results = m_db->QueryDatabase(query);
	if(!results.Success())
	{
		return false;
	}

	if(results.RowCount() != 1)
		return false;

	auto row = results.begin();

	bool IsAlt = atoi(row[0]);

	return IsAlt;
}

bool BaseGuildManager::DBSetTributeFlag(uint32 charid, bool enabled) {
	std::string query = StringFormat("UPDATE guild_members SET tribute_enable=%d WHERE char_id=%d",
		enabled ? 1: 0, charid);
	return(QueryWithLogging(query, "setting a guild member's tribute flag"));
}

bool BaseGuildManager::DBSetPublicNote(uint32 charid, const char* note) {
	if(m_db == nullptr)
		return(false);

	//escape our strings.
	uint32 len = strlen(note);
	auto esc = new char[len * 2 + 1];
	m_db->DoEscapeString(esc, note, len);

	//insert the new `guilds` entry
	std::string query = StringFormat("UPDATE guild_members SET public_note='%s' WHERE char_id=%d", esc, charid);
	safe_delete_array(esc);
	auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		return false;
	}

	LogGuilds("Set public not for char [{}]", charid);

	return true;
}

bool BaseGuildManager::QueryWithLogging(std::string query, const char *errmsg) {
	if(m_db == nullptr)
		return(false);

    auto results = m_db->QueryDatabase(query);

	if (!results.Success())
	{
		return(false);
	}

	return(true);
}

//factored out so I dont have to copy this crap.
#ifdef BOTS
#define GuildMemberBaseQuery \
"SELECT c.`id`, c.`name`, c.`class`, c.`level`, c.`last_login`, c.`zone_id`," \
" g.`guild_id`, g.`rank`, g.`tribute_enable`, g.`total_tribute`, g.`last_tribute`," \
" g.`banker`, g.`public_note`, g.`alt`" \
" FROM `vw_bot_character_mobs` AS c LEFT JOIN `vw_guild_members` AS g ON c.`id` = g.`char_id` AND c.`mob_type` = g.`mob_type` "
#else
#define GuildMemberBaseQuery \
"SELECT c.`id`, c.`name`, c.`class`, c.`level`, c.`last_login`, c.`zone_id`," \
" g.`guild_id`, g.`rank`, g.`tribute_enable`, g.`total_tribute`, g.`last_tribute`," \
" g.`banker`, g.`public_note`, g.`alt` " \
" FROM `character_data` AS c LEFT JOIN `guild_members` AS g ON c.`id` = g.`char_id` "
#endif
static void ProcessGuildMember(MySQLRequestRow row, CharGuildInfo &into) {
	//fields from `characer_`
	into.char_id		= atoi(row[0]);
	into.char_name		= row[1];
	into.class_			= atoi(row[2]);
	into.level			= atoi(row[3]);
	into.time_last_on	= atoul(row[4]);
	into.zone_id		= atoi(row[5]);

	//fields from `guild_members`, leave at defaults if missing
	into.guild_id		= row[6] ? atoi(row[6]) : GUILD_NONE;
	into.rank			= row[7] ? atoi(row[7]) : (GUILD_MAX_RANK+1);
	into.tribute_enable = row[8] ? (row[8][0] == '0'?false:true) : false;
	into.total_tribute	= row[9] ? atoi(row[9]) : 0;
	into.last_tribute	= row[10]? atoul(row[10]) : 0;		//timestamp
	into.banker			= row[11]? (row[11][0] == '0'?false:true) : false;
	into.public_note	= row[12]? row[12] : "";
	into.alt		= row[13]? (row[13][0] == '0'?false:true) : false;

	//a little sanity checking/cleanup
	if(into.guild_id == 0)
		into.guild_id = GUILD_NONE;
	if(into.rank > GUILD_MAX_RANK)
		into.rank = GUILD_RANK_NONE;
}


bool BaseGuildManager::GetEntireGuild(uint32 guild_id, std::vector<CharGuildInfo *> &members) {
	members.clear();

	if(m_db == nullptr)
		return(false);

	//load up the rank info for each guild.
	std::string query = StringFormat(GuildMemberBaseQuery " WHERE g.guild_id=%d AND c.deleted_at IS NULL", guild_id);
	auto results = m_db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto ci = new CharGuildInfo;
		ProcessGuildMember(row, *ci);
		members.push_back(ci);
	}

	LogGuilds("Retreived entire guild member list for guild [{}] from the database", guild_id);

	return true;
}

bool BaseGuildManager::GetCharInfo(const char *char_name, CharGuildInfo &into) {
	if(m_db == nullptr) {
		LogGuilds("Requested char info on [{}] when we have no database object", char_name);
		return(false);
	}

	//escape our strings.
	uint32 nl = strlen(char_name);
	auto esc = new char[nl * 2 + 1];
	m_db->DoEscapeString(esc, char_name, nl);

	//load up the rank info for each guild.
    std::string query = StringFormat(GuildMemberBaseQuery " WHERE c.name='%s' AND c.deleted_at IS NULL", esc);
    safe_delete_array(esc);
    auto results = m_db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (results.RowCount() == 0)
        return false;

    auto row = results.begin();
    ProcessGuildMember(row, into);
    LogGuilds("Retreived guild member info for char [{}] from the database", char_name);

	return true;


}

bool BaseGuildManager::GetCharInfo(uint32 char_id, CharGuildInfo &into) {
	if(m_db == nullptr) {
		LogGuilds("Requested char info on [{}] when we have no database object", char_id);
		return false;
	}

	//load up the rank info for each guild.
	std::string query;
#ifdef BOTS
    query = StringFormat(GuildMemberBaseQuery " WHERE c.id=%d AND c.mob_type = 'C' AND c.deleted_at IS NULL", char_id);
#else
    query = StringFormat(GuildMemberBaseQuery " WHERE c.id=%d AND c.deleted_at IS NULL", char_id);
#endif
    auto results = m_db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

    if (results.RowCount() == 0)
        return false;

    auto row = results.begin();
    ProcessGuildMember(row, into);
    LogGuilds("Retreived guild member info for char [{}]", char_id);

	return true;

}

//returns ownership of the buffer.
uint8 *BaseGuildManager::MakeGuildList(const char *head_name, uint32 &length) const {
	//dynamic structs will make this a lot less painful.

	length = sizeof(GuildsList_Struct);
	auto buffer = new uint8[length];

	//a bit little better than memsetting the whole thing...
	uint32 r,pos;
	for(r = 0, pos = 0; r <= MAX_NUMBER_GUILDS; r++, pos += 64) {
		//strcpy((char *) buffer+pos, "BAD GUILD");
		// These 'BAD GUILD' entries were showing in the drop-downs for selecting guilds in the LFP window,
		// so just fill unused entries with an empty string instead.
		buffer[pos] = '\0';
	}

	strn0cpy((char *) buffer, head_name, 64);

	std::map<uint32, GuildInfo *>::const_iterator cur, end;
	cur = m_guilds.begin();
	end = m_guilds.end();
	for(; cur != end; ++cur) {
		pos = 64 + (64 * cur->first);
		strn0cpy((char *) buffer + pos, cur->second->name.c_str(), 64);
	}
	return(buffer);
}

const char *BaseGuildManager::GetRankName(uint32 guild_id, uint8 rank) const {
	if(rank > GUILD_MAX_RANK)
		return("Invalid Rank");
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return("Invalid Guild Rank");
	return(res->second->ranks[rank].name.c_str());
}

const char *BaseGuildManager::GetGuildName(uint32 guild_id) const {
	if(guild_id == GUILD_NONE)
		return("");
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return("Invalid Guild");
	return(res->second->name.c_str());
}

bool BaseGuildManager::GetGuildNameByID(uint32 guild_id, std::string &into) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return(false);
	into = res->second->name;
	return(true);
}

uint32 BaseGuildManager::GetGuildIDByName(const char *GuildName)
{
	std::map<uint32, GuildInfo *>::iterator Iterator;

	for(Iterator = m_guilds.begin(); Iterator != m_guilds.end(); ++Iterator)
	{
		if(!strcasecmp((*Iterator).second->name.c_str(), GuildName))
			return (*Iterator).first;
	}

	return GUILD_NONE;
}

bool BaseGuildManager::GetGuildMOTD(uint32 guild_id, char *motd_buffer, char *setter_buffer) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return(false);
	strn0cpy(motd_buffer, res->second->motd.c_str(), 512);
	strn0cpy(setter_buffer, res->second->motd_setter.c_str(), 64);
	return(true);
}

bool BaseGuildManager::GetGuildURL(uint32 GuildID, char *URLBuffer) const
{
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(GuildID);
	if(res == m_guilds.end())
		return(false);
	strn0cpy(URLBuffer, res->second->url.c_str(), 512);

	return(true);
}

bool BaseGuildManager::GetGuildChannel(uint32 GuildID, char *ChannelBuffer) const
{
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(GuildID);
	if(res == m_guilds.end())
		return(false);
	strn0cpy(ChannelBuffer, res->second->channel.c_str(), 128);
	return(true);
}

bool BaseGuildManager::GuildExists(uint32 guild_id) const {
	if(guild_id == GUILD_NONE)
		return(false);
	return(m_guilds.find(guild_id) != m_guilds.end());
}

bool BaseGuildManager::IsGuildLeader(uint32 guild_id, uint32 char_id) const {
	if(guild_id == GUILD_NONE) {
		LogGuilds("Check leader for char [{}]: not a guild", char_id);
		return(false);
	}
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		LogGuilds("Check leader for char [{}]: invalid guild", char_id);
		return(false);	//invalid guild
	}
	LogGuilds("Check leader for guild [{}], char [{}]: leader id=[{}]", guild_id, char_id, res->second->leader_char_id);
	return(char_id == res->second->leader_char_id);
}

uint32 BaseGuildManager::FindGuildByLeader(uint32 leader) const {
	std::map<uint32, GuildInfo *>::const_iterator cur, end;
	cur = m_guilds.begin();
	end = m_guilds.end();
	for(; cur != end; ++cur) {
		if(cur->second->leader_char_id == leader)
			return(cur->first);
	}
	return(GUILD_NONE);
}

//returns the rank to be sent to the client for display purposes, given their eqemu rank.
uint8 BaseGuildManager::GetDisplayedRank(uint32 guild_id, uint8 rank, uint32 char_id) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return(3);	//invalid guild rank
	if (res->second->ranks[rank].permissions[GUILD_WARPEACE] || res->second->leader_char_id == char_id)
		return(2);	//leader rank
	else if (res->second->ranks[rank].permissions[GUILD_INVITE] || res->second->ranks[rank].permissions[GUILD_REMOVE] || res->second->ranks[rank].permissions[GUILD_MOTD])
		return(1);	//officer rank
	return(0);	//member rank
}

bool BaseGuildManager::CheckGMStatus(uint32 guild_id, uint8 status) const {
	if(status >= 250) {
		LogGuilds("Check permission on guild [{}] with user status [{}] > 250, granted", guild_id, status);
		return(true);	//250+ as allowed anything
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		LogGuilds("Check permission on guild [{}] with user status [{}], no such guild, denied", guild_id, status);
		return(false);	//invalid guild
	}

	bool granted = (res->second->minstatus <= status);

	LogGuilds("Check permission on guild [{}] ([{}]) with user status [{}]. Min status [{}]: [{}]",
		res->second->name.c_str(), guild_id, status, res->second->minstatus, granted?"granted":"denied");

	return(granted);
}

bool BaseGuildManager::CheckPermission(uint32 guild_id, uint8 rank, GuildAction act) const {
	if(rank > GUILD_MAX_RANK) {
		LogGuilds("Check permission on guild [{}] and rank [{}] for action [{}] ([{}]): Invalid rank, denied",
			guild_id, rank, GuildActionNames[act], act);
		return(false);	//invalid rank
	}
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		LogGuilds("Check permission on guild [{}] and rank [{}] for action [{}] ([{}]): Invalid guild, denied",
			guild_id, rank, GuildActionNames[act], act);
		return(false);	//invalid guild
	}

	bool granted = res->second->ranks[rank].permissions[act];

	LogGuilds("Check permission on guild [{}] ([{}]) and rank [{}] ([{}]) for action [{}] ([{}]): [{}]",
		res->second->name.c_str(), guild_id,
		res->second->ranks[rank].name.c_str(), rank,
		GuildActionNames[act], act,
		granted?"granted":"denied");

	return(granted);
}

bool BaseGuildManager::LocalDeleteGuild(uint32 guild_id) {
	std::map<uint32, GuildInfo *>::iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return(false);	//invalid guild
	m_guilds.erase(res);
	return(true);
}

void BaseGuildManager::ClearGuilds() {
	std::map<uint32, GuildInfo *>::iterator cur, end;
	cur = m_guilds.begin();
	end = m_guilds.end();
	for(; cur != end; ++cur) {
		delete cur->second;
	}
	m_guilds.clear();
}

BaseGuildManager::RankInfo::RankInfo() {
	uint8 r;
	for(r = 0; r < _MaxGuildAction; r++)
		permissions[r] = false;
}

BaseGuildManager::GuildInfo::GuildInfo() {
	leader_char_id = 0;
	minstatus = 0;
}

uint32 BaseGuildManager::DoesAccountContainAGuildLeader(uint32 AccountID)
{
	std::string query = StringFormat("SELECT guild_id FROM guild_members WHERE char_id IN "
		"(SELECT id FROM `character_data` WHERE account_id = %i) AND rank = 2",
                                    AccountID);
    auto results = m_db->QueryDatabase(query);
	if (!results.Success())
	{
		return 0;
	}

	return results.RowCount();
}




