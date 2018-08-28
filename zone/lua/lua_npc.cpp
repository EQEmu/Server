#include <sol.hpp>
#include "../npc.h"

void npc_add_item(NPC &n, uint32 item_id, uint16 charges) {
	n.AddItem(item_id, charges);
}

void npc_add_item1(NPC &n, uint32 item_id, uint16 charges, bool equipitem) {
	n.AddItem(item_id, charges, equipitem);
}

void npc_add_item2(NPC &n, uint32 item_id, uint16 charges, bool equipitem, uint32 aug1) {
	n.AddItem(item_id, charges, equipitem, aug1);
}

void npc_add_item3(NPC &n, uint32 item_id, uint16 charges, bool equipitem, uint32 aug1, uint32 aug2) {
	n.AddItem(item_id, charges, equipitem, aug1, aug2);
}

void npc_add_item4(NPC &n, uint32 item_id, uint16 charges, bool equipitem, uint32 aug1, uint32 aug2, uint32 aug3) {
	n.AddItem(item_id, charges, equipitem, aug1, aug2, aug3);
}

void npc_add_item5(NPC &n, uint32 item_id, uint16 charges, bool equipitem, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4) {
	n.AddItem(item_id, charges, equipitem, aug1, aug2, aug3, aug4);
}

void npc_add_item6(NPC &n, uint32 item_id, uint16 charges, bool equipitem, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	n.AddItem(item_id, charges, equipitem, aug1, aug2, aug3, aug4, aug5);
}

void npc_add_item7(NPC &n, uint32 item_id, uint16 charges, bool equipitem, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6) {
	n.AddItem(item_id, charges, equipitem, aug1, aug2, aug3, aug4, aug5, aug6);
}

void npc_add_loot_table(NPC &n) {
	n.AddLootTable();
}

void npc_add_loot_table1(NPC &n, uint32 ltid) {
	n.AddLootTable(ltid);
}

void npc_remove_item(NPC &n, uint32 item_id) {
	n.RemoveItem(item_id);
}

void npc_remove_item1(NPC &n, uint32 item_id, uint16 quantity) {
	n.RemoveItem(item_id, quantity);
}

void npc_remove_item2(NPC &n, uint32 item_id, uint16 quantity, uint16 slot) {
	n.RemoveItem(item_id, quantity, slot);
}

void npc_add_cash(NPC &n, uint16 copper, uint16 silver, uint16 gold, uint16 plat) {
	n.AddCash(copper, silver, gold, plat);
}

void npc_save_guard_spot(NPC &n) {
	n.SaveGuardSpot();
}

void npc_save_guard_spot1(NPC &n, bool clear_guard_spot) {
	n.SaveGuardSpot(clear_guard_spot);
}

void npc_ai_set_roambox(NPC &n, float max_distance, float variance) {
	n.AI_SetRoambox(max_distance, variance);
}

void npc_ai_set_roambox1(NPC &n, float max_distance, float variance, uint32 delay) {
	n.AI_SetRoambox(max_distance, variance, delay);
}

void npc_ai_set_roambox2(NPC &n, float max_distance, float variance, uint32 delay, uint32 min_delay) {
	n.AI_SetRoambox(max_distance, variance, delay, min_delay);
}

float npc_get_spawn_point_x(NPC &n) {
	return n.GetSpawnPoint().x;
}

float npc_get_spawn_point_y(NPC &n) {
	return n.GetSpawnPoint().y;
}

float npc_get_spawn_point_z(NPC &n) {
	return n.GetSpawnPoint().z;
}

float npc_get_spawn_point_h(NPC &n) {
	return n.GetSpawnPoint().w;
}

float npc_get_guard_point_x(NPC &n) {
	return n.GetGuardPoint().x;
}

float npc_get_guard_point_y(NPC &n) {
	return n.GetGuardPoint().y;
}

float npc_get_guard_point_z(NPC &n) {
	return n.GetGuardPoint().z;
}

float npc_get_guard_point_h(NPC &n) {
	return n.GetGuardPoint().w;
}

void npc_add_ai_spell(NPC &n, int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust) {
	return n.AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, 0, 0);
}

void npc_add_ai_spell1(NPC &n, int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust, int min_hp) {
	return n.AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, min_hp, 0);
}

void npc_add_ai_spell2(NPC &n, int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust, int min_hp, int max_hp) {
	return n.AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, min_hp, max_hp);
}

void lua_register_npc(sol::state *state) {
	auto npc = state->create_simple_usertype<NPC>(sol::base_classes, sol::bases<Mob, Entity>());

	npc.set("Signal", &NPC::Signal);
	npc.set("CheckNPCFactionAlly", &NPC::CheckNPCFactionAlly);
	npc.set("AddItem", sol::overload(
		&npc_add_item,
		&npc_add_item1,
		&npc_add_item2,
		&npc_add_item3,
		&npc_add_item4,
		&npc_add_item5,
		&npc_add_item6,
		&npc_add_item7
	));
	npc.set("AddLootTable", sol::overload(
		&npc_add_loot_table,
		&npc_add_loot_table1
	));
	npc.set("RemoveItem", sol::overload(
		&npc_remove_item,
		&npc_remove_item1,
		&npc_remove_item2
	));
	npc.set("ClearItemList", &NPC::ClearItemList);
	npc.set("AddCash", &npc_add_cash);
	npc.set("RemoveCash", &NPC::RemoveCash);
	npc.set("CountLoot", &NPC::CountLoot);
	npc.set("GetLoottableID", &NPC::GetLoottableID);
	npc.set("GetCopper", &NPC::GetCopper);
	npc.set("GetSilver", &NPC::GetSilver);
	npc.set("GetGold", &NPC::GetGold);
	npc.set("GetPlatinum", &NPC::GetPlatinum);
	npc.set("SetCopper", &NPC::SetCopper);
	npc.set("SetSilver", &NPC::SetSilver);
	npc.set("SetGold", &NPC::SetGold);
	npc.set("SetPlatinum", &NPC::SetPlatinum);
	npc.set("SetGrid", &NPC::SetGrid);
	npc.set("SetSaveWaypoint", &NPC::SetSaveWaypoint);
	npc.set("SetSp2", &NPC::SetSp2);
	npc.set("GetWaypointMax", &NPC::GetWaypointMax);
	npc.set("GetGrid", &NPC::GetGrid);
	npc.set("GetSp2", &NPC::GetSp2);
	npc.set("GetNPCFactionID", &NPC::GetNPCFactionID);
	npc.set("GetPrimaryFaction", &NPC::GetPrimaryFaction);
	npc.set("GetNPCHate", &NPC::GetNPCHate);
	npc.set("IsOnHatelist", &NPC::IsOnHatelist);
	npc.set("SetNPCFactionID", &NPC::SetNPCFactionID);
	npc.set("GetMaxDMG", &NPC::GetMaxDMG);
	npc.set("GetMinDMG", &NPC::GetMinDMG);
	npc.set("IsAnimal", &NPC::IsAnimal);
	npc.set("GetPetSpellID", &NPC::GetPetSpellID);
	npc.set("SetPetSpellID", &NPC::SetPetSpellID);
	npc.set("GetMaxDamage", &NPC::GetMaxDamage);
	npc.set("SetTaunting", &NPC::SetTaunting);
	npc.set("PickPocket", &NPC::PickPocket);
	npc.set("StartSwarmTimer", &NPC::StartSwarmTimer);
	npc.set("DoClassAttacks", &NPC::DoClassAttacks);
	npc.set("GetMaxWp", &NPC::GetMaxWp);
	npc.set("DisplayWaypointInfo", &NPC::DisplayWaypointInfo);
	npc.set("CalculateNewWaypoint", &NPC::CalculateNewWaypoint);
	npc.set("AssignWaypoints", &NPC::AssignWaypoints);
	npc.set("SetWaypointPause", &NPC::SetWaypointPause);
	npc.set("UpdateWaypoint", &NPC::UpdateWaypoint);
	npc.set("StopWandering", &NPC::StopWandering);
	npc.set("ResumeWandering", &NPC::ResumeWandering);
	npc.set("PauseWandering", &NPC::PauseWandering);
	npc.set("MoveTo", &NPC::MoveTo);
	npc.set("NextGuardPosition", &NPC::NextGuardPosition);
	npc.set("SaveGuardSpot", sol::overload(
		&npc_save_guard_spot,
		&npc_save_guard_spot1
	));
	npc.set("IsGuarding", &NPC::IsGuarding);
	npc.set("AI_SetRoambox", sol::overload(
		&npc_ai_set_roambox,
		&npc_ai_set_roambox1
	));
	npc.set("GetNPCSpellsID", &NPC::GetNPCSpellsID);
	npc.set("GetSpawnPointID", &NPC::GetSpawnPointID);
	npc.set("GetSpawnPointX", &npc_get_spawn_point_x);
	npc.set("GetSpawnPointY", &npc_get_spawn_point_y);
	npc.set("GetSpawnPointZ", &npc_get_spawn_point_z);
	npc.set("GetSpawnPointH", &npc_get_spawn_point_h);
	npc.set("GetGuardPointX", &npc_get_guard_point_x);
	npc.set("GetGuardPointY", &npc_get_guard_point_y);
	npc.set("GetGuardPointZ", &npc_get_guard_point_z);
	npc.set("GetGuardPointH", &npc_get_guard_point_h);
	npc.set("SetPrimSkill", &NPC::SetPrimSkill);
	npc.set("SetSecSkill", &NPC::SetSecSkill);
	npc.set("GetPrimSkill", &NPC::GetPrimSkill);
	npc.set("GetSecSkill", &NPC::GetSecSkill);
	npc.set("GetSwarmOwner", &NPC::GetSwarmOwner);
	npc.set("GetSwarmTarget", &NPC::GetSwarmTarget);
	npc.set("SetSwarmTarget", &NPC::SetSwarmTarget);
	npc.set("ModifyNPCStat", &NPC::ModifyNPCStat);
	npc.set("AI_SetRoambox", sol::overload(
		&npc_add_ai_spell,
		&npc_add_ai_spell1,
		&npc_add_ai_spell2
	));
	npc.set("RemoveAISpell", &NPC::RemoveSpellFromNPCList);
	npc.set("SetSpellFocusDMG", &NPC::SetSpellFocusDMG);
	npc.set("SetSpellFocusHeal", &NPC::SetSpellFocusHeal);
	npc.set("GetSpellFocusDMG", &NPC::GetSpellFocusDMG);
	npc.set("GetSpellFocusHeal", &NPC::GetSpellFocusHeal);
	npc.set("GetSlowMitigation", &NPC::GetSlowMitigation);
	npc.set("GetAttackSpeed", &NPC::GetAttackSpeed);
	npc.set("GetAttackDelay", &NPC::GetAttackDelay);
	npc.set("GetAccuracyRating", &NPC::GetAccuracyRating);
	npc.set("GetSpawnKillCount", &NPC::GetSpawnKillCount);
	npc.set("GetScore", &NPC::GetScore);
	npc.set("MerchantOpenShop", &NPC::MerchantOpenShop);
	npc.set("MerchantCloseShop", &NPC::MerchantCloseShop);
	npc.set("GetRawAC", &NPC::GetRawAC);
	npc.set("GetAvoidanceRating", &NPC::GetAvoidanceRating);
	state->set_usertype("NPC", npc);
}
