#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../../common/shared_tasks.h"

void command_task(Client *c, const Seperator *sep)
{
	//super-command for managing tasks
	if (sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
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

	if (!strcasecmp(sep->arg[1], "show")) {
		c->ShowClientTasks(client_target);
		return;
	}

	if (!strcasecmp(sep->arg[1], "purgetimers")) {
		c->Message(15, fmt::format("{}'s task timers have been purged", client_target->GetCleanName()).c_str());
		if (client_target != c) {
			client_target->Message(15, "[GM] Your task timers have been purged by a GM");
		}
		client_target->PurgeTaskTimers();
		return;
	}

	if (!strcasecmp(sep->arg[1], "update")) {
		if (sep->argnum >= 3) {
			int task_id     = atoi(sep->arg[2]);
			int activity_id = atoi(sep->arg[3]);
			int count       = 1;

			if (sep->argnum >= 4) {
				count = atoi(sep->arg[4]);
				if (count <= 0) {
					count = 1;
				}
			}
			c->Message(
				Chat::Yellow,
				"Updating Task [%i] Activity [%i] Count [%i] for client [%s]",
				task_id,
				activity_id,
				count,
				client_target->GetCleanName()
			);
			client_target->UpdateTaskActivity(task_id, activity_id, count);
			c->ShowClientTasks(client_target);
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "sharedpurge")) {
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
	}

	if (!strcasecmp(sep->arg[1], "assign")) {
		int task_id = atoi(sep->arg[2]);
		if ((task_id > 0) && (task_id < MAXTASKS)) {
			client_target->AssignTask(task_id, 0, false);
			c->Message(Chat::Yellow, "Assigned task [%i] to [%s]", task_id, client_target->GetCleanName());
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "reloadall")) {
		c->Message(Chat::Yellow, "Sending reloadtasks to world");
		worldserver.SendReloadTasks(RELOADTASKS);
		c->Message(Chat::Yellow, "Back again");
		return;
	}

	if (!strcasecmp(sep->arg[1], "reload")) {
		if (sep->arg[2][0] != '\0') {
			if (!strcasecmp(sep->arg[2], "lists")) {
				c->Message(Chat::Yellow, "Sending reload lists to world");
				worldserver.SendReloadTasks(RELOADTASKGOALLISTS);
				c->Message(Chat::Yellow, "Reloaded");
				return;
			}
			if (!strcasecmp(sep->arg[2], "prox")) {
				c->Message(Chat::Yellow, "Sending reload proximities to world");
				worldserver.SendReloadTasks(RELOADTASKPROXIMITIES);
				c->Message(Chat::Yellow, "Reloaded");
				return;
			}
			if (!strcasecmp(sep->arg[2], "sets")) {
				c->Message(Chat::Yellow, "Sending reload task sets to world");
				worldserver.SendReloadTasks(RELOADTASKSETS);
				c->Message(Chat::Yellow, "Reloaded");
				return;
			}
			if (!strcasecmp(sep->arg[2], "task") && (sep->arg[3][0] != '\0')) {
				int task_id = atoi(sep->arg[3]);
				if ((task_id > 0) && (task_id < MAXTASKS)) {
					c->Message(Chat::Yellow, "Sending reload task %i to world", task_id);
					worldserver.SendReloadTasks(RELOADTASKS, task_id);
					c->Message(Chat::Yellow, "Reloaded");
					return;
				}
			}
		}

	}
	c->Message(Chat::White, "Unable to interpret command. Type #task help");

}
