#pragma once

struct CharCreatePointAllocation
{
	unsigned int Index;
	unsigned int BaseStats[7];
	unsigned int DefaultPointAllocation[7];
	unsigned int BaseResists[7];
};

struct CharCreateCombination {
	unsigned int ExpansionRequired;
	unsigned int Race;
	unsigned int Class;
	unsigned int Deity;
	unsigned int AllocationIndex;
	unsigned int Zone;
};
