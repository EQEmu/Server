#include <sol.hpp>
#include "../object.h"

void lua_register_object(sol::state *state) {
	auto object = state->create_simple_usertype<Object>(sol::base_classes, sol::bases<Entity>());
	object.set("Depop", &Object::Depop);
	object.set("Repop", &Object::Repop);
	object.set("SetModelName", &Object::SetModelName);
	object.set("GetModelName", &Object::GetModelName);
	object.set("GetX", &Object::GetX);
	object.set("GetY", &Object::GetY);
	object.set("GetZ", &Object::GetZ);
	object.set("GetHeading", &Object::GetHeading);
	object.set("SetX", &Object::SetX);
	object.set("SetY", &Object::SetY);
	object.set("SetZ", &Object::SetZ);
	object.set("SetHeading", &Object::SetHeading);
	object.set("SetLocation", &Object::SetLocation);
	object.set("SetItemID", &Object::SetItemID);
	object.set("SetIcon", &Object::SetIcon);
	object.set("GetIcon", &Object::GetIcon);
	object.set("SetType", &Object::SetType);
	object.set("GetType", &Object::GetType);
	object.set("GetDBID", &Object::GetDBID);
	object.set("ClearUser", &Object::ClearUser);
	object.set("Save", &Object::Save);
	object.set("VarSave", &Object::VarSave);
	object.set("DeleteItem", &Object::DeleteItem);
	object.set("StartDecay", &Object::StartDecay);
	object.set("Delete", sol::overload((void(Object::*)())&Object::Delete, (void(Object::*)(bool))&Object::Delete));
	object.set("IsGroundSpawn", &Object::IsGroundSpawn);
	object.set("GetEntityVariable", &Object::GetEntityVariable);
	object.set("SetEntityVariable", &Object::SetEntityVariable);
	object.set("EntityVariableExists", &Object::EntityVariableExists);
	state->set_usertype("Object", object);
}
