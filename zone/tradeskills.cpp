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

#include "../common/global_define.h"

#include <stdlib.h>
#include <list>

#ifndef WIN32
#include <netinet/in.h>	//for htonl
#endif

#include "../common/rulesys.h"
#include "../common/string_util.h"

#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "titles.h"
#include "zonedb.h"
#include "zone_store.h"
#include "../common/repositories/character_recipe_list_repository.h"
#include "../common/repositories/tradeskill_recipe_repository.h"

extern QueryServ* QServ;

static const EQ::skills::SkillType TradeskillUnknown = EQ::skills::Skill1HBlunt; /* an arbitrary non-tradeskill */

void Object::HandleAugmentation(Client* user, const AugmentItem_Struct* in_augment, Object *worldo)
{
	if (!user || !in_augment) {
		LogError("Client or AugmentItem_Struct not set in Object::HandleAugmentation");
		return;
	}

	EQ::ItemInstance* container = nullptr;

	if (worldo) {
		container = worldo->m_inst;
	} else { // Check to see if they have an inventory container type 53 that is used for this.
		EQ::InventoryProfile& user_inv = user->GetInv();
		EQ::ItemInstance* inst = nullptr;

		inst = user_inv.GetItem(in_augment->container_slot);
		if (inst) {
			const EQ::ItemData* item = inst->GetItem();
			if (item && inst->IsType(EQ::item::ItemClassBag) && item->BagType == EQ::item::BagTypeAugmentationSealer) { // We have found an appropriate inventory augmentation sealer
				container = inst;

				// Verify that no more than two items are in container to guarantee no inadvertant wipes.
				uint8 items_found = 0;
				for (uint8 i = EQ::invbag::SLOT_BEGIN; i < EQ::invtype::WORLD_SIZE; i++) {
					const EQ::ItemInstance* inst = container->GetItem(i);
					if (inst) {
						items_found++;
					}
				}

				if (items_found < 2) {
					user->Message(Chat::Red, "Error: Too few items in augmentation container.");
					return;
				} else if (items_found > 2) {
					user->Message(Chat::Red, "Error: Too many items in augmentation container.");
					return;
				}
			}
		}
	}

	if(!container) {
		LogError("Player tried to augment an item without a container set");
		user->Message(Chat::Red, "Error: This item is not a container!");
		return;
	}

	EQ::ItemInstance *tobe_auged = nullptr, *auged_with = nullptr;
	int8 slot = -1;

	if (container->GetItem(0) && container->GetItem(1)) { // Verify 2 items in the augmentation device
		// Verify 1 item is augmentable and the other is not
		if (container->GetItem(0)->IsAugmentable() && !container->GetItem(1)->IsAugmentable()) {
			tobe_auged = container->GetItem(0);
			auged_with = container->GetItem(1);
		} else if (!container->GetItem(0)->IsAugmentable() && container->GetItem(1)->IsAugmentable()) {
			tobe_auged = container->GetItem(1);
			auged_with = container->GetItem(0);
		} else {
			// Either 2 augmentable items found or none found
			// This should never occur due to client restrictions, but prevent in case of a hack
			user->Message(Chat::Red, "Error: There must be 1 augmentable item in the sealer.");
			return;
		}
	} else { // This happens if the augment button is clicked more than once quickly while augmenting
		if (!container->GetItem(0))	{
			user->Message(Chat::Red, "Error: No item in the first slot of sealer.");
		}

		if (!container->GetItem(1)) {
			user->Message(Chat::Red, "Error: No item in the second slot of sealer.");
		}
		return;
	}

	if (!RuleB(Inventory, AllowMultipleOfSameAugment) && tobe_auged->ContainsAugmentByID(auged_with->GetID())) {
		user->Message(Chat::Red, "Error: Cannot put multiple of the same augment in an item.");
		return;
	}

	bool delete_items = false;

	EQ::ItemInstance *item_one_to_push = nullptr, *item_two_to_push = nullptr;

	if (in_augment->augment_slot == -1) { // Adding augment
		if (
			((slot = tobe_auged->AvailableAugmentSlot(auged_with->GetAugmentType())) != -1) &&
			tobe_auged->AvailableWearSlot(auged_with->GetItem()->Slots)
		) {
			tobe_auged->PutAugment(slot, *auged_with);

			EQ::ItemInstance *aug = tobe_auged->GetAugment(slot);
			if(aug) {
				std::vector<EQ::Any> args;
				args.push_back(aug);
				parse->EventItem(EVENT_AUGMENT_ITEM, user, tobe_auged, nullptr, "", slot, &args);

				args.assign(1, tobe_auged);
				parse->EventItem(EVENT_AUGMENT_INSERT, user, aug, nullptr, "", slot, &args);
			}

			item_one_to_push = tobe_auged->Clone();
			delete_items = true;
		} else {
			user->Message(Chat::Red, "Error: No available slot for augment.");
		}
	} else {
		EQ::ItemInstance *old_aug = nullptr;
		bool is_solvent = auged_with->GetItem()->ItemType == EQ::item::ItemTypeAugmentationSolvent;
		if (!is_solvent && auged_with->GetItem()->ItemType != EQ::item::ItemTypeAugmentationDistiller) {
			LogError("Player tried to remove an augment without a solvent or distiller");
			user->Message(Chat::Red, "Error: Missing an augmentation solvent or distiller for removing this augment.");

			return;
		}

		EQ::ItemInstance *aug = tobe_auged->GetAugment(in_augment->augment_slot);
		if (aug) {
			if (!is_solvent && auged_with->GetItem()->ID != aug->GetItem()->AugDistiller) {
				LogError("Player tried to safely remove an augment with the wrong distiller (item [{}] vs expected [{}])", auged_with->GetItem()->ID, aug->GetItem()->AugDistiller);
				user->Message(Chat::Red, "Error: Wrong augmentation distiller for safely removing this augment.");
				return;
			}
			std::vector<EQ::Any> args;
			args.push_back(aug);
			parse->EventItem(EVENT_UNAUGMENT_ITEM, user, tobe_auged, nullptr, "", slot, &args);

			args.assign(1, tobe_auged);
			args.push_back(&is_solvent);

			parse->EventItem(EVENT_AUGMENT_REMOVE, user, aug, nullptr, "", slot, &args);
		}

		if (is_solvent) {
			tobe_auged->DeleteAugment(in_augment->augment_slot);
		} else {
			old_aug = tobe_auged->RemoveAugment(in_augment->augment_slot);
		}

		item_one_to_push = tobe_auged->Clone();
		if (old_aug) {
			item_two_to_push = old_aug->Clone();
		}

		delete_items = true;
	}

	if (delete_items) {
		if (worldo) {
			container->Clear();
			auto outapp = new EQApplicationPacket(OP_ClearObject, sizeof(ClearObject_Struct));
			ClearObject_Struct *cos = (ClearObject_Struct *)outapp->pBuffer;
			cos->Clear = 1;
			user->QueuePacket(outapp);
			safe_delete(outapp);
			database.DeleteWorldContainer(worldo->m_id, zone->GetZoneID());
		} else { // Delete items in our inventory container...
			for (uint8 i = EQ::invbag::SLOT_BEGIN; i < EQ::invtype::WORLD_SIZE; i++) {
				const EQ::ItemInstance* inst = container->GetItem(i);
				if (inst) {
					user->DeleteItemInInventory(EQ::InventoryProfile::CalcSlotId(in_augment->container_slot, i), 0, true);
				}
			}

			container->Clear(); // Explicitly mark container as cleared.
		}
	}

	// Must push items after the items in inventory are deleted - necessary due to lore items...
	if (item_one_to_push) {
		user->PushItemOnCursor(*item_one_to_push, true);
	}

	if (item_two_to_push) {
		user->PushItemOnCursor(*item_two_to_push, true);
	}
}

// Perform tradeskill combine
void Object::HandleCombine(Client* user, const NewCombine_Struct* in_combine, Object *worldo)
{
	if (!user || !in_combine) {
		LogError("Client or NewCombine_Struct not set in Object::HandleCombine");
		return;
	}

	LogTradeskills(
		"[HandleCombine] container_slot [{}] guildtribute_slot [{}]",
		in_combine->container_slot,
		in_combine->guildtribute_slot
	);

	EQ::InventoryProfile &user_inv  = user->GetInv();
	PlayerProfile_Struct    &user_pp   = user->GetPP();
	EQ::ItemInstance     *container = nullptr;
	EQ::ItemInstance     *inst      = nullptr;

	uint8  c_type         = 0xE8;
	uint32 some_id        = 0;
	bool   worldcontainer = false;


	if (in_combine->container_slot == EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
		if(!worldo) {
			user->Message(
				Chat::Red,
				"Error: Server is not aware of the tradeskill container you are attempting to use"
			);
			return;
		}
		c_type         = worldo->m_type;
		inst           = worldo->m_inst;
		worldcontainer = true;
		// if we're a world container with an item, use that too
		if (inst) {
			const EQ::ItemData *item = inst->GetItem();
			if (item) {
				some_id = item->ID;
			}
		}
	}
	else {
		inst = user_inv.GetItem(in_combine->container_slot);
		if (inst) {
			const EQ::ItemData* item = inst->GetItem();
			if (item && inst->IsType(EQ::item::ItemClassBag)) {
				c_type = item->BagType;
				some_id = item->ID;
			}
		}
	}

	if (!inst || !inst->IsType(EQ::item::ItemClassBag)) {
		user->Message(Chat::Red, "Error: Server does not recognize specified tradeskill container");
		return;
	}

	container = inst;
	if (container->GetItem() && container->GetItem()->BagType == EQ::item::BagTypeTransformationmold) {
		const EQ::ItemInstance *inst    = container->GetItem(0);
		bool                      AllowAll = RuleB(Inventory, AllowAnyWeaponTransformation);
		if (inst && EQ::ItemInstance::CanTransform(inst->GetItem(), container->GetItem(), AllowAll)) {
			const EQ::ItemData *new_weapon = inst->GetItem();
			user->DeleteItemInInventory(EQ::InventoryProfile::CalcSlotId(in_combine->container_slot, 0), 0, true);
			container->Clear();
			user->SummonItem(
				new_weapon->ID,
				inst->GetCharges(),
				inst->GetAugmentItemID(0),
				inst->GetAugmentItemID(1),
				inst->GetAugmentItemID(2),
				inst->GetAugmentItemID(3),
				inst->GetAugmentItemID(4),
				inst->GetAugmentItemID(5),
				inst->IsAttuned(),
				EQ::invslot::slotCursor,
				container->GetItem()->Icon,
				atoi(container->GetItem()->IDFile + 2)
			);

			user->MessageString(Chat::LightBlue, TRANSFORM_COMPLETE, inst->GetItem()->Name);
			if (RuleB(Inventory, DeleteTransformationMold)) {
				user->DeleteItemInInventory(in_combine->container_slot, 0, true);
			}
		}
		else if (inst) {
			user->MessageString(Chat::LightBlue, TRANSFORM_FAILED, inst->GetItem()->Name);
		}
		auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);

		LogTradeskills(
			"[HandleCombine] inst_item [{}] container_item [{}]",
			inst->GetItem()->ID,
			container->GetItem()->ID
		);

		return;
	}

	if (container->GetItem() && container->GetItem()->BagType == EQ::item::BagTypeDetransformationmold) {
		LogTradeskillsDetail("[HandleCombine] Check 1");

		const EQ::ItemInstance* inst = container->GetItem(0);
		if (inst && inst->GetOrnamentationIcon() && inst->GetOrnamentationIcon()) {
			const EQ::ItemData* new_weapon = inst->GetItem();
			user->DeleteItemInInventory(EQ::InventoryProfile::CalcSlotId(in_combine->container_slot, 0), 0, true);
			container->Clear();
			user->SummonItem(new_weapon->ID, inst->GetCharges(), inst->GetAugmentItemID(0), inst->GetAugmentItemID(1), inst->GetAugmentItemID(2), inst->GetAugmentItemID(3), inst->GetAugmentItemID(4), inst->GetAugmentItemID(5), inst->IsAttuned(), EQ::invslot::slotCursor, 0, 0);
			user->MessageString(Chat::LightBlue, TRANSFORM_COMPLETE, inst->GetItem()->Name);
		}
		else if (inst) {
			user->MessageString(Chat::LightBlue, DETRANSFORM_FAILED, inst->GetItem()->Name);
		}
		auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	DBTradeskillRecipe_Struct spec;

	if (parse->EventPlayer(EVENT_COMBINE, user, std::to_string(in_combine->container_slot), 0) == 1) {
		auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	if (!content_db.GetTradeRecipe(container, c_type, some_id, user->CharacterID(), &spec)) {

		LogTradeskillsDetail("[HandleCombine] Check 2");

		user->MessageString(Chat::Emote,TRADESKILL_NOCOMBINE);
		auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
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
	if ((spec.must_learn & 0xF) == 1 && !spec.has_learnt) {

		LogTradeskillsDetail("[HandleCombine] Check 3");

		// Made up message for the client. Just giving a DNC is the other option.
		user->Message(Chat::LightBlue, "You need to learn how to combine these first.");
		auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}
	// Character does not have the required skill.
	if(spec.skill_needed > 0 && user->GetSkill(spec.tradeskill) < spec.skill_needed ) {

		LogTradeskillsDetail("Check 4");

		// Notify client.
		user->Message(Chat::LightBlue, "You are not skilled enough.");
		auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	//changing from a switch to string of if's since we don't need to iterate through all of the skills in the SkillType enum
	if (spec.tradeskill == EQ::skills::SkillAlchemy) {
		if (user_pp.class_ != SHAMAN) {
			user->Message(Chat::Red, "This tradeskill can only be performed by a shaman.");
			return;
		}
		else if (user_pp.level < MIN_LEVEL_ALCHEMY) {
			user->Message(Chat::Red, "You cannot perform alchemy until you reach level %i.", MIN_LEVEL_ALCHEMY);
			return;
		}
	}
	else if (spec.tradeskill == EQ::skills::SkillTinkering) {
		if (user_pp.race != GNOME) {
			user->Message(Chat::Red, "Only gnomes can tinker.");
			return;
		}
	}
	else if (spec.tradeskill == EQ::skills::SkillMakePoison) {
		if (user_pp.class_ != ROGUE) {
			user->Message(Chat::Red, "Only rogues can mix poisons.");
			return;
		}
	}

	// final check for any additional quest requirements .. "check_zone" in this case - exported as variable [validate_type]
	std::string export_string = fmt::format("check_zone {}", zone->GetZoneID());
	if (parse->EventPlayer(EVENT_COMBINE_VALIDATE, user, export_string, spec.recipe_id) != 0) {
		user->Message(Chat::Emote, "You cannot make this combine because the location requirement has not been met.");
		auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	// Send acknowledgement packets to client
	auto outapp = new EQApplicationPacket(OP_TradeSkillCombine, 0);
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
		for (uint8 i = EQ::invbag::SLOT_BEGIN; i < EQ::invtype::WORLD_SIZE; i++) {
			const EQ::ItemInstance* inst = container->GetItem(i);
			if (inst) {
				user->DeleteItemInInventory(EQ::InventoryProfile::CalcSlotId(in_combine->container_slot, i), 0, true);
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
			user->MessageString(Chat::LightBlue, TRADESKILL_LEARN_RECIPE, spec.name.c_str());
		}
		database.UpdateRecipeMadecount(spec.recipe_id, user->CharacterID(), spec.madecount+1);
	}

	// Replace the container on success if required.
	//

	if (success && spec.replace_container) {
		if (worldcontainer) {
			//should report this error, but we dont have the recipe ID, so its not very useful
			LogError("Replace container combine executed in a world container");
		}
		else {
			user->DeleteItemInInventory(in_combine->container_slot, 0, true);
		}
	}

	if (success) {
		parse->EventPlayer(EVENT_COMBINE_SUCCESS, user, spec.name, spec.recipe_id);
	} else {
		parse->EventPlayer(EVENT_COMBINE_FAILURE, user, spec.name, spec.recipe_id);
	}
}

void Object::HandleAutoCombine(Client* user, const RecipeAutoCombine_Struct* rac) {

	//get our packet ready, gotta send one no matter what...
	auto outapp = new EQApplicationPacket(OP_RecipeAutoCombine, sizeof(RecipeAutoCombine_Struct));
	RecipeAutoCombine_Struct *outp = (RecipeAutoCombine_Struct *)outapp->pBuffer;
	outp->object_type = rac->object_type;
	outp->some_id = rac->some_id;
	outp->unknown1 = rac->unknown1;
	outp->recipe_id = rac->recipe_id;
	outp->reply_code = 0xFFFFFFF5;	//default fail.


	//ask the database for the recipe to make sure it exists...
	DBTradeskillRecipe_Struct spec;
	if (!content_db.GetTradeRecipe(rac->recipe_id, rac->object_type, rac->some_id, user->CharacterID(), &spec)) {
		LogError("Unknown recipe for HandleAutoCombine: [{}]\n", rac->recipe_id);
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	// Character hasn't learnt the recipe yet.
	// This shouldn't happen.
	if ((spec.must_learn&0xf) && !spec.has_learnt) {
		// Made up message for the client. Just giving a DNC is the other option.
		user->Message(Chat::LightBlue, "You need to learn how to combine these first.");
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	// Character does not have the required skill.
	if (spec.skill_needed > 0 && user->GetSkill(spec.tradeskill) < spec.skill_needed) {
		// Notify client.
		user->Message(Chat::Red, "You are not skilled enough.");
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

    //pull the list of components
	std::string query = StringFormat("SELECT tre.item_id, tre.componentcount "
                                    "FROM tradeskill_recipe_entries AS tre "
                                    "WHERE tre.componentcount > 0 AND tre.recipe_id = %u",
                                    rac->recipe_id);
    auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	if(results.RowCount() < 1) {
		LogError("Error in HandleAutoCombine: no components returned");
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	if(results.RowCount() > 10) {
		LogError("Error in HandleAutoCombine: too many components returned ([{}])", results.RowCount());
		user->QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	uint32 items[10];
	memset(items, 0, sizeof(items));
	uint8 counts[10];
	memset(counts, 0, sizeof(counts));

	//search for all the items in their inventory
	EQ::InventoryProfile& user_inv = user->GetInv();
	uint8 count = 0;
	uint8 needcount = 0;

	std::list<int> MissingItems;

    uint8 needItemIndex = 0;
	for (auto row = results.begin(); row != results.end(); ++row, ++needItemIndex) {
		uint32 item = (uint32)atoi(row[0]);
		uint8 num = (uint8) atoi(row[1]);

		needcount += num;

		//because a HasItem on items with num > 1 only returns the
		//last-most slot... the results of this are useless to us
		//when we go to delete them because we cannot assume it is in a single stack.
		if (user_inv.HasItem(item, num, invWherePersonal) != INVALID_INDEX)
			count += num;
		else
			MissingItems.push_back(item);

		//dont start deleting anything until we have found it all.
		items[needItemIndex] = item;
		counts[needItemIndex] = num;
	}

	//make sure we found it all...
	if(count != needcount)
	{
		user->QueuePacket(outapp);

		safe_delete(outapp);

		user->MessageString(Chat::Skills, TRADESKILL_MISSING_COMPONENTS);

		for (auto it = MissingItems.begin(); it != MissingItems.end(); ++it) {
			const EQ::ItemData* item = database.GetItem(*it);

			if(item)
				user->MessageString(Chat::Skills, TRADESKILL_MISSING_ITEM, item->Name);
		}

		return;
	}

	//now we know they have everything...

	//remove all the items from the players inventory, with updates...
	int16 slot;
	for(uint8 r = 0; r < results.RowCount(); r++) {
		if(items[r] == 0 || counts[r] == 0)
			continue;	//skip empties, could prolly break here

		//we have to loop here to delete 1 at a time in case its in multiple stacks.
		for(uint8 k = 0; k < counts[r]; k++) {
			slot = user_inv.HasItem(items[r], 1, invWherePersonal);
			if (slot == INVALID_INDEX) {
				//WTF... I just checked this above, but just to be sure...
				//we cant undo the previous deletes without a lot of work.
				//so just call it quits, this shouldent ever happen anyways.
				user->QueuePacket(outapp);
				safe_delete(outapp);
				return;
			}

			const EQ::ItemInstance* inst = user_inv.GetItem(slot);

			if (inst && !inst->IsStackable())
				user->DeleteItemInInventory(slot, 0, true);
			else
				user->DeleteItemInInventory(slot, 1, true);
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
			user->MessageString(Chat::LightBlue, TRADESKILL_LEARN_RECIPE, spec.name.c_str());
		}
		database.UpdateRecipeMadecount(spec.recipe_id, user->CharacterID(), spec.madecount+1);
	}


	//TODO: find in-pack containers in inventory, make sure they are really
	//there, and then use that slot to handle replace_container too.
	if(success && spec.replace_container) {
//		user->DeleteItemInInventory(in_combine->container_slot, 0, true);
	}

	if (success) {
		parse->EventPlayer(EVENT_COMBINE_SUCCESS, user, spec.name, spec.recipe_id);
	} else {
		parse->EventPlayer(EVENT_COMBINE_FAILURE, user, spec.name, spec.recipe_id);
	}
}

EQ::skills::SkillType Object::TypeToSkill(uint32 type)
{
	switch(type) { // grouped and ordered by SkillUseTypes name - new types need to be verified for proper SkillUseTypes and use
/*SkillAlchemy*/
	case EQ::item::BagTypeMedicineBag:
		return EQ::skills::SkillAlchemy;

/*SkillBaking*/
	//case EQ::item::BagTypeMixingBowl: // No idea...
	case EQ::item::BagTypeOven:
		return EQ::skills::SkillBaking;

/*SkillBlacksmithing*/
	case EQ::item::BagTypeForge:
	//case EQ::item::BagTypeKoadaDalForge:
	case EQ::item::BagTypeTeirDalForge:
	case EQ::item::BagTypeOggokForge:
	case EQ::item::BagTypeStormguardForge:
	//case EQ::item::BagTypeAkanonForge:
	//case EQ::item::BagTypeNorthmanForge:
	//case EQ::item::BagTypeCabilisForge:
	//case EQ::item::BagTypeFreeportForge:
	//case EQ::item::BagTypeRoyalQeynosForge:
	//case EQ::item::BagTypeTrollForge:
	case EQ::item::BagTypeFierDalForge:
	case EQ::item::BagTypeValeForge:
	//case EQ::item::BagTypeErudForge:
	//case EQ::item::BagTypeGuktaForge:
		return EQ::skills::SkillBlacksmithing;

/*SkillBrewing*/
	//case EQ::item::BagTypeIceCreamChurn: // No idea...
	case EQ::item::BagTypeBrewBarrel:
		return EQ::skills::SkillBrewing;

/*SkillFishing*/
	case EQ::item::BagTypeTackleBox:
		return EQ::skills::SkillFishing;

/*SkillFletching*/
	case EQ::item::BagTypeFletchingKit:
	//case EQ::item::BagTypeFierDalFletchingKit:
		return EQ::skills::SkillFletching;

/*SkillJewelryMaking*/
	case EQ::item::BagTypeJewelersKit:
		return EQ::skills::SkillJewelryMaking;

/*SkillMakePoison*/
	// This is a guess and needs to be verified... (Could be SkillAlchemy)
	//case EQ::item::BagTypeMortar:
		// return SkillMakePoison;

/*SkillPottery*/
	case EQ::item::BagTypePotteryWheel:
	case EQ::item::BagTypeKiln:
	//case EQ::item::BagTypeIksarPotteryWheel:
		return EQ::skills::SkillPottery;

/*SkillResearch*/
	//case EQ::item::BagTypeLexicon:
	case EQ::item::BagTypeWizardsLexicon:
	case EQ::item::BagTypeMagesLexicon:
	case EQ::item::BagTypeNecromancersLexicon:
	case EQ::item::BagTypeEnchantersLexicon:
	//case EQ::item::BagTypeConcordanceofResearch:
		return EQ::skills::SkillResearch;

/*SkillTailoring*/
	case EQ::item::BagTypeSewingKit:
	//case EQ::item::BagTypeHalflingTailoringKit:
	//case EQ::item::BagTypeErudTailoringKit:
	//case EQ::item::BagTypeFierDalTailoringKit:
		return EQ::skills::SkillTailoring;

/*SkillTinkering*/
	case EQ::item::BagTypeToolBox:
		return EQ::skills::SkillTinkering;

/*Undefined*/
	default:
		return TradeskillUnknown;
	}
}

void Client::SendTradeskillSearchResults(
	const std::string &query,
	unsigned long objtype,
	unsigned long someid
)
{
	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	if (results.RowCount() < 1) {
		return;
	}

	auto character_learned_recipe_list = CharacterRecipeListRepository::GetLearnedRecipeList(CharacterID());

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (row == nullptr || row[0] == nullptr || row[1] == nullptr || row[2] == nullptr || row[3] == nullptr ||
			row[4] == nullptr || row[5] == nullptr) {
			continue;
		}

		uint32     recipe_id  = (uint32) atoi(row[0]);
		const char *name      = row[1];
		uint32     trivial    = (uint32) atoi(row[2]);
		uint32     comp_count = (uint32) atoi(row[3]);
		uint32     tradeskill = (uint16) atoi(row[4]);
		uint32     must_learn = (uint16) atoi(row[5]);


		// Skip the recipes that exceed the threshold in skill difference
		// Recipes that have either been made before or were
		// explicitly learned are excempt from that limit

		auto character_learned_recipe = CharacterRecipeListRepository::GetRecipe(
			character_learned_recipe_list,
			recipe_id
		);

		if (RuleB(Skills, UseLimitTradeskillSearchSkillDiff) &&
			((int32) trivial - (int32) GetSkill((EQ::skills::SkillType) tradeskill)) >
			RuleI(Skills, MaxTradeskillSearchSkillDiff)) {

			LogTradeskills("Checking limit recipe_id [{}] name [{}]", recipe_id, name);

			if (character_learned_recipe.made_count == 0) {
				continue;
			}
		}

		//Skip recipes that must be learned
		if ((must_learn & 0xf) && !character_learned_recipe.recipe_id) {
			continue;
		}


		auto               outapp = new EQApplicationPacket(OP_RecipeReply, sizeof(RecipeReply_Struct));
		RecipeReply_Struct *reply = (RecipeReply_Struct *) outapp->pBuffer;

		reply->object_type     = objtype;
		reply->some_id         = someid;
		reply->component_count = comp_count;
		reply->recipe_id       = recipe_id;
		reply->trivial         = trivial;
		strn0cpy(reply->recipe_name, name, sizeof(reply->recipe_name));
		FastQueuePacket(&outapp);
	}
}

void Client::SendTradeskillDetails(uint32 recipe_id) {

	std::string query = StringFormat("SELECT tre.item_id,tre.componentcount,i.icon,i.Name "
                                    "FROM tradeskill_recipe_entries AS tre "
                                    "LEFT JOIN items AS i ON tre.item_id = i.id "
                                    "WHERE tre.componentcount > 0 AND tre.recipe_id = %u",
                                    recipe_id);
    auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	if(results.RowCount() < 1) {
		LogError("Error in SendTradeskillDetails: no components returned");
		return;
	}

	if(results.RowCount() > 10) {
		LogError("Error in SendTradeskillDetails: too many components returned ([{}])", results.RowCount());
		return;
	}

	//biggest this packet can ever be:
	// 64 * 10 + 8 * 10 + 4 + 4 * 10 = 764
	auto buf = new char[775]; // dynamic so we can just give it to EQApplicationPacket
	uint8 r,k;

	uint32 *header = (uint32 *) buf;
	//Hell if I know why this is in the wrong byte order....
	*header = htonl(recipe_id);

	char *startblock = buf;
	startblock += sizeof(uint32);

	uint32 *ffff_start = (uint32 *) startblock;
	//fill in the FFFF's as if there were 0 items
	for(r = 0; r < 10; r++) { // world:item container size related?
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

	for(auto row = results.begin(); row != results.end(); ++row) {

		//watch for references to items which are not in the
		//items table, which the left join will make nullptr...
		if(row[2] == nullptr || row[3] == nullptr)
			continue;

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
		for(k = 0; k < num && count < 10; k++) { // world:item container size related?
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

	//now move the item data over top of the FFFFs
	uint8 dist = sizeof(uint32) * (10 - count);
	startblock += dist;
	memmove(startblock, datastart, datalen);

	uint32 total = sizeof(uint32) + dist + datalen;

	auto outapp = new EQApplicationPacket(OP_RecipeDetails);
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
	case EQ::skills::SkillFletching:
		skillup_modifier = RuleI(Character, TradeskillUpFletching);
		break;
	case EQ::skills::SkillAlchemy:
		skillup_modifier = RuleI(Character, TradeskillUpAlchemy);
		break;
	case EQ::skills::SkillJewelryMaking:
		skillup_modifier = RuleI(Character, TradeskillUpJewelcrafting);
		break;
	case EQ::skills::SkillPottery:
		skillup_modifier = RuleI(Character, TradeskillUpPottery);
		break;
	case EQ::skills::SkillBaking:
		skillup_modifier = RuleI(Character, TradeskillUpBaking);
		break;
	case EQ::skills::SkillBrewing:
		skillup_modifier = RuleI(Character, TradeskillUpBrewing);
		break;
	case EQ::skills::SkillBlacksmithing:
		skillup_modifier = RuleI(Character, TradeskillUpBlacksmithing);
		break;
	case EQ::skills::SkillResearch:
		skillup_modifier = RuleI(Character, TradeskillUpResearch);
		break;
	case EQ::skills::SkillMakePoison:
		skillup_modifier = RuleI(Character, TradeskillUpMakePoison);
		break;
	case EQ::skills::SkillTinkering:
		skillup_modifier = RuleI(Character, TradeskillUpTinkering);
		break;
	case EQ::skills::SkillTailoring:
		skillup_modifier = RuleI(Character, TradeskillUpTailoring);
		break;
	default:
		skillup_modifier = 2;
		break;
	}

	// Some tradeskills take the higher of one additional stat beside INT and WIS
	// to determine the skillup rate. Additionally these tradeskills do not have an
	// -15 modifier on their statbonus.
	if (spec->tradeskill == EQ::skills::SkillFletching || spec->tradeskill == EQ::skills::SkillMakePoison) {
		thirdstat = GetDEX();
		stat_modifier = 0;
	}
	else if (spec->tradeskill == EQ::skills::SkillBlacksmithing) {
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
		LogTradeskills("This combine cannot fail");
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
		MessageString(Chat::Emote, TRADESKILL_TRIVIAL);
	} else if(chance < 5) {
		// Minimum chance is always 5
		chance = 5;
	} else if(chance > 95) {
		//cap is 95, shouldent reach this before trivial, but just in case.
		chance = 95;
	}

	LogTradeskills("Current skill: [{}] , Trivial: [{}] , Success chance: [{}] percent", user_skill , spec->trivial , chance);
	LogTradeskills("Bonusstat: [{}] , INT: [{}] , WIS: [{}] , DEX: [{}] , STR: [{}]", bonusstat , GetINT() , GetWIS() , GetDEX() , GetSTR());

	float res = zone->random.Real(0, 99);
	int aa_chance = 0;

	aa_chance = spellbonuses.ReduceTradeskillFail[spec->tradeskill] + itembonuses.ReduceTradeskillFail[spec->tradeskill] + aabonuses.ReduceTradeskillFail[spec->tradeskill];

	const EQ::ItemData* item = nullptr;

	chance = mod_tradeskill_chance(chance, spec);

	if (((spec->tradeskill==75) || GetGM() || (chance > res)) || zone->random.Roll(aa_chance)) {

		success_modifier = 1;

		if (over_trivial < 0) {
			CheckIncreaseTradeskill(bonusstat, stat_modifier, skillup_modifier, success_modifier, spec->tradeskill);
		}

		MessageString(Chat::LightBlue, TRADESKILL_SUCCEED, spec->name.c_str());

		LogTradeskills("Tradeskill success");

		itr = spec->onsuccess.begin();
		while(itr != spec->onsuccess.end() && !spec->quest) {

			item = database.GetItem(itr->first);
			if (item) {
				SummonItem(itr->first, itr->second);
				if (GetGroup()) {
					entity_list.MessageGroup(this, true, Chat::Skills, "%s has successfully fashioned %s!", GetName(), item->Name);
				}
			}
			else {
				Log(
					Logs::General,
					Logs::Tradeskills,
					StringFormat(
						"Failure (null item pointer [id: %u, qty: %u]) :: recipe_id:%i tskillid:%i trivial:%i chance:%4.2f  in zoneid:%i instid:%i",
						itr->first,
						itr->second,
						spec->recipe_id,
						spec->tradeskill,
						spec->trivial,
						chance,
						GetZoneID(),
						GetInstanceID()
					).c_str()
				);
			}

			/* QS: Player_Log_Trade_Skill_Events */
			if (RuleB(QueryServ, PlayerLogTradeSkillEvents)) {

				std::string event_desc = StringFormat("Success :: fashioned recipe_id:%i tskillid:%i trivial:%i chance:%4.2f  in zoneid:%i instid:%i", spec->recipe_id, spec->tradeskill, spec->trivial, chance, GetZoneID(), GetInstanceID());
				QServ->PlayerLogEvent(Player_Log_Trade_Skill_Events, CharacterID(), event_desc);
			}

			if (RuleB(TaskSystem, EnableTaskSystem)) {
				UpdateTasksForItem(TaskActivityType::TradeSkill, itr->first, itr->second);
			}

			++itr;
		}
		return(true);
	}
	/* Tradeskill Fail */
	else {
		success_modifier = 2; // Halves the chance

		if(over_trivial < 0)
			CheckIncreaseTradeskill(bonusstat, stat_modifier, skillup_modifier, success_modifier, spec->tradeskill);

		MessageString(Chat::Emote,TRADESKILL_FAILED);

		LogTradeskills("Tradeskill failed");
			if (GetGroup())
		{
			entity_list.MessageGroup(this, true, Chat::Skills,"%s was unsuccessful in %s tradeskill attempt.",GetName(),GetGender() == 0 ? "his" : GetGender() == 1 ? "her" : "its");

		}

		/* QS: Player_Log_Trade_Skill_Events */
		if (RuleB(QueryServ, PlayerLogTradeSkillEvents)){
			std::string event_desc = StringFormat("Failed :: recipe_id:%i tskillid:%i trivial:%i chance:%4.2f  in zoneid:%i instid:%i", spec->recipe_id, spec->tradeskill, spec->trivial, chance, GetZoneID(), GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Trade_Skill_Events, CharacterID(), event_desc);
		}

		itr = spec->onfail.begin();
		while(itr != spec->onfail.end()) {
			//should we check these arguments?
			SummonItem(itr->first, itr->second);
			++itr;
		}

		/* Salvage Item rolls */

		// Rolls on each item, is possible to return everything
		int SalvageChance = aabonuses.SalvageChance + itembonuses.SalvageChance + spellbonuses.SalvageChance;
		// Skip check if not a normal TS or if a quest recipe these should be nofail, but check amyways
		if(SalvageChance && spec->tradeskill != 75 && !spec->quest) {
			itr = spec->salvage.begin();
			uint8 sc = 0;
			while(itr != spec->salvage.end()) {
				for (sc = 0; sc < itr->second; sc++) {
					if (zone->random.Roll(SalvageChance)) {
						SummonItem(itr->first, 1);
					}
				}
				++itr;
			}
		}

	}
	return(false);
}

void Client::CheckIncreaseTradeskill(int16 bonusstat, int16 stat_modifier, float skillup_modifier, uint16 success_modifier, EQ::skills::SkillType tradeskill)
{
	uint16 current_raw_skill = GetRawSkill(tradeskill);

	if(!CanIncreaseTradeskill(tradeskill))
		return;	//not allowed to go higher.
	uint16 maxskill = MaxSkill(tradeskill);

	float chance_stage2 = 0;

	//A successfull combine doubles the stage1 chance for an skillup
	//Some tradeskill are harder than others. See above for more.
	float chance_stage1 = (bonusstat - stat_modifier) / (skillup_modifier * success_modifier);

	//In stage2 the only thing that matters is your current unmodified skill.
	//If you want to customize here you probbably need to implement your own
	//formula instead of tweaking the below one.
	if (chance_stage1 > zone->random.Real(0, 99)) {
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

	if (chance_stage2 > zone->random.Real(0, 99)) {
		//Only if stage1 and stage2 succeeded you get a skillup.
		SetSkill(tradeskill, current_raw_skill + 1);
		std::string export_string = fmt::format(
			"{} {} {} {}",
			tradeskill,
			current_raw_skill + 1,
			maxskill,
			1
		);
		parse->EventPlayer(EVENT_SKILL_UP, this, export_string, 0);
		if(title_manager.IsNewTradeSkillTitleAvailable(tradeskill, current_raw_skill + 1))
			NotifyNewTitlesAvailable();
	}

	LogTradeskills("[CheckIncreaseTradeskill] skillup_modifier: [{}] , success_modifier: [{}] , stat modifier: [{}]", skillup_modifier , success_modifier , stat_modifier);
	LogTradeskills("[CheckIncreaseTradeskill] Stage1 chance was: [{}] percent", chance_stage1);
	LogTradeskills("[CheckIncreaseTradeskill] Stage2 chance was: [{}] percent. 0 percent means stage1 failed", chance_stage2);
}

bool ZoneDatabase::GetTradeRecipe(
	const EQ::ItemInstance *container,
	uint8 c_type,
	uint32 some_id,
	uint32 char_id,
	DBTradeskillRecipe_Struct *spec
)
{
	if (container == nullptr) {
		LogTradeskills("[GetTradeRecipe] Container null");
		return false;
	}

	std::string containers;// make where clause segment for container(s)
	if (some_id == 0) {
		containers = StringFormat("= %u", c_type); // world combiner so no item number
	}
	else {
		containers = StringFormat("IN (%u,%u)", c_type, some_id);
	} // container in inventory

	//Could prolly watch for stacks in this loop and handle them properly...
	//just increment sum and count accordingly
	bool        first = true;
	std::string buf2;
	uint32      count = 0;
	uint32      sum   = 0;
	for (uint8  i     = 0; i < 10; i++) { // <watch> TODO: need to determine if this is bound to world/item container size
		LogTradeskills("[GetTradeRecipe] Fetching item [{}]", i);

		const EQ::ItemInstance *inst = container->GetItem(i);
		if (!inst) {
			continue;
		}

		const EQ::ItemData *item = database.GetItem(inst->GetItem()->ID);
		if (!item) {
			LogTradeskills("[GetTradeRecipe] item [{}] not found!", inst->GetItem()->ID);
			continue;
		}

		if (first) {
			buf2 += StringFormat("%d", item->ID);
			first = false;
		}
		else {
			buf2 += StringFormat(",%d", item->ID);
		}

		sum += item->ID;
		count++;

		LogTradeskills(
			"[GetTradeRecipe] Item in container index [{}] item [{}] found [{}]",
			i,
			item->ID,
			count
		);
	}

	//no items == no recipe
	if (count == 0) {
		return false;
	}

	std::string query = StringFormat("SELECT tre.recipe_id "
                                    "FROM tradeskill_recipe_entries AS tre "
                                    "INNER JOIN tradeskill_recipe AS tr ON (tre.recipe_id = tr.id) "
                                    "WHERE tr.enabled AND (( tre.item_id IN(%s) AND tre.componentcount > 0) "
                                    "OR ( tre.item_id %s AND tre.iscontainer=1 ))"
                                    "GROUP BY tre.recipe_id HAVING sum(tre.componentcount) = %u "
                                    "AND sum(tre.item_id * tre.componentcount) = %u",
                                    buf2.c_str(), containers.c_str(), count, sum);
    auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in GetTradeRecipe search, query: [{}]", query.c_str());
		LogError("Error in GetTradeRecipe search, error: [{}]", results.ErrorMessage().c_str());
		return false;
	}

    if (results.RowCount() > 1) {
        //multiple recipes, partial match... do an extra query to get it exact.
        //this happens when combining components for a smaller recipe
        //which is completely contained within another recipe
        first = true;
        uint32 index = 0;
        buf2 = "";
        for (auto row = results.begin(); row != results.end(); ++row, ++index) {
            uint32 recipeid = (uint32)atoi(row[0]);
            if(first) {
                buf2 += StringFormat("%u", recipeid);
                first = false;
            } else
                buf2 += StringFormat(",%u", recipeid);

            //length limit on buf2
            if(index == 214) { //Maximum number of recipe matches (19 * 215 = 4096)
        		LogError("GetTradeRecipe warning: Too many matches. Unable to search all recipe entries. Searched [{}] of [{}] possible entries", index + 1, results.RowCount());
                break;
            }
        }

        query = StringFormat("SELECT tre.recipe_id "
                            "FROM tradeskill_recipe_entries AS tre "
							 "WHERE tre.recipe_id IN (%s) "
							 "GROUP BY tre.recipe_id HAVING sum(tre.componentcount) = %u "
							 "AND sum(tre.item_id * tre.componentcount) = %u", buf2.c_str(), count, sum
		);
		results = QueryDatabase(query);
		if (!results.Success()) {
			LogError("Error in GetTradeRecipe, re-query: [{}]", query.c_str());
			LogError("Error in GetTradeRecipe, error: [{}]", results.ErrorMessage().c_str());
			return false;
		}
	}

    if (results.RowCount() < 1)
        return false;

	if (results.RowCount() > 1) {
		//The recipe is not unique, so we need to compare the container were using.
		uint32 containerId = 0;

		if (some_id) { //Standard container
			containerId = some_id;
		}
		else if (c_type) {//World container
			containerId = c_type;
		}
		else { //Invalid container
			return false;
		}

		query   = StringFormat(
			"SELECT tre.recipe_id "
			"FROM tradeskill_recipe_entries AS tre "
			"WHERE tre.recipe_id IN (%s) "
			"AND tre.item_id = %u;", buf2.c_str(), containerId
		);
		results = QueryDatabase(query);
		if (!results.Success()) {
			LogError("Error in GetTradeRecipe, re-query: [{}]", query.c_str());
			LogError("Error in GetTradeRecipe, error: [{}]", results.ErrorMessage().c_str());
			return false;
		}

		if (results.RowCount() == 0) { //Recipe contents matched more than 1 recipe, but not in this container
			LogError("Combine error: Incorrect container is being used!");
			return false;
		}

		if (results.RowCount() > 1) { //Recipe contents matched more than 1 recipe in this container
			LogError(
				"Combine error: Recipe is not unique! [{}] matches found for container [{}]. Continuing with first recipe match",
				results.RowCount(),
				containerId
			);
		}
	}

	auto row = results.begin();
	uint32 recipe_id = (uint32)atoi(row[0]);

	//Right here we verify that we actually have ALL of the tradeskill components..
	//instead of part which is possible with experimentation.
	//This is here because something's up with the query above.. it needs to be rethought out
	bool has_components = true;
	query = StringFormat("SELECT item_id, componentcount "
                        "FROM tradeskill_recipe_entries "
                        "WHERE recipe_id = %i AND componentcount > 0",
                        recipe_id);
	results = QueryDatabase(query);
    if (!results.Success()) {
        return GetTradeRecipe(recipe_id, c_type, some_id, char_id, spec);
    }

	if (results.RowCount() == 0) {
		return GetTradeRecipe(recipe_id, c_type, some_id, char_id, spec);
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int component_count = 0;

		for (int x = EQ::invbag::SLOT_BEGIN; x < EQ::invtype::WORLD_SIZE; x++) {
            const EQ::ItemInstance* inst = container->GetItem(x);
            if(!inst)
                continue;

			const EQ::ItemData* item = database.GetItem(inst->GetItem()->ID);
            if (!item)
                continue;

			if (item->ID == atoi(row[0])) {
				component_count++;
			}

			LogTradeskills(
				"[GetTradeRecipe] Component count loop [{}] item [{}] recipe component_count [{}]",
				component_count,
				item->ID,
				atoi(row[1])
			);
		}

		if (component_count != atoi(row[1])) {
			return false;
		}
	}

	return GetTradeRecipe(recipe_id, c_type, some_id, char_id, spec);
}

bool ZoneDatabase::GetTradeRecipe(
	uint32 recipe_id,
	uint8 c_type,
	uint32 some_id,
	uint32 char_id,
	DBTradeskillRecipe_Struct *spec
)
{
	std::string container_where_filter;
	if (some_id == 0) {
		// world combiner so no item number
		container_where_filter = StringFormat("= %u", c_type);
	}
	else {
		// container in inventory
		container_where_filter = StringFormat("IN (%u,%u)", c_type, some_id);
	}

	std::string query = StringFormat(
		SQL (
			SELECT
			tradeskill_recipe.id,
			tradeskill_recipe.tradeskill,
			tradeskill_recipe.skillneeded,
			tradeskill_recipe.trivial,
			tradeskill_recipe.nofail,
			tradeskill_recipe.replace_container,
			tradeskill_recipe.name,
			tradeskill_recipe.must_learn,
			tradeskill_recipe.quest
			FROM
				tradeskill_recipe
				INNER JOIN tradeskill_recipe_entries ON tradeskill_recipe.id = tradeskill_recipe_entries.recipe_id
			WHERE
				tradeskill_recipe.id = %lu
				AND tradeskill_recipe_entries.item_id %s
				AND tradeskill_recipe.enabled
				GROUP BY
				tradeskill_recipe.id
			)
		,
		(unsigned long) recipe_id,
		container_where_filter.c_str()
	);
    auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in GetTradeRecipe, query: [{}]", query.c_str());
		LogError("Error in GetTradeRecipe, error: [{}]", results.ErrorMessage().c_str());
		return false;
	}

	if (results.RowCount() != 1) {
		return false;
	}

	auto row = results.begin();

	spec->tradeskill        = (EQ::skills::SkillType) atoi(row[1]);
	spec->skill_needed      = (int16) atoi(row[2]);
	spec->trivial           = (uint16) atoi(row[3]);
	spec->nofail            = atoi(row[4]) ? true : false;
	spec->replace_container = atoi(row[5]) ? true : false;
	spec->name              = row[6];
	spec->must_learn        = (uint8) atoi(row[7]);
	spec->quest             = atoi(row[8]) ? true : false;
	spec->has_learnt        = false;
	spec->madecount         = 0;
	spec->recipe_id         = recipe_id;

	auto character_learned_recipe_list = CharacterRecipeListRepository::GetLearnedRecipeList(char_id);
	auto character_learned_recipe      = CharacterRecipeListRepository::GetRecipe(
		character_learned_recipe_list,
		recipe_id
	);

	if (character_learned_recipe.recipe_id) { //If this exists we learned it
		LogTradeskills("[GetTradeRecipe] made_count [{}]", character_learned_recipe.made_count);

		spec->has_learnt = true;
		spec->madecount = (uint32)character_learned_recipe.made_count;
	}

	//Pull the on-success items...
	query = StringFormat("SELECT item_id,successcount FROM tradeskill_recipe_entries "
                        "WHERE successcount > 0 AND recipe_id = %u", recipe_id);
    results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if(results.RowCount() < 1 && !spec->quest) {
		LogError("Error in GetTradeRecept success: no success items returned");
		return false;
	}

	spec->onsuccess.clear();
	for(auto row = results.begin(); row != results.end(); ++row) {
		uint32 item = (uint32)atoi(row[0]);
		uint8 num = (uint8) atoi(row[1]);
		spec->onsuccess.push_back(std::pair<uint32,uint8>(item, num));
	}

    spec->onfail.clear();
	//Pull the on-fail items...
	query   = StringFormat(
		"SELECT item_id, failcount FROM tradeskill_recipe_entries "
		"WHERE failcount > 0 AND recipe_id = %u", recipe_id
	);
	results = QueryDatabase(query);
	if (results.Success()) {
		for (auto row = results.begin(); row != results.end(); ++row) {
			uint32 item = (uint32) atoi(row[0]);
			uint8  num  = (uint8) atoi(row[1]);
			spec->onfail.push_back(std::pair<uint32, uint8>(item, num));
		}
	}

	spec->salvage.clear();

	// Don't bother with the query if TS is nofail
	if (spec->nofail) {
		return true;
	}

	// Pull the salvage list
	query = StringFormat(
		"SELECT item_id, salvagecount "
		"FROM tradeskill_recipe_entries "
		"WHERE salvagecount > 0 AND recipe_id = %u", recipe_id
	);

	results = QueryDatabase(query);
	if (results.Success()) {
		for (auto row = results.begin(); row != results.end(); ++row) {
			uint32 item = (uint32) atoi(row[0]);
			uint8  num  = (uint8) atoi(row[1]);
			spec->salvage.push_back(std::pair<uint32, uint8>(item, num));
		}
	}

	return true;
}

void ZoneDatabase::UpdateRecipeMadecount(uint32 recipe_id, uint32 char_id, uint32 madeCount)
{
	std::string query = StringFormat("INSERT INTO char_recipe_list "
                                    "SET recipe_id = %u, char_id = %u, madecount = %u "
                                    "ON DUPLICATE KEY UPDATE madecount = %u;",
                                    recipe_id, char_id, madeCount, madeCount);
    QueryDatabase(query);
}

void Client::LearnRecipe(uint32 recipe_id)
{
	std::string query = fmt::format(
		SQL(
			select
			char_id,
			recipe_id,
			madecount
				from
				char_recipe_list
				where
				char_id = {}
				and recipe_id = {}
			LIMIT 1
		),
		CharacterID(),
		recipe_id
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	auto tradeskill_recipe = TradeskillRecipeRepository::FindOne(content_db, recipe_id);
	if (tradeskill_recipe.id == 0) {
		LogError("Invalid recipe [{}]", recipe_id);
		return;
	}

	LogTradeskills(
		"[LearnRecipe] recipe_id [{}] name [{}] learned [{}]",
		recipe_id,
		tradeskill_recipe.name,
		results.RowCount()
	);

	auto row = results.begin();
	if (results.RowCount() > 0) {
		return;
	}

	MessageString(Chat::LightBlue, TRADESKILL_LEARN_RECIPE, tradeskill_recipe.name.c_str());

	database.QueryDatabase(
		fmt::format(
			"REPLACE INTO char_recipe_list (recipe_id, char_id, madecount) VALUES ({}, {}, 0)",
			recipe_id,
			CharacterID()
		)
	);
}

bool Client::CanIncreaseTradeskill(EQ::skills::SkillType tradeskill) {
	uint32 rawskill = GetRawSkill(tradeskill);
	uint16 maxskill = MaxSkill(tradeskill);

	if (rawskill >= maxskill) //Max skill sanity check
		return false;

	uint8 Baking = (GetRawSkill(EQ::skills::SkillBaking) > 200) ? 1 : 0;
	uint8 Smithing = (GetRawSkill(EQ::skills::SkillBlacksmithing) > 200) ? 1 : 0;
	uint8 Brewing = (GetRawSkill(EQ::skills::SkillBrewing) > 200) ? 1 : 0;
	uint8 Fletching = (GetRawSkill(EQ::skills::SkillFletching) > 200) ? 1 : 0;
	uint8 Jewelry = (GetRawSkill(EQ::skills::SkillJewelryMaking) > 200) ? 1 : 0;
	uint8 Pottery = (GetRawSkill(EQ::skills::SkillPottery) > 200) ? 1 : 0;
	uint8 Tailoring = (GetRawSkill(EQ::skills::SkillTailoring) > 200) ? 1 : 0;
	uint8 SkillTotal = Baking + Smithing + Brewing + Fletching + Jewelry + Pottery + Tailoring; //Tradeskills above 200
	//New Tanaan AA: Each level allows an additional tradeskill above 200 (first one is free)
	uint8 aaLevel = spellbonuses.TradeSkillMastery + itembonuses.TradeSkillMastery + aabonuses.TradeSkillMastery;

	switch (tradeskill) {
	case EQ::skills::SkillBaking:
	case EQ::skills::SkillBlacksmithing:
	case EQ::skills::SkillBrewing:
	case EQ::skills::SkillFletching:
	case EQ::skills::SkillJewelryMaking:
	case EQ::skills::SkillPottery:
	case EQ::skills::SkillTailoring:
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

bool ZoneDatabase::EnableRecipe(uint32 recipe_id)
{
	std::string query = StringFormat("UPDATE tradeskill_recipe SET enabled = 1 "
                                    "WHERE id = %u;", recipe_id);
    auto results = QueryDatabase(query);
	if (!results.Success())

	return results.RowsAffected() > 0;

	return false;
}

bool ZoneDatabase::DisableRecipe(uint32 recipe_id)
{
	std::string query = StringFormat("UPDATE tradeskill_recipe SET enabled = 0 "
                                    "WHERE id = %u;", recipe_id);
    auto results = QueryDatabase(query);
	if (!results.Success())

	return results.RowsAffected() > 0;

	return false;
}
