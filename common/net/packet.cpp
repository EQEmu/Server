#include "packet.h"
#include <fmt/format.h>
#include <cctype>
#include <fmt/format.h>

bool EQ::Net::StaticPacket::Resize(size_t new_size)
{
	if (new_size > _max_len) {
		return false;
	}

	_len = new_size;
	return true;
}

char ToSafePrint(unsigned char in) {
	if (std::isprint(in)) {
		return in;
	}

	return '.';
}

std::string EQ::Net::Packet::ToString() const
{
	return ToString(16);
}

std::string EQ::Net::Packet::ToString(size_t line_length) const
{
	if (Length() == 0) {
		return fmt::format("{:0>5x} |", 0);
	}

	std::string ret;
	size_t lines = Length() / line_length;
	size_t i;

	char *data = (char*)Data();

	for (i = 0; i < lines; ++i) {
		ret += fmt::format("{:0>5x} |", i * line_length);
		std::string hex;
		std::string ascii;
		for (size_t j = 0; j < line_length; ++j) {
			hex += fmt::format(" {:0>2x}", (uint8_t)data[(i * line_length) + j]);
			ascii += fmt::format("{}", ToSafePrint(data[(i * line_length) + j]));
		}

		ret += hex;
		ret += " | ";
		ret += ascii;
		ret += "\n";
	}

	if (Length() % line_length > 0) {
		ret += fmt::format("{:0>5x} |", i * line_length);

		size_t non_blank_count = Length() % line_length;
		size_t blank_count = line_length - non_blank_count;
		std::string hex;
		std::string ascii;

		for (size_t j = 0; j < non_blank_count; ++j) {
			hex += fmt::format(" {:0>2x}", (uint8_t)data[(i * line_length) + j]);
			ascii += fmt::format("{}", ToSafePrint(data[(i * line_length) + j]));
		}

		for (size_t j = 0; j < blank_count; ++j) {
			hex += "   ";
			ascii += " ";
		}

		ret += hex;
		ret += " | ";
		ret += ascii;
		ret += "\n";
	}

	return ret;
}
