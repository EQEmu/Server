/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#ifndef GLOBAL_TASKS_H
#define GLOBAL_TASKS_H

#include <string>
#include <vector>
#include <algorithm>

#include "types.h"

/* This file contains what is needed for both zone and world managers
 */

#define MAXTASKS 10000
#define MAXTASKSETS 1000
// The Client has a hard cap of 19 active quests, 29 in SoD+
#define MAXACTIVEQUESTS 19
// The Max Chooser (Task Selector entries) is capped at 40 in the Titanium Client.
#define MAXCHOOSERENTRIES 40
// The Client has a hard cap of 20 activities per task.
#define MAXACTIVITIESPERTASK 20
// This is used to determine if a client's active task slot is empty.
#define TASKSLOTEMPTY 0

// Command Codes for worldserver ServerOP_ReloadTasks
#define RELOADTASKS				0
#define RELOADTASKGOALLISTS		1
#define RELOADTASKPROXIMITIES	2
#define RELOADTASKSETS			3

// used for timer lockouts and /tasktimers
struct TaskTimer {
	int ID; // ID used in task timer (replay group)
	int original_id; // original ID of the task (book keeping)
	int expires; // UNIX timestamp of when it expires, what happens with DLS? Fuck it.
};

typedef enum { METHODSINGLEID = 0, METHODLIST = 1, METHODQUEST = 2 } TaskMethodType;

struct ActivityInformation {
	int		StepNumber;
	int		Type;
	std::string target_name; // name mob, location -- default empty
	std::string item_list; // likely defaults to empty
	std::string skill_list; // IDs ; separated -- default -1
	std::string spell_list; // IDs ; separated -- default 0
	std::string desc_override; // overrides auto generated description -- default empty
	int		skill_id; // older clients, first id from above
	int		spell_id; // older clients, first id from above
	int		GoalID;
	TaskMethodType GoalMethod;
	int		GoalCount;
	int		DeliverToNPC;
	std::vector<int>	ZoneIDs;
	std::string zones; // IDs ; searated, ZoneID is the first in this list for older clients -- default empty string
	bool	Optional;

	inline bool CheckZone(int zone_id) {
		if (ZoneIDs.empty())
			return true;
		return std::find(ZoneIDs.begin(), ZoneIDs.end(), zone_id) != ZoneIDs.end();
	}
};

typedef enum { ActivitiesSequential = 0, ActivitiesStepped = 1 } SequenceType;

enum class TaskType {
	Task = 0,		// can have at max 1
	Shared = 1,		// can have at max 1
	Quest = 2,		// can have at max 19 or 29 depending on client
	E = 3			// can have at max 19 or 29 depending on client, not present in live anymore
};

enum class DurationCode {
	None = 0,
	Short = 1,
	Medium = 2,
	Long = 3
};

// need to capture more, shared are just Radiant/Ebon though
enum class PointType {
	None = 0,
	Radiant = 4,
	Ebon = 5,
};

struct TaskInformation {
	TaskType type;
	int	Duration;
	DurationCode dur_code; // description for time investment for when Duration == 0
	std::string Title;			// max length 64
	std::string Description;	// max length 4000, 2048 on Tit
	std::string Reward;
	std::string item_link;		// max length 128 older clients, item link gets own string
	std::string completion_emote; // emote after completing task, yellow. Maybe should make more generic ... but yellow for now!
	int	RewardID;
	int	CashReward; // Expressed in copper
	int	XPReward;
	int faction_reward; // just a npc_faction_id
	TaskMethodType RewardMethod;
	int reward_points; // DoN crystals for shared. Generic "points" for non-shared
	PointType reward_type; // 4 for Radiant Crystals else Ebon crystals when shared task
	int	ActivityCount;
	SequenceType SequenceMode;
	int	LastStep;
	short	MinLevel;
	short	MaxLevel;
	bool	Repeatable;
	int replay_group; // ID of our replay timer group (0 means none)
	int min_players; // shared tasks
	int max_players;
	int task_lock_step; // task locks after this step is completed
	uint32 instance_zone_id; // instance shit
	uint32 zone_version;
	uint16 zone_in_zone_id;
	float zone_in_x;
	float zone_in_y;
	uint16 zone_in_object_id;
	float dest_x;
	float dest_y;
	float dest_z;
	float dest_h;
	/* int graveyard_zone_id;
	float graveyard_x;
	float graveyard_y;
	float graveyard_z;
	float graveyard_radius; */
	ActivityInformation Activity[MAXACTIVITIESPERTASK];
};

typedef enum { ActivityHidden = 0, ActivityActive = 1, ActivityCompleted = 2 } ActivityState;

typedef enum { ActivityDeliver = 1, ActivityKill = 2, ActivityLoot = 3, ActivitySpeakWith = 4, ActivityExplore = 5,
			ActivityTradeSkill = 6, ActivityFish = 7, ActivityForage = 8, ActivityCastOn = 9, ActivitySkillOn = 10,
			ActivityTouch = 11, ActivityCollect = 13, ActivityGiveCash = 100 } ActivityType;

struct ClientActivityInformation {
	int ActivityID;
	int DoneCount;
	ActivityState State;
	bool Updated; // Flag so we know if we need to update the database
};

struct ClientTaskInformation {
	int slot; // intrusive, but makes things easier :P
	int TaskID;
	int CurrentStep;
	int AcceptedTime;
	bool Updated;
	ClientActivityInformation Activity[MAXACTIVITIESPERTASK];
};

#endif /* !GLOBAL_TASKS_H */
