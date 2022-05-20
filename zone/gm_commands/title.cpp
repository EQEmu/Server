#include "../client.h"
#include "../titles.h"

void command_title(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"Usage: #title [Remove|Title] [Save (0 = False, 1 = True)]"
		);
		return;
	}
	
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	std::string title = is_remove ? "" : sep->arg[1];
	bool save_title = sep->IsNumber(2) ? atobool(sep->arg[2]) : false;
		
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	if (title.size() > 31) {
		c->Message(Chat::White, "Title must be 31 characters or less.");
		return;
	}

	if (!title.empty()) {
		find_replace(title, "_", " ");
	}

	if (!save_title || is_remove) {
		target->SetAATitle(title.c_str());
	} else if (save_title) {
		title_manager.CreateNewPlayerTitle(target, title.c_str());
	}

	target->Save();

	c->Message(
		Chat::White,
		fmt::format(
			"Title has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove && save_title ? " and saved" : "",
			c->GetTargetDescription(target),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					title
				)
			)
		).c_str()
	);
}


