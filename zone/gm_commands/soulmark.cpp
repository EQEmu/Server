#include "../client.h"
#include "../zone.h"
#include "../../common/repositories/character_data_repository.h"
#include "../questmgr.h"
#include <algorithm> // For std::transform

void command_soulmark(Client *c, const Seperator *sep)
{
    int arguments = sep->argnum;
    if (arguments < 2) {  // Expecting at least 3 arguments to include reason
        c->Message(Chat::White, "Insufficient Number of Arguments");
        c->Message(Chat::White, "Usage: #soulmark [Add/Remove] [Character] [Reason]");
        return;
    }

    std::string action = sep->arg[1];
    std::string character_name = sep->arg[2];

    // Convert action to lowercase to make the comparison case-insensitive
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);

    // Check if action is valid
    if (action != "add" && action != "remove") {
        c->Message(Chat::White, "Invalid Action Verb");
        c->Message(Chat::White, "Usage: #soulmark [Add/Remove] [Character] [Reason]");
        return;
    }

    if (action == "add" && arguments <= 2) {
        c->Message(Chat::White, "Insufficient Number of Arguments");
        c->Message(Chat::White, "Usage: #soulmark [Add/Remove] [Character] [Reason]");
        return;
    }

    // Concatenate all arguments from sep->arg[3] onwards to form the reason string
    std::string reason;
    for (int i = 3; i <= arguments; i++) {
        if (i > 3) {
            reason += " ";
        }
        reason += sep->arg[i];
    }

    if (action == "add" && reason.empty()) {
        c->Message(Chat::White, "Reason is a required argument");
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

    if (action == "add") {
        DataBucketKey k;
        k.key = key;
        k.value = reason;  // Set the flag with the reason

        DataBucket::SetData(k);
        c->Message(Chat::White, "Soulmark added to %s (AccountID: %d) Reason: %s.", character_name.c_str(), account_id, reason.c_str());
    } else if (action == "remove") {
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
