/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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
#include "../common/eq_packet.h"
#include "../common/eq_stream_intf.h"
#include "../common/misc.h"
#include "../common/rulesys.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "../common/eq_stream_intf.h"
#include "../common/inventory_profile.h"
#include "../common/races.h"
#include "../common/classes.h"
#include "../common/languages.h"
#include "../common/skills.h"
#include "../common/extprofile.h"
#include "../common/string_util.h"
#include "../common/emu_versions.h"
#include "../common/random.h"
#include "../common/shareddb.h"

#include "client.h"
#include "worlddb.h"
#include "world_config.h"
#include "login_server.h"
#include "login_server_list.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "clientlist.h"
#include "wguild_mgr.h"
#include "sof_char_create_data.h"

#include <iostream>
#include <iomanip>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <limits.h>

//FatherNitwit: uncomment to enable my IP based authentication hack
//#define IPBASED_AUTH_HACK

// Disgrace: for windows compile
#ifdef _WINDOWS
	#include <winsock2.h>
	#include <windows.h>
#else
	
	#ifdef FREEBSD //Timothy Whitman - January 7, 2003
		#include <sys/types.h>
	#endif

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

std::vector<RaceClassAllocation> character_create_allocations;
std::vector<RaceClassCombos> character_create_race_class_combos;

extern ZSList zoneserver_list;
extern LoginServerList loginserverlist;
extern ClientList client_list;
extern EQEmu::Random emu_random;
extern uint32 numclients;
extern volatile bool RunLoops;
extern volatile bool UCSServerAvailable_;

Client::Client(EQStreamInterface* ieqs)
:	autobootup_timeout(RuleI(World, ZoneAutobootTimeoutMS)),
	CLE_keepalive_timer(RuleI(World, ClientKeepaliveTimeoutMS)),
	connect(1000),
	eqs(ieqs)
{
	// Live does not send datarate as of 3/11/2005
	//eqs->SetDataRate(7);
	ip = eqs->GetRemoteIP();
	port = ntohs(eqs->GetRemotePort());

	autobootup_timeout.Disable();
	connect.Disable();
	seen_character_select = false;
	cle = 0;
	zone_id = 0;
	char_name[0] = 0;
	charid = 0;
	zone_waiting_for_bootup = 0;
	StartInTutorial = false;

	m_ClientVersion = eqs->ClientVersion();
	m_ClientVersionBit = EQEmu::versions::ConvertClientVersionToClientVersionBit(m_ClientVersion);
	
	numclients++;
}

Client::~Client() {
	if (RunLoops && cle && zone_id == 0)
		cle->SetOnline(CLE_Status_Offline);

	numclients--;

	//let the stream factory know were done with this stream
	eqs->Close();
	eqs->ReleaseFromUse();
	safe_delete(eqs);
}

void Client::SendLogServer()
{
	auto outapp = new EQApplicationPacket(OP_LogServer, sizeof(LogServer_Struct));
	LogServer_Struct *l=(LogServer_Struct *)outapp->pBuffer;
	const char *wsn=WorldConfig::get()->ShortName.c_str();
	memcpy(l->worldshortname,wsn,strlen(wsn));

	if(RuleB(Mail, EnableMailSystem))
		l->enablemail = 1;

	if(RuleB(Chat, EnableVoiceMacros))
		l->enablevoicemacros = 1;

	l->enable_pvp = (RuleI(World, PVPSettings));

	if(RuleB(World, IsGMPetitionWindowEnabled))
		l->enable_petition_wnd = 1;

	if((RuleI(World, FVNoDropFlag) == 1 || RuleI(World, FVNoDropFlag) == 2) && GetAdmin() > RuleI(Character, MinStatusForNoDropExemptions))
		l->enable_FV = 1;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendEnterWorld(std::string name)
{
	char char_name[64] = { 0 };
	if (is_player_zoning && database.GetLiveChar(GetAccountID(), char_name)) {
		if(database.GetAccountIDByChar(char_name) != GetAccountID()) {
			eqs->Close();
			return;
		} else {
			Log(Logs::Detail, Logs::World_Server,"Telling client to continue session.");
		}
	}

	auto outapp = new EQApplicationPacket(OP_EnterWorld, strlen(char_name) + 1);
	memcpy(outapp->pBuffer,char_name,strlen(char_name)+1);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendExpansionInfo() {
	auto outapp = new EQApplicationPacket(OP_ExpansionInfo, sizeof(ExpansionInfo_Struct));
	ExpansionInfo_Struct *eis = (ExpansionInfo_Struct*)outapp->pBuffer;
	if(RuleB(World, UseClientBasedExpansionSettings)) {
		eis->Expansions = EQEmu::versions::ConvertClientVersionToExpansion(eqs->ClientVersion());
	} else {
		eis->Expansions = (RuleI(World, ExpansionSettings));
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendCharInfo() {
	if (cle) {
		cle->SetOnline(CLE_Status_CharSelect);
	}

	if (m_ClientVersionBit & EQEmu::versions::bit_RoFAndLater) {
		SendMaxCharCreate();
		SendMembership();
		SendMembershipSettings();
	}

	seen_character_select = true;

	// Send OP_SendCharInfo
	EQApplicationPacket *outapp = nullptr;
	database.GetCharSelectInfo(GetAccountID(), &outapp, m_ClientVersionBit);

	if (outapp) {
		QueuePacket(outapp);
	}
	else {
		Log(Logs::General, Logs::World_Server, "[Error] Database did not return an OP_SendCharInfo packet for account %u", GetAccountID());
	}
	safe_delete(outapp);
}

void Client::SendMaxCharCreate() {
	auto outapp = new EQApplicationPacket(OP_SendMaxCharacters, sizeof(MaxCharacters_Struct));
	MaxCharacters_Struct* mc = (MaxCharacters_Struct*)outapp->pBuffer;

	mc->max_chars = EQEmu::constants::Lookup(m_ClientVersion)->CharacterCreationLimit;
	if (mc->max_chars > EQEmu::constants::CharacterCreationMax)
		mc->max_chars = EQEmu::constants::CharacterCreationMax;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMembership() {
	auto outapp = new EQApplicationPacket(OP_SendMembership, sizeof(Membership_Struct));
	Membership_Struct* mc = (Membership_Struct*)outapp->pBuffer;

	/*
		The remaining entry fields probably hold more membership restriction data that needs to be identified.
		Here is a possible list based on the EQ Website membership comparison list:
		1. Spell Ranks Allowed
		2. Prestige Items Usable
		3. In-Game Mail Service (send/recieve)
		4. Parcel Delivery (send/recieve)
		5. Loyalty Rewards Per-Week (30, 60, Max)
		6. Mercenary Tiers (Apprentice 1-2, Apprentice All Tiers, All Tiers)
		7. Neighborhood House
		8. Active Journal Quests (Tasks?) (10, 15, Max)
		9. Guild Function (join, join and create)
		10. Broker System (restricted, unlimited)
		11. Voice Chat
		12. Chat Ability
		13. Progression Server
		14. Customer Support
		15. In-Game Popup Advertising
		That is 15 possible fields, and there are 15 unknowns in the struct...Coincidence?
	*/

	mc->membership = 2;				//Hardcode to gold for now. We don't use anything else.
	mc->races = 0x1ffff;			// Available Races (4110 for silver)
	mc->classes = 0x1ffff;			// Available Classes (4614 for silver) - Was 0x101ffff
	mc->entrysize = 21;				// Number of membership setting entries below
	mc->entries[0] = 0xffffffff;	// Max AA Restriction
	mc->entries[1] = 0xffffffff;	// Max Level Restriction
	mc->entries[2] = 0xffffffff;	// Max Char Slots per Account (not used by client?)
	mc->entries[3] = 0xffffffff;	// 1 for Silver
	mc->entries[4] = 8;				// Main Inventory Size (0xffffffff on Live for Gold, but limiting to 8 until 10 is supported)
	mc->entries[5] = 0xffffffff;	// Max Platinum per level
	mc->entries[6] = 1;				// 0 for Silver
	mc->entries[7] = 1;				// 0 for Silver
	mc->entries[8] = 1;				// 1 for Silver
	mc->entries[9] = 0xffffffff;	// Unknown - Maybe Loyalty Points every 12 hours? 60 per week for Silver
	mc->entries[10] = 1;			// 1 for Silver
	mc->entries[11] = 0xffffffff;	// Shared Bank Slots
	mc->entries[12] = 0xffffffff;	// Unknown - Maybe Max Active Tasks?
	mc->entries[13] = 1;			// 1 for Silver
	mc->entries[14] = 1;			// 0 for Silver
	mc->entries[15] = 1;			// 0 for Silver
	mc->entries[16] = 1;			// 1 for Silver
	mc->entries[17] = 1;			// 0 for Silver
	mc->entries[18] = 1;			// 0 for Silver
	mc->entries[19] = 0xffffffff;	// 0 for Silver
	mc->entries[20] = 0xffffffff;	// 0 for Silver
	mc->exit_url_length = 0;
	//mc->exit_url = 0; // Used on Live: "http://www.everquest.com/free-to-play/exit-silver"

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMembershipSettings() {
	auto outapp = new EQApplicationPacket(OP_SendMembershipDetails, sizeof(Membership_Details_Struct));
	Membership_Details_Struct* mds = (Membership_Details_Struct*)outapp->pBuffer;

	mds->membership_setting_count = 66;
	int32 gold_settings[22] = {-1,-1,-1,-1,-1,-1,1,1,1,-1,1,-1,-1,1,1,1,1,1,1,-1,-1,0};
	uint32 entry_count = 0;
	for (int setting_id=0; setting_id < 22; setting_id++)
	{
		for (int setting_index=0; setting_index < 3; setting_index++)
		{

			mds->settings[entry_count].setting_index = setting_index;
			mds->settings[entry_count].setting_id = setting_id;
			mds->settings[entry_count].setting_value = gold_settings[setting_id];
			entry_count++;
		}
	}

	mds->race_entry_count = 15;
	mds->class_entry_count = 15;

	uint32 cur_purchase_id = 90287;
	uint32 cur_purchase_id2 = 90301;
	uint32 cur_bitwise_value = 1;
	for (int entry_id=0; entry_id < 15; entry_id++)
	{
		if (entry_id == 0)
		{
			mds->membership_races[entry_id].purchase_id = 1;
			mds->membership_races[entry_id].bitwise_entry = 0x1ffff;
			mds->membership_classes[entry_id].purchase_id = 1;
			mds->membership_classes[entry_id].bitwise_entry = 0x1ffff;
		}
		else
		{
			mds->membership_races[entry_id].purchase_id = cur_purchase_id;

			if (entry_id < 3)
			{
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id;
			}
			else
			{
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id2;
				cur_purchase_id2++;
			}

			if (entry_id == 1)
			{
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else if (entry_id == 2)
			{
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else
			{
				if (entry_id == 12)
				{
					// Live Skips 4096
					cur_bitwise_value *= 2;
				}
				mds->membership_races[entry_id].bitwise_entry = cur_bitwise_value;
				mds->membership_classes[entry_id].bitwise_entry = cur_bitwise_value;
			}
			cur_purchase_id++;
		}
		cur_bitwise_value *= 2;
	}
	mds->exit_url_length = 0;	// Live uses 42
	//strcpy(eq->exit_url, "http://www.everquest.com/free-to-play/exit");
	mds->exit_url_length2 = 0;	// Live uses 49
	//strcpy(eq->exit_url2, "http://www.everquest.com/free-to-play/exit-silver");

	/*
	Account Access Level Settings

	ID	-	Free	Silver	Gold	-	Possible Setting
	00	-	250		1000	-1		-	Max AA Restriction
	01	-	-1		-1		-1		-	Max Level Restriction
	02	-	2		4		-1		-	Max Char Slots per Account
	03	-	1		1		-1		-	Max Spell Rank
	04	-	4		6		-1		-	Main Inventory Size
	05	-	100		500		-1		-	Max Platinum per level
	06	-	0		0		1		-	Send Mail?
	07	-	0		0		1		-	Send Parcels?
	08	-	1		1		1		-	Voice Chat Unlimited?
	09	-	2		5		-1		-	Mercenary Tiers
	10	-	0		1		1		-	Create Guilds?
	11	-	0		0		-1		-	Shared Bank Slots
	12	-	9		14		-1		-	Max Journal Quests - 1
	13	-	0		1		1		-	Neighborhood-House Allowed?
	14	-	0		0		1		-	Prestige Enabled?
	15	-	0		0		1		-	Broker System Unlimited?
	16	-	0		1		1		-	Chat UnRestricted?
	17	-	0		0		1		-	Progression Server Access?
	18	-	0		0		1		-	Full Customer Support?
	19	-	0		0		-1		-	0 for Silver
	20	-	0		0		-1		-	0 for Silver
	21	-	0		0		0		-	Unknown 0
	*/

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendPostEnterWorld() {
	auto outapp = new EQApplicationPacket(OP_PostEnterWorld, 1);
	outapp->size=0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::HandleSendLoginInfoPacket(const EQApplicationPacket *app) {
	if (app->size != sizeof(LoginInfo_Struct)) {
		return false;
	}

	LoginInfo_Struct *li=(LoginInfo_Struct *)app->pBuffer;

	// Quagmire - max len for name is 18, pass 15
	char name[19] = {0};
	char password[16] = {0};
	strn0cpy(name, (char*)li->login_info,18);
	strn0cpy(password, (char*)&(li->login_info[strlen(name)+1]), 15);

	if (strlen(password) <= 1) {
		// TODO: Find out how to tell the client wrong username/password
		Log(Logs::Detail, Logs::World_Server,"Login without a password");
		return false;
	}

	is_player_zoning=(li->zoning==1);

#ifdef IPBASED_AUTH_HACK
	struct in_addr tmpip;
	tmpip.s_addr = ip;
#endif
	uint32 id=0;
	bool minilogin = loginserverlist.MiniLogin();
	if(minilogin){
		struct in_addr miniip;
		miniip.s_addr = ip;
		id = database.GetMiniLoginAccount(inet_ntoa(miniip));
	}
	else if(strncasecmp(name, "LS#", 3) == 0)
		id=atoi(&name[3]);
	else if(database.GetAccountIDByName(name)){
		int16 status = 0;
		uint32 lsid = 0;		
		id = database.GetAccountIDByName(name, &status, &lsid);
	}
	else
		id=atoi(name);
	if (loginserverlist.Connected() == false && !is_player_zoning) {
		Log(Logs::General, Logs::World_Server,"Error: Login server login while not connected to login server.");
		return false;
	}
	if (((cle = client_list.CheckAuth(name, password)) || (cle = client_list.CheckAuth(id, password))))
	{
		if (cle->AccountID() == 0 || (!minilogin && cle->LSID()==0)) {
			Log(Logs::General, Logs::World_Server,"ID is 0. Is this server connected to minilogin?");
			if (!minilogin) {
				Log(Logs::General, Logs::World_Server, "If so you forget the minilogin variable...");
			}
			else {
				Log(Logs::General, Logs::World_Server, "Could not find a minilogin account, verify ip address logging into minilogin is the same that is in your account table.");
			}
			return false;
		}

		if(minilogin){
			cle->SetOnline();
			WorldConfig::DisableStats();
			Log(Logs::General, Logs::World_Server, "MiniLogin Account #%d",cle->AccountID());
		}
		else if (!is_player_zoning) {
			// Track who is in and who is out of the game
			char *inout= (char *) "";
			
			if (cle->GetOnline() == CLE_Status_Never){
				// Desktop -> Char Select
				inout = (char *) "In";
			}
			else {
				// Game -> Char Select
				inout=(char *) "Out";
			}

			// Always at Char select at this point.
			// Either from a fresh client launch or coming back from the game.
			// Exiting the game entirely does not come through here.
			// Could use a Logging Out Completely message somewhere.
			cle->SetOnline(CLE_Status_CharSelect);
			
			Log(Logs::General, Logs::World_Server, 
				"Account (%s) Logging(%s) to character select :: LSID: %d ", 
				cle->AccountName(), inout, cle->LSID());
		}
		else {
			cle->SetOnline();
		}

		const WorldConfig *Config=WorldConfig::get();

		if(Config->UpdateStats){
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_LSPlayerJoinWorld;
			pack->size = sizeof(ServerLSPlayerJoinWorld_Struct);
			pack->pBuffer = new uchar[pack->size];
			memset(pack->pBuffer,0,pack->size);
			ServerLSPlayerJoinWorld_Struct* join =(ServerLSPlayerJoinWorld_Struct*)pack->pBuffer;
			strcpy(join->key,GetLSKey());
			join->lsaccount_id = GetLSID();
			loginserverlist.SendPacket(pack);
			safe_delete(pack);
		}

		if (!is_player_zoning)
			SendGuildList();

		SendLogServer();
		SendApproveWorld();
		SendEnterWorld(cle->name());
		SendPostEnterWorld();
		if (!is_player_zoning) {
			SendExpansionInfo();
			SendCharInfo();
			database.LoginIP(cle->AccountID(), long2ip(GetIP()).c_str());
		}

	}
	else {
		// TODO: Find out how to tell the client wrong username/password
		Log(Logs::Detail, Logs::World_Server,"Bad/Expired session key '%s'",name);
		return false;
	}

	if (!cle)
		return true;

	cle->SetIP(GetIP());
	return true;
}

bool Client::HandleNameApprovalPacket(const EQApplicationPacket *app)
{
	if (GetAccountID() == 0) {
		Log(Logs::Detail, Logs::World_Server,"Name approval request with no logged in account");
		return false;
	}

	snprintf(char_name, 64, "%s", (char*)app->pBuffer);
	uchar race = app->pBuffer[64];
	uchar clas = app->pBuffer[68];

	Log(Logs::Detail, Logs::World_Server, "Name approval request. Name=%s, race=%s, class=%s", char_name, GetRaceIDName(race), GetClassIDName(clas));

	EQApplicationPacket *outapp;
	outapp = new EQApplicationPacket;
	outapp->SetOpcode(OP_ApproveName);
	outapp->pBuffer = new uchar[1];
	outapp->size = 1;

	bool valid = false;
	if(!database.CheckNameFilter(char_name)) { 
		valid = false; 
	}
	/* Name must begin with an upper-case letter. */
	else if (islower(char_name[0])) { 
		valid = false; 
	} 
	else if (database.ReserveName(GetAccountID(), char_name)) { 
		valid = true; 	
	}
	else { 
		valid = false; 
	}

	outapp->pBuffer[0] = valid? 1 : 0;
	QueuePacket(outapp);
	safe_delete(outapp);

	if (!valid)
		memset(char_name, 0, sizeof(char_name));

	return true;
}

bool Client::HandleGenerateRandomNamePacket(const EQApplicationPacket *app) {
	// creates up to a 10 char name
	char vowels[18]="aeiouyaeiouaeioe";
	char cons[48]="bcdfghjklmnpqrstvwxzybcdgklmnprstvwbcdgkpstrkd";
	char rndname[17]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char paircons[33]="ngrkndstshthphsktrdrbrgrfrclcr";
	int rndnum=emu_random.Int(0, 75),n=1;
	bool dlc=false;
	bool vwl=false;
	bool dbl=false;
	if (rndnum>63)
	{	// rndnum is 0 - 75 where 64-75 is cons pair, 17-63 is cons, 0-16 is vowel
		rndnum=(rndnum-61)*2;	// name can't start with "ng" "nd" or "rk"
		rndname[0]=paircons[rndnum];
		rndname[1]=paircons[rndnum+1];
		n=2;
	}
	else if (rndnum>16)
	{
		rndnum-=17;
		rndname[0]=cons[rndnum];
	}
	else
	{
		rndname[0]=vowels[rndnum];
		vwl=true;
	}
	int namlen=emu_random.Int(5, 10);
	for (int i=n;i<namlen;i++)
	{
		dlc=false;
		if (vwl)	//last char was a vowel
		{			// so pick a cons or cons pair
			rndnum=emu_random.Int(0, 62);
			if (rndnum>46)
			{	// pick a cons pair
				if (i>namlen-3)	// last 2 chars in name?
				{	// name can only end in cons pair "rk" "st" "sh" "th" "ph" "sk" "nd" or "ng"
					rndnum=emu_random.Int(0, 7)*2;
				}
				else
				{	// pick any from the set
					rndnum=(rndnum-47)*2;
				}
				rndname[i]=paircons[rndnum];
				rndname[i+1]=paircons[rndnum+1];
				dlc=true;	// flag keeps second letter from being doubled below
				i+=1;
			}
			else
			{	// select a single cons
				rndname[i]=cons[rndnum];
			}
		}
		else
		{		// select a vowel
			rndname[i]=vowels[emu_random.Int(0, 16)];
		}
		vwl=!vwl;
		if (!dbl && !dlc)
		{	// one chance at double letters in name
			if (!emu_random.Int(0, i+9))	// chances decrease towards end of name
			{
				rndname[i+1]=rndname[i];
				dbl=true;
				i+=1;
			}
		}
	}

	rndname[0]=toupper(rndname[0]);
	NameGeneration_Struct* ngs = (NameGeneration_Struct*)app->pBuffer;
	memset(ngs->name,0,64);
	strcpy(ngs->name,rndname);

	QueuePacket(app);
	return true;
}

bool Client::HandleCharacterCreateRequestPacket(const EQApplicationPacket *app) {
	// New OpCode in SoF
	uint32 allocs = character_create_allocations.size();
	uint32 combos = character_create_race_class_combos.size();
	uint32 len = sizeof(RaceClassAllocation) * allocs;
	len += sizeof(RaceClassCombos) * combos;
	len += sizeof(uint8);
	len += sizeof(uint32);
	len += sizeof(uint32);

	auto outapp = new EQApplicationPacket(OP_CharacterCreateRequest, len);
	unsigned char *ptr = outapp->pBuffer;
	*((uint8*)ptr) = 0;
	ptr += sizeof(uint8);

	*((uint32*)ptr) = allocs;
	ptr += sizeof(uint32);

	for(int i = 0; i < allocs; ++i) {
		RaceClassAllocation *alc = (RaceClassAllocation*)ptr;

		alc->Index = character_create_allocations[i].Index;
		for(int j = 0; j < 7; ++j) {
			alc->BaseStats[j] = character_create_allocations[i].BaseStats[j];
			alc->DefaultPointAllocation[j] = character_create_allocations[i].DefaultPointAllocation[j];
		}
		ptr += sizeof(RaceClassAllocation);
	}

	*((uint32*)ptr) = combos;
	ptr += sizeof(uint32);
	for(int i = 0; i < combos; ++i) {
		RaceClassCombos *cmb = (RaceClassCombos*)ptr;
		cmb->ExpansionRequired = character_create_race_class_combos[i].ExpansionRequired;
		cmb->Race = character_create_race_class_combos[i].Race;
		cmb->Class = character_create_race_class_combos[i].Class;
		cmb->Deity = character_create_race_class_combos[i].Deity;
		cmb->AllocationIndex = character_create_race_class_combos[i].AllocationIndex;
		cmb->Zone = character_create_race_class_combos[i].Zone;
		ptr += sizeof(RaceClassCombos);
	}

	QueuePacket(outapp);
	safe_delete(outapp);
	return true;
}

bool Client::HandleCharacterCreatePacket(const EQApplicationPacket *app) {
	if (GetAccountID() == 0) {
		Log(Logs::Detail, Logs::World_Server,"Account ID not set; unable to create character.");
		return false;
	}
	else if (app->size != sizeof(CharCreate_Struct)) {
		Log(Logs::Detail, Logs::World_Server,"Wrong size on OP_CharacterCreate. Got: %d, Expected: %d",app->size,sizeof(CharCreate_Struct));
		DumpPacket(app);
		// the previous behavior was essentially returning true here
		// but that seems a bit odd to me.
		return true;
	}

	CharCreate_Struct *cc = (CharCreate_Struct*)app->pBuffer;
	if(OPCharCreate(char_name, cc) == false) {
		database.DeleteCharacter(char_name);
		auto outapp = new EQApplicationPacket(OP_ApproveName, 1);
		outapp->pBuffer[0] = 0;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
	else
	{
		if (m_ClientVersionBit & EQEmu::versions::bit_TitaniumAndEarlier)
			StartInTutorial = true;
		SendCharInfo();
	}

	return true;
}

bool Client::HandleEnterWorldPacket(const EQApplicationPacket *app) { 
	if (GetAccountID() == 0) {
		Log(Logs::Detail, Logs::World_Server,"Enter world with no logged in account");
		eqs->Close();
		return true;
	}

	if(GetAdmin() < 0)
	{
		Log(Logs::Detail, Logs::World_Server,"Account banned or suspended.");
		eqs->Close();
		return true;
	}

	if (RuleB(World, EnableIPExemptions) || RuleI(World, MaxClientsPerIP) >= 0) {
		client_list.GetCLEIP(this->GetIP()); //Check current CLE Entry IPs against incoming connection
	}

	EnterWorld_Struct *ew=(EnterWorld_Struct *)app->pBuffer;
	strn0cpy(char_name, ew->name, 64);

	EQApplicationPacket *outapp;
	uint32 tmpaccid = 0;
	charid = database.GetCharacterInfo(char_name, &tmpaccid, &zone_id, &instance_id);
	if (charid == 0 || tmpaccid != GetAccountID()) {
		Log(Logs::Detail, Logs::World_Server,"Could not get CharInfo for '%s'",char_name);
		eqs->Close();
		return true;
	}

	// Make sure this account owns this character
	if (tmpaccid != GetAccountID()) {
		Log(Logs::Detail, Logs::World_Server,"This account does not own the character named '%s'",char_name);
		eqs->Close();
		return true;
	}

	// This can probably be moved outside and have another method return requested info (don't forget to remove the #include "../common/shareddb.h" above)
	// (This is a literal translation of the original process..I don't see why it can't be changed to a single-target query over account iteration)
	if (!is_player_zoning) {
		size_t character_limit = EQEmu::constants::Lookup(eqs->ClientVersion())->CharacterCreationLimit;
		if (character_limit > EQEmu::constants::CharacterCreationMax) { character_limit = EQEmu::constants::CharacterCreationMax; }
		if (eqs->ClientVersion() == EQEmu::versions::ClientVersion::Titanium) { character_limit = 8; }

		std::string tgh_query = StringFormat(
			"SELECT                     "
			"`id`,                      "
			"name,                      "
			"`level`,                   "
			"last_login                 "
			"FROM                       "
			"character_data             "
			"WHERE `account_id` = %i ORDER BY `name` LIMIT %u", GetAccountID(), character_limit);
		auto tgh_results = database.QueryDatabase(tgh_query);

		/* Check GoHome */
		if (ew->return_home && !ew->tutorial) {
			bool home_enabled = false;
			for (auto row = tgh_results.begin(); row != tgh_results.end(); ++row) {
				if (strcasecmp(row[1], char_name) == 0) {
					if (RuleB(World, EnableReturnHomeButton)) {
						int now = time(nullptr);
						if ((now - atoi(row[3])) >= RuleI(World, MinOfflineTimeToReturnHome)) {
							home_enabled = true;
							break;
						}
					}
				}
			}

			if (home_enabled) {
				zone_id = database.MoveCharacterToBind(charid, 4);
			}
			else {
				Log(Logs::Detail, Logs::World_Server, "'%s' is trying to go home before they're able...", char_name);
				database.SetHackerFlag(GetAccountName(), char_name, "MQGoHome: player tried to go home before they were able.");
				eqs->Close();
				return true;
			}
		}

		/* Check Tutorial*/
		if (RuleB(World, EnableTutorialButton) && (ew->tutorial || StartInTutorial)) {
			bool tutorial_enabled = false;
			for (auto row = tgh_results.begin(); row != tgh_results.end(); ++row) {
				if (strcasecmp(row[1], char_name) == 0) {
					if (RuleB(World, EnableTutorialButton) && ((uint8)atoi(row[2]) <= RuleI(World, MaxLevelForTutorial))) {
						tutorial_enabled = true;
						break;
					}
				}
			}

			if (tutorial_enabled) {
				zone_id = RuleI(World, TutorialZoneID);
				database.MoveCharacterToZone(charid, database.GetZoneName(zone_id));
			}
			else {
				Log(Logs::Detail, Logs::World_Server, "'%s' is trying to go to tutorial but are not allowed...", char_name);
				database.SetHackerFlag(GetAccountName(), char_name, "MQTutorial: player tried to enter the tutorial without having tutorial enabled for this character.");
				eqs->Close();
				return true;
			}
		}
	}

	if (zone_id == 0 || !database.GetZoneName(zone_id)) {
		// This is to save people in an invalid zone, once it's removed from the DB
		database.MoveCharacterToZone(charid, "arena");
		Log(Logs::Detail, Logs::World_Server, "Zone not found in database zone_id=%i, moveing char to arena character:%s", zone_id, char_name);
	}

	if(instance_id > 0)
	{
		if(!database.VerifyInstanceAlive(instance_id, GetCharID()))
		{
			zone_id = database.MoveCharacterToBind(charid);
			instance_id = 0;
		}
		else
		{
			if(!database.VerifyZoneInstance(zone_id, instance_id))
			{
				zone_id = database.MoveCharacterToBind(charid);
				instance_id = 0;
			}
		}
	}

	if(!is_player_zoning) {
		database.SetGroupID(char_name, 0, charid);
		database.SetLoginFlags(charid, false, false, 1);
	}
	else{
		uint32 groupid = database.GetGroupID(char_name);
		if(groupid > 0){
			char* leader = 0;
			char leaderbuf[64] = {0};
			if((leader = database.GetGroupLeaderForLogin(char_name, leaderbuf)) && strlen(leader)>1){
				auto outapp3 = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				GroupJoin_Struct* gj=(GroupJoin_Struct*)outapp3->pBuffer;
				gj->action=8;
				strcpy(gj->yourname, char_name);
				strcpy(gj->membername, leader);
				QueuePacket(outapp3);
				safe_delete(outapp3);
			}
		}
	}

	outapp = new EQApplicationPacket(OP_MOTD);
	std::string tmp;
	if (database.GetVariable("MOTD", tmp)) {
		outapp->size = tmp.length() + 1;
		outapp->pBuffer = new uchar[outapp->size];
		memset(outapp->pBuffer,0,outapp->size);
		strcpy((char*)outapp->pBuffer, tmp.c_str());

	} else {
		// Null Message of the Day. :)
		outapp->size = 1;
		outapp->pBuffer = new uchar[outapp->size];
		outapp->pBuffer[0] = 0;
	}
	QueuePacket(outapp);
	safe_delete(outapp);
	
	// set mailkey - used for duration of character session
	int MailKey = emu_random.Int(1, INT_MAX);

	database.SetMailKey(charid, GetIP(), MailKey);
	if (UCSServerAvailable_) {
		const WorldConfig *Config = WorldConfig::get();
		std::string buffer;

		EQEmu::versions::UCSVersion ConnectionType = EQEmu::versions::ucsUnknown;

		// chat server packet
		switch (GetClientVersion()) {
		case EQEmu::versions::ClientVersion::Titanium:
			ConnectionType = EQEmu::versions::ucsTitaniumChat;
			break;
		case EQEmu::versions::ClientVersion::SoF:
			ConnectionType = EQEmu::versions::ucsSoFCombined;
			break;
		case EQEmu::versions::ClientVersion::SoD:
			ConnectionType = EQEmu::versions::ucsSoDCombined;
			break;
		case EQEmu::versions::ClientVersion::UF:
			ConnectionType = EQEmu::versions::ucsUFCombined;
			break;
		case EQEmu::versions::ClientVersion::RoF:
			ConnectionType = EQEmu::versions::ucsRoFCombined;
			break;
		case EQEmu::versions::ClientVersion::RoF2:
			ConnectionType = EQEmu::versions::ucsRoF2Combined;
			break;
		default:
			ConnectionType = EQEmu::versions::ucsUnknown;
			break;
		}

		buffer = StringFormat("%s,%i,%s.%s,%c%08X",
			Config->ChatHost.c_str(),
			Config->ChatPort,
			Config->ShortName.c_str(),
			GetCharName(),
			ConnectionType,
			MailKey
		);

		outapp = new EQApplicationPacket(OP_SetChatServer, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		QueuePacket(outapp);
		safe_delete(outapp);

		// mail server packet
		switch (GetClientVersion()) {
		case EQEmu::versions::ClientVersion::Titanium:
			ConnectionType = EQEmu::versions::ucsTitaniumMail;
			break;
		default:
			// retain value from previous switch
			break;
		}

		buffer = StringFormat("%s,%i,%s.%s,%c%08X",
			Config->MailHost.c_str(),
			Config->MailPort,
			Config->ShortName.c_str(),
			GetCharName(),
			ConnectionType,
			MailKey
		);

		outapp = new EQApplicationPacket(OP_SetChatServer2, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		QueuePacket(outapp);
		safe_delete(outapp);
	}

	EnterWorld();

	return true;
}

bool Client::HandleDeleteCharacterPacket(const EQApplicationPacket *app) {

	uint32 char_acct_id = database.GetAccountIDByChar((char*)app->pBuffer);
	if(char_acct_id == GetAccountID()) {
		Log(Logs::Detail, Logs::World_Server,"Delete character: %s",app->pBuffer);
		database.DeleteCharacter((char *)app->pBuffer);
		SendCharInfo();
	}

	return true;
}

bool Client::HandleZoneChangePacket(const EQApplicationPacket *app) {
	// HoT sends this to world while zoning and wants it echoed back.
	if (m_ClientVersionBit & EQEmu::versions::bit_RoFAndLater)
	{
		QueuePacket(app);
	}
	return true;
}

bool Client::HandlePacket(const EQApplicationPacket *app) {

	EmuOpcode opcode = app->GetOpcode();

	Log(Logs::Detail, Logs::World_Server,"Recevied EQApplicationPacket");

	if (!eqs->CheckState(ESTABLISHED)) {
		Log(Logs::Detail, Logs::World_Server,"Client disconnected (net inactive on send)");
		return false;
	}

	// Voidd: Anti-GM Account hack, Checks source ip against valid GM Account IP Addresses
	if (RuleB(World, GMAccountIPList) && this->GetAdmin() >= (RuleI(World, MinGMAntiHackStatus))) {
		if(!database.CheckGMIPs(long2ip(this->GetIP()).c_str(), this->GetAccountID())) {
			Log(Logs::Detail, Logs::World_Server,"GM Account not permited from source address %s and accountid %i", long2ip(this->GetIP()).c_str(), this->GetAccountID());
			eqs->Close();
		}
	}

	if (GetAccountID() == 0 && opcode != OP_SendLoginInfo) {
		// Got a packet other than OP_SendLoginInfo when not logged in
		Log(Logs::Detail, Logs::World_Server,"Expecting OP_SendLoginInfo, got %s", OpcodeNames[opcode]);
		return false;
	}
	else if (opcode == OP_AckPacket) {
		return true;
	}

	switch(opcode)
	{
		case OP_World_Client_CRC1:
		case OP_World_Client_CRC2:
		{
			// There is no obvious entry in the CC struct to indicate that the 'Start Tutorial button
			// is selected when a character is created. I have observed that in this case, OP_EnterWorld is sent
			// before OP_World_Client_CRC1. Therefore, if we receive OP_World_Client_CRC1 before OP_EnterWorld,
			// then 'Start Tutorial' was not chosen.
			StartInTutorial = false;
			return true;
		}
		case OP_SendLoginInfo:
		{
			return HandleSendLoginInfoPacket(app);
		}
		case OP_ApproveName: //Name approval
		{
			return HandleNameApprovalPacket(app);
		}
		case OP_RandomNameGenerator:
		{
			return HandleGenerateRandomNamePacket(app);
		}
		case OP_CharacterCreateRequest:
		{
			// New OpCode in SoF
			return HandleCharacterCreateRequestPacket(app);
		}
		case OP_CharacterCreate: //Char create
		{
			return HandleCharacterCreatePacket(app);
		}
		case OP_EnterWorld: // Enter world
		{
			return HandleEnterWorldPacket(app);
		}
		case OP_DeleteCharacter:
		{
			return HandleDeleteCharacterPacket(app);
		}
		case OP_WorldComplete:
		{
			eqs->Close();
			return true;
		}
		case OP_WorldLogout:
		{
			// I don't see this getting executed on logout
			eqs->Close();
			cle->SetOnline(CLE_Status_Offline); //allows this player to log in again without an ip restriction.
			return false;
		}
		case OP_ZoneChange:
		{
			// HoT sends this to world while zoning and wants it echoed back.
			return HandleZoneChangePacket(app);
		}
		case OP_LoginUnknown1:
		case OP_LoginUnknown2:
		case OP_CrashDump:
		case OP_WearChange:
		case OP_LoginComplete:
		case OP_ApproveWorld:
		case OP_WorldClientReady:
		{
			// Essentially we are just 'eating' these packets, indicating
			// they are handled.
			return true;
		}
		default:
		{
			Log(Logs::Detail, Logs::World_Server,"Received unknown EQApplicationPacket");
			return true;
		}
	}
	return true;
}

bool Client::Process() {
	bool ret = true;
	//bool sendguilds = true;
	sockaddr_in to = {};

	memset((char *) &to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_port = port;
	to.sin_addr.s_addr = ip;

	if (autobootup_timeout.Check()) {
		Log(Logs::General, Logs::World_Server, "Zone bootup timer expired, bootup failed or too slow.");
		TellClientZoneUnavailable();
	}
	if(connect.Check()){
		SendGuildList();// Send OPCode: OP_GuildsList
		SendApproveWorld();
		connect.Disable();
	}
	if (CLE_keepalive_timer.Check()) {
		if (cle)
			cle->KeepAlive();
	}

	/************ Get all packets from packet manager out queue and process them ************/
	EQApplicationPacket *app = 0;
	while(ret && (app = (EQApplicationPacket *)eqs->PopPacket())) {
		ret = HandlePacket(app);

		delete app;
	}

	if (!eqs->CheckState(ESTABLISHED)) {
		if(WorldConfig::get()->UpdateStats){
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_LSPlayerLeftWorld;
			pack->size = sizeof(ServerLSPlayerLeftWorld_Struct);
			pack->pBuffer = new uchar[pack->size];
			memset(pack->pBuffer,0,pack->size);
			ServerLSPlayerLeftWorld_Struct* logout =(ServerLSPlayerLeftWorld_Struct*)pack->pBuffer;
			strcpy(logout->key,GetLSKey());
			logout->lsaccount_id = GetLSID();
			loginserverlist.SendPacket(pack);
			safe_delete(pack);
		}
		Log(Logs::Detail, Logs::World_Server,"Client disconnected (not active in process)");
		return false;
	}

	return ret;
}

void Client::EnterWorld(bool TryBootup) {
	if (zone_id == 0)
		return;

	ZoneServer* zone_server = nullptr;
	if(instance_id > 0)
	{
		if(database.VerifyInstanceAlive(instance_id, GetCharID()))
		{
			if(database.VerifyZoneInstance(zone_id, instance_id))
			{
				zone_server = zoneserver_list.FindByInstanceID(instance_id);
			}
			else
			{
				instance_id = 0;
				zone_server = nullptr;
				database.MoveCharacterToBind(GetCharID());
				TellClientZoneUnavailable();
				return;
			}
		}
		else
		{
			instance_id = 0;
			zone_server = nullptr;
			database.MoveCharacterToBind(GetCharID());
			TellClientZoneUnavailable();
			return;
		}
	}
	else
		zone_server = zoneserver_list.FindByZoneID(zone_id);


	const char *zone_name = database.GetZoneName(zone_id, true);
	if (zone_server) {
		// warn the world we're comming, so it knows not to shutdown
		zone_server->IncomingClient(this);
	}
	else {
		if (TryBootup) {
			Log(Logs::General, Logs::World_Server, "Attempting autobootup of %s (%d:%d)", zone_name, zone_id, instance_id);
			autobootup_timeout.Start();
			zone_waiting_for_bootup = zoneserver_list.TriggerBootup(zone_id, instance_id);
			if (zone_waiting_for_bootup == 0) {
				Log(Logs::General, Logs::World_Server, "No zoneserver available to boot up.");
				TellClientZoneUnavailable();
			}
			return;
		}
		else {
			Log(Logs::General, Logs::World_Server, "Requested zone %s is not running.", zone_name);
			TellClientZoneUnavailable();
			return;
		}
	}
	zone_waiting_for_bootup = 0;

	if(!cle) {
		return;
	}

	cle->SetChar(charid, char_name);
	database.UpdateLiveChar(char_name, GetAccountID());

	Log(Logs::General, Logs::World_Server, 
		"(%s) %s %s (Zone ID %d: Instance ID: %d) ", 
		char_name,
		(seen_character_select ? "Zoning from character select" : "Zoning to"), 
		zone_name, 
		zone_id, 
		instance_id
	);

	if (seen_character_select) {
		if (GetAdmin() < 80 && zoneserver_list.IsZoneLocked(zone_id)) {
			Log(Logs::General, Logs::World_Server, "Enter world failed. Zone is locked.");
			TellClientZoneUnavailable();
			return;
		}

		auto pack = new ServerPacket;
		pack->opcode = ServerOP_AcceptWorldEntrance;
		pack->size = sizeof(WorldToZone_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, pack->size);
		WorldToZone_Struct* wtz = (WorldToZone_Struct*) pack->pBuffer;
		wtz->account_id = GetAccountID();
		wtz->response = 0;
		zone_server->SendPacket(pack);
		delete pack;
	}
	else {	// if they havent seen character select screen, we can assume this is a zone
			// to zone movement, which should be preauthorized before they leave the previous zone
		Clearance(1);
	}
}

void Client::Clearance(int8 response)
{
	ZoneServer* zs = nullptr;
	if(instance_id > 0)
	{
		zs = zoneserver_list.FindByInstanceID(instance_id);
	}
	else
	{
		zs = zoneserver_list.FindByZoneID(zone_id);
	}

	if(zs == 0 || response == -1 || response == 0)
	{
		if (zs == 0)
		{
			Log(Logs::Detail, Logs::World_Server,"Unable to find zoneserver in Client::Clearance!!");
		} else {
			Log(Logs::Detail, Logs::World_Server, "Invalid response %d in Client::Clearance", response);
		}

		TellClientZoneUnavailable();
		return;
	}

	EQApplicationPacket* outapp;

	if (zs->GetCAddress() == nullptr) {
		Log(Logs::Detail, Logs::World_Server, "Unable to do zs->GetCAddress() in Client::Clearance!!");
		TellClientZoneUnavailable();
		return;
	}

	if (zone_id == 0) {
		Log(Logs::Detail, Logs::World_Server, "zoneID is nullptr in Client::Clearance!!");
		TellClientZoneUnavailable();
		return;
	}

	const char* zonename = database.GetZoneName(zone_id);
	if (zonename == 0) {
		Log(Logs::Detail, Logs::World_Server, "zonename is nullptr in Client::Clearance!!");
		TellClientZoneUnavailable();
		return;
	}

	// Send zone server IP data
	outapp = new EQApplicationPacket(OP_ZoneServerInfo, sizeof(ZoneServerInfo_Struct));
	ZoneServerInfo_Struct* zsi = (ZoneServerInfo_Struct*)outapp->pBuffer;

	const char *zs_addr = nullptr;
	if(cle && cle->IsLocalClient()) {
		const char *local_addr = zs->GetCLocalAddress();

		if(local_addr[0]) {
			zs_addr = local_addr;
		} else {
			zs_addr = zs->GetIP().c_str();

			if (!zs_addr[0]) {
				zs_addr = WorldConfig::get()->LocalAddress.c_str();
			}

			if(strcmp(zs_addr, "127.0.0.1") == 0)
			{
				Log(Logs::Detail, Logs::World_Server, "Local zone address was %s, setting local address to: %s", zs_addr, WorldConfig::get()->LocalAddress.c_str());
				zs_addr = WorldConfig::get()->LocalAddress.c_str();
			} else {
				Log(Logs::Detail, Logs::World_Server, "Local zone address %s", zs_addr);
			}
		}

	} else {
		const char *addr = zs->GetCAddress();
		if(addr[0]) {
			zs_addr = addr;
		} else {
			zs_addr = WorldConfig::get()->WorldAddress.c_str();
		}
	}

	strcpy(zsi->ip, zs_addr);
	zsi->port =zs->GetCPort();
	Log(Logs::Detail, Logs::World_Server,"Sending client to zone %s (%d:%d) at %s:%d",zonename,zone_id,instance_id,zsi->ip,zsi->port);
	QueuePacket(outapp);
	safe_delete(outapp);

	if (cle)
		cle->SetOnline(CLE_Status_Zoning);
}

void Client::TellClientZoneUnavailable() {
	auto outapp = new EQApplicationPacket(OP_ZoneUnavail, sizeof(ZoneUnavail_Struct));
	ZoneUnavail_Struct* ua = (ZoneUnavail_Struct*)outapp->pBuffer;
	const char* zonename = database.GetZoneName(zone_id);
	if (zonename)
		strcpy(ua->zonename, zonename);
	QueuePacket(outapp);
	delete outapp;

	zone_id = 0;
	zone_waiting_for_bootup = 0;
	autobootup_timeout.Disable();
}

bool Client::GenPassKey(char* key) {
	char* passKey=nullptr;
	*passKey += ((char)('A'+((int)emu_random.Int(0, 25))));
	*passKey += ((char)('A'+((int)emu_random.Int(0, 25))));
	memcpy(key, passKey, strlen(passKey));
	return true;
}

void Client::QueuePacket(const EQApplicationPacket* app, bool ack_req) {
	Log(Logs::Detail, Logs::World_Server, "Sending EQApplicationPacket OpCode 0x%04x",app->GetOpcode());

	ack_req = true;	// It's broke right now, dont delete this line till fix it. =P
	eqs->QueuePacket(app, ack_req);
}

void Client::SendGuildList() {
	EQApplicationPacket *outapp;
	outapp = new EQApplicationPacket(OP_GuildsList);

	//ask the guild manager to build us a nice guild list packet
	outapp->pBuffer = guild_mgr.MakeGuildList("", outapp->size);
	if(outapp->pBuffer == nullptr) {
		return;
	}


	eqs->FastQueuePacket((EQApplicationPacket **)&outapp);
}

// @merth: I have no idea what this struct is for, so it's hardcoded for now
void Client::SendApproveWorld()
{
	EQApplicationPacket* outapp;

	// Send OPCode: OP_ApproveWorld, size: 544
	outapp = new EQApplicationPacket(OP_ApproveWorld, sizeof(ApproveWorld_Struct));
	ApproveWorld_Struct* aw = (ApproveWorld_Struct*)outapp->pBuffer;
	uchar foo[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x87,0x13,0xbe,0xc8,0xa7,0x77,0xcb,
		0x27,0xed,0xe1,0xe6,0x5d,0x1c,0xaa,0xd3,0x3c,0x26,0x3b,0x6d,0x8c,0xdb,0x36,0x8d,
		0x91,0x72,0xf5,0xbb,0xe0,0x5c,0x50,0x6f,0x09,0x6d,0xc9,0x1e,0xe7,0x2e,0xf4,0x38,
		0x1b,0x5e,0xa8,0xc2,0xfe,0xb4,0x18,0x4a,0xf7,0x72,0x85,0x13,0xf5,0x63,0x6c,0x16,
		0x69,0xf4,0xe0,0x17,0xff,0x87,0x11,0xf3,0x2b,0xb7,0x73,0x04,0x37,0xca,0xd5,0x77,
		0xf8,0x03,0x20,0x0a,0x56,0x8b,0xfb,0x35,0xff,0x59,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
	};
	memcpy(aw->unknown544, foo, sizeof(foo));
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::OPCharCreate(char *name, CharCreate_Struct *cc)
{
	PlayerProfile_Struct pp;
	ExtendedProfile_Struct ext;
	EQEmu::InventoryProfile inv;
	time_t bday = time(nullptr);
	char startzone[50]={0};
	uint32 i;
	struct in_addr in;

	int stats_sum = cc->STR + cc->STA + cc->AGI + cc->DEX + cc->WIS + cc->INT + cc->CHA;

	in.s_addr = GetIP();

	Log(Logs::Detail, Logs::World_Server, "Character creation request from %s LS#%d (%s:%d) : ", GetCLE()->LSName(), GetCLE()->LSID(), inet_ntoa(in), GetPort());
	Log(Logs::Detail, Logs::World_Server, "Name: %s", name);
	Log(Logs::Detail, Logs::World_Server, "Race: %d  Class: %d  Gender: %d  Deity: %d  Start zone: %d  Tutorial: %s",
		cc->race, cc->class_, cc->gender, cc->deity, cc->start_zone, cc->tutorial ? "true" : "false");
	Log(Logs::Detail, Logs::World_Server, "STR  STA  AGI  DEX  WIS  INT  CHA    Total");
	Log(Logs::Detail, Logs::World_Server, "%3d  %3d  %3d  %3d  %3d  %3d  %3d     %3d",
		cc->STR, cc->STA, cc->AGI, cc->DEX, cc->WIS, cc->INT, cc->CHA,
		stats_sum);
	Log(Logs::Detail, Logs::World_Server, "Face: %d  Eye colors: %d %d", cc->face, cc->eyecolor1, cc->eyecolor2);
	Log(Logs::Detail, Logs::World_Server, "Hairstyle: %d  Haircolor: %d", cc->hairstyle, cc->haircolor);
	Log(Logs::Detail, Logs::World_Server, "Beard: %d  Beardcolor: %d", cc->beard, cc->beardcolor);

	/* Validate the char creation struct */
	if (m_ClientVersionBit & EQEmu::versions::bit_SoFAndLater) {
		if (!CheckCharCreateInfoSoF(cc)) {
			Log(Logs::Detail, Logs::World_Server,"CheckCharCreateInfo did not validate the request (bad race/class/stats)");
			return false;
		}
	} else {
		if (!CheckCharCreateInfoTitanium(cc)) {
			Log(Logs::Detail, Logs::World_Server,"CheckCharCreateInfo did not validate the request (bad race/class/stats)");
			return false;
		}
	}

	/* Convert incoming cc_s to the new PlayerProfile_Struct */
	memset(&pp, 0, sizeof(PlayerProfile_Struct));	// start building the profile

	strn0cpy(pp.name, name, 63);

	pp.race				= cc->race;
	pp.class_			= cc->class_;
	pp.gender			= cc->gender;
	pp.deity			= cc->deity;
	pp.STR				= cc->STR;
	pp.STA				= cc->STA;
	pp.AGI				= cc->AGI;
	pp.DEX				= cc->DEX;
	pp.WIS				= cc->WIS;
	pp.INT				= cc->INT;
	pp.CHA				= cc->CHA;
	pp.face				= cc->face;
	pp.eyecolor1		= cc->eyecolor1;
	pp.eyecolor2		= cc->eyecolor2;
	pp.hairstyle		= cc->hairstyle;
	pp.haircolor		= cc->haircolor;
	pp.beard			= cc->beard;
	pp.beardcolor		= cc->beardcolor;
	pp.drakkin_heritage		= cc->drakkin_heritage;
	pp.drakkin_tattoo		= cc->drakkin_tattoo;
	pp.drakkin_details		= cc->drakkin_details;
	pp.birthday		= bday;
	pp.lastlogin	= bday;
	pp.level			= 1;
	pp.points			= 5;
	pp.cur_hp			= 1000; // 1k hp during dev only
	pp.hunger_level = 6000;
	pp.thirst_level = 6000;

	/* Set Racial and Class specific language and skills */
	SetRacialLanguages(&pp);
	SetRaceStartingSkills(&pp);
	SetClassStartingSkills(&pp);
	SetClassLanguages(&pp);

	pp.skills[EQEmu::skills::SkillSwimming] = RuleI(Skills, SwimmingStartValue);
	pp.skills[EQEmu::skills::SkillSenseHeading] = RuleI(Skills, SenseHeadingStartValue);

//	strcpy(pp.servername, WorldConfig::get()->ShortName.c_str());


	for (i = 0; i < MAX_PP_REF_SPELLBOOK; i++)
		pp.spell_book[i] = 0xFFFFFFFF;

	for(i = 0; i < MAX_PP_MEMSPELL; i++)
		pp.mem_spells[i] = 0xFFFFFFFF;

	for(i = 0; i < BUFF_COUNT; i++)
		pp.buffs[i].spellid = 0xFFFF;

	/* If server is PVP by default, make all character set to it. */
	pp.pvp = database.GetServerType() == 1 ? 1 : 0;

	/* If it is an SoF Client and the SoF Start Zone rule is set, send new chars there */
	if (m_ClientVersionBit & EQEmu::versions::bit_SoFAndLater) {
		Log(Logs::Detail, Logs::World_Server,"Found 'SoFStartZoneID' rule setting: %i", RuleI(World, SoFStartZoneID));
		if (RuleI(World, SoFStartZoneID) > 0) {
			pp.zone_id = RuleI(World, SoFStartZoneID);
			cc->start_zone = pp.zone_id;
		}
	}
	else {
		Log(Logs::General, Logs::World_Server, "Found 'TitaniumStartZoneID' rule setting: %i", RuleI(World, TitaniumStartZoneID));
		if (RuleI(World, TitaniumStartZoneID) > 0) { 	/* if there's a startzone variable put them in there */
		
			pp.zone_id = RuleI(World, TitaniumStartZoneID);
			cc->start_zone = pp.zone_id;
		}
	} 	
	/* use normal starting zone logic to either get defaults, or if startzone was set, load that from the db table.*/
	bool ValidStartZone = database.GetStartZone(&pp, cc, m_ClientVersionBit & EQEmu::versions::bit_TitaniumAndEarlier);

	if (!ValidStartZone){
		return false;
	}

	/* just in case  */
	if (!pp.zone_id) {
		pp.zone_id = 1;		// qeynos
		pp.x = pp.y = pp.z = -1;
	}

	/* Set Home Binds  -- yep, all of them */
	pp.binds[1].zoneId = pp.zone_id;
	pp.binds[1].x = pp.x;
	pp.binds[1].y = pp.y;
	pp.binds[1].z = pp.z;
	pp.binds[1].heading = pp.heading;

	pp.binds[2].zoneId = pp.zone_id;
	pp.binds[2].x = pp.x;
	pp.binds[2].y = pp.y;
	pp.binds[2].z = pp.z;
	pp.binds[2].heading = pp.heading;

	pp.binds[3].zoneId = pp.zone_id;
	pp.binds[3].x = pp.x;
	pp.binds[3].y = pp.y;
	pp.binds[3].z = pp.z;
	pp.binds[3].heading = pp.heading;

	pp.binds[4].zoneId = pp.zone_id;
	pp.binds[4].x = pp.x;
	pp.binds[4].y = pp.y;
	pp.binds[4].z = pp.z;
	pp.binds[4].heading = pp.heading;

	/* Overrides if we have the tutorial flag set! */
	if (cc->tutorial && RuleB(World, EnableTutorialButton)) {
		pp.zone_id = RuleI(World, TutorialZoneID);
		database.GetSafePoints(pp.zone_id, 0, &pp.x, &pp.y, &pp.z);
	}

	/*  Will either be the same as home or tutorial if enabled. */
	if(RuleB(World, StartZoneSameAsBindOnCreation))	{
		pp.binds[0].zoneId = pp.zone_id;
		pp.binds[0].x = pp.x;
		pp.binds[0].y = pp.y;
		pp.binds[0].z = pp.z;
		pp.binds[0].heading = pp.heading;
	}

	Log(Logs::Detail, Logs::World_Server,"Current location: %s (%d)  %0.2f, %0.2f, %0.2f, %0.2f",
		database.GetZoneName(pp.zone_id), pp.zone_id, pp.x, pp.y, pp.z, pp.heading);
	Log(Logs::Detail, Logs::World_Server,"Bind location: %s (%d) %0.2f, %0.2f, %0.2f",
		database.GetZoneName(pp.binds[0].zoneId), pp.binds[0].zoneId,  pp.binds[0].x, pp.binds[0].y, pp.binds[0].z);
	Log(Logs::Detail, Logs::World_Server,"Home location: %s (%d) %0.2f, %0.2f, %0.2f",
		database.GetZoneName(pp.binds[4].zoneId), pp.binds[4].zoneId,  pp.binds[4].x, pp.binds[4].y, pp.binds[4].z);

	/* Starting Items inventory */
	database.SetStartingItems(&pp, &inv, pp.race, pp.class_, pp.deity, pp.zone_id, pp.name, GetAdmin());

	// now we give the pp and the inv we made to StoreCharacter
	// to see if we can store it
	if (!database.StoreCharacter(GetAccountID(), &pp, &inv)) {
		Log(Logs::Detail, Logs::World_Server,"Character creation failed: %s", pp.name);
		return false;
	}
	Log(Logs::Detail, Logs::World_Server,"Character creation successful: %s", pp.name);
	return true;
}

// returns true if the request is ok, false if there's an error
bool CheckCharCreateInfoSoF(CharCreate_Struct *cc)
{
	if (!cc)
		return false;

	Log(Logs::Detail, Logs::World_Server, "Validating char creation info...");

	RaceClassCombos class_combo;
	bool found = false;
	int combos = character_create_race_class_combos.size();
	for (int i = 0; i < combos; ++i) {
		if (character_create_race_class_combos[i].Class == cc->class_ &&
				character_create_race_class_combos[i].Race == cc->race &&
				character_create_race_class_combos[i].Deity == cc->deity &&
				character_create_race_class_combos[i].Zone == cc->start_zone) {
			class_combo = character_create_race_class_combos[i];
			found = true;
			break;
		}
	}

	if (!found) {
		Log(Logs::Detail, Logs::World_Server, "Could not find class/race/deity/start_zone combination");
		return false;
	}

	uint32 max_stats = 0;
	uint32 allocs = character_create_allocations.size();
	RaceClassAllocation allocation = {0};
	found = false;
	for (int i = 0; i < allocs; ++i) {
		if (character_create_allocations[i].Index == class_combo.AllocationIndex) {
			allocation = character_create_allocations[i];
			found = true;
			break;
		}
	}

	if (!found) {
		Log(Logs::Detail, Logs::World_Server, "Could not find starting stats for selected character combo, cannot verify stats");
		return false;
	}

	max_stats = allocation.DefaultPointAllocation[0] +
		allocation.DefaultPointAllocation[1] +
		allocation.DefaultPointAllocation[2] +
		allocation.DefaultPointAllocation[3] +
		allocation.DefaultPointAllocation[4] +
		allocation.DefaultPointAllocation[5] +
		allocation.DefaultPointAllocation[6];

	if (cc->STR > allocation.BaseStats[0] + max_stats || cc->STR < allocation.BaseStats[0]) {
		Log(Logs::Detail, Logs::World_Server, "Strength out of range");
		return false;
	}

	if (cc->DEX > allocation.BaseStats[1] + max_stats || cc->DEX < allocation.BaseStats[1]) {
		Log(Logs::Detail, Logs::World_Server, "Dexterity out of range");
		return false;
	}

	if (cc->AGI > allocation.BaseStats[2] + max_stats || cc->AGI < allocation.BaseStats[2]) {
		Log(Logs::Detail, Logs::World_Server, "Agility out of range");
		return false;
	}

	if (cc->STA > allocation.BaseStats[3] + max_stats || cc->STA < allocation.BaseStats[3]) {
		Log(Logs::Detail, Logs::World_Server, "Stamina out of range");
		return false;
	}

	if (cc->INT > allocation.BaseStats[4] + max_stats || cc->INT < allocation.BaseStats[4]) {
		Log(Logs::Detail, Logs::World_Server, "Intelligence out of range");
		return false;
	}

	if (cc->WIS > allocation.BaseStats[5] + max_stats || cc->WIS < allocation.BaseStats[5]) {
		Log(Logs::Detail, Logs::World_Server, "Wisdom out of range");
		return false;
	}

	if (cc->CHA > allocation.BaseStats[6] + max_stats || cc->CHA < allocation.BaseStats[6]) {
		Log(Logs::Detail, Logs::World_Server, "Charisma out of range");
		return false;
	}

	uint32 current_stats = 0;
	current_stats += cc->STR - allocation.BaseStats[0];
	current_stats += cc->DEX - allocation.BaseStats[1];
	current_stats += cc->AGI - allocation.BaseStats[2];
	current_stats += cc->STA - allocation.BaseStats[3];
	current_stats += cc->INT - allocation.BaseStats[4];
	current_stats += cc->WIS - allocation.BaseStats[5];
	current_stats += cc->CHA - allocation.BaseStats[6];
	if (current_stats > max_stats) {
		Log(Logs::Detail, Logs::World_Server, "Current Stats > Maximum Stats");
		return false;
	}

	return true;
}

bool CheckCharCreateInfoTitanium(CharCreate_Struct *cc)
{
	uint32 bSTR, bSTA, bAGI, bDEX, bWIS, bINT, bCHA, bTOTAL, cTOTAL, stat_points; //these are all uint32 in CharCreate_Struct, so we'll make them uint32 here to make the compiler shut up
	int classtemp, racetemp;
	int Charerrors = 0;


// if this is increased you'll have to add a column to the classrace
// table below
#define _TABLE_RACES 16

	static const int BaseRace[_TABLE_RACES][7] =
	{            /* STR  STA  AGI  DEX  WIS  INT  CHR */
	{ /*Human*/      75,  75,  75,  75,  75,  75,  75},
	{ /*Barbarian*/ 103,  95,  82,  70,  70,  60,  55},
	{ /*Erudite*/    60,  70,  70,  70,  83, 107,  70},
	{ /*Wood Elf*/   65,  65,  95,  80,  80,  75,  75},
	{ /*High Elf*/   55,  65,  85,  70,  95,  92,  80},
	{ /*Dark Elf*/   60,  65,  90,  75,  83,  99,  60},
	{ /*Half Elf*/   70,  70,  90,  85,  60,  75,  75},
	{ /*Dwarf*/      90,  90,  70,  90,  83,  60,  45},
	{ /*Troll*/     108, 109,  83,  75,  60,  52,  40},
	{ /*Ogre*/      130, 122,  70,  70,  67,  60,  37},
	{ /*Halfling*/   70,  75,  95,  90,  80,  67,  50},
	{ /*Gnome*/      60,  70,  85,  85,  67,  98,  60},
	{ /*Iksar*/      70,  70,  90,  85,  80,  75,  55},
	{ /*Vah Shir*/   90,  75,  90,  70,  70,  65,  65},
	{ /*Froglok*/    70,  80, 100, 100,  75,  75,  50},
	{ /*Drakkin*/    70,  80,  85,  75,  80,  85,  75}
	};

	static const int BaseClass[PLAYER_CLASS_COUNT][8] =
	{              /* STR  STA  AGI  DEX  WIS  INT  CHR  ADD*/
	{ /*Warrior*/      10,  10,   5,   0,   0,   0,   0,  25},
	{ /*Cleric*/        5,   5,   0,   0,  10,   0,   0,  30},
	{ /*Paladin*/      10,   5,   0,   0,   5,   0,  10,  20},
	{ /*Ranger*/        5,  10,  10,   0,   5,   0,   0,  20},
	{ /*ShadowKnight*/ 10,   5,   0,   0,   0,   10,  5,  20},
	{ /*Druid*/         0,  10,   0,   0,  10,   0,   0,  30},
	{ /*Monk*/          5,   5,  10,  10,   0,   0,   0,  20},
	{ /*Bard*/          5,   0,   0,  10,   0,   0,  10,  25},
	{ /*Rouge*/         0,   0,  10,  10,   0,   0,   0,  30},
	{ /*Shaman*/        0,   5,   0,   0,  10,   0,   5,  30},
	{ /*Necromancer*/   0,   0,   0,  10,   0,  10,   0,  30},
	{ /*Wizard*/        0,  10,   0,   0,   0,  10,   0,  30},
	{ /*Magician*/      0,  10,   0,   0,   0,  10,   0,  30},
	{ /*Enchanter*/     0,   0,   0,   0,   0,  10,  10,  30},
	{ /*Beastlord*/     0,  10,   5,   0,  10,   0,   5,  20},
	{ /*Berserker*/    10,   5,   0,  10,   0,   0,   0,  25}
	};

	static const bool ClassRaceLookupTable[PLAYER_CLASS_COUNT][_TABLE_RACES]=
	{                   /*Human  Barbarian Erudite Woodelf Highelf Darkelf Halfelf Dwarf  Troll  Ogre   Halfling Gnome  Iksar  Vahshir Froglok Drakkin*/
	{ /*Warrior*/         true,  true,     false,  true,   false,  true,   true,   true,  true,  true,  true,    true,  true,  true,   true,   true},
	{ /*Cleric*/          true,  false,    true,   false,  true,   true,   true,   true,  false, false, true,    true,  false, false,  true,   true},
	{ /*Paladin*/         true,  false,    true,   false,  true,   false,  true,   true,  false, false, true,    true,  false, false,  true,   true},
	{ /*Ranger*/          true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false, false,  false,  true},
	{ /*ShadowKnight*/    true,  false,    true,   false,  false,  true,   false,  false, true,  true,  false,   true,  true,  false,  true,   true},
	{ /*Druid*/           true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false, false,  false,  true},
	{ /*Monk*/            true,  false,    false,  false,  false,  false,  false,  false, false, false, false,   false, true,  false,  false,  true},
	{ /*Bard*/            true,  false,    false,  true,   false,  false,  true,   false, false, false, false,   false, false, true,   false,  true},
	{ /*Rogue*/           true,  true,     false,  true,   false,  true,   true,   true,  false, false, true,    true,  false, true,   true,   true},
	{ /*Shaman*/          false, true,     false,  false,  false,  false,  false,  false, true,  true,  false,   false, true,  true,   true,   false},
	{ /*Necromancer*/     true,  false,    true,   false,  false,  true,   false,  false, false, false, false,   true,  true,  false,  true,   true},
	{ /*Wizard*/          true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  true,   true},
	{ /*Magician*/        true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  false,  true},
	{ /*Enchanter*/       true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  false,  true},
	{ /*Beastlord*/       false, true,     false,  false,  false,  false,  false,  false, true,  true,  false,   false, true,  true,   false,  false},
	{ /*Berserker*/       false, true,     false,  false,  false,  false,  false,  true,  true,  true,  false,   false, false, true,   false,  false}
	};

	if (!cc)
		return false;

	Log(Logs::Detail, Logs::World_Server,"Validating char creation info...");

	classtemp = cc->class_ - 1;
	racetemp = cc->race - 1;
	// these have non sequential race numbers so they need to be mapped
	if (cc->race == FROGLOK) racetemp = 14;
	if (cc->race == VAHSHIR) racetemp = 13;
	if (cc->race == IKSAR) racetemp = 12;
	if (cc->race == DRAKKIN) racetemp = 15;

	// if out of range looking it up in the table would crash stuff
	// so we return from these
	if (classtemp >= PLAYER_CLASS_COUNT) {
		Log(Logs::Detail, Logs::World_Server,"  class is out of range");
		return false;
	}
	if (racetemp >= _TABLE_RACES) {
		Log(Logs::Detail, Logs::World_Server,"  race is out of range");
		return false;
	}

	if (!ClassRaceLookupTable[classtemp][racetemp]) { //Lookup table better than a bunch of ifs?
		Log(Logs::Detail, Logs::World_Server,"  invalid race/class combination");
		// we return from this one, since if it's an invalid combination our table
		// doesn't have meaningful values for the stats
		return false;
	}

	// add up the base values for this class/race
	// this is what they start with, and they have stat_points more
	// that can distributed
	bSTR = BaseClass[classtemp][0] + BaseRace[racetemp][0];
	bSTA = BaseClass[classtemp][1] + BaseRace[racetemp][1];
	bAGI = BaseClass[classtemp][2] + BaseRace[racetemp][2];
	bDEX = BaseClass[classtemp][3] + BaseRace[racetemp][3];
	bWIS = BaseClass[classtemp][4] + BaseRace[racetemp][4];
	bINT = BaseClass[classtemp][5] + BaseRace[racetemp][5];
	bCHA = BaseClass[classtemp][6] + BaseRace[racetemp][6];
	stat_points = BaseClass[classtemp][7];
	bTOTAL = bSTR + bSTA + bAGI + bDEX + bWIS + bINT + bCHA;
	cTOTAL = cc->STR + cc->STA + cc->AGI + cc->DEX + cc->WIS + cc->INT + cc->CHA;

	// the first check makes sure the total is exactly what was expected.
	// this will catch all the stat cheating, but there's still the issue
	// of reducing CHA or INT or something, to use for STR, so we check
	// that none are lower than the base or higher than base + stat_points
	// NOTE: these could just be else if, but i want to see all the stats
	// that are messed up not just the first hit

	if (bTOTAL + stat_points != cTOTAL) {
		Log(Logs::Detail, Logs::World_Server,"  stat points total doesn't match expected value: expecting %d got %d", bTOTAL + stat_points, cTOTAL);
		Charerrors++;
	}

	if (cc->STR > bSTR + stat_points || cc->STR < bSTR) {
		Log(Logs::Detail, Logs::World_Server,"  stat STR is out of range");
		Charerrors++;
	}
	if (cc->STA > bSTA + stat_points || cc->STA < bSTA) {
		Log(Logs::Detail, Logs::World_Server,"  stat STA is out of range");
		Charerrors++;
	}
	if (cc->AGI > bAGI + stat_points || cc->AGI < bAGI) {
		Log(Logs::Detail, Logs::World_Server,"  stat AGI is out of range");
		Charerrors++;
	}
	if (cc->DEX > bDEX + stat_points || cc->DEX < bDEX) {
		Log(Logs::Detail, Logs::World_Server,"  stat DEX is out of range");
		Charerrors++;
	}
	if (cc->WIS > bWIS + stat_points || cc->WIS < bWIS) {
		Log(Logs::Detail, Logs::World_Server,"  stat WIS is out of range");
		Charerrors++;
	}
	if (cc->INT > bINT + stat_points || cc->INT < bINT) {
		Log(Logs::Detail, Logs::World_Server,"  stat INT is out of range");
		Charerrors++;
	}
	if (cc->CHA > bCHA + stat_points || cc->CHA < bCHA) {
		Log(Logs::Detail, Logs::World_Server,"  stat CHA is out of range");
		Charerrors++;
	}

	/*TODO: Check for deity/class/race.. it'd be nice, but probably of any real use to hack(faction, deity based items are all I can think of)
	I am NOT writing those tables - kathgar*/

	Log(Logs::Detail, Logs::World_Server,"Found %d errors in character creation request", Charerrors);

	return Charerrors == 0;
}

void Client::SetClassStartingSkills(PlayerProfile_Struct *pp)
{
	for (uint32 i = 0; i <= EQEmu::skills::HIGHEST_SKILL; ++i) {
		if (pp->skills[i] == 0) {
			// Skip specialized, tradeskills (fishing excluded), Alcohol Tolerance, and Bind Wound
			if (EQEmu::skills::IsSpecializedSkill((EQEmu::skills::SkillType)i) ||
				(EQEmu::skills::IsTradeskill((EQEmu::skills::SkillType)i) && i != EQEmu::skills::SkillFishing) ||
				i == EQEmu::skills::SkillAlcoholTolerance || i == EQEmu::skills::SkillBindWound)
				continue;

			pp->skills[i] = database.GetSkillCap(pp->class_, (EQEmu::skills::SkillType)i, 1);
		}
	}

	if (cle->GetClientVersion() < static_cast<uint8>(EQEmu::versions::ClientVersion::RoF2) && pp->class_ == BERSERKER) {
		pp->skills[EQEmu::skills::Skill1HPiercing] = pp->skills[EQEmu::skills::Skill2HPiercing];
		pp->skills[EQEmu::skills::Skill2HPiercing] = 0;
	}
}

void Client::SetRaceStartingSkills( PlayerProfile_Struct *pp )
{
	switch( pp->race )
	{
	case BARBARIAN:
	case DWARF:
	case ERUDITE:
	case HALF_ELF:
	case HIGH_ELF:
	case HUMAN:
	case OGRE:
	case TROLL:
	case DRAKKIN:	//Drakkin are supposed to get a starting AA Skill
		{
			// No Race Specific Skills
			break;
		}
	case DARK_ELF:
		{
			pp->skills[EQEmu::skills::SkillHide] = 50;
			break;
		}
	case FROGLOK:
		{
			pp->skills[EQEmu::skills::SkillSwimming] = 125;
			break;
		}
	case GNOME:
		{
			pp->skills[EQEmu::skills::SkillTinkering] = 50;
			break;
		}
	case HALFLING:
		{
			pp->skills[EQEmu::skills::SkillHide] = 50;
			pp->skills[EQEmu::skills::SkillSneak] = 50;
			break;
		}
	case IKSAR:
		{
			pp->skills[EQEmu::skills::SkillForage] = 50;
			pp->skills[EQEmu::skills::SkillSwimming] = 100;
			break;
		}
	case WOOD_ELF:
		{
			pp->skills[EQEmu::skills::SkillForage] = 50;
			pp->skills[EQEmu::skills::SkillHide] = 50;
			break;
		}
	case VAHSHIR:
		{
			pp->skills[EQEmu::skills::SkillSafeFall] = 50;
			pp->skills[EQEmu::skills::SkillSneak] = 50;
			break;
		}
	}
}

void Client::SetRacialLanguages( PlayerProfile_Struct *pp )
{
	switch( pp->race )
	{
	case BARBARIAN:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_BARBARIAN] = 100;
			break;
		}
	case DARK_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_ELVISH] = 100;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_ELDER_ELVISH] = 100;
			pp->languages[LANG_ELVISH] = 25;
			break;
		}
	case DWARF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DWARVISH] = 100;
			pp->languages[LANG_GNOMISH] = 25;
			break;
		}
	case ERUDITE:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ERUDIAN] = 100;
			break;
		}
	case FROGLOK:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_FROGLOK] = 100;
			pp->languages[LANG_TROLL] = 25;
			break;
		}
	case GNOME:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DWARVISH] = 25;
			pp->languages[LANG_GNOMISH] = 100;
			break;
		}
	case HALF_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
	case HALFLING:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_HALFLING] = 100;
			break;
		}
	case HIGH_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_ELVISH] = 25;
			pp->languages[LANG_ELDER_ELVISH] = 25;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
	case HUMAN:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			break;
		}
	case IKSAR:
		{
			pp->languages[LANG_COMMON_TONGUE] = RuleI(Character, IksarCommonTongue);
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_LIZARDMAN] = 100;
			break;
		}
	case OGRE:
		{
			pp->languages[LANG_COMMON_TONGUE] = RuleI(Character, OgreCommonTongue);
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_OGRE] = 100;
			break;
		}
	case TROLL:
		{
			pp->languages[LANG_COMMON_TONGUE] = RuleI(Character, TrollCommonTongue);
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_TROLL] = 100;
			break;
		}
	case WOOD_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
	case VAHSHIR:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_COMBINE_TONGUE] = 100;
			pp->languages[LANG_ERUDIAN] = 25;
			pp->languages[LANG_VAH_SHIR] = 100;
			break;
		}
	case DRAKKIN:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELDER_DRAGON] = 100;
			pp->languages[LANG_DRAGON] = 100;
			break;
		}
	}
}

void Client::SetClassLanguages(PlayerProfile_Struct *pp)
{
	// we only need to handle one class, but custom server might want to do more
	switch(pp->class_) {
	case ROGUE:
		pp->languages[LANG_THIEVES_CANT] = 100;
		break;
	default:
		break;
	}
}

