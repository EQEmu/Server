/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef OBJECT_H
#define OBJECT_H

// Object Class:
// Represents Zone Objects (forges, ovens, brew barrels, items dropped to ground, etc)

#include "../common/types.h"
#include "../common/linked_list.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/Item.h"
#include "client.h"
#include "mob.h"
#include "npc.h"
#include "entity.h"
#include "../common/timer.h"

/*
item icon numbers (showeq)
IT1_ACTORDEF=Long Sword
IT5_ACTORDEF=Throwing Knife
IT8_ACTORDEF=Basic Staff
IT10_ACTORDEF=Arrow
IT14_ACTORDEF=Basic Hammer
IT16_ACTORDEF=Basic Spear
IT27_ACTORDEF=Book
IT35_ACTORDEF=Mod Rod
IT62_ACTORDEF=Flaming Sword
IT63_ACTORDEF=Small Bag
IT64_ACTORDEF=Large Bag
IT65_ACTORDEF=Scroll
IT66_ACTORDEF=Forge
IT67_ACTORDEF=Voodoo Doll
IT68_ACTORDEF=Glowing Black Stone
IT69_ACTORDEF=Oven
IT70_ACTORDEF=Brew Barrel
IT73_ACTORDEF=Kiln
IT74_ACTORDEF=Pottery Wheel
IT78_ACTORDEF=Campfire (Oven)
IT128_ACTORDEF=Loom
IT177_ACTORDEF=Shattering Hammer
IT203_ACTORDEF=Round Shield
IT210_ACTORDEF=Shimmering Orb
IT400_ACTORDEF=Globe of Slush Water
IT401_ACTORDEF=Red Mushroom
IT402_ACTORDEF=Blue Mushroom
IT403_ACTORDEF=Yew Leaf
IT10511_ACTORDEF=A Soulstone Shard
IT10512_ACTORDEF=Orb of Exploration
IT10630_ACTORDEF=Fish Sword
IT10661_ACTORDEF=Blade of Walnan
IT10714_ACTORDEF=Augmentation Sealer
IT10725_ACTORDEF=Shuriken
*/

// Object Types
#define OT_DROPPEDITEM	0x01
#define OT_MEDICINEBAG	0x09
#define OT_TOOLBOX		0x0A
#define OT_OVEN			0x0F
#define OT_SEWINGKIT	0x10	//and loom
#define OT_FORGE		0x11
#define OT_FLETCHINGKIT	0x12
#define OT_BREWBARREL	0x13
#define OT_JEWELERSKIT	0x14
#define OT_POTTERYWHEEL	0x15
#define OT_KILN			0x16
#define OT_KEYMAKER		0x17
#define OT_WIZARDLEX	0x18
#define OT_MAGELEX		0x19
#define OT_NECROLEX		0x1A
#define OT_ENCHLEX		0x1B
// high elf forge is 0x1F (Koada'dal forge)
#define OT_TEIRDALFORGE	0x20	//dark elf
#define OT_OGGOKFORGE	0x21	//ogre
#define OT_STORMGUARDF	0x22	//dwarven
#define OT_VALEFORGE    0x31  //halfling
// gnome forge 0x23 (ak'anon forge)
// barbarian forge 0x24 (northman forge)
// 
// iksar forge 0x26	(cabilis forge)
// human forge 0x27 (qeynos or freeport?) (royal qeynos forge or freeport forge)
// human forge 0x28 (qeynos or freeport?)
// halfling tailoring kit 0x29
// erudite tailoring kit 0x2A
// wood elf tailoring kit 0x2B
// wood elf fletching kit 0x2C
// iksar pottery wheel 0x2D
#define OT_TACKLEBOX	0x2e
// troll forge 0x2F (grobb forge)
#define OT_FIERDALFFORGE	0x30
// erudite forge 0x32 (erud forge)
#define OT_AUGMENT		0x35
//... (shar vahl forge)
//... wood elf (fier'dal forge)
//... (froglok forge)

// Icon values:
//0x0453 a pie
//0x0454 cookies?
//0x0455 is a piece of meat?
//0x0456 is fletching sticks
//0x0457 looks like a burnt cookie or something :/
//0x0458 is a pottery wheel
//0x0459 is a oven
//0x045A is an oven
//0x045B is a forge
//0x045C is brewing barrel
//0x045D is a hammer
//0x045E is a wierd rope shape

class Object: public Entity
{
public:
	// Loading object from database
	Object(uint32 id, uint32 type, uint32 icon, const Object_Struct& data, const ItemInst* inst);
	Object(const ItemInst* inst, char* name,float max_x,float min_x,float max_y,float min_y,float z,float heading,uint32 respawntimer);
	// Loading object from client dropping item on ground
	Object(Client* client, const ItemInst* inst);
	Object(const ItemInst *inst, float x, float y, float z, float heading, uint32 decay_time = 300000);
	Object(const char *model, float x, float y, float z, float heading, uint8 type, uint32 decay_time = 0);
	
	// Destructor
	~Object();
	bool Process();
	bool IsGroundSpawn() { return m_ground_spawn; }
	// Event handlers
	bool HandleClick(Client* sender, const ClickObject_Struct* click_object);
	void Close();
	void Delete(bool reset_state=false); // Object itself
	static void HandleCombine(Client* user, const NewCombine_Struct* in_combine, Object *worldo);
	static void HandleAugmentation(Client* user, const AugmentItem_Struct* in_augment, Object *worldo);
	static void HandleAutoCombine(Client* user, const RecipeAutoCombine_Struct* rac);
	
	static SkillType TypeToSkill(uint32 type);
	
	// Packet functions
	void CreateSpawnPacket(EQApplicationPacket* app);
	void CreateDeSpawnPacket(EQApplicationPacket* app);
	void Depop();
	void Repop();
	
	//Decay functions
	void StartDecay() {decay_timer.Start();}
	
	// Container functions
	void PutItem(uint8 index, const ItemInst* inst);
	void DeleteItem(uint8 index); // Item inside container
	ItemInst* PopItem(uint8 index); // Pop item out of container
	
	// Override base class implementations
	virtual bool IsObject()	const { return true; }
	virtual bool Save();
	virtual uint16 VarSave();
	virtual void SetID(uint16 set_id);
	
	void ClearUser() { user = nullptr; }

	uint32 GetDBID();
	uint32 GetType();
	void  SetType(uint32 type);
	void  SetDBID(uint32 dbid);
	uint32 GetIcon();
	void  SetIcon(uint32 icon);
	uint32 GetItemID();
	void  SetItemID(uint32 itemid);
	void GetObjectData(Object_Struct* Data);
	void SetObjectData(Object_Struct* Data);
	void GetLocation(float* x, float* y, float* z);
	void SetLocation(float x, float y, float z);
	void GetHeading(float* heading);
	void SetHeading(float heading);
	float GetX();
	float GetY();
	float GetZ();
	float GetHeadingData();
	void SetX(float pos);
	void SetY(float pos);
	void SetZ(float pos);
	void SetModelName(const char* modelname);
	const char* GetModelName();

	const char* GetEntityVariable(const char *id);
	void SetEntityVariable(const char *id, const char *m_var);
	bool EntityVariableExists(const char *id);

protected:
	void	ResetState();	// Set state back to original
	void	RandomSpawn(bool send_packet = false);		//spawn this ground spawn at a random place
	
	Object_Struct	m_data;		// Packet data
	ItemInst*		m_inst;		// Item representing object
	bool			m_inuse;	// Currently in use by a client?
	uint32			m_id;		// Database key, different than drop_id
	uint32			m_type;		// Object Type, ie, forge, oven, dropped item, etc
	uint32			m_icon;		// Icon to use for forge, oven, etc
	float			m_max_x;
	float			m_max_y;
	float			m_min_x;
	float			m_min_y;
	float			m_z;
	float			m_heading;
	bool			m_ground_spawn;

	std::map<std::string, std::string> o_EntityVariables;
	
	Client *user;
	Client *last_user;
	
	Timer respawn_timer;
	Timer decay_timer;
};

#endif
