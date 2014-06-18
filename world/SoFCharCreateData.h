#ifndef SOFCHARCREATEDATA_H
#define SOFCHARCREATEDATA_H

#pragma pack(1)

struct RaceClassAllocation {
	unsigned int Index;
	unsigned int BaseStats[7];
	unsigned int DefaultPointAllocation[7];
};

struct RaceClassCombos {
	unsigned int ExpansionRequired;
	unsigned int Race;
	unsigned int Class;
	unsigned int Deity;
	unsigned int AllocationIndex;
	unsigned int Zone;
};

#pragma pack()

#endif
