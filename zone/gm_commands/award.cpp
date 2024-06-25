#include "../client.h"
#include "../zone.h"
#include "../../common/repositories/character_data_repository.h"
#include "../questmgr.h"

void command_award(Client *c, const Seperator *sep)
{
    int arguments = sep->argnum;
    if (arguments < 4 || !sep->IsNumber(2)) {
        c->Message(Chat::White, "Usage: #award [Character Name] [Amount] [Reason]");
        return;
    }

    std::string character_name = Strings::Escape(sep->arg[1]);

    const auto& l = CharacterDataRepository::GetWhere(
        database,
        fmt::format(
            "`name` = '{}'",
            character_name
        )
    );
    
    if (l.empty()) {
        c->Message(Chat::White, "Unable to find character %s", character_name.c_str());
        return;
    }

    auto& e = l.front();
    
    DataBucketKey k;
    k.character_id = e.id;
    k.key = "EoM-Award";
    k.value = sep->arg[2];

    DataBucket::SetData(k);    

    // Join all arguments from sep->arg[3] onwards to form the reason string
    std::string reason;
    for (int i = 3; i < arguments; ++i) {
        if (i > 3) {
            reason += " ";
        }
        reason += sep->arg[i];
    }

    c->Message(Chat::White, "Awarded %d EoM to %s for %s.", Strings::ToInt(sep->arg[2]), character_name.c_str(), reason.c_str());
    zone->SendDiscordMessage("admin", fmt::to_string(c->GetCleanName()) + " awarded " + sep->arg[2] + " EoM to " + character_name + " for " + reason);

    quest_manager.WorldWideSignal(WWSignalUpdateType_Character, 666);
}
