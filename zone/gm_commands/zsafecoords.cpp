#include "../client.h"

void command_zsafecoords(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (
		!arguments ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3)
	) {
		c->Message(Chat::White, "Usage: #zsafecoords [X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)]");
		c->Message(Chat::White, "Not sending Heading defaults to current Heading and the change is temporary.");
		return;
	}

	auto x = std::stof(sep->arg[1]);
	auto y = std::stof(sep->arg[2]);
	auto z = std::stof(sep->arg[3]);
	auto heading = sep->arg[3] ? std::stof(sep->arg[3]) : c->GetHeading();
	auto permanent = sep->arg[4] ? atobool(sep->arg[4]) : false;
	if (permanent) {
		auto query = fmt::format(
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

