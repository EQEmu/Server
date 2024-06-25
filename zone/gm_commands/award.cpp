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

    std::string reason = sep->arg[3];

    c->Message(Chat::White, "Awarded %d EoM to %s for %s.", Strings::ToInt(sep->arg[2]), sep->arg[1], reason.c_str());
    zone->SendDiscordMessage("admin", fmt::to_string(c->GetCleanName()) + " awarded " + sep->arg[2] + " EoM to " + sep->arg[1] + " Reason: " + reason);

    quest_manager.WorldWideSignal(WWSignalUpdateType_Character, 666);
}
