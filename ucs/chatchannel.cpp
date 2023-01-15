/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/eqemu_logsys.h"
#include "../common/strings.h"
#include "chatchannel.h"
#include "clientlist.h"
#include "database.h"
#include <cstdlib>
#include <algorithm>

extern UCSDatabase database;
extern uint32 ChatMessagesSent;

void ServerToClient45SayLink(std::string& clientSayLink, const std::string& serverSayLink);
void ServerToClient50SayLink(std::string& clientSayLink, const std::string& serverSayLink);
void ServerToClient55SayLink(std::string& clientSayLink, const std::string& serverSayLink);

ChatChannel::ChatChannel(std::string inName, std::string inOwner, std::string inPassword, bool inPermanent, int inMinimumStatus) :
	DeleteTimer(0) {

	Name = inName;

	Owner = inOwner;

	Password = inPassword;

	Permanent = inPermanent;

	MinimumStatus = inMinimumStatus;

	Moderated = false;

	LogDebug(
		"New ChatChannel created: Name: [{}], Owner: [{}], Password: [{}], MinStatus: [{}]",
		Name.c_str(),
		Owner.c_str(),
		Password.c_str(),
		MinimumStatus
	);

}

ChatChannel::~ChatChannel() {

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements())
		iterator.RemoveCurrent(false);
}

ChatChannel* ChatChannelList::CreateChannel(std::string name, std::string owner, std::string password, bool permanent, int minimum_status, bool save_to_db) {
	uint8 max_perm_player_channels = RuleI(Chat, MaxPermanentPlayerChannels);

	if (!database.CheckChannelNameFilter(name)) {
		if (!(owner == "*System*")) {
			return nullptr;
		}
		else {
			LogDebug("Ignoring Name Filter as channel is owned by System...");
		}
	}


	ChatChannel *NewChannel = new ChatChannel(CapitaliseName(name), owner, password, permanent, minimum_status);

	ChatChannels.Insert(NewChannel);

	if (owner == "*System*") {
		save_to_db = false;
	}

	if ((max_perm_player_channels > 0) && !(owner == "*System*") && save_to_db) { // If permenant player channels are enabled (and not a system channel), save channel to database if not exceeding limit.
		if (database.CurrentPlayerChannelCount(owner) + 1 <= max_perm_player_channels) { // Ensure there is room to save another chat channel to the database.
			database.SaveChatChannel(CapitaliseName(name), owner, password, minimum_status); // Save chat channel to database.
		}
		else {
			LogDebug("Maximum number of channels [{}] reached for player [{}], channel [{}] save to database aborted.", max_perm_player_channels, owner, CapitaliseName(name));
		}
	}

	return NewChannel;
}

ChatChannel* ChatChannelList::FindChannel(std::string Name) {

	std::string NormalisedName = CapitaliseName(Name);

	LinkedListIterator<ChatChannel*> iterator(ChatChannels);

	iterator.Reset();

	while(iterator.MoreElements()) {

		ChatChannel *CurrentChannel = iterator.GetData();

		if(CurrentChannel && (CurrentChannel->Name == NormalisedName))
			return iterator.GetData();

		iterator.Advance();
	}

	return nullptr;
}

void ChatChannelList::SendAllChannels(Client *c) {

	if(!c) return;

	if(!c->CanListAllChannels()) {
		c->GeneralChannelMessage("You do not have permission to list all the channels.");
		return;
	}

	c->GeneralChannelMessage("All current channels:");

	int ChannelsInLine = 0;

	LinkedListIterator<ChatChannel*> iterator(ChatChannels);

	iterator.Reset();

	std::string Message;

	char CountString[10];

	while(iterator.MoreElements()) {

		ChatChannel *CurrentChannel = iterator.GetData();

		if(!CurrentChannel || (CurrentChannel->GetMinStatus() > c->GetAccountStatus())) {

			iterator.Advance();

			continue;
		}

		if(ChannelsInLine > 0)
			Message += ", ";

		sprintf(CountString, "(%i)", CurrentChannel->MemberCount(c->GetAccountStatus()));

		Message += CurrentChannel->GetName();

		Message += CountString;

		ChannelsInLine++;

		if(ChannelsInLine == 6) {

			c->GeneralChannelMessage(Message);

			ChannelsInLine = 0;

			Message.clear();
		}

		iterator.Advance();
	}

	if(ChannelsInLine > 0)
		c->GeneralChannelMessage(Message);

}

void ChatChannelList::RemoveChannel(ChatChannel *Channel) {

	LogDebug("RemoveChannel ([{}])", Channel->GetName().c_str());

	LinkedListIterator<ChatChannel*> iterator(ChatChannels);

	iterator.Reset();

	while(iterator.MoreElements()) {

		if(iterator.GetData() == Channel) {

			iterator.RemoveCurrent();

			return;
		}

		iterator.Advance();
	}
}

void ChatChannelList::RemoveAllChannels() {

	LogDebug("RemoveAllChannels");

	LinkedListIterator<ChatChannel*> iterator(ChatChannels);

	iterator.Reset();

	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}

int ChatChannel::MemberCount(int Status) {

	int Count = 0;

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		Client *ChannelClient = iterator.GetData();

		if(ChannelClient && (!ChannelClient->GetHideMe() || (ChannelClient->GetAccountStatus() < Status)))
			Count++;

		iterator.Advance();
	}

	return Count;
}

void ChatChannel::SetPassword(std::string inPassword) {

	Password = inPassword;

	if(Permanent)
	{
		RemoveApostrophes(Password);
		database.SetChannelPassword(Name, Password);
	}
}

void ChatChannel::SetOwner(std::string inOwner) {

	Owner = inOwner;

	if(Permanent)
		database.SetChannelOwner(Name, Owner);
}

std::string ChatChannel::GetOwnerName() {
	return Owner;
}

void ChatChannel::SetTemporary() {
	Permanent = false;
}

void ChatChannel::SetPermanent() {
	Permanent = true;
}

void ChatChannel::AddClient(Client *c) {

	if(!c) return;

	DeleteTimer.Disable();

	if(IsClientInChannel(c)) {

		LogInfo("Client [{}] already in channel [{}]", c->GetName().c_str(), GetName().c_str());

		return;
	}

	bool HideMe = c->GetHideMe();

	int AccountStatus = c->GetAccountStatus();

	LogDebug("Adding [{}] to channel [{}]", c->GetName().c_str(), Name.c_str());

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		Client *CurrentClient = iterator.GetData();

		if(CurrentClient && CurrentClient->IsAnnounceOn())
			if(!HideMe || (CurrentClient->GetAccountStatus() > AccountStatus))
				CurrentClient->AnnounceJoin(this, c);

		iterator.Advance();
	}

	ClientsInChannel.Insert(c);

}

bool ChatChannel::RemoveClient(Client *c) {

	if(!c) return false;

	LogDebug("RemoveClient [{}] from channel [{}]", c->GetName().c_str(), GetName().c_str());

	bool HideMe = c->GetHideMe();

	int AccountStatus = c->GetAccountStatus();

	int PlayersInChannel = 0;

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		Client *CurrentClient = iterator.GetData();

		if(CurrentClient == c) {
			iterator.RemoveCurrent(false);
		}
		else if(CurrentClient) {

			PlayersInChannel++;

			if(CurrentClient->IsAnnounceOn())
				if(!HideMe || (CurrentClient->GetAccountStatus() > AccountStatus))
					CurrentClient->AnnounceLeave(this, c);

			iterator.Advance();
		}

	}

	if((PlayersInChannel == 0) && !Permanent) {

		if((Password.length() == 0) || (RuleI(Channels, DeleteTimer) == 0))
			return false;

		LogDebug("Starting delete timer for empty password protected channel [{}]", Name.c_str());

		DeleteTimer.Start(RuleI(Channels, DeleteTimer) * 60000);
	}

	return true;
}

void ChatChannel::SendOPList(Client *c)
{
	if (!c)
		return;

	c->GeneralChannelMessage("Channel " + Name + " op-list: (Owner=" + Owner + ")");

	for (auto &&m : Moderators)
		c->GeneralChannelMessage(m);
}

void ChatChannel::SendChannelMembers(Client *c) {

	if(!c) return;

	char CountString[10];

	sprintf(CountString, "(%i)", MemberCount(c->GetAccountStatus()));

	std::string Message = "Channel " + GetName();

	Message += CountString;

	Message += " members:";

	c->GeneralChannelMessage(Message);

	int AccountStatus = c->GetAccountStatus();

	Message.clear();

	int MembersInLine = 0;

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		Client *ChannelClient = iterator.GetData();

		// Don't list hidden characters with status higher or equal than the character requesting the list.
		//
		if(!ChannelClient || (ChannelClient->GetHideMe() && (ChannelClient->GetAccountStatus() >= AccountStatus))) {
			iterator.Advance();
			continue;
		}

		if(MembersInLine > 0)
			Message += ", ";

		Message += ChannelClient->GetName();

		MembersInLine++;

		if(MembersInLine == 6) {

			c->GeneralChannelMessage(Message);

			MembersInLine = 0;

			Message.clear();
		}

		iterator.Advance();
	}

	if(MembersInLine > 0)
		c->GeneralChannelMessage(Message);

}

void ChatChannel::SendMessageToChannel(std::string Message, Client* Sender) {

	if(!Sender) return;

	std::string cv_messages[EQ::versions::ClientVersionCount];

	ChatMessagesSent++;

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		Client *ChannelClient = iterator.GetData();

		if(ChannelClient)
		{
			LogDebug("Sending message to [{}] from [{}]",
				ChannelClient->GetName().c_str(), Sender->GetName().c_str());

			if (cv_messages[static_cast<uint32>(ChannelClient->GetClientVersion())].length() == 0) {
				switch (ChannelClient->GetClientVersion()) {
				case EQ::versions::ClientVersion::Titanium:
					ServerToClient45SayLink(cv_messages[static_cast<uint32>(ChannelClient->GetClientVersion())], Message);
					break;
				case EQ::versions::ClientVersion::SoF:
				case EQ::versions::ClientVersion::SoD:
				case EQ::versions::ClientVersion::UF:
					ServerToClient50SayLink(cv_messages[static_cast<uint32>(ChannelClient->GetClientVersion())], Message);
					break;
				case EQ::versions::ClientVersion::RoF:
					ServerToClient55SayLink(cv_messages[static_cast<uint32>(ChannelClient->GetClientVersion())], Message);
					break;
				case EQ::versions::ClientVersion::RoF2:
				default:
					cv_messages[static_cast<uint32>(ChannelClient->GetClientVersion())] = Message;
					break;
				}
			}

			ChannelClient->SendChannelMessage(Name, cv_messages[static_cast<uint32>(ChannelClient->GetClientVersion())], Sender);
		}

		iterator.Advance();
	}
}

void ChatChannel::SetModerated(bool inModerated) {

	Moderated = inModerated;

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		Client *ChannelClient = iterator.GetData();

		if(ChannelClient) {

			if(Moderated)
				ChannelClient->GeneralChannelMessage("Channel " + Name + " is now moderated.");
			else
				ChannelClient->GeneralChannelMessage("Channel " + Name + " is no longer moderated.");
		}

		iterator.Advance();
	}

}
bool ChatChannel::IsClientInChannel(Client *c) {

	if(!c) return false;

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		if(iterator.GetData() == c)
			return true;

		iterator.Advance();
	}

	return false;
}

ChatChannel *ChatChannelList::AddClientToChannel(std::string channel_name, Client *c, bool command_directed) {

	if(!c) return nullptr;

	if (IsOnChannelBlockList(channel_name)) { // Ensure channel name is not blocked
		c->GeneralChannelMessage("That channel name is blocked by the server operator.");
		return nullptr;
	}
	else if ((channel_name.length() > 0) && (isdigit(channel_name[0]))) { // Ensure channel name does not start with a number
		c->GeneralChannelMessage("The channel name can not begin with a number.");
		return nullptr;
	}
	else if (channel_name.empty()) { // Ensure channel name is not empty
		return nullptr;
	}

	std::string normalized_name, password;

	std::string::size_type Colon = channel_name.find_first_of(":");

	if(Colon == std::string::npos)
		normalized_name = CapitaliseName(channel_name);
	else {
		normalized_name = CapitaliseName(channel_name.substr(0, Colon));

		password = channel_name.substr(Colon + 1);
	}

	if((normalized_name.length() > 64) || (password.length() > 64)) {

		c->GeneralChannelMessage("The channel name or password cannot exceed 64 characters.");

		return nullptr;
	}

	ChatChannel *RequiredChannel = FindChannel(normalized_name);
	std::string channel_owner = c->GetName();

	bool permanent = false;
	if (command_directed && RuleI(Chat, MaxPermanentPlayerChannels) > 0) {
		permanent = true;
	}

	if (!RequiredChannel) {
		RequiredChannel = CreateChannel(normalized_name, channel_owner, password, permanent, 0, command_directed);
		if (RequiredChannel == nullptr) {
			LogDebug("Failed to create new channel with name: {}. Possible blocked or reserved channel name.", normalized_name);
			c->GeneralChannelMessage("Failed to create new channel with provided name. Possible blocked or reserved channel name.");
			return nullptr;
		}
		LogDebug("Created and added Client to channel [{}] with password [{}]. Owner: {}. Command Directed: {}", normalized_name.c_str(), password.c_str(), channel_owner, command_directed);
	}

	if(RequiredChannel->GetMinStatus() > c->GetAccountStatus()) {

		std::string Message = "You do not have the required account status to join channel " + normalized_name;

		c->GeneralChannelMessage(Message);

		return nullptr;
	}

	if (RequiredChannel->IsClientInChannel(c)) {
		return nullptr;
	}


	if(RequiredChannel->IsInvitee(c->GetName())) {

		RequiredChannel->AddClient(c);

		RequiredChannel->RemoveInvitee(c->GetName());

		return RequiredChannel;
	}

	if(RequiredChannel->CheckPassword(password) || RequiredChannel->IsOwner(c->GetName()) || RequiredChannel->IsModerator(c->GetName()) ||
			c->IsChannelAdmin()) {

		RequiredChannel->AddClient(c);

		return RequiredChannel;
	}

	c->GeneralChannelMessage("Incorrect password for channel " + (normalized_name));

	return nullptr;
}

ChatChannel *ChatChannelList::RemoveClientFromChannel(std::string in_channel_name, Client *c, bool command_directed) {
	if(!c) return nullptr;

	std::string channel_name = in_channel_name;

	if ((in_channel_name.length() > 0) && isdigit(channel_name[0])) {
		channel_name = c->ChannelSlotName(atoi(in_channel_name.c_str()));
	}

	ChatChannel *RequiredChannel = FindChannel(channel_name);

	if (!RequiredChannel) {
		return nullptr;
	}

	LogDebug("Client [{}] removed from channel [{}]. Channel is owned by {}. Command directed: {}", c->GetName(), channel_name, RequiredChannel->GetOwnerName(), command_directed);
	if ((c->GetName() == RequiredChannel->GetOwnerName()) && command_directed) { // Check if the client that is leaving is the the channel owner
		LogDebug("Owner left the channel [{}], removing channel from database...", channel_name);
		database.DeleteChatChannel(channel_name); // Remove the channel from the database.
		LogDebug("Flagging [{}] channel as temporary...", channel_name);
		RequiredChannel->SetTemporary();
	}

	// RemoveClient will return false if there is no-one left in the channel, and the channel is not permanent and has
	// no password.
	if (!RequiredChannel->RemoveClient(c)) {
		LogDebug("Noone left in the temporary channel [{}] and no password is set; removing temporary channel.", channel_name);
		RemoveChannel(RequiredChannel);
	}

	return RequiredChannel;
}

void ChatChannelList::Process() {

	LinkedListIterator<ChatChannel*> iterator(ChatChannels);

	iterator.Reset();

	while(iterator.MoreElements()) {

		ChatChannel *CurrentChannel = iterator.GetData();

		if(CurrentChannel && CurrentChannel->ReadyToDelete()) {

			LogDebug("Empty temporary password protected channel [{}] being destroyed",
				CurrentChannel->GetName().c_str());

			RemoveChannel(CurrentChannel);
		}

		iterator.Advance();

	}
}

void ChatChannel::AddInvitee(const std::string &Invitee)
{
	if (!IsInvitee(Invitee)) {
		Invitees.push_back(Invitee);

		LogDebug("Added [{}] as invitee to channel [{}]", Invitee.c_str(), Name.c_str());
	}

}

void ChatChannel::RemoveInvitee(std::string Invitee)
{
	auto it = std::find(std::begin(Invitees), std::end(Invitees), Invitee);

	if(it != std::end(Invitees)) {
		Invitees.erase(it);
		LogDebug("Removed [{}] as invitee to channel [{}]", Invitee.c_str(), Name.c_str());
	}
}

bool ChatChannel::IsInvitee(std::string Invitee)
{
	return std::find(std::begin(Invitees), std::end(Invitees), Invitee) != std::end(Invitees);
}

void ChatChannel::AddModerator(const std::string &Moderator)
{
	if (!IsModerator(Moderator)) {
		Moderators.push_back(Moderator);

		LogInfo("Added [{}] as moderator to channel [{}]", Moderator.c_str(), Name.c_str());
	}

}

void ChatChannel::RemoveModerator(const std::string &Moderator)
{
	auto it = std::find(std::begin(Moderators), std::end(Moderators), Moderator);

	if (it != std::end(Moderators)) {
		Moderators.erase(it);
		LogInfo("Removed [{}] as moderator to channel [{}]", Moderator.c_str(), Name.c_str());
	}
}

bool ChatChannel::IsModerator(std::string Moderator)
{
	return std::find(std::begin(Moderators), std::end(Moderators), Moderator) != std::end(Moderators);
}

void ChatChannel::AddVoice(const std::string &inVoiced)
{
	if (!HasVoice(inVoiced)) {
		Voiced.push_back(inVoiced);

		LogInfo("Added [{}] as voiced to channel [{}]", inVoiced.c_str(), Name.c_str());
	}
}

void ChatChannel::RemoveVoice(const std::string &inVoiced)
{
	auto it = std::find(std::begin(Voiced), std::end(Voiced), inVoiced);

	if (it != std::end(Voiced)) {
		Voiced.erase(it);

		LogInfo("Removed [{}] as voiced to channel [{}]", inVoiced.c_str(), Name.c_str());
	}
}

bool ChatChannel::HasVoice(std::string inVoiced)
{
	return std::find(std::begin(Voiced), std::end(Voiced), inVoiced) != std::end(Voiced);
}

std::string CapitaliseName(std::string inString) {

	std::string NormalisedName = inString;

	for(unsigned int i = 0; i < NormalisedName.length(); i++) {

		if(i == 0)
			NormalisedName[i] = toupper(NormalisedName[i]);
		else
			NormalisedName[i] = tolower(NormalisedName[i]);
	}

	return NormalisedName;
}

bool ChatChannelList::IsOnChannelBlockList(const std::string& channel_name) {
	if (channel_name.empty()) {
		return false;
	}

	// Check if channel_name is already in the BlockedChannelNames vector
	return Strings::Contains(ChatChannelList::GetBlockedChannelNames(), channel_name);
}

void ChatChannelList::AddToChannelBlockList(const std::string& channel_name) {
	if (channel_name.empty()) {
		return;
	}

	// Check if channelName is already in the BlockedChannelNames vector
	bool is_found = Strings::Contains(ChatChannelList::GetBlockedChannelNames(), channel_name);

	// Add channelName to the BlockedChannelNames vector if it is not already present
	if (!is_found) {
		auto blocked_channel_names = GetBlockedChannelNames(); // Get current blocked list
		blocked_channel_names.push_back(channel_name); // Add new name to local blocked list
		SetChannelBlockList(blocked_channel_names); // Set blocked list to match local blocked list
	}
}

void ServerToClient45SayLink(std::string& clientSayLink, const std::string& serverSayLink) {
	if (serverSayLink.find('\x12') == std::string::npos) {
		clientSayLink = serverSayLink;
		return;
	}

	auto segments = Strings::Split(serverSayLink, '\x12');

	for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
		if (segment_iter & 1) {
			if (segments[segment_iter].length() <= 56) {
				clientSayLink.append(segments[segment_iter]);
				// TODO: log size mismatch error
				continue;
			}

			// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
			// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
			// 6.2:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X       XXXXXXXX (45)
			// Diff:                                       ^^^^^         ^  ^^^^^

			clientSayLink.push_back('\x12');
			clientSayLink.append(segments[segment_iter].substr(0, 31));
			clientSayLink.append(segments[segment_iter].substr(36, 5));

			if (segments[segment_iter][41] == '0')
				clientSayLink.push_back(segments[segment_iter][42]);
			else
				clientSayLink.push_back('F');

			clientSayLink.append(segments[segment_iter].substr(48));
			clientSayLink.push_back('\x12');
		}
		else {
			clientSayLink.append(segments[segment_iter]);
		}
	}
}

void ServerToClient50SayLink(std::string& clientSayLink, const std::string& serverSayLink) {
	if (serverSayLink.find('\x12') == std::string::npos) {
		clientSayLink = serverSayLink;
		return;
	}

	auto segments = Strings::Split(serverSayLink, '\x12');

	for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
		if (segment_iter & 1) {
			if (segments[segment_iter].length() <= 56) {
				clientSayLink.append(segments[segment_iter]);
				// TODO: log size mismatch error
				continue;
			}

			// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
			// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
			// SoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X XXXXX XXXXXXXX (50)
			// Diff:                                       ^^^^^         ^

			clientSayLink.push_back('\x12');
			clientSayLink.append(segments[segment_iter].substr(0, 31));
			clientSayLink.append(segments[segment_iter].substr(36, 5));

			if (segments[segment_iter][41] == '0')
				clientSayLink.push_back(segments[segment_iter][42]);
			else
				clientSayLink.push_back('F');

			clientSayLink.append(segments[segment_iter].substr(43));
			clientSayLink.push_back('\x12');
		}
		else {
			clientSayLink.append(segments[segment_iter]);
		}
	}
}

void ServerToClient55SayLink(std::string& clientSayLink, const std::string& serverSayLink) {
	if (serverSayLink.find('\x12') == std::string::npos) {
		clientSayLink = serverSayLink;
		return;
	}

	auto segments = Strings::Split(serverSayLink, '\x12');

	for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
		if (segment_iter & 1) {
			if (segments[segment_iter].length() <= 56) {
				clientSayLink.append(segments[segment_iter]);
				// TODO: log size mismatch error
				continue;
			}

			// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
			// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
			// RoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX  X XXXXX XXXXXXXX (55)
			// Diff:                                                     ^

			clientSayLink.push_back('\x12');
			clientSayLink.append(segments[segment_iter].substr(0, 41));

			if (segments[segment_iter][41] == '0')
				clientSayLink.push_back(segments[segment_iter][42]);
			else
				clientSayLink.push_back('F');

			clientSayLink.append(segments[segment_iter].substr(43));
			clientSayLink.push_back('\x12');
		}
		else {
			clientSayLink.append(segments[segment_iter]);
		}
	}
}
