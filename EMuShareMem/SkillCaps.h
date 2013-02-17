#include "../common/types.h"
#include "../common/EMuShareMem.h"

struct MMFSkillCaps_Struct {
	uint8		ClassCount;
	uint8		SkillCount;
	uint8		LevelCount;
	uint16		caps[0];
};

