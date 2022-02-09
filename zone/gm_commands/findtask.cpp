#include "../client.h"

void command_findtask(Client *c, const Seperator *sep)
{
	if (RuleB(TaskSystem, EnableTaskSystem)) {
		int arguments = sep->argnum;

		if (arguments == 0) {
			c->Message(Chat::White, "Command Syntax: #findtask [search criteria]");
			return;
		}

		if (sep->IsNumber(1)) {
			auto task_id      = std::stoul(sep->arg[1]);
			auto task_name    = task_manager->GetTaskName(task_id);
			auto task_message = (
				!task_name.empty() ?
					fmt::format(
						"Task {}: {}",
						task_id,
						task_name
					).c_str() :
					fmt::format(
						"Task ID {} was not found.",
						task_id
					).c_str()
			);

			c->Message(
				Chat::White,
				task_message
			);
		}
		else {
			std::string search_criteria = str_tolower(sep->argplus[1]);
			if (!search_criteria.empty()) {
				int         found_count = 0;
				for (uint32 task_id     = 1; task_id <= MAXTASKS; task_id++) {
					auto        task_name       = task_manager->GetTaskName(task_id);
					std::string task_name_lower = str_tolower(task_name);
					if (task_name_lower.find(search_criteria) == std::string::npos) {
						continue;
					}

					c->Message(
						Chat::White,
						fmt::format(
							"Task {}: {}",
							task_id,
							task_name
						).c_str()
					);
					found_count++;

					if (found_count == 20) {
						break;
					}
				}

				if (found_count == 20) {
					c->Message(Chat::White, "20 Tasks were found, max reached.");
				}
				else {
					auto task_message = (
						found_count > 0 ?
							(
								found_count == 1 ?
									"A Task was" :
									fmt::format("{} Tasks were", found_count)
							) :
							"No Tasks were"
					);

					c->Message(
						Chat::White,
						fmt::format(
							"{} found.",
							task_message
						).c_str()
					);
				}
			}
		}
	}
	else {
		c->Message(Chat::White, "This command cannot be used while the Task system is disabled.");
	}
}

