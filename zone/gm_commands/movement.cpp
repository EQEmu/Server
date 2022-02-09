#include "../client.h"
#include "../mob_movement_manager.h"

void command_movement(Client *c, const Seperator *sep)
{
	auto &mgr = MobMovementManager::Get();

	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #movement stats/clearstats/walkto/runto/rotateto/stop/packet");
		return;
	}

	if (strcasecmp(sep->arg[1], "stats") == 0) {
		mgr.DumpStats(c);
	}
	else if (strcasecmp(sep->arg[1], "clearstats") == 0) {
		mgr.ClearStats();
	}
	else if (strcasecmp(sep->arg[1], "walkto") == 0) {
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->WalkTo(c->GetX(), c->GetY(), c->GetZ());
	}
	else if (strcasecmp(sep->arg[1], "runto") == 0) {
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->RunTo(c->GetX(), c->GetY(), c->GetZ());
	}
	else if (strcasecmp(sep->arg[1], "rotateto") == 0) {
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->RotateToWalking(target->CalculateHeadingToTarget(c->GetX(), c->GetY()));
	}
	else if (strcasecmp(sep->arg[1], "stop") == 0) {
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->StopNavigation();
	}
	else if (strcasecmp(sep->arg[1], "packet") == 0) {
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		mgr.SendCommandToClients(
			target,
			atof(sep->arg[2]),
			atof(sep->arg[3]),
			atof(sep->arg[4]),
			atof(sep->arg[5]),
			atoi(sep->arg[6]),
			ClientRangeAny
		);
	}
	else {
		c->Message(Chat::White, "Usage: #movement stats/clearstats/walkto/runto/rotateto/stop/packet");
	}
}

