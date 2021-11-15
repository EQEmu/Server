#include "../client.h"
#include "../fastmath.h"

extern FastMath g_Math;

void command_push(Client *c, const Seperator *sep)
{
	Mob *t = c;
	if (c->GetTarget() != nullptr) {
		t = c->GetTarget();
	}

	if (!sep->arg[1] || !sep->IsNumber(1)) {
		c->Message(Chat::White, "ERROR: Must provide at least a push back.");
		return;
	}

	float back = atof(sep->arg[1]);
	float up   = 0.0f;

	if (sep->arg[2] && sep->IsNumber(2)) {
		up = atof(sep->arg[2]);
	}

	if (t->IsNPC()) {
		t->IncDeltaX(back * g_Math.FastSin(c->GetHeading()));
		t->IncDeltaY(back * g_Math.FastCos(c->GetHeading()));
		t->IncDeltaZ(up);
		t->SetForcedMovement(6);
	}
	else if (t->IsClient()) {
		// TODO: send packet to push
	}
}

