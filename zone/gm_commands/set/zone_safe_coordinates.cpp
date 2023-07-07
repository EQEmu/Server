#include "../../client.h"

void SetZoneSafeCoordinates(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (
		arguments < 2 ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3) ||
		!sep->IsNumber(4)
	) {
		c->Message(Chat::White, "Usage: #set zone_safe_coordinates [X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)]");
		c->Message(Chat::White, "Not sending Heading defaults to current Heading and the change is temporary.");
		return;
	}

	const float x         = Strings::ToFloat(sep->arg[1]);
	const float y         = Strings::ToFloat(sep->arg[2]);
	const float z         = Strings::ToFloat(sep->arg[3]);
	const float heading   = sep->arg[3] ? Strings::ToFloat(sep->arg[3]) : c->GetHeading();
	const bool  permanent = sep->arg[4] ? Strings::ToBool(sep->arg[4]) : false;

	if (permanent) {
		const std::string& query = fmt::format(
			"UPDATE zone SET safe_x = {:.2f}, safe_y = {:.2f}, safe_z = {:.2f}, safe_heading = {:.2f} WHERE zoneidnumber = {} AND version = {}",
			x,
			y,
			z,
			heading,
			zone->GetZoneID(),
			zone->GetInstanceVersion()
		);
		database.QueryDatabase(query);
	}

	zone->newzone_data.safe_x = x;
	zone->newzone_data.safe_y = y;
	zone->newzone_data.safe_z = z;

	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);

	c->Message(
		Chat::White,
		fmt::format(
			"Safe Coordinates Changed | Zone: {} XYZ: {:.2f}, {:.2f}, {:.2f} Heading: {:.2f} Permanent: {} ",
			zone->GetZoneDescription(),
			x,
			y,
			z,
			heading,
			permanent ? "Yes" : "No"
		).c_str()
	);
}
