#ifndef CHATCHANNEL_H
#define CHATCHANNEL_H

//#include "clientlist.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include <string>
#include <vector>

class Client;

#define SYSTEM_OWNER std::string("*System*")

class ChatChannel {

public:

	ChatChannel(std::string inName, std::string inOwner, std::string inPassword, bool inPermanent, int inMinimumStatus = 0);
	~ChatChannel();

	void AddClient(Client *c);
	bool RemoveClient(Client *c);
	bool IsClientInChannel(Client *c);

	int MemberCount(int Status);
	const std::string &GetName() { return Name; }
	void SendMessageToChannel(std::string Message, Client* Sender);
	bool CheckPassword(std::string inPassword) { return Password.empty() || Password == inPassword; }
	void SetPassword(std::string inPassword);
	bool IsOwner(std::string Name) { return (Owner == Name); }
	const std::string& GetPassword() { return Password; }
	void SetOwner(std::string inOwner);
	std::string GetOwnerName();
	void SetTemporary();
	void SetPermanent();
	void SendChannelMembers(Client *c);
	int GetMinStatus() { return MinimumStatus; }
	bool ReadyToDelete() { return DeleteTimer.Check(); }
	void SendOPList(Client *c);
	void AddInvitee(const std::string &Invitee);
	void RemoveInvitee(std::string Invitee);
	bool IsInvitee(std::string Invitee);
	void AddModerator(const std::string &Moderator);
	void RemoveModerator(const std::string &Moderator);
	bool IsModerator(std::string Moderator);
	void AddVoice(const std::string &Voiced);
	void RemoveVoice(const std::string &Voiced);
	bool HasVoice(std::string Voiced);
	inline bool IsModerated() { return Moderated; }
	void SetModerated(bool inModerated);

	friend class ChatChannelList;

private:

	std::string Name;
	std::string Owner;
	std::string Password;

	bool Permanent;
	bool Moderated;

	int MinimumStatus;

	Timer DeleteTimer;

	LinkedList<Client*> ClientsInChannel;

	std::vector<std::string> Moderators;
	std::vector<std::string> Invitees;
	std::vector<std::string> Voiced;

};

class ChatChannelList {

public:
	ChatChannel* CreateChannel(std::string Name, std::string Owner, std::string Password, bool Permanent, int MinimumStatus, bool SaveToDB = false);
	ChatChannel* FindChannel(std::string Name);
	ChatChannel* AddClientToChannel(std::string ChannelName, Client* c, bool command_directed = false);
	ChatChannel* RemoveClientFromChannel(std::string in_channel_name, Client* c, bool command_directed = false);
	void RemoveChannel(ChatChannel *Channel);
	void RemoveAllChannels();
	void SendAllChannels(Client *c);
	void Process();
	static inline std::vector<std::string> GetBlockedChannelNames() { return m_blocked_channel_names; }
	static inline void ClearChannelBlockList() { m_blocked_channel_names.clear(); };
	static void AddToChannelBlockList(const std::string& channel_name);
	static bool IsOnChannelBlockList(const std::string& channel_name);
	static inline void SetChannelBlockList(std::vector<std::string> new_list) { m_blocked_channel_names = new_list; }
private:

	LinkedList<ChatChannel*> ChatChannels;
	static inline std::vector<std::string> m_blocked_channel_names;

};

std::string CapitaliseName(std::string inString);

#endif
