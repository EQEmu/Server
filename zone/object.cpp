/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/string_util.h"

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "object.h"

#include "quest_parser_collection.h"
#include "zonedb.h"

#include <iostream>

const char DEFAULT_OBJECT_NAME[] = "IT63_ACTORDEF";
const char DEFAULT_OBJECT_NAME_SUFFIX[] = "_ACTORDEF";


extern Zone* zone;
extern EntityList entity_list;

// Loading object from database
Object::Object(uint32 id, uint32 type, uint32 icon, const Object_Struct& object, const ItemInst* inst)
 : respawn_timer(0), decay_timer(300000)
{

	user = nullptr;
	last_user = nullptr;

	// Initialize members
	m_id = id;
	m_type = type;
	m_icon = icon;
	m_inuse = false;
	m_inst = nullptr;
	m_ground_spawn=false;
	// Copy object data
	memcpy(&m_data, &object, sizeof(Object_Struct));
	if (inst) {
		m_inst = inst->Clone();
		decay_timer.Start();
	} else {
		decay_timer.Disable();
	}
	respawn_timer.Disable();

	// Set drop_id to zero - it will be set when added to zone with SetID()
	m_data.drop_id = 0;
}

//creating a re-ocurring ground spawn.
Object::Object(const ItemInst* inst, char* name,float max_x,float min_x,float max_y,float min_y,float z,float heading,uint32 respawntimer)
 : respawn_timer(respawntimer), decay_timer(300000)
{

	user = nullptr;
	last_user = nullptr;
	m_max_x=max_x;
	m_max_y=max_y;
	m_min_x=min_x;
	m_min_y=min_y;
	m_id	= 0;
	m_inst	= (inst) ? inst->Clone() : nullptr;
	m_type	= OT_DROPPEDITEM;
	m_icon	= 0;
	m_inuse	= false;
	m_ground_spawn = true;
	decay_timer.Disable();
	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));
	m_data.heading = heading;
	m_data.z = z;
	m_data.zone_id = zone->GetZoneID();
	respawn_timer.Disable();
	strcpy(m_data.object_name, name);
	RandomSpawn(false);

	// Hardcoded portion for unknown members
	m_data.unknown024	= 0x7f001194;
	m_data.unknown076	= 0x0000d5fe;
	m_data.unknown084	= 0xFFFFFFFF;
}

// Loading object from client dropping item on ground
Object::Object(Client* client, const ItemInst* inst)
 : respawn_timer(0), decay_timer(300000)
{
	user = nullptr;
	last_user = nullptr;

	// Initialize members
	m_id	= 0;
	m_inst	= (inst) ? inst->Clone() : nullptr;
	m_type	= OT_DROPPEDITEM;
	m_icon	= 0;
	m_inuse	= false;
	m_ground_spawn = false;
	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));
	m_data.heading = client->GetHeading();
	m_data.x = client->GetX();
	m_data.y = client->GetY();
	if (client->GetClientVersion() >= ClientVersion::RoF2)
	{
		// RoF2 places items at player's Z, which is 0.625 of their height.
		m_data.z = client->GetZ() - (client->GetSize() * 0.625f);
	}
	else
	{
		m_data.z = client->GetZ();
	}
	m_data.zone_id = zone->GetZoneID();

	decay_timer.Start();
	respawn_timer.Disable();

	// Hardcoded portion for unknown members
	m_data.unknown024	= 0x7f001194;
	m_data.unknown076	= 0x0000d5fe;
	m_data.unknown084	= 0xFFFFFFFF;

	// Set object name
	if (inst) {
		const Item_Struct* item = inst->GetItem();
		if (item && item->IDFile) {
			if (strlen(item->IDFile) == 0) {
				strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
			}
			else {
				// Object name is idfile + _ACTORDEF
				uint32 len_idfile = strlen(inst->GetItem()->IDFile);
				uint32 len_copy = sizeof(m_data.object_name) - len_idfile - 1;
				if (len_copy > sizeof(DEFAULT_OBJECT_NAME_SUFFIX)) {
					len_copy = sizeof(DEFAULT_OBJECT_NAME_SUFFIX);
				}

				memcpy(&m_data.object_name[0], inst->GetItem()->IDFile, len_idfile);
				memcpy(&m_data.object_name[len_idfile], DEFAULT_OBJECT_NAME_SUFFIX, len_copy);
			}
		}
		else {
			strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
		}
	}
}

Object::Object(const ItemInst *inst, float x, float y, float z, float heading, uint32 decay_time)
 : respawn_timer(0), decay_timer(decay_time)
{
	user = nullptr;
	last_user = nullptr;

	// Initialize members
	m_id	= 0;
	m_inst	= (inst) ? inst->Clone() : nullptr;
	m_type	= OT_DROPPEDITEM;
	m_icon	= 0;
	m_inuse	= false;
	m_ground_spawn = false;
	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));
	m_data.heading = heading;
	m_data.x = x;
	m_data.y = y;
	m_data.z = z;
	m_data.zone_id = zone->GetZoneID();

	if (decay_time)
		decay_timer.Start();

	respawn_timer.Disable();

	// Hardcoded portion for unknown members
	m_data.unknown024	= 0x7f001194;
	m_data.unknown076	= 0x0000d5fe;
	m_data.unknown084	= 0xFFFFFFFF;

	// Set object name
	if (inst) {
		const Item_Struct* item = inst->GetItem();
		if (item && item->IDFile) {
			if (strlen(item->IDFile) == 0) {
				strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
			}
			else {
				// Object name is idfile + _ACTORDEF
				uint32 len_idfile = strlen(inst->GetItem()->IDFile);
				uint32 len_copy = sizeof(m_data.object_name) - len_idfile - 1;
				if (len_copy > sizeof(DEFAULT_OBJECT_NAME_SUFFIX)) {
					len_copy = sizeof(DEFAULT_OBJECT_NAME_SUFFIX);
				}

				memcpy(&m_data.object_name[0], inst->GetItem()->IDFile, len_idfile);
				memcpy(&m_data.object_name[len_idfile], DEFAULT_OBJECT_NAME_SUFFIX, len_copy);
			}
		}
		else {
			strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
		}
	}
}

Object::Object(const char *model, float x, float y, float z, float heading, uint8 type, uint32 decay_time)
 : respawn_timer(0), decay_timer(decay_time)
{
	user = nullptr;
	last_user = nullptr;
	ItemInst* inst = nullptr;
	inst = new ItemInst(ItemInstWorldContainer);

	// Initialize members
	m_id	= 0;
	m_inst	= (inst) ? inst->Clone() : nullptr;
	m_type	= type;
	m_icon	= 0;
	m_inuse	= false;
	m_ground_spawn = false;
	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));
	m_data.heading = heading;
	m_data.x = x;
	m_data.y = y;
	m_data.z = z;
	m_data.zone_id = zone->GetZoneID();

	if (decay_time)
		decay_timer.Start();

	respawn_timer.Disable();

	//Hardcoded portion for unknown members
	m_data.unknown024	= 0x7f001194;
	m_data.unknown076	= 0x0000d5fe;
	m_data.unknown084	= 0xFFFFFFFF;

	if(model)
		strcpy(m_data.object_name, model);
	else
		strcpy(m_data.object_name, "IT64_ACTORDEF"); //default object name if model isn't specified for some unknown reason
}

Object::~Object()
{
	safe_delete(m_inst);
	if(user != nullptr) {
		user->SetTradeskillObject(nullptr);
	}
}

void Object::SetID(uint16 set_id)
{
	// Invoke base class
	Entity::SetID(set_id);

	// Store new id as drop_id
	m_data.drop_id = (uint32)this->GetID();
}

// Reset state of object back to zero
void Object::ResetState()
{
	safe_delete(m_inst);

	m_id	= 0;
	m_type	= 0;
	m_icon	= 0;
	memset(&m_data, 0, sizeof(Object_Struct));
}

bool Object::Save()
{
	if (m_id) {
		// Update existing
		database.UpdateObject(m_id, m_type, m_icon, m_data, m_inst);
	}
	else {
		// Doesn't yet exist, add now
		m_id = database.AddObject(m_type, m_icon, m_data, m_inst);
	}

	return true;
}

uint16 Object::VarSave()
{
	if (m_id) {
		// Update existing
		database.UpdateObject(m_id, m_type, m_icon, m_data, m_inst);
	}
	else {
		// Doesn't yet exist, add now
		m_id = database.AddObject(m_type, m_icon, m_data, m_inst);
	}
	return m_id;
}

// Remove object from database
void Object::Delete(bool reset_state)
{
	if (m_id != 0) {
		database.DeleteObject(m_id);
	}

	if (reset_state) {
		ResetState();
	}
}

const ItemInst* Object::GetItem(uint8 index) {
	if (index < EmuConstants::MAP_WORLD_SIZE) {
		return m_inst->GetItem(index);
	}

	return nullptr;
}

// Add item to object (only logical for world tradeskill containers
void Object::PutItem(uint8 index, const ItemInst* inst)
{
	if (index > 9) {
		Log.Out(Logs::General, Logs::Error, "Object::PutItem: Invalid index specified (%i)", index);
		return;
	}

	if (m_inst && m_inst->IsType(ItemClassContainer)) {
		if (inst) {
			m_inst->PutItem(index, *inst);
		}
		else {
			m_inst->DeleteItem(index);
		}
		database.SaveWorldContainer(zone->GetZoneID(),m_id,m_inst);
		// This is _highly_ inefficient, but for now it will work: Save entire object to database
		Save();
	}
}

void Object::Close() {
	m_inuse = false;
	if(user != nullptr)
	{
		last_user = user;
		// put any remaining items from the world container back into the player's inventory to avoid item loss
		// if they close the container without removing all items
		ItemInst* container = this->m_inst;
		if(container != nullptr)
		{
			for (uint8 i = SUB_BEGIN; i < EmuConstants::ITEM_CONTAINER_SIZE; i++)
			{
				ItemInst* inst = container->PopItem(i);
				if(inst != nullptr)
				{
					user->MoveItemToInventory(inst, true);
				}
			}
		}

		user->SetTradeskillObject(nullptr);
	}
	user = nullptr;
}

// Remove item from container
void Object::DeleteItem(uint8 index)
{
	if (m_inst && m_inst->IsType(ItemClassContainer)) {
		m_inst->DeleteItem(index);

		// This is _highly_ inefficient, but for now it will work: Save entire object to database
		Save();
	}
}

// Pop item out of container
ItemInst* Object::PopItem(uint8 index)
{
	ItemInst* inst = nullptr;

	if (m_inst && m_inst->IsType(ItemClassContainer)) {
		inst = m_inst->PopItem(index);

		// This is _highly_ inefficient, but for now it will work: Save entire object to database
		Save();
	}

	return inst;
}

void Object::CreateSpawnPacket(EQApplicationPacket* app)
{
	app->SetOpcode(OP_GroundSpawn);
	app->pBuffer = new uchar[sizeof(Object_Struct)];
	app->size = sizeof(Object_Struct);
	memcpy(app->pBuffer, &m_data, sizeof(Object_Struct));
}

void Object::CreateDeSpawnPacket(EQApplicationPacket* app)
{
	app->SetOpcode(OP_ClickObject);
	app->pBuffer = new uchar[sizeof(ClickObject_Struct)];
	app->size = sizeof(ClickObject_Struct);
	memset(app->pBuffer, 0, sizeof(ClickObject_Struct));
	ClickObject_Struct* co = (ClickObject_Struct*) app->pBuffer;
	co->drop_id = m_data.drop_id;
	co->player_id = 0;
}

bool Object::Process(){
	if(m_type == OT_DROPPEDITEM && decay_timer.Enabled() && decay_timer.Check()) {
		// Send click to all clients (removes entity on client)
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ClickObject, sizeof(ClickObject_Struct));
		ClickObject_Struct* click_object = (ClickObject_Struct*)outapp->pBuffer;
		click_object->drop_id = GetID();
		entity_list.QueueClients(nullptr, outapp, false);
		safe_delete(outapp);

		// Remove object
		database.DeleteObject(m_id);
		return false;
	}

	if(m_ground_spawn && respawn_timer.Check()){
		RandomSpawn(true);
	}
	return true;
}

void Object::RandomSpawn(bool send_packet) {
	if(!m_ground_spawn)
		return;

	m_data.x = zone->random.Real(m_min_x, m_max_x);
	m_data.y = zone->random.Real(m_min_y, m_max_y);
	respawn_timer.Disable();

	if(send_packet) {
		EQApplicationPacket app;
		CreateSpawnPacket(&app);
		entity_list.QueueClients(nullptr, &app, true);
	}
}

bool Object::HandleClick(Client* sender, const ClickObject_Struct* click_object)
{
	if(m_ground_spawn){//This is a Cool Groundspawn
		respawn_timer.Start();
	}
	if (m_type == OT_DROPPEDITEM) {
		bool cursordelete = false;
		if (m_inst && sender) {
			// if there is a lore conflict, delete the offending item from the server inventory
			// the client updates itself and takes care of sending "duplicate lore item" messages
			auto item = m_inst->GetItem();
			if(sender->CheckLoreConflict(item)) {
				int16 loreslot = sender->GetInv().HasItem(item->ID, 0, invWhereBank);
				if (loreslot != INVALID_INDEX) // if the duplicate is in the bank, delete it.
					sender->DeleteItemInInventory(loreslot);
				else
					cursordelete = true;	// otherwise, we delete the new one
			}

			if (item->RecastDelay)
				m_inst->SetRecastTimestamp(
				    database.GetItemRecastTimestamp(sender->CharacterID(), item->RecastType));

			char buf[10];
			snprintf(buf, 9, "%u", item->ID);
			buf[9] = '\0';
			std::vector<EQEmu::Any> args;
			args.push_back(m_inst);
			if(parse->EventPlayer(EVENT_PLAYER_PICKUP, sender, buf, this->GetID(), &args))
			{
				EQApplicationPacket* outapp = new EQApplicationPacket(OP_ClickObject, sizeof(ClickObject_Struct));
				memcpy(outapp->pBuffer, click_object, sizeof(ClickObject_Struct));
				ClickObject_Struct* co = (ClickObject_Struct*)outapp->pBuffer;
				co->drop_id = 0;
				entity_list.QueueClients(nullptr, outapp, false);
				safe_delete(outapp);
				
				// No longer using a tradeskill object
				sender->SetTradeskillObject(nullptr);
				user = nullptr;

				return true;
			}


			// Transfer item to client
			sender->PutItemInInventory(MainCursor, *m_inst, false);
			sender->SendItemPacket(MainCursor, m_inst, ItemPacketTrade);

			if(cursordelete)	// delete the item if it's a duplicate lore. We have to do this because the client expects the item packet
				sender->DeleteItemInInventory(MainCursor);

			if(!m_ground_spawn)
				safe_delete(m_inst);

			// No longer using a tradeskill object
			sender->SetTradeskillObject(nullptr);
			user = nullptr;
		}

		// Send click to all clients (removes entity on client)
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ClickObject, sizeof(ClickObject_Struct));
		memcpy(outapp->pBuffer, click_object, sizeof(ClickObject_Struct));
		entity_list.QueueClients(nullptr, outapp, false);
		safe_delete(outapp);

		// Remove object
		database.DeleteObject(m_id);
		if(!m_ground_spawn)
			entity_list.RemoveEntity(this->GetID());
	} else {
		// Tradeskill item
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ClickObjectAction, sizeof(ClickObjectAction_Struct));
		ClickObjectAction_Struct* coa = (ClickObjectAction_Struct*)outapp->pBuffer;

		//TODO: there is prolly a better way to do this.
		//if this is not the main user, send them a close and a message
		if(user == nullptr || user == sender)
			coa->open		= 0x01;
		else {
			coa->open		= 0x00;
			//sender->Message(13, "Somebody is allready using that container.");
		}
		m_inuse			= true;
		coa->type		= m_type;
		coa->unknown16	= 0x0a;

		coa->drop_id	= click_object->drop_id;
		coa->player_id	= click_object->player_id;
		coa->icon		= m_icon;

		if(sender->IsLooting())
		{
			coa->open = 0x00;
			user = sender;
		}

		sender->QueuePacket(outapp);
		safe_delete(outapp);

		//if the object allready had a user, we are done
		if(user != nullptr)
			return(false);

		// Starting to use this object
		sender->SetTradeskillObject(this);

		user = sender;

		// Send items inside of container

		if (m_inst && m_inst->IsType(ItemClassContainer)) {

			//Clear out no-drop and no-rent items first if different player opens it
			if(user != last_user)
				m_inst->ClearByFlags(byFlagSet, byFlagSet);

			EQApplicationPacket* outapp=new EQApplicationPacket(OP_ClientReady,0);
			sender->QueuePacket(outapp);
			safe_delete(outapp);
			for (uint8 i = SUB_BEGIN; i < EmuConstants::ITEM_CONTAINER_SIZE; i++) {
				const ItemInst* inst = m_inst->GetItem(i);
				if (inst) {
					//sender->GetInv().PutItem(i+4000,inst);
					sender->SendItemPacket(i, inst, ItemPacketWorldContainer);
				}
			}
		}
	}

	return true;
}

// Add new Zone Object (theoretically only called for items dropped to ground)
uint32 ZoneDatabase::AddObject(uint32 type, uint32 icon, const Object_Struct& object, const ItemInst* inst)
{
	uint32 database_id = 0;
	uint32 item_id = 0;
	int16 charges = 0;

	if (inst && inst->GetItem()) {
		item_id = inst->GetItem()->ID;
		charges = inst->GetCharges();
	}

	// SQL Escape object_name
	uint32 len = strlen(object.object_name) * 2 + 1;
	char* object_name = new char[len];
	DoEscapeString(object_name, object.object_name, strlen(object.object_name));

    // Save new record for object
	std::string query = StringFormat("INSERT INTO object "
                                    "(zoneid, xpos, ypos, zpos, heading, "
                                    "itemid, charges, objectname, type, icon) "
                                    "values (%i, %f, %f, %f, %f, %i, %i, '%s', %i, %i)",
                                    object.zone_id, object.x, object.y, object.z, object.heading,
                                    item_id, charges, object_name, type, icon);
    safe_delete_array(object_name);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		Log.Out(Logs::General, Logs::Error, "Unable to insert object: %s", results.ErrorMessage().c_str());
		return 0;
	}

    // Save container contents, if container
    if (inst && inst->IsType(ItemClassContainer))
        SaveWorldContainer(object.zone_id, database_id, inst);

	return database_id;
}

// Update information about existing object in database
void ZoneDatabase::UpdateObject(uint32 id, uint32 type, uint32 icon, const Object_Struct& object, const ItemInst* inst)
{
	uint32 item_id = 0;
	int16 charges = 0;

	if (inst && inst->GetItem()) {
		item_id = inst->GetItem()->ID;
		charges = inst->GetCharges();
	}

	// SQL Escape object_name
	uint32 len = strlen(object.object_name) * 2 + 1;
	char* object_name = new char[len];
	DoEscapeString(object_name, object.object_name, strlen(object.object_name));

	// Save new record for object
	std::string query = StringFormat("UPDATE object SET "
                                    "zoneid = %i, xpos = %f, ypos = %f, zpos = %f, heading = %f, "
                                    "itemid = %i, charges = %i, objectname = '%s', type = %i, icon = %i "
                                    "WHERE id = %i",
                                    object.zone_id, object.x, object.y, object.z, object.heading,
                                    item_id, charges, object_name, type, icon, id);
    safe_delete_array(object_name);
    auto results = QueryDatabase(query);
	if (!results.Success()) {
		Log.Out(Logs::General, Logs::Error, "Unable to update object: %s", results.ErrorMessage().c_str());
		return;
	}

    // Save container contents, if container
    if (inst && inst->IsType(ItemClassContainer))
        SaveWorldContainer(object.zone_id, id, inst);
}

Ground_Spawns* ZoneDatabase::LoadGroundSpawns(uint32 zone_id, int16 version, Ground_Spawns* gs) {

	std::string query = StringFormat("SELECT max_x, max_y, max_z, "
                                    "min_x, min_y, heading, name, "
                                    "item, max_allowed, respawn_timer "
                                    "FROM ground_spawns "
                                    "WHERE zoneid = %i AND (version = %u OR version = -1) "
                                    "LIMIT 50", zone_id, version);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return gs;
	}

	int spawnIndex=0;
    for (auto row = results.begin(); row != results.end(); ++row, ++spawnIndex) {
        gs->spawn[spawnIndex].max_x=atof(row[0]);
        gs->spawn[spawnIndex].max_y=atof(row[1]);
        gs->spawn[spawnIndex].max_z=atof(row[2]);
        gs->spawn[spawnIndex].min_x=atof(row[3]);
        gs->spawn[spawnIndex].min_y=atof(row[4]);
        gs->spawn[spawnIndex].heading=atof(row[5]);
        strcpy(gs->spawn[spawnIndex].name,row[6]);
        gs->spawn[spawnIndex].item=atoi(row[7]);
        gs->spawn[spawnIndex].max_allowed=atoi(row[8]);
        gs->spawn[spawnIndex].respawntimer=atoi(row[9]);
    }
	return gs;
}

void ZoneDatabase::DeleteObject(uint32 id)
{
	// delete record of object
	std::string query = StringFormat("DELETE FROM object WHERE id = %i", id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		Log.Out(Logs::General, Logs::Error, "Unable to delete object: %s", results.ErrorMessage().c_str());
	}
}

uint32 Object::GetDBID()
{
	return this->m_id;
}

uint32 Object::GetType()
{
	return this->m_type;
}

void Object::SetType(uint32 type)
{
	this->m_type = type;
	this->m_data.object_type = type;
}

uint32 Object::GetIcon()
{
	return this->m_icon;
}

float Object::GetX()
{
	return this->m_data.x;
}

float Object::GetY()
{
	return this->m_data.y;
}


float Object::GetZ()
{
	return this->m_data.z;
}

float Object::GetHeadingData()
{
	return this->m_data.heading;
}

void Object::SetX(float pos)
{
	this->m_data.x = pos;

	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetY(float pos)
{
	this->m_data.y = pos;

	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::Depop()
{
	EQApplicationPacket* app = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	entity_list.QueueClients(0, app);
	safe_delete(app);
	entity_list.RemoveObject(this->GetID());
}

void Object::Repop()
{
	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}



void Object::SetZ(float pos)
{
	this->m_data.z = pos;

	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetModelName(const char* modelname)
{
	strn0cpy(m_data.object_name, modelname, sizeof(m_data.object_name)); // 32 is the max for chars in object_name, this should be safe
	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetSize(uint16 size)
{
	m_data.size = size;
	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetSolidType(uint16 solidtype)
{
	m_data.solidtype = solidtype;
	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

uint16 Object::GetSize()
{
	return m_data.size;
}

uint16 Object::GetSolidType()
{
	return m_data.solidtype;
}

const char* Object::GetModelName()
{
	return this->m_data.object_name;
}

void Object::SetIcon(uint32 icon)
{
	this->m_icon = icon;
}

uint32 Object::GetItemID()
{
	if (this->m_inst == 0)
	{
		return 0;
	}

	const Item_Struct* item = this->m_inst->GetItem();

	if (item == 0)
	{
		return 0;
	}

	return item->ID;
}

void Object::SetItemID(uint32 itemid)
{
	safe_delete(this->m_inst);

	if (itemid)
	{
		this->m_inst = database.CreateItem(itemid);
	}
}

void Object::GetObjectData(Object_Struct* Data)
{
	if (Data)
	{
		memcpy(Data, &this->m_data, sizeof(this->m_data));
	}
}

void Object::SetObjectData(Object_Struct* Data)
{
	if (Data)
	{
		memcpy(&this->m_data, Data, sizeof(this->m_data));
	}
}

void Object::GetLocation(float* x, float* y, float* z)
{
	if (x)
	{
		*x = this->m_data.x;
	}

	if (y)
	{
		*y = this->m_data.y;
	}

	if (z)
	{
		*z = this->m_data.z;
	}
}

void Object::SetLocation(float x, float y, float z)
{
	this->m_data.x = x;
	this->m_data.y = y;
	this->m_data.z = z;
	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::GetHeading(float* heading)
{
	if (heading)
	{
		*heading = this->m_data.heading;
	}
}

void Object::SetHeading(float heading)
{
	this->m_data.heading = heading;
	EQApplicationPacket* app = new EQApplicationPacket();
	EQApplicationPacket* app2 = new EQApplicationPacket();
	this->CreateDeSpawnPacket(app);
	this->CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetEntityVariable(const char *id, const char *m_var)
{
	std::string n_m_var = m_var;
	o_EntityVariables[id] = n_m_var;
}

const char* Object::GetEntityVariable(const char *id)
{
	if(!id)
		return nullptr;

	std::map<std::string, std::string>::iterator iter = o_EntityVariables.find(id);
	if(iter != o_EntityVariables.end())
	{
		return iter->second.c_str();
	}
	return nullptr;
}

bool Object::EntityVariableExists(const char * id)
{
	if(!id)
		return false;

	std::map<std::string, std::string>::iterator iter = o_EntityVariables.find(id);
	if(iter != o_EntityVariables.end())
	{
		return true;
	}
	return false;
}
