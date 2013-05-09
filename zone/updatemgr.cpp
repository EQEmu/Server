/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/debug.h"
#include "../common/features.h"

#ifdef PACKET_UPDATE_MANAGER
#include "updatemgr.h"
#include "mob.h"
#include "../common/EQStream.h"

//squared distances for each level
//these values are pulled out of my ass, should be tuned some day
const float UpdateManager::level_distances2[UPDATE_LEVELS]
	= { 50*50, 250*250, 500*500, 800*800 };

//delay between sending packets in each level, in ms
//its best if they are all multiples of UPDATE_RESOLUTION
//these values are pulled out of my ass, should be tuned some day
const uint32 UpdateManager::level_timers[UPDATE_LEVELS+1] = { UPDATE_RESOLUTION, //.3s
		2*UPDATE_RESOLUTION,	//.6s
		3*UPDATE_RESOLUTION,	//.9s
		9*UPDATE_RESOLUTION,	//~2s
		34*UPDATE_RESOLUTION	//~10s
	};


/*
	This system assumes that two packets sent by a mob with the same
	opcodes contain the same info at different times, and will prefer
	to send only the most recent packet. If this is bad, then this
	thing needs a redesign.

*/

//build a unique ID based on opcode and mob id..
#define MakeUpdateID(mob, app) (((mob->GetID())<<12) | (app->GetOpcode()&0xFFF))

UpdateManager::UpdateManager(EQStream *c)
 : limiter(UPDATE_RESOLUTION)
{
	net = c;
	int r;
	for(r = 0; r <= UPDATE_LEVELS; r++) {
		timers[r] = new Timer(level_timers[r]);
	}
}

UpdateManager::~UpdateManager() {
	int r;
	UMMap::iterator cur,end;
	for(r = 0; r <= UPDATE_LEVELS; r++) {
		safe_delete(timers[r]);
		cur = levels[r].begin();
		end = levels[r].end();
		for(; cur != end; cur++) {
			EQApplicationPacket *tmp = cur->second.app;
			EQApplicationPacket::PacketUsed(&tmp);
		}
		levels[r].clear();
	}
}

/*
	Puts a packet into its proper spacial queue
*/
void UpdateManager::QueuePacket(EQApplicationPacket *app, bool ack_req, Mob *from, float range2) {
	int r = UPDATE_LEVELS;
	UMMap *cur = levels;
	const float *cur_d = level_distances2;
	cur += UPDATE_LEVELS;	//move to the end.
	cur_d += UPDATE_LEVELS - 1;
	//work backwards since mobs are more likely to be further away
	for(r = UPDATE_LEVELS; r >= 0; r--, cur--, cur_d--) {
		if(range2 < *cur_d)
			continue;
		//this packet falls into this queue...
		uint32 id = MakeUpdateID(from, app);
//		if(r < 2)
//			net->QueuePacket(app, ack_req);
//LogFile->write(EQEMuLog::Debug, "Queueing packet from %s (0x%.4x) id=0x%x at level %d\n", from->GetName(), app->GetOpcode(), id, r);
		app->PacketReferenced();
		//reference decrementing is taken care of my UMType destructor
		//if anything is overwritten
		(*cur)[id] = UMType(app, ack_req);
//		(*cur)[id] = UMType(app->Copy(), ack_req);
		return;
	}
	//if we get here, were in trouble...
}

void UpdateManager::Process() {
	if(!limiter.Check())
		return;
	Timer **curt = timers;
	int r;
	for(r = 0; r <= UPDATE_LEVELS; r++, curt++) {
		if(!(*curt)->Check())
			continue;
		_SendLevel(r);
	}
}

void UpdateManager::FlushQueues() {
	limiter.Start();
	Timer **curt = timers;
	int r;
	for(r = 0; r <= UPDATE_LEVELS; r++, curt++) {
		(*curt)->Start();
		_SendLevel(r);
	}
}

void UpdateManager::_SendLevel(int level) {
/*LogFile->write(EQEMuLog::Error, "Sending for level %d", level);
if(level > 0)
{
	int r;
	for(r = 0; r <= UPDATE_LEVELS; r++) {
		LogFile->write(EQEMuLog::Error, "Level %d: %d", r, levels[r].size());
	}
	{
	float range2 = 5;
		int r = UPDATE_LEVELS;
	UMMap *cur = levels;
	const float *cur_d = level_distances2;
	cur += UPDATE_LEVELS;	//move to the end.
	cur_d += UPDATE_LEVELS - 1;
	//work backwards since mobs are more likely to be further away
	for(r = UPDATE_LEVELS; r >= 0; r--, cur--, cur_d--) {
		LogFile->write(EQEMuLog::Error, "If they are less than %f away, they dont go in level %d", *cur_d, r);
		if(range2 < *cur_d)
			continue;
		LogFile->write(EQEMuLog::Error, "A mob %f away gets put into level %d", range2, r);
	}
	}
}*/

	UMMap::iterator cur,end;
	UMMap *curm;
	UMMap *om = levels + level;
	cur = om->begin();
	end = om->end();
	uint32 key;
	int r;

	while(cur != end) {
		key = cur->first;
		//relies on fast queue setting .app to null if it eats it
//LogFile->write(EQEMuLog::Debug, "Sending id 0x%x for level %d\n", key, level);
		net->FastQueuePacket(&cur->second.app, cur->second.ack);
//EQApplicationPacket::PacketUsed(&cur->second.app);
		cur++;
		om->erase(key);

		//need to clear our any updates in slower levels
		//so mobs dont jump backwards from old updates
		curm = om + 1;
		for(r = level+1; r <= UPDATE_LEVELS; r++, curm++) {
			//do we need this count check?
			if(curm->count(key) != 0) {
				//reference decrementing is taken care of my UMType destructor
				EQApplicationPacket *tmp = (*curm)[key].app;
				curm->erase(key);
				EQApplicationPacket::PacketUsed(&tmp);
			}
		}
	}
}


#endif	//PACKET_UPDATE_MANAGER

