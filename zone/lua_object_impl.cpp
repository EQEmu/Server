#ifdef LUA_EQEMU
#include "object.h"
#include "lua_object.h"

void Lua_Object::Depop() {
	Lua_Safe_Call_Void();
	self->Depop();
}

void Lua_Object::Repop() {
	Lua_Safe_Call_Void();
	self->Repop();
}

void Lua_Object::SetModelName(const char *name) {
	Lua_Safe_Call_Void();
	self->SetModelName(name);
}

const char *Lua_Object::GetModelName() {
	Lua_Safe_Call_String();
	return self->GetModelName();
}

float Lua_Object::GetX() {
	Lua_Safe_Call_Real();
	return self->GetX();
}

float Lua_Object::GetY() {
	Lua_Safe_Call_Real();
	return self->GetY();
}

float Lua_Object::GetZ() {
	Lua_Safe_Call_Real();
	return self->GetZ();
}

float Lua_Object::GetHeading() {
	Lua_Safe_Call_Real();
	float h = 0.0f;
	self->GetHeading(&h);
	return h;
}

void Lua_Object::SetX(float x) {
	Lua_Safe_Call_Void();
	self->SetX(x);
}

void Lua_Object::SetY(float y) {
	Lua_Safe_Call_Void();
	self->SetY(y);
}

void Lua_Object::SetZ(float z) {
	Lua_Safe_Call_Void();
	self->SetZ(z);
}

void Lua_Object::SetHeading(float h) {
	Lua_Safe_Call_Void();
	self->SetHeading(h);
}

void Lua_Object::SetLocation(float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetLocation(x, y, z);
}

void Lua_Object::SetItemID(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->SetItemID(item_id);
}

uint32 Lua_Object::GetItemID() {
	Lua_Safe_Call_Int();
	return self->GetItemID();
}

void Lua_Object::SetIcon(uint32 icon) {
	Lua_Safe_Call_Void();
	self->SetIcon(icon);
}

uint32 Lua_Object::GetIcon() {
	Lua_Safe_Call_Int();
	return self->GetIcon();
}

void Lua_Object::SetType(uint32 type) {
	Lua_Safe_Call_Void();
	self->SetType(type);
}

uint32 Lua_Object::GetType() {
	Lua_Safe_Call_Int();
	return self->GetType();
}

uint32 Lua_Object::GetDBID() {
	Lua_Safe_Call_Int();
	return self->GetDBID();
}

void Lua_Object::ClearUser() {
	Lua_Safe_Call_Void();
	self->ClearUser();
}

void Lua_Object::SetID(int user) {
	Lua_Safe_Call_Void();
	self->SetID(user);
}

int Lua_Object::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

bool Lua_Object::Save() {
	Lua_Safe_Call_Int();
	return self->Save();
}

uint32 Lua_Object::VarSave() {
	Lua_Safe_Call_Int();
	return self->VarSave();
}

void Lua_Object::DeleteItem(int index) {
	Lua_Safe_Call_Void();
	self->DeleteItem(index);
}

void Lua_Object::StartDecay() {
	Lua_Safe_Call_Void();
	self->StartDecay();
}

void Lua_Object::Delete() {
	Lua_Safe_Call_Void();
	self->Delete();
}

void Lua_Object::Delete(bool reset_state) {
	Lua_Safe_Call_Void();
	self->Delete(reset_state);
}

bool Lua_Object::IsGroundSpawn() {
	Lua_Safe_Call_Int();
	return self->IsGroundSpawn();
}

void Lua_Object::Close() {
	Lua_Safe_Call_Void();
	self->Close();
}

const char *Lua_Object::GetEntityVariable(const char *name) {
	Lua_Safe_Call_String();
	return self->GetEntityVariable(name);
}

void Lua_Object::SetEntityVariable(const char *name, const char *value) {
	Lua_Safe_Call_Void();
	self->SetEntityVariable(name, value);
}

bool Lua_Object::EntityVariableExists(const char *name) {
	Lua_Safe_Call_Int();
	return self->EntityVariableExists(name);
}
#endif
