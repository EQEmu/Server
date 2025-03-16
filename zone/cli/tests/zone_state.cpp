#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/platform.h"
#include "../../zone.h"
#include "../../client.h"
#include "../../common/net/eqstream.h"

extern Zone *zone;

inline void ClearState() {
	ZoneStateSpawnsRepository::DeleteWhere(database, "zone_id = 32 and instance_id = 0");
}

inline std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> GetStateSpawns() {
	return ZoneStateSpawnsRepository::GetWhere(database, "zone_id = 32 and instance_id = 0");
}

void ZoneCLI::TestZoneState(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	SetupZone("soldungb");

	// clean slate
	ClearState();

	std::cout << "===========================================\n";
	std::cout << "⚙\uFE0F> Running Zone State Tests... (soldungb)\n";
	std::cout << "===========================================\n\n";

	RunTest("Ensure no state spawns exist before shutdown", 0, (int) GetStateSpawns().size());

	zone->Shutdown();

	auto entries = GetStateSpawns().size();
	RunTest(fmt::format("State exists after shutdown, entries ({})", entries), true, entries > 0);

	SetupZone("soldungb");

	entries = GetStateSpawns().size();
	RunTest(fmt::format("State exists after bootup, entries ({})", entries), true, entries > 0);

	zone->Repop();

	entries = GetStateSpawns().size();
	RunTest(fmt::format("State does not exist after repop, entries ({})", entries), true, entries == 0);

	std::cout << "\n===========================================\n";
	std::cout << "✅ All Zone State Tests Completed!\n";
	std::cout << "===========================================\n";
}
