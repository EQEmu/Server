#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../../common/shared_tasks.h"
#include "../../common/repositories/completed_tasks_repository.h"

void command_task(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Syntax: #task [subcommand]");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] List active tasks for a client",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task show", false, "show")
			).c_str()
		);
		c->Message(Chat::White, "--- update <task_id> <activity_id> [count] | Updates task");
		c->Message(Chat::White, "--- assign <task_id> | Assigns task to client");
		c->Message(Chat::White, "--- uncomplete <task_id> | Uncompletes a task if a client has completed it");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload all Task information from the database",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reloadall", false, "reloadall")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] <task_id> Reload Task and Activity information for a single task",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload task", false, "reload task")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload goal/reward list information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload lists", false, "reload lists")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload proximity information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload prox", false, "reload prox")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload task set information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload sets", false, "reload sets")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges targeted characters task timers",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task purgetimers", false, "purgetimers")
			).c_str()
		);

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Shared Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges all active Shared Tasks in memory and database ",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task sharedpurge", false, "sharedpurge")
			).c_str()
		);
		return;
	}

	Client *client_target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		client_target = c->GetTarget()->CastToClient();
	}

	bool is_assign = !strcasecmp(sep->arg[1], "assign");
	bool is_purgetimers = !strcasecmp(sep->arg[1], "purgetimers");
	bool is_reload = !strcasecmp(sep->arg[1], "reload");
	bool is_reloadall = !strcasecmp(sep->arg[1], "reloadall");
	bool is_sharedpurge = !strcasecmp(sep->arg[1], "sharedpurge");
	bool is_show = !strcasecmp(sep->arg[1], "show");
	bool is_uncomplete = !strcasecmp(sep->arg[1], "uncomplete");
	bool is_update = !strcasecmp(sep->arg[1], "update");

	if (
		!is_assign &&
		!is_purgetimers &&
		!is_reload &&
		!is_reloadall &&
		!is_sharedpurge &&
		!is_show &&
		!is_uncomplete &&
		!is_update
	) {
		c->Message(Chat::White, "Syntax: #task [subcommand]");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] List active tasks for a client",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task show", false, "show")
			).c_str()
		);
		c->Message(Chat::White, "--- update <task_id> <activity_id> [count] | Updates task");
		c->Message(Chat::White, "--- assign <task_id> | Assigns task to client");
		c->Message(Chat::White, "--- uncomplete <task_id> | Uncompletes a task if a client has completed it");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload all Task information from the database",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reloadall", false, "reloadall")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] <task_id> Reload Task and Activity information for a single task",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload task", false, "reload task")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload goal/reward list information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload lists", false, "reload lists")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload proximity information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload prox", false, "reload prox")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload task set information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload sets", false, "reload sets")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges targeted characters task timers",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task purgetimers", false, "purgetimers")
			).c_str()
		);

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Shared Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges all active Shared Tasks in memory and database ",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task sharedpurge", false, "sharedpurge")
			).c_str()
		);
		return;
	}

	if (is_assign) {
		auto task_id = std::stoul(sep->arg[2]);
		if (task_id && task_id < MAXTASKS) {
			client_target->AssignTask(task_id, 0, false);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Assigned task ID {} to {}.",
					task_id,
					(
						client_target == c ?
						"yourself" :
						fmt::format(
							"{} ({})",
							client_target->GetCleanName(),
							client_target->GetID()
						)
					)
				).c_str()
			);
		}
		return;
	} else if (is_purgetimers) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Task timers have been purged for {}.",
				(
					client_target == c ?
					"yourself" :
					fmt::format(
						"{} ({})",
						client_target->GetCleanName(),
						client_target->GetID()
					)
				)
			).c_str()
		);

		if (client_target != c) {
			client_target->Message(Chat::Yellow, "Your task timers have been purged by a GM.");
		}

		client_target->PurgeTaskTimers();
		return;
	} else if (is_reload) {
		if (arguments >= 2) {
			if (!strcasecmp(sep->arg[2], "lists")) {
				c->Message(Chat::Yellow, "Attempting to reload goal lists.");
				worldserver.SendReloadTasks(RELOADTASKGOALLISTS);
				c->Message(Chat::Yellow, "Successfully reloaded goal lists.");
				return;
			} else if (!strcasecmp(sep->arg[2], "prox")) {
				c->Message(Chat::Yellow, "Attempting to reload task proximites.");
				worldserver.SendReloadTasks(RELOADTASKPROXIMITIES);
				c->Message(Chat::Yellow, "Successfully reloaded task proximites.");
				return;
			} else if (!strcasecmp(sep->arg[2], "sets")) {
				c->Message(Chat::Yellow, "Attempting to reload task sets.");
				worldserver.SendReloadTasks(RELOADTASKSETS);
				c->Message(Chat::Yellow, "Successfully reloaded task sets.");
				return;
			} else if (!strcasecmp(sep->arg[2], "task") && arguments == 3) {
				int task_id = std::stoul(sep->arg[3]);
				if (task_id && task_id < MAXTASKS) {
					c->Message(
						Chat::Yellow,
						fmt::format(
							"Attempted to reload task ID {}.",
							task_id
						).c_str()
					);
					worldserver.SendReloadTasks(RELOADTASKS, task_id);
					c->Message(
						Chat::Yellow,
						fmt::format(
							"Successfully reloaded task ID {}.",
							task_id
						).c_str()
					);
					return;
				}
			}
		}
	} else if (is_reloadall) {
		c->Message(Chat::Yellow, "Attempting to reload tasks.");
		worldserver.SendReloadTasks(RELOADTASKS);
		c->Message(Chat::Yellow, "Successfully reloaded tasks.");
		return;
	} else if (is_sharedpurge) {
		if (!strcasecmp(sep->arg[2], "confirm")) {
			LogTasksDetail("Sending purge request");
			auto pack = new ServerPacket(ServerOP_SharedTaskPurgeAllCommand, 0);
			worldserver.SendPacket(pack);
			safe_delete(pack);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"[WARNING] This will purge all active Shared Tasks [{}]?",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task sharedpurge confirm", false, "confirm")
			).c_str()
		);

		return;
	} else if (is_show) {
		c->ShowClientTasks(client_target);
		return;
	} else if (is_uncomplete) {
		if (sep->IsNumber(2)) {
			auto task_id = std::stoul(sep->arg[2]);
			if (task_id && task_id < MAXTASKS) {
				if (
					CompletedTasksRepository::DeleteWhere(
						database,					
						fmt::format(
							"charid = {} AND taskid = {}",
							client_target->CharacterID(),
							task_id
						)
					)
				) {
					c->Message(
						Chat::Yellow,
						fmt::format(
							"Successfully uncompleted task ID {} for {}.",
							task_id,
							(
								client_target == c ?
								"yourself" :
								fmt::format(
									"{} ({})",
									client_target->GetCleanName(),
									client_target->GetID()
								)
							)
						).c_str()
					);
					return;
				} else {
					c->Message(
						Chat::Yellow,
						fmt::format(
							"{} not completed task ID {}.",
							(
								client_target == c ?
								"You have" :
								fmt::format(
									"{} ({}) has",
									client_target->GetCleanName(),
									client_target->GetID()
								)
							),
							task_id
						).c_str()
					);
					return;
				}
			} else {
				c->Message(Chat::White, "Invalid task ID specified.");
				return;
			}
		}
	} else if (is_update) {
		if (arguments >= 3) {
			auto task_id = std::stoul(sep->arg[2]);
			auto activity_id = std::stoul(sep->arg[3]);
			int count = 1;

			if (arguments >= 4) {
				count = std::stoi(sep->arg[4]);
				if (count <= 0) {
					count = 1;
				}
			}

			c->Message(
				Chat::Yellow,
				fmt::format(
					"Updating task ID {}, activity {} with a count of {} for {}.",
					task_id,
					activity_id,
					count,
					(
						client_target == c ?
						"yourself" :
						fmt::format(
							"{} ({})",
							client_target->GetCleanName(),
							client_target->GetID()
						)
					)
				).c_str()
			);

			client_target->UpdateTaskActivity(task_id, activity_id, count);
			c->ShowClientTasks(client_target);
		}
		return;
	}
}
