#include "../common/global_define.h"
#include "../common/rulesys.h"
#include "../common/strings.h"

#include "database.h"
#include "database/database_update.h"


// Disgrace: for windows compile
#ifdef _WINDOWS
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else

#include "unix.h"
#include <netinet/in.h>
#include <sys/time.h>

#endif

#pragma pack(1)

DatabaseUpdate database_update;

bool Database::CheckDatabaseConversions()
{
	auto *r = RuleManager::Instance();
	r->LoadRules(this, "default", false);
	if (!RuleB(Bots, Enabled) && DoesTableExist("bot_data")) {
		LogInfo("Bot tables found but rule not enabled, enabling");
		r->SetRule("Bots:Enabled", "true", this, true, true);
	}

	database_update.SetDatabase(this)->CheckDbUpdates();

	return true;
}
