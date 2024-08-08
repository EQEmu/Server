#include "../client.h"
#include "../zone.h"
#include "../../common/repositories/character_data_repository.h"
#include "../questmgr.h"

void command_award(Client *c, const Seperator *sep)
{
    int arguments = sep->argnum;
    if (arguments <= 3) {
        c->Message(Chat::White, "Insufficient Number of Arguments");
        c->Message(Chat::White, "Usage: #award [Character Name] [Amount] [Reason]");
        return;
    }

    std::string character_name = Strings::Escape(sep->arg[1]);

    if (!Strings::IsNumber(sep->arg[2])) {
        c->Message(Chat::White, "Specify an award amount.");
        c->Message(Chat::White, "Usage: #award [Character Name] [Amount] [Reason]");
    }

    const auto& l = CharacterDataRepository::GetWhere(
        database,
        fmt::format(
            "`name` = '{}'",
            character_name
        )
    );

    if (l.empty()) {
        c->Message(Chat::White, "Unable to find character %s", character_name.c_str());
        c->Message(Chat::White, "Usage: #award [Character Name] [Amount] [Reason]");
        return;
    }

    auto& e = l.front();

    // Join all arguments from sep->arg[3] onwards to form the reason string
    std::string reason;
    for (int i = 3; i <= arguments; i++) {
        if (i > 3) {
            reason += " ";
        }
        reason += sep->arg[i];
    }

    if (reason.empty()) {
        c->Message(Chat::White, "Reason is a required argument");
        c->Message(Chat::White, "Usage: #award [Character Name] [Amount] [Reason]");
        return;
    }

    DataBucketKey k;
    k.character_id = e.id;
    k.key = "EoM-Award";

	DataBucket::GetData(k);

	k.value += sep->arg[2];

    DataBucket::SetData(k);

    c->Message(Chat::White, "Awarded %d EoM to %s. Reason: %s", Strings::ToInt(sep->arg[2]), character_name.c_str(), reason.c_str());
    zone->SendDiscordMessage("admin", fmt::to_string(c->GetCleanName()) + " awarded " + sep->arg[2] + " EoM to " + character_name + " Reason: " + reason);

	quest_manager.CrossZoneSignal(CZUpdateType_Expedition, 0, 666, character_name.c_str());
}
