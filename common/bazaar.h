#ifndef EQEMU_BAZAAR_H
#define EQEMU_BAZAAR_H

#include <vector>
#include "shareddb.h"

class Bazaar {
public:
	static std::vector<BazaarSearchResultsFromDB_Struct>
	GetSearchResults(SharedDatabase &db, BazaarSearchCriteria_Struct search, unsigned int char_zone_id, int char_zone_instance_id);
};


#endif //EQEMU_BAZAAR_H
