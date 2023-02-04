#include "../client.h"

void command_weather(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #weather [0|1|2] - [Off|Rain|Snow]");
		c->Message(Chat::White, "Usage: #weather 3 [Type] [Intensity] - Manually set weather type and intensity");
		return;
	}

	int arguments = sep->argnum;
	if (arguments == 1) {
		auto new_weather = static_cast<uint8>(std::stoul(sep->arg[1]));
		uint8 new_intensity = 0;
		std::string weather_message = "The sky clears.";

		if (new_weather == EQ::constants::WeatherTypes::Snowing) {	
			weather_message = "Snowflakes begin to fall from the sky.";
			new_weather = EQ::constants::WeatherTypes::Snowing;
			new_intensity = 0x02;
		} else if (new_weather == EQ::constants::WeatherTypes::Raining) {
			weather_message = "Raindrops begin to fall from the sky.";
			new_weather = EQ::constants::WeatherTypes::Raining;
			new_intensity = 0x01; // This is how it's done in Fear, and you can see a decent distance with it at this value
		}

		zone->zone_weather = new_weather;
		auto outapp = new EQApplicationPacket(OP_Weather, 8);

		if (new_weather != EQ::constants::WeatherTypes::None) {
			if (new_weather == EQ::constants::WeatherTypes::Snowing) {
				outapp->pBuffer[0] = EQ::constants::WeatherTypes::Snowing;
			}

			outapp->pBuffer[4] = new_intensity;
		}


		c->Message(Chat::White, weather_message.c_str());
		entity_list.QueueClients(c, outapp);

		safe_delete(outapp);
	} else if (arguments == 3) {
		auto command_type = static_cast<uint8>(std::stoul(sep->arg[1]));
		uint8 new_weather = EQ::constants::WeatherTypes::None;
		uint8 new_intensity = 0;
		std::string weather_message;

		if (zone->zone_weather == EQ::constants::WeatherTypes::None) {
			if (command_type > EQ::constants::WeatherTypes::Snowing) {
				new_weather = static_cast<uint8>(std::stoul(sep->arg[2]));
				new_intensity = static_cast<uint8>(std::stoul(sep->arg[3]));

				weather_message = fmt::format(
					"Sending {} ({}) with an intensity of {}.",
					EQ::constants::GetWeatherTypeName(new_weather),
					new_weather,
					new_intensity
				);
			} else if (command_type == EQ::constants::WeatherTypes::Snowing) {	
				weather_message = "Snowflakes begin to fall from the sky.";
				new_weather = EQ::constants::WeatherTypes::Snowing;
				new_intensity = 0x02;
			} else if (command_type == EQ::constants::WeatherTypes::Raining) {
				weather_message = "Raindrops begin to fall from the sky.";
				new_weather = EQ::constants::WeatherTypes::Raining;
				new_intensity = 0x01; // This is how it's done in Fear, and you can see a decent distance with it at this value
			}

			zone->zone_weather = new_weather;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);

			if (new_weather != EQ::constants::WeatherTypes::Raining) {
				outapp->pBuffer[0] = new_weather;
			}
			
			outapp->pBuffer[4] = new_intensity;
 
			c->Message(Chat::White, weather_message.c_str());
			entity_list.QueueClients(c, outapp);

			safe_delete(outapp);
		} else {
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			weather_message = "The sky clears.";

			if (zone->zone_weather == EQ::constants::WeatherTypes::Snowing) {
				weather_message = "The sky clears as the snow stops falling.";
				outapp->pBuffer[0] = 0x01; // Snow has it's own shutoff packet
			} else if (zone->zone_weather == EQ::constants::WeatherTypes::Raining) {
				weather_message = "The sky clears as the rain ceases to fall.";
			}

			zone->zone_weather = EQ::constants::WeatherTypes::None;

			c->Message(Chat::White, weather_message.c_str());
			entity_list.QueueClients(c, outapp);

			safe_delete(outapp);
		}
	}
}

