#ifndef EQEMU_BAZAAR_H
#define EQEMU_BAZAAR_H

#include <vector>
#include "shareddb.h"
#include "../../common/item_instance.h"

class Bazaar {
public:
	static std::vector<BazaarSearchResultsFromDB_Struct>
	GetSearchResults(Database &content_db, Database &db, BazaarSearchCriteria_Struct search, unsigned int char_zone_id, int char_zone_instance_id);

};


#endif //EQEMU_BAZAAR_H