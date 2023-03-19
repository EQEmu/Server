#include "../client.h"

void command_logcommand(Client *c, std::string message)
{
	int admin = c->Admin();

	bool log = false;
	switch (zone->loglevelvar) { //catch failsafe
		case 9: { // log only LeadGM
			if (
				admin >= AccountStatus::GMLeadAdmin &&
				admin < AccountStatus::GMMgmt
			) {
				log = true;
			}

			break;
		}
		case 8: { // log only GM
			if (
				admin >= AccountStatus::GMAdmin &&
				admin < AccountStatus::GMLeadAdmin
			) {
				log = true;
			}

			break;
		}
		case 1: {
			if (admin >= AccountStatus::GMMgmt) {
				log = true;
			}

			break;
		}
		case 2: {
			if (admin >= AccountStatus::GMLeadAdmin) {
				log = true;
			}

			break;
		}
		case 3: {
			if (admin >= AccountStatus::GMAdmin) {
				log = true;
			}

			break;
		}
		case 4: {
			if (admin >= AccountStatus::QuestTroupe) {
				log = true;
			}

			break;
		}
		case 5: {
			if (admin >= AccountStatus::ApprenticeGuide) {
				log = true;
			}

			break;
		}
		case 6: {
			if (admin >= AccountStatus::Steward) {
				log = true;
			}

			break;
		}
		case 7: {
			log = true;
			break;
		}
	}

	if (log) {
		database.logevents(
			c->AccountName(),
			c->AccountID(),
			admin,
			c->GetName(),
			c->GetTarget() ? c->GetTarget()->GetName() : "None",
			"Command",
			message.c_str(),
			1
		);
	}
}
