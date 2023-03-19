#include "../client.h"
#include "../dialogue_window.h"

void command_timers(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	std::vector<std::pair<pTimerType, PersistentTimer *>> l;
	t->GetPTimers().ToVector(l);

	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no recast timers.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has"
			).c_str()
		);
		return;
	}

	auto m = DialogueWindow::TableRow(
		DialogueWindow::TableCell("Timer ID") +
		DialogueWindow::TableCell("Remaining Time")
	);

	for (const auto& e : l) {
		auto r = e.second->GetRemainingTime();
		if (r) {
			m += DialogueWindow::TableRow(
				DialogueWindow::TableCell(std::to_string(e.first)) +
				DialogueWindow::TableCell(Strings::SecondsToTime(r))
			);
		}
	}

	c->SendPopupToClient(
		fmt::format(
			"Recast Timers for {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf)
		).c_str(),
		DialogueWindow::Table(m).c_str()
	);
}
