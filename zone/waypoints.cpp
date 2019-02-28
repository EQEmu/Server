/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/global_define.h"
#ifdef _EQDEBUG
#include <iostream>
#endif

#include "../common/features.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/misc_functions.h"

#include "map.h"
#include "npc.h"
#include "quest_parser_collection.h"
#include "water_map.h"
#include "fastmath.h"
#include "mob_movement_manager.h"

#include <math.h>
#include <stdlib.h>

extern FastMath g_Math;

struct wp_distance
{
	float dist;
	int index;
};

void NPC::AI_SetRoambox(float max_distance, float roam_distance_variance, uint32 delay, uint32 min_delay) {
	AI_SetRoambox(
		max_distance,
		GetX() + roam_distance_variance,
		GetX() - roam_distance_variance,
		GetY() + roam_distance_variance,
		GetY() - roam_distance_variance,
		delay,
		min_delay
	);
}

void NPC::AI_SetRoambox(float distance, float max_x, float min_x, float max_y, float min_y, uint32 delay, uint32 min_delay) {
	roambox_distance      = distance;
	roambox_max_x         = max_x;
	roambox_min_x         = min_x;
	roambox_max_y         = max_y;
	roambox_min_y         = min_y;
	roambox_destination_x = roambox_max_x + 1; // this will trigger a recalc
	roambox_delay         = delay;
	roambox_min_delay     = min_delay;
}

void NPC::DisplayWaypointInfo(Client *c) {

	c->Message(0, "Mob is on grid %d, in spawn group %d, on waypoint %d/%d",
		GetGrid(),
		GetSp2(),
		GetCurWp(),
		GetMaxWp());


	std::vector<wplist>::iterator cur, end;
	cur = Waypoints.begin();
	end = Waypoints.end();
	for (; cur != end; ++cur) {
		c->Message(0, "Waypoint %d: (%.2f,%.2f,%.2f,%.2f) pause %d",
			cur->index,
			cur->x,
			cur->y,
			cur->z,
			cur->heading,
			cur->pause);
	}
}

void NPC::StopWandering()
{	// stops a mob from wandering, takes him off grid and sends him back to spawn point
	roamer = false;
	CastToNPC()->SetGrid(0);
	StopNavigation();
	Log(Logs::Detail, Logs::Pathing, "Stop Wandering requested.");
	return;
}

void NPC::ResumeWandering()
{	// causes wandering to continue - overrides waypoint pause timer and PauseWandering()
	if (!IsNPC())
		return;
	if (GetGrid() != 0)
	{
		if (GetGrid() < 0)
		{	// we were paused by a quest
			AI_walking_timer->Disable();
			SetGrid(0 - GetGrid());
			if (cur_wp == -1)
			{	// got here by a MoveTo()
				cur_wp = save_wp;
				UpdateWaypoint(cur_wp);	// have him head to last destination from here
			}
			Log(Logs::Detail, Logs::Pathing, "Resume Wandering requested. Grid %d, wp %d", GetGrid(), cur_wp);
		}
		else if (AI_walking_timer->Enabled())
		{	// we are at a waypoint paused normally
			Log(Logs::Detail, Logs::Pathing, "Resume Wandering on timed pause. Grid %d, wp %d", GetGrid(), cur_wp);
			AI_walking_timer->Trigger();	// disable timer to end pause now
		}
		else
		{
			Log(Logs::General, Logs::Error, "NPC not paused - can't resume wandering: %lu", (unsigned long)GetNPCTypeID());
			return;
		}

		if (m_CurrentWayPoint.x == GetX() && m_CurrentWayPoint.y == GetY())
		{	// are we we at a waypoint? if so, trigger event and start to next
			char temp[100];
			itoa(cur_wp, temp, 10);	//do this before updating to next waypoint
			CalculateNewWaypoint();
			SetAppearance(eaStanding, false);
			parse->EventNPC(EVENT_WAYPOINT_DEPART, this, nullptr, temp, 0);
		}	// if not currently at a waypoint, we continue on to the one we were headed to before the stop
	}
	else
	{
		Log(Logs::General, Logs::Error, "NPC not on grid - can't resume wandering: %lu", (unsigned long)GetNPCTypeID());
	}
	return;
}

void NPC::PauseWandering(int pausetime)
{	// causes wandering to stop but is resumable
	// 0 pausetime means pause until resumed
	// otherwise automatically resume when time is up
	if (GetGrid() != 0) {
		moving = false;
		DistractedFromGrid = true;
		Log(Logs::Detail, Logs::Pathing, "Paused Wandering requested. Grid %d. Resuming in %d ms (0=not until told)", GetGrid(), pausetime);
		StopNavigation();
		if (pausetime < 1) {	// negative grid number stops him dead in his tracks until ResumeWandering()
			SetGrid(0 - GetGrid());
		}
		else {	// specified waiting time, he'll resume after that
			AI_walking_timer->Start(pausetime * 1000); // set the timer
		}
	}
	else {
		Log(Logs::General, Logs::Error, "NPC not on grid - can't pause wandering: %lu", (unsigned long)GetNPCTypeID());
	}
	return;
}

void NPC::MoveTo(const glm::vec4& position, bool saveguardspot)
{	// makes mob walk to specified location
	if (IsNPC() && GetGrid() != 0)
	{	// he is on a grid
		if (GetGrid() < 0)
		{	// currently stopped by a quest command
			SetGrid(0 - GetGrid());	// get him moving again
			Log(Logs::Detail, Logs::AI, "MoveTo during quest wandering. Canceling quest wandering and going back to grid %d when MoveTo is done.", GetGrid());
		}
		AI_walking_timer->Disable();	// disable timer in case he is paused at a wp
		if (cur_wp >= 0)
		{	// we've not already done a MoveTo()
			save_wp = cur_wp;	// save the current waypoint
			cur_wp = -1;		// flag this move as quest controlled
		}
		Log(Logs::Detail, Logs::AI, "MoveTo %s, pausing regular grid wandering. Grid %d, save_wp %d", to_string(static_cast<glm::vec3>(position)).c_str(), -GetGrid(), save_wp);
	}
	else
	{	// not on a grid
		roamer = true;
		save_wp = 0;
		cur_wp = -2;		// flag as quest controlled w/no grid
		Log(Logs::Detail, Logs::AI, "MoveTo %s without a grid.", to_string(static_cast<glm::vec3>(position)).c_str());
	}

	glm::vec3 dest(position);

	m_CurrentWayPoint = position;
	m_CurrentWayPoint.z = GetFixedZ(dest);

	if (saveguardspot)
	{
		m_GuardPoint = m_CurrentWayPoint;

		if (m_GuardPoint.w == 0)
			m_GuardPoint.w = 0.0001;		//hack to make IsGuarding simpler

		if (m_GuardPoint.w == -1)
			m_GuardPoint.w = this->CalculateHeadingToTarget(position.x, position.y);

		Log(Logs::Detail, Logs::AI, "Setting guard position to %s", to_string(static_cast<glm::vec3>(m_GuardPoint)).c_str());
	}

	cur_wp_pause = 0;
	time_until_can_move = 0;
	if (AI_walking_timer->Enabled())
		AI_walking_timer->Start(100);
}

void NPC::UpdateWaypoint(int wp_index)
{
	if (wp_index >= static_cast<int>(Waypoints.size())) {
		Log(Logs::Detail, Logs::AI, "Update to waypoint %d failed. Not found.", wp_index);
		return;
	}
	std::vector<wplist>::iterator cur;
	cur = Waypoints.begin();
	cur += wp_index;

	m_CurrentWayPoint = glm::vec4(cur->x, cur->y, cur->z, cur->heading);
	cur_wp_pause = cur->pause;
	Log(Logs::Detail, Logs::AI, "Next waypoint %d: (%.3f, %.3f, %.3f, %.3f)", wp_index, m_CurrentWayPoint.x, m_CurrentWayPoint.y, m_CurrentWayPoint.z, m_CurrentWayPoint.w);

}

void NPC::CalculateNewWaypoint()
{
	int old_wp = cur_wp;
	bool reached_end = false;
	bool reached_beginning = false;
	if (cur_wp == max_wp)
		reached_end = true;
	if (cur_wp == 0)
		reached_beginning = true;

	switch (wandertype)
	{
	case 0: //circle
	{
		if (reached_end)
			cur_wp = 0;
		else
			cur_wp = cur_wp + 1;
		break;
	}
	case 1: //10 closest
	{
		std::list<wplist> closest;
		GetClosestWaypoint(closest, 10, glm::vec3(GetPosition()));
		auto iter = closest.begin();
		if (closest.size() != 0)
		{
			iter = closest.begin();
			std::advance(iter, zone->random.Int(0, closest.size() - 1));
			cur_wp = (*iter).index;
		}

		break;
	}
	case 2: //random
	{
		cur_wp = zone->random.Int(0, Waypoints.size() - 1);
		if (cur_wp == old_wp)
		{
			if (cur_wp == (Waypoints.size() - 1))
			{
				if (cur_wp > 0)
				{
					cur_wp--;
				}
			}
			else if (cur_wp == 0)
			{
				if ((Waypoints.size() - 1) > 0)
				{
					cur_wp++;
				}
			}
		}

		break;
	}
	case 3: //patrol
	{
		if (reached_end)
			patrol = 1;
		else if (reached_beginning)
			patrol = 0;
		if (patrol == 1)
			cur_wp = cur_wp - 1;
		else
			cur_wp = cur_wp + 1;

		break;
	}
	case 4: //goto the end and depop with spawn timer
	case 6: //goto the end and depop without spawn timer
	{
		cur_wp = cur_wp + 1;
		break;
	}
	case 5: //pick random closest 5 and pick one that's in sight
	{
		std::list<wplist> closest;
		GetClosestWaypoint(closest, 5, glm::vec3(GetPosition()));

		auto iter = closest.begin();
		while (iter != closest.end())
		{
			if (CheckLosFN((*iter).x, (*iter).y, (*iter).z, GetSize()))
			{
				++iter;
			}
			else
			{
				iter = closest.erase(iter);
			}
		}

		if (closest.size() != 0)
		{
			iter = closest.begin();
			std::advance(iter, zone->random.Int(0, closest.size() - 1));
			cur_wp = (*iter).index;
		}
		break;
	}
	}

	// Preserve waypoint setting for quest controlled NPCs
	if (cur_wp < 0)
		cur_wp = old_wp;

	// Check to see if we need to update the waypoint.
	if (cur_wp != old_wp)
		UpdateWaypoint(cur_wp);
}

bool wp_distance_pred(const wp_distance& left, const wp_distance& right)
{
	return left.dist < right.dist;
}

void NPC::GetClosestWaypoint(std::list<wplist> &wp_list, int count, const glm::vec3& location)
{
	wp_list.clear();
	if (Waypoints.size() <= count)
	{
		for (int i = 0; i < Waypoints.size(); ++i)
		{
			wp_list.push_back(Waypoints[i]);
		}
		return;
	}

	std::list<wp_distance> distances;
	for (int i = 0; i < Waypoints.size(); ++i)
	{
		float cur_x = (Waypoints[i].x - location.x);
		cur_x *= cur_x;
		float cur_y = (Waypoints[i].y - location.y);
		cur_y *= cur_y;
		float cur_z = (Waypoints[i].z - location.z);
		cur_z *= cur_z;
		float cur_dist = cur_x + cur_y + cur_z;
		wp_distance w_dist;
		w_dist.dist = cur_dist;
		w_dist.index = i;
		distances.push_back(w_dist);
	}
	distances.sort([](const wp_distance& a, const wp_distance& b) {
		return a.dist < b.dist;
	});

	auto iter = distances.begin();
	for (int i = 0; i < count; ++i)
	{
		wp_list.push_back(Waypoints[(*iter).index]);
		++iter;
	}
}

void NPC::SetWaypointPause()
{
	//Declare time to wait on current WP

	if(cur_wp_pause > 0)
	{
		switch (pausetype)
		{
		case 0: //Random Half
			AI_walking_timer->Start((cur_wp_pause - zone->random.Int(0, cur_wp_pause - 1) / 2) * 1000);
			break;
		case 1: //Full
			AI_walking_timer->Start(cur_wp_pause * 1000);
			break;
		case 2: //Random Full
			AI_walking_timer->Start(zone->random.Int(0, cur_wp_pause - 1) * 1000);
			break;
		}
	}
}

void NPC::SaveGuardSpot(const glm::vec4 &pos)
{
	m_GuardPoint = pos;

	if (m_GuardPoint.w == 0)
		m_GuardPoint.w = 0.0001;		//hack to make IsGuarding simpler
	LogF(Logs::Detail, Logs::AI, "Setting guard position to {0}", to_string(static_cast<glm::vec3>(m_GuardPoint)));
}

void NPC::NextGuardPosition() {
	NavigateTo(m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z);
	if (IsPositionEqualWithinCertainZ(m_Position, m_GuardPoint, 5.0f))
	{
		if (moved)
		{
			moved = false;
		}
	}
}

float Mob::CalculateDistance(float x, float y, float z) {
	return (float)sqrtf(((m_Position.x - x)*(m_Position.x - x)) + ((m_Position.y - y)*(m_Position.y - y)) + ((m_Position.z - z)*(m_Position.z - z)));
}

void Mob::WalkTo(float x, float y, float z)
{
	mMovementManager->NavigateTo(this, x, y, z, MovementWalking);
}

void Mob::RunTo(float x, float y, float z)
{
	mMovementManager->NavigateTo(this, x, y, z, MovementRunning);
}

void Mob::NavigateTo(float x, float y, float z)
{
	if (IsRunning()) {
		RunTo(x, y, z);
	}
	else {
		WalkTo(x, y, z);
	}
}

void Mob::RotateTo(float new_heading)
{
	if (IsRunning()) {
		RotateToRunning(new_heading);
	}
	else {
		RotateToWalking(new_heading);
	}
}

void Mob::RotateToWalking(float new_heading)
{
	mMovementManager->RotateTo(this, new_heading, MovementWalking);
}

void Mob::RotateToRunning(float new_heading)
{
	mMovementManager->RotateTo(this, new_heading, MovementRunning);
}

void Mob::StopNavigation() {
	mMovementManager->StopNavigation(this);
}

void NPC::AssignWaypoints(int32 grid)
{
	if (grid == 0)
		return; // grid ID 0 not supported

	if (grid < 0) {
		// Allow setting negative grid values for pausing pathing
		this->CastToNPC()->SetGrid(grid);
		return;
	}

	Waypoints.clear();
	roamer = false;

	// Retrieve the wander and pause types for this grid
	std::string query = StringFormat("SELECT `type`, `type2` FROM `grid` WHERE `id` = %i AND `zoneid` = %i", grid,
		zone->GetZoneID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	if (results.RowCount() == 0)
		return;

	auto row = results.begin();

	wandertype = atoi(row[0]);
	pausetype = atoi(row[1]);

	SetGrid(grid);	// Assign grid number

					// Retrieve all waypoints for this grid
	query = StringFormat("SELECT `x`,`y`,`z`,`pause`,`heading` "
		"FROM grid_entries WHERE `gridid` = %i AND `zoneid` = %i "
		"ORDER BY `number`", grid, zone->GetZoneID());
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	roamer = true;
	max_wp = 0;	// Initialize it; will increment it for each waypoint successfully added to the list

	for (auto row = results.begin(); row != results.end(); ++row, ++max_wp)
	{
		wplist newwp;
		newwp.index = max_wp;
		newwp.x = atof(row[0]);
		newwp.y = atof(row[1]);
		newwp.z = atof(row[2]);

		newwp.pause = atoi(row[3]);
		newwp.heading = atof(row[4]);
		Waypoints.push_back(newwp);
	}

	UpdateWaypoint(0);
	SetWaypointPause();

	if (wandertype == 1 || wandertype == 2 || wandertype == 5)
		CalculateNewWaypoint();
}

void Mob::SendTo(float new_x, float new_y, float new_z) {
	if (IsNPC()) {
		entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
	}

	m_Position.x = new_x;
	m_Position.y = new_y;
	m_Position.z = new_z;
	Log(Logs::Detail, Logs::AI, "Sent To (%.3f, %.3f, %.3f)", new_x, new_y, new_z);

	if (flymode == GravityBehavior::Flying)
		return;

	//fix up pathing Z, this shouldent be needed IF our waypoints
	//are corrected instead
	if (zone->HasMap() && RuleB(Map, FixPathingZOnSendTo))
	{
		if (!RuleB(Watermap, CheckForWaterOnSendTo) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(glm::vec3(m_Position))))
		{
			glm::vec3 dest(m_Position.x, m_Position.y, m_Position.z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			Log(Logs::Detail, Logs::AI, "BestZ returned %4.3f at %4.3f, %4.3f, %4.3f", newz, m_Position.x, m_Position.y, m_Position.z);

			if ((newz > -2000) && std::abs(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaSendTo)) // Sanity check.
				m_Position.z = newz + 1;
		}
	}
	else
		m_Position.z += 0.1;
}

void Mob::SendToFixZ(float new_x, float new_y, float new_z) {
	if (IsNPC()) {
		entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z + 0.1);
	}

	m_Position.x = new_x;
	m_Position.y = new_y;
	m_Position.z = new_z + 0.1;

	if (zone->HasMap() && RuleB(Map, FixPathingZOnSendTo))
	{
		if (!RuleB(Watermap, CheckForWaterOnSendTo) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(glm::vec3(m_Position))))
		{
			glm::vec3 dest(m_Position.x, m_Position.y, m_Position.z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			Log(Logs::Moderate, Logs::Pathing, "BestZ returned %4.3f at %4.3f, %4.3f, %4.3f", newz, m_Position.x, m_Position.y, m_Position.z);

			if ((newz > -2000) && std::abs(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaSendTo)) // Sanity check.
				m_Position.z = newz + 1;
		}
	}
}

float Mob::GetFixedZ(const glm::vec3 &destination, int32 z_find_offset) {
	BenchTimer timer;
	timer.reset();

	float new_z = destination.z;

	if (zone->HasMap()) {

		if (flymode == GravityBehavior::Flying)
			return new_z;

		if (zone->HasWaterMap() && zone->watermap->InLiquid(glm::vec3(m_Position)))
			return new_z;

		/*
		 * Any more than 5 in the offset makes NPC's hop/snap to ceiling in small corridors
		 */
		new_z = this->FindDestGroundZ(destination, z_find_offset);
		if (new_z != BEST_Z_INVALID) {
			new_z += this->GetZOffset();

			if (new_z < -2000) {
				new_z = m_Position.z;
			}
		}

		auto duration = timer.elapsed();

		Log(Logs::Moderate,
			Logs::FixZ,
			"Mob::GetFixedZ() (%s) returned %4.3f at %4.3f, %4.3f, %4.3f - Took %lf",
			this->GetCleanName(),
			new_z,
			destination.x,
			destination.y,
			destination.z,
			duration);
	}

	return new_z;
}

void Mob::FixZ(int32 z_find_offset /*= 5*/, bool fix_client_z /*= false*/) {
	if (IsClient() && !fix_client_z) {
		return;
	}
	
	if (flymode == GravityBehavior::Flying) {
		return;
	}

	if (zone->watermap && zone->watermap->InLiquid(m_Position)) {
		return;
	}

	glm::vec3 current_loc(m_Position);
	float new_z = GetFixedZ(current_loc, z_find_offset);

	if (new_z == m_Position.z)
		return;

	if ((new_z > -2000) && new_z != BEST_Z_INVALID) {
		if (RuleB(Map, MobZVisualDebug)) {
			this->SendAppearanceEffect(78, 0, 0, 0, 0);
		}

		m_Position.z = new_z;
	}
	else {
		if (RuleB(Map, MobZVisualDebug)) {
			this->SendAppearanceEffect(103, 0, 0, 0, 0);
		}

		Log(Logs::General,
			Logs::FixZ,
			"%s is failing to find Z %f",
			this->GetCleanName(),
			std::abs(m_Position.z - new_z));
	}
}

float Mob::GetZOffset() const {
	float offset = 3.125f;

	switch (race) {
		case RACE_BASILISK_436:
			offset = 0.577f;
			break;
		case RACE_DRAKE_430:
			offset = 0.5f;
			break;
		case RACE_DRAKE_432:
			offset = 1.9f;
			break;
		case RACE_DRAGON_435:
			offset = 0.93f;
			break;
		case RACE_LAVA_SPIDER_450:
			offset = 0.938f;
			break;
		case RACE_ALLIGATOR_479:
			offset = 0.8f;
			break;
		case RACE_LAVA_SPIDER_QUEEN_451:
			offset = 0.816f;
			break;
		case RACE_DRAGON_437:
			offset = 0.527f;
			break;
		case RACE_PUMA_439:
			offset = 1.536f;
			break;
		case RACE_RAT_415:
			offset = 1.0f;
			break;
		case RACE_DRAGON_438:
			offset = 0.776f;
			break;
		case RACE_DRAGON_452:
			offset = 0.776f;
			break;
		case RACE_SPIDER_QUEEN_441:
			offset = 0.816f;
			break;
		case RACE_SPIDER_440:
			offset = 0.938f;
			break;
		case RACE_SNAKE_468:
			offset = 1.0f;
			break;
		case RACE_CORATHUS_459:
			offset = 1.0f;
			break;
		case RACE_DRACHNID_COCOON_462:
			offset = 1.5f;
			break;
		case RACE_DRAGON_530:
			offset = 1.2f;
			break;
		case RACE_GOO_549:
			offset = 0.5f;
			break;
		case RACE_GOO_548:
			offset = 0.5f;
			break;
		case RACE_GOO_547:
			offset = 0.5f;
			break;
		case RACE_DRACOLICH_604:
			offset = 1.2f;
			break;
		case RACE_TELMIRA_653:
			offset = 5.9f;
			break;
		case RACE_MORELL_THULE_658:
			offset = 4.0f;
			break;
		case RACE_ARMOR_OF_MARR_323:
			offset = 5.0f;
			break;
		case RACE_AMYGDALAN_663:
			offset = 5.0f;
			break;
		case RACE_SANDMAN_664:
			offset = 4.0f;
			break;
		case RACE_ALARAN_SENTRY_STONE_703:
			offset = 9.0f;
			break;
		case RACE_RABBIT_668:
			offset = 5.0f;
			break;
		case RACE_BLIND_DREAMER_669:
			offset = 7.0f;
			break;
		case RACE_GORAL_687:
			offset = 2.0f;
			break;
		case RACE_SELYRAH_686:
			offset = 2.0f;
			break;
		default:
			offset = 3.125f;
	}

	float mob_size = (GetSize() > 0 ? GetSize() : GetDefaultRaceSize());

	return static_cast<float>(0.2 * mob_size * offset);
}

// This function will try to move the mob along the relative angle a set distance
// if it can't be moved, it will lower the distance and try again
// If we want to move on like say a spawn, we can pass send as false
void Mob::TryMoveAlong(float distance, float angle, bool send)
{
	angle += GetHeading();
	angle = FixHeading(angle);

	glm::vec3 tmp_pos;
	glm::vec3 new_pos = GetPosition();
	new_pos.x += distance * g_Math.FastSin(angle);
	new_pos.y += distance * g_Math.FastCos(angle);
	new_pos.z += GetZOffset();

	if (zone->HasMap()) {
		auto new_z = zone->zonemap->FindClosestZ(new_pos, nullptr);
		if (new_z != BEST_Z_INVALID)
			new_pos.z = new_z;

		if (zone->zonemap->LineIntersectsZone(GetPosition(), new_pos, 0.0f, &tmp_pos))
			new_pos = tmp_pos;
	}

	new_pos.z = GetFixedZ(new_pos);
	Teleport(new_pos);
}

int	ZoneDatabase::GetHighestGrid(uint32 zoneid) {

	std::string query = StringFormat("SELECT COALESCE(MAX(id), 0) FROM grid WHERE zoneid = %i", zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint8 ZoneDatabase::GetGridType2(uint32 grid, uint16 zoneid) {

	int type2 = 0;
	std::string query = StringFormat("SELECT type2 FROM grid WHERE id = %i AND zoneid = %i", grid, zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

bool ZoneDatabase::GetWaypoints(uint32 grid, uint16 zoneid, uint32 num, wplist* wp) {

	if (wp == nullptr)
		return false;

	std::string query = StringFormat("SELECT x, y, z, pause, heading FROM grid_entries "
		"WHERE gridid = %i AND number = %i AND zoneid = %i", grid, num, zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (results.RowCount() != 1)
		return false;

	auto row = results.begin();

	wp->x = atof(row[0]);
	wp->y = atof(row[1]);
	wp->z = atof(row[2]);
	wp->pause = atoi(row[3]);
	wp->heading = atof(row[4]);

	return true;
}

void ZoneDatabase::AssignGrid(Client *client, int grid, int spawn2id) {
	std::string query = StringFormat("UPDATE spawn2 SET pathgrid = %d WHERE id = %d", grid, spawn2id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	if (results.RowsAffected() != 1) {
		return;
	}

	client->Message(0, "Grid assign: spawn2 id = %d updated", spawn2id);
}


/******************
* ModifyGrid - Either adds an empty grid, or removes a grid and all its waypoints, for a particular zone.
*	remove:		TRUE if we are deleting the specified grid, FALSE if we are adding it
*	id:		The ID# of the grid to add or delete
*	type,type2:	The type and type2 values for the grid being created (ignored if grid is being deleted)
*	zoneid:		The ID number of the zone the grid is being created/deleted in
*/
void ZoneDatabase::ModifyGrid(Client *client, bool remove, uint32 id, uint8 type, uint8 type2, uint16 zoneid) {

	if (!remove)
	{
		std::string query = StringFormat("INSERT INTO grid(id, zoneid, type, type2) "
			"VALUES (%i, %i, %i, %i)", id, zoneid, type, type2);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		return;
	}

	std::string query = StringFormat("DELETE FROM grid where id=%i", id);
	auto results = QueryDatabase(query);

	query = StringFormat("DELETE FROM grid_entries WHERE zoneid = %i AND gridid = %i", zoneid, id);
	results = QueryDatabase(query);
}

/**************************************
* AddWP - Adds a new waypoint to a specific grid for a specific zone.
*/
void ZoneDatabase::AddWP(Client *client, uint32 gridid, uint32 wpnum, const glm::vec4& position, uint32 pause, uint16 zoneid)
{
	std::string query = StringFormat("INSERT INTO grid_entries (gridid, zoneid, `number`, x, y, z, pause, heading) "
		"VALUES (%i, %i, %i, %f, %f, %f, %i, %f)",
		gridid, zoneid, wpnum, position.x, position.y, position.z, pause, position.w);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}
}


/**********
* ModifyWP() has been obsoleted. The #wp command either uses AddWP() or DeleteWaypoint()
***********/

/******************
* DeleteWaypoint - Removes a specific waypoint from the grid
*	grid_id:	The ID number of the grid whose wp is being deleted
*	wp_num:		The number of the waypoint being deleted
*	zoneid:		The ID number of the zone that contains the waypoint being deleted
*/
void ZoneDatabase::DeleteWaypoint(Client *client, uint32 grid_num, uint32 wp_num, uint16 zoneid)
{
	std::string query = StringFormat("DELETE FROM grid_entries WHERE "
		"gridid = %i AND zoneid = %i AND `number` = %i",
		grid_num, zoneid, wp_num);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}
}


/******************
* AddWPForSpawn - Used by the #wpadd command - for a given spawn, this will add a new waypoint to whatever grid that spawn is assigned to.
* If there is currently no grid assigned to the spawn, a new grid will be created using the next available Grid ID number for the zone
* the spawn is in.
* Returns 0 if the function didn't have to create a new grid. If the function had to create a new grid for the spawn, then the ID of
* the created grid is returned.
*/
uint32 ZoneDatabase::AddWPForSpawn(Client *client, uint32 spawn2id, const glm::vec4& position, uint32 pause, int type1, int type2, uint16 zoneid) {

	uint32 grid_num;	 // The grid number the spawn is assigned to (if spawn has no grid, will be the grid number we end up creating)
	uint32 next_wp_num;	 // The waypoint number we should be assigning to the new waypoint
	bool createdNewGrid; // Did we create a new grid in this function?

						 // See what grid number our spawn is assigned
	std::string query = StringFormat("SELECT pathgrid FROM spawn2 WHERE id = %i", spawn2id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		// Query error
		return 0;
	}

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	grid_num = atoi(row[0]);

	if (grid_num == 0)
	{ // Our spawn doesn't have a grid assigned to it -- we need to create a new grid and assign it to the spawn
		createdNewGrid = true;
		grid_num = GetFreeGrid(zoneid);
		if (grid_num == 0)	// There are no grids for the current zone -- create Grid #1
			grid_num = 1;

		query = StringFormat("INSERT INTO grid SET id = '%i', zoneid = %i, type ='%i', type2 = '%i'",
			grid_num, zoneid, type1, type2);
		QueryDatabase(query);

		query = StringFormat("UPDATE spawn2 SET pathgrid = '%i' WHERE id = '%i'", grid_num, spawn2id);
		QueryDatabase(query);

	}
	else	// NPC had a grid assigned to it
		createdNewGrid = false;

	// Find out what the next waypoint is for this grid
	query = StringFormat("SELECT max(`number`) FROM grid_entries WHERE zoneid = '%i' AND gridid = '%i'", zoneid, grid_num);

	results = QueryDatabase(query);
	if (!results.Success()) { // Query error
		return 0;
	}

	row = results.begin();
	if (row[0] != 0)
		next_wp_num = atoi(row[0]) + 1;
	else	// No waypoints in this grid yet
		next_wp_num = 1;

	query = StringFormat("INSERT INTO grid_entries(gridid, zoneid, `number`, x, y, z, pause, heading) "
		"VALUES (%i, %i, %i, %f, %f, %f, %i, %f)",
		grid_num, zoneid, next_wp_num, position.x, position.y, position.z, pause, position.w);
	results = QueryDatabase(query);

	return createdNewGrid ? grid_num : 0;
}

uint32 ZoneDatabase::GetFreeGrid(uint16 zoneid) {

	std::string query = StringFormat("SELECT max(id) FROM grid WHERE zoneid = %i", zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	uint32 freeGridID = row[0] ? atoi(row[0]) + 1 : 1;

	return freeGridID;
}

int ZoneDatabase::GetHighestWaypoint(uint32 zoneid, uint32 gridid) {

	std::string query = StringFormat("SELECT COALESCE(MAX(number), 0) FROM grid_entries "
		"WHERE zoneid = %i AND gridid = %i", zoneid, gridid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

void NPC::SaveGuardSpotCharm()
{
	m_GuardPointSaved = m_GuardPoint;
}

void NPC::RestoreGuardSpotCharm()
{
	m_GuardPoint = m_GuardPointSaved;
}

/******************
* Bot-specific overloads to make them play nice with the new movement system
*/
#ifdef BOTS
#include "bot.h"

void Bot::WalkTo(float x, float y, float z)
{
	if (IsSitting())
		Stand();

	Mob::WalkTo(x, y, z);
}

void Bot::RunTo(float x, float y, float z)
{
	if (IsSitting())
		Stand();

	Mob::RunTo(x, y, z);
}
#endif
