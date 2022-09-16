#ifdef LUA_EQEMU
#include "lua_door.h"
#include "lua_mob.h"
#include "doors.h"

void Lua_Door::SetDoorName(const char *name) {
	Lua_Safe_Call_Void();
	self->SetDoorName(name);
}

const char *Lua_Door::GetDoorName() {
	Lua_Safe_Call_String();
	return self->GetDoorName();
}

float Lua_Door::GetX() {
	Lua_Safe_Call_Real();
	return self->GetPosition().x;
}

float Lua_Door::GetY() {
	Lua_Safe_Call_Real();
	return self->GetPosition().y;
}

float Lua_Door::GetZ() {
	Lua_Safe_Call_Real();
	return self->GetPosition().z;
}

float Lua_Door::GetHeading() {
	Lua_Safe_Call_Real();
	return self->GetPosition().w;
}

void Lua_Door::SetX(float x) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.x = x;
	self->SetPosition(position);
}

void Lua_Door::SetY(float y) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.y = y;
	self->SetPosition(position);
}

void Lua_Door::SetZ(float z) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.z = z;
	self->SetPosition(position);
}

void Lua_Door::SetHeading(float h) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.w = h;
	self->SetPosition(position);
}

void Lua_Door::SetLocation(float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetLocation(x, y, z);
}

uint32 Lua_Door::GetDoorDBID() {
	Lua_Safe_Call_Int();
	return self->GetDoorDBID();
}

uint32 Lua_Door::GetDoorID() {
	Lua_Safe_Call_Int();
	return self->GetDoorID();
}

void Lua_Door::SetSize(uint32 sz) {
	Lua_Safe_Call_Void();
	self->SetSize(sz);
}

uint32 Lua_Door::GetSize() {
	Lua_Safe_Call_Int();
	return self->GetSize();
}

void Lua_Door::SetIncline(uint32 incline) {
	Lua_Safe_Call_Void();
	self->SetIncline(incline);
}

uint32 Lua_Door::GetIncline() {
	Lua_Safe_Call_Int();
	return self->GetIncline();
}

void Lua_Door::SetOpenType(uint32 type) {
	Lua_Safe_Call_Void();
	self->SetOpenType(type);
}

uint32 Lua_Door::GetOpenType() {
	Lua_Safe_Call_Int();
	return self->GetOpenType();
}

void Lua_Door::SetDisableTimer(bool flag) {
	Lua_Safe_Call_Void();
	self->SetDisableTimer(flag);
}

bool Lua_Door::GetDisableTimer() {
	Lua_Safe_Call_Bool();
	return self->GetDisableTimer();
}

void Lua_Door::SetLockPick(uint32 pick) {
	Lua_Safe_Call_Void();
	self->SetLockpick(pick);
}

uint32 Lua_Door::GetLockPick() {
	Lua_Safe_Call_Int();
	return self->GetLockpick();
}

void Lua_Door::SetKeyItem(uint32 key) {
	Lua_Safe_Call_Void();
	self->SetKeyItem(key);
}

uint32 Lua_Door::GetKeyItem() {
	Lua_Safe_Call_Int();
	return self->GetKeyItem();
}

void Lua_Door::SetNoKeyring(int type) {
	Lua_Safe_Call_Void();
	self->SetNoKeyring(type);
}

int Lua_Door::GetNoKeyring() {
	Lua_Safe_Call_Int();
	return self->GetNoKeyring();
}

void Lua_Door::CreateDatabaseEntry() {
	Lua_Safe_Call_Void();
	self->CreateDatabaseEntry();
}

void Lua_Door::ForceOpen(Lua_Mob sender) {
	Lua_Safe_Call_Void();
	self->ForceOpen(sender);
}

void Lua_Door::ForceOpen(Lua_Mob sender, bool alt_mode) {
	Lua_Safe_Call_Void();
	self->ForceOpen(sender, alt_mode);
}

void Lua_Door::ForceClose(Lua_Mob sender) {
	Lua_Safe_Call_Void();
	self->ForceClose(sender);
}

void Lua_Door::ForceClose(Lua_Mob sender, bool alt_mode) {
	Lua_Safe_Call_Void();
	self->ForceClose(sender, alt_mode);
}
#endif
