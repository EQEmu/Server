#include "pets.h"

std::string Pets::Command::GetName(uint32 pet_command)
{
	return IsValid(pet_command) ? pet_commands[pet_command] : "UNKNOWN PET COMMAND";
}

bool Pets::Command::IsValid(uint32 pet_command)
{
	return pet_commands.find(pet_command) != pet_commands.end();
}

std::string Pets::Type::GetName(uint8 pet_type)
{
	return IsValid(pet_type) ? pet_types[pet_type] : "UNKNOWN PET TYPE";
}

bool Pets::Type::IsValid(uint8 pet_type)
{
	return pet_types.find(pet_type) != pet_types.end();
}
