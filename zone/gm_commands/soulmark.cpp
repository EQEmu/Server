#include "../client.h"
#include "../zone.h"
#include "../../common/repositories/character_data_repository.h"
#include "../questmgr.h"

void command_soulmark(Client *c, const Seperator *sep)
{
    int arguments = sep->argnum;
    if (arguments < 4) {  // Adjusted to expect a reason
        c->Message(Chat::White, "Usage: #soulmark [Add/Remove] [Character] [Reason]");
        return;
    }

    std::string action = sep->arg[1];
    std::string character_name = sep->arg[2];
    std::string reason;

    // Concatenate all arguments from sep->arg[3] onwards to form the reason string
    for (int i = 3; i < arguments; ++i) {
        if (i > 3) {
            reason += " ";
        }
        reason += sep->arg[i];
    }

    if (action != "Add" && action != "Remove") {
        c->Message(Chat::White, "Usage: #soulmark [Add/Remove] [Character] [Reason]");
        return;
    }

    const auto& l = CharacterDataRepository::GetWhere(
        database,
        fmt::format(
            "`name` = '{}'",
            Strings::Escape(character_name)
        )
    );

    if (l.empty()) {
        c->Message(Chat::White, "Unable to find character %s", Strings::Escape(character_name).c_str());
        return;
    }

    auto& e = l.front();
    auto account_id = e.account_id;

    std::string key = fmt::format("{}-CheaterFlag", account_id);

    if (action == "Add") {
        DataBucketKey k;
        k.key = key;
        k.value = reason;  // Set the flag with the reason

        DataBucket::SetData(k);
        c->Message(Chat::White, "Soulmark added to %s (AccountID: %d) with reason: %s.", character_name.c_str(), account_id, reason.c_str());
    } else if (action == "Remove") {
        std::string existing_flag = DataBucket::GetData(key);
        if (!existing_flag.empty()) {
            DataBucketKey k;
            k.key = key;

            DataBucket::DeleteData(k);
            c->Message(Chat::White, "Soulmark removed from %s (AccountID: %d).", character_name.c_str(), account_id);
        } else {
            c->Message(Chat::White, "Soulmark not found for %s (AccountID: %d) to remove.", character_name.c_str(), account_id);
            return;
        }
    }

    zone->SendDiscordMessage("admin", fmt::to_string(c->GetCleanName()) + " " + action + "ed Soulmark for " + character_name + " (AccountID: " + std::to_string(account_id) + "). Reason: " + reason);
}
