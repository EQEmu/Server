#include "../client.h"
#include "../titles.h"

void command_title(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #title [Title] (use \"-1\" to remove title)");
		return;
	}
	
	bool is_remove = !strcasecmp(sep->argplus[1], "-1");
	std::string title = is_remove ? "" : sep->argplus[1];
		
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

	if (is_remove) {
		target->SetAATitle(title);
	} else {
		title_manager.CreateNewPlayerTitle(target, title);
	}

	target->Save();

	c->Message(
		Chat::White,
		fmt::format(
			"Title has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
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


