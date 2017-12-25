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

#include <math.h>
#include <stdlib.h>

struct wp_distance
{
	float dist;
	int index;
};

void NPC::AI_SetRoambox(float iDist, float iRoamDist, uint32 iDelay, uint32 iMinDelay) {
	AI_SetRoambox(iDist, GetX() + iRoamDist, GetX() - iRoamDist, GetY() + iRoamDist, GetY() - iRoamDist, iDelay, iMinDelay);
}

void NPC::AI_SetRoambox(float iDist, float iMaxX, float iMinX, float iMaxY, float iMinY, uint32 iDelay, uint32 iMinDelay) {
	roambox_distance = iDist;
	roambox_max_x = iMaxX;
	roambox_min_x = iMinX;
	roambox_max_y = iMaxY;
	roambox_min_y = iMinY;
	roambox_movingto_x = roambox_max_x + 1; // this will trigger a recalc
	roambox_delay = iDelay;
	roambox_min_delay = iMinDelay;
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
	SendPosition();
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
		SendPosition();
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
	pLastFightingDelayMoving = 0;
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

	tar_ndx = 52;

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

	if (cur_wp_pause == 0) {
		AI_walking_timer->Start(100);
		AI_walking_timer->Trigger();
	}
	else
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

void NPC::SaveGuardSpot(bool iClearGuardSpot) {
	if (iClearGuardSpot) {
		Log(Logs::Detail, Logs::AI, "Clearing guard order.");
		m_GuardPoint = glm::vec4();
	}
	else {
		m_GuardPoint = m_Position;

		if (m_GuardPoint.w == 0)
			m_GuardPoint.w = 0.0001;		//hack to make IsGuarding simpler
		Log(Logs::Detail, Logs::AI, "Setting guard position to %s", to_string(static_cast<glm::vec3>(m_GuardPoint)).c_str());
	}
}

void NPC::NextGuardPosition() {
	if (!CalculateNewPosition2(m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z, GetMovespeed())) {
		SetHeading(m_GuardPoint.w);
		Log(Logs::Detail, Logs::AI, "Unable to move to next guard position. Probably rooted.");
	}
	else if ((m_Position.x == m_GuardPoint.x) && (m_Position.y == m_GuardPoint.y) && (m_Position.z == m_GuardPoint.z))
	{
		if (moved)
		{
			moved = false;
			SetCurrentSpeed(0);
		}
	}
}

float Mob::CalculateDistance(float x, float y, float z) {
	return (float)sqrtf(((m_Position.x - x)*(m_Position.x - x)) + ((m_Position.y - y)*(m_Position.y - y)) + ((m_Position.z - z)*(m_Position.z - z)));
}

float Mob::CalculateHeadingToTarget(float in_x, float in_y) {
	float angle;

	if (in_x - m_Position.x > 0)
		angle = -90 + atan((float)(in_y - m_Position.y) / (float)(in_x - m_Position.x)) * 180 / M_PI;
	else if (in_x - m_Position.x < 0)
		angle = +90 + atan((float)(in_y - m_Position.y) / (float)(in_x - m_Position.x)) * 180 / M_PI;
	else // Added?
	{
		if (in_y - m_Position.y > 0)
			angle = 0;
		else
			angle = 180;
	}
	if (angle < 0)
		angle += 360;
	if (angle > 360)
		angle -= 360;
	return (256 * (360 - angle) / 360.0f);
}

bool Mob::MakeNewPositionAndSendUpdate(float x, float y, float z, int speed) {
	if (GetID() == 0)
		return true;

	if (speed <= 0)
	{
		SetCurrentSpeed(0);
		return true;
	}

	if ((m_Position.x - x == 0) && (m_Position.y - y == 0)) {//spawn is at target coords
		if (m_Position.z - z != 0) {
			m_Position.z = z;
			Log(Logs::Detail, Logs::AI, "Calc Position2 (%.3f, %.3f, %.3f): Jumping pure Z.", x, y, z);
			return true;
		}
		return false;
	}
	else if ((std::abs(m_Position.x - x) < 0.1) && (std::abs(m_Position.y - y) < 0.1)) {
		if (IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), x, y, z);
		}

		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
		return true;
	}

	bool send_update = false;
	int compare_steps = 20;
	if (tar_ndx < compare_steps && m_TargetLocation.x == x && m_TargetLocation.y == y) {

		float new_x = m_Position.x + m_TargetV.x*tar_vector;
		float new_y = m_Position.y + m_TargetV.y*tar_vector;
		float new_z = m_Position.z + m_TargetV.z*tar_vector;
		if (IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
		}

		m_Position.x = new_x;
		m_Position.y = new_y;
		m_Position.z = new_z;

		if(fix_z_timer.Check() && 
			(!this->IsEngaged() || flee_mode || currently_fleeing))
			this->FixZ();

		tar_ndx++;
		return true;
	}


	if (tar_ndx>50) {
		tar_ndx--;
	}
	else {
		tar_ndx = 0;
	}
	m_TargetLocation = glm::vec3(x, y, z);

	float nx = this->m_Position.x;
	float ny = this->m_Position.y;
	float nz = this->m_Position.z;
	//	float nh = this->heading;

	m_TargetV.x = x - nx;
	m_TargetV.y = y - ny;
	m_TargetV.z = z - nz;
	SetCurrentSpeed((int8)speed);
	pRunAnimSpeed = speed;
#ifdef BOTS
	if (IsClient() || IsBot())
#else
	if (IsClient())
#endif
	{
		animation = speed / 2;
	}

	// --------------------------------------------------------------------------
	// 2: get unit vector
	// --------------------------------------------------------------------------
	float mag = sqrtf(m_TargetV.x*m_TargetV.x + m_TargetV.y*m_TargetV.y + m_TargetV.z*m_TargetV.z);
	tar_vector = (float)speed / mag;

	// mob move fix
	int numsteps = (int)(mag * 13.5f / (float)speed + 0.5f);


	// mob move fix

	if (numsteps<20)
	{
		if (numsteps>1)
		{
			tar_vector = 1.0f;
			m_TargetV.x = m_TargetV.x / (float)numsteps;
			m_TargetV.y = m_TargetV.y / (float)numsteps;
			m_TargetV.z = m_TargetV.z / (float)numsteps;

			float new_x = m_Position.x + m_TargetV.x;
			float new_y = m_Position.y + m_TargetV.y;
			float new_z = m_Position.z + m_TargetV.z;
			if (IsNPC()) {
				entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
			}

			m_Position.x = new_x;
			m_Position.y = new_y;
			m_Position.z = new_z;
			m_Position.w = CalculateHeadingToTarget(x, y);
			tar_ndx = 20 - numsteps;
		}
		else
		{
			if (IsNPC()) {
				entity_list.ProcessMove(CastToNPC(), x, y, z);
			}

			m_Position.x = x;
			m_Position.y = y;
			m_Position.z = z;
		}
	}

	else {
		tar_vector /= 13.5f;
		float dur = Timer::GetCurrentTime() - pLastChange;
		if (dur < 0.0f) {
			dur = 0.0f;
		}

		if (dur > 100.f) {
			dur = 100.f;
		}

		tar_vector *= (dur / 100.0f);

		float new_x = m_Position.x + m_TargetV.x*tar_vector;
		float new_y = m_Position.y + m_TargetV.y*tar_vector;
		float new_z = m_Position.z + m_TargetV.z*tar_vector;
		if (IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
		}

		m_Position.x = new_x;
		m_Position.y = new_y;
		m_Position.z = new_z;
		m_Position.w = CalculateHeadingToTarget(x, y);
	}

	if (fix_z_timer.Check() && !this->IsEngaged())
		this->FixZ();

	SetMoving(true);
	moved = true;

	m_Delta = glm::vec4(m_Position.x - nx, m_Position.y - ny, m_Position.z - nz, 0.0f);

	if (IsClient())
	{
		SendPositionUpdate(1);
		CastToClient()->ResetPositionTimer();
	}
	else
	{
		SendPositionUpdate();
		SetAppearance(eaStanding, false);
	}

	pLastChange = Timer::GetCurrentTime();
	return true;
}

bool Mob::CalculateNewPosition2(float x, float y, float z, int speed, bool checkZ, bool calcHeading) {
	return MakeNewPositionAndSendUpdate(x, y, z, speed);
}

bool Mob::CalculateNewPosition(float x, float y, float z, int speed, bool checkZ, bool calcHeading) {
	if (GetID() == 0)
		return true;

	float nx = m_Position.x;
	float ny = m_Position.y;
	float nz = m_Position.z;

	// if NPC is rooted
	if (speed == 0) {
		SetHeading(CalculateHeadingToTarget(x, y));
		if (moved) {
			SetCurrentSpeed(0);
			moved = false;
		}
		Log(Logs::Detail, Logs::AI, "Rooted while calculating new position to (%.3f, %.3f, %.3f)", x, y, z);
		return true;
	}

	float old_test_vector = test_vector;
	m_TargetV.x = x - nx;
	m_TargetV.y = y - ny;
	m_TargetV.z = z - nz;

	if (m_TargetV.x == 0 && m_TargetV.y == 0)
		return false;
	SetCurrentSpeed((int8)(speed)); //*NPC_RUNANIM_RATIO);
									//speed *= NPC_SPEED_MULTIPLIER;

	Log(Logs::Detail, Logs::AI, "Calculating new position to (%.3f, %.3f, %.3f) vector (%.3f, %.3f, %.3f) rate %.3f RAS %d", x, y, z, m_TargetV.x, m_TargetV.y, m_TargetV.z, speed, pRunAnimSpeed);

	// --------------------------------------------------------------------------
	// 2: get unit vector
	// --------------------------------------------------------------------------
	test_vector = sqrtf(x*x + y*y + z*z);
	tar_vector = speed / sqrtf(m_TargetV.x*m_TargetV.x + m_TargetV.y*m_TargetV.y + m_TargetV.z*m_TargetV.z);
	m_Position.w = CalculateHeadingToTarget(x, y);

	if (tar_vector >= 1.0) {
		if (IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), x, y, z);
		}

		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
		Log(Logs::Detail, Logs::AI, "Close enough, jumping to waypoint");
	}
	else {
		float new_x = m_Position.x + m_TargetV.x*tar_vector;
		float new_y = m_Position.y + m_TargetV.y*tar_vector;
		float new_z = m_Position.z + m_TargetV.z*tar_vector;
		if (IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
		}

		m_Position.x = new_x;
		m_Position.y = new_y;
		m_Position.z = new_z;
		Log(Logs::Detail, Logs::AI, "Next position (%.3f, %.3f, %.3f)", m_Position.x, m_Position.y, m_Position.z);
	}

	if (fix_z_timer.Check())
		this->FixZ();

	//OP_MobUpdate
	if ((old_test_vector != test_vector) || tar_ndx>20) { //send update
		tar_ndx = 0;
		this->SetMoving(true);
		moved = true;
		m_Delta = glm::vec4(m_Position.x - nx, m_Position.y - ny, m_Position.z - nz, 0.0f);
		SendPositionUpdate();
	}
	tar_ndx++;

	// now get new heading
	SetAppearance(eaStanding, false); // make sure they're standing
	pLastChange = Timer::GetCurrentTime();
	return true;
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

	if (flymode == FlyMode1)
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

float Mob::GetFixedZ(glm::vec3 dest, int32 z_find_offset)
{
	BenchTimer timer;
	timer.reset();
	float new_z = dest.z;

	if (zone->HasMap() && RuleB(Map, FixZWhenMoving) &&
		(flymode != 1 && flymode != 2))
	{
		if (!RuleB(Watermap, CheckForWaterWhenMoving) || !zone->HasWaterMap()
			|| (zone->HasWaterMap() &&
				!zone->watermap->InWater(glm::vec3(m_Position))))
		{
			/* Any more than 5 in the offset makes NPC's hop/snap to ceiling in small corridors */
			new_z = this->FindDestGroundZ(dest, z_find_offset);
			if (new_z != BEST_Z_INVALID)
			{
				new_z += this->GetZOffset();

				// If bad new Z restore old one
				if (new_z < -2000) {
					new_z = m_Position.z;
				}
			}
		}

		auto duration = timer.elapsed();

		Log(Logs::Moderate, Logs::FixZ,
			"Mob::GetFixedZ() (%s) returned %4.3f at %4.3f, %4.3f, %4.3f - Took %lf",
			this->GetCleanName(), new_z, dest.x, dest.y, dest.z, duration);
	}

	return new_z;
}

void Mob::FixZ(int32 z_find_offset /*= 5*/)
{
	glm::vec3 current_loc(m_Position);
	float new_z = GetFixedZ(current_loc, z_find_offset);

	if (new_z != m_Position.z)
	{
		if ((new_z > -2000) && new_z != BEST_Z_INVALID) {
			if (RuleB(Map, MobZVisualDebug))
				this->SendAppearanceEffect(78, 0, 0, 0, 0);

			m_Position.z = new_z;
		}
		else {
			if (RuleB(Map, MobZVisualDebug))
				this->SendAppearanceEffect(103, 0, 0, 0, 0);

			Log(Logs::General, Logs::FixZ, "%s is failing to find Z %f",
				this->GetCleanName(), std::abs(m_Position.z - new_z));
		}
	}
}

float Mob::GetZOffset() const {
	float offset = 3.125f;

	switch (race) {
		case 436:
			offset = 0.577f;
			break;
		case 430:
			offset = 0.5f;
			break;
		case 432:
			offset = 1.9f;
			break;
		case 435:
			offset = 0.93f;
			break;
		case 450:
			offset = 0.938f;
			break;
		case 479:
			offset = 0.8f;
			break;
		case 451:
			offset = 0.816f;
			break;
		case 437:
			offset = 0.527f;
			break;
		case 439:
			offset = 1.536f;
			break;
		case 415:
			offset = 1.0f;
			break;
		case 438:
			offset = 0.776f;
			break;
		case 452:
			offset = 0.776f;
			break;
		case 441:
			offset = 0.816f;
			break;
		case 440:
			offset = 0.938f;
			break;
		case 468:
			offset = 1.0f;
			break;
		case 459:
			offset = 1.0f;
			break;
		case 462:
			offset = 1.5f;
			break;
		case 530:
			offset = 1.2f;
			break;
		case 549:
			offset = 0.5f;
			break;
		case 548:
			offset = 0.5f;
			break;
		case 547:
			offset = 0.5f;
			break;
		case 604:
			offset = 1.2f;
			break;
		case 653:
			offset = 5.9f;
			break;
		case 658:
			offset = 4.0f;
			break;
		case 323:
			offset = 5.0f;
			break;
		case 663:
			offset = 5.0f;
			break;
		case 664:
			offset = 4.0f;
			break;
		case 703:
			offset = 9.0f;
			break;
		case 688:
			offset = 5.0f;
			break;
		case 669:
			offset = 7.0f;
			break;
		case 687:
			offset = 2.0f;
			break;
		case 686:
			offset = 2.0f;
			break;
		default:
			offset = 3.125f;
	}

	return 0.2 * GetSize() * offset;
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
