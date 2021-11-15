#include "../client.h"

void command_logcommand(Client *c, const char *message)
{
	int admin = c->Admin();

	bool continueevents = false;
	switch (zone->loglevelvar) { //catch failsafe
		case 9: { // log only LeadGM
			if (
				admin >= AccountStatus::GMLeadAdmin &&
				admin < AccountStatus::GMMgmt
				) {
				continueevents = true;
			}
			break;
		}
		case 8: { // log only GM
			if (
				admin >= AccountStatus::GMAdmin &&
				admin < AccountStatus::GMLeadAdmin
				) {
				continueevents = true;
			}
			break;
		}
		case 1: {
			if (admin >= AccountStatus::GMMgmt) {
				continueevents = true;
			}
			break;
		}
		case 2: {
			if (admin >= AccountStatus::GMLeadAdmin) {
				continueevents = true;
			}
			break;
		}
		case 3: {
			if (admin >= AccountStatus::GMAdmin) {
				continueevents = true;
			}
			break;
		}
		case 4: {
			if (admin >= AccountStatus::QuestTroupe) {
				continueevents = true;
			}
			break;
		}
		case 5: {
			if (admin >= AccountStatus::ApprenticeGuide) {
				continueevents = true;
			}
			break;
		}
		case 6: {
			if (admin >= AccountStatus::Steward) {
				continueevents = true;
			}
			break;
		}
		case 7: {
			continueevents = true;
			break;
		}
	}

	if (continueevents) {
		database.logevents(
			c->AccountName(),
			c->AccountID(),
			admin, c->GetName(),
			c->GetTarget() ? c->GetTarget()->GetName() : "None",
			"Command",
			message,
			1
		);
	}
}


/*
 * commands go below here
 */
