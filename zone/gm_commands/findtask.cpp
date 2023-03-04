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
			auto task_id      = Strings::ToUnsignedInt(sep->arg[1]);
			auto task_name    = task_manager->GetTaskName(task_id);

			std::string task_message = (
				!task_name.empty() ?
					fmt::format(
						"Task {}: {}",
						task_id,
						task_name
					) :
					fmt::format(
						"Task ID {} was not found.",
						task_id
					)
			);

			c->Message(
				Chat::White,
				task_message.c_str()
			);
		}
		else {
			std::string search_criteria = Strings::ToLower(sep->argplus[1]);
			if (!search_criteria.empty()) {
				int found_count = 0;
				for (const auto &task: task_manager->GetTaskData()) {
					auto        task_name       = task.second.title;
					std::string task_name_lower = Strings::ToLower(task_name);
					if (task_name_lower.find(search_criteria) == std::string::npos) {
						continue;
					}

					c->Message(
						Chat::White,
						fmt::format(
							"Task {}: {}",
							task.first,
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

