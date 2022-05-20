#include "../client.h"

void command_timers(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	std::vector<std::pair<pTimerType, PersistentTimer *>> timers;
	target->GetPTimers().ToVector(timers);

	std::string popup_title = fmt::format(
		"Recast Timers for {}",
		c->GetTargetDescription(target, TargetDescriptionType::UCSelf)
	);

	std::string popup_text = "<table>";

	popup_text += "<tr><td>Timer ID</td><td>Remaining</td></tr>";

	for (const auto& timer : timers) {
		auto remaining_time = timer.second->GetRemainingTime();
		if (remaining_time) {
			popup_text += fmt::format(
				"<tr><td>{}</td><td>{}</td></tr>",
				timer.first,
				ConvertSecondsToTime(remaining_time)
			);
		}
	}

	popup_text += "</table>";

	c->SendPopupToClient(
		popup_title.c_str(),
		popup_text.c_str()
	);
}

