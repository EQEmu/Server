#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/platform.h"
#include "../zone.h"
#include "../client.h"
#include "../../common/net/eqstream.h"

extern Zone *zone;

void ZoneCLI::NpcHandins(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	RegisterExecutablePlatform(EQEmuExePlatform::ExePlatformZoneSidecar);

	LogInfo("----------------------------------------");
	LogInfo("Booting test zone for NPC handins");
	LogInfo("----------------------------------------");

	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	entity_list.Process();
	entity_list.MobProcess();

	LogInfo("----------------------------------------");
	LogInfo("Done booting test zone");
	LogInfo("----------------------------------------");

	auto c = Client();
	auto npc_type = content_db.LoadNPCTypesData(754008);
	if (npc_type) {
		auto npc = new NPC(
			npc_type,
			nullptr,
			glm::vec4(0, 0, 0, 0),
			GravityBehavior::Water
		);

		entity_list.AddNPC(npc);

		LogInfo("Spawned NPC [{}]", npc->GetCleanName());

		LogInfo("Spawned client [{}]", c.GetCleanName());



	}
}
