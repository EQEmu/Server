#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include "client.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "tasks.h"
#include "zonedb.h"
#include "../common/repositories/character_task_timers_repository.h"

extern QueryServ *QServ;

void Client::LoadClientTaskState()
{
	if (RuleB(TaskSystem, EnableTaskSystem) && task_manager) {
		if (task_state) {
			safe_delete(task_state);
		}

		task_state = new ClientTaskState();
		if (!task_manager->LoadClientState(this, task_state)) {
			safe_delete(task_state);
		}
		else {
			task_manager->SendActiveTasksToClient(this);
			task_manager->SendCompletedTasksToClient(this, task_state);
		}
	}
}

void Client::RemoveClientTaskState()
{
	if (task_state) {
		task_state->CancelAllTasks(this);
		safe_delete(task_state);
	}
}

#if 0
void Client::SendTaskComplete(int TaskIndex) {

	// 0x4c8c

	TaskComplete_Struct* tcs;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskComplete, sizeof(TaskComplete_Struct));

	tcs = (TaskComplete_Struct*)outapp->pBuffer;

	// I have seen unknown0 as non-zero. It always seems to match the value in the first word of the
	// Task activity_information Complete packet sent immediately prior to it.
	//tcs->unknown00 = 0x00000000;
	tcs->unknown00 = TaskIndex;
	// I have only seen 0x00000002 in the next field. This is a common 'unknown' value in the task packets.
	// I suspect this is the type field to indicate this is a quest task, as opposed to other types.
	tcs->unknown04 = 0x00000002;

	Log.LogDebugType(Logs::Detail, Logs::Tasks, "SendTasksComplete");
	DumpPacket(outapp); fflush(stdout);

	QueuePacket(outapp);
	safe_delete(outapp);



}
#endif

void Client::SendTaskActivityComplete(
	int task_id,
	int activity_id,
	int task_index,
	TaskType task_type,
	int task_incomplete
)
{
	TaskActivityComplete_Struct *task_activity_complete;

	auto outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	task_activity_complete = (TaskActivityComplete_Struct *) outapp->pBuffer;

	task_activity_complete->TaskIndex      = task_index;
	task_activity_complete->TaskType       = static_cast<uint32>(task_type);
	task_activity_complete->TaskID         = task_id;
	task_activity_complete->ActivityID     = activity_id;
	task_activity_complete->task_completed = 0x00000001;
	task_activity_complete->stage_complete = task_incomplete;

	QueuePacket(outapp);
	safe_delete(outapp);
}


void Client::SendTaskFailed(int task_id, int task_index, TaskType task_type)
{
	// 0x54eb
	std::string export_string = fmt::format("{}", task_id);
	parse->EventPlayer(EVENT_TASK_FAIL, this, export_string, 0);

	TaskActivityComplete_Struct *task_activity_complete;

	auto outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	task_activity_complete = (TaskActivityComplete_Struct *) outapp->pBuffer;
	task_activity_complete->TaskIndex      = task_index;
	task_activity_complete->TaskType       = static_cast<uint32>(task_type);
	task_activity_complete->TaskID         = task_id;
	task_activity_complete->ActivityID     = 0;
	task_activity_complete->task_completed = 0; //Fail
	task_activity_complete->stage_complete = 0; // 0 for task complete or failed.

	LogTasks("Sending failure to client [{}]", GetCleanName());

	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::HasTaskRequestCooldownTimer()
{
	if (task_request_timer.Check(false))
	{
		task_request_timer.Disable();
	}

	return (!GetGM() && task_request_timer.Enabled());
}

void Client::SendTaskRequestCooldownTimerMessage()
{
	if (HasTaskRequestCooldownTimer())
	{
		uint32_t seconds = task_request_timer.GetRemainingTime() / 1000;
		MessageString(Chat::Yellow, TASK_REQUEST_COOLDOWN_TIMER,
			".", ".", // args start at %3 for this eqstr
			GetName(),
			fmt::format_int(seconds / 60).c_str(), // minutes
			fmt::format_int(seconds % 60).c_str()  // seconds
		);
	}
}

void Client::StartTaskRequestCooldownTimer()
{
	uint32_t milliseconds = RuleI(TaskSystem, RequestCooldownTimerSeconds) * 1000;
	task_request_timer.Start(milliseconds);

	uint32_t size = sizeof(uint32_t);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_TaskRequestTimer, size);
	outapp->WriteUInt32(milliseconds);
	QueuePacket(outapp.get());
}

void Client::PurgeTaskTimers()
{
	CharacterTaskTimersRepository::DeleteWhere(database, fmt::format("character_id = {}", CharacterID()));
}
