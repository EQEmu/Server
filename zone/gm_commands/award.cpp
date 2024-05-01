#include "../client.h"
#include "../zone.h"
#include "../../common/repositories/character_data_repository.h"
#include "../questmgr.h"

void command_award(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #award [Character Name] [Amount]");
		return;
	}

	const auto& l = CharacterDataRepository::GetWhere(
		database,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(sep->arg[1])
		)
	);

	if (l.empty()) {
		c->Message(Chat::White, "Unable to find character %s", Strings::Escape(sep->arg[1]));
		return;
	}

	auto& e = l.front();
	
	DataBucketKey k;
	k.character_id = e.id;
	k.key = "EoM-Award";
	k.value = sep->arg[2];

	DataBucket::SetData(k);	

	c->Message(Chat::White, "Awarded %d EoM to %s.", Strings::ToInt(sep->arg[2]), sep->arg[1]);
	zone->SendDiscordMessage("admin", fmt::to_string(c->GetCleanName()) + " awarded " + sep->arg[2] + " EoM to " + sep->arg[1] + ".");

	quest_manager.WorldWideSignal(WWSignalUpdateType_Character, 666);
}
