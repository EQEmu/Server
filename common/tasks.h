#ifndef EQEMU_TASKS_H
#define EQEMU_TASKS_H

#include "serialize_buffer.h"

#define MAXTASKS 10000
#define MAXTASKSETS 1000
#define MAXACTIVEQUESTS 19 // The Client has a hard cap of 19 active quests, 29 in SoD+
#define MAXCHOOSERENTRIES 40 // The Max Chooser (Task Selector entries) is capped at 40 in the Titanium Client.
#define MAXACTIVITIESPERTASK 20 // The Client has a hard cap of 20 activities per task.
#define TASKSLOTEMPTY 0 // This is used to determine if a client's active task slot is empty.

#define TASKSLOTTASK 0
#define TASKSLOTSHAREDTASK 0

// Command Codes for worldserver ServerOP_ReloadTasks
#define RELOADTASKS 0
#define RELOADTASKGOALLISTS 1
#define RELOADTASKPROXIMITIES 2
#define RELOADTASKSETS 3

typedef enum {
	METHODSINGLEID = 0,
	METHODLIST     = 1,
	METHODQUEST    = 2
} TaskMethodType;

enum class TaskActivityType : int32_t // task element/objective
{
	Unknown    = -1, // hidden
	None       = 0,
	Deliver    = 1,
	Kill       = 2,
	Loot       = 3,
	SpeakWith  = 4,
	Explore    = 5,
	TradeSkill = 6,
	Fish       = 7,
	Forage     = 8,
	CastOn     = 9,
	SkillOn    = 10,
	Touch      = 11,
	Collect    = 13,
	GiveCash   = 100
};

enum class TaskTimerType
{
	Replay = 0,
	Request
};

struct ActivityInformation {
	int              step_number;
	TaskActivityType activity_type;
	std::string      target_name; // name mob, location -- default empty, max length 64
	std::string      item_list; // likely defaults to empty
	std::string      skill_list; // IDs ; separated -- default -1
	std::string      spell_list; // IDs ; separated -- default 0
	std::string      description_override; // overrides auto generated description -- default empty, max length 128
	int              skill_id; // older clients, first id from above
	int              spell_id; // older clients, first id from above
	int              goal_id;
	std::string      goal_match_list;
	TaskMethodType   goal_method;
	int              goal_count;
	int              deliver_to_npc;
	std::vector<int> zone_ids;
	std::string      zones; // IDs ; separated, ZoneID is the first in this list for older clients -- default empty string, max length 64
	bool             optional;

	inline bool CheckZone(int zone_id)
	{
		if (zone_ids.empty()) {
			return true;
		}
		return std::find(zone_ids.begin(), zone_ids.end(), zone_id) != zone_ids.end();
	}

	void SerializeSelector(SerializeBuffer& out, EQ::versions::ClientVersion client_version) const
	{
		out.WriteInt32(static_cast<int32_t>(activity_type));
		out.WriteInt32(0); // solo/group/raid request type? (no longer in live)
		out.WriteString(target_name); // target name used in objective type string (max 64)

		if (client_version >= EQ::versions::ClientVersion::RoF)
		{
			out.WriteLengthString(item_list);  // used in objective type string (can be empty for none)
			out.WriteInt32(activity_type == TaskActivityType::GiveCash ? 1 : goal_count);
			out.WriteLengthString(skill_list); // used in SkillOn objective type string, "-1" for none
			out.WriteLengthString(spell_list); // used in CastOn objective type string, "0" for none
			out.WriteString(zones);            // used in objective zone column and task select "begins in" (may have multiple, "0" for "unknown zone", empty for "ALL")
		}
		else
		{
			out.WriteString(item_list);
			out.WriteInt32(activity_type == TaskActivityType::GiveCash ? 1 : goal_count);
			out.WriteInt32(skill_id);
			out.WriteInt32(spell_id);
			out.WriteInt32(zone_ids.empty() ? 0 : zone_ids.front());
		}

		out.WriteString(description_override);

		if (client_version >= EQ::versions::ClientVersion::RoF) {
			out.WriteString(zones); // serialized again after description (seems unused)
		}
	}

	void SerializeObjective(SerializeBuffer& out, EQ::versions::ClientVersion client_version, int done_count) const
	{
		// cash objectives internally repurpose goal_count to store cash amount and
		// done_count as a boolean (should not be sent as actual completed/goal counts)
		int real_goal_count = goal_count;
		if (activity_type == TaskActivityType::GiveCash)
		{
			done_count = (done_count >= goal_count) ? 1 : 0;
			real_goal_count = 1;
		}

		out.WriteInt32(static_cast<int32_t>(activity_type));

		if (client_version >= EQ::versions::ClientVersion::RoF) {
			out.WriteInt8(optional ? 1 : 0);
		} else {
			out.WriteInt32(optional ? 1 : 0);
		}

		out.WriteInt32(0); // solo/group/raid request type? (no longer in live)
		out.WriteString(target_name); // target name used in objective type string (max 64)

		if (client_version >= EQ::versions::ClientVersion::RoF)
		{
			out.WriteLengthString(item_list);  // used in objective type string (can be empty for none)
			out.WriteInt32(real_goal_count);
			out.WriteLengthString(skill_list); // used in SkillOn objective type string, "-1" for none
			out.WriteLengthString(spell_list); // used in CastOn objective type string, "0" for none
			out.WriteString(zones);            // used in objective zone column and task select "begins in" ("0" for "unknown zone", empty for "ALL")
		}
		else
		{
			out.WriteString(item_list);
			out.WriteInt32(real_goal_count);
			out.WriteInt32(skill_id);
			out.WriteInt32(spell_id);
			out.WriteInt32(zone_ids.empty() ? 0 : zone_ids.front());
		}

		out.WriteInt32(0); // unknown id
		out.WriteString(description_override);
		out.WriteInt32(done_count);
		out.WriteInt8(1); // unknown

		if (client_version >= EQ::versions::ClientVersion::RoF)
		{
			out.WriteString(zones); // serialized again after description (seems unused)
		}
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

static const uint8 TASK_TYPE_TASK   = 0;
static const uint8 TASK_TYPE_SHARED = 1;
static const uint8 TASK_TYPE_QUEST  = 2;

enum class DurationCode {
	None   = 0,
	Short  = 1,
	Medium = 2,
	Long   = 3
};

struct TaskInformation {
	TaskType            type;
	int                 duration{};
	DurationCode        duration_code;         // description for time investment for when duration == 0
	std::string         title{};            // max length 64
	std::string         description{};      // max length 4000, 2048 on Tit
	std::string         reward{};
	std::string         item_link{};        // max length 128 older clients, item link gets own string
	std::string         completion_emote{}; // emote after completing task, yellow. Maybe should make more generic ... but yellow for now!
	int                 reward_id{};
	int                 cash_reward{};       // Expressed in copper
	int                 experience_reward{};
	int                 faction_reward{};   // just a npc_faction_id
	TaskMethodType      reward_method;
	int                 reward_radiant_crystals;
	int                 reward_ebon_crystals;
	int                 activity_count{};
	SequenceType        sequence_mode;
	int                 last_step{};
	short               min_level{};
	short               max_level{};
	int                 level_spread;
	int                 min_players;
	int                 max_players;
	bool                repeatable{};
	int                 replay_timer_seconds;
	int                 request_timer_seconds;
	ActivityInformation activity_information[MAXACTIVITIESPERTASK];

	void SerializeSelector(SerializeBuffer& out, EQ::versions::ClientVersion client_version) const
	{
		if (client_version != EQ::versions::ClientVersion::Titanium) {
			out.WriteFloat(1.0f); // reward multiplier (affects color, <1.0: yellow-red, 1.0: white, >1.0: lightgreen-green)
		}

		out.WriteUInt32(duration);    // task duration (seconds) (0: task_duration_code used, "Unlimited" on live)
		out.WriteUInt32(static_cast<int>(duration_code)); // 1: Short 2: Medium 3: Long anything else Unlimited (no longer in live)
		out.WriteString(title);       // max 64 with null
		out.WriteString(description); // max 4000 with null

		if (client_version != EQ::versions::ClientVersion::Titanium) {
			out.WriteUInt8(0); // 0: no rewards 1: enables "Reward Preview" button
		}

		// selector only needs to send the first objective to fill description starting zone
		out.WriteUInt32(std::min(activity_count, 1)); // number of task objectives
		if (activity_count > 0)
		{
			out.WriteUInt32(0); // objective index
			activity_information[0].SerializeSelector(out, client_version);
		}
	}
};

typedef enum {
	ActivityHidden    = 0,
	ActivityActive    = 1,
	ActivityCompleted = 2
}                  ActivityState;

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

namespace Tasks {

	inline int GetActivityStateIdentifier(ActivityState activity_state)
	{
		switch (activity_state) {
			case ActivityHidden:
				return 0;
			case ActivityActive:
				return 1;
			case ActivityCompleted:
				return 2;
			default:
				return 0;
		}
	}
	inline std::string GetActivityStateDescription(ActivityState activity_state)
	{
		switch (activity_state) {
			case ActivityHidden:
				return "Hidden";
			case ActivityActive:
				return "Active";
			case ActivityCompleted:
				return "Completed";
			default:
				return "Hidden";
		}
	}

	inline int GetTaskTypeIdentifier(TaskType task_type)
	{
		switch (task_type) {
			case TaskType::Task:
				return 0;
			case TaskType::Shared:
				return 1;
			case TaskType::Quest:
				return 2;
			case TaskType::E:
				return 3;
			default:
				return 0;
		}
	}
	inline std::string GetTaskTypeDescription(TaskType task_type)
	{
		switch (task_type) {
			case TaskType::Task:
				return "Task";
			case TaskType::Shared:
				return "Shared";
			case TaskType::Quest:
				return "Quest";
			case TaskType::E:
				return "E";
			default:
				return "Task";
		}
	}
}

namespace SharedTaskMessage {
	constexpr uint16 TASK_ASSIGN_WAIT_REPLAY_TIMER                     = 8017; // This task can not be assigned to you because you must wait %1d:%2h:%3m before you can do another task of this type.
	constexpr uint16 COULD_NOT_USE_COMMAND                             = 8272; // You could not use this command because you are not currently assigned to a shared task.
	constexpr uint16 AVG_LVL_LOW                                       = 8553; // You can not be assigned this shared task because your party's average level is too low.
	constexpr uint16 AVG_LVL_HIGH                                      = 8889; // You can not be assigned this shared task because your party's average level is too high.
	constexpr uint16 LVL_SPREAD_HIGH                                   = 8890; // You can not be assigned this shared task because your party's level spread is too high.
	constexpr uint16 PARTY_EXCEED_MAX_PLAYER                           = 8891; // You can not be assigned this shared task because your party exceeds the maximum allowed number of players.
	constexpr uint16 LEADER_NOT_MEET_REQUIREMENTS                      = 8892; // You can not be assigned this shared task because the leader does not meet the shared task requirements.
	constexpr uint16 SHARED_TASK_NOT_MEET_MIN_NUM_PLAYER               = 8895; // You can not be assigned this shared task because your party does not contain the minimum required number of players.
	constexpr uint16 WILL_REMOVE_ZONE_TWO_MIN_RAID_NOT_MIN_NUM_PLAYER  = 8908; // %1 will be removed from their zone in two minutes because your raid does not meet the minimum requirement of qualified players.
	constexpr uint16 WILL_REMOVE_ZONE_TWO_MIN_GROUP_NOT_MIN_NUM_PLAYER = 8909; // %1 will be removed from their zone in two minutes because your group does not meet the minimum requirement of qualified players.
	constexpr uint16 WILL_REMOVE_AREA_TWO_MIN_RAID_NOT_MIN_NUM_PLAYER  = 8910; // %1 will be removed from their area in two minutes because your raid does not meet the minimum requirement of qualified players.
	constexpr uint16 WILL_REMOVE_AREA_TWO_MIN_GROUP_NOT_MIN_NUM_PLAYER = 8911; // %1 will be removed from their area in two minutes because your group does not meet the minimum requirement of qualified players.
	constexpr uint16 HAS_REMOVED_ZONE_TWO_MIN_RAID_NOT_MIN_NUM_PLAYER  = 8912; // %1 has been removed from their zone because your raid does not meet the minimum requirement of qualified players.
	constexpr uint16 HAS_REMOVED_ZONE_TWO_MIN_GROUP_NOT_MIN_NUM_PLAYER = 8913; // %1 has been removed from their zone because your group does not meet the minimum requirement of qualified players.
	constexpr uint16 HAS_REMOVED_AREA_TWO_MIN_RAID_NOT_MIN_NUM_PLAYER  = 8914; // %1 has been removed from their area because your raid does not meet the minimum requirement of qualified players.
	constexpr uint16 HAS_REMOVED_AREA_TWO_MIN_GROUP_NOT_MIN_NUM_PLAYER = 8915; // %1 has been removed from their area because your group does not meet the minimum requirement of qualified players.
	constexpr uint16 SEND_INVITE_TO                                    = 8916; // Sending a shared task invitation to %1.
	constexpr uint16 COULD_NOT_BE_INVITED                              = 8917; // %1 could not be invited to join you.
	constexpr uint16 YOU_ARE_NOT_LEADER_COMMAND_ISSUE                  = 8919; // You are not the shared task leader.  Only %1 can issue this command.
	constexpr uint16 SWAP_SENDING_INVITATION_TO                        = 8920; // Sending an invitation to: %1.  They must accept in order to swap party members.
	constexpr uint16 SWAP_ACCEPTED_OFFER                               = 8921; // %1 has accepted your offer to join your shared task.  Swapping %1 for %2.
	constexpr uint16 IS_NOT_MEMBER                                     = 8922; // %1 is not a member of this shared task.
	constexpr uint16 NOT_ALLOW_PLAYER_REMOVE                           = 8923; // The shared task is not allowing players to be removed from it at this time.
	constexpr uint16 PLAYER_HAS_BEEN_REMOVED                           = 8924; // %1 has been removed from your shared task, '%2'.
	constexpr uint16 TRANSFER_LEADERSHIP_NOT_ONLINE                    = 8925; // %1 is not currently online.  You can only transfer leadership to an online member of the shared task.
	constexpr uint16 MADE_LEADER                                       = 8926; // %1 has been made the leader for this shared task.
	constexpr uint16 YOU_MADE_LEADER                                   = 8927; // You have been made the leader of this shared task.
	constexpr uint16 LEADER_PRINT                                      = 8928; // Shared Task Leader: %1
	constexpr uint16 MEMBERS_PRINT                                     = 8929; // Shared Task Members: %1
	constexpr uint16 PLAYER_ACCEPTED_OFFER_JOIN                        = 8930; // %1 has accepted your offer to join your shared task.
	constexpr uint16 PLAYER_HAS_BEEN_ADDED                             = 8931; // %1 has been added to your shared task, '%2'.
	constexpr uint16 ACCEPTED_OFFER_TO_JOIN_BUT_COULD_NOT              = 8932; // %1 accepted your offer to join your shared task but could not.
	constexpr uint16 PLAYER_DECLINED_OFFER                             = 8933; // %1 has declined your offer to join your shared task.
	constexpr uint16 PLAYER_HAS_ASKED_YOU_TO_JOIN                      = 8934; // %1 has asked you to join the shared task '%2'.        Would you like to join?
	constexpr uint16 NO_REQUEST_BECAUSE_HAVE_ONE                       = 8935; // You may not request a shared task because you already have one.
	constexpr uint16 NO_REQUEST_BECAUSE_RAID_HAS_ONE                   = 8936; // You may not request a shared task because someone in your raid, %1, already has one.
	constexpr uint16 NO_REQUEST_BECAUSE_GROUP_HAS_ONE                  = 8937; // You may not request a shared task because someone in your group, %1, already has one.
	constexpr uint16 YOU_DO_NOT_MEET_REQ_AVAILABLE                     = 8938; // You do not meet the requirements for any available shared tasks.
	constexpr uint16 YOUR_RAID_DOES_NOT_MEET_REQ                       = 8939; // Your raid does not meet the requirements for any available shared tasks.
	constexpr uint16 YOUR_GROUP_DOES_NOT_MEET_REQ                      = 8940; // Your group does not meet the requirements for any available shared tasks.
	constexpr uint16 YOUR_GROUP__RAID_DOES_NOT_MEET_REQ                = 8941; // You can not be assigned this shared task because the raid or group does not meet the shared task requirements.
	constexpr uint16 YOU_NO_LONGER_MEMBER                              = 8942; // You are no longer a member of the shared task.
	constexpr uint16 YOU_MAY_NOT_REQUEST_EXPANSION                     = 8943; // You may not request this shared task because you do not have the required expansion.
	constexpr uint16 PLAYER_MAY_NOT_REQUEST_EXPANSION                  = 8944; // You may not request this shared task because %1 does not have the required expansion.
	constexpr uint16 TWO_MIN_REQ_TASK_TERMINATED                       = 8945; // If your party does not meet the requirements in two minutes, the shared task will be terminated.
	constexpr uint16 YOU_MUST_WAIT_REPLAY_TIMER                        = 8946; // You may not request this shared task because you must wait %1d:%2h:%3m before you can do another task of this type.
	constexpr uint16 PLAYER_MUST_WAIT_REPLAY_TIMER                     = 8947; // You may not request this shared task because %1 must wait %2d:%3h:%4m before they can do another task of this type.
	constexpr uint16 PLAYER_NOW_LEADER                                 = 8948; // %1 is now the leader of your shared task, '%2'.
	constexpr uint16 HAS_ENDED                                         = 8951; // Your shared task, '%1', has ended.
	constexpr uint16 YOU_ALREADY_LEADER                                = 8952; // You are already the leader of the shared task.
	constexpr uint16 TASK_NO_LONGER_ACTIVE                             = 8953; // Your shared task, '%1', is no longer active.
	constexpr uint16 YOU_HAVE_BEEN_ADDED_TO_TASK                       = 8954; // You have been added to the shared task '%1'.
	constexpr uint16 YOU_ARE_NOW_LEADER                                = 8955; // You are now the leader of your shared task, '%1'.
	constexpr uint16 YOU_HAVE_BEEN_REMOVED                             = 8956; // You have been removed from the shared task '%1'.
	constexpr uint16 YOU_ARE_NO_LONGER_A_MEMBER                        = 8960; // You are no longer a member of the shared task, '%1'.
	constexpr uint16 YOUR_TASK_NOW_LOCKED                              = 8961; // Your shared task is now locked.  You may no longer add or remove players.
	constexpr uint16 TASK_NOT_ALLOWING_PLAYERS_AT_TIME                 = 8962; // The shared task is not allowing players to be added at this time.
	constexpr uint16 PLAYER_NOT_ONLINE_TO_ADD                          = 8963; // %1 is not currently online.  A player needs to be online to be added to a shared task.
	constexpr uint16 CANT_ADD_PLAYER_ALREADY_MEMBER                    = 8964; // You can not add %1 because they are already a member of this shared task.
	constexpr uint16 CANT_ADD_PLAYER_ALREADY_ASSIGNED                  = 8965; // You can not add %1 because they are already assigned to another shared task.
	constexpr uint16 PLAYER_ALREADY_OUTSTANDING_INVITATION_THIS        = 8966; // %1 already has an outstanding invitation to join this shared task.
	constexpr uint16 PLAYER_ALREADY_OUTSTANDING_ANOTHER                = 8967; // %1 already has an outstanding invitation to join another shared task.  Players may only have one invitation outstanding.
	constexpr uint16 CANT_ADD_PLAYER_MAX_PLAYERS                       = 8968; // You can not add another player since you currently have the maximum number of players allowed (%1) in this shared task.
	constexpr uint16 CANT_ADD_PLAYER_MAX_LEVEL_SPREAD                  = 8969; // You can not add this player because you would exceed the maximum level spread (%1) for this shared task.
	constexpr uint16 CANT_ADD_PLAYER_MAX_AVERAGE_LEVEL                 = 8970; // You can not add this player because you would exceed the maximum average level for this shared task.
	constexpr uint16 CANT_ADD_PLAYER_FALL_MIN_AVG_LEVEL                = 8971; // You can not add this player because you would fall below the minimum average level for this shared task.
	constexpr uint16 PLAYER_DOES_NOT_OWN_EXPANSION                     = 8972; // %1 does not own the expansion needed for this shared task.
	constexpr uint16 CANT_ADD_PLAYER_PARTY_FILTER_REQ_FOR_TASK         = 8973; // You can not add this player because your party would no longer meet the filter requirements for this shared task.
	constexpr uint16 CANT_ADD_PLAYER_ONE_OF_GROUP_RAID_HAS_TASK        = 8977; // You can not add %1 because they or one of their group or raid members is in another shared task.
	constexpr uint16 CANT_JOIN_GROUP_ACTIVE_TASK                       = 8978; // You can not join that group because you have an active shared task.
	constexpr uint16 CANT_ADD_PLAYER_REPLAY_TIMER                      = 8979; // You may not add %1 because they must wait %2d:%3h:%4m before they can do another task of this type.
	constexpr uint16 CANT_LOOT_BECAUSE_TASK_LOCKED_BELONG              = 8980; // You may not loot that corpse because you are not in the shared task the corpse belongs to.
	constexpr uint16 CANT_ADD_PLAYER_BECAUSE_GROUP_RAID_BELONG_TASK    = 8981; // The player could not be added to the raid because they or one of their group members is in a different shared task.
	constexpr uint16 PLAYER_CANT_ADD_GROUP_BECAUSE_DIFF_TASK           = 8982; // %1 can not be added to the group because they are in a different shared task.
	constexpr uint16 YOU_CANT_ADD_TO_GROUP_BECAUSE_DIFF_TASK           = 8983; // You can not be added to the group because you are in a different shared task.
	constexpr uint16 PLAYER_CANT_ADD_RAID_BECAUSE_DIFF_TASK            = 8984; // %1 can not be added to the raid because they are in a different shared task.
	constexpr uint16 YOU_CANT_ADD_RAID_BECAUSE_DIFF_TASK               = 8985; // You can not be added to the raid because you are in a different shared task.
	constexpr uint16 REPLAY_TIMER_REMAINING                            = 8987; // '%1' replay timer:  %2d:%3h:%4m remaining.
	constexpr uint16 YOU_NO_CURRENT_REPLAY_TIMERS                      = 8989; // You do not currently have any task replay timers.
	constexpr uint16 SURE_QUIT_TASK                                    = 8995; // Are you sure you want to quit the task '%1'?
	constexpr uint16 SURE_REMOVE_SELF_FROM_TASK                        = 8996; // Are you sure you want to remove yourself from the shared task '%1'
	constexpr uint16 TASK_ASSIGN_WAIT_REQUEST_TIMER                    = 14506; // This task can not be assigned to you because you must wait %1d:%2h:%3m before you can request another task of this type.
	constexpr uint16 REQUEST_TIMER_REMAINING                           = 14507; // '%1' request timer:  %2d:%3h:%4m remaining.
	constexpr uint16 YOU_MUST_WAIT_REQUEST_TIMER                       = 14508; // You may not request this shared task because you must wait %1d:%2h:%3m before you can request another task of this type.
	constexpr uint16 RECEIVED_REQUEST_TIMER                            = 14509; // You have received a request timer for '%1': %2d:%3h:%4m remaining.
	constexpr uint16 RECEIVED_REPLAY_TIMER                             = 14510; // You have received a replay timer for '%1': %2d:%3h:%4m remaining.
	constexpr uint16 PLAYER_MUST_WAIT_REQUEST_TIMER                    = 14511; // You may not request this shared task because %1 must wait %2d:%3h:%4m before they can request another task of this type.
	constexpr uint16 CANT_ADD_PLAYER_REQUEST_TIMER                     = 14512; // You may not add %1 because they must wait %2d:%3h:%4m before they can request another task of this type.

	// for eqstrs not in current emu clients (some are also used by non-shared tasks)
	constexpr auto GetEQStr(uint16 eqstr_id)
	{
		switch (eqstr_id)
		{
		case SharedTaskMessage::COULD_NOT_USE_COMMAND:
			return "You could not use this command because you are not currently assigned to a shared task.";
		case SharedTaskMessage::TASK_ASSIGN_WAIT_REQUEST_TIMER:
			return "This task can not be assigned to you because you must wait {}d:{}h:{}m before you can request another task of this type.";
		case SharedTaskMessage::REQUEST_TIMER_REMAINING:
			return "'{}' request timer:  {}d:{}h:{}m remaining.";
		case SharedTaskMessage::YOU_MUST_WAIT_REQUEST_TIMER:
			return "You may not request this shared task because you must wait {}d:{}h:{}m before you can request another task of this type.";
		case SharedTaskMessage::RECEIVED_REQUEST_TIMER:
			return "You have received a request timer for '{}': {}d:{}h:{}m remaining.";
		case SharedTaskMessage::RECEIVED_REPLAY_TIMER:
			return "You have received a replay timer for '{}': {}d:{}h:{}m remaining.";
		case SharedTaskMessage::PLAYER_MUST_WAIT_REQUEST_TIMER:
			return "You may not request this shared task because {} must wait {}d:{}h:{}m before they can request another task of this type.";
		case SharedTaskMessage::CANT_ADD_PLAYER_REQUEST_TIMER:
			return "You may not add {} because they must wait {}d:{}h:{}m before they can request another task of this type.";
		default:
			LogTasks("[GetEQStr] Unhandled eqstr id [{}]", eqstr_id);
			break;
		}
		return "Unknown EQStr";
	}
}

#endif //EQEMU_TASKS_H
