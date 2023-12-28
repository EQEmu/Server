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
#include "../common/rulesys.h"
#include "../common/repositories/guilds_repository.h"
#include "../common/repositories/guild_ranks_repository.h"
#include "../common/repositories/guild_permissions_repository.h"
#include "../common/repositories/guild_members_repository.h"
#include "../common/repositories/guild_bank_repository.h"
#include "../common/repositories/guild_tributes_repository.h"


//#include "misc_functions.h"
#include "strings.h"
#include <cstdlib>
#include <cstring>

//until we move MAX_NUMBER_GUILDS
#include "eq_packet_structs.h"

std::vector<default_permission_struct> default_permissions = {
	{GUILD_ACTION_BANK_CHANGE_ITEM_PERMISSIONS,128},
	{GUILD_ACTION_BANK_DEPOSIT_ITEMS,248},
	{GUILD_ACTION_BANK_PROMOTE_ITEMS,128},
	{GUILD_ACTION_BANK_VIEW_ITEMS,248},
	{GUILD_ACTION_BANK_WITHDRAW_ITEMS,224},
	{GUILD_ACTION_BANNER_CHANGE,224},
	{GUILD_ACTION_BANNER_PLANT,224},
	{GUILD_ACTION_BANNER_REMOVE,224},
	{GUILD_ACTION_CHANGE_THE_MOTD,224},
	{GUILD_ACTION_DISPLAY_GUILD_NAME,255},
	{GUILD_ACTION_EDIT_PUBLIC_NOTES,224},
	{GUILD_ACTION_EDIT_RECRUITING_SETTINGS,224},
	{GUILD_ACTION_GUILD_CHAT_SEE,255},
	{GUILD_ACTION_GUILD_CHAT_SPEAK_IN,255},
	{GUILD_ACTION_MEMBERS_CHANGE_ALT_FLAG_FOR_OTHER,224},
	{GUILD_ACTION_MEMBERS_DEMOTE,128},
	{GUILD_ACTION_MEMBERS_DEMOTE_SELF,	224},
	{GUILD_ACTION_MEMBERS_INVITE,224},
	{GUILD_ACTION_MEMBERS_PROMOTE,224},
	{GUILD_ACTION_MEMBERS_REMOVE,128},
	{GUILD_ACTION_RANKS_CHANGE_PERMISSIONS,	128},
	{GUILD_ACTION_RANKS_CHANGE_RANK_NAMES,	128},
	{GUILD_ACTION_REAL_ESTATE_GUILD_PLOT_BUY,128},
	{GUILD_ACTION_REAL_ESTATE_GUILD_PLOT_SELL,128},
	{GUILD_ACTION_REAL_ESTATE_MODIFY_TROPHIES,224},
	{GUILD_ACTION_SEND_THE_WHOLE_GUILD_E_MAIL,224},
	{GUILD_ACTION_TRIBUTE_CHANGE_ACTIVE_BENEFIT,224},
	{GUILD_ACTION_TRIBUTE_CHANGE_FOR_OTHERS,224},
	{GUILD_ACTION_TROPHY_TRIBUTE_CHANGE_ACTIVE_BENEFIT,224},
	{GUILD_ACTION_TROPHY_TRIBUTE_CHANGE_FOR_OTHERS,224}
};

std::vector<default_rank_names_struct> default_rank_names = {
	{1, "Leader"},
	{2, "Senior Officer"},
	{3, "Officer"},
	{4, "Senior Member"},
	{5, "Member"},
	{6, "Junior Member"},
	{7, "Initiate"},
	{8, "Recruit"}
};

BaseGuildManager::BaseGuildManager()
: m_db(nullptr)
{
}

BaseGuildManager::~BaseGuildManager() {
	ClearGuilds();
}

bool BaseGuildManager::LoadGuilds() 
{
	ClearGuilds();

	if(m_db == nullptr) {
		LogError("Requested to load guilds from the database however there is no database connectivity.");
		return false;
	}

	auto guilds = BaseGuildsRepository::All(*m_db);
	if (guilds.empty()) {
		LogGuilds("No Guilds found in database.");
		return false;
	}
	
	LogGuilds("Found {} guilds.  Loading.....", guilds.size());
	for (auto const& g : guilds) {
		_CreateGuild(g.id, g.name.c_str(), g.leader, g.minstatus, g.motd.c_str(), g.motd_setter.c_str(), g.channel.c_str(), g.url.c_str(), g.favor);
	}
	
	bool store_to_db = false;
	for (auto g : m_guilds) {
		auto where_filter = fmt::format("guild_id = '{}'", g.first);
		auto g_ranks = BaseGuildRanksRepository::GetWhere(*m_db, where_filter);
		for (auto const& r : g_ranks) {
			g.second->rank_names[r.rank] = r.title;
		}

		where_filter = fmt::format("guild_id = '{}'", g.first);
		auto g_permissions = BaseGuildPermissionsRepository::GetWhere(*m_db, where_filter);
		if (g_permissions.size() < GUILD_MAX_FUNCTIONS) {
			store_to_db = true;
		}

		for (auto const& p : g_permissions) {
			g.second->functions[p.perm_id].id = p.id;
			g.second->functions[p.perm_id].guild_id = p.guild_id;
			g.second->functions[p.perm_id].perm_id = p.perm_id;
			g.second->functions[p.perm_id].perm_value = p.permission;
		}
		LogGuilds("Loaded guild id [{}]", g.first);
		if (store_to_db) {
			LogGuilds("Found missing permissions for guild id [{}].  Setting missing to default values.", g.first);
			_StoreGuildDB(g.first);
			store_to_db = false;
		}
		auto g_tributes = BaseGuildTributesRepository::FindOne(*m_db, g.first);
		if (g_tributes.guild_id) {
			g.second->tribute.id_1			 = g_tributes.tribute_id_1;
			g.second->tribute.id_2			 = g_tributes.tribute_id_2;
			g.second->tribute.id_1_tier		 = g_tributes.tribute_id_1_tier;
			g.second->tribute.id_2_tier		 = g_tributes.tribute_id_2_tier;
			g.second->tribute.enabled		 = g_tributes.enabled;
			if (g_tributes.time_remaining > RuleI(Guild, TributeTime) ||
				g_tributes.time_remaining <= 0) {
				g_tributes.time_remaining = RuleI(Guild, TributeTime);
			}
			g.second->tribute.time_remaining = g_tributes.time_remaining;
			LogGuilds("Timer has [{}] time remaining from the load function.", g.second->tribute.time_remaining);
		}
	}
	LogGuilds("Completed loading {} guilds.", guilds.size());

	return true;
}

bool BaseGuildManager::RefreshGuild(uint32 guild_id) 
{
	if (guild_id <= 0) {
		LogError("Requested to refresh guild id [{}] but id must be greater than 0.", guild_id);
		return false;
	}

	auto db_guild = BaseGuildsRepository::FindOne(*m_db, guild_id);
	if (!db_guild.id) {
		LogGuilds("Guild ID [{}] not found in database.", db_guild.id);
		return false;
	}

	LogGuilds("Found guild id [{}].  Loading details.....", db_guild.id);
	_CreateGuild(db_guild.id, db_guild.name.c_str(), db_guild.leader, db_guild.minstatus, db_guild.motd.c_str(), db_guild.motd_setter.c_str(), db_guild.channel.c_str(), db_guild.url.c_str(), db_guild.favor);
	auto guild = GetGuildByGuildID(guild_id);
	auto where_filter = fmt::format("guild_id = '{}'", guild_id);
	auto g_ranks = BaseGuildRanksRepository::GetWhere(*m_db, where_filter);
	for (auto const& r : g_ranks) {
		guild->rank_names[r.rank] = r.title;
	}

	where_filter = fmt::format("guild_id = '{}'", guild_id);
	auto g_permissions = BaseGuildPermissionsRepository::GetWhere(*m_db, where_filter);
	for (auto const& p : g_permissions) {
		guild->functions[p.perm_id].id = p.id;
		guild->functions[p.perm_id].guild_id = p.guild_id;
		guild->functions[p.perm_id].perm_id = p.perm_id;
		guild->functions[p.perm_id].perm_value = p.permission;
	}

	auto g_tributes = BaseGuildTributesRepository::FindOne(*m_db, guild_id);
	if (g_tributes.guild_id) {
		guild->tribute.id_1			  = g_tributes.tribute_id_1;
		guild->tribute.id_2			  = g_tributes.tribute_id_2;
		guild->tribute.id_1_tier	  = g_tributes.tribute_id_1_tier;
		guild->tribute.id_2_tier	  = g_tributes.tribute_id_2_tier;
		guild->tribute.enabled		  = g_tributes.enabled;
	}

	LogGuilds("Successfully refreshed guild id [{}] from the database", guild_id);
	LogGuilds("Timer has [{}] time remaining from the refresh.", guild->tribute.time_remaining);

	return true;
}

BaseGuildManager::GuildInfo* BaseGuildManager::_CreateGuild(uint32 guild_id, std::string guild_name, uint32 leader_char_id, uint8 minstatus, std::string guild_motd, std::string motd_setter, std::string Channel, std::string URL, uint32 favor)
{
	std::map<uint32, GuildInfo*>::iterator res;

	//remove any old entry.
	res = m_guilds.find(guild_id);
	if (res != m_guilds.end()) {
		delete res->second;
		m_guilds.erase(res);
	}

	//make the new entry and store it into the map.
	auto info = new GuildInfo;
	info->name = guild_name;
	info->leader = leader_char_id;
	info->minstatus = minstatus;
	info->motd = guild_motd;
	info->motd_setter = motd_setter;
	info->url = URL;
	info->channel = Channel;
	info->tribute.favor = favor;

	for (auto r : default_rank_names) {
		info->rank_names[r.id] = r.name;
	}

	for (auto p : default_permissions) {
		info->functions[p.id].id = 0;
		info->functions[p.id].guild_id = guild_id;
		info->functions[p.id].perm_id = p.id;
		info->functions[p.id].perm_value = p.value;
	}

	info->tribute.id_1			 = 0xffffffff;
	info->tribute.id_2			 = 0xffffffff;
	info->tribute.id_1_tier		 = 0;
	info->tribute.id_2_tier		 = 0;
	info->tribute.enabled		 = 0;
	info->tribute.time_remaining = RuleI(Guild, TributeTime);

	m_guilds[guild_id] = info;

	return info;
}

bool BaseGuildManager::_StoreGuildDB(uint32 guild_id) 
{
	if(m_db == nullptr) {
		LogError("Requested to store guild [{}] however there is no database object", guild_id);
		return false;
	}

	auto in = GetGuildByGuildID(guild_id);
	if (!in) {
		LogGuilds("Request to save guild id [{}] to the database however guild could not be found.", guild_id);
		return false;
	}

	{
		BaseGuildsRepository::Guilds out;
		out.id = guild_id;
		out.favor = in->tribute.favor;
		GOUT(channel);
		GOUT(name);
		GOUT(url);
		GOUT(motd);
		GOUT(motd_setter);
		GOUT(leader);
		GOUT(minstatus);

		if (!GuildsRepository::ReplaceOne(*m_db, out)) {
			LogGuilds("Error storing guild [{}] details in the database", guild_id);
			return false;
		}
		LogGuilds("Stored guild [{}] details in the database", guild_id);
	}

	{
		std::vector<BaseGuildRanksRepository::GuildRanks> out;
		BaseGuildRanksRepository::GuildRanks gr;
		for (int i = 1; i <= GUILD_MAX_RANK; i++) {
			gr.guild_id = guild_id;
			gr.rank = i;
			gr.title = in->rank_names[i];
			out.push_back(gr);
		}
		if (!GuildRanksRepository::ReplaceMany(*m_db, out)) {
			LogGuilds("Error storing guild [{}] ranks in the database", guild_id);
			return false;
		}
		LogGuilds("Stored guild [{}] ranks in the database", guild_id);
	}

	{
		std::vector<BaseGuildPermissionsRepository::GuildPermissions> out;
		BaseGuildPermissionsRepository::GuildPermissions gp;
		for (int i = 1; i <= GUILD_MAX_FUNCTIONS; i++) {
			gp.id = in->functions[i].id;
			gp.guild_id = in->functions[i].guild_id;
			gp.perm_id = in->functions[i].perm_id;
			gp.permission = in->functions[i].perm_value;
			out.push_back(gp);
		}
		if (!GuildPermissionsRepository::ReplaceMany(*m_db, out)) {
			LogGuilds("Error storing guild [{}] permissions in the database", guild_id);
			return false;
		}
		LogGuilds("Stored guild [{}] permissions in the database", guild_id);

		BaseGuildTributesRepository::GuildTributes gt{};
		gt.tribute_id_1		 = in->tribute.id_1;
		gt.tribute_id_2		 = in->tribute.id_2;
		gt.tribute_id_1_tier = in->tribute.id_1_tier;
		gt.tribute_id_2_tier = in->tribute.id_2_tier;
		gt.enabled			 = in->tribute.enabled;
		gt.time_remaining	 = in->tribute.time_remaining;
		GuildTributesRepository::ReplaceOne(*m_db, gt);
	}

	LogGuilds("Stored guild [{}] in the database successfully.", guild_id);

	return true;
}

uint32 BaseGuildManager::_GetFreeGuildID() 
{
	if(m_db == nullptr) {
		LogError("Requested to find a free guild ID however there is no database object");
		return GUILD_NONE;
	}
	BaseGuildsRepository::DeleteWhere(*m_db, "`name` = ''");

	BaseGuildsRepository::Guilds out;
	out.id          = 0;
	out.leader      = 0;
	out.minstatus   = 0;
	out.tribute     = 0;
	out.name        = "";
	out.motd        = "";
	out.motd_setter = "";
	out.url         = "";
	out.channel     = "";
	auto last_insert_id = BaseGuildsRepository::InsertOne(*m_db, out);
	if (last_insert_id.id > 0) {
		LogGuilds("Located a free guild ID [{}] in the database", last_insert_id.id);
		return last_insert_id.id;
	}

	LogGuilds("Unable to find a free guild ID in the database");
	return GUILD_NONE;
}

uint32 BaseGuildManager::CreateGuild(std::string name, uint32 leader_char_id) 
{
	uint32 guild_id = DBCreateGuild(name, leader_char_id);
	if(guild_id == GUILD_NONE)
		return(GUILD_NONE);
	RefreshGuild(guild_id);
	SendGuildRefresh(guild_id, true, false, false, false);
	SendCharRefresh(GUILD_NONE, guild_id, leader_char_id);

	return guild_id;
}
	
bool BaseGuildManager::DeleteGuild(uint32 guild_id) 
{
	if (!DBDeleteGuild(guild_id)) {
		return false;
	}

	SendGuildDelete(guild_id);
	return true;
}

bool BaseGuildManager::RenameGuild(uint32 guild_id, std::string name) {
	if(!DBRenameGuild(guild_id, name))
		return false;

	SendGuildRefresh(guild_id, true, false, false, false);

	return(true);
}

bool BaseGuildManager::SetGuildLeader(uint32 guild_id, uint32 leader_char_id) 
{
	//get old leader first.
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end())
		return(false);
	GuildInfo *info = res->second;
	uint32 old_leader = info->leader;

	if(!DBSetGuildLeader(guild_id, leader_char_id))
		return(false);

	SendGuildRefresh(guild_id, false, false, false, false);
	SendCharRefresh(GUILD_NONE, guild_id, old_leader);
	SendCharRefresh(GUILD_NONE, guild_id, leader_char_id);

	return(true);
}

bool BaseGuildManager::SetGuildMOTD(uint32 guild_id, std::string motd, std::string setter) {
	if(!DBSetGuildMOTD(guild_id, motd, setter))
		return false;

	SendGuildRefresh(guild_id, false, true, false, false);

	return(true);
}

bool BaseGuildManager::SetGuildURL(uint32 GuildID, std::string URL)
{
	if(!DBSetGuildURL(GuildID, URL))
		return false;

	SendGuildRefresh(GuildID, false, true, false, false);

	return(true);
}

bool BaseGuildManager::SetGuildChannel(uint32 GuildID, std::string Channel)
{
	if(!DBSetGuildChannel(GuildID, Channel))
		return false;

	SendGuildRefresh(GuildID, false, true, false, false);

	return(true);
}

bool BaseGuildManager::SetGuild(uint32 charid, uint32 guild_id, uint8 rank) 
{
	if (rank > GUILD_MAX_RANK) {
		return false;
	}

	//lookup their old guild, if they had one.
	uint32 old_guild = GUILD_NONE;
	CharGuildInfo gci;
	if(GetCharInfo(charid, gci)) {
		old_guild = gci.guild_id;
	}

	if (!DBSetGuild(charid, guild_id, rank)) {
		return false;
	}

	SendGuildRefresh(guild_id, false, false, false, false);
	SendCharRefresh(old_guild, guild_id, charid);

	return true;
}

bool BaseGuildManager::SetGuildRank(uint32 charid, uint8 rank) {
	if(rank > GUILD_MAX_RANK)
		return(false);

	if(!DBSetGuildRank(charid, rank))
		return(false);

	auto guild_id = GetGuildIDByCharacterID(charid);
	
	SendGuildRefresh(guild_id,false, false, false, false);

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

uint32 BaseGuildManager::DBCreateGuild(std::string name, uint32 leader) 
{
	auto new_id = _GetFreeGuildID();
	if (new_id == GUILD_NONE) {
		return GUILD_NONE;
	}

	_CreateGuild(new_id, name, leader, 0, "", "", "", "", 0);

	if(!_StoreGuildDB(new_id)) {
		LogGuilds("Error storing new guild with id [{}]", new_id);
		return GUILD_NONE;
	}

	LogGuilds("Created new guild with id [{}] in the database", new_id);

	return new_id;
}

bool BaseGuildManager::DBDeleteGuild(uint32 guild_id, bool local_delete, bool db_delete) 
{
	if (local_delete) {
		auto where_filter = fmt::format("guildid = {}", guild_id);
		auto bank_items = BaseGuildBankRepository::GetWhere(*m_db, where_filter);
		if (!bank_items.empty()) {
			LogError("Attempt to delete guild id [{}] that still has [{}] items in the bank. Please remove them and try again.",
				guild_id,
				bank_items.size()
			);
			LogGuilds("Attempt to delete guild id [{}] that still has [{}] items in the bank. Please remove them and try again.",
				guild_id,
				bank_items.size()
			);
			return false;
		}
		else
		{
			std::map<uint32, GuildInfo*>::iterator res;
			res = m_guilds.find(guild_id);
			if (res != m_guilds.end()) {
				delete res->second;
				m_guilds.erase(res);
				LogGuilds("Deleted guild [{}] from memory", guild_id);
				//Does this need to be sent to world?
			}
		}
	}

	if (db_delete) 
	{
		if (m_db == nullptr) 
		{
			LogError("Requested to delete guild [{}] when we have no database object", guild_id);
			return(false);
		}

		auto where_filter = fmt::format("guildid = {}", guild_id);
		auto bank_items = BaseGuildBankRepository::GetWhere(*m_db, where_filter);
		if (!bank_items.empty()) {
			LogError("Attempt to delete guild id [{}] that still has [{}] items in the bank. Please remove them and try again.",
				guild_id,
				bank_items.size()
			);
			LogGuilds("Attempt to delete guild id [{}] that still has [{}] items in the bank. Please remove them and try again.",
				guild_id,
				bank_items.size()
			);
			return false;
		}
		else 
		{
			auto where_filter = fmt::format("guild_id = {}", guild_id);
			BaseGuildTributesRepository::DeleteOne(*m_db, guild_id);
			BaseGuildsRepository::DeleteOne(*m_db, guild_id);
			BaseGuildRanksRepository::DeleteWhere(*m_db, where_filter);
			BaseGuildPermissionsRepository::DeleteWhere(*m_db, where_filter);
			BaseGuildMembersRepository::DeleteWhere(*m_db, where_filter);
			LogGuilds("Deleted guild [{}] from the database", guild_id);
		}
	}
	return true;
}

bool BaseGuildManager::DBRenameGuild(uint32 guild_id, std::string new_name) 
{
	if(m_db == nullptr) {
		LogGuilds("Requested to rename guild [{}] when we have no database object", guild_id);
		return false;
	}

	auto in = GetGuildByGuildID(guild_id);
	if (!in) {
		LogGuilds("Request to rename guild id [{}] though guild could not be found", guild_id);
		return false;
	}

	auto old_name = in->name;
	in->name = new_name;
	BaseGuildsRepository::Guilds out;
	out.id = guild_id;
	out.favor = in->tribute.favor;
	GOUT(channel);
	GOUT(name);
	GOUT(url);
	GOUT(motd);
	GOUT(motd_setter);
	GOUT(leader);
	GOUT(minstatus);
	if (BaseGuildsRepository::UpdateOne(*m_db, out)) {
		LogGuilds("Renamed guild id [{}] ([{}]) to [{}] in database", guild_id, old_name.c_str(), in->name.c_str());
		return true;
	}
	return false;
}

bool BaseGuildManager::DBSetGuildLeader(uint32 guild_id, uint32 leader) 
{
	if (m_db == nullptr) {
		LogGuilds("Requested to appoint new guild leader for guild [{}] when we have no database object", guild_id);
		return false;
	}

	auto in = GetGuildByGuildID(guild_id);
	if (!in) {
		LogGuilds("Request to appoint new guild leader for guild id [{}] though guild could not be found", guild_id);
		return false;
	}

	auto old_leader = in->leader;
	in->leader = leader;
	BaseGuildsRepository::Guilds out;
	out.id = guild_id;
	GOUT(channel);
	GOUT(name);
	GOUT(url);
	GOUT(motd);
	GOUT(motd_setter);
	GOUT(leader);
	GOUT(minstatus);
	if (!BaseGuildsRepository::UpdateOne(*m_db, out)) {
		LogGuilds("Could not make character id [{}] the leader for guild id [{}] in database", leader, guild_id);
		return false;
	}

	if (!DBSetGuildRank(old_leader, GUILD_OFFICER))
	{
		return false;
	}
	
	if (!DBSetGuildRank(out.leader, GUILD_LEADER)) {
		return false;
	}
	
	LogGuilds("Set guild leader for guild [{}] to [{}] in the database", guild_id, leader);

	return true;
}

bool BaseGuildManager::DBSetGuildMOTD(uint32 guild_id, std::string motd, std::string setter) 
{
	if(m_db == nullptr) {
		LogGuilds("Requested to set the MOTD for guild [{}] however there is no database object", guild_id);
		return(false);
	}

	auto in = GetGuildByGuildID(guild_id);
	if (!in) {
		LogGuilds("Request to rename guild id [{}] though guild could not be found", guild_id);
		return false;
	}

	in->motd = motd;
	in->motd_setter = setter;
	BaseGuildsRepository::Guilds out;
	out.id = guild_id;
	out.favor = in->tribute.favor;
	GOUT(channel);
	GOUT(name);
	GOUT(url);
	GOUT(motd);
	GOUT(motd_setter);
	GOUT(leader);
	GOUT(minstatus);
	if (BaseGuildsRepository::UpdateOne(*m_db, out)) {
		LogGuilds("Updated the motd for guild id [{}] in database", guild_id);
		return true;
	}
	return false;
}

bool BaseGuildManager::DBSetGuildURL(uint32 guild_id, std::string URL)
{
	if (m_db == nullptr) {
		LogGuilds("Requested to set the URL for guild [{}] however there is no database object", guild_id);
		return(false);
	}

	auto in = GetGuildByGuildID(guild_id);
	if (!in) {
		LogGuilds("Request to update url for guild id [{}] though guild could not be found", guild_id);
		return false;
	}

	in->url = URL;
	BaseGuildsRepository::Guilds out;
	out.id = guild_id;
	out.favor = in->tribute.favor;
	GOUT(channel);
	GOUT(name);
	GOUT(url);
	GOUT(motd);
	GOUT(motd_setter);
	GOUT(leader);
	GOUT(minstatus);
	if (BaseGuildsRepository::UpdateOne(*m_db, out)) {
		LogGuilds("Updated the url for guild id [{}] in database", guild_id);
		return true;
	}
	return false;
}

bool BaseGuildManager::DBSetGuildChannel(uint32 guild_id, std::string Channel)
{
	if (m_db == nullptr) {
		LogGuilds("Requested to set the Channel for guild [{}] however there is no database object", guild_id);
		return(false);
	}

	auto in = GetGuildByGuildID(guild_id);
	if (!in) {
		LogGuilds("Request to update channel message for guild id [{}] though guild could not be found", guild_id);
		return false;
	}

	in->channel = Channel;
	BaseGuildsRepository::Guilds out;
	out.id = guild_id;
	out.favor = in->tribute.favor;
	GOUT(channel);
	GOUT(name);
	GOUT(url);
	GOUT(motd);
	GOUT(motd_setter);
	GOUT(leader);
	GOUT(minstatus);
	if (BaseGuildsRepository::UpdateOne(*m_db, out)) {
		LogGuilds("Updated the channel message for guild id [{}] in database", guild_id);
		return true;
	}
	return false;
}

bool BaseGuildManager::DBSetGuild(uint32 char_id, uint32 guild_id, uint8 rank) 
{
	if(m_db == nullptr) {
		LogGuilds("Requested to set char [{}] to guild [{}] when we have no database object", char_id, guild_id);
		return false;
	}
	if (guild_id == GUILD_NONE) {
		if (!GuildMembersRepository::DeleteOne(*m_db, char_id)) {
			LogError("Request to remove a character id {} from guild_members who is not in a guild {}.", char_id, guild_id);
				return false;
		}
		else {
			LogGuilds("Removed character id {} from guild id {}", char_id, guild_id);
				return true;
		}
	}

	BaseGuildMembersRepository::GuildMembers out;
	out.alt = 0;
	out.banker = 0;
	out.char_id = char_id;
	out.guild_id = guild_id;
	out.last_tribute = 0;
	out.public_note = "";
	out.rank = rank;
	out.total_tribute = 0;
	out.tribute_enable = 0;
	auto member = GuildMembersRepository::ReplaceOne(*m_db, out);
	if (!member) {
		LogGuilds("Error adding character id [{}] to guild id [{}]", char_id, guild_id);
		return false;		
	}

	LogGuilds("Set char [{}] to guild [{}] and rank [{}] in the database", char_id, guild_id, rank);
	return true;
}

bool BaseGuildManager::DBSetGuildRank(uint32 char_id, uint8 rank_id) 
{
	if (!GuildMembersRepository::UpdateMemberRank(*m_db, char_id, rank_id)) {
		return false;
	}
	return true;
}

bool BaseGuildManager::DBSetBankerFlag(uint32 charid, bool is_banker) {
	std::string query = StringFormat("UPDATE guild_members SET banker=%d WHERE char_id=%d",
		is_banker? 1: 0, charid);
	return(QueryWithLogging(query, "setting a guild member's banker flag"));
}

bool BaseGuildManager::GetBankerFlag(uint32 CharID, bool compat_mode)
{
	if (!m_db) {
		return false;
	}
	
	auto db_banker = 0;
	auto member = GuildMembersRepository::FindOne(*m_db, CharID);

	if (compat_mode) {
		auto db_banker = member.banker;
		return db_banker;
	}

	return member.banker || GetGuildBankerStatus(member.guild_id, member.rank);
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

	return Strings::ToBool(row[0]);
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

#define GuildMemberBaseQuery \
"SELECT c.`id`, c.`name`, c.`class`, c.`level`, c.`last_login`, c.`zone_id`," \
" g.`guild_id`, g.`rank`, g.`tribute_enable`, g.`total_tribute`, g.`last_tribute`," \
" g.`banker`, g.`public_note`, g.`alt` " \
" FROM `character_data` AS c LEFT JOIN `guild_members` AS g ON c.`id` = g.`char_id` "
static void ProcessGuildMember(MySQLRequestRow row, CharGuildInfo &into) {
	//fields from `characer_`
	into.char_id		= Strings::ToUnsignedInt(row[0]);
	into.char_name		= row[1];
	into.class_			= Strings::ToUnsignedInt(row[2]);
	into.level			= Strings::ToUnsignedInt(row[3]);
	into.time_last_on	= Strings::ToUnsignedInt(row[4]);
	into.zone_id		= Strings::ToUnsignedInt(row[5]);

	//fields from `guild_members`, leave at defaults if missing
	into.guild_id		= row[6] ? Strings::ToUnsignedInt(row[6]) : GUILD_NONE;
	into.rank			= row[7] ? Strings::ToUnsignedInt(row[7]) : (GUILD_MAX_RANK);
	into.tribute_enable = row[8] ? (row[8][0] == '0'?false:true) : false;
	into.total_tribute	= row[9] ? Strings::ToUnsignedInt(row[9]) : 0;
	into.last_tribute	= row[10]? Strings::ToUnsignedInt(row[10]) : 0;		//timestamp
	into.banker			= row[11]? (row[11][0] == '0'?false:true) : false;
	into.public_note	= row[12]? row[12] : "";
	into.alt		    = row[13] ? (row[13][0] == '0' ? false : true) : false;
	
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
	std::string query   = StringFormat(GuildMemberBaseQuery " WHERE c.id=%d AND c.deleted_at IS NULL", char_id);
	auto        results = m_db->QueryDatabase(query);
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
	return(res->second->rank_names[rank].c_str());
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
	LogGuilds("Check leader for guild [{}], char [{}]: leader id=[{}]", guild_id, char_id, res->second->leader);
	return(char_id == res->second->leader);
}

uint32 BaseGuildManager::FindGuildByLeader(uint32 leader) const {
	std::map<uint32, GuildInfo *>::const_iterator cur, end;
	cur = m_guilds.begin();
	end = m_guilds.end();
	for(; cur != end; ++cur) {
		if(cur->second->leader == leader)
			return(cur->first);
	}
	return(GUILD_NONE);
}

//returns the rank to be sent to the client for display purposes, given their eqemu rank.
uint8 BaseGuildManager::GetDisplayedRank(uint32 guild_id, uint8 rank, uint32 char_id) const 
{
	return rank;
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
		LogGuilds("Check permission on guild [{}] and rank [{}] for action ([{}]): Invalid rank, denied",
			guild_id, rank, act);
		return(false);	//invalid rank
	}
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		LogGuilds("Check permission on guild [{}] and rank [{}] for action ([{}]): Invalid guild, denied",
			guild_id, rank, act);
		return(false);	//invalid guild
	}

	bool granted = (res->second->functions[act].perm_value >> (8 - rank)) & 1;

	return granted;
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
//	for(r = 0; r < _MaxGuildAction; r++)
//		permissions[r] = false;
}

BaseGuildManager::GuildInfo::GuildInfo() {
	leader = 0;
	minstatus = AccountStatus::Player;
}

uint32 BaseGuildManager::DoesAccountContainAGuildLeader(uint32 AccountID)
{
	
	std::string query = StringFormat("SELECT guild_id FROM guild_members WHERE char_id IN "
		"(SELECT id FROM `character_data` WHERE account_id = %i) AND rank = 1",
		AccountID);
    auto results = m_db->QueryDatabase(query);
	if (!results.Success())
	{
		return 0;
	}

	return results.RowCount();
}

std::string BaseGuildManager::GetGuildNameByID(uint32 guild_id) const {
	if(guild_id == GUILD_NONE) {
		return std::string();
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		return "Invalid Guild";
	}

	return res->second->name;
}

std::string BaseGuildManager::GetGuildRankName(uint32 guild_id, uint8 rank) const
{
	if(rank > GUILD_MAX_RANK) {
		return "Invalid Rank";
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = m_guilds.find(guild_id);
	if(res == m_guilds.end()) {
		return "Invalid Guild Rank";
	}

	return res->second->rank_names[rank];
}

uint32 BaseGuildManager::GetGuildIDByCharacterID(uint32 character_id)
{
    if(!m_db) {
		return GUILD_NONE;
	}

    std::string query = fmt::format(
		"SELECT `guild_id` FROM `guild_members` WHERE char_id = {} LIMIT 1",
		character_id
	);
    auto results = m_db->QueryDatabase(query);
	if(!results.Success() || !results.RowCount()) {
		return GUILD_NONE;
	}

	auto row = results.begin();
	auto guild_id = Strings::ToUnsignedInt(row[0]);
	return guild_id;
}

bool BaseGuildManager::IsCharacterInGuild(uint32 character_id, uint32 guild_id)
{
	auto current_guild_id = GetGuildIDByCharacterID(character_id);

	if (current_guild_id == GUILD_NONE) {
		return false;
	}

	if (guild_id && current_guild_id != guild_id) {
		return false;
	}

	return true;
}

BaseGuildManager::GuildInfo* BaseGuildManager::GetGuildByGuildID(uint32 guild_id) 
{
	auto guild = m_guilds.find(guild_id);
	if (guild != m_guilds.end()) {
		return guild->second;
	}
	return nullptr;
}

bool BaseGuildManager::GetGuildBankerStatus(uint32 guild_id, uint32 guild_rank)
{
	auto guild = m_guilds.find(guild_id);
	if (guild != m_guilds.end()) {
		return (CheckPermission(guild_id, guild_rank, GUILD_ACTION_BANK_DEPOSIT_ITEMS) &&
			CheckPermission(guild_id, guild_rank, GUILD_ACTION_BANK_PROMOTE_ITEMS) &&
			CheckPermission(guild_id, guild_rank, GUILD_ACTION_BANK_VIEW_ITEMS) &&
			CheckPermission(guild_id, guild_rank, GUILD_ACTION_BANK_WITHDRAW_ITEMS)) ? true : false;
	}
	return false;
}

std::vector<BaseGuildMembersRepository::GuildMembers> BaseGuildManager::GetGuildMembers(uint32 guild_id) 
{
	std::string where_filter = fmt::format("`guild_id` = '{}'", guild_id);
	auto guild_members = GuildMembersRepository::GetWhere(*m_db, where_filter);
	return guild_members;
}

bool BaseGuildManager::StoreGuildDB(uint32 guild_id) {
	return _StoreGuildDB(guild_id);
}

uint32 BaseGuildManager::DBSetGuildFavor(uint32 guild_id, uint32 favor)
{
	if (m_db == nullptr) {
		LogGuilds("Requested to set favor [{}] to guild [{}] when we have no database object", favor, guild_id);
		return false;
	}

	if (!GuildsRepository::UpdateFavor(*m_db, guild_id, favor)) {
		LogError("Error updating guild favor [{}] for guild id [{}] in database.", favor, guild_id);
		return false;
	}

	LogGuilds("Set guild favor of [{}] for guild id [{}] in the database", favor, guild_id);

	return favor;
}

bool BaseGuildManager::DBSetGuildTributeEnabled(uint32 guild_id, uint32 enabled)
{
	if (m_db == nullptr) {
		LogGuilds("Requested to set tribute enabled [{}] to guild [{}] when we have no database object", enabled, guild_id);
		return false;
	}

	if (!GuildTributesRepository::UpdateEnabled(*m_db, guild_id, enabled)) {
		LogError("Error updating tribute enabled [{}] for guild id [{}] in database.", enabled, guild_id);
		return false;
	}

	LogGuilds("Set tribute enabled [{}] for guild id [{}] in the database", enabled, guild_id);

	return true;
}

bool BaseGuildManager::DBSetTributeTimeRemaining(uint32 guild_id, uint32 time_remaining)
{
	if (m_db == nullptr) {
		LogGuilds("Requested to set tribute time_remaining [{}] to guild [{}] when we have no database object", time_remaining, guild_id);
		return false;
	}

	if (!GuildTributesRepository::UpdateTimeRemaining(*m_db, guild_id, time_remaining)) {
		LogError("Error updating tribute time_remaining [{}] for guild id [{}] in database.", time_remaining, guild_id);
		return false;
	}

	LogGuilds("Set tribute time_remaining [{}] for guild id [{}] in the database", time_remaining, guild_id);

	return true;
}

bool BaseGuildManager::DBSetMemberTributeEnabled(uint32 guild_id, uint32 char_id, uint32 enabled)
{
	CharGuildInfo gci;
	GetCharInfo(char_id, gci);
	if (gci.char_name.empty()) {
		LogGuilds("Requested to set member id {} tribute to enabled [{}] in guild [{}] but we could not find the character.", char_id, enabled, guild_id);
		return false;
	}

	if (m_db == nullptr) {
		LogGuilds("Requested to set member id {} tribute enabled [{}] in guild [{}] when we have no database object", gci.char_name.c_str(), enabled, guild_id);
		return false;
	}

	if (!GuildMembersRepository::UpdateEnabled(*m_db, guild_id, char_id, enabled)) {
		LogError("Error updating member id {} tribute enabled [{}] for guild id [{}] in database.", char_id, enabled, guild_id);
		return false;
	}

	LogGuilds("Set member {} id {} tribute enabled [{}] for guild id [{}] in the database", gci.char_name.c_str(), char_id, enabled, guild_id);

	return true;
}

uint32 BaseGuildManager::DBSetMemberFavor(uint32 guild_id, uint32 char_id, uint32 favor)
{
	CharGuildInfo gci;
	GetCharInfo(char_id, gci);

	if (gci.char_name.empty()) {
		LogGuilds("Requested to set member id {} tribute to favor [{}] in guild [{}] but we could not find the character.", char_id, favor, guild_id);
		return false;
	}

	if (m_db == nullptr) {
		LogGuilds("Requested to set member id {} tribute favor [{}] in guild [{}] when we have no database object", gci.char_name.c_str(), favor, guild_id);
		return false;
	}

	gci.total_tribute += favor;
	if (!GuildMembersRepository::UpdateFavor(*m_db, guild_id, char_id, gci.total_tribute)) {
		LogError("Error updating member id {} tribute favor [{}] for guild id [{}] in database.", char_id, favor, guild_id);
		return false;
	}

	LogGuilds("Set member {} id {} tribute enabled [{}] for guild id [{}] in the database", gci.char_name.c_str(), char_id, favor, guild_id);

	return gci.total_tribute;
}


