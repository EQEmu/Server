
#include "../debug.h"
#include "RoF.h"
#include "../opcodemgr.h"
#include "../logsys.h"
#include "../EQStreamIdent.h"
#include "../crc32.h"

#include "../eq_packet_structs.h"
#include "../MiscFunctions.h"
#include "../StringUtil.h"
#include "../Item.h"
#include "RoF_structs.h"
#include "../rulesys.h"

#include <iostream>
#include <sstream>

namespace RoF {

static const char *name = "RoF";
static OpcodeManager *opcodes = nullptr;
static Strategy struct_strategy;

char* SerializeItem(const ItemInst *inst, int16 slot_id, uint32 *length, uint8 depth);

void Register(EQStreamIdentifier &into) {
	//create our opcode manager if we havent already
	if(opcodes == nullptr) {
		//TODO: get this file name from the config file
		std::string opfile = "patch_";
		opfile += name;
		opfile += ".conf";
		//load up the opcode manager.
		//TODO: figure out how to support shared memory with multiple patches...
		opcodes = new RegularOpcodeManager();
		if(!opcodes->LoadOpcodes(opfile.c_str())) {
			_log(NET__OPCODES, "Error loading opcodes file %s. Not registering patch %s.", opfile.c_str(), name);
			return;
		}
	}

	//ok, now we have what we need to register.

	EQStream::Signature signature;
	std::string pname;

	//register our world signature.
	pname = std::string(name) + "_world";
	signature.ignore_eq_opcode = 0;
	signature.first_length = sizeof(structs::LoginInfo_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_SendLoginInfo);
	into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);

	//register our zone signature.
	pname = std::string(name) + "_zone";
	signature.ignore_eq_opcode = opcodes->EmuToEQ(OP_AckPacket);
	signature.first_length = sizeof(structs::ClientZoneEntry_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_ZoneEntry);
	into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);



	_log(NET__IDENTIFY, "Registered patch %s", name);
}

void Reload() {

	//we have a big problem to solve here when we switch back to shared memory
	//opcode managers because we need to change the manager pointer, which means
	//we need to go to every stream and replace it's manager.

	if(opcodes != nullptr) {
		//TODO: get this file name from the config file
		std::string opfile = "patch_";
		opfile += name;
		opfile += ".conf";
		if(!opcodes->ReloadOpcodes(opfile.c_str())) {
			_log(NET__OPCODES, "Error reloading opcodes file %s for patch %s.", opfile.c_str(), name);
			return;
		}
		_log(NET__OPCODES, "Reloaded opcodes for patch %s", name);
	}
}



Strategy::Strategy()
: StructStrategy()
{
	//all opcodes default to passthrough.
	#include "SSRegister.h"
	#include "RoF_ops.h"
}

std::string Strategy::Describe() const {
       std::string r;
	r += "Patch ";
	r += name;
	return(r);
}

const EQClientVersion Strategy::ClientVersion() const
{
	return EQClientRoF;
}

#include "SSDefine.h"

// Converts Titanium Slot IDs to RoF Slot IDs for use in Encodes
static inline structs::ItemSlotStruct TitaniumToRoFSlot(uint32 TitaniumSlot)
{
	structs::ItemSlotStruct RoFSlot;
	RoFSlot.SlotType = 0xffff;
	RoFSlot.Unknown02 = 0;
	RoFSlot.MainSlot = 0xffff;
	RoFSlot.SubSlot = 0xffff;
	RoFSlot.AugSlot = 0xffff;
	RoFSlot.Unknown01 = 0;
	uint32 TempSlot = 0;

	if (TitaniumSlot < 56 || TitaniumSlot == 9999)	// Main Inventory and Cursor
	{
		RoFSlot.SlotType = 0;
		RoFSlot.MainSlot = TitaniumSlot;
		if (TitaniumSlot == 9999)
		{
			RoFSlot.MainSlot = 21;
		}
		else if (TitaniumSlot >= 30)	// Cursor and Extended Corpse Inventory
		{
			RoFSlot.MainSlot += 3;
		}
		else if (TitaniumSlot > 20)
		{
			RoFSlot.MainSlot += 1;
		}

	}
	/*else if (TitaniumSlot < 51)		// Cursor Buffer
	{
		RoFSlot.SlotType = 5;
		RoFSlot.MainSlot = TitaniumSlot - 31;
	}*/
	else if (TitaniumSlot > 250 && TitaniumSlot < 341)
	{
		RoFSlot.SlotType = 0;
		TempSlot = TitaniumSlot - 1;
		RoFSlot.MainSlot = int(TempSlot / 10) - 2;
		RoFSlot.SubSlot = TempSlot - ((RoFSlot.MainSlot + 2) * 10);
		if (RoFSlot.MainSlot > 30)
		{
			RoFSlot.MainSlot = 33;
		}
	}
	else if (TitaniumSlot > 399 && TitaniumSlot < 405)	// Tribute
	{
		RoFSlot.SlotType = 6;
		RoFSlot.MainSlot = TitaniumSlot - 400;
	}
	else if (TitaniumSlot > 1999 && TitaniumSlot < 2271)
	{
		RoFSlot.SlotType = 1;
		TempSlot = TitaniumSlot - 2000;
		RoFSlot.MainSlot = TempSlot;
		if (TempSlot > 30)
		{
			RoFSlot.MainSlot = int(TempSlot / 10) - 3;
			RoFSlot.SubSlot = TempSlot - ((RoFSlot.MainSlot + 3) * 10);
		}
	}
	else if (TitaniumSlot > 2499 && TitaniumSlot < 2551)
	{
		RoFSlot.SlotType = 2;
		TempSlot = TitaniumSlot - 2500;
		RoFSlot.MainSlot = TempSlot;
		if (TempSlot > 30)
		{
			RoFSlot.MainSlot = int(TempSlot / 10) - 3;
			RoFSlot.SubSlot = TempSlot - ((RoFSlot.MainSlot + 3) * 10);
		}
	}
	else if (TitaniumSlot > 2999 && TitaniumSlot < 3180)
	{
		RoFSlot.SlotType = 3;
		TempSlot = TitaniumSlot - 3000;
		RoFSlot.MainSlot = TempSlot;
		if (TempSlot > 99)
		{
			if (TempSlot > 100)
			{
				RoFSlot.MainSlot = int((TempSlot - 100) / 10);
			}
			else
			{
				RoFSlot.MainSlot = 0;
			}
			RoFSlot.SubSlot = TempSlot - (100 + RoFSlot.MainSlot);
		}
	}
	else if (TitaniumSlot > 3999 && TitaniumSlot < 4009)
	{
		RoFSlot.SlotType = 4;
		TempSlot = TitaniumSlot - 4000;
		RoFSlot.MainSlot = TempSlot;
	}

	_log(NET__ERROR, "Convert Titanium Slot %i to RoF Slots: Type %i, Unk2 %i, Main %i, Sub %i, Aug %i, Unk1 %i", TitaniumSlot, RoFSlot.SlotType, RoFSlot.Unknown02, RoFSlot.MainSlot, RoFSlot.SubSlot, RoFSlot.AugSlot, RoFSlot.Unknown01);

	return RoFSlot;
}

static inline uint32 RoFToTitaniumSlot(structs::ItemSlotStruct RoFSlot)
{
	uint32 TitaniumSlot = 0xffffffff;
	uint32 TempSlot = 0;

	if (RoFSlot.SlotType == 0 && RoFSlot.MainSlot < 57)	// Worn/Personal Inventory and Cursor (Originally 51)
	{
		if (RoFSlot.MainSlot == 21)			// Power Source
		{
			TempSlot = 9999;
		}
		else if (RoFSlot.MainSlot >= 33)	// Cursor and Extended Corpse Inventory
		{
			TempSlot = RoFSlot.MainSlot - 3;
		}
		/*else if (RoFSlot.MainSlot == 31 || RoFSlot.MainSlot == 32) { // 9th and 10th RoF inventory/corpse slots
			// Need to figure out what to do when we get these

			// The slot range of 0 - client_max is cross-utilized between player inventory and corpse inventory.
			// In the case of RoF, player inventory is addressed as 0 - 33 and corpse inventory is addressed as 23 - 56.
			// We 'could' assign the two new inventory slots as 9997 and 9998, and then work around their bag
			// slot assignments, but doing so may disrupt our ability to utilize the corpse looting range properly.

			// For now, it's probably best to leave as-is and let this work itself out in the inventory rework.
		}*/
		else if (RoFSlot.MainSlot >= 22)	// Ammo and Main Inventory
		{
			TempSlot = RoFSlot.MainSlot - 1;
		}
		else								// Worn Slots
		{
			TempSlot = RoFSlot.MainSlot;
		}

		if (RoFSlot.SubSlot >= 0)			// Bag Slots
		{
			TempSlot = ((TempSlot + 3) * 10) + RoFSlot.SubSlot + 1;
		}

		TitaniumSlot = TempSlot;
	}
	else if (RoFSlot.SlotType == 1)		// Bank Slots
	{
		TempSlot = 2000;
		if (RoFSlot.SubSlot >= 0)
		{
			TempSlot += ((RoFSlot.MainSlot + 3) * 10) + RoFSlot.SubSlot + 1;
		}
		else
		{
			TempSlot += RoFSlot.MainSlot;
		}
		TitaniumSlot = TempSlot;
	}
	else if (RoFSlot.SlotType == 2)		// Shared Bank Slots
	{
		TempSlot = 2500;
		if (RoFSlot.SubSlot >= 0)
		{
			TempSlot += ((RoFSlot.MainSlot + 3) * 10) + RoFSlot.SubSlot + 1;
		}
		else
		{
			TempSlot += RoFSlot.MainSlot;
		}
		TitaniumSlot = TempSlot;
	}
	else if (RoFSlot.SlotType == 3)		// Trade Slots
	{
		TempSlot = 3000;
		if (RoFSlot.SubSlot >= 0)
		{
			TempSlot += 100 + (RoFSlot.MainSlot * 10) + RoFSlot.SubSlot;
		}
		else
		{
			TempSlot += RoFSlot.MainSlot;
		}
		TitaniumSlot = TempSlot;
	}
	else if (RoFSlot.SlotType == 4)		// Tradeskill Container Slots
	{
		TempSlot = 4000;
		if (RoFSlot.MainSlot >= 0)
		{
			TempSlot += RoFSlot.MainSlot;
		}
		TitaniumSlot = TempSlot;
	}
	/*else if (RoFSlot.SlotType == 5)		// Cursor Buffer
	{
		TempSlot = 31;
		if (RoFSlot.MainSlot >= 0)
		{
			TempSlot += RoFSlot.MainSlot;
		}
		TitaniumSlot = TempSlot;
	}*/
	_log(NET__ERROR, "Convert RoF Slots: Type %i, Unk2 %i, Main %i, Sub %i, Aug %i, Unk1 %i to Titanium Slot %i", RoFSlot.SlotType, RoFSlot.Unknown02, RoFSlot.MainSlot, RoFSlot.SubSlot, RoFSlot.AugSlot, RoFSlot.Unknown01, TitaniumSlot);

	return TitaniumSlot;
}

static inline uint32 MainInvRoFToTitaniumSlot(structs::MainInvItemSlotStruct RoFSlot)
{
	uint32 TitaniumSlot = 0xffffffff;
	uint32 TempSlot = 0;

	if (RoFSlot.MainSlot < 57)				// Worn/Personal Inventory and Cursor (Originally 33)
	{
		if (RoFSlot.MainSlot == 21)
		{
			TempSlot = 9999;
		}
		else if (RoFSlot.MainSlot >= 33)	// Cursor and Extended Corpse Inventory
		{
			TempSlot = RoFSlot.MainSlot - 3;
		}
		/*else if (RoFSlot.MainSlot == 31 || RoFSlot.MainSlot == 32) { // 9th and 10th RoF inventory slots
			// Need to figure out what to do when we get these

			// Same as above
		}*/
		else if (RoFSlot.MainSlot >= 22)	// Main Inventory and Ammo Slots
		{
			TempSlot = RoFSlot.MainSlot - 1;
		}
		else
		{
			TempSlot = RoFSlot.MainSlot;
		}

		if (RoFSlot.SubSlot >= 0)			// Bag Slots
		{
			TempSlot = ((TempSlot + 3) * 10) + RoFSlot.SubSlot + 1;
		}

		TitaniumSlot = TempSlot;
	}

	_log(NET__ERROR, "Convert RoF Slots: Main %i, Sub %i, Aug %i, Unk1 %i to Titanium Slot %i", RoFSlot.MainSlot, RoFSlot.SubSlot, RoFSlot.AugSlot, RoFSlot.Unknown01, TitaniumSlot);

	return TitaniumSlot;
}

// Converts Titanium Slot IDs to RoF Slot IDs for use in Encodes
static inline structs::MainInvItemSlotStruct MainInvTitaniumToRoFSlot(uint32 TitaniumSlot)
{
	structs::MainInvItemSlotStruct RoFSlot;
	RoFSlot.MainSlot = 0xffff;
	RoFSlot.SubSlot = 0xffff;
	RoFSlot.AugSlot = 0xffff;
	RoFSlot.Unknown01 = 0;
	uint32 TempSlot = 0;

	if (TitaniumSlot < 56 || TitaniumSlot == 9999) // (Originally 52)
	{
		RoFSlot.MainSlot = TitaniumSlot;
		if (TitaniumSlot == 9999)
		{
			RoFSlot.MainSlot = 21;
		}
		else if (TitaniumSlot > 29) // Cursor and Extended Corpse Inventory
		{
			RoFSlot.MainSlot += 3;
		}
		else if(TitaniumSlot > 20) // Ammo and Personl Inventory
		{
			RoFSlot.MainSlot += 1;
		}
		/*else if (TitaniumSlot > 29)		// Cursor
		{
			RoFSlot.MainSlot = 33;
			if (TitaniumSlot > 30)
			{
				RoFSlot.SubSlot = (TitaniumSlot + 3) - 33;
			}
		}*/
	}
	else if (TitaniumSlot > 250 && TitaniumSlot < 341)
	{
		TempSlot = TitaniumSlot - 1;
		RoFSlot.MainSlot = int(TempSlot / 10) - 2;
		RoFSlot.SubSlot = TempSlot - ((RoFSlot.MainSlot + 2) * 10);
	}

	_log(NET__ERROR, "Convert Titanium Slot %i to RoF Slots: Main %i, Sub %i, Aug %i, Unk1 %i", TitaniumSlot, RoFSlot.MainSlot, RoFSlot.SubSlot, RoFSlot.AugSlot, RoFSlot.Unknown01);

	return RoFSlot;
}

ENCODE(OP_TaskHistoryReply)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	// First we need to calculate the length of the new packet
	in->SetReadPosition(4);
	uint32 ActivityCount = in->ReadUInt32();

	uint32 Text1Length = 0;
	uint32 Text2Length = 0;
	uint32 Text3Length = 0;

	uint32 OutboundPacketSize = 8;

	for(uint32 i = 0; i < ActivityCount; ++i)
	{
		Text1Length = 0;
		Text2Length = 0;
		Text3Length = 0;

		in->ReadUInt32(); // Activity type

		// Skip past Text1
		while(in->ReadUInt8())
			++Text1Length;

		// Skip past Text2
		while(in->ReadUInt8())
			++Text2Length;

		in->ReadUInt32();
		in->ReadUInt32();
		in->ReadUInt32();
		uint32 ZoneID = in->ReadUInt32();
		in->ReadUInt32();

		// Skip past Text3
		while(in->ReadUInt8())
			++Text3Length;

		char ZoneNumber[10];

		sprintf(ZoneNumber, "%i", ZoneID);

		OutboundPacketSize += (24 + Text1Length + 1 + Text2Length + Text3Length + 1 + 7 + (strlen(ZoneNumber) * 2));
	}

	in->SetReadPosition(0);

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_TaskHistoryReply, OutboundPacketSize);

	outapp->WriteUInt32(in->ReadUInt32());	// Task index
	outapp->WriteUInt32(in->ReadUInt32());	// Activity count

	for(uint32 i = 0; i < ActivityCount; ++i)
	{
		Text1Length = 0;
		Text2Length = 0;
		Text3Length = 0;

		outapp->WriteUInt32(in->ReadUInt32()); // ActivityType

		// Copy Text1
		while(uint8 c = in->ReadUInt8())
			outapp->WriteUInt8(c);

		outapp->WriteUInt8(0);	// Text1 has a null terminator

		uint32 CurrentPosition = in->GetReadPosition();

		// Determine Length of Text2
		while(in->ReadUInt8())
			++Text2Length;

		outapp->WriteUInt32(Text2Length);

		in->SetReadPosition(CurrentPosition);

		// Copy Text2
		while(uint8 c = in->ReadUInt8())
			outapp->WriteUInt8(c);

		outapp->WriteUInt32(in->ReadUInt32()); // Goalcount
		in->ReadUInt32();
		in->ReadUInt32();
		uint32 ZoneID = in->ReadUInt32();
		in->ReadUInt32();

		char ZoneNumber[10];

		sprintf(ZoneNumber, "%i", ZoneID);

		outapp->WriteUInt32(2);
		outapp->WriteUInt8(0x2d); // "-"
		outapp->WriteUInt8(0x31); // "1"

		outapp->WriteUInt32(2);
		outapp->WriteUInt8(0x2d); // "-"
		outapp->WriteUInt8(0x31); // "1"
		outapp->WriteString(ZoneNumber);

		outapp->WriteUInt32(0);

		// Copy Tex3t
		while(uint8 c = in->ReadUInt8())
			outapp->WriteUInt8(c);

		outapp->WriteUInt8(0);	// Text3 has a null terminator

		outapp->WriteUInt8(0x31); // "1"
		outapp->WriteString(ZoneNumber);
	}

	delete in;

	dest->FastQueuePacket(&outapp, ack_req);
}

ENCODE(OP_TaskDescription)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_TaskDescription, in->size + 1);
	// Set the Write pointer as we don't know what has been done with the packet before we get it.
	in->SetReadPosition(0);
	// Copy the header
	for(int i = 0; i < 5; ++i)
		outapp->WriteUInt32(in->ReadUInt32());

	// Copy Title
	while(uint8 c = in->ReadUInt8())
		outapp->WriteUInt8(c);
	outapp->WriteUInt8(0);

	outapp->WriteUInt32(in->ReadUInt32());	// Duration
	outapp->WriteUInt32(in->ReadUInt32());	// Unknown
	uint32 StartTime = in->ReadUInt32();
	outapp->WriteUInt32(time(nullptr) - StartTime);	// RoF has elapsed time here rather than starttime

	// Copy the rest of the packet verbatim
	uint32 BytesLeftToCopy = in->size - in->GetReadPosition();
	memcpy(outapp->pBuffer + outapp->GetWritePosition(), in->pBuffer + in->GetReadPosition(), BytesLeftToCopy);

	delete in;

	dest->FastQueuePacket(&outapp, ack_req);
}

/*
ENCODE(OP_OpenNewTasksWindow) {

	AvailableTaskHeader_Struct*	__emu_AvailableTaskHeader;
	AvailableTaskData1_Struct* 	__emu_AvailableTaskData1;
	AvailableTaskData2_Struct* 	__emu_AvailableTaskData2;
	AvailableTaskTrailer_Struct* 	__emu_AvailableTaskTrailer;

	structs::AvailableTaskHeader_Struct*	__eq_AvailableTaskHeader;
	structs::AvailableTaskData1_Struct* 	__eq_AvailableTaskData1;
	structs::AvailableTaskData2_Struct* 	__eq_AvailableTaskData2;
	structs::AvailableTaskTrailer_Struct* 	__eq_AvailableTaskTrailer;

	EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *__emu_buffer = in->pBuffer;

	__emu_AvailableTaskHeader = (AvailableTaskHeader_Struct*)__emu_buffer;

	// For each task, SoF has an extra uint32 and what appears to be space for a null terminated string.
	//
	in->size = in->size + (__emu_AvailableTaskHeader->TaskCount * 5);

	in->pBuffer = new unsigned char[in->size];

	unsigned char *__eq_buffer = in->pBuffer;

	__eq_AvailableTaskHeader = (structs::AvailableTaskHeader_Struct*)__eq_buffer;

	char *__eq_ptr, *__emu_Ptr;

	// Copy Header
	//
	//

	__eq_AvailableTaskHeader->TaskCount = __emu_AvailableTaskHeader->TaskCount;
	__eq_AvailableTaskHeader->unknown1 = __emu_AvailableTaskHeader->unknown1;
	__eq_AvailableTaskHeader->TaskGiver = __emu_AvailableTaskHeader->TaskGiver;

	__emu_Ptr = (char *) __emu_AvailableTaskHeader + sizeof(AvailableTaskHeader_Struct);
	__eq_ptr = (char *) __eq_AvailableTaskHeader + sizeof(structs::AvailableTaskHeader_Struct);

	for(uint32 i=0; i<__emu_AvailableTaskHeader->TaskCount; i++) {

		__emu_AvailableTaskData1 = (AvailableTaskData1_Struct*)__emu_Ptr;
		__eq_AvailableTaskData1 = (structs::AvailableTaskData1_Struct*)__eq_ptr;

		__eq_AvailableTaskData1->TaskID = __emu_AvailableTaskData1->TaskID;
		// This next unknown seems to affect the colour of the task title. 0x3f80000 is what I have seen
		// in RoF packets. Changing it to 0x3f000000 makes the title red.
		__eq_AvailableTaskData1->unknown1 = 0x3f800000;
		__eq_AvailableTaskData1->TimeLimit = __emu_AvailableTaskData1->TimeLimit;
		__eq_AvailableTaskData1->unknown2 = __emu_AvailableTaskData1->unknown2;

		__emu_Ptr += sizeof(AvailableTaskData1_Struct);
		__eq_ptr += sizeof(structs::AvailableTaskData1_Struct);

		strcpy(__eq_ptr, __emu_Ptr); // Title

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		strcpy(__eq_ptr, __emu_Ptr); // Description

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		__eq_ptr[0] = 0;
		__eq_ptr += strlen(__eq_ptr) + 1;

		__emu_AvailableTaskData2 = (AvailableTaskData2_Struct*)__emu_Ptr;
		__eq_AvailableTaskData2 = (structs::AvailableTaskData2_Struct*)__eq_ptr;

		__eq_AvailableTaskData2->unknown1 = __emu_AvailableTaskData2->unknown1;
		__eq_AvailableTaskData2->unknown2 = __emu_AvailableTaskData2->unknown2;
		__eq_AvailableTaskData2->unknown3 = __emu_AvailableTaskData2->unknown3;
		__eq_AvailableTaskData2->unknown4 = __emu_AvailableTaskData2->unknown4;

		__emu_Ptr += sizeof(AvailableTaskData2_Struct);
		__eq_ptr += sizeof(structs::AvailableTaskData2_Struct);

		strcpy(__eq_ptr, __emu_Ptr); // Unknown string

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		strcpy(__eq_ptr, __emu_Ptr); // Unknown string

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		__emu_AvailableTaskTrailer = (AvailableTaskTrailer_Struct*)__emu_Ptr;
		__eq_AvailableTaskTrailer = (structs::AvailableTaskTrailer_Struct*)__eq_ptr;

		__eq_AvailableTaskTrailer->ItemCount = __emu_AvailableTaskTrailer->ItemCount;
		__eq_AvailableTaskTrailer->unknown1 = __emu_AvailableTaskTrailer->unknown1;
		__eq_AvailableTaskTrailer->unknown2 = __emu_AvailableTaskTrailer->unknown2;
		__eq_AvailableTaskTrailer->StartZone = __emu_AvailableTaskTrailer->StartZone;

		__emu_Ptr += sizeof(AvailableTaskTrailer_Struct);
		__eq_ptr += sizeof(structs::AvailableTaskTrailer_Struct);

		strcpy(__eq_ptr, __emu_Ptr); // Unknown string

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;
	}

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}
*/

ENCODE(OP_SendCharInfo) {
	ENCODE_LENGTH_EXACT(CharacterSelect_Struct);
	SETUP_VAR_ENCODE(CharacterSelect_Struct);


	//EQApplicationPacket *packet = *p;
	//const CharacterSelect_Struct *emu = (CharacterSelect_Struct *) packet->pBuffer;

	int char_count;
	int namelen = 0;
	for(char_count = 0; char_count < 10; char_count++) {
		if(emu->name[char_count][0] == '\0')
			break;
		if(strcmp(emu->name[char_count], "<none>") == 0)
			break;
		namelen += strlen(emu->name[char_count]);
    }

	int total_length = sizeof(structs::CharacterSelect_Struct)
		+ char_count * sizeof(structs::CharacterSelectEntry_Struct)
		+ namelen;

	ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, total_length);

	//unsigned char *eq_buffer = new unsigned char[total_length];
	//structs::CharacterSelect_Struct *eq_head = (structs::CharacterSelect_Struct *) eq_buffer;

	eq->char_count = char_count;
	//eq->total_chars = 10;

	unsigned char *bufptr = (unsigned char *) eq->entries;
	int r;
	for(r = 0; r < char_count; r++) {
		{	//pre-name section...
			structs::CharacterSelectEntry_Struct *eq2 = (structs::CharacterSelectEntry_Struct *) bufptr;
			memcpy(eq2->name, emu->name[r], strlen(emu->name[r])+1);
		}
		//adjust for name.
		bufptr += strlen(emu->name[r]);
		{	//post-name section...
			structs::CharacterSelectEntry_Struct *eq2 = (structs::CharacterSelectEntry_Struct *) bufptr;
			eq2->class_ = emu->class_[r];
			eq2->race = emu->race[r];
			eq2->level = emu->level[r];
			eq2->class_2 = emu->class_[r];
			eq2->race2 = emu->race[r];
			eq2->zone = emu->zone[r];
			eq2->instance = 0;
			eq2->gender = emu->gender[r];
			eq2->face = emu->face[r];
			int k;
			for(k = 0; k < MAX_MATERIALS; k++) {
				eq2->equip[k].equip0 = emu->equip[r][k];
				eq2->equip[k].equip1 = 0;
				eq2->equip[k].equip2 = 0;
				eq2->equip[k].itemid = 0;
				eq2->equip[k].equip3 = emu->equip[r][k];
				eq2->equip[k].color.color = emu->cs_colors[r][k].color;
			}
			eq2->u15 = 0xff;
			eq2->u19 = 0xFF;
			eq2->drakkin_tattoo = emu->drakkin_tattoo[r];
			eq2->drakkin_details = emu->drakkin_details[r];
			eq2->deity = emu->deity[r];
			eq2->primary = emu->primary[r];
			eq2->secondary = emu->secondary[r];
			eq2->haircolor = emu->haircolor[r];
			eq2->beardcolor = emu->beardcolor[r];
			eq2->eyecolor1 = emu->eyecolor1[r];
			eq2->eyecolor2 = emu->eyecolor2[r];
			eq2->hairstyle = emu->hairstyle[r];
			eq2->beard = emu->beard[r];
			eq2->char_enabled = 1;
			eq2->tutorial = emu->tutorial[r];
			eq2->drakkin_heritage = emu->drakkin_heritage[r];
			eq2->unknown1 = 0;
			eq2->gohome = emu->gohome[r];
			eq2->LastLogin = 1212696584;
			eq2->unknown2 = 0;
		}
		bufptr += sizeof(structs::CharacterSelectEntry_Struct);
	}

	FINISH_ENCODE();

}

ENCODE(OP_ZoneServerInfo) {
	SETUP_DIRECT_ENCODE(ZoneServerInfo_Struct, ZoneServerInfo_Struct);
	OUT_str(ip);
	OUT(port);
	FINISH_ENCODE();
}

ENCODE(OP_SendZonepoints) {
	SETUP_VAR_ENCODE(ZonePoints);
	ALLOC_VAR_ENCODE(structs::ZonePoints, sizeof(structs::ZonePoints) + sizeof(structs::ZonePoint_Entry) * (emu->count + 1));

	eq->count = emu->count;
	for(uint32 i = 0; i < emu->count; ++i)
	{
		eq->zpe[i].iterator = emu->zpe[i].iterator;
		eq->zpe[i].x = emu->zpe[i].x;
		eq->zpe[i].y = emu->zpe[i].y;
		eq->zpe[i].z = emu->zpe[i].z;
		eq->zpe[i].heading = emu->zpe[i].heading;
		eq->zpe[i].zoneid = emu->zpe[i].zoneid;
		eq->zpe[i].zoneinstance = emu->zpe[i].zoneinstance;
	}

	FINISH_ENCODE();
}

ENCODE(OP_SendAATable) {
	ENCODE_LENGTH_ATLEAST(SendAA_Struct);

	SETUP_VAR_ENCODE(SendAA_Struct);
	ALLOC_VAR_ENCODE(structs::SendAA_Struct, sizeof(structs::SendAA_Struct) + emu->total_abilities*sizeof(structs::AA_Ability));

	// Check clientver field to verify this AA should be sent for SoF
	// clientver 1 is for all clients and 5 is for Live
	if (emu->clientver <= 5 )
	{
		OUT(id);
		eq->unknown004 = 1;
		//eq->hotkey_sid = (emu->hotkey_sid==4294967295UL)?0:(emu->id - emu->current_level + 1);
		//eq->hotkey_sid2 = (emu->hotkey_sid2==4294967295UL)?0:(emu->id - emu->current_level + 1);
		//eq->title_sid = emu->id - emu->current_level + 1;
		//eq->desc_sid = emu->id - emu->current_level + 1;
		eq->hotkey_sid = (emu->hotkey_sid==4294967295UL)?-1:(emu->sof_next_skill);
		eq->hotkey_sid2 = (emu->hotkey_sid2==4294967295UL)?-1:(emu->sof_next_skill);
		eq->title_sid = emu->sof_next_skill;
		eq->desc_sid = emu->sof_next_skill;
		OUT(class_type);
		OUT(cost);
		OUT(seq);
		OUT(current_level);
		eq->unknown037 = 1;	// Introduced during HoT
		OUT(prereq_skill);
		eq->unknown045 = 1;	// New Mar 21 2012 - Seen 1
		OUT(prereq_minpoints);
		eq->type = emu->sof_type;
		OUT(spellid);
		eq->unknown057 = 1;	// Introduced during HoT
		OUT(spell_type);
		OUT(spell_refresh);
		OUT(classes);
		OUT(berserker);
		//eq->max_level = emu->sof_max_level;
		OUT(max_level);
		OUT(last_id);
		OUT(next_id);
		OUT(cost2);
		eq->aa_expansion = emu->aa_expansion;
		eq->special_category = emu->special_category;
		OUT(total_abilities);
		unsigned int r;
		for(r = 0; r < emu->total_abilities; r++) {
			OUT(abilities[r].skill_id);
			OUT(abilities[r].base1);
			OUT(abilities[r].base2);
			OUT(abilities[r].slot);
		}
	}

	_hex(NET__ERROR, eq, sizeof(structs::SendAA_Struct) + emu->total_abilities*sizeof(structs::AA_Ability));
	FINISH_ENCODE();
}

ENCODE(OP_LeadershipExpUpdate) {
	SETUP_DIRECT_ENCODE(LeadershipExpUpdate_Struct, structs::LeadershipExpUpdate_Struct);
	OUT(group_leadership_exp);
	OUT(group_leadership_points);
	OUT(raid_leadership_exp);
	OUT(raid_leadership_points);
	FINISH_ENCODE();
}


ENCODE(OP_PlayerProfile)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *__emu_buffer = in->pBuffer;
	PlayerProfile_Struct *emu = (PlayerProfile_Struct *) __emu_buffer;

	uint32 PacketSize = 40000;	// Calculate this later

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_PlayerProfile, PacketSize);

	outapp->WriteUInt32(0);		// Checksum, we will update this later
	outapp->WriteUInt32(0);		// Checksum size, we will update this later

	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown


	outapp->WriteUInt8(emu->gender);	// Gender
	outapp->WriteUInt32(emu->race);		// Race
	outapp->WriteUInt8(emu->class_);	// Class
	outapp->WriteUInt8(emu->level);		// Level
	outapp->WriteUInt8(emu->level);		// Level1


	outapp->WriteUInt32(5);			// Bind count

	for(int r = 0; r < 5; r++)
	{
		outapp->WriteUInt32(emu->binds[r].zoneId);
		outapp->WriteFloat(emu->binds[r].x);
		outapp->WriteFloat(emu->binds[r].y);
		outapp->WriteFloat(emu->binds[r].z);
		outapp->WriteFloat(emu->binds[r].heading);
	}

	outapp->WriteUInt32(emu->deity);
	outapp->WriteUInt32(emu->intoxication);

	outapp->WriteUInt32(10);		// Unknown count

	for(int r = 0; r < 10; r++)
	{
		outapp->WriteUInt32(0);		// Unknown
	}

	outapp->WriteUInt32(22);		// Equipment count

	for(int r = 0; r < 9; r++)
	{
		outapp->WriteUInt32(emu->item_material[r]);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
	}

	// Write zeroes for the next 13 equipment slots

	for(int r = 0; r < 13; r++)
	{
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(9);		// Equipment2 count

	for(int r = 0; r < 9; r++)
	{
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(9);		// Tint Count

	for(int r = 0; r < 7; r++)
	{
		outapp->WriteUInt32(emu->item_tint[r].color);
	}
	// Write zeroes for extra two tint values
	outapp->WriteUInt32(0);
	outapp->WriteUInt32(0);

	outapp->WriteUInt32(9);		// Tint2 Count

	for(int r = 0; r < 7; r++)
	{
		outapp->WriteUInt32(emu->item_tint[r].color);
	}
	// Write zeroes for extra two tint values
	outapp->WriteUInt32(0);
	outapp->WriteUInt32(0);


	outapp->WriteUInt8(emu->haircolor);
	outapp->WriteUInt8(emu->beardcolor);
	outapp->WriteUInt32(0);			// Unknown
	outapp->WriteUInt8(emu->eyecolor1);
	outapp->WriteUInt8(emu->eyecolor2);
	outapp->WriteUInt8(emu->hairstyle);
	outapp->WriteUInt8(emu->beard);
	outapp->WriteUInt8(emu->face);

	// Think there should be an extra byte before the drakkin stuff (referred to as oldface in client)
	// Then one of the five bytes following the drakkin stuff needs removing.

	outapp->WriteUInt32(emu->drakkin_heritage);
	outapp->WriteUInt32(emu->drakkin_tattoo);
	outapp->WriteUInt32(emu->drakkin_details);


	outapp->WriteUInt8(0);			// Unknown
	outapp->WriteUInt8(0);			// Unknown
	outapp->WriteUInt8(0);			// Unknown
	outapp->WriteUInt8(0);			// Unknown
	outapp->WriteUInt8(0);			// Unknown

	outapp->WriteFloat(5.0f);		// Height ?

	outapp->WriteFloat(3.0f);			// Unknown
	outapp->WriteFloat(2.5f);			// Unknown
	outapp->WriteFloat(5.5f);			// Unknown

	outapp->WriteUInt32(0);			// Primary ?
	outapp->WriteUInt32(0);			// Secondary ?

	outapp->WriteUInt32(emu->points);	// Unspent skill points
	outapp->WriteUInt32(emu->mana);
	outapp->WriteUInt32(emu->cur_hp);

	outapp->WriteUInt32(emu->STR);
	outapp->WriteUInt32(emu->STA);
	outapp->WriteUInt32(emu->CHA);
	outapp->WriteUInt32(emu->DEX);
	outapp->WriteUInt32(emu->INT);
	outapp->WriteUInt32(emu->AGI);
	outapp->WriteUInt32(emu->WIS);

	outapp->WriteUInt32(0);			// Unknown
	outapp->WriteUInt32(0);			// Unknown
	outapp->WriteUInt32(0);			// Unknown
	outapp->WriteUInt32(0);			// Unknown
	outapp->WriteUInt32(0);			// Unknown
	outapp->WriteUInt32(0);			// Unknown
	outapp->WriteUInt32(0);			// Unknown


	outapp->WriteUInt32(300);		// AA Count

	for(uint32 r = 0; r < MAX_PP_AA_ARRAY; r++)
	{
		outapp->WriteUInt32(emu->aa_array[r].AA);
		outapp->WriteUInt32(emu->aa_array[r].value);
		outapp->WriteUInt32(0);
	}

	// Fill the other 60 AAs with zeroes

	for(uint32 r = 0; r < structs::MAX_PP_AA_ARRAY - MAX_PP_AA_ARRAY; r++)
	{
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
	}


	outapp->WriteUInt32(structs::MAX_PP_SKILL);

	for(uint32 r = 0; r < MAX_PP_SKILL; r++)
	{
		outapp->WriteUInt32(emu->skills[r]);
	}

	// Write zeroes for the rest of the skills
	for(uint32 r = 0; r < structs::MAX_PP_SKILL - MAX_PP_SKILL; r++)
	{
		outapp->WriteUInt32(emu->skills[r]);
	}

	outapp->WriteUInt32(25);			// Unknown count

	for(uint32 r = 0; r < 25; r++)
	{
		outapp->WriteUInt32(0);			// Unknown
	}

	outapp->WriteUInt32(structs::MAX_PP_DISCIPLINES);	// Discipline count

	for(uint32 r = 0; r < MAX_PP_DISCIPLINES; r++)
	{
		outapp->WriteUInt32(emu->disciplines.values[r]);
	}

	// Write zeroes for the rest of the disciplines
	for(uint32 r = 0; r < structs::MAX_PP_DISCIPLINES - MAX_PP_DISCIPLINES; r++)
	{
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(20);			// Timestamp count

	for(uint32 r = 0; r < 20; r++)
	{
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(MAX_RECAST_TYPES);			// Timestamp count

	for(uint32 r = 0; r < MAX_RECAST_TYPES; r++)
	{
		outapp->WriteUInt32(emu->recastTimers[r]);
	}

	outapp->WriteUInt32(100);			// Timestamp2 count

	for(uint32 r = 0; r < 100; r++)
	{
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(structs::MAX_PP_SPELLBOOK);		// Spellbook slots

	for(uint32 r = 0; r < MAX_PP_SPELLBOOK; r++)
	{
		outapp->WriteUInt32(emu->spell_book[r]);
	}
	// zeroes for the rest of the spellbook slots
	for(uint32 r = 0; r < structs::MAX_PP_SPELLBOOK - MAX_PP_SPELLBOOK; r++)
	{
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(structs::MAX_PP_MEMSPELL);		// Memorised spell slots

	for(uint32 r = 0; r < MAX_PP_MEMSPELL; r++)
	{
		outapp->WriteUInt32(emu->mem_spells[r]);
	}
	// zeroes for the rest of the slots
	for(uint32 r = 0; r < structs::MAX_PP_MEMSPELL - MAX_PP_MEMSPELL; r++)
	{
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(13);			// Unknown count

	for(uint32 r = 0; r < 13; r++)
	{
		outapp->WriteUInt32(0);			// Unknown
	}

	outapp->WriteUInt8(0);			// Unknown

	outapp->WriteUInt32(structs::BUFF_COUNT);

	//*000*/ uint8 slotid;				// badly named... seems to be 2 for a real buff, 0 otherwise
	//*001*/ float unknown004;			// Seen 1 for no buff
	//*005*/ uint32 player_id;			// 'global' ID of the caster, for wearoff messages
	//*009*/ uint32 unknown016;
	//*013*/ uint8 bard_modifier;
	//*014*/ uint32 duration;
	//*018*/ uint8 level;
	//*019*/ uint32 spellid;
	//*023*/ uint32 counters;
	//*027*/ uint8 unknown0028[53];
	//*080*/

	for(uint32 r = 0; r < BUFF_COUNT; r++)
	{
		float instrument_mod = 0.0f;
		uint8 slotid = emu->buffs[r].slotid;
		uint32 player_id = emu->buffs[r].player_id;;

		if(emu->buffs[r].spellid != 0xFFFF && emu->buffs[r].spellid != 0)
		{
			instrument_mod = 1.0f + (emu->buffs[r].bard_modifier - 10) / 10.0f;
			slotid = 2;
			player_id = 0x000717fd;
		}
		else
		{
			slotid = 0;
		}
		outapp->WriteUInt8(0);		// Had this as slot, but always appears to be 0 on live.
		outapp->WriteFloat(instrument_mod);
		outapp->WriteUInt32(player_id);
		outapp->WriteUInt8(0);
		outapp->WriteUInt32(emu->buffs[r].counters);
		//outapp->WriteUInt8(emu->buffs[r].bard_modifier);
		outapp->WriteUInt32(emu->buffs[r].duration);
		outapp->WriteUInt8(emu->buffs[r].level);
		outapp->WriteUInt32(emu->buffs[r].spellid);
		outapp->WriteUInt32(slotid);			// Only ever seen 2
		outapp->WriteUInt32(0);
		outapp->WriteUInt8(0);
		outapp->WriteUInt32(emu->buffs[r].counters);	// Appears twice ?

		for(uint32 j = 0; j < 44; ++j)
			outapp->WriteUInt8(0);	// Unknown
	}

	for(uint32 r = 0; r < structs::BUFF_COUNT - BUFF_COUNT; r++)
	{
		// 80 bytes of zeroes
		for(uint32 j = 0; j < 20; ++j)
			outapp->WriteUInt32(0);

	}

	outapp->WriteUInt32(emu->platinum);
	outapp->WriteUInt32(emu->gold);
	outapp->WriteUInt32(emu->silver);
	outapp->WriteUInt32(emu->copper);

	outapp->WriteUInt32(emu->platinum_cursor);
	outapp->WriteUInt32(emu->gold_cursor);
	outapp->WriteUInt32(emu->silver_cursor);
	outapp->WriteUInt32(emu->copper_cursor);

	outapp->WriteUInt32(0);		// Unknown

	outapp->WriteUInt32(0);		// This is the cooldown timer for the monk 'Mend' skill. Client will add 6 minutes to this value the first time the
								// player logs in. After that it will honour whatever value we send here.

	outapp->WriteUInt32(0);		// Unknown

	outapp->WriteUInt32(emu->thirst_level);
	outapp->WriteUInt32(emu->hunger_level);

	outapp->WriteUInt32(emu->aapoints_spent);

	outapp->WriteUInt32(5);				// AA Points count ??
	outapp->WriteUInt32(1234);			// AA Points assigned
	outapp->WriteUInt32(0);				// AA Points in General ?
	outapp->WriteUInt32(0);				// AA Points in Class ?
	outapp->WriteUInt32(0);				// AA Points in Archetype ?
	outapp->WriteUInt32(0);				// AA Points in Special ?
	outapp->WriteUInt32(emu->aapoints);		// AA Points unspent

	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown


	outapp->WriteUInt32(structs::MAX_PLAYER_BANDOLIER);

	for(uint32 r = 0; r < MAX_PLAYER_BANDOLIER; r++)
	{
		outapp->WriteString(emu->bandoliers[r].name);

		for(uint32 j = 0; j < MAX_PLAYER_BANDOLIER_ITEMS; ++j)
		{
			outapp->WriteString(emu->bandoliers[r].items[j].item_name);
			outapp->WriteUInt32(emu->bandoliers[r].items[j].item_id);
			outapp->WriteUInt32(emu->bandoliers[r].items[j].icon);
		}
	}

	for(uint32 r = 0; r < structs::MAX_PLAYER_BANDOLIER - MAX_PLAYER_BANDOLIER; r++)
	{
		outapp->WriteString("");

		for(uint32 j = 0; j < MAX_PLAYER_BANDOLIER_ITEMS; ++j)
		{
			outapp->WriteString("");
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}
	}


	outapp->WriteUInt32(structs::MAX_POTIONS_IN_BELT);

	for(uint32 r = 0; r < MAX_POTIONS_IN_BELT; r++)
	{
		outapp->WriteString(emu->potionbelt.items[r].item_name);
		outapp->WriteUInt32(emu->potionbelt.items[r].item_id);
		outapp->WriteUInt32(emu->potionbelt.items[r].icon);
	}


	for(uint32 r = 0; r < structs::MAX_POTIONS_IN_BELT - MAX_POTIONS_IN_BELT; r++)
	{
		outapp->WriteString("");
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);
	}

	outapp->WriteSInt32(-1);	// Unknown;
	outapp->WriteSInt32(123);	// HP Total ?
	outapp->WriteSInt32(234);	// Endurance Total ?
	outapp->WriteSInt32(345);	// Mana Total ?


	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown

	outapp->WriteUInt32(20);	// Unknown - Expansion count ?

	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(0);		// Unknown
	outapp->WriteUInt32(emu->endurance);
	outapp->WriteUInt32(0);		// Unknown - Observed 0x7cde - This is also seen in guild packets sent to this character.
	outapp->WriteUInt32(0);		// Unknown - Observed 0x64

	outapp->WriteUInt32(64);	// Name Length

	uint32 CurrentPosition = outapp->GetWritePosition();

	outapp->WriteString(emu->name);

	outapp->SetWritePosition(CurrentPosition + 64);

	outapp->WriteUInt32(32);	// Last Name Length

	CurrentPosition = outapp->GetWritePosition();

	outapp->WriteString(emu->last_name);

	outapp->SetWritePosition(CurrentPosition + 32);

	outapp->WriteUInt32(emu->birthday);
	outapp->WriteUInt32(emu->birthday);		// Account start date ?
	outapp->WriteUInt32(emu->lastlogin);
	outapp->WriteUInt32(emu->timePlayedMin);
	outapp->WriteUInt32(emu->timeentitledonaccount);
	outapp->WriteUInt32(0x0007ffff);		// Expansion bitmask

	outapp->WriteUInt32(structs::MAX_PP_LANGUAGE);

	for(uint32 r = 0; r < MAX_PP_LANGUAGE; r++)
	{
		outapp->WriteUInt8(emu->languages[r]);
	}

	for(uint32 r = 0; r < structs::MAX_PP_LANGUAGE - MAX_PP_LANGUAGE; r++)
	{
		outapp->WriteUInt8(0);
	}

	outapp->WriteUInt16(emu->zone_id);
	outapp->WriteUInt16(emu->zoneInstance);

	outapp->WriteFloat(emu->y);
	outapp->WriteFloat(emu->x);
	outapp->WriteFloat(emu->z);
	outapp->WriteFloat(emu->heading);

	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(emu->pvp);
	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(emu->gm);

	outapp->WriteUInt32(emu->guild_id);
	outapp->WriteUInt8(0);				// Unknown - observed 1 in a live packet.
	outapp->WriteUInt32(0);				// Unknown - observed 1 in a live packet.
	outapp->WriteUInt8(0);				// Unknown - observed 1 in a live packet.
	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt64(emu->exp);
	outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(emu->platinum_bank);
	outapp->WriteUInt32(emu->gold_bank);
	outapp->WriteUInt32(emu->silver_bank);
	outapp->WriteUInt32(emu->copper_bank);

	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt32(42);			// The meaning of life ?

	for(uint32 r = 0; r < 42; r++)
	{
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
	}

	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt32(emu->career_tribute_points);
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(emu->tribute_points);
	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(MAX_PLAYER_TRIBUTES);

	for(uint32 r = 0; r < MAX_PLAYER_TRIBUTES; r++)
	{
		outapp->WriteUInt32(emu->tributes[r].tribute);
		outapp->WriteUInt32(emu->tributes[r].tier);
	}

	outapp->WriteUInt32(10);		// Guild Tribute Count ?

	for(uint32 r = 0; r < 10; r++)
	{
		outapp->WriteUInt32(0xffffffff);
		outapp->WriteUInt32(0);
	}

	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown

	// Block of 121 unknown bytes
	for(uint32 r = 0; r < 121; r++)
		outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(emu->currentRadCrystals);
	outapp->WriteUInt32(emu->careerRadCrystals);
	outapp->WriteUInt32(emu->currentEbonCrystals);
	outapp->WriteUInt32(emu->careerEbonCrystals);
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown

	// Unknown String ?
	outapp->WriteUInt32(64);			// Unknown
	for(uint32 r = 0; r < 64; r++)
		outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown
	outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown

	// Unknown String ?
	outapp->WriteUInt32(64);			// Unknown
	for(uint32 r = 0; r < 64; r++)
		outapp->WriteUInt8(0);				// Unknown

	// Unknown String ?
	outapp->WriteUInt32(64);			// Unknown
	for(uint32 r = 0; r < 64; r++)
		outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown

	// Block of 320 unknown bytes
	for(uint32 r = 0; r < 320; r++)
		outapp->WriteUInt8(0);				// Unknown

	// Block of 343 unknown bytes
	for(uint32 r = 0; r < 343; r++)
		outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt8(emu->leadAAActive);

	outapp->WriteUInt32(6);				// Count ... of LDoN stats ?
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(emu->ldon_points_guk);
	outapp->WriteUInt32(emu->ldon_points_mir);
	outapp->WriteUInt32(emu->ldon_points_mmc);
	outapp->WriteUInt32(emu->ldon_points_ruj);
	outapp->WriteUInt32(emu->ldon_points_tak);

	outapp->WriteUInt32(emu->ldon_points_available);

	outapp->WriteDouble(emu->group_leadership_exp);
	outapp->WriteDouble(emu->raid_leadership_exp);

	outapp->WriteUInt32(emu->group_leadership_points);
	outapp->WriteUInt32(emu->raid_leadership_points);

	outapp->WriteUInt32(64);			// Group of 64 int32s follow	Group/Raid Leadership abilities ?

	for(uint32 r = 0; r < MAX_LEADERSHIP_AA_ARRAY; r++)
		outapp->WriteUInt32(emu->leader_abilities.ranks[r]);

	for(uint32 r = 0; r < 64 - MAX_LEADERSHIP_AA_ARRAY; r++)
		outapp->WriteUInt32(0);				// Unused/unsupported Leadership abilities

	outapp->WriteUInt32(emu->air_remaining);		// ?

	// PVP Stats

	outapp->WriteUInt32(emu->PVPKills);
	outapp->WriteUInt32(emu->PVPDeaths);
	outapp->WriteUInt32(emu->PVPCurrentPoints);
	outapp->WriteUInt32(emu->PVPCareerPoints);
	outapp->WriteUInt32(emu->PVPBestKillStreak);
	outapp->WriteUInt32(emu->PVPWorstDeathStreak);
	outapp->WriteUInt32(emu->PVPCurrentKillStreak);

	// Last PVP Kill

	outapp->WriteString(emu->PVPLastKill.Name);
	outapp->WriteUInt32(emu->PVPLastKill.Level);
	outapp->WriteUInt32(emu->PVPLastKill.Race);
	outapp->WriteUInt32(emu->PVPLastKill.Class);
	outapp->WriteUInt32(emu->PVPLastKill.Zone);
	outapp->WriteUInt32(emu->PVPLastKill.Time);
	outapp->WriteUInt32(emu->PVPLastKill.Points);

	// Last PVP Death

	outapp->WriteString(emu->PVPLastDeath.Name);
	outapp->WriteUInt32(emu->PVPLastDeath.Level);
	outapp->WriteUInt32(emu->PVPLastDeath.Race);
	outapp->WriteUInt32(emu->PVPLastDeath.Class);
	outapp->WriteUInt32(emu->PVPLastDeath.Zone);
	outapp->WriteUInt32(emu->PVPLastDeath.Time);
	outapp->WriteUInt32(emu->PVPLastDeath.Points);

	outapp->WriteUInt32(emu->PVPNumberOfKillsInLast24Hours);

	// Last 50 Kills
	outapp->WriteUInt32(50);
	for(uint32 r = 0; r < 50; ++r)
	{
		outapp->WriteString(emu->PVPRecentKills[r].Name);
		outapp->WriteUInt32(emu->PVPRecentKills[r].Level);
		outapp->WriteUInt32(emu->PVPRecentKills[r].Race);
		outapp->WriteUInt32(emu->PVPRecentKills[r].Class);
		outapp->WriteUInt32(emu->PVPRecentKills[r].Zone);
		outapp->WriteUInt32(emu->PVPRecentKills[r].Time);
		outapp->WriteUInt32(emu->PVPRecentKills[r].Points);
	}


	outapp->WriteUInt32(emu->expAA);
	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown

	outapp->WriteUInt8(emu->groupAutoconsent);
	outapp->WriteUInt8(emu->raidAutoconsent);
	outapp->WriteUInt8(emu->guildAutoconsent);

	outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(emu->level);				// Level3 ?

	outapp->WriteUInt8(emu->showhelm);

	outapp->WriteUInt32(emu->RestTimer);

	outapp->WriteUInt32(1024);			// Unknown Count

	// Block of 1024 unknown bytes
	outapp->WriteUInt8(31);				// Unknown

	for(uint32 r = 0; r < 1023; r++)
		outapp->WriteUInt8(0);				// Unknown

	outapp->WriteUInt32(0);				// Unknown
	outapp->WriteUInt32(0);				// Unknown

	// Think we need 1 byte of padding at the end

	outapp->WriteUInt8(0);				// Unknown


	_log(NET__STRUCTS, "Player Profile Packet is %i bytes", outapp->GetWritePosition());

	unsigned char *NewBuffer = new unsigned char[outapp->GetWritePosition()];
	memcpy(NewBuffer, outapp->pBuffer, outapp->GetWritePosition());
	safe_delete_array(outapp->pBuffer);
	outapp->pBuffer = NewBuffer;
	outapp->size = outapp->GetWritePosition();
	outapp->SetWritePosition(4);
	outapp->WriteUInt32(outapp->size - 9);


	CRC32::SetEQChecksum(outapp->pBuffer, outapp->size - 1, 8);
	//_hex(NET__ERROR, outapp->pBuffer, outapp->size);
	dest->FastQueuePacket(&outapp, ack_req);

	delete in;

	return;

}

ENCODE(OP_NewZone) {
	SETUP_DIRECT_ENCODE(NewZone_Struct, structs::NewZone_Struct);
	OUT_str(char_name);
	OUT_str(zone_short_name);
	OUT_str(zone_long_name);
	OUT(ztype);
	int r;
	for(r = 0; r < 4; r++) {
		OUT(fog_red[r]);
		OUT(fog_green[r]);
		OUT(fog_blue[r]);
		OUT(fog_minclip[r]);
		OUT(fog_maxclip[r]);
	}
	OUT(gravity);
	OUT(time_type);
	for(r = 16; r < 48; r++) {
		eq->unknown521[r] = 0xFF;	//observed
	}
	OUT(sky);
	OUT(zone_exp_multiplier);
	OUT(safe_y);
	OUT(safe_x);
	OUT(safe_z);
	OUT(max_z);
	OUT(underworld);
	OUT(minclip);
	OUT(maxclip);
	OUT_str(zone_short_name2);
	OUT(zone_id);
	OUT(zone_instance);
	OUT(SuspendBuffs);

	eq->FogDensity = emu->fog_density;

    /*fill in some unknowns with observed values, hopefully it will help */
	eq->unknown800 = -1;
	eq->unknown844 = 600;
	eq->unknown880 = 50;
	eq->unknown884 = 10;
	eq->unknown888 = 1;
	eq->unknown889 = 0;
	eq->unknown890 = 1;
	eq->unknown891 = 0;
	eq->unknown892 = 0;
	eq->unknown893 = 0;
	eq->fall_damage = 0;	// 0 = Fall Damage on, 1 = Fall Damage off
	eq->unknown895 = 0;
	eq->unknown896 = 180;
	eq->unknown900 = 180;
	eq->unknown904 = 180;
	eq->unknown908 = 2;
	eq->unknown912 = 2;
	eq->unknown932 = -1;	// Set from PoK Example
	eq->unknown936 = -1;	// Set from PoK Example
	eq->unknown944 = 1.0;	// Set from PoK Example

	FINISH_ENCODE();
}


ENCODE(OP_Track)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *__emu_buffer = in->pBuffer;
	Track_Struct *emu = (Track_Struct *) __emu_buffer;

	int EntryCount = in->size / sizeof(Track_Struct);

	if(EntryCount == 0 || ((in->size % sizeof(Track_Struct))) != 0)
	{
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Track_Struct));
		delete in;
		return;
	}

	int PacketSize = 2;

	for(int i = 0; i < EntryCount; ++i, ++emu)
		PacketSize += (12 + strlen(emu->name));

	emu = (Track_Struct *) __emu_buffer;

	in->size = PacketSize;
	in->pBuffer = new unsigned char[in->size];

	char *Buffer = (char *)in->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint16, Buffer, EntryCount);

	for(int i = 0; i < EntryCount; ++i, ++emu)
	{
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->entityid);
		VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->distance);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->level);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->NPC);
		VARSTRUCT_ENCODE_STRING(Buffer, emu->name);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->GroupMember);
	}

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_PetBuffWindow)
{
	// The format of the RoF packet is identical to the OP_BuffCreate packet.

	SETUP_VAR_ENCODE(PetBuff_Struct);

	uint32 sz = 12 + (17 * emu->buffcount);
	__packet->size = sz;
	__packet->pBuffer = new unsigned char[sz];
	memset(__packet->pBuffer, 0, sz);

	__packet->WriteUInt32(emu->petid);
	__packet->WriteUInt32(0);		// PlayerID ?
	__packet->WriteUInt8(1);		// 1 indicates all buffs on the pet (0 to add or remove a single buff)
	__packet->WriteUInt16(emu->buffcount);

	for(uint16 i = 0; i < BUFF_COUNT; ++i)
	{
		if(emu->spellid[i])
		{
			__packet->WriteUInt32(i);
			__packet->WriteUInt32(emu->spellid[i]);
			__packet->WriteUInt32(emu->ticsremaining[i]);
			__packet->WriteUInt32(0); // Unknown
			__packet->WriteString("");
		}
	}
	__packet->WriteUInt8(0); // Unknown

	FINISH_ENCODE();
}

ENCODE(OP_Barter)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	char *Buffer = (char *)in->pBuffer;

	uint32 SubAction = VARSTRUCT_DECODE_TYPE(uint32, Buffer);

	if(SubAction != Barter_BuyerAppearance)
	{
		dest->FastQueuePacket(&in, ack_req);

		return;
	}

	unsigned char *__emu_buffer = in->pBuffer;

	in->size = 80;

	in->pBuffer = new unsigned char[in->size];

	char *OutBuffer = (char *)in->pBuffer;

	char Name[64];

	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, SubAction);
	uint32 EntityID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, EntityID);
	uint8 Toggle = VARSTRUCT_DECODE_TYPE(uint8, Buffer);
	VARSTRUCT_DECODE_STRING(Name, Buffer);
	VARSTRUCT_ENCODE_STRING(OutBuffer, Name);
	OutBuffer = (char *)in->pBuffer + 72;
	VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, Toggle);

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);

}

ENCODE(OP_BazaarSearch)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	char *Buffer = (char *)in->pBuffer;

	uint8 SubAction = VARSTRUCT_DECODE_TYPE(uint8, Buffer);

	if(SubAction != BazaarSearchResults)
	{
		dest->FastQueuePacket(&in, ack_req);

		return;
	}

	unsigned char *__emu_buffer = in->pBuffer;

	BazaarSearchResults_Struct *emu = (BazaarSearchResults_Struct *) __emu_buffer;

	int EntryCount = in->size / sizeof(BazaarSearchResults_Struct);

	if(EntryCount == 0 || (in->size % sizeof(BazaarSearchResults_Struct)) != 0)
	{
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(BazaarSearchResults_Struct));
		delete in;
		return;
	}
	in->size = EntryCount * sizeof(structs::BazaarSearchResults_Struct);

	in->pBuffer = new unsigned char[in->size];

	memset(in->pBuffer, 0, in->size);

	structs::BazaarSearchResults_Struct *eq = (structs::BazaarSearchResults_Struct *)in->pBuffer;

	for(int i = 0; i < EntryCount; ++i, ++emu, ++eq)
	{
		OUT(Beginning.Action);
		OUT(SellerID);
		memcpy(eq->SellerName, emu->SellerName, sizeof(eq->SellerName));
		OUT(NumItems);
		OUT(ItemID);
		OUT(SerialNumber);
		memcpy(eq->ItemName, emu->ItemName, sizeof(eq->ItemName));
		OUT(Cost);
		OUT(ItemStat);
	}

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_NewSpawn) {  ENCODE_FORWARD(OP_ZoneSpawns); }
ENCODE(OP_ZoneEntry){  ENCODE_FORWARD(OP_ZoneSpawns); }
ENCODE(OP_ZoneSpawns)
{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		Spawn_Struct *emu = (Spawn_Struct *) __emu_buffer;

		//determine and verify length
		int entrycount = in->size / sizeof(Spawn_Struct);
		if(entrycount == 0 || (in->size % sizeof(Spawn_Struct)) != 0) {
			_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Spawn_Struct));
			delete in;
			return;
		}


		//_log(NET__STRUCTS, "Spawn name is [%s]", emu->name);

		emu = (Spawn_Struct *) __emu_buffer;

		//_log(NET__STRUCTS, "Spawn packet size is %i, entries = %i", in->size, entrycount);

		char *Buffer = (char *) in->pBuffer, *BufferStart;


		int r;
		int k;
		for(r = 0; r < entrycount; r++, emu++) {

			int PacketSize = 206;

			PacketSize += strlen(emu->name);
			PacketSize += strlen(emu->lastName);

			emu->title[0] = 0;
			emu->suffix[0] = 0;

			if(strlen(emu->title))
				PacketSize += strlen(emu->title) + 1;

			if(strlen(emu->suffix))
				PacketSize += strlen(emu->suffix) + 1;

			bool ShowName = 1;
			if(emu->bodytype >= 66)
			{
				emu->race = 127;
				emu->bodytype = 11;
				emu->gender = 0;
				ShowName = 0;
			}

			float SpawnSize = emu->size;
			if(!((emu->NPC == 0) || (emu->race <=12) || (emu->race == 128) || (emu->race == 130) || (emu->race == 330) || (emu->race == 522)))
			{
				PacketSize += 60;

				if(emu->size == 0)
				{
					emu->size = 6;
					SpawnSize = 6;
				}
			}
			else
				PacketSize += 216;

			if(SpawnSize == 0)
			{
				SpawnSize = 3;
			}

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_ZoneEntry, PacketSize);
			Buffer = (char *) outapp->pBuffer;
			BufferStart = Buffer;
			VARSTRUCT_ENCODE_STRING(Buffer, emu->name);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->spawnId);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->level);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, SpawnSize - 0.7);	// Eye Height?
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->NPC);

			structs::Spawn_Struct_Bitfields *Bitfields = (structs::Spawn_Struct_Bitfields*)Buffer;

			Bitfields->gender = emu->gender;
			Bitfields->ispet = emu->is_pet;
			Bitfields->afk = emu->afk;
			Bitfields->anon = emu->anon;
			Bitfields->gm = emu->gm;
			Bitfields->sneak = 0;
			Bitfields->lfg = emu->lfg;
			Bitfields->invis = emu->invis;
			Bitfields->linkdead = 0;
			Bitfields->showhelm = emu->showhelm;
			Bitfields->trader = 0;
			Bitfields->targetable = 1;
			Bitfields->targetable_with_hotkey = (emu->IsMercenary ? 0 : 1);
			Bitfields->showname = ShowName;

			// Not currently found
			// Bitfields->statue = 0;
			// Bitfields->buyer = 0;

			Buffer += sizeof(structs::Spawn_Struct_Bitfields);

			uint8 OtherData = 0;

			if(strlen(emu->title))
				OtherData = OtherData | 16;

			if(strlen(emu->suffix))
				OtherData = OtherData | 32;

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, OtherData);

			VARSTRUCT_ENCODE_TYPE(float, Buffer, -1);	// unknown3
			VARSTRUCT_ENCODE_TYPE(float, Buffer, 0);	// unknown4

			// Setting this next field to zero will cause a crash. Looking at ShowEQ, if it is zero, the bodytype field is not
			// present. Will sort that out later.
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 1);	// This is a properties count field
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->bodytype);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->curHp);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->haircolor);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->beardcolor);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->eyecolor1);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->eyecolor2);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->hairstyle);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->beard);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_heritage);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_tattoo);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_details);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->equip_chest2); // unknown8
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown9
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown10
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->helm); // unknown11

			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->size);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->face);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->walkspeed);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->runspeed);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->race);


			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// ShowEQ calls this 'Holding'
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->deity);
			if(emu->NPC)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xFFFFFFFF);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x00000000);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildID);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildrank);
			}

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->class_);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// pvp
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->StandState);	// standstate
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->light);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->flymode);

			VARSTRUCT_ENCODE_STRING(Buffer, emu->lastName);

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// aatitle ??
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->NPC ? 0 : 1); // unknown - Must be 1 for guild name to be shown abover players head.
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->petOwnerId);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown13
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown14 - Stance 64 = normal 4 = aggressive 40 = stun/mezzed
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown15
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown16
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown17
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // unknown18
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // unknown19

			if((emu->NPC == 0) || (emu->race <=12) || (emu->race == 128) || (emu->race == 130) || (emu->race == 330) || (emu->race == 522))
			{
				for(k = 0; k < 9; ++k)
				{
					{
						VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->colors[k].color);
					}
				}

				structs::EquipStruct *Equipment = (structs::EquipStruct *)Buffer;

				for(k = 0; k < 9; k++) {
					Equipment[k].equip0 = emu->equipment[k];
					Equipment[k].equip1 = 0;
					Equipment[k].equip2 = 0;
					Equipment[k].equip3 = 0;
					Equipment[k].itemId = 0;
				}

				Buffer += (sizeof(structs::EquipStruct) * 9);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment[MATERIAL_PRIMARY]);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment[MATERIAL_SECONDARY]);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			}


			structs::Spawn_Struct_Position *Position = (structs::Spawn_Struct_Position*)Buffer;

			Position->deltaX = emu->deltaX;
			Position->deltaHeading = emu->deltaHeading;
			Position->deltaY = emu->deltaY;
			Position->y = emu->y;
			Position->animation = emu->animation;
			Position->heading = emu->heading;
			Position->x = emu->x;
			Position->z = emu->z;
			Position->deltaZ = emu->deltaZ;

			Buffer += sizeof(structs::Spawn_Struct_Position);

			if(strlen(emu->title))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->title);
			}

			if(strlen(emu->suffix))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->suffix);
			}

			Buffer += 8;
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->IsMercenary);
			VARSTRUCT_ENCODE_STRING(Buffer, "0000000000000000");
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff);
			// 29 zero bytes follow
			Buffer += 29;
			if(Buffer != (BufferStart + PacketSize))
			{
				_log(NET__ERROR, "SPAWN ENCODE LOGIC PROBLEM: Buffer pointer is now %i from end", Buffer - (BufferStart + PacketSize));
			}
			//_log(NET__ERROR, "Sending zone spawn for %s packet is %i bytes", emu->name, outapp->size);
			//_hex(NET__ERROR, outapp->pBuffer, outapp->size);
			dest->FastQueuePacket(&outapp, ack_req);
	}

	delete in;
}

ENCODE(OP_MercenaryDataResponse) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;

	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	MercenaryMerchantList_Struct *emu = (MercenaryMerchantList_Struct *) __emu_buffer;

	char *Buffer = (char *) in->pBuffer;

	int PacketSize = sizeof(structs::MercenaryMerchantList_Struct) - 4 + emu->MercTypeCount * 4;
	PacketSize += (sizeof(structs::MercenaryListEntry_Struct) - sizeof(structs::MercenaryStance_Struct)) * emu->MercCount;

	uint32 r;
	uint32 k;
	for(r = 0; r < emu->MercCount; r++)
	{
		PacketSize += sizeof(structs::MercenaryStance_Struct) * emu->Mercs[r].StanceCount;
	}

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryDataResponse, PacketSize);
	Buffer = (char *) outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercTypeCount);

	for(r = 0; r < emu->MercTypeCount; r++)
	{
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercGrades[r]);
	}

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);

	for(r = 0; r < emu->MercCount; r++)
	{
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercID);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercType);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercSubType);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].PurchaseCost);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].UpkeepCost);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Status);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyCost);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyUpkeep);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyType);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->Mercs[r].MercUnk01);
		VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->Mercs[r].TimeLeft);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MerchantSlot);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercUnk02);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].StanceCount);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercUnk03);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->Mercs[r].MercUnk04);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// MercName
		for(k = 0; k < emu->Mercs[r].StanceCount; k++)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].StanceIndex);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].Stance);
		}
	}

	dest->FastQueuePacket(&outapp, ack_req);

	delete in;
}

ENCODE(OP_MercenaryDataUpdate) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;

	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	MercenaryDataUpdate_Struct *emu = (MercenaryDataUpdate_Struct *) __emu_buffer;

	char *Buffer = (char *) in->pBuffer;

	EQApplicationPacket *outapp;

	uint32 PacketSize = 0;

	// There are 2 different sized versions of this packet depending if a merc is hired or not
	if (emu->MercStatus >= 0)
	{
		PacketSize += sizeof(structs::MercenaryDataUpdate_Struct) + (sizeof(structs::MercenaryData_Struct) - sizeof(structs::MercenaryStance_Struct)) * emu->MercCount;

		uint32 r;
		uint32 k;
		for(r = 0; r < emu->MercCount; r++)
		{
			PacketSize += sizeof(structs::MercenaryStance_Struct) * emu->MercData[r].StanceCount;
			PacketSize += strlen(emu->MercData[r].MercName);	// Null Terminator size already accounted for in the struct
		}

		outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, PacketSize);
		Buffer = (char *) outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercStatus);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);

		for(r = 0; r < emu->MercCount; r++)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercID);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercType);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercSubType);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].PurchaseCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].UpkeepCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Status);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyUpkeep);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyType);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->MercData[r].MercUnk01);
			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercData[r].TimeLeft);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MerchantSlot);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercUnk02);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].StanceCount);
			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercData[r].MercUnk03);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->MercData[r].MercUnk04);
			//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// MercName
			VARSTRUCT_ENCODE_STRING(Buffer,emu->MercData[r].MercName);
			for(k = 0; k < emu->MercData[r].StanceCount; k++)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].StanceIndex);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].Stance);
			}
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);	// MercUnk05
		}
	}
	else
	{
		PacketSize += sizeof(structs::NoMercenaryHired_Struct);

		outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, PacketSize);
		Buffer = (char *) outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercStatus);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);
	}

	dest->FastQueuePacket(&outapp, ack_req);

	delete in;
}

ENCODE(OP_ItemLinkResponse) {  ENCODE_FORWARD(OP_ItemPacket); }
ENCODE(OP_ItemPacket) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *__emu_buffer = in->pBuffer;
	ItemPacket_Struct *old_item_pkt=(ItemPacket_Struct *)__emu_buffer;
	InternalSerializedItem_Struct *int_struct=(InternalSerializedItem_Struct *)(old_item_pkt->SerializedItem);

	uint32 length;
	char *serialized=SerializeItem((ItemInst *)int_struct->inst,int_struct->slot_id,&length,0);

	if (!serialized) {
		_log(NET__STRUCTS, "Serialization failed on item slot %d.",int_struct->slot_id);
		delete in;
		return;
	}
	in->size = length+4;
	in->pBuffer = new unsigned char[in->size];
	ItemPacket_Struct *new_item_pkt=(ItemPacket_Struct *)in->pBuffer;
	new_item_pkt->PacketType=old_item_pkt->PacketType;
	memcpy(new_item_pkt->SerializedItem,serialized,length);

	delete[] __emu_buffer;
	safe_delete_array(serialized);
	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_CharInventory) {
	//consume the packet
	EQApplicationPacket *in = *p;

	*p = nullptr;

	if(in->size == 0) {

		in->size = 4;

		in->pBuffer = new uchar[in->size];

		*((uint32 *) in->pBuffer) = 0;

		dest->FastQueuePacket(&in, ack_req);

		return;
	}

	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;

	int ItemCount = in->size / sizeof(InternalSerializedItem_Struct);

	if(ItemCount == 0 || (in->size % sizeof(InternalSerializedItem_Struct)) != 0) {

		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d",
				   opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(InternalSerializedItem_Struct));

		delete in;

		return;
	}

	InternalSerializedItem_Struct *eq = (InternalSerializedItem_Struct *) in->pBuffer;

	in->pBuffer = new uchar[4];

	*(uint32 *)in->pBuffer = ItemCount;

	in->size = 4;

	for(int r = 0; r < ItemCount; r++, eq++) {

		uint32 Length = 0;

		char* Serialized = SerializeItem((const ItemInst*)eq->inst, eq->slot_id, &Length, 0);

		if(Serialized) {

			uchar *OldBuffer = in->pBuffer;

			in->pBuffer = new uchar[in->size + Length];

			memcpy(in->pBuffer, OldBuffer, in->size);

			safe_delete_array(OldBuffer);

			memcpy(in->pBuffer + in->size, Serialized, Length);

			in->size += Length;

			safe_delete_array(Serialized);

		}
		else {
			_log(NET__ERROR, "Serialization failed on item slot %d during OP_CharInventory.  Item skipped.",eq->slot_id);
		}
	}

	delete[] __emu_buffer;

	//_log(NET__ERROR, "Sending inventory to client");

	//_hex(NET__ERROR, in->pBuffer, in->size);

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_GuildMemberList) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;

	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	Internal_GuildMembers_Struct *emu = (Internal_GuildMembers_Struct *) in->pBuffer;



	//make a new EQ buffer.
	uint32 pnl = strlen(emu->player_name);
	uint32 length = sizeof(structs::GuildMembers_Struct) + pnl +
		emu->count*sizeof(structs::GuildMemberEntry_Struct)
		+ emu->name_length + emu->note_length;
	in->pBuffer = new uint8[length];
	in->size = length;
	//no memset since we fill every byte.

	uint8 *buffer;
	buffer = in->pBuffer;

	//easier way to setup GuildMembers_Struct
	//set prefix name
	strcpy((char *)buffer, emu->player_name);
	buffer += pnl;
	*buffer = '\0';
	buffer++;

	// Guild ID
	buffer += sizeof(uint32);

	//add member count.
	*((uint32 *) buffer) = htonl( emu->count );
	buffer += sizeof(uint32);

	if(emu->count > 0) {
		Internal_GuildMemberEntry_Struct *emu_e = emu->member;
		const char *emu_name = (const char *) (__emu_buffer +
				sizeof(Internal_GuildMembers_Struct) + //skip header
				emu->count * sizeof(Internal_GuildMemberEntry_Struct)	//skip static length member data
				);
		const char *emu_note = (emu_name +
				emu->name_length + //skip name contents
				emu->count	//skip string terminators
				);

		structs::GuildMemberEntry_Struct *e = (structs::GuildMemberEntry_Struct *) buffer;

		uint32 r;
		for(r = 0; r < emu->count; r++, emu_e++) {

			//the order we set things here must match the struct

//nice helper macro
#define SlideStructString(field, str) \
		{ \
			int sl = strlen(str); \
			memcpy(e->field, str, sl+1); \
			e = (structs::GuildMemberEntry_Struct *) ( ((uint8 *)e) + sl ); \
			str += sl + 1; \
		}
#define PutFieldN(field) \
		e->field = htonl(emu_e->field)

			SlideStructString( name, emu_name );
			PutFieldN(level);
			PutFieldN(banker);
			PutFieldN(class_);
			PutFieldN(rank);
			PutFieldN(time_last_on);
			PutFieldN(tribute_enable);
			e->unknown01 = 0;
			PutFieldN(total_tribute);
			PutFieldN(last_tribute);
			e->unknown_one = htonl(1);
			SlideStructString( public_note, emu_note );
			e->zoneinstance = 0;
			e->zone_id = htons(emu_e->zone_id);
			e->unknown_one2 = htonl(1);
			e->unknown04 = 0;


#undef SlideStructString
#undef PutFieldN

			e++;
		}
	}


	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}


ENCODE(OP_SpawnDoor) {
	SETUP_VAR_ENCODE(Door_Struct);
	int door_count = __packet->size/sizeof(Door_Struct);
	int total_length = door_count * sizeof(structs::Door_Struct);
	ALLOC_VAR_ENCODE(structs::Door_Struct, total_length);
	int r;
	for(r = 0; r < door_count; r++) {
		strncpy(eq[r].name, emu[r].name, sizeof(eq[r].name));
		eq[r].xPos = emu[r].xPos;
		eq[r].yPos = emu[r].yPos;
		eq[r].zPos = emu[r].zPos;
		eq[r].heading = emu[r].heading;
		eq[r].incline = emu[r].incline;
		eq[r].size = emu[r].size;
		eq[r].doorId = emu[r].doorId;
		eq[r].opentype = emu[r].opentype;
		eq[r].state_at_spawn = emu[r].state_at_spawn;
		eq[r].invert_state = emu[r].invert_state;
		eq[r].door_param = emu[r].door_param;
		eq[r].unknown0080 = 0;
		eq[r].unknown0081 = 1; // Both must be 1 to allow clicking doors
		eq[r].unknown0082 = 0;
		eq[r].unknown0083 = 1; // Both must be 1 to allow clicking doors
		eq[r].unknown0084 = 0;
		eq[r].unknown0085 = 0;
		eq[r].unknown0086 = 0;
	}
	FINISH_ENCODE();
}

ENCODE(OP_GroundSpawn)
{

	// We are not encoding the spawn_id field here, but it doesn't appear to matter.
	//
	EQApplicationPacket *in = *p;
	*p = nullptr;

	Object_Struct *emu = (Object_Struct *) in->pBuffer;

	unsigned char *__emu_buffer = in->pBuffer;

	in->size = strlen(emu->object_name) + sizeof(Object_Struct) - 1;

	in->pBuffer = new unsigned char[in->size];

	char *OutBuffer = (char *)in->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->drop_id);
	VARSTRUCT_ENCODE_STRING(OutBuffer, emu->object_name);
	VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_id);
	VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_instance);
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->drop_id);	// Some unique id
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Same for all objects in the zone
	VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->heading);
	VARSTRUCT_ENCODE_TYPE(float, OutBuffer, 0);	// Normally 0, but seen (float)255.0 as well
	VARSTRUCT_ENCODE_TYPE(float, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(float, OutBuffer, 1);	// Need to add emu->size to struct
	VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->y);
	VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->x);
	VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->z);
	VARSTRUCT_ENCODE_TYPE(int32, OutBuffer, emu->object_type);	// Unknown, observed 0x00000014

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_ClickObjectAction) {
	ENCODE_LENGTH_EXACT(ClickObjectAction_Struct);
	SETUP_DIRECT_ENCODE(ClickObjectAction_Struct, structs::ClickObjectAction_Struct);
	OUT(drop_id);
	eq->unknown04 = -1;
	eq->unknown08 = -1;
	OUT(type);
	OUT(icon);
	eq->unknown16 = 0;
	OUT_str(object_name);
	FINISH_ENCODE();
}

ENCODE(OP_SendMembership) {
	ENCODE_LENGTH_EXACT(Membership_Struct);
	SETUP_DIRECT_ENCODE(Membership_Struct, structs::Membership_Struct);

	eq->membership = emu->membership;
	eq->races = emu->races;
	eq->classes = emu->classes;
	eq->entrysize = 22;
	for (int i=0; i<21; i++)
	{
		eq->entries[i] = emu->entries[i];
	}
	eq->entries[21] = 0;

	FINISH_ENCODE();
}

ENCODE(OP_GMTrainSkillConfirm) {
	ENCODE_LENGTH_EXACT(GMTrainSkillConfirm_Struct);
	SETUP_DIRECT_ENCODE(GMTrainSkillConfirm_Struct, structs::GMTrainSkillConfirm_Struct);
	OUT(SkillID);
	OUT(Cost);
	OUT(NewSkill);
	OUT_str(TrainerName);
	FINISH_ENCODE();
}

ENCODE(OP_GMLastName) {
	ENCODE_LENGTH_EXACT(GMLastName_Struct);
	SETUP_DIRECT_ENCODE(GMLastName_Struct, structs::GMLastName_Struct);
	OUT_str(name);
	OUT_str(gmname);
	OUT_str(lastname);
	for (int i=0; i<4; i++)
	{
		eq->unknown[i] = emu->unknown[i];
	}
	FINISH_ENCODE();
}

ENCODE(OP_SkillUpdate) {
	ENCODE_LENGTH_EXACT(SkillUpdate_Struct);
	SETUP_DIRECT_ENCODE(SkillUpdate_Struct, structs::SkillUpdate_Struct);
	OUT(skillId);
	OUT(value);
	eq->unknown08 = 1;		// Observed
	eq->unknown09 = 80;		// Observed
	eq->unknown10 = 136;	// Observed
	eq->unknown11 = 54;		// Observed
	FINISH_ENCODE();
}

ENCODE(OP_ManaChange) {
	ENCODE_LENGTH_EXACT(ManaChange_Struct);
	SETUP_DIRECT_ENCODE(ManaChange_Struct, structs::ManaChange_Struct);
	OUT(new_mana);
	OUT(stamina);
	OUT(spell_id);
	eq->unknown16 = -1; // Self Interrupt/Success = -1, Fizzle = 1, Other Interrupt = 2?
	FINISH_ENCODE();
}

ENCODE(OP_RequestClientZoneChange) {
	ENCODE_LENGTH_EXACT(RequestClientZoneChange_Struct);
	SETUP_DIRECT_ENCODE(RequestClientZoneChange_Struct, structs::RequestClientZoneChange_Struct);
	OUT(zone_id);
	OUT(instance_id);
	OUT(y);
	OUT(x);
	OUT(z);
	OUT(heading);
	eq->type = 0x0b;
	eq->unknown004 = 0xffffffff;
	eq->unknown172 = 0x0168b500;
	FINISH_ENCODE();
}

ENCODE(OP_OnLevelMessage)
{
	ENCODE_LENGTH_EXACT(OnLevelMessage_Struct);
	SETUP_DIRECT_ENCODE(OnLevelMessage_Struct, structs::OnLevelMessage_Struct);

	// This packet is variable sized now, but forcing it to the old packet size for now.
	eq->Title_Count = 128;
	memcpy(eq->Title, emu->Title, sizeof(eq->Title));
	eq->Text_Count = 4096;
	memcpy(eq->Text, emu->Text, sizeof(eq->Text));
	OUT(Buttons);
	OUT(Duration);
	OUT(PopupID);
	OUT(NegativeID);
	// These two field names are used if Buttons == 1. We should add an interface to them via Perl.
	eq->ButtonName0_Count = 25;
	OUT_str(ButtonName0);
	eq->ButtonName1_Count = 25;
	OUT_str(ButtonName1);

	FINISH_ENCODE();
}

ENCODE(OP_Illusion) {
	ENCODE_LENGTH_EXACT(Illusion_Struct);
	SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);
	OUT(spawnid);
	OUT_str(charname);
	OUT(race);
	OUT(unknown006[0]);
	OUT(unknown006[1]);
	OUT(gender);
	OUT(texture);
	OUT(helmtexture);
	OUT(face);
	OUT(hairstyle);
	OUT(haircolor);
	OUT(beard);
	OUT(beardcolor);
	OUT(size);
	OUT(drakkin_heritage);
	OUT(drakkin_tattoo);
	OUT(drakkin_details);
	eq->unknown316 = -1;	// Observed

	FINISH_ENCODE();
}

ENCODE(OP_ShopPlayerBuy)
{
	ENCODE_LENGTH_EXACT(Merchant_Sell_Struct);
	SETUP_DIRECT_ENCODE(Merchant_Sell_Struct, structs::Merchant_Sell_Struct);
	OUT(npcid);
	OUT(playerid);
	OUT(itemslot);
	OUT(quantity);
	OUT(price);

	FINISH_ENCODE();
}

ENCODE(OP_DeleteSpawn)
{
	ENCODE_LENGTH_EXACT(DeleteSpawn_Struct);
	SETUP_DIRECT_ENCODE(DeleteSpawn_Struct, structs::DeleteSpawn_Struct);
	OUT(spawn_id);
	eq->unknown04 = 1;	// Observed

	FINISH_ENCODE();
}


ENCODE(OP_ClientUpdate) {
	ENCODE_LENGTH_EXACT(PlayerPositionUpdateServer_Struct);
	SETUP_DIRECT_ENCODE(PlayerPositionUpdateServer_Struct, structs::PlayerPositionUpdateServer_Struct);
	OUT(spawn_id);
	OUT(x_pos);
	OUT(delta_x);
	OUT(delta_y);
	OUT(z_pos);
	OUT(delta_heading);
	OUT(y_pos);
	OUT(delta_z);
	OUT(animation);
	OUT(heading);
	FINISH_ENCODE();
}

ENCODE(OP_ExpansionInfo) {
	ENCODE_LENGTH_EXACT(ExpansionInfo_Struct);
	SETUP_DIRECT_ENCODE(ExpansionInfo_Struct, structs::ExpansionInfo_Struct);
	OUT(Expansions);
	FINISH_ENCODE();
}

ENCODE(OP_LogServer) {
	ENCODE_LENGTH_EXACT(LogServer_Struct);
 	SETUP_DIRECT_ENCODE(LogServer_Struct, structs::LogServer_Struct);
 	strncpy(eq->worldshortname, emu->worldshortname, sizeof(eq->worldshortname));

 	//OUT(enablevoicemacros);	// These two are lost, but must be one of the 1s in unknown[249]
 	//OUT(enablemail);
	OUT(enable_pvp);
	OUT(enable_FV);

	eq->unknown016 = 1;
	eq->unknown020[0] = 1;

 	eq->unknown249[0] = 1;
 	eq->unknown249[1] = 1;
 	eq->unknown249[8] = 1;
 	eq->unknown249[9] = 1;
 	eq->unknown249[12] = 1;
 	eq->unknown249[14] = 1;
 	eq->unknown249[15] = 1;
 	eq->unknown249[16] = 1;

	eq->unknown276[0] = 1.0f;
	eq->unknown276[1] = 1.0f;
	eq->unknown276[6] = 1.0f;

 	FINISH_ENCODE();
}

ENCODE(OP_Animation) {
	ENCODE_LENGTH_EXACT(Animation_Struct);
	SETUP_DIRECT_ENCODE(Animation_Struct, structs::Animation_Struct);
	OUT(spawnid);
	OUT(value);
	OUT(action);
	FINISH_ENCODE();
}

ENCODE(OP_Damage) {
	ENCODE_LENGTH_EXACT(CombatDamage_Struct);
	SETUP_DIRECT_ENCODE(CombatDamage_Struct, structs::CombatDamage_Struct);
	OUT(target);
	OUT(source);
	OUT(type);
	OUT(spellid);
	OUT(damage);
	eq->sequence = emu->sequence;
	FINISH_ENCODE();
}

ENCODE(OP_Consider) {
	ENCODE_LENGTH_EXACT(Consider_Struct);
	SETUP_DIRECT_ENCODE(Consider_Struct, structs::Consider_Struct);
	OUT(playerid);
	OUT(targetid);
	OUT(faction);
	OUT(level);
	OUT(pvpcon);
	FINISH_ENCODE();
}

ENCODE(OP_Action) {
	ENCODE_LENGTH_EXACT(Action_Struct);
	SETUP_DIRECT_ENCODE(Action_Struct, structs::ActionAlt_Struct);
	OUT(target);
	OUT(source);
	OUT(level);
	eq->unknown06 = 0;
	eq->instrument_mod = 1.0f + (emu->instrument_mod - 10) / 10.0f;
	eq->bard_focus_id = emu->bard_focus_id;
	eq->knockback_angle = emu->sequence;
	eq->unknown22 = 0;
	OUT(type);
	eq->damage = 0;
	eq->unknown31 = 0;
	OUT(spell);
	eq->level2 = eq->level;
	eq->effect_flag = emu->buff_unknown;
	eq->unknown39 = 14;
	eq->unknown43 = 0;
	eq->unknown44 = 17;
	eq->unknown45 = 0;
	eq->unknown46 = -1;
	eq->unknown50 = 0;
	eq->unknown54 = 0;
	FINISH_ENCODE();
}

ENCODE(OP_Buff) {
	ENCODE_LENGTH_EXACT(SpellBuffFade_Struct);
	SETUP_DIRECT_ENCODE(SpellBuffFade_Struct, structs::SpellBuffFade_Struct_Live);
	OUT(entityid);
	eq->unknown004 = 2;
	//eq->level = 80;
	//eq->effect = 0;
	OUT(level);
	OUT(effect);
	eq->unknown007 = 0;
	eq->unknown008 = 1.0f;
	OUT(spellid);
	OUT(duration);
	eq->playerId = 0x7cde;
	OUT(slotid);
	if(emu->bufffade == 1)
		eq->bufffade = 1;
	else
		eq->bufffade = 2;

	// Bit of a hack. OP_Buff appears to add/remove the buff while OP_BuffCreate adds/removes the actual buff icon
	EQApplicationPacket *outapp = nullptr;
	if(eq->bufffade == 1)
	{
		outapp = new EQApplicationPacket(OP_BuffCreate, 29);
		outapp->WriteUInt32(emu->entityid);
		outapp->WriteUInt32(0x0271);	// Unk
		outapp->WriteUInt8(0);		// Type of OP_BuffCreate packet ?
		outapp->WriteUInt16(1);		// 1 buff in this packet
		outapp->WriteUInt32(emu->slotid);
		outapp->WriteUInt32(0xffffffff);		// SpellID (0xffff to remove)
		outapp->WriteUInt32(0);			// Duration
		outapp->WriteUInt32(0);			// ?
		outapp->WriteUInt8(0);		// Caster name
		outapp->WriteUInt8(0);		// Terminating byte
	}
	FINISH_ENCODE();

	if(outapp)
		dest->FastQueuePacket(&outapp);	// Send the OP_BuffCreate to remove the buff
}

ENCODE(OP_CancelTrade) {
	ENCODE_LENGTH_EXACT(CancelTrade_Struct);
	SETUP_DIRECT_ENCODE(CancelTrade_Struct, structs::CancelTrade_Struct);
	OUT(fromid);
	OUT(action);
	FINISH_ENCODE();
}

ENCODE(OP_InterruptCast) {
	ENCODE_LENGTH_EXACT(InterruptCast_Struct);
	SETUP_DIRECT_ENCODE(InterruptCast_Struct, structs::InterruptCast_Struct);
	OUT(spawnid);
	OUT(messageid);
	FINISH_ENCODE();
}

ENCODE(OP_ShopPlayerSell) {
	ENCODE_LENGTH_EXACT(Merchant_Purchase_Struct);
	SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);
	OUT(npcid);
	eq->itemslot = MainInvTitaniumToRoFSlot(emu->itemslot);
	//OUT(itemslot);
	OUT(quantity);
	OUT(price);
	FINISH_ENCODE();
}

ENCODE(OP_ApplyPoison) {
	ENCODE_LENGTH_EXACT(ApplyPoison_Struct);
	SETUP_DIRECT_ENCODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);
	eq->inventorySlot = MainInvTitaniumToRoFSlot(emu->inventorySlot);
	OUT(success);
	FINISH_ENCODE();
}

ENCODE(OP_RecipeAutoCombine) {
	ENCODE_LENGTH_EXACT(RecipeAutoCombine_Struct);
	SETUP_DIRECT_ENCODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);
	OUT(object_type);
	OUT(some_id);
	eq->container_slot = TitaniumToRoFSlot(emu->unknown1);
	structs::ItemSlotStruct RoFSlot;
	RoFSlot.SlotType = 8;	// Observed
	RoFSlot.Unknown02 = 0;
	RoFSlot.MainSlot = 0xffff;
	RoFSlot.SubSlot = 0xffff;
	RoFSlot.AugSlot = 0xffff;
	RoFSlot.Unknown01 = 0;
	eq->unknown_slot = RoFSlot;
	OUT(recipe_id);
	OUT(reply_code);
	FINISH_ENCODE();
}

ENCODE(OP_DeleteItem) {
	ENCODE_LENGTH_EXACT(DeleteItem_Struct);
	SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

	eq->from_slot = TitaniumToRoFSlot(emu->from_slot);
	eq->to_slot = TitaniumToRoFSlot(emu->to_slot);
	OUT(number_in_stack);

	FINISH_ENCODE();
}

ENCODE(OP_DeleteCharge) {  ENCODE_FORWARD(OP_MoveItem); }
ENCODE(OP_MoveItem) {
	ENCODE_LENGTH_EXACT(MoveItem_Struct);
	SETUP_DIRECT_ENCODE(MoveItem_Struct, structs::MoveItem_Struct);

	eq->from_slot = TitaniumToRoFSlot(emu->from_slot);
	eq->to_slot = TitaniumToRoFSlot(emu->to_slot);
	OUT(number_in_stack);
	FINISH_ENCODE();
}

ENCODE(OP_ItemVerifyReply) {
	ENCODE_LENGTH_EXACT(ItemVerifyReply_Struct);
	SETUP_DIRECT_ENCODE(ItemVerifyReply_Struct, structs::ItemVerifyReply_Struct);

	eq->slot = TitaniumToRoFSlot(emu->slot);
	OUT(spell);
	OUT(target);

	FINISH_ENCODE();
}

ENCODE(OP_Trader) {

	if((*p)->size == sizeof(ClickTrader_Struct))
	{
		ENCODE_LENGTH_EXACT(ClickTrader_Struct);
		SETUP_DIRECT_ENCODE(ClickTrader_Struct, structs::ClickTrader_Struct);

		eq->Code = emu->Code;
		// Live actually has 200 items now, but 80 is the most our internal struct supports
		for (uint32 i = 0; i < 200; i++)
		{
			strncpy(eq->items[i].SerialNumber, "0000000000000000", sizeof(eq->items[i].SerialNumber));
			eq->items[i].Unknown18 = 0;
			if (i < 80) {
				eq->ItemCost[i] = emu->ItemCost[i];
			} else {
				eq->ItemCost[i] = 0;
			}
		}
		FINISH_ENCODE();
	}
	else if((*p)->size == sizeof(Trader_ShowItems_Struct))
	{
		ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
		SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
		eq->Code = emu->Code;
		strncpy(eq->SerialNumber, "0000000000000000", sizeof(eq->SerialNumber));
		eq->TraderID = emu->TraderID;
		eq->Stacksize = 0;
		eq->Price = 0;
		FINISH_ENCODE();
	}
	else if((*p)->size == sizeof(TraderStatus_Struct))
	{
		ENCODE_LENGTH_EXACT(TraderStatus_Struct);
		SETUP_DIRECT_ENCODE(TraderStatus_Struct, structs::TraderStatus_Struct);
		eq->Code = emu->Code;
		FINISH_ENCODE();
	}
	else if((*p)->size == sizeof(TraderBuy_Struct))
	{
		ENCODE_FORWARD(OP_TraderBuy);
	}
}

ENCODE(OP_TraderBuy) {

	ENCODE_LENGTH_EXACT(TraderBuy_Struct);
	SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);

	OUT(Action);
	OUT(Price);
	OUT(TraderID);
	memcpy(eq->ItemName, emu->ItemName, sizeof(eq->ItemName));
	OUT(ItemID);
	OUT(Quantity);
	OUT(AlreadySold);

	FINISH_ENCODE();
}

ENCODE(OP_LootItem) {

	ENCODE_LENGTH_EXACT(LootingItem_Struct);
	SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);
	OUT(lootee);
	OUT(looter);
	eq->slot_id = emu->slot_id + 1;
	OUT(auto_loot);

	FINISH_ENCODE();
}

ENCODE(OP_TributeItem) {
	ENCODE_LENGTH_EXACT(TributeItem_Struct);
	SETUP_DIRECT_ENCODE(TributeItem_Struct, structs::TributeItem_Struct);

	eq->slot = TitaniumToRoFSlot(emu->slot);
	OUT(quantity);
	OUT(tribute_master_id);
	OUT(tribute_points);

	FINISH_ENCODE();
}

ENCODE(OP_SomeItemPacketMaybe) {
	// This Opcode is not named very well. It is used for the animation of arrows leaving the player's bow
	// and flying to the target.
	//

	ENCODE_LENGTH_EXACT(Arrow_Struct);
	SETUP_DIRECT_ENCODE(Arrow_Struct, structs::Arrow_Struct);

	OUT(src_y);
	OUT(src_x);
	OUT(src_z);
	OUT(velocity);
	OUT(launch_angle);
	OUT(tilt);
	OUT(arc);
	OUT(source_id);
	OUT(target_id);
	OUT(item_id);

	eq->unknown070 = 175; // This needs to be set to something, else we get a 1HS animation instead of ranged.

	OUT(item_type);
	OUT(skill);

	strncpy(eq->model_name, emu->model_name, sizeof(eq->model_name));

	FINISH_ENCODE();
}

ENCODE(OP_ReadBook) {

	ENCODE_LENGTH_ATLEAST(BookText_Struct);
	SETUP_DIRECT_ENCODE(BookText_Struct, structs::BookRequest_Struct);

	if(emu->window == 0xFF)
		eq->window = 0xFFFFFFFF;
	else
		eq->window = emu->window;
	OUT(type);
	eq->invslot = 0; // Set to hard 0 since it's not required for the structure to work
	strn0cpy(eq->txtfile, emu->booktext, sizeof(eq->txtfile));
	FINISH_ENCODE();
}

ENCODE(OP_Stun) {

	ENCODE_LENGTH_EXACT(Stun_Struct);
	SETUP_DIRECT_ENCODE(Stun_Struct, structs::Stun_Struct);
	OUT(duration);
	eq->unknown005 = 163;
	eq->unknown006 = 67;

	FINISH_ENCODE();
}

ENCODE(OP_ZonePlayerToBind)
{
	ENCODE_LENGTH_ATLEAST(ZonePlayerToBind_Struct);
	ZonePlayerToBind_Struct *zps = (ZonePlayerToBind_Struct*)(*p)->pBuffer;

	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	ss.clear();

	unsigned char *buffer1 = new unsigned char[sizeof(structs::ZonePlayerToBindHeader_Struct) + strlen(zps->zone_name)];
	structs::ZonePlayerToBindHeader_Struct *zph = (structs::ZonePlayerToBindHeader_Struct*)buffer1;
	unsigned char *buffer2 = new unsigned char[sizeof(structs::ZonePlayerToBindFooter_Struct)];
	structs::ZonePlayerToBindFooter_Struct *zpf = (structs::ZonePlayerToBindFooter_Struct*)buffer2;

	zph->x = zps->x;
	zph->y = zps->y;
	zph->z = zps->z;
	zph->heading = zps->heading;
	zph->bind_zone_id = 0;
	zph->bind_instance_id = zps->bind_instance_id;
	strncpy(zph->zone_name, zps->zone_name, sizeof(zph->zone_name));

	zpf->unknown021 = 1;
	zpf->unknown022 = 0;
	zpf->unknown023 = 0;
	zpf->unknown024 = 0;

	ss.write((const char*)buffer1, (sizeof(structs::ZonePlayerToBindHeader_Struct) + strlen(zps->zone_name)));
	ss.write((const char*)buffer2, sizeof(structs::ZonePlayerToBindFooter_Struct));

	delete[] buffer1;
	delete[] buffer2;
	delete[] (*p)->pBuffer;

	(*p)->pBuffer = new unsigned char[ss.str().size()];
	(*p)->size = ss.str().size();

	memcpy((*p)->pBuffer, ss.str().c_str(), ss.str().size());
	dest->FastQueuePacket(&(*p));
}

ENCODE(OP_AdventureMerchantSell) {
	ENCODE_LENGTH_EXACT(Adventure_Sell_Struct);
	SETUP_DIRECT_ENCODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

	eq->unknown000 = 1;
	OUT(npcid);
	eq->slot = MainInvTitaniumToRoFSlot(emu->slot);
	OUT(charges);
	OUT(sell_price);

	FINISH_ENCODE();
}

ENCODE(OP_RaidUpdate)
{
	EQApplicationPacket *inapp = *p;
	*p = nullptr;
	unsigned char * __emu_buffer = inapp->pBuffer;
	RaidGeneral_Struct *raid_gen = (RaidGeneral_Struct*)__emu_buffer;

	if(raid_gen->action == 0) // raid add has longer length than other raid updates
	{
		RaidAddMember_Struct* in_add_member = (RaidAddMember_Struct*)__emu_buffer;

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidAddMember_Struct));
		structs::RaidAddMember_Struct *add_member = (structs::RaidAddMember_Struct*)outapp->pBuffer;

		add_member->raidGen.action = in_add_member->raidGen.action;
		add_member->raidGen.parameter = in_add_member->raidGen.parameter;
		strn0cpy(add_member->raidGen.leader_name, in_add_member->raidGen.leader_name, 64);
		strn0cpy(add_member->raidGen.player_name, in_add_member->raidGen.player_name, 64);
		add_member->_class = in_add_member->_class;
		add_member->level= in_add_member->level;
		add_member->isGroupLeader = in_add_member->isGroupLeader;
		add_member->flags[0] = in_add_member->flags[0];
		add_member->flags[1] = in_add_member->flags[1];
		add_member->flags[2] = in_add_member->flags[2];
		add_member->flags[3] = in_add_member->flags[3];
		add_member->flags[4] = in_add_member->flags[4];
		dest->FastQueuePacket(&outapp);
	}
	else
	{
		RaidGeneral_Struct* in_raid_general = (RaidGeneral_Struct*)__emu_buffer;

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidGeneral_Struct));
		structs::RaidGeneral_Struct *raid_general = (structs::RaidGeneral_Struct*)outapp->pBuffer;
		strn0cpy(raid_general->leader_name, in_raid_general->leader_name, 64);
		strn0cpy(raid_general->player_name, in_raid_general->player_name, 64);
		raid_general->action = in_raid_general->action;
		raid_general->parameter = in_raid_general->parameter;
		dest->FastQueuePacket(&outapp);
	}
	delete[] __emu_buffer;
}

ENCODE(OP_RaidJoin)
{
	EQApplicationPacket *inapp = *p;
	unsigned char * __emu_buffer = inapp->pBuffer;
	RaidCreate_Struct *raid_create = (RaidCreate_Struct*)__emu_buffer;

	EQApplicationPacket *outapp_create = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidGeneral_Struct));
	structs::RaidGeneral_Struct *general = (structs::RaidGeneral_Struct*)outapp_create->pBuffer;

	general->action = 8;
	general->parameter = 1;
	strn0cpy(general->leader_name, raid_create->leader_name, 64);
	strn0cpy(general->player_name, raid_create->leader_name, 64);

	dest->FastQueuePacket(&outapp_create);
	delete[] __emu_buffer;
}

ENCODE(OP_VetRewardsAvaliable)
{
	EQApplicationPacket *inapp = *p;
	unsigned char * __emu_buffer = inapp->pBuffer;

	uint32 count = ((*p)->Size() / sizeof(InternalVeteranReward));
	*p = nullptr;

	EQApplicationPacket *outapp_create = new EQApplicationPacket(OP_VetRewardsAvaliable, (sizeof(structs::VeteranReward)*count));
	uchar *old_data = __emu_buffer;
	uchar *data = outapp_create->pBuffer;
	for(unsigned int i = 0; i < count; ++i)
	{
		structs::VeteranReward *vr = (structs::VeteranReward*)data;
		InternalVeteranReward *ivr = (InternalVeteranReward*)old_data;

		vr->claim_count = ivr->claim_count;
		vr->claim_id = ivr->claim_id;
		vr->number_available = ivr->number_available;
		for(int x = 0; x < 8; ++x)
		{
			vr->items[x].item_id = ivr->items[x].item_id;
			strncpy(vr->items[x].item_name, ivr->items[x].item_name, sizeof(vr->items[x].item_name));
			vr->items[x].charges = ivr->items[x].charges;
		}

		old_data += sizeof(InternalVeteranReward);
		data += sizeof(structs::VeteranReward);
	}

	dest->FastQueuePacket(&outapp_create);
	delete inapp;
}

ENCODE(OP_WhoAllResponse)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	char *InBuffer = (char *)in->pBuffer;

	WhoAllReturnStruct *wars = (WhoAllReturnStruct*)InBuffer;

	int Count = wars->playercount;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_WhoAllResponse, in->size + (Count * 4));

	char *OutBuffer = (char *)outapp->pBuffer;

	// The struct fields were moved around a bit, so adjust values before copying
	wars->unknown44[0] = Count;
	wars->unknown52 = 0;

	memcpy(OutBuffer, InBuffer, sizeof(WhoAllReturnStruct));

	OutBuffer += sizeof(WhoAllReturnStruct);
	InBuffer += sizeof(WhoAllReturnStruct);

	for(int i = 0; i < Count; ++i)
	{
		uint32 x;

		x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);

		InBuffer += 4;
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0xffffffff);

		char Name[64];

		VARSTRUCT_DECODE_STRING(Name, InBuffer);	// Char Name
		VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

		x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);

		VARSTRUCT_DECODE_STRING(Name, InBuffer);	// Guild Name
		VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

		for(int j = 0; j < 7; ++j)
		{
			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);
		}

		VARSTRUCT_DECODE_STRING(Name, InBuffer);		// Account
		VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

		x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);
	}

	//_hex(NET__ERROR, outapp->pBuffer, outapp->size);
	dest->FastQueuePacket(&outapp);

	delete in;
}

ENCODE(OP_InspectRequest) {
	ENCODE_LENGTH_EXACT(Inspect_Struct);
	SETUP_DIRECT_ENCODE(Inspect_Struct, structs::Inspect_Struct);
	OUT(TargetID);
	OUT(PlayerID);
	FINISH_ENCODE();
}

/*ENCODE(OP_InspectAnswer) {
	ENCODE_LENGTH_EXACT(InspectResponse_Struct);
	SETUP_DIRECT_ENCODE(InspectResponse_Struct, structs::InspectResponse_Struct);

	OUT(TargetID);
	OUT(playerid);

	int r;
	for (r = 0; r < 21; r++) {
		strn0cpy(eq->itemnames[r], emu->itemnames[r], sizeof(eq->itemnames[r]));
	}
	// Swap last 2 slots for Arrow and Power Source
	strn0cpy(eq->itemnames[21], emu->itemnames[22], sizeof(eq->itemnames[21]));
	strn0cpy(eq->unknown_zero, emu->itemnames[21], sizeof(eq->unknown_zero));

	int k;
	for (k = 0; k < 21; k++) {
		OUT(itemicons[k]);
	}
	// Swap last 2 slots for Arrow and Power Source
	eq->itemicons[21] = emu->itemicons[22];
	eq->unknown_zero2 = emu->itemicons[21];
	strn0cpy(eq->text, emu->text, sizeof(eq->text));

	FINISH_ENCODE();
}*/

ENCODE(OP_GroupInvite) {
	ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
	SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupInvite_Struct);
	memcpy(eq->invitee_name, emu->name1, sizeof(eq->invitee_name));
	memcpy(eq->inviter_name, emu->name2, sizeof(eq->inviter_name));
	FINISH_ENCODE();
}

ENCODE(OP_GroupFollow) {
	ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
	SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupFollow_Struct);
	memcpy(eq->name1, emu->name1, sizeof(eq->name1));
	memcpy(eq->name2, emu->name2, sizeof(eq->name2));
	FINISH_ENCODE();
}

ENCODE(OP_GroupFollow2) {
	ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
	SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupFollow_Struct);
	memcpy(eq->name1, emu->name1, sizeof(eq->name1));
	memcpy(eq->name2, emu->name2, sizeof(eq->name2));
	FINISH_ENCODE();
}

ENCODE(OP_GroupCancelInvite)
{
	ENCODE_LENGTH_EXACT(GroupCancel_Struct);
	SETUP_DIRECT_ENCODE(GroupCancel_Struct, structs::GroupCancel_Struct);
	memcpy(eq->name1, emu->name1, sizeof(eq->name1));
	memcpy(eq->name2, emu->name2, sizeof(eq->name2));
	OUT(toggle);
	FINISH_ENCODE();
}

ENCODE(OP_SetGuildRank)
{
	ENCODE_LENGTH_EXACT(GuildSetRank_Struct);
	SETUP_DIRECT_ENCODE(GuildSetRank_Struct, structs::GuildSetRank_Struct);
	eq->GuildID = emu->Unknown00;
	OUT(Rank);
	memcpy(eq->MemberName, emu->MemberName, sizeof(eq->MemberName));
	OUT(Banker);
	eq->Unknown76 = 1;
	FINISH_ENCODE();
}


ENCODE(OP_GroupUpdate)
{
	//_log(NET__ERROR, "OP_GroupUpdate");
	EQApplicationPacket *in = *p;

	GroupJoin_Struct *gjs = (GroupJoin_Struct*)in->pBuffer;

	//_log(NET__ERROR, "Received outgoing OP_GroupUpdate with action code %i", gjs->action);
	if((gjs->action == groupActLeave) || (gjs->action == groupActDisband))
	{
		if((gjs->action == groupActDisband) || !strcmp(gjs->yourname, gjs->membername))
		{
			//_log(NET__ERROR, "Group Leave, yourname = %s, membername = %s", gjs->yourname, gjs->membername);

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupDisbandYou, sizeof(structs::GroupGeneric_Struct));

			structs::GroupGeneric_Struct *ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
			memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
			memcpy(ggs->name2, gjs->membername, sizeof(ggs->name1));
			dest->FastQueuePacket(&outapp);

			// Make an empty GLAA packet to clear out their useable GLAAs
			//
			outapp = new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));

			dest->FastQueuePacket(&outapp);

			delete in;

			return;
		}
		//if(gjs->action == groupActLeave)
		//	_log(NET__ERROR, "Group Leave, yourname = %s, membername = %s", gjs->yourname, gjs->membername);

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupDisbandOther, sizeof(structs::GroupGeneric_Struct));

		structs::GroupGeneric_Struct *ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
		memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
		memcpy(ggs->name2, gjs->membername, sizeof(ggs->name2));
		//_hex(NET__ERROR, outapp->pBuffer, outapp->size);
		dest->FastQueuePacket(&outapp);

		delete in;
		return;


	}

	if(in->size == sizeof(GroupUpdate2_Struct))
	{
		// Group Update2
		//_log(NET__ERROR, "Struct is GroupUpdate2");

		unsigned char *__emu_buffer = in->pBuffer;
		GroupUpdate2_Struct *gu2 = (GroupUpdate2_Struct*) __emu_buffer;

		//_log(NET__ERROR, "Yourname is %s", gu2->yourname);

		int MemberCount = 1;

		int PacketLength = 8 + strlen(gu2->leadersname) + 1 + 22 + strlen(gu2->yourname) + 1;

		for(int i = 0; i < 5; ++i)
		{
			//_log(NET__ERROR, "Membername[%i] is %s", i,  gu2->membername[i]);
			if(gu2->membername[i][0] != '\0')
			{
				PacketLength += (22 + strlen(gu2->membername[i]) + 1);
				++MemberCount;
			}
		}

		//_log(NET__ERROR, "Leadername is %s", gu2->leadersname);

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupUpdateB, PacketLength);

		char *Buffer = (char *)outapp->pBuffer;

		// Header
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Think this should be SpawnID, but it doesn't seem to matter
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, MemberCount);
		VARSTRUCT_ENCODE_STRING(Buffer, gu2->leadersname);

		// Leader
		//

		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
		VARSTRUCT_ENCODE_STRING(Buffer, gu2->yourname);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
		//VARSTRUCT_ENCODE_STRING(Buffer, "");
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x46);	// Observed 0x41 and 0x46 here
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
		VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);

		int MemberNumber = 1;

		for(int i = 0; i < 5; ++i)
		{
			if(gu2->membername[i][0] == '\0')
				continue;

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, MemberNumber++);
			VARSTRUCT_ENCODE_STRING(Buffer, gu2->membername[i]);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			//VARSTRUCT_ENCODE_STRING(Buffer, "");
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x41);	// Observed 0x41 and 0x46 here
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Low byte is Main Assist Flag
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);
		}

		//_hex(NET__ERROR, outapp->pBuffer, outapp->size);
		dest->FastQueuePacket(&outapp);

		outapp = new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));

		GroupLeadershipAAUpdate_Struct *GLAAus = (GroupLeadershipAAUpdate_Struct*)outapp->pBuffer;

		GLAAus->NPCMarkerID = gu2->NPCMarkerID;
		memcpy(&GLAAus->LeaderAAs, &gu2->leader_aas, sizeof(GLAAus->LeaderAAs));

		dest->FastQueuePacket(&outapp);
		delete in;

		return;

	}
	//_log(NET__ERROR, "Generic GroupUpdate, yourname = %s, membername = %s", gjs->yourname, gjs->membername);
	ENCODE_LENGTH_EXACT(GroupJoin_Struct);
	SETUP_DIRECT_ENCODE(GroupJoin_Struct, structs::GroupJoin_Struct);

	memcpy(eq->membername, emu->membername, sizeof(eq->membername));

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));

	GroupLeadershipAAUpdate_Struct *GLAAus = (GroupLeadershipAAUpdate_Struct*)outapp->pBuffer;

	GLAAus->NPCMarkerID = emu->NPCMarkerID;

	memcpy(&GLAAus->LeaderAAs, &emu->leader_aas, sizeof(GLAAus->LeaderAAs));
	//_hex(NET__ERROR, __packet->pBuffer, __packet->size);
	FINISH_ENCODE();

	dest->FastQueuePacket(&outapp);
}

ENCODE(OP_ChannelMessage)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	ChannelMessage_Struct *emu = (ChannelMessage_Struct *) in->pBuffer;

	unsigned char *__emu_buffer = in->pBuffer;

	in->size = strlen(emu->sender) + 1 + strlen(emu->targetname) + 1 + strlen(emu->message) + 1 + 36;

	in->pBuffer = new unsigned char[in->size];

	char *OutBuffer = (char *)in->pBuffer;

	VARSTRUCT_ENCODE_STRING(OutBuffer, emu->sender);
	VARSTRUCT_ENCODE_STRING(OutBuffer, emu->targetname);
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->language);
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->chan_num);
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->skill_in_language);
	VARSTRUCT_ENCODE_STRING(OutBuffer, emu->message);

	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_GuildsList)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	uint32 NumberOfGuilds = in->size / 64;

	uint32 PacketSize = 68;	// 64 x 0x00 + a uint32 that I am guessing is the highest guild ID in use.

	unsigned char *__emu_buffer = in->pBuffer;

	char *InBuffer = (char *)__emu_buffer;

	uint32 HighestGuildID = 0;

	for(unsigned int i = 0; i < NumberOfGuilds; ++i)
	{
		if(InBuffer[0])
		{
			PacketSize += (5 + strlen(InBuffer));
			HighestGuildID = i - 1;
		}
		InBuffer += 64;
	}

	PacketSize++;	// Appears to be an extra 0x00 at the very end.

	in->size = PacketSize;

	in->pBuffer = new unsigned char[in->size];

	InBuffer = (char *)__emu_buffer;

	char *OutBuffer = (char *)in->pBuffer;

	// Init the first 64 bytes to zero, as per live.
	//
	memset(OutBuffer, 0, 64);

	OutBuffer += 64;

	VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, HighestGuildID);

	for(unsigned int i = 0; i < NumberOfGuilds; ++i)
	{
		if(InBuffer[0])
		{
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, i - 1);
			VARSTRUCT_ENCODE_STRING(OutBuffer, InBuffer);
		}
		InBuffer += 64;
	}

	VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0x00);

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_DzExpeditionEndsWarning)
{
	ENCODE_LENGTH_EXACT(ExpeditionExpireWarning);
	SETUP_DIRECT_ENCODE(ExpeditionExpireWarning, structs::ExpeditionExpireWarning);
	OUT(minutes_remaining);
	FINISH_ENCODE();
}

ENCODE(OP_DzExpeditionInfo)
{
	ENCODE_LENGTH_EXACT(ExpeditionInfo_Struct);
	SETUP_DIRECT_ENCODE(ExpeditionInfo_Struct, structs::ExpeditionInfo_Struct);
	OUT(max_players);
	eq->unknown004 = 785316192;
	eq->unknown008 = 435601;
	strncpy(eq->expedition_name, emu->expedition_name, sizeof(eq->expedition_name));
	strncpy(eq->leader_name, emu->leader_name, sizeof(eq->leader_name));
	FINISH_ENCODE();
}

ENCODE(OP_DzCompass)
{
	SETUP_VAR_ENCODE(ExpeditionCompass_Struct);
	ALLOC_VAR_ENCODE(structs::ExpeditionCompass_Struct, sizeof(structs::ExpeditionInfo_Struct) + sizeof(structs::ExpeditionCompassEntry_Struct) * emu->count);
	OUT(count);

	for(uint32 i = 0; i < emu->count; ++i)
	{
		OUT(entries[i].x);
		OUT(entries[i].y);
		OUT(entries[i].z);
	}

	FINISH_ENCODE();
}

ENCODE(OP_DzMemberList)
{
	SETUP_VAR_ENCODE(ExpeditionMemberList_Struct);
	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

	uint32 client_id = 0;
	uint8 null_term = 0;
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&emu->count, sizeof(uint32));
	for(uint32 i = 0; i < emu->count; ++i)
	{
		ss.write(emu->entries[i].name, strlen(emu->entries[i].name));
		ss.write((const char*)&null_term, sizeof(char));
		ss.write((const char*)&emu->entries[i].status, sizeof(char));
	}

	__packet->size = ss.str().length();
	__packet->pBuffer = new unsigned char[__packet->size];
	memcpy(__packet->pBuffer, ss.str().c_str(), __packet->size);

	FINISH_ENCODE();
}

ENCODE(OP_DzExpeditionList)
{
	SETUP_VAR_ENCODE(ExpeditionLockoutList_Struct);
	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	uint32 client_id = 0;
	uint8 null_term = 0;
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&emu->count, sizeof(uint32));
	for(uint32 i = 0; i < emu->count; ++i)
	{
		ss.write(emu->entries[i].expedition, strlen(emu->entries[i].expedition));
		ss.write((const char*)&null_term, sizeof(char));
		ss.write((const char*)&emu->entries[i].time_left, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write(emu->entries[i].expedition_event, strlen(emu->entries[i].expedition_event));
		ss.write((const char*)&null_term, sizeof(char));
	}

	__packet->size = ss.str().length();
	__packet->pBuffer = new unsigned char[__packet->size];
	memcpy(__packet->pBuffer, ss.str().c_str(), __packet->size);

	FINISH_ENCODE();
}

ENCODE(OP_DzLeaderStatus)
{
	SETUP_VAR_ENCODE(ExpeditionLeaderSet_Struct);
	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	uint32 client_id = 0;
	uint8 null_term = 0;

	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write(emu->leader_name, strlen(emu->leader_name));
	ss.write((const char*)&null_term, sizeof(char));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));//0xffffffff
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));
	ss.write((const char*)&client_id, sizeof(uint32));//1
	ss.write((const char*)&client_id, sizeof(uint32));

	__packet->size = ss.str().length();
	__packet->pBuffer = new unsigned char[__packet->size];
	memcpy(__packet->pBuffer, ss.str().c_str(), __packet->size);

	FINISH_ENCODE();
}

ENCODE(OP_DzJoinExpeditionConfirm)
{
	ENCODE_LENGTH_EXACT(ExpeditionJoinPrompt_Struct);
	SETUP_DIRECT_ENCODE(ExpeditionJoinPrompt_Struct, structs::ExpeditionJoinPrompt_Struct);
	strncpy(eq->expedition_name, emu->expedition_name, sizeof(eq->expedition_name));
	strncpy(eq->player_name, emu->player_name, sizeof(eq->player_name));
	FINISH_ENCODE();
}

ENCODE(OP_TargetBuffs) {  ENCODE_FORWARD(OP_BuffCreate); }
ENCODE(OP_BuffCreate)
{
	SETUP_VAR_ENCODE(BuffIcon_Struct);

	uint32 sz = 12 + (17 * emu->count);
	__packet->size = sz;
	__packet->pBuffer = new unsigned char[sz];
	memset(__packet->pBuffer, 0, sz);

	__packet->WriteUInt32(emu->entity_id);
	__packet->WriteUInt32(0);		// PlayerID ?
	__packet->WriteUInt8(1);			// 1 indicates all buffs on the player (0 to add or remove a single buff)
	__packet->WriteUInt16(emu->count);

	for(uint16 i = 0; i < emu->count; ++i)
	{
		uint16 buffslot = emu->entries[i].buff_slot;
		// Not sure if this is needs amending for RoF yet.
		if(emu->entries[i].buff_slot >= 25)
		{
			buffslot += 17;
		}

		__packet->WriteUInt32(buffslot);
		__packet->WriteUInt32(emu->entries[i].spell_id);
		__packet->WriteUInt32(emu->entries[i].tics_remaining);
		__packet->WriteUInt32(0); // Unknown
		__packet->WriteString("");
	}
	__packet->WriteUInt8(0); // Unknown

	FINISH_ENCODE();
}

ENCODE(OP_ZoneChange)
{
	ENCODE_LENGTH_EXACT(ZoneChange_Struct);
	SETUP_DIRECT_ENCODE(ZoneChange_Struct, structs::ZoneChange_Struct);

	memcpy(eq->char_name, emu->char_name, sizeof(emu->char_name));
	OUT(zoneID);
	OUT(instanceID);
	OUT(y);
	OUT(x);
	OUT(z)
	OUT(zone_reason);
	OUT(success);
	FINISH_ENCODE();
}

ENCODE(OP_WearChange)
{
	ENCODE_LENGTH_EXACT(WearChange_Struct);
	SETUP_DIRECT_ENCODE(WearChange_Struct, structs::WearChange_Struct);
	OUT(spawn_id);
	OUT(material);
	OUT(unknown06);
	OUT(elite_material);
	OUT(hero_forge_model);
	OUT(unknown18);
	OUT(color.color);
	OUT(wear_slot_id);
	FINISH_ENCODE();
}

ENCODE(OP_SpawnAppearance)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *emu_buffer = in->pBuffer;

	SpawnAppearance_Struct *sas = (SpawnAppearance_Struct *)emu_buffer;

	if(sas->type != AT_Size)
	{
		dest->FastQueuePacket(&in, ack_req);
		return;
	}

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ChangeSize, sizeof(ChangeSize_Struct));

	ChangeSize_Struct *css = (ChangeSize_Struct *)outapp->pBuffer;

	css->EntityID = sas->spawn_id;
	css->Size = (float)sas->parameter;
	css->Unknown08 = 0;
	css->Unknown12 = 1.0f;

	dest->FastQueuePacket(&outapp, ack_req);

	delete in;
}

ENCODE(OP_CastSpell)
{
	ENCODE_LENGTH_EXACT(CastSpell_Struct);
	SETUP_DIRECT_ENCODE(CastSpell_Struct, structs::CastSpell_Struct);
	if(emu->slot == 10)
	{
		eq->slot = 13;
	}
	else
	{
		OUT(slot);
	}
	OUT(spell_id);
	eq->inventoryslot = TitaniumToRoFSlot(emu->inventoryslot);
	//OUT(inventoryslot);
	OUT(target_id);
	FINISH_ENCODE();
}

ENCODE(OP_ShopRequest)
{
	ENCODE_LENGTH_EXACT(Merchant_Click_Struct);
	SETUP_DIRECT_ENCODE(Merchant_Click_Struct, structs::Merchant_Click_Struct);
	OUT(npcid);
	OUT(playerid);
	OUT(command);
	OUT(rate);
	eq->unknown01 = 3;	// Not sure what these values do yet, but list won't display without them
	eq->unknown02 = 2592000;
	FINISH_ENCODE();
}

ENCODE(OP_DisciplineUpdate)
{
	ENCODE_LENGTH_EXACT(Disciplines_Struct);
	SETUP_DIRECT_ENCODE(Disciplines_Struct, structs::Disciplines_Struct);

	memcpy(&eq->values, &emu->values, sizeof(Disciplines_Struct));

	FINISH_ENCODE();
}

ENCODE(OP_RespondAA) {
	SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

	eq->aa_spent = emu->aa_spent;
	// These fields may need to be correctly populated at some point
	eq->aapoints_assigned = emu->aa_spent + 1;
	eq->aa_spent_general = 0;
	eq->aa_spent_archetype = 0;
	eq->aa_spent_class = 0;
	eq->aa_spent_special = 0;

	for(uint32 i = 0; i < MAX_PP_AA_ARRAY; ++i)
	{
		eq->aa_list[i].aa_skill = emu->aa_list[i].aa_skill;
		eq->aa_list[i].aa_value = emu->aa_list[i].aa_value;
		eq->aa_list[i].unknown08 = emu->aa_list[i].unknown08;
	}

	FINISH_ENCODE();
}

ENCODE(OP_AltCurrencySell)
{
    ENCODE_LENGTH_EXACT(AltCurrencySellItem_Struct);
	SETUP_DIRECT_ENCODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

    OUT(merchant_entity_id);
    eq->slot_id = TitaniumToRoFSlot(emu->slot_id);
    OUT(charges);
    OUT(cost);
    FINISH_ENCODE();
}

ENCODE(OP_AltCurrency)
{
    EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *emu_buffer = in->pBuffer;
    uint32 opcode = *((uint32*)emu_buffer);

    if(opcode == 8) {
        AltCurrencyPopulate_Struct *populate = (AltCurrencyPopulate_Struct*)emu_buffer;

        EQApplicationPacket *outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(structs::AltCurrencyPopulate_Struct)
            + sizeof(structs::AltCurrencyPopulateEntry_Struct) * populate->count);
        structs::AltCurrencyPopulate_Struct *out_populate = (structs::AltCurrencyPopulate_Struct*)outapp->pBuffer;

        out_populate->opcode = populate->opcode;
        out_populate->count = populate->count;
        for(uint32 i = 0; i < populate->count; ++i) {
            out_populate->entries[i].currency_number = populate->entries[i].currency_number;
			out_populate->entries[i].unknown00 = populate->entries[i].unknown00;
            out_populate->entries[i].currency_number2 = populate->entries[i].currency_number2;
            out_populate->entries[i].item_id = populate->entries[i].item_id;
            out_populate->entries[i].item_icon = populate->entries[i].item_icon;
            out_populate->entries[i].stack_size = populate->entries[i].stack_size;
			out_populate->entries[i].display = ((populate->entries[i].stack_size > 0) ? 1 : 0);
        }

        dest->FastQueuePacket(&outapp, ack_req);
    } else {
        EQApplicationPacket *outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(AltCurrencyUpdate_Struct));
        memcpy(outapp->pBuffer, emu_buffer, sizeof(AltCurrencyUpdate_Struct));
        dest->FastQueuePacket(&outapp, ack_req);
    }

    //dest->FastQueuePacket(&outapp, ack_req);
    delete in;
}

ENCODE(OP_HPUpdate)
{
	SETUP_DIRECT_ENCODE(SpawnHPUpdate_Struct, structs::SpawnHPUpdate_Struct);
	OUT(spawn_id);
	OUT(cur_hp);
	OUT(max_hp);
	FINISH_ENCODE();
}

ENCODE(OP_RemoveBlockedBuffs) { ENCODE_FORWARD(OP_BlockedBuffs); }

ENCODE(OP_BlockedBuffs)
{
	ENCODE_LENGTH_EXACT(BlockedBuffs_Struct);
	SETUP_DIRECT_ENCODE(BlockedBuffs_Struct, structs::BlockedBuffs_Struct);

	for(uint32 i = 0; i < BLOCKED_BUFF_COUNT; ++i)
		eq->SpellID[i] = emu->SpellID[i];

	// -1 for the extra 10 added in RoF. We should really be encoding for the older clients, not RoF, but
	// we can sort that out later.

	for(uint32 i = BLOCKED_BUFF_COUNT; i < structs::BLOCKED_BUFF_COUNT; ++i)
		eq->SpellID[i] = -1;

	OUT(Count);
	OUT(Pet);
	OUT(Initialise);
	OUT(Flags);

	FINISH_ENCODE();
}

ENCODE(OP_TributeInfo)
{
	ENCODE_LENGTH_ATLEAST(TributeAbility_Struct);
	SETUP_VAR_ENCODE(TributeAbility_Struct);

	ALLOC_VAR_ENCODE(structs::TributeAbility_Struct, sizeof(structs::TributeAbility_Struct) + strlen(emu->name) + 1);

	OUT(tribute_id);
	OUT(tier_count);

	for(uint32 i = 0; i < MAX_TRIBUTE_TIERS; ++i)
	{
		eq->tiers[i].level = emu->tiers[i].level;
		eq->tiers[i].tribute_item_id = emu->tiers[i].tribute_item_id;
		eq->tiers[i].cost = emu->tiers[i].cost;
	}

	eq->unknown128 = 0;

	strcpy(eq->name, emu->name);

	FINISH_ENCODE();
}

ENCODE(OP_GuildMemberUpdate)
{
	SETUP_DIRECT_ENCODE(GuildMemberUpdate_Struct, structs::GuildMemberUpdate_Struct);

	OUT(GuildID);
	memcpy(eq->MemberName, emu->MemberName, sizeof(eq->MemberName));
	OUT(ZoneID);
	OUT(InstanceID);
	OUT(LastSeen);
	eq->Unknown76 = 0;
	FINISH_ENCODE();
}

ENCODE(OP_BeginCast)
{
	SETUP_DIRECT_ENCODE(BeginCast_Struct, structs::BeginCast_Struct);

	OUT(spell_id);
	OUT(caster_id);
	OUT(cast_time);

	FINISH_ENCODE();
}

ENCODE(OP_RezzRequest)
{
	SETUP_DIRECT_ENCODE(Resurrect_Struct, structs::Resurrect_Struct);

	OUT(zone_id);
	OUT(instance_id);
	OUT(y);
	OUT(x);
	OUT(z);
	OUT_str(your_name);
	OUT_str(rezzer_name);
	OUT(spellid);
	OUT_str(corpse_name);
	OUT(action);

	FINISH_ENCODE();
}

DECODE(OP_BuffRemoveRequest)
{
	// This is to cater for the fact that short buff box buffs start at 30 as opposed to 25 in prior clients.
	//
	DECODE_LENGTH_EXACT(structs::BuffRemoveRequest_Struct);
	SETUP_DIRECT_DECODE(BuffRemoveRequest_Struct, structs::BuffRemoveRequest_Struct);

	emu->SlotID = (eq->SlotID < 42 ) ? eq->SlotID : (eq->SlotID - 17);

	IN(EntityID);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_PetCommands)
{
	DECODE_LENGTH_EXACT(structs::PetCommand_Struct);
	SETUP_DIRECT_DECODE(PetCommand_Struct, structs::PetCommand_Struct);

	switch(eq->command)
	{
		case 0x00:
			emu->command = 0x04;	// Health
			break;
		case 0x01:
			emu->command = 0x10;	// Leader
			break;
		case 0x02:
			emu->command = 0x07;	// Attack
			break;
		case 0x04:
			emu->command = 0x08;	// Follow
			break;
		case 0x05:
			emu->command = 0x05;	// Guard
			break;
		case 0x06:
			emu->command = 0x09;	// Sit. Needs work. This appears to be a toggle between Sit/Stand now.
			break;
		case 0x0c:
			emu->command = 0x0b;	// Taunt
			break;
		case 0x0f:
			emu->command = 0x0c;	// Hold
			break;
		case 0x1c:
			emu->command = 0x01;	// Back
			break;
		case 0x1d:
			emu->command = 0x02;	// Leave/Go Away
			break;
		default:
			emu->command = eq->command;
	}

	FINISH_DIRECT_DECODE();
}

DECODE(OP_AltCurrencySellSelection)
{
    DECODE_LENGTH_EXACT(structs::AltCurrencySelectItem_Struct);
	SETUP_DIRECT_DECODE(AltCurrencySelectItem_Struct, structs::AltCurrencySelectItem_Struct);
    IN(merchant_entity_id);
    emu->slot_id = RoFToTitaniumSlot(eq->slot_id);
    FINISH_DIRECT_DECODE();
}

DECODE(OP_AltCurrencySell)
{
    DECODE_LENGTH_EXACT(structs::AltCurrencySellItem_Struct);
	SETUP_DIRECT_DECODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);
    IN(merchant_entity_id);
    emu->slot_id = RoFToTitaniumSlot(eq->slot_id);
    IN(charges);
    IN(cost);
    FINISH_DIRECT_DECODE();
}

DECODE(OP_ShopRequest) {
	DECODE_LENGTH_EXACT(structs::Merchant_Click_Struct);
	SETUP_DIRECT_DECODE(Merchant_Click_Struct, structs::Merchant_Click_Struct);
	IN(npcid);
	IN(playerid);
	IN(command);
	IN(rate);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GuildRemove)
{
    DECODE_LENGTH_EXACT(structs::GuildCommand_Struct);
	SETUP_DIRECT_DECODE(GuildCommand_Struct, structs::GuildCommand_Struct);
    strn0cpy(emu->othername, eq->othername, sizeof(emu->othername));
	strn0cpy(emu->myname, eq->myname, sizeof(emu->myname));
	IN(guildeqid);
	IN(officer);
    FINISH_DIRECT_DECODE();
}

DECODE(OP_GuildDemote)
{
    DECODE_LENGTH_EXACT(structs::GuildDemoteStruct);
	SETUP_DIRECT_DECODE(GuildDemoteStruct, structs::GuildDemoteStruct);
    strn0cpy(emu->target, eq->target, sizeof(emu->target));
	strn0cpy(emu->name, eq->name, sizeof(emu->name));
	// IN(rank);
    FINISH_DIRECT_DECODE();
}

DECODE(OP_BazaarSearch)
{
	char *Buffer = (char *)__packet->pBuffer;

	uint8 SubAction = VARSTRUCT_DECODE_TYPE(uint8, Buffer);

	if((SubAction != BazaarInspectItem) || (__packet->size != sizeof(structs::NewBazaarInspect_Struct)))
		return;

	SETUP_DIRECT_DECODE(NewBazaarInspect_Struct, structs::NewBazaarInspect_Struct);
	MEMSET_IN(structs::NewBazaarInspect_Struct);
	IN(Beginning.Action);
	memcpy(emu->Name, eq->Name, sizeof(emu->Name));
	IN(SerialNumber);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_InspectRequest) {
	DECODE_LENGTH_EXACT(structs::Inspect_Struct);
	SETUP_DIRECT_DECODE(Inspect_Struct, structs::Inspect_Struct);
	IN(TargetID);
	IN(PlayerID);
	FINISH_DIRECT_DECODE();
}

/*DECODE(OP_InspectAnswer) {
	DECODE_LENGTH_EXACT(structs::InspectResponse_Struct);
	SETUP_DIRECT_DECODE(InspectResponse_Struct, structs::InspectResponse_Struct);

	IN(TargetID);
	IN(playerid);

	int r;
	for (r = 0; r < 21; r++) {
		strn0cpy(emu->itemnames[r], eq->itemnames[r], sizeof(emu->itemnames[r]));
	}
	// Swap last 2 slots for Arrow and Power Source
	strn0cpy(emu->itemnames[22], eq->itemnames[21], sizeof(emu->itemnames[22]));
	strn0cpy(emu->itemnames[21], eq->unknown_zero, sizeof(emu->itemnames[21]));
	strn0cpy(emu->unknown_zero, eq->unknown_zero, sizeof(emu->unknown_zero));

	int k;
	for (k = 0; k < 21; k++) {
		IN(itemicons[k]);
	}
	// Swap last 2 slots for Arrow and Power Source
	emu->itemicons[22] = eq->itemicons[21];
	emu->itemicons[21] = eq->unknown_zero2;
	emu->unknown_zero2 = eq->unknown_zero2;
	strn0cpy(emu->text, eq->text, sizeof(emu->text));
	//emu->unknown1772 = 0;

	FINISH_DIRECT_DECODE();
}*/

DECODE(OP_RaidInvite) {
	DECODE_LENGTH_EXACT(structs::RaidGeneral_Struct);
	SETUP_DIRECT_DECODE(RaidGeneral_Struct, structs::RaidGeneral_Struct);

	strn0cpy(emu->leader_name, eq->leader_name, 64);
	strn0cpy(emu->player_name, eq->player_name, 64);
	IN(action);
	IN(parameter);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_AdventureMerchantSell) {
	DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
	SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

	IN(npcid);
	emu->slot = MainInvRoFToTitaniumSlot(eq->slot);
	IN(charges);
	IN(sell_price);

	FINISH_DIRECT_DECODE();
}


DECODE(OP_ApplyPoison) {
	DECODE_LENGTH_EXACT(structs::ApplyPoison_Struct);
	SETUP_DIRECT_DECODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

	emu->inventorySlot = MainInvRoFToTitaniumSlot(eq->inventorySlot);
	IN(success);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_ItemVerifyRequest) {
	DECODE_LENGTH_EXACT(structs::ItemVerifyRequest_Struct);
	SETUP_DIRECT_DECODE(ItemVerifyRequest_Struct, structs::ItemVerifyRequest_Struct);

	emu->slot = RoFToTitaniumSlot(eq->slot);
	IN(target);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_Consume) {
	DECODE_LENGTH_EXACT(structs::Consume_Struct);
	SETUP_DIRECT_DECODE(Consume_Struct, structs::Consume_Struct);

	emu->slot = RoFToTitaniumSlot(eq->slot);
	IN(auto_consumed);
	IN(type);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_CastSpell) {
	DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
	SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);

	if(eq->slot == 13)
	{
		emu->slot = 10;
	}
	else
	{
		IN(slot);
	}
	IN(spell_id);
	emu->inventoryslot = RoFToTitaniumSlot(eq->inventoryslot);
	//IN(inventoryslot);
	IN(target_id);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_DeleteItem)
{
	DECODE_LENGTH_EXACT(structs::DeleteItem_Struct);
	SETUP_DIRECT_DECODE(DeleteItem_Struct, structs::DeleteItem_Struct);

	emu->from_slot = RoFToTitaniumSlot(eq->from_slot);
	emu->to_slot = RoFToTitaniumSlot(eq->to_slot);
	IN(number_in_stack);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_MoveItem)
{
	DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
	SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

	//_log(NET__ERROR, "Moved item from %u to %u", eq->from_slot.MainSlot, eq->to_slot.MainSlot);
	_log(NET__ERROR, "MoveItem SlotType from %i to %i, MainSlot from %i to %i, SubSlot from %i to %i, AugSlot from %i to %i, Unknown01 from %i to %i, Number %u", eq->from_slot.SlotType, eq->to_slot.SlotType, eq->from_slot.MainSlot, eq->to_slot.MainSlot, eq->from_slot.SubSlot, eq->to_slot.SubSlot, eq->from_slot.AugSlot, eq->to_slot.AugSlot, eq->from_slot.Unknown01, eq->to_slot.Unknown01, eq->number_in_stack);
	emu->from_slot = RoFToTitaniumSlot(eq->from_slot);
	emu->to_slot = RoFToTitaniumSlot(eq->to_slot);
	IN(number_in_stack);

	_hex(NET__ERROR, eq, sizeof(structs::MoveItem_Struct));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ItemLinkClick) {
	DECODE_LENGTH_EXACT(structs::ItemViewRequest_Struct);
	SETUP_DIRECT_DECODE(ItemViewRequest_Struct, structs::ItemViewRequest_Struct);
	MEMSET_IN(ItemViewRequest_Struct);

	IN(item_id);
	int r;
	for (r = 0; r < 5; r++) {
		IN(augments[r]);
	}
	// Max Augs is now 6, but no code to support that many yet
	IN(link_hash);
	IN(icon);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_SetServerFilter) {
	DECODE_LENGTH_EXACT(structs::SetServerFilter_Struct);
	SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);
	int r;
	for(r = 0; r < 29; r++) {
		// Size 40 in RoF
		IN(filters[r]);
	}
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ConsiderCorpse) { DECODE_FORWARD(OP_Consider); }
DECODE(OP_Consider) {
	DECODE_LENGTH_EXACT(structs::Consider_Struct);
	SETUP_DIRECT_DECODE(Consider_Struct, structs::Consider_Struct);
	IN(playerid);
	IN(targetid);
	IN(faction);
	IN(level);
	//emu->cur_hp = 1;
	//emu->max_hp = 2;
	//emu->pvpcon = 0;
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ShopPlayerBuy)
{
	DECODE_LENGTH_EXACT(structs::Merchant_Sell_Struct);
	SETUP_DIRECT_DECODE(Merchant_Sell_Struct, structs::Merchant_Sell_Struct);

	IN(npcid);
	IN(playerid);
	IN(itemslot);
	IN(quantity);
	IN(price);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_ClientUpdate) {
    // for some odd reason, there is an extra byte on the end of this on occasion..
	DECODE_LENGTH_ATLEAST(structs::PlayerPositionUpdateClient_Struct);
	SETUP_DIRECT_DECODE(PlayerPositionUpdateClient_Struct, structs::PlayerPositionUpdateClient_Struct);
	IN(spawn_id);
	IN(sequence);
	IN(x_pos);
	IN(y_pos);
	IN(z_pos);
	IN(heading);
	IN(delta_x);
	IN(delta_y);
	IN(delta_z);
	IN(delta_heading);
	IN(animation);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_CharacterCreate) {
	DECODE_LENGTH_EXACT(structs::CharCreate_Struct);
	SETUP_DIRECT_DECODE(CharCreate_Struct, structs::CharCreate_Struct);

	IN(gender);
	IN(race);
	IN(class_);
	IN(deity);

	if(RuleB(World, EnableTutorialButton) && eq->tutorial)
		emu->start_zone = RuleI(World, TutorialZoneID);
	else
		emu->start_zone = eq->start_zone;

	IN(haircolor);
	IN(beard);
	IN(beardcolor);
	IN(hairstyle);
	IN(face);
	IN(eyecolor1);
	IN(eyecolor2);
	IN(drakkin_heritage);
	IN(drakkin_tattoo);
	IN(drakkin_details);
	IN(STR);
	IN(STA);
	IN(AGI);
	IN(DEX);
	IN(WIS);
	IN(INT);
	IN(CHA);
	//IN(tutorial);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_WhoAllRequest) {
	DECODE_LENGTH_EXACT(structs::Who_All_Struct);
	SETUP_DIRECT_DECODE(Who_All_Struct, structs::Who_All_Struct);

	memcpy(emu->whom, eq->whom, sizeof(emu->whom));
	IN(wrace);
	IN(wclass);
	IN(lvllow);
	IN(lvlhigh);
	IN(gmlookup);
	IN(guildid);
	IN(type);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_GroupInvite2)
{
	//_log(NET__ERROR, "Received incoming OP_GroupInvite2. Forwarding");
	DECODE_FORWARD(OP_GroupInvite);
}

DECODE(OP_GroupInvite) {
	//EQApplicationPacket *in = __packet;
	//_log(NET__ERROR, "Received incoming OP_GroupInvite");
	//_hex(NET__ERROR, in->pBuffer, in->size);
	DECODE_LENGTH_EXACT(structs::GroupInvite_Struct);
	SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupInvite_Struct);
	memcpy(emu->name1, eq->invitee_name, sizeof(emu->name1));
	memcpy(emu->name2, eq->inviter_name, sizeof(emu->name2));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GroupFollow) {
	//EQApplicationPacket *in = __packet;
	//_log(NET__ERROR, "Received incoming OP_GroupFollow");
	//_hex(NET__ERROR, in->pBuffer, in->size);
	DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
	SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);
	memcpy(emu->name1, eq->name1, sizeof(emu->name1));
	memcpy(emu->name2, eq->name2, sizeof(emu->name2));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GroupFollow2) {
	//EQApplicationPacket *in = __packet;
	//_log(NET__ERROR, "Received incoming OP_GroupFollow2");
	//_hex(NET__ERROR, in->pBuffer, in->size);
	DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
	SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);
	memcpy(emu->name1, eq->name1, sizeof(emu->name1));
	memcpy(emu->name2, eq->name2, sizeof(emu->name2));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GroupDisband) {
	//EQApplicationPacket *in = __packet;
	//_log(NET__ERROR, "Received incoming OP_Disband");
	//_hex(NET__ERROR, in->pBuffer, in->size);
	DECODE_LENGTH_EXACT(structs::GroupGeneric_Struct);
	SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupGeneric_Struct);
	memcpy(emu->name1, eq->name1, sizeof(emu->name1));
	memcpy(emu->name2, eq->name2, sizeof(emu->name2));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GroupCancelInvite)
{
	DECODE_LENGTH_EXACT(structs::GroupCancel_Struct);
	SETUP_DIRECT_DECODE(GroupCancel_Struct, structs::GroupCancel_Struct);
	memcpy(emu->name1, eq->name1, sizeof(emu->name1));
	memcpy(emu->name2, eq->name2, sizeof(emu->name2));
	IN(toggle);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GMLastName)
{
	DECODE_LENGTH_EXACT(structs::GMLastName_Struct);
	SETUP_DIRECT_DECODE(GMLastName_Struct, structs::GMLastName_Struct);
	memcpy(emu->name, eq->name, sizeof(emu->name));
	memcpy(emu->gmname, eq->gmname, sizeof(emu->gmname));
	memcpy(emu->lastname, eq->lastname, sizeof(emu->lastname));
	for (int i=0; i<4; i++)
	{
		emu->unknown[i] = eq->unknown[i];
	}
	FINISH_DIRECT_DECODE();
}

DECODE(OP_Buff) {
	DECODE_LENGTH_EXACT(structs::SpellBuffFade_Struct_Live);
	SETUP_DIRECT_DECODE(SpellBuffFade_Struct, structs::SpellBuffFade_Struct_Live);
	IN(entityid);
	//IN(slot);
	IN(level);
	IN(effect);
	IN(spellid);
	IN(duration);
	IN(slotid);
	IN(bufffade);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ShopPlayerSell) {
	DECODE_LENGTH_EXACT(structs::Merchant_Purchase_Struct);
	SETUP_DIRECT_DECODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

	IN(npcid);
	emu->itemslot = MainInvRoFToTitaniumSlot(eq->itemslot);
	//IN(itemslot);
	IN(quantity);
	IN(price);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_Save) {
	DECODE_LENGTH_EXACT(structs::Save_Struct);
	SETUP_DIRECT_DECODE(Save_Struct, structs::Save_Struct);
	memcpy(emu->unknown00, eq->unknown00, sizeof(emu->unknown00));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_FindPersonRequest) {
	DECODE_LENGTH_EXACT(structs::FindPersonRequest_Struct);
	SETUP_DIRECT_DECODE(FindPersonRequest_Struct, structs::FindPersonRequest_Struct);
	IN(npc_id);
	IN(client_pos.x);
	IN(client_pos.y);
	IN(client_pos.z);
	FINISH_DIRECT_DECODE();
}


DECODE(OP_Trader) {
	uint32 psize = __packet->size;
	if(psize == sizeof(structs::ClickTrader_Struct))
	{
		DECODE_LENGTH_EXACT(structs::ClickTrader_Struct);
		SETUP_DIRECT_DECODE(ClickTrader_Struct, structs::ClickTrader_Struct);
		MEMSET_IN(ClickTrader_Struct);

		emu->Code = eq->Code;
		// Live actually has 200 items now, but 80 is the most our internal struct supports
		for (uint32 i = 0; i < 80; i++)
		{
			emu->SerialNumber[i] = 0;	// eq->SerialNumber[i];
			emu->ItemCost[i] = eq->ItemCost[i];
		}
		FINISH_DIRECT_DECODE();
	}
	else if(psize == sizeof(structs::Trader_ShowItems_Struct))
	{
		DECODE_LENGTH_EXACT(structs::Trader_ShowItems_Struct);
		SETUP_DIRECT_DECODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
		MEMSET_IN(Trader_ShowItems_Struct);

		emu->Code = eq->Code;
		emu->TraderID = eq->TraderID;

		FINISH_DIRECT_DECODE();
	}
	else if(psize == sizeof(structs::TraderStatus_Struct))
	{
		DECODE_LENGTH_EXACT(structs::TraderStatus_Struct);
		SETUP_DIRECT_DECODE(TraderStatus_Struct, structs::TraderStatus_Struct);
		MEMSET_IN(TraderStatus_Struct);

		emu->Code = eq->Code;

		FINISH_DIRECT_DECODE();
	}
}

DECODE(OP_TraderBuy)
{
	DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
	SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
	MEMSET_IN(TraderBuy_Struct);

	IN(Action);
	IN(Price);
	IN(TraderID);
	memcpy(emu->ItemName, eq->ItemName, sizeof(emu->ItemName));
	IN(ItemID);
	IN(Quantity);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_LootItem) {
	DECODE_LENGTH_EXACT(structs::LootingItem_Struct);
	SETUP_DIRECT_DECODE(LootingItem_Struct, structs::LootingItem_Struct);
	IN(lootee);
	IN(looter);
	emu->slot_id = eq->slot_id - 1;
	IN(auto_loot);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_TributeItem) {
	DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
	SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

	emu->slot = RoFToTitaniumSlot(eq->slot);
	IN(quantity);
	IN(tribute_master_id);
	IN(tribute_points);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_ReadBook) {
	DECODE_LENGTH_EXACT(structs::BookRequest_Struct);
	SETUP_DIRECT_DECODE(BookRequest_Struct, structs::BookRequest_Struct);

	IN(type);
	emu->invslot = 0; // Set to hard 0 since it's not required for the structure to work
	emu->window = (uint8) eq->window;
	strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

	FINISH_DIRECT_DECODE();
}

DECODE(OP_TradeSkillCombine) {
	DECODE_LENGTH_EXACT(structs::NewCombine_Struct);
	SETUP_DIRECT_DECODE(NewCombine_Struct, structs::NewCombine_Struct);

	int16 slot_id = RoFToTitaniumSlot(eq->container_slot);
	if (slot_id == 4000) {
		slot_id = SLOT_TRADESKILL;	// 1000
	}
	emu->container_slot = slot_id;

	FINISH_DIRECT_DECODE();
}

DECODE(OP_RecipeAutoCombine) {
	DECODE_LENGTH_EXACT(structs::RecipeAutoCombine_Struct);
	SETUP_DIRECT_DECODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);

	IN(object_type);
	IN(some_id);
	emu->unknown1 = RoFToTitaniumSlot(eq->container_slot);
	IN(recipe_id);
	IN(reply_code);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_AugmentItem) {
	DECODE_LENGTH_EXACT(structs::AugmentItem_Struct);
	SETUP_DIRECT_DECODE(AugmentItem_Struct, structs::AugmentItem_Struct);

	emu->container_slot = RoFToTitaniumSlot(eq->container_slot);
	//emu->augment_slot = eq->augment_slot;

	FINISH_DIRECT_DECODE();
}

DECODE(OP_AugmentInfo) {
	DECODE_LENGTH_EXACT(structs::AugmentInfo_Struct);
	SETUP_DIRECT_DECODE(AugmentInfo_Struct, structs::AugmentInfo_Struct);

	IN(itemid);
	IN(window);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_FaceChange) {

	DECODE_LENGTH_EXACT(structs::FaceChange_Struct);
	SETUP_DIRECT_DECODE(FaceChange_Struct, structs::FaceChange_Struct);
	IN(haircolor);
	IN(beardcolor);
	IN(eyecolor1);
	IN(eyecolor2);
	IN(hairstyle);
	IN(beard);
	IN(face);
	IN(drakkin_heritage);
	IN(drakkin_tattoo);
	IN(drakkin_details);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_LoadSpellSet)
{
	DECODE_LENGTH_EXACT(structs::LoadSpellSet_Struct);
	SETUP_DIRECT_DECODE(LoadSpellSet_Struct, structs::LoadSpellSet_Struct);

	for(unsigned int i = 0; i < MAX_PP_MEMSPELL; ++i)
	{
		if (eq->spell[i] == 0)
			emu->spell[i] = 0xFFFFFFFF;
		else
			emu->spell[i] = eq->spell[i];
	}

	FINISH_DIRECT_DECODE();
}
DECODE(OP_Damage) {
	DECODE_LENGTH_EXACT(structs::CombatDamage_Struct);
	SETUP_DIRECT_DECODE(CombatDamage_Struct, structs::CombatDamage_Struct);
	IN(target);
	IN(source);
	IN(type);
	IN(spellid);
	IN(damage);
	emu->sequence = eq->sequence;
	FINISH_DIRECT_DECODE();
}

DECODE(OP_EnvDamage) {
	DECODE_LENGTH_EXACT(structs::EnvDamage2_Struct);
	SETUP_DIRECT_DECODE(EnvDamage2_Struct, structs::EnvDamage2_Struct);
	IN(id);
	IN(damage);
	IN(dmgtype);
	emu->constant = 0xFFFF;
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ZoneChange)
{
	DECODE_LENGTH_EXACT(structs::ZoneChange_Struct);
	SETUP_DIRECT_DECODE(ZoneChange_Struct, structs::ZoneChange_Struct);
	memcpy(emu->char_name, eq->char_name, sizeof(emu->char_name));
	IN(zoneID);
	IN(instanceID);
	IN(y);
	IN(x);
	IN(z)
	IN(zone_reason);
	IN(success);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ChannelMessage)
{
	unsigned char *__eq_buffer = __packet->pBuffer;

	char *InBuffer = (char *)__eq_buffer;

	char Sender[64];
	char Target[64];

	VARSTRUCT_DECODE_STRING(Sender, InBuffer);
	VARSTRUCT_DECODE_STRING(Target, InBuffer);

	InBuffer += 4;

	uint32 Language = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
	uint32 Channel = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

	InBuffer += 5;

	uint32 Skill = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

	__packet->size = sizeof(ChannelMessage_Struct) + strlen(InBuffer) + 1;
	__packet->pBuffer = new unsigned char[__packet->size];
	ChannelMessage_Struct *emu = (ChannelMessage_Struct *) __packet->pBuffer;

	strn0cpy(emu->targetname, Target, sizeof(emu->targetname));
	strn0cpy(emu->sender, Target, sizeof(emu->sender));
	emu->language = Language;
	emu->chan_num = Channel;
	emu->skill_in_language = Skill;
	strcpy(emu->message, InBuffer);

	delete [] __eq_buffer;
}

DECODE(OP_ZoneEntry)
{
	DECODE_LENGTH_EXACT(structs::ClientZoneEntry_Struct);
	SETUP_DIRECT_DECODE(ClientZoneEntry_Struct, structs::ClientZoneEntry_Struct);
	memcpy(emu->char_name, eq->char_name, sizeof(emu->char_name));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_RemoveBlockedBuffs) { DECODE_FORWARD(OP_BlockedBuffs); }

DECODE(OP_BlockedBuffs)
{
	DECODE_LENGTH_EXACT(structs::BlockedBuffs_Struct);
	SETUP_DIRECT_DECODE(BlockedBuffs_Struct, structs::BlockedBuffs_Struct);

	for(uint32 i = 0; i < BLOCKED_BUFF_COUNT; ++i)
		emu->SpellID[i] = eq->SpellID[i];

	IN(Count);
	IN(Pet);
	IN(Initialise);
	IN(Flags);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_GuildStatus)
{
	DECODE_LENGTH_EXACT(structs::GuildStatus_Struct);
	SETUP_DIRECT_DECODE(GuildStatus_Struct, structs::GuildStatus_Struct);

	memcpy(emu->Name, eq->Name, sizeof(emu->Name));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_RezzAnswer)
{
	DECODE_LENGTH_EXACT(structs::Resurrect_Struct);
	SETUP_DIRECT_DECODE(Resurrect_Struct, structs::Resurrect_Struct);

	IN(zone_id);
	IN(instance_id);
	IN(y);
	IN(x);
	IN(z);
	memcpy(emu->your_name, eq->your_name, sizeof(emu->your_name));
	memcpy(emu->rezzer_name, eq->rezzer_name, sizeof(emu->rezzer_name));
	IN(spellid);
	memcpy(emu->corpse_name, eq->corpse_name, sizeof(emu->corpse_name));
	IN(action);

	FINISH_DIRECT_DECODE();
}

uint32 NextItemInstSerialNumber = 1;
uint32 MaxInstances = 2000000000;

static inline int32 GetNextItemInstSerialNumber() {

	if(NextItemInstSerialNumber >= MaxInstances)
		NextItemInstSerialNumber = 1;
	else
		NextItemInstSerialNumber++;

	return NextItemInstSerialNumber;
}


char* SerializeItem(const ItemInst *inst, int16 slot_id_in, uint32 *length, uint8 depth) {
	uint8 null_term = 0;
	bool stackable = inst->IsStackable();
	uint32 merchant_slot = inst->GetMerchantSlot();
	uint32 charges = inst->GetCharges();
	if (!stackable && charges > 254)
		charges = 0xFFFFFFFF;

	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	ss.clear();

	const Item_Struct *item = inst->GetItem();
	//_log(NET__ERROR, "Serialize called for: %s", item->Name);

	RoF::structs::ItemSerializationHeader hdr;

	//sprintf(hdr.unknown000, "06e0002Y1W00");

	snprintf( hdr.unknown000, sizeof(hdr.unknown000), "%012d", item->ID );

	hdr.stacksize = stackable ? charges : 1;
	hdr.unknown004 = 0;

	structs::ItemSlotStruct slot_id = TitaniumToRoFSlot(slot_id_in);

	hdr.slot_type = (merchant_slot == 0) ? slot_id.SlotType : 9; // 9 is merchant 20 is reclaim items?
	hdr.main_slot = (merchant_slot == 0) ? slot_id.MainSlot : merchant_slot;
	hdr.sub_slot = (merchant_slot == 0) ? slot_id.SubSlot : 0xffff;
	hdr.unknown013 = (merchant_slot == 0) ? slot_id.AugSlot : 0xffff;
	//hdr.unknown013 = 0xffff;
	hdr.price = inst->GetPrice();
	hdr.merchant_slot = (merchant_slot == 0) ? 1 : inst->GetMerchantCount();
	//hdr.merchant_slot = (merchant_slot == 0) ? 1 : 0xffffffff;
	hdr.unknown020 = 0;
	hdr.instance_id = (merchant_slot == 0) ? inst->GetSerialNumber() : merchant_slot;
	hdr.unknown028 = 0;
	hdr.last_cast_time = ((item->RecastDelay > 1) ? 1212693140 : 0);
	hdr.charges = (stackable ? (item->MaxCharges ? 1 : 0) : charges);
	hdr.inst_nodrop = inst->IsInstNoDrop() ? 1 : 0;
	hdr.unknown044 = 0;
	hdr.unknown048 = 0;
	hdr.unknown052 = 0;
	hdr.unknown056 = 0;
	hdr.unknown060 = 0;
	hdr.unknown061 = 0;
	hdr.unknown062 = 0;
	hdr.unknowna1 = 0xffffffff;
	hdr.unknowna2 = 0;
	hdr.unknown063 = 0;
	hdr.unknowna3 = 0;
	hdr.unknowna4 = 0xffffffff;
	hdr.unknowna5 = 0;
	hdr.ItemClass = item->ItemClass;

	ss.write((const char*)&hdr, sizeof(RoF::structs::ItemSerializationHeader));


	if(strlen(item->Name) > 0)
	{
		ss.write(item->Name, strlen(item->Name));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	if(strlen(item->Lore) > 0)
	{
		ss.write(item->Lore, strlen(item->Lore));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	if(strlen(item->IDFile) > 0)
	{
		ss.write(item->IDFile, strlen(item->IDFile));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	ss.write((const char*)&null_term, sizeof(uint8));
	//_log(NET__ERROR, "ItemBody struct is %i bytes", sizeof(RoF::structs::ItemBodyStruct));
	RoF::structs::ItemBodyStruct ibs;
	memset(&ibs, 0, sizeof(RoF::structs::ItemBodyStruct));

	uint32 adjusted_slots = item->Slots;

	// Conversions for Ammo and Power Source Slots
	if(item->Slots & (1 << 21) & (1 << 22))
	{
		// Do nothing
	}
	else
	{
		if(item->Slots & (1 << 21))	// Ammo Slot from Database
		{
			adjusted_slots -= (1 << 21);	// Ammo Slot in Titanium
			adjusted_slots += (1 << 22);	// Ammo Slot in SoF
		}

		if(item->Slots & (1 << 22))	// Power Source Slot from Database
		{
			adjusted_slots -= (1 << 22);	// Non Existant Worn Slot in Titanium
			adjusted_slots += (1 << 21);	// Power Source Slot in SoF
		}
	}

	ibs.id = item->ID;
	ibs.weight = item->Weight;
	ibs.norent = item->NoRent;
	ibs.nodrop = item->NoDrop;
	ibs.attune = item->Attuneable;
	ibs.size = item->Size;
	ibs.slots = adjusted_slots;
	ibs.price = item->Price;
	ibs.icon = item->Icon;
	ibs.unknown1 = 1;
	ibs.unknown2 = 1;
	ibs.BenefitFlag = item->BenefitFlag;
	ibs.tradeskills = item->Tradeskills;
	ibs.CR = item->CR;
	ibs.DR = item->DR;
	ibs.PR = item->PR;
	ibs.MR = item->MR;
	ibs.FR = item->FR;
	ibs.SVCorruption = item->SVCorruption;
	ibs.AStr = item->AStr;
	ibs.ASta = item->ASta;
	ibs.AAgi = item->AAgi;
	ibs.ADex = item->ADex;
	ibs.ACha = item->ACha;
	ibs.AInt = item->AInt;
	ibs.AWis = item->AWis;

	ibs.HP = item->HP;
	ibs.Mana = item->Mana;
	ibs.Endur = item->Endur;
	ibs.AC = item->AC;
	ibs.regen = item->Regen;
	ibs.mana_regen = item->ManaRegen;
	ibs.end_regen = item->EnduranceRegen;
	ibs.Classes = item->Classes;
	ibs.Races = item->Races;
	ibs.Deity = item->Deity;
	ibs.SkillModValue = item->SkillModValue;
	ibs.SkillModMax = 0xffffffff;
	ibs.SkillModType = (int8)(item->SkillModType);
	ibs.SkillModExtra = 0;
	ibs.BaneDmgRace = item->BaneDmgRace;
	ibs.BaneDmgBody = item->BaneDmgBody;
	ibs.BaneDmgRaceAmt = item->BaneDmgRaceAmt;
	ibs.BaneDmgAmt = item->BaneDmgAmt;
	ibs.Magic = item->Magic;
	ibs.CastTime_ = item->CastTime_;
	ibs.ReqLevel = item->ReqLevel;
	ibs.RecLevel = item->RecLevel;
	ibs.RecSkill = item->RecSkill;
	ibs.BardType = item->BardType;
	ibs.BardValue = item->BardValue;
	ibs.Light = item->Light;
	ibs.Delay = item->Delay;
	ibs.ElemDmgType = item->ElemDmgType;
	ibs.ElemDmgAmt = item->ElemDmgAmt;
	ibs.Range = item->Range;
	ibs.Damage = item->Damage;
	ibs.Color = item->Color;
	ibs.Prestige = 0;
	ibs.ItemType = item->ItemType;
	ibs.Material = item->Material;
	ibs.unknown7 = 0;
	ibs.EliteMaterial = item->EliteMaterial;
	ibs.unknown_RoF3 = 0;
	ibs.unknown_RoF4 = 0;
	ibs.SellRate = item->SellRate;
	ibs.CombatEffects = item->CombatEffects;
	ibs.Shielding = item->Shielding;
	ibs.StunResist = item->StunResist;
	ibs.StrikeThrough = item->StrikeThrough;
	ibs.ExtraDmgSkill = item->ExtraDmgSkill;
	ibs.ExtraDmgAmt = item->ExtraDmgAmt;
	ibs.SpellShield = item->SpellShield;
	ibs.Avoidance = item->Avoidance;
	ibs.Accuracy = item->Accuracy;
	ibs.FactionAmt1 = item->FactionAmt1;
	ibs.FactionMod1 = item->FactionMod1;
	ibs.FactionAmt2 = item->FactionAmt2;
	ibs.FactionMod2 = item->FactionMod2;
	ibs.FactionAmt3 = item->FactionAmt3;
	ibs.FactionMod3 = item->FactionMod3;
	ibs.FactionAmt4 = item->FactionAmt4;
	ibs.FactionMod4 = item->FactionMod4;

	ss.write((const char*)&ibs, sizeof(RoF::structs::ItemBodyStruct));

	//charm text
	if(strlen(item->CharmFile) > 0)
	{
		ss.write((const char*)item->CharmFile, strlen(item->CharmFile));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	//_log(NET__ERROR, "ItemBody secondary struct is %i bytes", sizeof(RoF::structs::ItemSecondaryBodyStruct));
	RoF::structs::ItemSecondaryBodyStruct isbs;
	memset(&isbs, 0, sizeof(RoF::structs::ItemSecondaryBodyStruct));

	isbs.augtype = item->AugType;
	isbs.augrestrict = item->AugRestrict;
	isbs.augdistiller = 0;

	for(int x = 0; x < 5; ++x)
	{
		isbs.augslots[x].type = item->AugSlotType[x];
		isbs.augslots[x].visible = item->AugSlotVisible[x];
		isbs.augslots[x].unknown = item->AugSlotUnk2[x];
	}

	// Increased to 6 max aug slots
	isbs.augslots[5].type = 0;
	isbs.augslots[5].visible = 1;
	isbs.augslots[5].unknown = 0;

	isbs.ldonpoint_type = item->PointType;
	isbs.ldontheme = item->LDoNTheme;
	isbs.ldonprice = item->LDoNPrice;
	isbs.ldonsellbackrate = item->LDoNSellBackRate;
	isbs.ldonsold = item->LDoNSold;

	isbs.bagtype = item->BagType;
	isbs.bagslots = item->BagSlots;
	isbs.bagsize = item->BagSize;
	isbs.wreduction = item->BagWR;

	isbs.book = item->Book;
	isbs.booktype = item->BookType;

	ss.write((const char*)&isbs, sizeof(RoF::structs::ItemSecondaryBodyStruct));

	if(strlen(item->Filename) > 0)
	{
		ss.write((const char*)item->Filename, strlen(item->Filename));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	//_log(NET__ERROR, "ItemBody tertiary struct is %i bytes", sizeof(RoF::structs::ItemTertiaryBodyStruct));
	RoF::structs::ItemTertiaryBodyStruct itbs;
	memset(&itbs, 0, sizeof(RoF::structs::ItemTertiaryBodyStruct));

	itbs.loregroup = item->LoreGroup;
	itbs.artifact = item->ArtifactFlag;
	itbs.summonedflag = item->SummonedFlag;
	itbs.favor = item->Favor;
	itbs.fvnodrop = item->FVNoDrop;
	itbs.dotshield = item->DotShielding;
	itbs.atk = item->Attack;
	itbs.haste = item->Haste;
	itbs.damage_shield = item->DamageShield;
	itbs.guildfavor = item->GuildFavor;
	itbs.augdistil = item->AugDistiller;
	itbs.unknown3 = 0xffffffff;
	itbs.unknown4 = 0;
	itbs.no_pet = item->NoPet;
	itbs.unknown5 = 0;

	itbs.potion_belt_enabled = item->PotionBelt;
	itbs.potion_belt_slots = item->PotionBeltSlots;
	itbs.stacksize = stackable ? item->StackSize : 0;
	itbs.no_transfer = item->NoTransfer;
	itbs.expendablearrow = item->ExpendableArrow;

	itbs.unknown8 = 0;
	itbs.unknown9 = 0;
	itbs.unknown10 = 0;
	itbs.unknown11 = 0;
	itbs.unknown12 = 0;
	itbs.unknown13 = 0;
	itbs.unknown14 = 0;

	ss.write((const char*)&itbs, sizeof(RoF::structs::ItemTertiaryBodyStruct));

	// Effect Structures Broken down to allow variable length strings for effect names
	int32 effect_unknown = 0;

	//_log(NET__ERROR, "ItemBody Click effect struct is %i bytes", sizeof(RoF::structs::ClickEffectStruct));
	RoF::structs::ClickEffectStruct ices;
	memset(&ices, 0, sizeof(RoF::structs::ClickEffectStruct));

	ices.effect = item->Click.Effect;
	ices.level2 = item->Click.Level2;
	ices.type = item->Click.Type;
	ices.level = item->Click.Level;
	ices.max_charges = item->MaxCharges;
	ices.cast_time = item->CastTime;
	ices.recast = item->RecastDelay;
	ices.recast_type = item->RecastType;

	ss.write((const char*)&ices, sizeof(RoF::structs::ClickEffectStruct));

	if(strlen(item->ClickName) > 0)
	{
		ss.write((const char*)item->ClickName, strlen(item->ClickName));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	ss.write((const char*)&effect_unknown, sizeof(int32));	// clickunk7

	//_log(NET__ERROR, "ItemBody proc effect struct is %i bytes", sizeof(RoF::structs::ProcEffectStruct));
	RoF::structs::ProcEffectStruct ipes;
	memset(&ipes, 0, sizeof(RoF::structs::ProcEffectStruct));

	ipes.effect = item->Proc.Effect;
	ipes.level2 = item->Proc.Level2;
	ipes.type = item->Proc.Type;
	ipes.level = item->Proc.Level;
	ipes.procrate = item->ProcRate;

	ss.write((const char*)&ipes, sizeof(RoF::structs::ProcEffectStruct));

	if(strlen(item->ProcName) > 0)
	{
		ss.write((const char*)item->ProcName, strlen(item->ProcName));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown5

	//_log(NET__ERROR, "ItemBody worn effect struct is %i bytes", sizeof(RoF::structs::WornEffectStruct));
	RoF::structs::WornEffectStruct iwes;
	memset(&iwes, 0, sizeof(RoF::structs::WornEffectStruct));

	iwes.effect = item->Worn.Effect;
	iwes.level2 = item->Worn.Level2;
	iwes.type = item->Worn.Type;
	iwes.level = item->Worn.Level;

	ss.write((const char*)&iwes, sizeof(RoF::structs::WornEffectStruct));

	if(strlen(item->WornName) > 0)
	{
		ss.write((const char*)item->WornName, strlen(item->WornName));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

	RoF::structs::WornEffectStruct ifes;
	memset(&ifes, 0, sizeof(RoF::structs::WornEffectStruct));

	ifes.effect = item->Focus.Effect;
	ifes.level2 = item->Focus.Level2;
	ifes.type = item->Focus.Type;
	ifes.level = item->Focus.Level;

	ss.write((const char*)&ifes, sizeof(RoF::structs::WornEffectStruct));

	if(strlen(item->FocusName) > 0)
	{
		ss.write((const char*)item->FocusName, strlen(item->FocusName));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

	RoF::structs::WornEffectStruct ises;
	memset(&ises, 0, sizeof(RoF::structs::WornEffectStruct));

	ises.effect = item->Scroll.Effect;
	ises.level2 = item->Scroll.Level2;
	ises.type = item->Scroll.Type;
	ises.level = item->Scroll.Level;

	ss.write((const char*)&ises, sizeof(RoF::structs::WornEffectStruct));

	if(strlen(item->ScrollName) > 0)
	{
		ss.write((const char*)item->ScrollName, strlen(item->ScrollName));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

	// Bard Effect?
	RoF::structs::WornEffectStruct ibes;
	memset(&ibes, 0, sizeof(RoF::structs::WornEffectStruct));

	ibes.effect = 0xffffffff;
	ibes.level2 = 0;
	ibes.type = 0;
	ibes.level = 0;
	//ibes.unknown6 = 0xffffffff;

	ss.write((const char*)&ibes, sizeof(RoF::structs::WornEffectStruct));

	/*
	if(strlen(item->BardName) > 0)
	{
		ss.write((const char*)item->BardName, strlen(item->BardName));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else */
		ss.write((const char*)&null_term, sizeof(uint8));

	ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6
	// End of Effects

	//_log(NET__ERROR, "ItemBody Quaternary effect struct is %i bytes", sizeof(RoF::structs::ItemQuaternaryBodyStruct));
	RoF::structs::ItemQuaternaryBodyStruct iqbs;
	memset(&iqbs, 0, sizeof(RoF::structs::ItemQuaternaryBodyStruct));

	iqbs.scriptfileid = item->ScriptFileID;
	iqbs.quest_item = item->QuestItemFlag;
	iqbs.Power = 0;
	iqbs.Purity = item->Purity;
	iqbs.unknown16 = 0;
	iqbs.BackstabDmg = item->BackstabDmg;
	iqbs.DSMitigation = item->DSMitigation;
	iqbs.HeroicStr = item->HeroicStr;
	iqbs.HeroicInt = item->HeroicInt;
	iqbs.HeroicWis = item->HeroicWis;
	iqbs.HeroicAgi = item->HeroicAgi;
	iqbs.HeroicDex = item->HeroicDex;
	iqbs.HeroicSta = item->HeroicSta;
	iqbs.HeroicCha = item->HeroicCha;
	iqbs.HeroicMR = item->HeroicMR;
	iqbs.HeroicFR = item->HeroicFR;
	iqbs.HeroicCR = item->HeroicCR;
	iqbs.HeroicDR = item->HeroicDR;
	iqbs.HeroicPR = item->HeroicPR;
	iqbs.HeroicSVCorrup = item->HeroicSVCorrup;
	iqbs.HealAmt = item->HealAmt;
	iqbs.SpellDmg = item->SpellDmg;
	iqbs.clairvoyance = item->Clairvoyance;
	iqbs.unknown28 = 0;
	iqbs.unknown30 = 0;
	iqbs.unknown39 = 1;

	iqbs.subitem_count = 0;

	char *SubSerializations[10];

	uint32 SubLengths[10];

	for(int x = 0; x < 10; ++x) {

		SubSerializations[x] = nullptr;

		const ItemInst* subitem = ((const ItemInst*)inst)->GetItem(x);

		if(subitem) {

			int SubSlotNumber;

			iqbs.subitem_count++;

			if(slot_id_in >= 22 && slot_id_in < 30)
				SubSlotNumber = (((slot_id_in + 3) * 10) + x + 1);
			else if(slot_id_in >= 2000 && slot_id_in <= 2023)
				SubSlotNumber = (((slot_id_in - 2000) * 10) + 2030 + x + 1);
			else if(slot_id_in >= 2500 && slot_id_in <= 2501)
				SubSlotNumber = (((slot_id_in - 2500) * 10) + 2530 + x + 1);
			else
				SubSlotNumber = slot_id_in; // ???????

			SubSerializations[x] = SerializeItem(subitem, SubSlotNumber, &SubLengths[x], depth + 1);
		}
	}

	ss.write((const char*)&iqbs, sizeof(RoF::structs::ItemQuaternaryBodyStruct));

	for(int x = 0; x < 10; ++x) {

		if(SubSerializations[x]) {

			ss.write((const char*)&x, sizeof(uint32));

			ss.write(SubSerializations[x], SubLengths[x]);

			safe_delete_array(SubSerializations[x]);
		}
	}

	char* item_serial = new char[ss.tellp()];
	memset(item_serial, 0, ss.tellp());
	memcpy(item_serial, ss.str().c_str(), ss.tellp());

	*length = ss.tellp();
	return item_serial;
}

} //end namespace RoF
