#include "../common/global_define.h"
#include "../common/repositories/proximities_repository.h"
#include "../common/rulesys.h"
#include "client.h"
#include "mob.h"
#include "quest_parser_collection.h"
#include "task_proximity_manager.h"
#include "tasks.h"
#include "zonedb.h"

TaskProximityManager::TaskProximityManager()
{


}

TaskProximityManager::~TaskProximityManager()
{


}

bool TaskProximityManager::LoadProximities(int zone_id)
{
	TaskProximity proximity{};
	m_task_proximities.clear();

	auto proximities = ProximitiesRepository::GetWhere(
		content_db,
		fmt::format("zoneid = {} ORDER BY `zoneid` ASC", zone_id)
	);

	for (auto &row: proximities) {
		proximity.explore_id = row.exploreid;
		proximity.min_x      = row.minx;
		proximity.max_x      = row.maxx;
		proximity.min_y      = row.miny;
		proximity.max_y      = row.maxy;
		proximity.min_z      = row.minz;
		proximity.max_z      = row.maxz;

		m_task_proximities.push_back(proximity);
	}

	LogTasks("Loaded [{}] Task Proximities", proximities.size());

	return true;
}

int TaskProximityManager::CheckProximities(float x, float y, float z)
{
	for (auto &task_proximity : m_task_proximities) {

		TaskProximity *p_proximity = &task_proximity;

		Log(
			Logs::General,
			Logs::Tasks,
			"[Proximity] Checking %8.3f, %8.3f, %8.3f against %8.3f, %8.3f, %8.3f, %8.3f, %8.3f, %8.3f",
			x,
			y,
			z,
			p_proximity->min_x,
			p_proximity->max_x,
			p_proximity->min_y,
			p_proximity->max_y,
			p_proximity->min_z,
			p_proximity->max_z
		);

		if (x < p_proximity->min_x || x > p_proximity->max_x || y < p_proximity->min_y || y > p_proximity->max_y ||
			z < p_proximity->min_z || z > p_proximity->max_z) {
			continue;
		}

		return p_proximity->explore_id;
	}

	return 0;
}
