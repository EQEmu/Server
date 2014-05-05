/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2004  EQEMu Development Team (http://eqemulator.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __QUEST_MANAGER_H__
#define __QUEST_MANAGER_H__

#include "../common/timer.h"
#include "tasks.h"

#include <string>
#include <list>
#include <stack>

class NPC;
class Client;

class QuestManager {
	struct running_quest {
		Mob *owner;
		Client *initiator;
		ItemInst* questitem;
		bool depop_npc;
	};
public:
	QuestManager();
	virtual ~QuestManager();

	void StartQuest(Mob *_owner, Client *_initiator = nullptr, ItemInst* _questitem = nullptr);
	void EndQuest();
	bool QuestsRunning() { return !quests_running_.empty(); }

	void Process();

	void ClearTimers(Mob *who);
	void ClearAllTimers();

	//quest functions
	void echo(int colour, const char *str);
	void say(const char *str);
	void say(const char *str, uint8 language);
	void me(const char *str);
	void summonitem(uint32 itemid, int16 charges = -1);
	void write(const char *file, const char *str);
	Mob* spawn2(int npc_type, int grid, int unused, float x, float y, float z, float heading);
	Mob* unique_spawn(int npc_type, int grid, int unused, float x, float y, float z, float heading = 0);
	Mob* spawn_from_spawn2(uint32 spawn2_id);
	void enable_spawn2(uint32 spawn2_id);
	void disable_spawn2(uint32 spawn2_id);
	void setstat(int stat, int value);
	void incstat(int stat, int value);
	void castspell(int spell_id, int target_id);
	void selfcast(int spell_id);
	void addloot(int item_id, int charges = 0, bool equipitem = true);
	void Zone(const char *zone_name);
	void settimer(const char *timer_name, int seconds);
	void settimerMS(const char *timer_name, int milliseconds);
	void settimerMS(const char *timer_name, int milliseconds, ItemInst *inst);
	void settimerMS(const char *timer_name, int milliseconds, Mob *mob);
	void stoptimer(const char *timer_name);
	void stoptimer(const char *timer_name, ItemInst *inst);
	void stoptimer(const char *timer_name, Mob *mob);
	void stopalltimers();
	void stopalltimers(ItemInst *inst);
	void stopalltimers(Mob *mob);
	void emote(const char *str);
	void shout(const char *str);
	void shout2(const char *str);
	void gmsay(const char *str, uint32 color, bool send_to_world, uint32 to_guilddbid, uint32 to_minstatus);
	void depop(int npc_type = 0); // depop NPC and don't start spawn timer
	void depop_withtimer(int npc_type = 0); // depop NPC and start spawn timer
	void depopall(int npc_type = 0);
	void depopzone(bool StartSpawnTimer = true);
	void repopzone();
	void settarget(const char *type, int target_id);
	void follow(int entity_id, int distance);
	void sfollow();
	void changedeity(int diety_id);
	void exp(int amt);
	void level(int newlevel);
	void traindisc(int discipline_tome_item_id);
	bool isdisctome(int item_id);
	void safemove();
	void rain(int weather);
	void snow(int weather);
	void surname(const char *name);
	void permaclass(int class_id);
	void permarace(int race_id);
	void permagender(int gender_id);
	uint16 scribespells(uint8 max_level, uint8 min_level = 1);
	uint16 traindiscs(uint8 max_level, uint8 min_level = 1);
	void unscribespells();
	void untraindiscs();
	void givecash(int copper, int silver, int gold, int platinum);
	void pvp(const char *mode);
	void movepc(int zone_id, float x, float y, float z, float heading);
	void gmmove(float x, float y, float z);
	void movegrp(int zoneid, float x, float y, float z);
	void doanim(int anim_id);
	void addskill(int skill_id, int value);
	void setlanguage(int skill_id, int value);
	void setskill(int skill_id, int value);
	void setallskill(int value);
	void attack(const char *client_name);
	void attacknpc(int npc_entity_id);
	void attacknpctype(int npc_type_id);
	void save();
	void faction(int faction_id, int faction_value, int temp);
	void setsky(uint8 new_sky);
	void setguild(uint32 new_guild_id, uint8 new_rank);
	void CreateGuild(const char *guild_name, const char *leader);
	void settime(uint8 new_hour, uint8 new_min);
	void itemlink(int item_id);
	void signal(int npc_id, int wait_ms = 0);
	void signalwith(int npc_id, int signal_id, int wait_ms = 0);
	void setglobal(const char *varname, const char *newvalue, int options, const char *duration);
	void targlobal(const char *varname, const char *value, const char *duration, int npcid, int charid, int zoneid);
	void delglobal(const char *varname);
	void ding();
	void rebind(int zoneid, float x, float y, float z);
	void start(int wp);
	void stop();
	void pause(int duration);
	void moveto(float x, float y, float z, float h, bool saveguardspot);
	void resume();
	void addldonpoints(int32 points, uint32 theme);
	void addldonwin(int32 wins, uint32 theme);
	void addldonloss(int32 losses, uint32 theme);
	void setnexthpevent(int at);
	void setnextinchpevent(int at);
	void respawn(int npc_type, int grid);
	void set_proximity(float minx, float maxx, float miny, float maxy, float minz=-999999, float maxz=999999);
	void clear_proximity();
	void enable_proximity_say();
	void disable_proximity_say();
	void setanim(int npc_type, int animnum);
	void showgrid(int gridid);
	void spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id, short new_value);
	short get_spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id);
	void toggle_spawn_event(int event_id, bool enable, bool strict, bool reset_base);
	bool has_zone_flag(int zone_id);
	void set_zone_flag(int zone_id);
	void clear_zone_flag(int zone_id);
	void sethp(int hpperc);
	bool summonburriedplayercorpse(uint32 char_id, float dest_x, float dest_y, float dest_z, float dest_heading);
	bool summonallplayercorpses(uint32 char_id, float dest_x, float dest_y, float dest_z, float dest_heading);
	uint32 getplayerburriedcorpsecount(uint32 char_id);
	bool buryplayercorpse(uint32 char_id);
	void forcedooropen(uint32 doorid, bool altmode);
	void forcedoorclose(uint32 doorid, bool altmode);
	void toggledoorstate(uint32 doorid);
	bool isdooropen(uint32 doorid);
	void npcrace(int race_id);
	void npcgender(int gender_id);	void npcsize(int newsize);
	void npctexture(int newtexture);
	void playerrace(int race_id);
	void playergender(int gender_id);
	void playersize(int newsize);
	void playertexture(int newtexture);
	void playerfeature(char *feature, int setting);
	void npcfeature(char *feature, int setting);
	void popup(const char *title, const char *text, uint32 popupid, uint32 buttons, uint32 Duration);
	void taskselector(int taskcount, int *tasks);
	void tasksetselector(int tasksettid);
	void enabletask(int taskcount, int *tasks);
	void disabletask(int taskcount, int *tasks);
	bool istaskenabled(int taskid);
	bool istaskactive(int task);
	bool istaskactivityactive(int task, int activity);
	int gettaskactivitydonecount(int task, int activity);
	void updatetaskactivity(int task, int activity, int count);
	void resettaskactivity(int task, int activity);
	void taskexploredarea(int exploreid);
	void assigntask(int taskid);
	void failtask(int taskid);
	int tasktimeleft(int taskid);
	int istaskcompleted(int taskid);
	int enabledtaskcount(int taskset);
	int firsttaskinset(int taskset);
	int lasttaskinset(int taskset);
	int nexttaskinset(int taskset, int taskid);
	int activespeaktask();
	int activespeakactivity(int taskid);
	int activetasksinset(int taskset);
	int completedtasksinset(int taskset);
	bool istaskappropriate(int task);
    void clearspawntimers();
	void ze(int type, const char *str);
	void we(int type, const char *str);
    int getlevel(uint8 type);
    int collectitems(uint32 item_id, bool remove);
    int collectitems_processSlot(int16 slot_id, uint32 item_id, bool remove);
    void enabletitle(int titleset);
   	bool checktitle(int titlecheck);
   	void removetitle(int titlecheck);
	uint16 CreateGroundObject(uint32 itemid, float x, float y, float z, float heading, uint32 decay_time = 300000);
	uint16 CreateGroundObjectFromModel(const char* model, float x, float y, float z, float heading, uint8 type = 0x00, uint32 decay_time = 0);
	void ModifyNPCStat(const char *identifier, const char *newValue);
	void UpdateSpawnTimer(uint32 id, uint32 newTime);
	void MerchantSetItem(uint32 NPCid, uint32 itemid, uint32 quantity = 0);
	uint32 MerchantCountItem(uint32 NPCid, uint32 itemid);
	uint16 CreateInstance(const char *zone, int16 version, uint32 duration);
	void DestroyInstance(uint16 instance_id);
	uint16 GetInstanceID(const char *zone, int16 version);
	void AssignToInstance(uint16 instance_id);
	void AssignGroupToInstance(uint16 instance_id);
	void AssignRaidToInstance(uint16 instance_id);
	void RemoveFromInstance(uint16 instance_id);
	//void RemoveGroupFromInstance(uint16 instance_id);	//potentially useful but not implmented at this time.
	//void RemoveRaidFromInstance(uint16 instance_id);	//potentially useful but not implmented at this time.
	void RemoveAllFromInstance(uint16 instance_id);
	void MovePCInstance(int zone_id, int instance_id, float x, float y, float z, float heading);
	void FlagInstanceByGroupLeader(uint32 zone, int16 version);
	void FlagInstanceByRaidLeader(uint32 zone, int16 version);
	const char* varlink(char* perltext, int item_id);
	const char* saylink(char* Phrase, bool silent, const char* LinkName);
	const char* getguildnamebyid(int guild_id);
	void SetRunning(bool val);
	bool IsRunning();
	void FlyMode(uint8 flymode);
	uint8 FactionValue();
	void wearchange(uint8 slot, uint16 texture);
	void voicetell(const char *str, int macronum, int racenum, int gendernum);
    void LearnRecipe(uint32 recipe_id);
    void SendMail(const char *to, const char *from, const char *subject, const char *message);
	uint16 CreateDoor( const char* model, float x, float y, float z, float heading, uint8 opentype, uint16 size);
    int32 GetZoneID(const char *zone);
    const char *GetZoneLongName(const char *zone);
	void CrossZoneSignalPlayerByCharID(int charid, uint32 data);
	void CrossZoneSignalPlayerByName(const char *CharName, uint32 data);
	void CrossZoneMessagePlayerByName(uint32 Type, const char *CharName, const char *Message);
	bool EnableRecipe(uint32 recipe_id);
	bool DisableRecipe(uint32 recipe_id);
	void ClearNPCTypeCache(int npctype_id);

	Client *GetInitiator() const;
	NPC *GetNPC() const;
	Mob *GetOwner() const;
	ItemInst *GetQuestItem() const;
	inline bool ProximitySayInUse() { return HaveProximitySays; }

#ifdef BOTS
	int createbotcount();
	int spawnbotcount();
	bool botquest();
	bool createBot(const char *name, const char *lastname, uint8 level, uint16 race, uint8 botclass, uint8 gender);
#endif

	inline uint16 GetMana(uint32 spell_id) { return( spells[spell_id].mana); }

private:
	std::stack<running_quest> quests_running_;

	bool HaveProximitySays;

	int QGVarDuration(const char *fmt);
	int InsertQuestGlobal(int charid, int npcid, int zoneid, const char *name, const char *value, int expdate);

	class QuestTimer {
	public:
		inline QuestTimer(int duration, Mob *_mob, std::string _name)
			: mob(_mob), name(_name), Timer_(duration) { Timer_.Start(duration, false); }
		Mob*   mob;
		std::string name;
		Timer Timer_;
	};
	class SignalTimer {
	public:
		inline SignalTimer(int duration, int _npc_id, int _signal_id) : npc_id(_npc_id), signal_id(_signal_id), Timer_(duration) { Timer_.Start(duration, false); }
		int npc_id;
		int signal_id;
		Timer Timer_;
	};
	std::list<QuestTimer>	QTimerList;
	std::list<SignalTimer>	STimerList;
	size_t item_timers;

};

extern QuestManager quest_manager;

#endif

