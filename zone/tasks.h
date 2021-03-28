#ifndef TASKS_H
#define TASKS_H

#include "../common/types.h"
#include <list>
#include <vector>
#include <string>
#include <algorithm>

#define MAXTASKS 10000
#define MAXTASKSETS 1000
#define MAXACTIVEQUESTS 19 // The Client has a hard cap of 19 active quests, 29 in SoD+
#define MAXCHOOSERENTRIES 40 // The Max Chooser (Task Selector entries) is capped at 40 in the Titanium Client.
#define MAXACTIVITIESPERTASK 20 // The Client has a hard cap of 20 activities per task.
#define TASKSLOTEMPTY 0 // This is used to determine if a client's active task slot is empty.

// Command Codes for worldserver ServerOP_ReloadTasks
#define RELOADTASKS 0
#define RELOADTASKGOALLISTS 1
#define RELOADTASKPROXIMITIES 2
#define RELOADTASKSETS 3

class Client;
class Mob;

namespace EQ {
	class ItemInstance;
}

typedef enum {
	METHODSINGLEID = 0,
	METHODLIST     = 1,
	METHODQUEST    = 2
} TaskMethodType;

struct ActivityInformation {
	int              step_number;
	int              activity_type;
	std::string      target_name; // name mob, location -- default empty
	std::string      item_list; // likely defaults to empty
	std::string      skill_list; // IDs ; separated -- default -1
	std::string      spell_list; // IDs ; separated -- default 0
	std::string      description_override; // overrides auto generated description -- default empty
	int              skill_id; // older clients, first id from above
	int              spell_id; // older clients, first id from above
	int              goal_id;
	TaskMethodType   goal_method;
	int              goal_count;
	int              deliver_to_npc;
	std::vector<int> zone_ids;
	std::string      zones; // IDs ; searated, ZoneID is the first in this list for older clients -- default empty string
	bool             optional;

	inline bool CheckZone(int zone_id)
	{
		if (zone_ids.empty()) {
			return true;
		}
		return std::find(zone_ids.begin(), zone_ids.end(), zone_id) != zone_ids.end();
	}
};

typedef enum {
	ActivitiesSequential = 0,
	ActivitiesStepped    = 1
} SequenceType;

enum class TaskType {
	Task   = 0,        // can have at max 1
	Shared = 1,        // can have at max 1
	Quest  = 2,        // can have at max 19 or 29 depending on client
	E      = 3         // can have at max 19 or 29 depending on client, not present in live anymore
};

enum class DurationCode {
	None   = 0,
	Short  = 1,
	Medium = 2,
	Long   = 3
};

struct TaskInformation {
	TaskType            type;
	int                 duration;
	DurationCode        duration_code;         // description for time investment for when duration == 0
	std::string         title;            // max length 64
	std::string         description;      // max length 4000, 2048 on Tit
	std::string         reward;
	std::string         item_link;        // max length 128 older clients, item link gets own string
	std::string         completion_emote; // emote after completing task, yellow. Maybe should make more generic ... but yellow for now!
	int                 reward_id;
	int                 cash_reward;       // Expressed in copper
	int                 experience_reward;
	int                 faction_reward;   // just a npc_faction_id
	TaskMethodType      reward_method;
	int                 activity_count;
	SequenceType        sequence_mode;
	int                 last_step;
	short               min_level;
	short               max_level;
	bool                repeatable;
	ActivityInformation activity_information[MAXACTIVITIESPERTASK];
};

typedef enum {
	ActivityHidden    = 0,
	ActivityActive    = 1,
	ActivityCompleted = 2
} ActivityState;

typedef enum {
	ActivityDeliver    = 1,
	ActivityKill       = 2,
	ActivityLoot       = 3,
	ActivitySpeakWith  = 4,
	ActivityExplore    = 5,
	ActivityTradeSkill = 6,
	ActivityFish       = 7,
	ActivityForage     = 8,
	ActivityCastOn     = 9,
	ActivitySkillOn    = 10,
	ActivityTouch      = 11,
	ActivityCollect    = 13,
	ActivityGiveCash   = 100
} ActivityType;

struct ClientActivityInformation {
	int           activity_id;
	int           done_count;
	ActivityState activity_state;
	bool          updated; // Flag so we know if we need to updated the database
};

struct ClientTaskInformation {
	int                       slot; // intrusive, but makes things easier :P
	int                       task_id;
	int                       current_step;
	int                       accepted_time;
	bool                      updated;
	ClientActivityInformation activity[MAXACTIVITIESPERTASK];
};

struct CompletedTaskInformation {
	int  task_id;
	int  completed_time;
	bool activity_done[MAXACTIVITIESPERTASK];
};

#endif
