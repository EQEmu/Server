#include "../client.h"

void command_appearanceeffects(Client *c, const Seperator *sep)
{
	Mob* m_target = c->GetTarget();
	if (m_target){
		m_target->GetAppearenceEffects();
	}
}
