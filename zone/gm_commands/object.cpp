#include "../client.h"
#include "../object.h"
#include "../doors.h"

void command_object(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	} // Crash Suppressant: No client. How did we get here?

	// Save it here. We sometimes have need to refer to it in multiple places.
	const char *usage_string = "Usage: #object List|Add|Edit|Move|Rotate|Save|Copy|Delete|Undo";

	if ((!sep) || (sep->argnum == 0)) {
		c->Message(Chat::White, usage_string);
		return;
	}

	Object              *o       = nullptr;
	Object_Struct       od;
	Door_Struct         *ds;
	uint32              id       = 0;
	uint32              itemid   = 0;
	uint32              icon     = 0;
	uint32              instance = 0;
	uint32              newid    = 0;
	uint16              radius;
	EQApplicationPacket *app;

	bool bNewObject = false;

	float x2;
	float y2;

	// Temporary object type for static objects to allow manipulation
	// NOTE: Zone::LoadZoneObjects() currently loads this as an uint8, so max value is 255!
	static const uint32 staticType = 255;

	// Case insensitive commands (List == list == LIST)
	strlwr(sep->arg[1]);

	if (strcasecmp(sep->arg[1], "list") == 0) {
		// Insufficient or invalid args
		if ((sep->argnum < 2) || (sep->arg[2][0] < '0') ||
			((sep->arg[2][0] > '9') && ((sep->arg[2][0] & 0xDF) != 'A'))) {
			c->Message(Chat::White, "Usage: #object List All|(radius)");
			return;
		}

		if ((sep->arg[2][0] & 0xDF) == 'A') {
			radius = 0; // List All
		}
		else if ((radius = atoi(sep->arg[2])) <= 0) {
			radius = 500;
		} // Invalid radius. Default to 500 units.

		if (radius == 0)
			c->Message(Chat::White, "Objects within this zone:");
		else
			c->Message(Chat::White, "Objects within %u units of your current location:", radius);

		std::string query;
		if (radius)
			query = StringFormat(
				"SELECT id, xpos, ypos, zpos, heading, itemid, "
				"objectname, type, icon, unknown08, unknown10, unknown20 "
				"FROM object WHERE zoneid = %u AND version = %u "
				"AND (xpos BETWEEN %.1f AND %.1f) "
				"AND (ypos BETWEEN %.1f AND %.1f) "
				"AND (zpos BETWEEN %.1f AND %.1f) "
				"ORDER BY id",
				zone->GetZoneID(), zone->GetInstanceVersion(),
				c->GetX() - radius, // Yes, we're actually using a bounding box instead of a radius.
				c->GetX() + radius, // Much less processing power used this way.
				c->GetY() - radius, c->GetY() + radius, c->GetZ() - radius, c->GetZ() + radius
			);
		else
			query = StringFormat(
				"SELECT id, xpos, ypos, zpos, heading, itemid, "
				"objectname, type, icon, unknown08, unknown10, unknown20 "
				"FROM object WHERE zoneid = %u AND version = %u "
				"ORDER BY id",
				zone->GetZoneID(), zone->GetInstanceVersion());

		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Error in objects query");
			return;
		}

		for (auto row = results.begin(); row != results.end(); ++row) {
			id = atoi(row[0]);
			od.x       = atof(row[1]);
			od.y       = atof(row[2]);
			od.z       = atof(row[3]);
			od.heading = atof(row[4]);
			itemid = atoi(row[5]);
			strn0cpy(od.object_name, row[6], sizeof(od.object_name));
			od.object_name[sizeof(od.object_name) - 1] =
				'\0'; // Required if strlen(row[col++]) exactly == sizeof(object_name)

			od.object_type = atoi(row[7]);
			icon = atoi(row[8]);
			od.size       = atoi(row[9]);
			od.solidtype  = atoi(row[10]);
			od.unknown020 = atoi(row[11]);

			switch (od.object_type) {
				case 0:                // Static Object
				case staticType:        // Static Object unlocked for changes
					if (od.size == 0) // Unknown08 field is optional Size parameter for static objects
						od.size = 100; // Static object default Size is 100%

					c->Message(
						Chat::White, "- STATIC Object (%s): id %u, x %.1f, y %.1f, z %.1f, h %.1f, model %s, "
									 "size %u, solidtype %u, incline %u",
						(od.object_type == 0) ? "locked" : "unlocked", id, od.x, od.y, od.z,
						od.heading, od.object_name, od.size, od.solidtype, od.unknown020
					);
					break;

				case OT_DROPPEDITEM: // Ground Spawn
					c->Message(
						Chat::White, "- TEMPORARY Object: id %u, x %.1f, y %.1f, z %.1f, h %.1f, itemid %u, "
									 "model %s, icon %u",
						id, od.x, od.y, od.z, od.heading, itemid, od.object_name, icon
					);
					break;

				default: // All others == Tradeskill Objects
					c->Message(
						Chat::White, "- TRADESKILL Object: id %u, x %.1f, y %.1f, z %.1f, h %.1f, model %s, "
									 "type %u, icon %u",
						id, od.x, od.y, od.z, od.heading, od.object_name, od.object_type, icon
					);
					break;
			}
		}

		c->Message(Chat::White, "%u object%s found", results.RowCount(), (results.RowCount() == 1) ? "" : "s");
		return;
	}

	if (strcasecmp(sep->arg[1], "add") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 3) ||
			((sep->arg[3][0] == '\0') && (sep->arg[4][0] < '0') && (sep->arg[4][0] > '9'))) {
			c->Message(
				Chat::White, "Usage: (Static Object): #object Add [ObjectID] 0 Model [SizePercent] "
							 "[SolidType] [Incline]"
			);
			c->Message(Chat::White, "Usage: (Tradeskill Object): #object Add [ObjectID] TypeNum Model Icon");
			c->Message(
				Chat::White, "- Notes: Model must start with a letter, max length 16. SolidTypes = 0 (Solid), "
							 "1 (Sometimes Non-Solid)"
			);
			return;
		}

		int col;

		if (sep->argnum > 3) { // Model name in arg3?
			if ((sep->arg[3][0] <= '9') && (sep->arg[3][0] >= '0')) {
				// Nope, user must have specified ObjectID. Extract it.
				id  = atoi(sep->arg[2]);
				col = 1; // Bump all other arguments one to the right. Model is in arg4.
			}
			else {
				// Yep, arg3 is non-numeric, ObjectID must be omitted and model must be arg3
				id  = 0;
				col = 0;
			}
		}
		else {
			// Nope, only 3 args. Object ID must be omitted and arg3 must be model.
			id  = 0;
			col = 0;
		}

		memset(&od, 0, sizeof(od));

		od.object_type = atoi(sep->arg[2 + col]);

		switch (od.object_type) {
			case 0: // Static Object
				if ((sep->argnum - col) > 3) {
					od.size = atoi(sep->arg[4 + col]); // Size specified

					if ((sep->argnum - col) > 4) {
						od.solidtype = atoi(sep->arg[5 + col]); // SolidType specified

						if ((sep->argnum - col) > 5) {
							od.unknown020 = atoi(sep->arg[6 + col]);
						} // Incline specified
					}
				}
				break;

			case 1: // Ground Spawn
				c->Message(
					Chat::White, "ERROR: Object Type 1 is used for temporarily spawned ground spawns and dropped "
								 "items, which are not supported with #object. See the 'ground_spawns' table in "
								 "the database."
				);
				return;

			default: // Everything else == Tradeskill Object
				icon = ((sep->argnum - col) > 3) ? atoi(sep->arg[4 + col]) : 0;

				if (icon == 0) {
					c->Message(Chat::White, "ERROR: Required property 'Icon' not specified for Tradeskill Object");
					return;
				}

				break;
		}

		od.x       = c->GetX();
		od.y       = c->GetY();
		od.z       = c->GetZ() - (c->GetSize() * 0.625f);
		od.heading = c->GetHeading();

		std::string query;
		if (id) {
			// ID specified. Verify that it doesn't already exist.
			query        = StringFormat("SELECT COUNT(*) FROM object WHERE ID = %u", id);
			auto results = content_db.QueryDatabase(query);
			if (results.Success() && results.RowCount() != 0) {
				auto row = results.begin();
				if (atoi(row[0]) > 0) { // Yep, in database already.
					id = 0;
				}
			}

			// Not in database. Already spawned, just not saved?
			// Yep, already spawned.
			if (id && entity_list.FindObject(id)) {
				id = 0;
			}

			if (id == 0) {
				c->Message(Chat::White, "ERROR: An object already exists with the id %u", atoi(sep->arg[2]));
				return;
			}
		}

		int objectsFound = 0;
		// Verify no other objects already in this spot (accidental double-click of Hotkey?)
		query = StringFormat(
			"SELECT COUNT(*) FROM object WHERE zoneid = %u "
			"AND version=%u AND (xpos BETWEEN %.1f AND %.1f) "
			"AND (ypos BETWEEN %.1f AND %.1f) "
			"AND (zpos BETWEEN %.1f AND %.1f)",
			zone->GetZoneID(), zone->GetInstanceVersion(), od.x - 0.2f,
			od.x + 0.2f,           // Yes, we're actually using a bounding box instead of a radius.
			od.y - 0.2f, od.y + 0.2f,  // Much less processing power used this way.
			od.z - 0.2f, od.z + 0.2f
		); // It's pretty forgiving, though, allowing for close-proximity objects

		auto results = content_db.QueryDatabase(query);
		if (results.Success() && results.RowCount() != 0) {
			auto row = results.begin();
			objectsFound = atoi(row[0]); // Number of nearby objects from database
		}

		// No objects found in database too close. How about spawned but not yet saved?
		if (objectsFound == 0 && entity_list.FindNearbyObject(od.x, od.y, od.z, 0.2f)) {
			objectsFound = 1;
		}

		if (objectsFound) {
			c->Message(Chat::White, "ERROR: Object already at this location.");
			return;
		}

		// Strip any single quotes from objectname (SQL injection FTL!)
		strn0cpy(od.object_name, sep->arg[3 + col], sizeof(od.object_name));

		uint32 len = strlen(od.object_name);
		for (col = 0; col < (uint32) len; col++) {
			if (od.object_name[col] != '\'') {
				continue;
			}

			// Uh oh, 1337 h4x0r monkeying around! Strip that apostrophe!
			memcpy(&od.object_name[col], &od.object_name[col + 1], len - col);
			len--;
			col--;
		}

		strupr(od.object_name); // Model names are always upper-case.

		if ((od.object_name[0] < 'A') || (od.object_name[0] > 'Z')) {
			c->Message(Chat::White, "ERROR: Model name must start with a letter.");
			return;
		}

		if (id == 0) {
			// No ID specified. Get a best-guess next number from the database
			// If there's a problem retrieving an ID from the database, it'll end up being object # 1. No
			// biggie.

			query   = "SELECT MAX(id) FROM object";
			results = content_db.QueryDatabase(query);
			if (results.Success() && results.RowCount() != 0) {
				auto row = results.begin();
				id       = atoi(row[0]);
			}

			id++;
		}

		// Make sure not to overwrite already-spawned objects that haven't been saved yet.
		while (o = entity_list.FindObject(id))
			id++;

		// Static object
		if (od.object_type == 0) {
			od.object_type = staticType;
		} // Temporary. We'll make it 0 when we Save

		od.zone_id       = zone->GetZoneID();
		od.zone_instance = zone->GetInstanceVersion();

		o = new Object(id, od.object_type, icon, od, nullptr);

		// Add to our zone entity list and spawn immediately for all clients
		entity_list.AddObject(o, true);

		// Bump player back to avoid getting stuck inside new object

		x2 = 10.0f * sin(c->GetHeading() / 256.0f * 3.14159265f);
		y2 = 10.0f * cos(c->GetHeading() / 256.0f * 3.14159265f);
		c->MovePC(c->GetX() - x2, c->GetY() - y2, c->GetZ(), c->GetHeading());

		c->Message(
			Chat::White, "Spawning object with tentative id %u at location (%.1f, %.1f, %.1f heading %.1f). Use "
						 "'#object Save' to save to database when satisfied with placement.",
			id, od.x, od.y, od.z, od.heading
		);

		// Temporary Static Object
		if (od.object_type == staticType)
			c->Message(
				Chat::White, "- Note: Static Object will act like a tradeskill container and will not reflect "
							 "size, solidtype, or incline values until you commit with '#object Save', after "
							 "which it will be unchangeable until you use '#object Edit' and zone back in."
			);

		return;
	}

	if (strcasecmp(sep->arg[1], "edit") == 0) {

		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) < 1)) {
			c->Message(Chat::White, "Usage: #object Edit (ObjectID) [PropertyName] [NewValue]");
			c->Message(Chat::White, "- Static Object (Type 0) Properties: model, type, size, solidtype, incline");
			c->Message(Chat::White, "- Tradeskill Object (Type 2+) Properties: model, type, icon");

			return;
		}

		o = entity_list.FindObject(id);

		// Object already available in-zone?
		if (o) {
			// Yep, looks like we can make real-time changes.
			if (sep->argnum < 4) {
				// Or not. '#object Edit (ObjectID)' called without PropertyName and NewValue
				c->Message(Chat::White, "Note: Object %u already unlocked and ready for changes", id);
				return;
			}
		}
		else {
			// Object not found in-zone in a modifiable form. Check for valid matching circumstances.
			std::string query   = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
			auto        results = content_db.QueryDatabase(query);
			if (!results.Success() || results.RowCount() == 0) {
				c->Message(Chat::White, "ERROR: Object %u not found", id);
				return;
			}

			auto row = results.begin();
			od.zone_id       = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type   = atoi(row[2]);
			uint32 objectsFound = 1;

			// Object not in this zone?
			if (od.zone_id != zone->GetZoneID()) {
				c->Message(Chat::White, "ERROR: Object %u not in this zone.", id);
				return;
			}

			// Object not in this instance?
			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(Chat::White, "ERROR: Object %u not part of this instance version.", id);
				return;
			}

			switch (od.object_type) {
				case 0: // Static object needing unlocking
					// Convert to tradeskill object temporarily for changes
					query = StringFormat("UPDATE object SET type = %u WHERE id = %u", staticType, id);

					content_db.QueryDatabase(query);

					c->Message(
						Chat::White, "Static Object %u unlocked for editing. You must zone out and back in to "
									 "make your changes, then commit them with '#object Save'.",
						id
					);
					if (sep->argnum >= 4) {
						c->Message(
							Chat::White, "NOTE: The change you specified has not been applied, since the "
										 "static object had not been unlocked for editing yet."
						);
					}
					return;

				case OT_DROPPEDITEM:
					c->Message(
						Chat::White, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, "
									 "which cannot be manipulated with #object. See the 'ground_spawns' table "
									 "in the database.",
						id
					);
					return;

				case staticType:
					c->Message(
						Chat::White, "ERROR: Object %u has been unlocked for editing, but you must zone out "
									 "and back in for your client to refresh its object table before you can "
									 "make changes to it.",
						id
					);
					return;

				default:
					// Unknown error preventing us from seeing the object in the zone.
					c->Message(Chat::White, "ERROR: Unknown problem attempting to manipulate object %u", id);
					return;
			}
		}

		// If we're here, we have a manipulable object ready for changes.
		strlwr(sep->arg[3]); // Case insensitive PropertyName
		strupr(sep->arg[4]); // In case it's model name, which should always be upper-case

		// Read current object info for reference
		icon = o->GetIcon();
		o->GetObjectData(&od);

		// We'll be a little more picky with property names, to prevent errors. Check against the whole word.
		if (strcmp(sep->arg[3], "model") == 0) {

			if ((sep->arg[4][0] < 'A') || (sep->arg[4][0] > 'Z')) {
				c->Message(Chat::White, "ERROR: Model names must begin with a letter.");
				return;
			}

			strn0cpy(od.object_name, sep->arg[4], sizeof(od.object_name));

			o->SetObjectData(&od);

			c->Message(Chat::White, "Object %u now being rendered with model '%s'", id, od.object_name);
		}
		else if (strcmp(sep->arg[3], "type") == 0) {
			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(Chat::White, "ERROR: Invalid type number");
				return;
			}

			od.object_type = atoi(sep->arg[4]);

			switch (od.object_type) {
				case 0:
					// Convert Static Object to temporary changeable type
					od.object_type = staticType;
					c->Message(
						Chat::White, "Note: Static Object will still act like tradeskill object and will not "
									 "reflect size, solidtype, or incline settings until committed to the "
									 "database with '#object Save', after which it will be unchangeable until "
									 "it is unlocked again with '#object Edit'."
					);
					break;

				case OT_DROPPEDITEM:
					c->Message(
						Chat::White, "ERROR: Object Type 1 is used for temporarily spawned ground spawns and "
									 "dropped items, which are not supported with #object. See the "
									 "'ground_spawns' table in the database."
					);
					return;

				default:
					c->Message(Chat::White, "Object %u changed to Tradeskill Object Type %u", id, od.object_type);
					break;
			}

			o->SetType(od.object_type);
		}
		else if (strcmp(sep->arg[3], "size") == 0) {
			if (od.object_type != staticType) {
				c->Message(
					0, "ERROR: Object %u is not a Static Object and does not support the Size property",
					id
				);
				return;
			}

			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(Chat::White, "ERROR: Invalid size specified. Please enter a number.");
				return;
			}

			od.size = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			if (od.size == 0) { // 0 == unspecified == 100%
				od.size = 100;
			}

			c->Message(
				Chat::White, "Static Object %u set to %u%% size. Size will take effect when you commit to the "
							 "database with '#object Save', after which the object will be unchangeable until "
							 "you unlock it again with '#object Edit' and zone out and back in.",
				id, od.size
			);
		}
		else if (strcmp(sep->arg[3], "solidtype") == 0) {

			if (od.object_type != staticType) {
				c->Message(
					Chat::White, "ERROR: Object %u is not a Static Object and does not support the "
								 "SolidType property",
					id
				);
				return;
			}

			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(Chat::White, "ERROR: Invalid solidtype specified. Please enter a number.");
				return;
			}

			od.solidtype = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			c->Message(
				Chat::White, "Static Object %u set to SolidType %u. Change will take effect when you commit "
							 "to the database with '#object Save'. Support for this property is on a "
							 "per-model basis, mostly seen in smaller objects such as chests and tables.",
				id, od.solidtype
			);
		}
		else if (strcmp(sep->arg[3], "icon") == 0) {

			if ((od.object_type < 2) || (od.object_type == staticType)) {
				c->Message(
					Chat::White, "ERROR: Object %u is not a Tradeskill Object and does not support the "
								 "Icon property",
					id
				);
				return;
			}

			if ((icon = atoi(sep->arg[4])) == 0) {
				c->Message(Chat::White, "ERROR: Invalid Icon specified. Please enter an icon number.");
				return;
			}

			o->SetIcon(icon);
			c->Message(Chat::White, "Tradeskill Object %u icon set to %u", id, icon);
		}
		else if (strcmp(sep->arg[3], "incline") == 0) {
			if (od.object_type != staticType) {
				c->Message(
					0,
					"ERROR: Object %u is not a Static Object and does not support the Incline property",
					id
				);
				return;
			}

			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(
					0,
					"ERROR: Invalid Incline specified. Please enter a number. Normal range is 0-512."
				);
				return;
			}

			od.unknown020 = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			c->Message(
				Chat::White, "Static Object %u set to %u incline. Incline will take effect when you commit to "
							 "the database with '#object Save', after which the object will be unchangeable "
							 "until you unlock it again with '#object Edit' and zone out and back in.",
				id, od.unknown020
			);
		}
		else {
			c->Message(Chat::White, "ERROR: Unrecognized property name: %s", sep->arg[3]);
			return;
		}

		// Repop object to have it reflect the change.
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	}

	if (strcasecmp(sep->arg[1], "move") == 0) {

		if ((sep->argnum < 2) ||           // Not enough arguments
			((id = atoi(sep->arg[2])) == 0) || // ID not specified
			(((sep->arg[3][0] < '0') || (sep->arg[3][0] > '9')) && ((sep->arg[3][0] & 0xDF) != 'T') &&
			 (sep->arg[3][0] != '-') && (sep->arg[3][0] != '.'))) { // Location argument not specified correctly
			c->Message(Chat::White, "Usage: #object Move (ObjectID) ToMe|(x y z [h])");
			return;
		}

		if (!(o = entity_list.FindObject(id))) {
			std::string query   = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
			auto        results = content_db.QueryDatabase(query);
			if (!results.Success() || results.RowCount() == 0) {
				c->Message(Chat::White, "ERROR: Object %u not found", id);
				return;
			}

			auto row = results.begin();
			od.zone_id       = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type   = atoi(row[2]);

			if (od.zone_id != zone->GetZoneID()) {
				c->Message(Chat::White, "ERROR: Object %u is not in this zone", id);
				return;
			}

			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(Chat::White, "ERROR: Object %u is not in this instance version", id);
				return;
			}

			switch (od.object_type) {
				case 0:
					c->Message(
						Chat::White, "ERROR: Object %u is not yet unlocked for editing. Use '#object Edit' "
									 "then zone out and back in to move it.",
						id
					);
					return;

				case staticType:
					c->Message(
						Chat::White, "ERROR: Object %u has been unlocked for editing, but you must zone out "
									 "and back in before your client sees the change and will allow you to "
									 "move it.",
						id
					);
					return;

				case 1:
					c->Message(
						Chat::White, "ERROR: Object %u is a temporary spawned object and cannot be "
									 "manipulated with #object. See the 'ground_spawns' table in the "
									 "database.",
						id
					);
					return;

				default:
					c->Message(Chat::White, "ERROR: Object %u not located in zone.", id);
					return;
			}
		}

		// Move To Me
		if ((sep->arg[3][0] & 0xDF) == 'T') {
			od.x = c->GetX();
			od.y = c->GetY();
			od.z = c->GetZ() -
				   (c->GetSize() *
					0.625f); // Compensate for #loc bumping up Z coordinate by 62.5% of character's size.

			o->SetHeading(c->GetHeading());

			// Bump player back to avoid getting stuck inside object

			x2 = 10.0f * std::sin(c->GetHeading() / 256.0f * 3.14159265f);
			y2 = 10.0f * std::cos(c->GetHeading() / 256.0f * 3.14159265f);
			c->MovePC(c->GetX() - x2, c->GetY() - y2, c->GetZ(), c->GetHeading());
		} // Move to x, y, z [h]
		else {
			od.x = atof(sep->arg[3]);
			if (sep->argnum > 3) {
				od.y = atof(sep->arg[4]);
			}
			else {
				o->GetLocation(nullptr, &od.y, nullptr);
			}

			if (sep->argnum > 4) {
				od.z = atof(sep->arg[5]);
			}
			else {
				o->GetLocation(nullptr, nullptr, &od.z);
			}

			if (sep->argnum > 5) {
				o->SetHeading(atof(sep->arg[6]));
			}
		}

		o->SetLocation(od.x, od.y, od.z);

		// Despawn and respawn object to reflect change
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	}

	if (strcasecmp(sep->arg[1], "rotate") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 3) || ((id = atoi(sep->arg[2])) == 0)) {
			c->Message(Chat::White, "Usage: #object Rotate (ObjectID) (Heading, 0-512)");
			return;
		}

		if ((o = entity_list.FindObject(id)) == nullptr) {
			c->Message(
				Chat::White, "ERROR: Object %u not found in zone, or is a static object not yet unlocked with "
							 "'#object Edit' for editing.",
				id
			);
			return;
		}

		o->SetHeading(atof(sep->arg[3]));

		// Despawn and respawn object to reflect change
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	}

	if (strcasecmp(sep->arg[1], "save") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) == 0)) {
			c->Message(Chat::White, "Usage: #object Save (ObjectID)");
			return;
		}

		o = entity_list.FindObject(id);

		od.zone_id       = 0;
		od.zone_instance = 0;
		od.object_type   = 0;

		// If this ID isn't in the database yet, it's a new object
		bNewObject = true;
		std::string query   = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
		auto        results = content_db.QueryDatabase(query);
		if (results.Success() && results.RowCount() != 0) {
			auto row = results.begin();
			od.zone_id       = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type   = atoi(row[2]);

			// ID already in database. Not a new object.
			bNewObject = false;
		}

		if (!o) {
			// Object not found in zone. Can't save an object we can't see.

			if (bNewObject) {
				c->Message(Chat::White, "ERROR: Object %u not found", id);
				return;
			}

			if (od.zone_id != zone->GetZoneID()) {
				c->Message(Chat::White, "ERROR: Wrong Object ID. %u is not part of this zone.", id);
				return;
			}

			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(Chat::White, "ERROR: Wrong Object ID. %u is not part of this instance version.", id);
				return;
			}

			if (od.object_type == 0) {
				c->Message(
					Chat::White, "ERROR: Static Object %u has already been committed. Use '#object Edit "
								 "%u' and zone out and back in to make changes.",
					id, id
				);
				return;
			}

			if (od.object_type == 1) {
				c->Message(
					Chat::White, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, "
								 "which is not supported with #object. See the 'ground_spawns' table in "
								 "the database.",
					id
				);
				return;
			}

			c->Message(Chat::White, "ERROR: Object %u not found.", id);
			return;
		}

		// Oops! Another GM already saved an object with our id from another zone.
		// We'll have to get a new one.
		if ((od.zone_id > 0) && (od.zone_id != zone->GetZoneID())) {
			id = 0;
		}

		// Oops! Another GM already saved an object with our id from another instance.
		// We'll have to get a new one.
		if ((id > 0) && (od.zone_instance != zone->GetInstanceVersion())) {
			id = 0;
		}

		// If we're asking for a new ID, it's a new object.
		bNewObject |= (id == 0);

		o->GetObjectData(&od);
		od.object_type = o->GetType();
		icon = o->GetIcon();

		// We're committing to the database now. Return temporary object type to actual.
		if (od.object_type == staticType) {
			od.object_type = 0;
		}

		if (!bNewObject) {
			query = StringFormat(
				"UPDATE object SET zoneid = %u, version = %u, "
				"xpos = %.1f, ypos=%.1f, zpos=%.1f, heading=%.1f, "
				"objectname = '%s', type = %u, icon = %u, "
				"unknown08 = %u, unknown10 = %u, unknown20 = %u "
				"WHERE ID = %u",
				zone->GetZoneID(), zone->GetInstanceVersion(), od.x, od.y, od.z,
				od.heading, od.object_name, od.object_type, icon, od.size,
				od.solidtype, od.unknown020, id
			);
		}
		else if (id == 0) {
			query = StringFormat(
				"INSERT INTO object "
				"(zoneid, version, xpos, ypos, zpos, heading, objectname, "
				"type, icon, unknown08, unknown10, unknown20) "
				"VALUES (%u, %u, %.1f, %.1f, %.1f, %.1f, '%s', %u, %u, %u, %u, %u)",
				zone->GetZoneID(), zone->GetInstanceVersion(), od.x, od.y, od.z,
				od.heading, od.object_name, od.object_type, icon, od.size,
				od.solidtype, od.unknown020
			);
		}
		else {
			query = StringFormat(
				"INSERT INTO object "
				"(id, zoneid, version, xpos, ypos, zpos, heading, objectname, "
				"type, icon, unknown08, unknown10, unknown20) "
				"VALUES (%u, %u, %u, %.1f, %.1f, %.1f, %.1f, '%s', %u, %u, %u, %u, %u)",
				id, zone->GetZoneID(), zone->GetInstanceVersion(), od.x, od.y, od.z,
				od.heading, od.object_name, od.object_type, icon, od.size,
				od.solidtype, od.unknown020
			);
		}

		results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		if (results.RowsAffected() == 0) {
			// No change made, but no error message given
			c->Message(Chat::White, "Database Error: Could not save change to Object %u", id);
			return;
		}

		if (bNewObject) {
			if (newid == results.LastInsertedID()) {
				c->Message(Chat::White, "Saved new Object %u to database", id);
				return;
			}

			c->Message(Chat::White, "Saved Object. NOTE: Database returned a new ID number for object: %u", newid);
			id = newid;
			return;
		}

		c->Message(Chat::White, "Saved changes to Object %u", id);
		newid = id;

		if (od.object_type == 0) {
			// Static Object - Respawn as nonfunctional door

			app = new EQApplicationPacket();
			o->CreateDeSpawnPacket(app);
			entity_list.QueueClients(0, app);
			safe_delete(app);

			entity_list.RemoveObject(o->GetID());

			auto door = DoorsRepository::NewEntity();

			door.zone = zone->GetShortName();

			door.id      = 1000000000 + id; // Out of range of normal use for doors.id
			door.doorid  = -1; // Client doesn't care if these are all the same door_id
			door.pos_x   = od.x;
			door.pos_y   = od.y;
			door.pos_z   = od.z;
			door.heading = od.heading;

			door.name = od.object_name;

			// Strip trailing "_ACTORDEF" if present. Client won't accept it for doors.
			int pos = door.name.size() - strlen("_ACTORDEF");
			if (pos > 0 && door.name.compare(pos, std::string::npos, "_ACTORDEF") == 0) {
				door.name.erase(pos);
			}

			door.dest_zone = "NONE";

			if ((door.size = od.size) == 0) { // unknown08 = optional size percentage
				door.size = 100;
			}

			door.opentype = od.solidtype;

			switch (door.opentype) // unknown10 = optional request_nonsolid (0 or 1 or experimental number)
			{
				case 0:
					door.opentype = 31;
					break;

				case 1:
					door.opentype = 9;
					break;
			}

			door.incline             = od.unknown020; // unknown20 = optional incline value
			door.client_version_mask = 0xFFFFFFFF;

			Doors *doors = new Doors(door);

			entity_list.AddDoor(doors);

			app = new EQApplicationPacket(OP_SpawnDoor, sizeof(Door_Struct));
			ds  = (Door_Struct *) app->pBuffer;

			memset(ds, 0, sizeof(Door_Struct));
			memcpy(ds->name, door.name.c_str(), 32);
			ds->xPos     = door.pos_x;
			ds->yPos     = door.pos_y;
			ds->zPos     = door.pos_z;
			ds->heading  = door.heading;
			ds->incline  = door.incline;
			ds->size     = door.size;
			ds->doorId   = door.doorid;
			ds->opentype = door.opentype;
			ds->unknown0052[9]  = 1; // *ptr-1 and *ptr-3 from EntityList::MakeDoorSpawnPacket()
			ds->unknown0052[11] = 1;

			entity_list.QueueClients(0, app);
			safe_delete(app);

			c->Message(
				Chat::White, "NOTE: Object %u is now a static object, and is unchangeable. To make future "
							 "changes, use '#object Edit' to convert it to a changeable form, then zone out "
							 "and back in.",
				id
			);
		}
		return;
	}

	if (strcasecmp(sep->arg[1], "copy") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 3) ||
			(((sep->arg[2][0] & 0xDF) != 'A') && ((sep->arg[2][0] < '0') || (sep->arg[2][0] > '9')))) {
			c->Message(Chat::White, "Usage: #object Copy All|(ObjectID) (InstanceVersion)");
			c->Message(Chat::White, "- Note: Only objects saved in the database can be copied to another instance.");
			return;
		}

		od.zone_instance = atoi(sep->arg[3]);

		if (od.zone_instance == zone->GetInstanceVersion()) {
			c->Message(Chat::White, "ERROR: Source and destination instance versions are the same.");
			return;
		}

		if ((sep->arg[2][0] & 0xDF) == 'A') {
			// Copy All

			std::string query   =
							StringFormat(
								"INSERT INTO object "
								"(zoneid, version, xpos, ypos, zpos, heading, itemid, "
								"objectname, type, icon, unknown08, unknown10, unknown20) "
								"SELECT zoneid, %u, xpos, ypos, zpos, heading, itemid, "
								"objectname, type, icon, unknown08, unknown10, unknown20 "
								"FROM object WHERE zoneid = %u) AND version = %u",
								od.zone_instance, zone->GetZoneID(), zone->GetInstanceVersion());
			auto        results = content_db.QueryDatabase(query);
			if (!results.Success()) {
				c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
				return;
			}

			c->Message(
				Chat::White, "Copied %u object%s into instance version %u", results.RowCount(),
				(results.RowCount() == 1) ? "" : "s", od.zone_instance
			);
			return;
		}

		id = atoi(sep->arg[2]);

		std::string query   = StringFormat(
			"INSERT INTO object "
			"(zoneid, version, xpos, ypos, zpos, heading, itemid, "
			"objectname, type, icon, unknown08, unknown10, unknown20) "
			"SELECT zoneid, %u, xpos, ypos, zpos, heading, itemid, "
			"objectname, type, icon, unknown08, unknown10, unknown20 "
			"FROM object WHERE id = %u AND zoneid = %u AND version = %u",
			od.zone_instance, id, zone->GetZoneID(), zone->GetInstanceVersion());
		auto        results = content_db.QueryDatabase(query);
		if (results.Success() && results.RowsAffected() > 0) {
			c->Message(Chat::White, "Copied Object %u into instance version %u", id, od.zone_instance);
			return;
		}

		// Couldn't copy the object.

		// got an error message
		if (!results.Success()) {
			c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		// No database error returned. See if we can figure out why.

		query   = StringFormat("SELECT zoneid, version FROM object WHERE id = %u", id);
		results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		if (results.RowCount() == 0) {
			c->Message(Chat::White, "ERROR: Object %u not found", id);
			return;
		}

		auto row = results.begin();
		// Wrong ZoneID?
		if (atoi(row[0]) != zone->GetZoneID()) {
			c->Message(Chat::White, "ERROR: Object %u is not part of this zone.", id);
			return;
		}

		// Wrong Instance Version?
		if (atoi(row[1]) != zone->GetInstanceVersion()) {
			c->Message(Chat::White, "ERROR: Object %u is not part of this instance version.", id);
			return;
		}

		// Well, NO clue at this point. Just let 'em know something screwed up.
		c->Message(
			Chat::White, "ERROR: Unknown database error copying Object %u to instance version %u", id,
			od.zone_instance
		);
		return;
	}

	if (strcasecmp(sep->arg[1], "delete") == 0) {

		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) <= 0)) {
			c->Message(
				Chat::White, "Usage: #object Delete (ObjectID) -- NOTE: Object deletions are permanent and "
							 "cannot be undone!"
			);
			return;
		}

		o = entity_list.FindObject(id);

		if (o) {
			// Object found in zone.

			app = new EQApplicationPacket();
			o->CreateDeSpawnPacket(app);
			entity_list.QueueClients(nullptr, app);

			entity_list.RemoveObject(o->GetID());

			// Verifying ZoneID and Version in case someone else ended up adding an object with our ID
			// from a different zone/version. Don't want to delete someone else's work.
			std::string query   = StringFormat(
				"DELETE FROM object "
				"WHERE id = %u AND zoneid = %u "
				"AND version = %u LIMIT 1",
				id, zone->GetZoneID(), zone->GetInstanceVersion());
			auto        results = content_db.QueryDatabase(query);

			c->Message(Chat::White, "Object %u deleted", id);
			return;
		}

		// Object not found in zone.
		std::string query   = StringFormat(
			"SELECT type FROM object "
			"WHERE id = %u AND zoneid = %u "
			"AND version = %u LIMIT 1",
			id, zone->GetZoneID(), zone->GetInstanceVersion());
		auto        results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		if (results.RowCount() == 0) {
			c->Message(Chat::White, "ERROR: Object %u not found in this zone or instance!", id);
			return;
		}

		auto row = results.begin();

		switch (atoi(row[0])) {
			case 0: // Static Object
				query   = StringFormat(
					"DELETE FROM object WHERE id = %u "
					"AND zoneid = %u AND version = %u LIMIT 1",
					id, zone->GetZoneID(), zone->GetInstanceVersion());
				results = content_db.QueryDatabase(query);

				c->Message(
					Chat::White, "Object %u deleted. NOTE: This static object will remain for anyone currently in "
								 "the zone until they next zone out and in.",
					id
				);
				return;

			case 1: // Temporary Spawn
				c->Message(
					Chat::White, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, which "
								 "is not supported with #object. See the 'ground_spawns' table in the database.",
					id
				);
				return;
		}

		return;
	}

	if (strcasecmp(sep->arg[1], "undo") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) == 0)) {
			c->Message(
				Chat::White, "Usage: #object Undo (ObjectID) -- Reload object from database, undoing any "
							 "changes you have made"
			);
			return;
		}

		o = entity_list.FindObject(id);

		if (!o) {
			c->Message(
				Chat::White, "ERROR: Object %u not found in zone in a manipulable form. No changes to undo.",
				id
			);
			return;
		}

		if (o->GetType() == OT_DROPPEDITEM) {
			c->Message(
				Chat::White, "ERROR: Object %u is a temporary spawned item and cannot be manipulated with "
							 "#object. See the 'ground_spawns' table in the database.",
				id
			);
			return;
		}

		// Despawn current item for reloading from database
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		entity_list.RemoveObject(o->GetID());
		safe_delete(app);

		std::string query   = StringFormat(
			"SELECT xpos, ypos, zpos, "
			"heading, objectname, type, icon, "
			"unknown08, unknown10, unknown20 "
			"FROM object WHERE id = %u",
			id
		);
		auto        results = content_db.QueryDatabase(query);
		if (!results.Success() || results.RowCount() == 0) {
			c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		memset(&od, 0, sizeof(od));

		auto row = results.begin();

		od.x       = atof(row[0]);
		od.y       = atof(row[1]);
		od.z       = atof(row[2]);
		od.heading = atof(row[3]);
		strn0cpy(od.object_name, row[4], sizeof(od.object_name));
		od.object_type = atoi(row[5]);
		icon = atoi(row[6]);
		od.size       = atoi(row[7]);
		od.solidtype  = atoi(row[8]);
		od.unknown020 = atoi(row[9]);

		if (od.object_type == 0) {
			od.object_type = staticType;
		}

		o = new Object(id, od.object_type, icon, od, nullptr);
		entity_list.AddObject(o, true);

		c->Message(Chat::White, "Object %u reloaded from database.", id);
		return;
	}

	c->Message(Chat::White, usage_string);
}

