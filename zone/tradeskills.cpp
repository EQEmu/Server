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
#include <stdlib.h>
#include <list>

#ifndef WIN32
#include <netinet/in.h>	//for htonl
#endif

#include "masterentity.h"
#include "zonedb.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "titles.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/StringUtil.h"
#include "../common/rulesys.h"
#include "QuestParserCollection.h"

static const SkillType TradeskillUnknown = _1H_BLUNT; /* an arbitrary non-tradeskill */

void Object::HandleAugmentation(Client* user, const AugmentItem_Struct* in_augment, Object *worldo)
{
	if (!user || !in_augment)
	{
		LogFile->write(EQEMuLog::Error, "Client or AugmentItem_Struct not set in Object::HandleAugmentation");
		return;
	}

	ItemInst* container = nullptr;

	if (worldo)
	{
		container = worldo->m_inst;
	}
	else
	{
		// Check to see if they have an inventory container type 53 that is used for this.
		Inventory& user_inv = user->GetInv();
		ItemInst* inst = nullptr;

		inst = user_inv.GetItem(in_augment->container_slot);
		if (inst)
		{
			const Item_Struct* item = inst->GetItem();
			if (item && inst->IsType(ItemClassContainer) && item->BagType == 53)
			{
				// We have found an appropriate inventory augmentation sealer
				container = inst;

				// Verify that no more than two items are in container to guarantee no inadvertant wipes.
				uint8 itemsFound = 0;
				for (uint8 i=0; i<10; i++)
				{
					const ItemInst* inst = container->GetItem(i);
					if (inst)
					{
						itemsFound++;
					}
				}

				if (itemsFound != 2)
				{
					user->Message(13, "Error: Too many/few items in augmentation container.");
					return;
				}
			}
		}
	}

	if(!container)
	{
		LogFile->write(EQEMuLog::Error, "Player tried to augment an item without a container set.");
		user->Message(13, "Error: This item is not a container!");
		return;
	}

	ItemInst *tobe_auged, *auged_with = nullptr;
	int8 slot=-1;

	// Verify 2 items in the augmentation device
	if (container->GetItem(0) && container->GetItem(1))
	{
		// Verify 1 item is augmentable and the other is not
		if (container->GetItem(0)->IsAugmentable() && !container->GetItem(1)->IsAugmentable())
		{
			tobe_auged = container->GetItem(0);
			auged_with = container->GetItem(1);
		}
		else if (!container->GetItem(0)->IsAugmentable() && container->GetItem(1)->IsAugmentable())
		{
			tobe_auged = container->GetItem(1);
			auged_with = container->GetItem(0);
		}
		else
		{
			// Either 2 augmentable items found or none found
			// This should never occur due to client restrictions, but prevent in case of a hack
			user->Message(13, "Error: Must be 1 augmentable item in the sealer");
			return;
		}
	}
	else
	{
		// This happens if the augment button is clicked more than once quickly while augmenting
		if (!container->GetItem(0))
		{
			user->Message(13, "Error: No item in slot 0 of sealer");
		}
		if (!container->GetItem(1))
		{
			user->Message(13, "Error: No item in slot 1 of sealer");
		}
		return;
	}

	bool deleteItems = false;

	ItemInst *itemOneToPush = nullptr, *itemTwoToPush = nullptr;

	// Adding augment
	if (in_augment->augment_slot == -1)
	{
		if (((slot=tobe_auged->AvailableAugmentSlot(auged_with->GetAugmentType()))!=-1) && 
			(tobe_auged->AvailableWearSlot(auged_with->GetItem()->Slots)))
		{
			tobe_auged->PutAugment(slot, *auged_with);

			ItemInst *aug = tobe_auged->GetAugment(slot);
			if(aug) {
				std::vector<void*> args;
				args.push_back(aug);
				parse->EventItem(EVENT_AUGMENT_ITEM, user, tobe_auged, nullptr, "", slot, &args);

				args.assign(1, tobe_auged);
				parse->EventItem(EVENT_AUGMENT_INSERT, user, aug, nullptr, "", slot, &args);
			}

			itemOneToPush = tobe_auged->Clone();
			deleteItems = true;
		}
		else
		{
			user->Message(13, "Error: No available slot for augment");
		}
	}
	else
	{
		ItemInst *old_aug = nullptr;
		const uint32 id = auged_with->GetID();
		ItemInst *aug = tobe_auged->GetAugment(in_augment->augment_slot);
		if(aug) {
			std::vector<void*> args;
			args.push_back(aug);
			parse->EventItem(EVENT_UNAUGMENT_ITEM, user, tobe_auged, nullptr, "", slot, &args);

			args.assign(1, tobe_auged);
			bool destroyed = false;
			if(id == 40408 || id == 40409 || id == 40410) {
				destroyed = true;
			}

			args.push_back(&destroyed);

			parse->EventItem(EVENT_AUGMENT_REMOVE, user, aug, nullptr, "", slot, &args);
		}

		if(id == 40408 || id == 40409 || id == 40410)
			tobe_auged->DeleteAugment(in_augment->augment_slot);
		else
			old_aug = tobe_auged->RemoveAugment(in_augment->augment_slot);

		itemOneToPush = tobe_auged->Clone();
		if (old_aug)
			itemTwoToPush = old_aug->Clone();



		deleteItems = true;
	}

	if (deleteItems)
	{
		if (worldo)
		{
			container->Clear();
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_ClearObject, sizeof(ClearObject_Struct));
			ClearObject_Struct *cos = (ClearObject_Struct *)outapp->pBuffer;
			cos->Clear = 1;
			user->QueuePacket(outapp);
			safe_delete(outapp);
			database.DeleteWorldContainer(worldo->m_id, zone->GetZoneID());
		}
		else
		{
			// Delete items in our inventory container...
			for (uint8 i=0; i<10; i++)
			{
				const ItemInst* inst = container->GetItem(i);
				if (inst)
				{
					user->DeleteItemInInventory(Inventory::CalcSlotId(in_augment->container_slot,i),0,true);
				}
			}
			// Explicitly mark container as cleared.
			container->Clear();
		}
	}

	// Must push items after the items in inventory are deleted - necessary due to lore items...
	if (itemOneToPush)
	{
		user->PushItemOnCursor(*itemOneToPush, true);
	}

	if (itemTwoToPush)
	{
		user->PushItemOnCursor(*itemTwoToPush, true);
	}

}

// Perform tradeskill combine
void Object::HandleCombine(Client* user, const NewCombine_Struct* in_combine, Object *worldo)
{
	if (!user || !in_combine) {
		LogFile->write(EQEMuLog::Error, "Client or NewCombine_Struct not set in Object::HandleCombine");
		return;
	}

	Inventory& user_inv = user->GetInv();
	PlayerProfile_Struct& user_pp = user->GetPP();
	ItemInst* container = nullptr;
	ItemInst* inst = nullptr;
	uint8 c_type = 0xE8;
	uint32 some_id = 0;
	bool worldcontainer=false;

	if (in_combine->container_slot == SLOT_TRADESKILL) {
		if(!worldo) {
			user->Message(13, "Error: Server is not aware of the tradeskill container you are attempting to use");
			return;
		}
		c_type = worldo->m_type;
		inst = worldo->m_inst;
		worldcontainer=true;
	}
	else {
		inst = user_inv.GetItem(in_combine->container_slot);
		if (inst) {
			const Item_Struct* item = inst->GetItem();
			if (item && inst->IsType(ItemClassContainer)) {
				c_type = item->BagType;
				some_id = item->ID;
			}
		}
	}

	if (!inst || !inst->IsType(ItemClassContainer)) {
		user->Message(13, "Error: Server does not recognize specified tradeskill container");
		return;
	}

	container = inst;

	DBTradeskillRecipe_Struct spec;
	if (!database.GetTradeRecipe(container, c_type, some_id, user->CharacterID(), &spec)) {
		user->Message_StringID(MT_Emote,TRADESKILL_NOCOMBINE);
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	// Character hasn't learnt the recipe yet.
	// must_learn:
	// bit 1 (0x01): recipe can't be experimented
	// bit 2 (0x02): can try to experiment but not useable for auto-combine until learnt
	// bit 5 (0x10): no learn message, use unlisted flag to prevent it showing up on search
	// bit 6 (0x20): unlisted recipe flag
	if ((spec.must_learn&0xF) == 1 && !spec.has_learnt) {
		// Made up message for the client. Just giving a DNC is the other option.
		user->Message(4, "You need to learn how to combine these first.");
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}
	// Character does not have the required skill.
	if(spec.skill_needed > 0 && user->GetSkill(spec.tradeskill) < spec.skill_needed ) {
		// Notify client.
		user->Message(4, "You are not skilled enough.");
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	//changing from a switch to string of if's since we don't need to iterate through all of the skills in the SkillType enum
	if (spec.tradeskill == ALCHEMY) {
		if (user_pp.class_ != SHAMAN) {
			user->Message(13, "This tradeskill can only be performed by a shaman.");
			return;
		}
		else if (user_pp.level < MIN_LEVEL_ALCHEMY) {
			user->Message(13, "You cannot perform alchemy until you reach level %i.", MIN_LEVEL_ALCHEMY);
			return;
		}
	}
	else if (spec.tradeskill == TINKERING) {
		if (user_pp.race != GNOME) {
			user->Message(13, "Only gnomes can tinker.");
			return;
		}
	}
	else if (spec.tradeskill == MAKE_POISON) {
		if (user_pp.class_ != ROGUE) {
			user->Message(13, "Only rogues can mix poisons.");
			return;
		}
	}

	// Send acknowledgement packets to client
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
	user->QueuePacket(outapp);
	safe_delete(outapp);

	//now clean out the containers.
	if(worldcontainer){
		container->Clear();
		outapp = new EQApplicationPacket(OP_ClearObject, sizeof(ClearObject_Struct));
		ClearObject_Struct *cos = (ClearObject_Struct *)outapp->pBuffer;
		cos->Clear = 1;
		user->QueuePacket(outapp);
		safe_delete(outapp);
		database.DeleteWorldContainer(worldo->m_id, zone->GetZoneID());
	} else{
		for (uint8 i=0; i<10; i++){
			const ItemInst* inst = container->GetItem(i);
			if (inst) {
				user->DeleteItemInInventory(Inventory::CalcSlotId(in_combine->container_slot,i),0,true);
			}
		}
		container->Clear();
	}
	//do the check and send results...
	bool success = user->TradeskillExecute(&spec);

	// Learn new recipe message
	// Update Made count
	if (success) {
		if (!spec.has_learnt && ((spec.must_learn&0x10) != 0x10)) {
			user->Message_StringID(4, TRADESKILL_LEARN_RECIPE, spec.name.c_str());
		}
		database.UpdateRecipeMadecount(spec.recipe_id, user->CharacterID(), spec.madecount+1);
	}

	// Replace the container on success if required.
	//

	if(success && spec.replace_container) {
		if(worldcontainer){
			//should report this error, but we dont have the recipe ID, so its not very useful
			LogFile->write(EQEMuLog::Error, "Replace container combine executed in a world container.");
		}
		else
			user->DeleteItemInInventory(in_combine->container_slot, 0, true);
	}
	if (success)
		parse->EventPlayer(EVENT_COMBINE_SUCCESS, user, spec.name.c_str(), spec.recipe_id);
	else
		parse->EventPlayer(EVENT_COMBINE_FAILURE, user, spec.name.c_str(), spec.recipe_id);
}

void Object::HandleAutoCombine(Client* user, const RecipeAutoCombine_Struct* rac) {

	//get our packet ready, gotta send one no matter what...
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RecipeAutoCombine, sizeof(RecipeAutoCombine_Struct));
	RecipeAutoCombine_Struct *outp = (RecipeAutoCombine_Struct *)outapp->pBuffer;
	outp->object_type = rac->object_type;
	outp->some_id = rac->some_id;
	outp->unknown1 = rac->unknown1;
	outp->recipe_id = rac->recipe_id;
	outp->reply_code = 0xFFFFFFF5;	//default fail.


	//ask the database for the recipe to make sure it exists...
	DBTradeskillRecipe_Struct spec;
	if (!database.GetTradeRecipe(rac->recipe_id, rac->object_type, rac->some_id, user->CharacterID(), &spec)) {
		LogFile->write(EQEMuLog::Error, "Unknown recipe for HandleAutoCombine: %u\n", rac->recipe_id);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	// Character hasn't learnt the recipe yet.
	// This shouldn't happen.
	if ((spec.must_learn&0xf) && !spec.has_learnt) {
		// Made up message for the client. Just giving a DNC is the other option.
		user->Message(4, "You need to learn how to combine these first.");
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}


	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *query = 0;

	uint32 qlen = 0;
	uint8 qcount = 0;

	//pull the list of components
	qlen = MakeAnyLenString(&query, "SELECT tre.item_id,tre.componentcount "
	 " FROM tradeskill_recipe_entries AS tre "
	 " WHERE tre.componentcount > 0 AND tre.recipe_id=%u", rac->recipe_id);

	if (!database.RunQuery(query, qlen, errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in HandleAutoCombine query '%s': %s", query, errbuf);
		safe_delete_array(query);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}
	safe_delete_array(query);

	qcount = mysql_num_rows(result);
	if(qcount < 1) {
		LogFile->write(EQEMuLog::Error, "Error in HandleAutoCombine: no components returned");
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}
	if(qcount > 10) {
		LogFile->write(EQEMuLog::Error, "Error in HandleAutoCombine: too many components returned (%u)", qcount);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	uint32 items[10];
	memset(items, 0, sizeof(items));
	uint8 counts[10];
	memset(counts, 0, sizeof(counts));


	//search for all the items in their inventory
	Inventory& user_inv = user->GetInv();
	uint8 count = 0;
	uint8 needcount = 0;
	uint8 r,k;

	std::list<int> MissingItems;

	for(r = 0; r < qcount; r++) {
		row = mysql_fetch_row(result);
		uint32 item = (uint32)atoi(row[0]);
		uint8 num = (uint8) atoi(row[1]);

		needcount += num;

		//because a HasItem on items with num > 1 only returns the
		//last-most slot... the results of this are useless to us
		//when we go to delete them because we cannot assume it is in a single stack.
		if(user_inv.HasItem(item, num, invWherePersonal) != SLOT_INVALID)
			count += num;
		else
			MissingItems.push_back(item);

		//dont start deleting anything until we have found it all.
		items[r] = item;
		counts[r] = num;
	}
	mysql_free_result(result);

	//make sure we found it all...
	if(count != needcount)
	{
		user->QueuePacket(outapp);

		safe_delete(outapp);

		user->Message_StringID(MT_Skills, TRADESKILL_MISSING_COMPONENTS);

		for(std::list<int>::iterator it = MissingItems.begin(); it != MissingItems.end(); ++it)
		{
			const Item_Struct* item = database.GetItem(*it);

			if(item)
				user->Message_StringID(MT_Skills, TRADESKILL_MISSING_ITEM, item->Name);
		}

		return;
	}

	//now we know they have everything...

	//remove all the items from the players inventory, with updates...
	int16 slot;
	for(r = 0; r < qcount; r++) {
		if(items[r] == 0 || counts[r] == 0)
			continue;	//skip empties, could prolly break here

		//we have to loop here to delete 1 at a time in case its in multiple stacks.
		for(k = 0; k < counts[r]; k++) {
			slot = user_inv.HasItem(items[r], 1, invWherePersonal);
			if(slot == SLOT_INVALID) {
				//WTF... I just checked this above, but just to be sure...
				//we cant undo the previous deletes without a lot of work.
				//so just call it quits, this shouldent ever happen anyways.
				user->QueuePacket(outapp);
				safe_delete(outapp);
				return;
			}

			const ItemInst* inst = user_inv.GetItem(slot);

			if (inst && !inst->IsStackable())
			{
				user->DeleteItemInInventory(slot, 0, true);
			}
			else
			{
				user->DeleteItemInInventory(slot, 1, true);
			}
		}
	}

	//otherwise, we found it all...
	outp->reply_code = 0x00000000;	//success for finding it...

	user->QueuePacket(outapp);
	safe_delete(outapp);


	//now actually try to make something...

	bool success = user->TradeskillExecute(&spec);

	if (success) {
		if (!spec.has_learnt && ((spec.must_learn & 0x10) != 0x10)) {
			user->Message_StringID(4, TRADESKILL_LEARN_RECIPE, spec.name.c_str());
		}
		database.UpdateRecipeMadecount(spec.recipe_id, user->CharacterID(), spec.madecount+1);
	}


	//TODO: find in-pack containers in inventory, make sure they are really
	//there, and then use that slot to handle replace_container too.
	if(success && spec.replace_container) {
//		user->DeleteItemInInventory(in_combine->container_slot, 0, true);
	}
	if (success)
		parse->EventPlayer(EVENT_COMBINE_SUCCESS, user, spec.name.c_str(), spec.recipe_id);
	else
		parse->EventPlayer(EVENT_COMBINE_FAILURE, user, spec.name.c_str(), spec.recipe_id);
}

SkillType Object::TypeToSkill(uint32 type) {
	SkillType tradeskill = TradeskillUnknown;
	switch (type) {
		case OT_MEDICINEBAG: {
			tradeskill = ALCHEMY;
			break;
		}
		case OT_SEWINGKIT: {
			tradeskill = TAILORING;
			break;
		}
		case OT_FORGE:
		case OT_TEIRDALFORGE:
		case OT_OGGOKFORGE:
		case OT_FIERDALFFORGE:
		case OT_STORMGUARDF:
			case OT_VALEFORGE: {
			tradeskill = BLACKSMITHING;
			break;
		}
		case OT_FLETCHINGKIT: {
			tradeskill = FLETCHING;
			break;
		}
		case OT_BREWBARREL: {
			tradeskill = BREWING;
			break;
		}
		case OT_JEWELERSKIT: {
			tradeskill = JEWELRY_MAKING;
			break;
		}
		case OT_POTTERYWHEEL:
		case OT_KILN: {
			tradeskill = POTTERY;
			break;
		}
		case OT_OVEN: {
			tradeskill = BAKING;
			break;
		}
		case OT_TACKLEBOX: {
			tradeskill = FISHING;
			break;
		}
		case OT_KEYMAKER: { //unknown for now...
			tradeskill = TradeskillUnknown;
			break;
		}
		case OT_TOOLBOX: {
			tradeskill = TINKERING;
			break;
		}
		case OT_WIZARDLEX:
		case OT_MAGELEX:
		case OT_NECROLEX:
		case OT_ENCHLEX: {
			tradeskill = RESEARCH;
			break;
		}
	}
	return(tradeskill);
}

void Client::TradeskillSearchResults(const char *query, unsigned long qlen, unsigned long objtype, unsigned long someid) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!database.RunQuery(query, qlen, errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in TradeskillSearchResults query '%s': %s", query, errbuf);
		return;
	}

	uint8 qcount = 0;

	qcount = mysql_num_rows(result);
	if(qcount < 1) {
		//search gave no results... not an error
		return;
	}
	if(mysql_num_fields(result) != 6) {
		LogFile->write(EQEMuLog::Error, "Error in TradeskillSearchResults query '%s': Invalid column count in result", query);
		return;
	}

	uint8 r;
	for(r = 0; r < qcount; r++) {
		row = mysql_fetch_row(result);
		if(row == nullptr || row[0] == nullptr || row[1] == nullptr || row[2] == nullptr || row[3] == nullptr || row[5] == nullptr)
			continue;
		uint32 recipe = (uint32)atoi(row[0]);
		const char *name = row[1];
		uint32 trivial = (uint32) atoi(row[2]);
		uint32 comp_count = (uint32) atoi(row[3]);
		uint32 tradeskill = (uint16) atoi(row[5]);

		// Skip the recipes that exceed the threshold in skill difference
		// Recipes that have either been made before or were
		// explicitly learned are excempt from that limit
		if (RuleB(Skills, UseLimitTradeskillSearchSkillDiff)) {
			if (((int32)trivial - (int32)GetSkill((SkillType)tradeskill)) > RuleI(Skills, MaxTradeskillSearchSkillDiff)
				&& row[4] == nullptr)
			{
				continue;
			}
		}


		EQApplicationPacket* outapp = new EQApplicationPacket(OP_RecipeReply, sizeof(RecipeReply_Struct));
		RecipeReply_Struct *reply = (RecipeReply_Struct *) outapp->pBuffer;

		reply->object_type = objtype;
		reply->some_id = someid;
		reply->component_count = comp_count;
		reply->recipe_id = recipe;
		reply->trivial = trivial;
		strn0cpy(reply->recipe_name, name, sizeof(reply->recipe_name));
		FastQueuePacket(&outapp);
	}
	mysql_free_result(result);
}

void Client::SendTradeskillDetails(uint32 recipe_id) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *query = 0;

	uint32 qlen = 0;
	uint8 qcount = 0;

	//pull the list of components
	qlen = MakeAnyLenString(&query, "SELECT tre.item_id,tre.componentcount,i.icon,i.Name "
	 " FROM tradeskill_recipe_entries AS tre "
	 " LEFT JOIN items AS i ON tre.item_id = i.id "
	 " WHERE tre.componentcount > 0 AND tre.recipe_id=%u", recipe_id);

	if (!database.RunQuery(query, qlen, errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in SendTradeskillDetails query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return;
	}
	safe_delete_array(query);

	qcount = mysql_num_rows(result);
	if(qcount < 1) {
		LogFile->write(EQEMuLog::Error, "Error in SendTradeskillDetails: no components returned");
		return;
	}
	if(qcount > 10) {
		LogFile->write(EQEMuLog::Error, "Error in SendTradeskillDetails: too many components returned (%u)", qcount);
		return;
	}

	//biggest this packet can ever be:
	// 64 * 10 + 8 * 10 + 4 + 4 * 10 = 764
	char *buf = new char[775];	//dynamic so we can just give it to EQApplicationPacket
	uint8 r,k;

	uint32 *header = (uint32 *) buf;
	//Hell if I know why this is in the wrong byte order....
	*header = htonl(recipe_id);

	char *startblock = buf;
	startblock += sizeof(uint32);

	uint32 *ffff_start = (uint32 *) startblock;
	//fill in the FFFF's as if there were 0 items
	for(r = 0; r < 10; r++) {
		*ffff_start = 0xFFFFFFFF;
		ffff_start++;
	}
	char * datastart = (char *) ffff_start;
	char * cblock = (char *) ffff_start;

	uint32 *itemptr;
	uint32 *iconptr;
	uint32 len;
	uint32 datalen = 0;
	uint8 count = 0;
	for(r = 0; r < qcount; r++) {
		row = mysql_fetch_row(result);

		//watch for references to items which are not in the
		//items table, which the left join will make nullptr...
		if(row[2] == nullptr || row[3] == nullptr) {
			continue;
		}

		uint32 item = (uint32)atoi(row[0]);
		uint8 num = (uint8) atoi(row[1]);


		uint32 icon = (uint32) atoi(row[2]);
		const char *name = row[3];
		len = strlen(name);
		if(len > 63)
			len = 63;

		//Hell if I know why these are in the wrong byte order....
		item = htonl(item);
		icon = htonl(icon);

		//if we get more than 10 items, just start skipping them...
		for(k = 0; k < num && count < 10; k++) {
			itemptr = (uint32 *) cblock;
			cblock += sizeof(uint32);
			datalen += sizeof(uint32);
			iconptr = (uint32 *) cblock;
			cblock += sizeof(uint32);
			datalen += sizeof(uint32);

			*itemptr = item;
			*iconptr = icon;
			strncpy(cblock, name, len);

			cblock[len] = '\0';	//just making sure.
			cblock += len + 1;	//get the null
			datalen += len + 1;	//get the null
			count++;
		}

	}
	mysql_free_result(result);

	//now move the item data over top of the FFFFs
	uint8 dist = sizeof(uint32) * (10 - count);
	startblock += dist;
	memmove(startblock, datastart, datalen);

	uint32 total = sizeof(uint32) + dist + datalen;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RecipeDetails);
	outapp->size = total;
	outapp->pBuffer = (uchar*) buf;
	QueuePacket(outapp);
	DumpPacket(outapp);
	safe_delete(outapp);
}

//returns true on success
bool Client::TradeskillExecute(DBTradeskillRecipe_Struct *spec) {
	if(spec == nullptr)
		return(false);

	uint16 user_skill = GetSkill(spec->tradeskill);
	float chance = 0.0;
	float skillup_modifier = 0.0;
	int16 thirdstat = 0;
	int16 stat_modifier = 15;
	uint16 success_modifier = 0;

	// Rework based on the info on eqtraders.com
	// http://mboards.eqtraders.com/eq/showthread.php?t=22246
	// 09/10/2006 v0.1 (eq4me)
	// 09/11/2006 v0.2 (eq4me)
	// Todo:
	// Implementing AAs
	// Success modifiers based on recipes
	// Skillup modifiers based on the rarity of the ingredients

	// Some tradeskills are more eqal then others. ;-)
	// If you want to customize the stage1 success rate do it here.
	// Remember: skillup_modifier is (float). Lower is better
	switch(spec->tradeskill) {
	case FLETCHING:
	case ALCHEMY:
	case JEWELRY_MAKING:
	case POTTERY:
		skillup_modifier = 4;
		break;
	case BAKING:
	case BREWING:
		skillup_modifier = 3;
		break;
	case RESEARCH:
		skillup_modifier = 1;
		break;
	default:
		skillup_modifier = 2;
		break;
	}

	// Some tradeskills take the higher of one additional stat beside INT and WIS
	// to determine the skillup rate. Additionally these tradeskills do not have an
	// -15 modifier on their statbonus.
	if (spec->tradeskill == FLETCHING || spec->tradeskill == MAKE_POISON) {
		thirdstat = GetDEX();
		stat_modifier = 0;
	} else if (spec->tradeskill == BLACKSMITHING) {
		thirdstat = GetSTR();
		stat_modifier = 0;
	}

	int16 higher_from_int_wis = (GetINT() > GetWIS()) ? GetINT() : GetWIS();
	int16 bonusstat = (higher_from_int_wis > thirdstat) ? higher_from_int_wis : thirdstat;

	std::vector< std::pair<uint32,uint8> >::iterator itr;


	//calculate the base success chance
	// For trivials over 68 the chance is (skill - 0.75*trivial) +51.5
	// For trivial up to 68 the chance is (skill - trivial) + 66
	if (spec->trivial >= 68) {
		chance = (user_skill - (0.75*spec->trivial)) + 51.5;
	} else {
		chance = (user_skill - spec->trivial) + 66;
	}

	int16 over_trivial = (int16)GetRawSkill(spec->tradeskill) - (int16)spec->trivial;

	//handle caps
	if(spec->nofail) {
		chance = 100;	//cannot fail.
		_log(TRADESKILLS__TRACE, "...This combine cannot fail.");
	} else if(over_trivial >= 0) {
		// At reaching trivial the chance goes to 95% going up an additional
		// percent for every 40 skillpoints above the trivial.
		// The success rate is not modified through stats.
		// Mastery AAs are unaccounted for so far.
		// chance_AA = chance + ((100 - chance) * mastery_modifier)
		// But the 95% limit with an additional 1% for every 40 skill points
		// above critical still stands.
		// Mastery modifier is: 10%/25%/50% for rank one/two/three
		chance = 95.0f + (float(user_skill - spec->trivial) / 40.0f);
		Message_StringID(MT_Emote, TRADESKILL_TRIVIAL);
	} else if(chance < 5) {
		// Minimum chance is always 5
		chance = 5;
	} else if(chance > 95) {
		//cap is 95, shouldent reach this before trivial, but just in case.
		chance = 95;
	}

	_log(TRADESKILLS__TRACE, "...Current skill: %d , Trivial: %d , Success chance: %f percent", user_skill , spec->trivial , chance);
	_log(TRADESKILLS__TRACE, "...Bonusstat: %d , INT: %d , WIS: %d , DEX: %d , STR: %d", bonusstat , GetINT() , GetWIS() , GetDEX() , GetSTR());

	float res = MakeRandomFloat(0, 99);
	int AAChance = 0;

	//AA modifiers
	//can we do this with nested switches?
	if(spec->tradeskill == ALCHEMY){
		switch(GetAA(aaAlchemyMastery)){
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	if(spec->tradeskill == JEWELRY_MAKING){
		switch(GetAA(aaJewelCraftMastery)){
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}
	const Item_Struct* item = nullptr;

	if (spec->tradeskill == BLACKSMITHING) {
		switch(GetAA(aaBlacksmithingMastery)) {
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	if (spec->tradeskill == BAKING) {
		switch(GetAA(aaBakingMastery)) {
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	if (spec->tradeskill == BREWING) {
		switch(GetAA(aaBrewingMastery)) {
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	if (spec->tradeskill == FLETCHING) {
		switch(GetAA(aaFletchingMastery2)) {
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	if (spec->tradeskill == POTTERY) {
		switch(GetAA(aaPotteryMastery)) {
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	if (spec->tradeskill == TAILORING) {
		switch(GetAA(aaTailoringMastery)) {
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	if (spec->tradeskill == RESEARCH) {
		switch(GetAA(aaArcaneTongues)) {
		case 1:
			AAChance = 10;
			break;
		case 2:
			AAChance = 25;
			break;
		case 3:
			AAChance = 50;
			break;
		}
	}

	chance = mod_tradeskill_chance(chance, spec);

	if (((spec->tradeskill==75) || GetGM() || (chance > res)) || MakeRandomInt(0, 99) < AAChance){
		success_modifier = 1;

		if(over_trivial < 0)
			CheckIncreaseTradeskill(bonusstat, stat_modifier, skillup_modifier, success_modifier, spec->tradeskill);

		Message_StringID(4,TRADESKILL_SUCCEED,spec->name.c_str());

		_log(TRADESKILLS__TRACE, "Tradeskill success");

		itr = spec->onsuccess.begin();
		while(itr != spec->onsuccess.end() && !spec->quest) {
			//should we check this crap?
			SummonItem(itr->first, itr->second);
			item = database.GetItem(itr->first);
			if (this->GetGroup())
			{
				entity_list.MessageGroup(this,true,MT_Skills,"%s has successfully fashioned %s!",GetName(),item->Name);
			}
			if(RuleB(TaskSystem, EnableTaskSystem))
				UpdateTasksForItem(ActivityTradeSkill, itr->first, itr->second);
			itr++;
		}
		return(true);
	} else {
		success_modifier = 2; // Halves the chance

		if(over_trivial < 0)
			CheckIncreaseTradeskill(bonusstat, stat_modifier, skillup_modifier, success_modifier, spec->tradeskill);

		Message_StringID(MT_Emote,TRADESKILL_FAILED);

		_log(TRADESKILLS__TRACE, "Tradeskill failed");
			if (this->GetGroup())
		{
			entity_list.MessageGroup(this,true,MT_Skills,"%s was unsuccessful in %s tradeskill attempt.",GetName(),this->GetGender() == 0 ? "his" : this->GetGender() == 1 ? "her" : "its");
		}

		itr = spec->onfail.begin();
		while(itr != spec->onfail.end()) {
			//should we check these arguments?
			SummonItem(itr->first, itr->second);
			itr++;
		}

		// Rolls on each item, is possible to return everything
		int SalvageChance = aabonuses.SalvageChance + itembonuses.SalvageChance + spellbonuses.SalvageChance;
		// Skip check if not a normal TS or if a quest recipe these should be nofail, but check amyways
		if(SalvageChance && spec->tradeskill != 75 && !spec->quest) {
			itr = spec->salvage.begin();
			uint8 sc = 0;
			while(itr != spec->salvage.end()) {
				for(sc = 0; sc < itr->second; sc++)
					if(MakeRandomInt(0,99) < SalvageChance)
						SummonItem(itr->first, 1);
				itr++;
			}
		}

	}
	return(false);
}

void Client::CheckIncreaseTradeskill(int16 bonusstat, int16 stat_modifier, float skillup_modifier, uint16 success_modifier, SkillType tradeskill)
{
	uint16 current_raw_skill = GetRawSkill(tradeskill);

	if(!CanIncreaseTradeskill(tradeskill))
		return;	//not allowed to go higher.

	float chance_stage2 = 0;

	//A successfull combine doubles the stage1 chance for an skillup
	//Some tradeskill are harder than others. See above for more.
	float chance_stage1 = (bonusstat - stat_modifier) / (skillup_modifier * success_modifier);

	//In stage2 the only thing that matters is your current unmodified skill.
	//If you want to customize here you probbably need to implement your own
	//formula instead of tweaking the below one.
	if (chance_stage1 > MakeRandomFloat(0, 99)) {
		if (current_raw_skill < 15) {
			//Always succeed
			chance_stage2 = 100;
		} else if (current_raw_skill < 175) {
			//From skill 16 to 174 your chance of success falls linearly from 92% to 13%.
			chance_stage2 = (200 - current_raw_skill) / 2;
		} else {
			//At skill 175, your chance of success falls linearly from 12.5% to 2.5% at skill 300.
			chance_stage2 = 12.5 - (.08 * (current_raw_skill - 175));
		}
	}
	   
	chance_stage2 = mod_tradeskill_skillup(chance_stage2);

	if (chance_stage2 > MakeRandomFloat(0, 99)) {
		//Only if stage1 and stage2 succeeded you get a skillup.
		SetSkill(tradeskill, current_raw_skill + 1);

		if(title_manager.IsNewTradeSkillTitleAvailable(tradeskill, current_raw_skill + 1))
			NotifyNewTitlesAvailable();
	}

	_log(TRADESKILLS__TRACE, "...skillup_modifier: %f , success_modifier: %d , stat modifier: %d", skillup_modifier , success_modifier , stat_modifier);
	_log(TRADESKILLS__TRACE, "...Stage1 chance was: %f percent", chance_stage1);
	_log(TRADESKILLS__TRACE, "...Stage2 chance was: %f percent. 0 percent means stage1 failed", chance_stage2);
}

bool ZoneDatabase::GetTradeRecipe(const ItemInst* container, uint8 c_type, uint32 some_id,
	uint32 char_id, DBTradeskillRecipe_Struct *spec)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *query = 0;
	char buf2[4096];

	uint32 sum = 0;
	uint32 count = 0;
	uint32 qcount = 0;
	uint32 qlen = 0;

	// make where clause segment for container(s)
	char containers[30];
	if (some_id == 0) {
		// world combiner so no item number
		snprintf(containers,29, "= %u", c_type);
	} else {
		// container in inventory
		snprintf(containers,29, "in (%u,%u)", c_type, some_id);
	}

	buf2[0] = '\0';

	//Could prolly watch for stacks in this loop and handle them properly...
	//just increment sum and count accordingly
	bool first = true;
	uint8 i;
	char *pos = buf2;
	for (i=0; i<10; i++) {
		const ItemInst* inst = container->GetItem(i);
		if (inst) {
			const Item_Struct* item = GetItem(inst->GetItem()->ID);
			if (item) {
				if(first) {
					pos += snprintf(pos, 19, "%d", item->ID);
					first = false;
				} else {
					pos += snprintf(pos, 19, ",%d", item->ID);
				}
				sum += item->ID;
				count++;
			}
		}
	}
	*pos = '\0';

	if(count < 1) {
		return(false);	//no items == no recipe
	}

	qlen = MakeAnyLenString(&query, "SELECT tre.recipe_id "
	" FROM tradeskill_recipe_entries AS tre"
	" WHERE ( tre.item_id IN(%s) AND tre.componentcount>0 )"
	" OR ( tre.item_id %s AND tre.iscontainer=1 )"
	" GROUP BY tre.recipe_id HAVING sum(tre.componentcount) = %u"
	" AND sum(tre.item_id * tre.componentcount) = %u", buf2, containers, count, sum);

	if (!RunQuery(query, qlen, errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe search, query: %s", query);
		safe_delete_array(query);
		LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe search, error: %s", errbuf);
		return(false);
	}
	safe_delete_array(query);

	qcount = mysql_num_rows(result);
	if(qcount > 1) {
		//multiple recipes, partial match... do an extra query to get it exact.
		//this happens when combining components for a smaller recipe
		//which is completely contained within another recipe

		first = true;
		pos = buf2;
		for (i = 0; i < qcount; i++) {
			row = mysql_fetch_row(result);
			uint32 recipeid = (uint32)atoi(row[0]);
			if(first) {
				pos += snprintf(pos, 19, "%u", recipeid);
				first = false;
			} else {
				pos += snprintf(pos, 19, ",%u", recipeid);
			}
			//length limit on buf2
			if(i == 214) { //Maximum number of recipe matches (19 * 215 = 4096)
				LogFile->write(EQEMuLog::Error, "GetTradeRecipe warning: Too many matches. Unable to search all recipe entries. Searched %u of %u possible entries.", i + 1, qcount);
				break;
			}
		}

		qlen = MakeAnyLenString(&query, "SELECT tre.recipe_id"
		" FROM tradeskill_recipe_entries AS tre"
		" WHERE tre.recipe_id IN (%s)"
		" GROUP BY tre.recipe_id HAVING sum(tre.componentcount) = %u"
		" AND sum(tre.item_id * tre.componentcount) = %u", buf2, count, sum);

		if (!RunQuery(query, qlen, errbuf, &result)) {
			LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe, re-query: %s", query);
			safe_delete_array(query);
			LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe, error: %s", errbuf);
			return(false);
		}
		safe_delete_array(query);

		qcount = mysql_num_rows(result);
	}

	if(qcount < 1)
		return(false);

	if(qcount > 1)
	{
		//The recipe is not unique, so we need to compare the container were using.

		uint32 containerId = 0;

		if(some_id) { //Standard container
			containerId = some_id;
		}
		else if(c_type) { //World container
			containerId = c_type;
		}
		else { //Invalid container
			return(false);
		}

		qlen = MakeAnyLenString(&query,"SELECT tre.recipe_id FROM tradeskill_recipe_entries as tre WHERE tre.recipe_id IN (%s)"
		" AND tre.item_id = %u;",buf2,containerId);

		if (!RunQuery(query, qlen, errbuf, &result)) {
			LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe, re-query: %s", query);
			safe_delete_array(query);
			LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe, error: %s", errbuf);
			return(false);
		}
		safe_delete_array(query);

		uint32 resultRowTotal = mysql_num_rows(result);

		if(resultRowTotal == 0) { //Recipe contents matched more than 1 recipe, but not in this container
			LogFile->write(EQEMuLog::Error, "Combine error: Incorrect container is being used!");
			return(false);
		}
		if(resultRowTotal > 1) { //Recipe contents matched more than 1 recipe in this container
			LogFile->write(EQEMuLog::Error, "Combine error: Recipe is not unique! %u matches found for container %u. Continuing with first recipe match.", resultRowTotal, containerId);
		}
	}

	row = mysql_fetch_row(result);
	uint32 recipe_id = (uint32)atoi(row[0]);
	mysql_free_result(result);

	//Right here we verify that we actually have ALL of the tradeskill components..
	//instead of part which is possible with experimentation.
	//This is here because something's up with the query above.. it needs to be rethought out
	bool has_components = true;
	char TSerrbuf[MYSQL_ERRMSG_SIZE];
	char *TSquery = 0;
	MYSQL_RES *TSresult;
	MYSQL_ROW TSrow;
	if (RunQuery(TSquery, MakeAnyLenString(&TSquery, "SELECT item_id, componentcount from tradeskill_recipe_entries where recipe_id=%i AND componentcount > 0", recipe_id), TSerrbuf, &TSresult)) {
		while((TSrow = mysql_fetch_row(TSresult))!=nullptr) {
			int ccnt = 0;
			for(int x = 0; x < 10; x++){
				const ItemInst* inst = container->GetItem(x);
				if(inst){
					const Item_Struct* item = GetItem(inst->GetItem()->ID);
					if (item) {
						if(item->ID == atoi(TSrow[0])){
							ccnt++;
						}
					}
				}
			}
			if(ccnt != atoi(TSrow[1]))
				has_components = false;
		}
		mysql_free_result(TSresult);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in tradeskill verify query: '%s': %s", TSquery, TSerrbuf);
	}
	safe_delete_array(TSquery);
	if(has_components == false){

		return false;
	}

	return(GetTradeRecipe(recipe_id, c_type, some_id, char_id, spec));
}

bool ZoneDatabase::GetTradeRecipe(uint32 recipe_id, uint8 c_type, uint32 some_id,
	uint32 char_id, DBTradeskillRecipe_Struct *spec)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *query = 0;

	uint32 qcount = 0;
	uint32 qlen;

	// make where clause segment for container(s)
	char containers[30];
	if (some_id == 0) {
		// world combiner so no item number
		snprintf(containers,29, "= %u", c_type);
	} else {
		// container in inventory
		snprintf(containers,29, "in (%u,%u)", c_type, some_id);
	}

	qlen = MakeAnyLenString(&query, "SELECT tr.id, tr.tradeskill, tr.skillneeded,"
	" tr.trivial, tr.nofail, tr.replace_container, tr.name, tr.must_learn, tr.quest, crl.madecount"
	" FROM tradeskill_recipe AS tr inner join tradeskill_recipe_entries as tre"
	" ON tr.id = tre.recipe_id"
	" LEFT JOIN (SELECT recipe_id, madecount from char_recipe_list WHERE char_id = %u) AS crl "
	" ON tr.id = crl.recipe_id "
	" WHERE tr.id = %lu AND tre.item_id %s"
	" GROUP BY tr.id", char_id, (unsigned long)recipe_id, containers);

	if (!RunQuery(query, qlen, errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe, query: %s", query);
		safe_delete_array(query);
		LogFile->write(EQEMuLog::Error, "Error in GetTradeRecipe, error: %s", errbuf);
		return(false);
	}
	safe_delete_array(query);

	qcount = mysql_num_rows(result);
	if(qcount != 1) {
		//just not found i guess..
		return(false);
	}

	row = mysql_fetch_row(result);
	spec->tradeskill			= (SkillType)atoi(row[1]);
	spec->skill_needed		= (int16)atoi(row[2]);
	spec->trivial			= (uint16)atoi(row[3]);
	spec->nofail			= atoi(row[4]) ? true : false;
	spec->replace_container	= atoi(row[5]) ? true : false;
	spec->name = row[6];
	spec->must_learn = (uint8)atoi(row[7]);
	spec->quest = atoi(row[8]) ? true : false;
	if (row[9] == nullptr) {
		spec->has_learnt = false;
		spec->madecount = 0;
	} else {
		spec->has_learnt = true;
		spec->madecount = (uint32)atoul(row[9]);
	}
	spec->recipe_id = recipe_id;
	mysql_free_result(result);

	//Pull the on-success items...
	qlen = MakeAnyLenString(&query, "SELECT item_id,successcount FROM tradeskill_recipe_entries"
	 " WHERE successcount>0 AND recipe_id=%u", recipe_id);

	if (!RunQuery(query, qlen, errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in GetTradeRecept success query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return(false);
	}
	safe_delete_array(query);

	qcount = mysql_num_rows(result);
	if(qcount < 1) {
		LogFile->write(EQEMuLog::Error, "Error in GetTradeRecept success: no success items returned");
		return(false);
	}
	uint8 r;
	spec->onsuccess.clear();
	for(r = 0; r < qcount; r++) {
		row = mysql_fetch_row(result);
		uint32 item = (uint32)atoi(row[0]);
		uint8 num = (uint8) atoi(row[1]);
		spec->onsuccess.push_back(std::pair<uint32,uint8>(item, num));
	}
	mysql_free_result(result);

	//Pull the on-fail items...
	qlen = MakeAnyLenString(&query, "SELECT item_id,failcount FROM tradeskill_recipe_entries"
	 " WHERE failcount>0 AND recipe_id=%u", recipe_id);

	spec->onfail.clear();
	if (RunQuery(query, qlen, errbuf, &result)) {

		qcount = mysql_num_rows(result);
		uint8 r;
		for(r = 0; r < qcount; r++) {
			row = mysql_fetch_row(result);
			uint32 item = (uint32)atoi(row[0]);
			uint8 num = (uint8) atoi(row[1]);
			spec->onfail.push_back(std::pair<uint32,uint8>(item, num));
		}
		mysql_free_result(result);
	}

	// Pull the salvage list
	qlen = MakeAnyLenString(&query, "SELECT item_id,salvagecount FROM tradeskill_recipe_entries WHERE salvagecount>0 AND recipe_id=%u", recipe_id);

	spec->salvage.clear();
	// Don't bother with the query if TS is nofail
	if (!spec->nofail && RunQuery(query, qlen, errbuf, &result)) {
		qcount = mysql_num_rows(result);
		uint8 r;
		for(r = 0; r < qcount; r++) {
			row = mysql_fetch_row(result);
			uint32 item = (uint32)atoi(row[0]);
			uint8 num = (uint8)atoi(row[1]);
			spec->salvage.push_back(std::pair<uint32,uint8>(item, num));
		}
		mysql_free_result(result);
	}

	safe_delete_array(query);

	return(true);
}

void ZoneDatabase::UpdateRecipeMadecount(uint32 recipe_id, uint32 char_id, uint32 madecount)
{
	char *query = 0;
	uint32 qlen;
	char errbuf[MYSQL_ERRMSG_SIZE];

	qlen = MakeAnyLenString(&query, "INSERT INTO char_recipe_list "
		" SET recipe_id = %u, char_id = %u, madecount = %u "
		" ON DUPLICATE KEY UPDATE madecount = %u;"
	, recipe_id, char_id, madecount, madecount);

	if (!RunQuery(query, qlen, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in UpdateRecipeMadecount query '%s': %s", query, errbuf);
	}
	safe_delete_array(query);
}

void Client::LearnRecipe(uint32 recipeID)
{
	char *query = 0;
	uint32 qlen;
	uint32 qcount = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;

	qlen = MakeAnyLenString(&query, "SELECT tr.name, crl.madecount "
		" FROM tradeskill_recipe as tr "
		" LEFT JOIN (SELECT recipe_id, madecount FROM char_recipe_list WHERE char_id = %u) AS crl "
		" ON tr.id = crl.recipe_id "
		" WHERE tr.id = %u ;", CharacterID(), recipeID);

	if (!database.RunQuery(query, qlen, errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in Client::LearnRecipe query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return;
	}

	qcount = mysql_num_rows(result);
	if (qcount != 1) {
		LogFile->write(EQEMuLog::Normal, "Client::LearnRecipe - RecipeID: %d had %d occurences.", recipeID, qcount);
		mysql_free_result(result);
		safe_delete_array(query);
		return;
	}
	safe_delete_array(query);

	row = mysql_fetch_row(result);

	if (row != nullptr && row[0] != nullptr) {
		// Only give Learn message if character doesn't know the recipe
		if (row[1] == nullptr) {
			Message_StringID(4, TRADESKILL_LEARN_RECIPE, row[0]);
			// Actually learn the recipe now
			qlen = MakeAnyLenString(&query, "INSERT INTO char_recipe_list "
				" SET recipe_id = %u, char_id = %u, madecount = 0 "
				" ON DUPLICATE KEY UPDATE madecount = madecount;"
			, recipeID, CharacterID());

			if (!database.RunQuery(query, qlen, errbuf)) {
				LogFile->write(EQEMuLog::Error, "Error in LearnRecipe query '%s': %s", query, errbuf);
			}
			safe_delete_array(query);
		}
	}

	mysql_free_result(result);

}

bool Client::CanIncreaseTradeskill(SkillType tradeskill) {
	uint32 rawskill = GetRawSkill(tradeskill);
	uint16 maxskill = MaxSkill(tradeskill);

	if (rawskill >= maxskill) //Max skill sanity check
		return false;

	uint8 Baking	= (GetRawSkill(BAKING) > 200) ? 1 : 0;
	uint8 Smithing	= (GetRawSkill(BLACKSMITHING) > 200) ? 1 : 0;
	uint8 Brewing	= (GetRawSkill(BREWING) > 200) ? 1 : 0;
	uint8 Fletching	= (GetRawSkill(FLETCHING) > 200) ? 1 : 0;
	uint8 Jewelry	= (GetRawSkill(JEWELRY_MAKING) > 200) ? 1 : 0;
	uint8 Pottery	= (GetRawSkill(POTTERY) > 200) ? 1 : 0;
	uint8 Tailoring	= (GetRawSkill(TAILORING) > 200) ? 1 : 0;
	uint8 SkillTotal = Baking + Smithing + Brewing + Fletching + Jewelry + Pottery + Tailoring; //Tradeskills above 200
	uint32 aaLevel	= GetAA(aaNewTanaanCraftingMastery); //New Tanaan AA: Each level allows an additional tradeskill above 200 (first one is free)

	switch (tradeskill) {
		case BAKING:
		case BLACKSMITHING:
		case BREWING:
		case FLETCHING:
		case JEWELRY_MAKING:
		case POTTERY:
		case TAILORING:
			if (aaLevel == 6)
				break; //Maxed AA
			if (SkillTotal == 0)
				break; //First tradeskill freebie
			if ((SkillTotal == (aaLevel + 1)) && (rawskill > 200))
				break; //One of the tradeskills already allowed to go over 200
			if ((SkillTotal >= (aaLevel + 1)) && (rawskill >= 200))
				return false; //One or more tradeskills already at or beyond limit
				break;
		default:
			break; //Other skills unchecked and ability to increase assumed true
	}
	return true;
}
