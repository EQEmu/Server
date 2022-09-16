#ifdef LUA_EQEMU
#include "npc.h"
#include "lua_npc.h"
#include "lua_client.h"

void Lua_NPC::Signal(int id) {
	Lua_Safe_Call_Void();
	self->SignalNPC(id);
}

int Lua_NPC::CheckNPCFactionAlly(int faction) {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->CheckNPCFactionAlly(faction));
}

void Lua_NPC::AddItem(int item_id, int charges) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3, aug4);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3, aug4, aug5);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5, int aug6) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3, aug4, aug5, aug6);
}

void Lua_NPC::AddLootTable() {
	Lua_Safe_Call_Void();
	self->AddLootTable();
}

void Lua_NPC::AddLootTable(int id) {
	Lua_Safe_Call_Void();
	self->AddLootTable(id);
}

void Lua_NPC::RemoveItem(int item_id) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id);
}

void Lua_NPC::RemoveItem(int item_id, int quantity) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id, quantity);
}

void Lua_NPC::RemoveItem(int item_id, int quantity, int slot) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id, quantity, slot);
}

void Lua_NPC::ClearItemList() {
	Lua_Safe_Call_Void();
	self->ClearItemList();
}

void Lua_NPC::AddCash(int copper, int silver, int gold, int platinum) {
	Lua_Safe_Call_Void();
	self->AddCash(copper, silver, gold, platinum);
}

void Lua_NPC::RemoveCash() {
	Lua_Safe_Call_Void();
	self->RemoveCash();
}

int Lua_NPC::CountLoot() {
	Lua_Safe_Call_Int();
	return self->CountLoot();
}

int Lua_NPC::GetLoottableID() {
	Lua_Safe_Call_Int();
	return self->GetLoottableID();
}

uint32 Lua_NPC::GetCopper() {
	Lua_Safe_Call_Int();
	return self->GetCopper();
}

uint32 Lua_NPC::GetSilver() {
	Lua_Safe_Call_Int();
	return self->GetSilver();
}

uint32 Lua_NPC::GetGold() {
	Lua_Safe_Call_Int();
	return self->GetGold();
}

uint32 Lua_NPC::GetPlatinum() {
	Lua_Safe_Call_Int();
	return self->GetPlatinum();
}

void Lua_NPC::SetCopper(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetCopper(amt);
}

void Lua_NPC::SetSilver(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetSilver(amt);
}

void Lua_NPC::SetGold(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetGold(amt);
}

void Lua_NPC::SetPlatinum(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetPlatinum(amt);
}

void Lua_NPC::SetGrid(int grid) {
	Lua_Safe_Call_Void();
	self->SetGrid(grid);
}

void Lua_NPC::SetSaveWaypoint(int wp) {
	Lua_Safe_Call_Void();
	self->SetSaveWaypoint(wp);
}

void Lua_NPC::SetSp2(int sg2) {
	Lua_Safe_Call_Void();
	self->SetSpawnGroupId(sg2);
}

int Lua_NPC::GetWaypointMax() {
	Lua_Safe_Call_Int();
	return self->GetWaypointMax();
}

int Lua_NPC::GetGrid() {
	Lua_Safe_Call_Int();
	return self->GetGrid();
}

uint32 Lua_NPC::GetSp2() {
	Lua_Safe_Call_Int();
	return self->GetSpawnGroupId();
}

int Lua_NPC::GetNPCFactionID() {
	Lua_Safe_Call_Int();
	return self->GetNPCFactionID();
}

int Lua_NPC::GetPrimaryFaction() {
	Lua_Safe_Call_Int();
	return self->GetPrimaryFaction();
}

int64 Lua_NPC::GetNPCHate(Lua_Mob ent) {
	Lua_Safe_Call_Int();
	return self->GetNPCHate(ent);
}

bool Lua_NPC::IsOnHatelist(Lua_Mob ent) {
	Lua_Safe_Call_Bool();
	return self->IsOnHatelist(ent);
}

void Lua_NPC::SetNPCFactionID(int id) {
	Lua_Safe_Call_Void();
	self->SetNPCFactionID(id);
}

uint32 Lua_NPC::GetMaxDMG() {
	Lua_Safe_Call_Int();
	return self->GetMaxDMG();
}

uint32 Lua_NPC::GetMinDMG() {
	Lua_Safe_Call_Int();
	return self->GetMinDMG();
}

bool Lua_NPC::IsAnimal() {
	Lua_Safe_Call_Bool();
	return self->IsAnimal();
}

int Lua_NPC::GetPetSpellID() {
	Lua_Safe_Call_Int();
	return self->GetPetSpellID();
}

void Lua_NPC::SetPetSpellID(int id) {
	Lua_Safe_Call_Void();
	self->SetPetSpellID(id);
}

uint32 Lua_NPC::GetMaxDamage(int level) {
	Lua_Safe_Call_Int();
	return self->GetMaxDamage(level);
}

void Lua_NPC::SetTaunting(bool t) {
	Lua_Safe_Call_Void();
	self->SetTaunting(t);
}

bool Lua_NPC::IsTaunting() {
	Lua_Safe_Call_Bool();
	return self->IsTaunting();
}

void Lua_NPC::PickPocket(Lua_Client thief) {
	Lua_Safe_Call_Void();
	self->PickPocket(thief);
}

void Lua_NPC::StartSwarmTimer(uint32 duration) {
	Lua_Safe_Call_Void();
	self->StartSwarmTimer(duration);
}

void Lua_NPC::DoClassAttacks(Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->DoClassAttacks(target);
}

int Lua_NPC::GetMaxWp() {
	Lua_Safe_Call_Int();
	return self->GetMaxWp();
}

void Lua_NPC::DisplayWaypointInfo(Lua_Client client) {
	Lua_Safe_Call_Void();
	self->DisplayWaypointInfo(client);
}

void Lua_NPC::CalculateNewWaypoint() {
	Lua_Safe_Call_Void();
	self->CalculateNewWaypoint();
}

void Lua_NPC::AssignWaypoints(int grid) {
	Lua_Safe_Call_Void();
	self->AssignWaypoints(grid);
}

void Lua_NPC::SetWaypointPause() {
	Lua_Safe_Call_Void();
	self->SetWaypointPause();
}

void Lua_NPC::UpdateWaypoint(int wp) {
	Lua_Safe_Call_Void();
	self->UpdateWaypoint(wp);
}

void Lua_NPC::StopWandering() {
	Lua_Safe_Call_Void();
	self->StopWandering();
}

void Lua_NPC::ResumeWandering() {
	Lua_Safe_Call_Void();
	self->ResumeWandering();
}

void Lua_NPC::PauseWandering(int pause_time) {
	Lua_Safe_Call_Void();
	self->PauseWandering(pause_time);
}

void Lua_NPC::MoveTo(float x, float y, float z, float h, bool save) {
	Lua_Safe_Call_Void();
	auto position = glm::vec4(x, y, z, h);
	self->MoveTo(position, save);
}

void Lua_NPC::NextGuardPosition() {
	Lua_Safe_Call_Void();
	self->NextGuardPosition();
}

void Lua_NPC::SaveGuardSpot() {
	Lua_Safe_Call_Void();
	self->SaveGuardSpot();
}

void Lua_NPC::SaveGuardSpot(bool clear) {
	Lua_Safe_Call_Void();
	self->SaveGuardSpot(clear);
}

void Lua_NPC::SaveGuardSpot(float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->SaveGuardSpot(glm::vec4(x, y, z, heading));
}

bool Lua_NPC::IsGuarding() {
	Lua_Safe_Call_Bool();
	return self->IsGuarding();
}

void Lua_NPC::AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y) {
	Lua_Safe_Call_Void();
	self->AI_SetRoambox(dist, max_x, min_x, max_y, min_y);
}

void Lua_NPC::AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y, uint32 delay, uint32 mindelay) {
	Lua_Safe_Call_Void();
	self->AI_SetRoambox(dist, max_x, min_x, max_y, min_y, delay, mindelay);
}

void Lua_NPC::SetFollowID(int id) {
	Lua_Safe_Call_Void();
	self->SetFollowID(id);
}

void Lua_NPC::SetFollowDistance(int dist) {
	Lua_Safe_Call_Void();
	self->SetFollowDistance(dist);
}

void Lua_NPC::SetFollowCanRun(bool v) {
	Lua_Safe_Call_Void();
	self->SetFollowCanRun(v);
}

int Lua_NPC::GetFollowID() {
	Lua_Safe_Call_Int();
	return self->GetFollowID();
}

int Lua_NPC::GetFollowDistance() {
	Lua_Safe_Call_Int();
	return self->GetFollowDistance();
}

bool Lua_NPC::GetFollowCanRun() {
	Lua_Safe_Call_Bool();
	return self->GetFollowCanRun();
}

int Lua_NPC::GetNPCSpellsID() {
	Lua_Safe_Call_Int();
	return self->GetNPCSpellsID();
}

int Lua_NPC::GetSpawnPointID() {
	Lua_Safe_Call_Int();
	return self->GetSpawnPointID();
}

float Lua_NPC::GetSpawnPointX() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().x;
}

float Lua_NPC::GetSpawnPointY() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().y;
}

float Lua_NPC::GetSpawnPointZ() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().z;
}

float Lua_NPC::GetSpawnPointH() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().w;
}

float Lua_NPC::GetGuardPointX() {
	Lua_Safe_Call_Real();
	return self->GetGuardPoint().x;
}

float Lua_NPC::GetGuardPointY() {
	Lua_Safe_Call_Real();
	return self->GetGuardPoint().y;
}

float Lua_NPC::GetGuardPointZ() {
	Lua_Safe_Call_Real();
	return self->GetGuardPoint().z;
}

void Lua_NPC::SetPrimSkill(int skill_id) {
	Lua_Safe_Call_Void();
	self->SetPrimSkill(skill_id);
}

void Lua_NPC::SetSecSkill(int skill_id) {
	Lua_Safe_Call_Void();
	self->SetSecSkill(skill_id);
}

int Lua_NPC::GetPrimSkill() {
	Lua_Safe_Call_Int();
	return self->GetPrimSkill();
}

int Lua_NPC::GetSecSkill() {
	Lua_Safe_Call_Int();
	return self->GetSecSkill();
}

int Lua_NPC::GetSwarmOwner() {
	Lua_Safe_Call_Int();
	return self->GetSwarmOwner();
}

int Lua_NPC::GetSwarmTarget() {
	Lua_Safe_Call_Int();
	return self->GetSwarmTarget();
}

void Lua_NPC::SetSwarmTarget(int target) {
	Lua_Safe_Call_Void();
	self->SetSwarmTarget(target);
}

void Lua_NPC::ModifyNPCStat(const char *stat, const char *value) {
	Lua_Safe_Call_Void();
	self->ModifyNPCStat(stat, value);
}

void Lua_NPC::AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust) {
	Lua_Safe_Call_Void();
	self->AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, 0, 0);
}

void Lua_NPC::AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust, int min_hp, int max_hp) {
	Lua_Safe_Call_Void();
	self->AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, min_hp, max_hp);
}

void Lua_NPC::RemoveAISpell(int spell_id) {
	Lua_Safe_Call_Void();
	self->RemoveSpellFromNPCList(spell_id);
}

void Lua_NPC::SetSpellFocusDMG(int focus) {
	Lua_Safe_Call_Void();
	self->SetSpellFocusDMG(focus);
}

void Lua_NPC::SetSpellFocusHeal(int focus) {
	Lua_Safe_Call_Void();
	self->SetSpellFocusHeal(focus);
}

int Lua_NPC::GetSpellFocusDMG() {
	Lua_Safe_Call_Int();
	return self->GetSpellFocusDMG();
}

int Lua_NPC::GetSpellFocusHeal() {
	Lua_Safe_Call_Int();
	return self->GetSpellFocusHeal();
}

float Lua_NPC::GetSlowMitigation() {
	Lua_Safe_Call_Real();
	return self->GetSlowMitigation();
}

float Lua_NPC::GetAttackSpeed() {
	Lua_Safe_Call_Real();
	return self->GetAttackSpeed();
}

int Lua_NPC::GetAttackDelay() {
	Lua_Safe_Call_Int();
	return self->GetAttackDelay();
}

int Lua_NPC::GetAccuracyRating() {
	Lua_Safe_Call_Int();
	return self->GetAccuracyRating();
}

int Lua_NPC::GetSpawnKillCount() {
	Lua_Safe_Call_Int();
	return self->GetSpawnKillCount();
}

int Lua_NPC::GetScore() {
	Lua_Safe_Call_Int();
	return self->GetScore();
}

void Lua_NPC::MerchantOpenShop() {
	Lua_Safe_Call_Void();
	self->MerchantOpenShop();
}

void Lua_NPC::MerchantCloseShop() {
	Lua_Safe_Call_Void();
	self->MerchantCloseShop();
}

int Lua_NPC::GetRawAC() {
	Lua_Safe_Call_Int();
	return self->GetRawAC();
}

int Lua_NPC::GetAvoidanceRating()
{
	Lua_Safe_Call_Int();
	return self->GetAvoidanceRating();
}

void Lua_NPC::SetSimpleRoamBox(float box_size)
{
	Lua_Safe_Call_Void();
	self->SetSimpleRoamBox(box_size);
}

void Lua_NPC::SetSimpleRoamBox(float box_size, float move_distance)
{
	Lua_Safe_Call_Void();
	self->SetSimpleRoamBox(box_size, move_distance);
}

void Lua_NPC::SetSimpleRoamBox(float box_size, float move_distance, int move_delay)
{
	Lua_Safe_Call_Void();
	self->SetSimpleRoamBox(box_size, move_distance, move_delay);
}

void Lua_NPC::RecalculateSkills()
{
	Lua_Safe_Call_Void();
	self->RecalculateSkills();
}

void Lua_NPC::ScaleNPC(uint8 npc_level)
{
	Lua_Safe_Call_Void();
	self->ScaleNPC(npc_level);
}

bool Lua_NPC::IsRaidTarget()
{
	Lua_Safe_Call_Bool();
	return self->IsRaidTarget();
}

void Lua_NPC::ChangeLastName(std::string last_name)
{
	Lua_Safe_Call_Void();
	self->ChangeLastName(last_name);
}

void Lua_NPC::ClearLastName()
{
	Lua_Safe_Call_Void();
	self->ClearLastName();
}

bool Lua_NPC::HasItem(uint32 item_id)
{
	Lua_Safe_Call_Bool();
	return self->HasItem(item_id);
}

uint16 Lua_NPC::CountItem(uint32 item_id)
{
	Lua_Safe_Call_Int();
	return self->CountItem(item_id);
}

uint32 Lua_NPC::GetItemIDBySlot(uint16 loot_slot)
{
	Lua_Safe_Call_Int();
	return self->GetItemIDBySlot(loot_slot);
}

uint16 Lua_NPC::GetFirstSlotByItemID(uint32 item_id)
{
	Lua_Safe_Call_Int();
	return self->GetFirstSlotByItemID(item_id);
}

float Lua_NPC::GetHealScale()
{
	Lua_Safe_Call_Real();
	return self->GetHealScale();
}

float Lua_NPC::GetSpellScale()
{
	Lua_Safe_Call_Real();
	return self->GetSpellScale();
}

void Lua_NPC::AddAISpellEffect(int spell_effect_id, int base_value, int limit_value, int max_value)
{
	Lua_Safe_Call_Void();
	self->AddSpellEffectToNPCList(spell_effect_id, base_value, limit_value, max_value, true);
}

void Lua_NPC::RemoveAISpellEffect(int spell_effect_id)
{
	Lua_Safe_Call_Void();
	self->RemoveSpellEffectFromNPCList(spell_effect_id, true);
}

bool Lua_NPC::HasAISpellEffect(int spell_effect_id)
{
	Lua_Safe_Call_Bool();
	return self->HasAISpellEffect(spell_effect_id);
}

float Lua_NPC::GetNPCStat(const char* identifier)
{
	Lua_Safe_Call_Real();
	return self->GetNPCStat(identifier);
}

bool Lua_NPC::IsRareSpawn()
{
	Lua_Safe_Call_Bool();
	return self->IsRareSpawn();
}

void Lua_NPC::ReloadSpells()
{
	Lua_Safe_Call_Void();
	self->ReloadSpells();
}
#endif
