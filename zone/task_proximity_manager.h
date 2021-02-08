#ifndef EQEMU_TASK_PROXIMITY_MANAGER_H
#define EQEMU_TASK_PROXIMITY_MANAGER_H

struct TaskProximity {
	int   explore_id;
	float min_x;
	float max_x;
	float min_y;
	float max_y;
	float min_z;
	float max_z;
};

// This class is used for managing proximities so that Quest NPC proximities don't need to be used.
class TaskProximityManager {

public:
	TaskProximityManager();
	~TaskProximityManager();
	bool LoadProximities(int zone_id);
	int CheckProximities(float x, float y, float z);

private:
	std::vector <TaskProximity> m_task_proximities;
};

#endif //EQEMU_TASK_PROXIMITY_MANAGER_H
