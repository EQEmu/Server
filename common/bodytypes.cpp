#include "../common/global_define.h"
#include "../common/bodytypes.h"

const std::string& EQ::bodytype::GetBodyTypeName(uint8 body_type_id)
{
	const auto& e = body_type_names.find(body_type_id);
	return e != body_type_names.end() ? e->second : std::string();
}

bool EQ::bodytype::IsValidBodyType(uint8 body_type_id)
{
	return body_type_names.find(body_type_id) != body_type_names.end();
}
