#ifdef _EQDEBUG
#ifndef DBMemLeak_H
#define DBMemLeak_H
#include "../common/types.h"
#include "../common/linked_list.h"

#define mysql_free_result(r)	{ DBMemLeak::Free(r); mysql_free_result(r); }

struct DBMemLeakStruct {
	const void* result;
	uint32 memblock;
	char query[0];
};

class DBMemLeak {
public:
	DBMemLeak();
	~DBMemLeak();

	static void Alloc(const void* result, const char* query);
	static void Free(const void* result);
};

#endif
#endif
