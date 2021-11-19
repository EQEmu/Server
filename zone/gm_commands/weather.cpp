#include "../client.h"

void command_weather(Client *c, const Seperator *sep)
{
	if (!(sep->arg[1][0] == '0' || sep->arg[1][0] == '1' || sep->arg[1][0] == '2' || sep->arg[1][0] == '3')) {
		c->Message(Chat::White, "Usage: #weather <0/1/2/3> - Off/Rain/Snow/Manual.");
	}
	else if (zone->zone_weather == 0) {
		if (sep->arg[1][0] ==
			'3') { // Put in modifications here because it had a very good chance at screwing up the client's weather system if rain was sent during snow -T7
			if (sep->arg[2][0] != 0 && sep->arg[3][0] != 0) {
				c->Message(Chat::White, "Sending weather packet... TYPE=%s, INTENSITY=%s", sep->arg[2], sep->arg[3]);
				zone->zone_weather = atoi(sep->arg[2]);
				auto outapp        = new EQApplicationPacket(OP_Weather, 8);
				outapp->pBuffer[0] = atoi(sep->arg[2]);
				outapp->pBuffer[4] = atoi(sep->arg[3]); // This number changes in the packets, intensity?
				entity_list.QueueClients(c, outapp);
				safe_delete(outapp);
			}
			else {
				c->Message(Chat::White, "Manual Usage: #weather 3 <type> <intensity>");
			}
		}
		else if (sep->arg[1][0] == '2') {
			entity_list.Message(0, 0, "Snowflakes begin to fall from the sky.");
			zone->zone_weather = 2;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			outapp->pBuffer[0] = 0x01;
			outapp->pBuffer[4] = 0x02; // This number changes in the packets, intensity?
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
		else if (sep->arg[1][0] == '1') {
			entity_list.Message(0, 0, "Raindrops begin to fall from the sky.");
			zone->zone_weather = 1;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			outapp->pBuffer[4] = 0x01; // This is how it's done in Fear, and you can see a decent distance with it at this value
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
	}
	else {
		if (zone->zone_weather == 1) { // Doing this because if you have rain/snow on, you can only turn one off.
			entity_list.Message(0, 0, "The sky clears as the rain ceases to fall.");
			zone->zone_weather = 0;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			// To shutoff weather you send an empty 8 byte packet (You get this everytime you zone even if the sky is clear)
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
		else if (zone->zone_weather == 2) {
			entity_list.Message(0, 0, "The sky clears as the snow stops falling.");
			zone->zone_weather = 0;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			// To shutoff weather you send an empty 8 byte packet (You get this everytime you zone even if the sky is clear)
			outapp->pBuffer[0] = 0x01; // Snow has it's own shutoff packet
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
		else {
			entity_list.Message(0, 0, "The sky clears.");
			zone->zone_weather = 0;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			// To shutoff weather you send an empty 8 byte packet (You get this everytime you zone even if the sky is clear)
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
	}
}

