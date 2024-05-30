#include "../common/global_define.h"
#include "../common/bodytypes.h"

std::string BodyType::GetBodyTypeName(uint8 body_type_id)
{
	return IsValidBodyType(body_type_id) ? body_type_names[body_type_id] : "UNKNOWN BODY TYPE";
}

bool BodyType::IsValidBodyType(uint8 body_type_id)
{
	return body_type_names.find(body_type_id) != body_type_names.end();
}
