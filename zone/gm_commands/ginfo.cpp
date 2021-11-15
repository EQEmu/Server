#include "../client.h"
#include "../groups.h"

void command_ginfo(Client *c, const Seperator *sep)
{
	Client *t;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}
	else {
		t = c;
	}

	Group *g = t->GetGroup();
	if (!g) {
		c->Message(Chat::White, "This client is not in a group");
		return;
	}

	c->Message(
		Chat::White,
		"Player: %s is in Group #%lu: with %i members",
		t->GetName(),
		(unsigned long) g->GetID(),
		g->GroupCount());

	uint32 r;
	for (r = 0; r < MAX_GROUP_MEMBERS; r++) {
		if (g->members[r] == nullptr) {
			if (g->membername[r][0] == '\0') {
				continue;
			}
			c->Message(
				Chat::White, "...Zoned Member: %s, Roles: %s %s %s", g->membername[r],
				(g->MemberRoles[r] & RoleAssist) ? "Assist" : "",
				(g->MemberRoles[r] & RoleTank) ? "Tank" : "",
				(g->MemberRoles[r] & RolePuller) ? "Puller" : ""
			);
		}
		else {
			c->Message(
				Chat::White, "...In-Zone Member: %s (0x%x) Roles: %s %s %s", g->membername[r], g->members[r],
				(g->MemberRoles[r] & RoleAssist) ? "Assist" : "",
				(g->MemberRoles[r] & RoleTank) ? "Tank" : "",
				(g->MemberRoles[r] & RolePuller) ? "Puller" : ""
			);

		}
	}
}

