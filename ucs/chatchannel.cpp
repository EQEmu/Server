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
#include "../common/string_util.h"
#include "chatchannel.h"
#include "clientlist.h"
#include "database.h"
#include <cstdlib>
#include <algorithm>

extern Database database;
extern uint32 ChatMessagesSent;

ChatChannel::ChatChannel(std::string inName, std::string inOwner, std::string inPassword, bool inPermanent, int inMinimumStatus) :
	DeleteTimer(0) {

	Name = inName;

	Owner = inOwner;

	Password = inPassword;

	Permanent = inPermanent;

	MinimumStatus = inMinimumStatus;

	Moderated = false;

	Log(Logs::Detail, Logs::UCS_Server, "New ChatChannel created: Name: [%s], Owner: [%s], Password: [%s], MinStatus: %i",
					Name.c_str(), Owner.c_str(), Password.c_str(), MinimumStatus);

}

ChatChannel::~ChatChannel() {

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements())
		iterator.RemoveCurrent(false);
}

ChatChannel* ChatChannelList::CreateChannel(std::string Name, std::string Owner, std::string Password, bool Permanent, int MinimumStatus) {

	ChatChannel *NewChannel = new ChatChannel(CapitaliseName(Name), Owner, Password, Permanent, MinimumStatus);

	ChatChannels.Insert(NewChannel);

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

	Log(Logs::Detail, Logs::UCS_Server, "RemoveChannel(%s)", Channel->GetName().c_str());

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

	Log(Logs::Detail, Logs::UCS_Server, "RemoveAllChannels");

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

void ChatChannel::AddClient(Client *c) {

	if(!c) return;

	DeleteTimer.Disable();

	if(IsClientInChannel(c)) {

		Log(Logs::Detail, Logs::UCS_Server, "Client %s already in channel %s", c->GetName().c_str(), GetName().c_str());

		return;
	}

	bool HideMe = c->GetHideMe();

	int AccountStatus = c->GetAccountStatus();

	Log(Logs::Detail, Logs::UCS_Server, "Adding %s to channel %s", c->GetName().c_str(), Name.c_str());

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

	Log(Logs::Detail, Logs::UCS_Server, "RemoveClient %s from channel %s", c->GetName().c_str(), GetName().c_str());

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

		Log(Logs::Detail, Logs::UCS_Server, "Starting delete timer for empty password protected channel %s", Name.c_str());

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

	ChatMessagesSent++;

	LinkedListIterator<Client*> iterator(ClientsInChannel);

	iterator.Reset();

	while(iterator.MoreElements()) {

		Client *ChannelClient = iterator.GetData();

		if(ChannelClient)
		{
			Log(Logs::Detail, Logs::UCS_Server, "Sending message to %s from %s",
					ChannelClient->GetName().c_str(), Sender->GetName().c_str());
			ChannelClient->SendChannelMessage(Name, Message, Sender);
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

ChatChannel *ChatChannelList::AddClientToChannel(std::string ChannelName, Client *c) {

	if(!c) return nullptr;

	if((ChannelName.length() > 0) && (isdigit(ChannelName[0]))) {

		c->GeneralChannelMessage("The channel name can not begin with a number.");

		return nullptr;
	}

	std::string NormalisedName, Password;

	std::string::size_type Colon = ChannelName.find_first_of(":");

	if(Colon == std::string::npos)
		NormalisedName = CapitaliseName(ChannelName);
	else {
		NormalisedName = CapitaliseName(ChannelName.substr(0, Colon));

		Password = ChannelName.substr(Colon + 1);
	}

	if((NormalisedName.length() > 64) || (Password.length() > 64)) {

		c->GeneralChannelMessage("The channel name or password cannot exceed 64 characters.");

		return nullptr;
	}

	Log(Logs::Detail, Logs::UCS_Server, "AddClient to channel [%s] with password [%s]", NormalisedName.c_str(), Password.c_str());

	ChatChannel *RequiredChannel = FindChannel(NormalisedName);

	if(!RequiredChannel)
		RequiredChannel = CreateChannel(NormalisedName, c->GetName(), Password, false, 0);

	if(RequiredChannel->GetMinStatus() > c->GetAccountStatus()) {

		std::string Message = "You do not have the required account status to join channel " + NormalisedName;

		c->GeneralChannelMessage(Message);

		return nullptr;
	}

	if(RequiredChannel->IsClientInChannel(c))
		return nullptr;

	if(RequiredChannel->IsInvitee(c->GetName())) {

		RequiredChannel->AddClient(c);

		RequiredChannel->RemoveInvitee(c->GetName());

		return RequiredChannel;
	}

	if(RequiredChannel->CheckPassword(Password) || RequiredChannel->IsOwner(c->GetName()) || RequiredChannel->IsModerator(c->GetName()) ||
			c->IsChannelAdmin()) {

		RequiredChannel->AddClient(c);

		return RequiredChannel;
	}

	c->GeneralChannelMessage("Incorrect password for channel " + (NormalisedName));

	return nullptr;
}

ChatChannel *ChatChannelList::RemoveClientFromChannel(std::string inChannelName, Client *c) {

	if(!c) return nullptr;

	std::string ChannelName = inChannelName;

	if((inChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = c->ChannelSlotName(atoi(inChannelName.c_str()));

	ChatChannel *RequiredChannel = FindChannel(ChannelName);

	if(!RequiredChannel)
		return nullptr;

	// RemoveClient will return false if there is no-one left in the channel, and the channel is not permanent and has
	// no password.
	//
	if(!RequiredChannel->RemoveClient(c))
		RemoveChannel(RequiredChannel);

	return RequiredChannel;
}

void ChatChannelList::Process() {

	LinkedListIterator<ChatChannel*> iterator(ChatChannels);

	iterator.Reset();

	while(iterator.MoreElements()) {

		ChatChannel *CurrentChannel = iterator.GetData();

		if(CurrentChannel && CurrentChannel->ReadyToDelete()) {

			Log(Logs::Detail, Logs::UCS_Server, "Empty temporary password protected channel %s being destroyed.",
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

		Log(Logs::Detail, Logs::UCS_Server, "Added %s as invitee to channel %s", Invitee.c_str(), Name.c_str());
	}

}

void ChatChannel::RemoveInvitee(std::string Invitee)
{
	auto it = std::find(std::begin(Invitees), std::end(Invitees), Invitee);

	if(it != std::end(Invitees)) {
		Invitees.erase(it);
		Log(Logs::Detail, Logs::UCS_Server, "Removed %s as invitee to channel %s", Invitee.c_str(), Name.c_str());
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

		Log(Logs::Detail, Logs::UCS_Server, "Added %s as moderator to channel %s", Moderator.c_str(), Name.c_str());
	}

}

void ChatChannel::RemoveModerator(const std::string &Moderator)
{
	auto it = std::find(std::begin(Moderators), std::end(Moderators), Moderator);

	if (it != std::end(Moderators)) {
		Moderators.erase(it);
		Log(Logs::Detail, Logs::UCS_Server, "Removed %s as moderator to channel %s", Moderator.c_str(), Name.c_str());
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

		Log(Logs::Detail, Logs::UCS_Server, "Added %s as voiced to channel %s", inVoiced.c_str(), Name.c_str());
	}
}

void ChatChannel::RemoveVoice(const std::string &inVoiced)
{
	auto it = std::find(std::begin(Voiced), std::end(Voiced), inVoiced);

	if (it != std::end(Voiced)) {
		Voiced.erase(it);

		Log(Logs::Detail, Logs::UCS_Server, "Removed %s as voiced to channel %s", inVoiced.c_str(), Name.c_str());
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

