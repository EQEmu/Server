#include "../common/types.h"
#include "../common/EMuShareMem.h"

struct MMFOpcodes_Struct {
	uint32		EQOpcodeCount;
	uint32		EmuOpcodeCount;
	uint16		eq_to_emu[0];
	//uint16	emu_to_eq[0];	//logical, not really here... EQOpcodeCount indexes in
};

