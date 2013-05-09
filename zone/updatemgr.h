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
#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#include "../common/features.h"
#ifdef PACKET_UPDATE_MANAGER

#include "../common/timer.h"
#include <map>
using namespace std;

/*typedef enum {
	updateMobPOS = 1,
	updateMobHP
} updateType;*/

//the number of different queue levels to use
#define UPDATE_LEVELS 4
#define UPDATE_RESOLUTION 300
#define UPDATE_DROP_RANGE 800

//if the player moves more than this ammount, all queues are flushed.
#define UPDATE_JUMP_FLUSH 200	//

class EQStream;
class EQApplicationPacket;
class Mob;

class UMType {
public:
	UMType() {
		app = nullptr; ack = false;
	}
	UMType(EQApplicationPacket *_app, bool _ack) {
		app = _app; ack = _ack;
	}

	EQApplicationPacket *app;
	bool ack;
};

typedef map<uint32, UMType > UMMap;

class UpdateManager {
protected:
	//squared distances for each level
	static const float level_distances2[UPDATE_LEVELS];

	//delay between sending packets in each level, in ms
	static const uint32 level_timers[UPDATE_LEVELS+1];

public:
	UpdateManager(EQStream *c);
	~UpdateManager();

	//range2 is the range of 'from' to this client, squared
	void QueuePacket(EQApplicationPacket *app, bool ack_req, Mob *from, float range2);
	void Process();
	void FlushQueues();

protected:
	void _SendLevel(int level);

	EQStream *net;

	UMMap levels[UPDATE_LEVELS+1];
	Timer *timers[UPDATE_LEVELS+1];
	Timer limiter;
};

#endif	//PACKET_UPDATE_MANAGER

#endif

