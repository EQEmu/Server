#include <sol.hpp>
#include "../object.h"

void lua_register_object(sol::state *state) {
	state->new_usertype<Object>("Object",
		sol::base_classes, sol::bases<Entity>(),
		"Depop", &Object::Depop,
		"Repop", &Object::Repop,
		"SetModelName", &Object::SetModelName,
		"GetModelName", &Object::GetModelName,
		"GetX", &Object::GetX,
		"GetY", &Object::GetY,
		"GetZ", &Object::GetZ,
		"GetHeading", &Object::GetHeading,
		"SetX", &Object::SetX,
		"SetY", &Object::SetY,
		"SetZ", &Object::SetZ,
		"SetHeading", &Object::SetHeading,
		"SetLocation", &Object::SetLocation,
		"SetItemID", &Object::SetItemID,
		"SetIcon", &Object::SetIcon,
		"GetIcon", &Object::GetIcon,
		"SetType", &Object::SetType,
		"GetType", &Object::GetType,
		"GetDBID", &Object::GetDBID,
		"ClearUser", &Object::ClearUser,
		"Save", &Object::Save,
		"VarSave", &Object::VarSave,
		"DeleteItem", &Object::DeleteItem,
		"StartDecay", &Object::StartDecay,
		"Delete", sol::overload((void(Object::*)())&Object::Delete, (void(Object::*)(bool))&Object::Delete),
		"IsGroundSpawn", &Object::IsGroundSpawn,
		"GetEntityVariable", &Object::GetEntityVariable,
		"SetEntityVariable", &Object::SetEntityVariable,
		"EntityVariableExists", &Object::EntityVariableExists
	);
}
