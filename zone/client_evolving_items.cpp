#include "../common/evolving_items.h"

#include "../common/events/player_event_logs.h"
#include "../common/global_define.h"

#include "client.h"
#include "string_ids.h"
#include "worldserver.h"

extern WorldServer worldserver;
extern QueryServ* QServ;

void Client::DoEvolveItemToggle(const EQApplicationPacket *app)
{
	const auto in   = reinterpret_cast<EvolveItemToggle *>(app->pBuffer);
	auto       item = CharacterEvolvingItemsRepository::FindOne(database, in->unique_id);

	LogEvolveItemDetail(
		"Character ID <green>[{}] requested to set evolve item with unique id <yellow>[{}] to status <yellow>[{}]",
		CharacterID(),
		in->unique_id,
		in->activated
	);

	if (!item.id) {
		LogEvolveItemDetail(
			"Character ID <green>[{}] toggle evolve item unique id <yellow>[{}] failed", CharacterID(), in->unique_id);
		return;
	}

	item.activated  = in->activated;
	const auto inst = GetInv().GetItem(GetInv().HasItem(item.item_id));
	inst->SetEvolveActivated(item.activated ? true : false);

	CharacterEvolvingItemsRepository::ReplaceOne(database, item);

	SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, item);
}

void Client::SendEvolvingPacket(const int8 action, const CharacterEvolvingItemsRepository::CharacterEvolvingItems &item)
{
	auto       out  = std::make_unique<EQApplicationPacket>(OP_EvolveItem, sizeof(EvolveItemToggle));
	const auto data = reinterpret_cast<EvolveItemToggle *>(out->pBuffer);

	LogEvolveItemDetail(
		"Character ID <green>[{}] requested info for evolving item with unique id <yellow>[{}] status <yellow>[{}] "
		"percentage <yellow>[{}]",
		CharacterID(),
		item.id,
		item.activated,
		item.progression
	);

	data->action     = action;
	data->unique_id  = item.id;
	data->percentage = item.progression;
	data->activated  = item.activated;

	QueuePacket(out.get());

	LogEvolveItem(
		"Sent evolve item with unique id <yellow>[{}] status <yellow>[{}] percentage <yellow>[{}] to Character ID "
		"<green>[{}]",
		data->unique_id,
		data->activated,
		data->percentage,
		CharacterID()
	);
}

void Client::ProcessEvolvingItem(const uint64 exp, const Mob *mob)
{
	std::vector<const EQ::ItemInstance *> queue{};

	for (auto &[key, inst]: GetInv().GetWorn()) {
		LogEvolveItemDetail(
			"CharacterID <green>[{}] found equipped item ID <yellow>[{}]", CharacterID(), inst->GetID());
		if (!inst->IsEvolving() || !inst->GetEvolveActivated()) {
			LogEvolveItemDetail(
				"CharacterID <green>[{}], item ID <yellow>[{}] not an evolving item.", CharacterID(), inst->GetID());
			continue;
		}

		if (inst->GetTimers().contains("evolve") && !inst->GetTimers().at("evolve").Check()) {
			LogEvolveItemDetail(
				"CharacterID <green>[{}], item ID <yellow>[{}] timer not yet expired. <red>[{}] secs remaining.",
				CharacterID(),
				inst->GetID(),
				inst->GetTimers().at("evolve").GetRemainingTime() / 1000);
			continue;
		}

		auto const type     = evolving_items_manager.GetEvolvingItemsCache().at(inst->GetID()).type;
		auto const sub_type = evolving_items_manager.GetEvolvingItemsCache().at(inst->GetID()).sub_type;

		LogEvolveItemDetail(
			"CharacterID <green>[{}] item id <green>[{}] type {} sub_type {} is Evolving.  Continue processing...",
			CharacterID(),
			inst->GetID(),
			type,
			sub_type);

		switch (type) {
			case EvolvingItems::Types::AMOUNT_OF_EXP: {
				LogEvolveItemDetail("Type <green>[{}] Processing sub_type", type);
				if (sub_type == EvolvingItems::SubTypes::ALL_EXP ||
					(sub_type == EvolvingItems::SubTypes::GROUP_EXP && IsGrouped())) {
					LogEvolveItemDetail("Sub_Type <green>[{}] Processing Item", sub_type);
					inst->SetEvolveAddToCurrentAmount(exp * RuleR(EvolvingItems, PercentOfGroupExperience) / 100);
				}
				else if (
					sub_type == EvolvingItems::SubTypes::ALL_EXP ||
					(sub_type == EvolvingItems::SubTypes::RAID_EXP && IsRaidGrouped())) {
					LogEvolveItemDetail("Sub_Type <green>[{}] Processing Item", sub_type);
					inst->SetEvolveAddToCurrentAmount(exp * RuleR(EvolvingItems, PercentOfRaidExperience) / 100);
				}
				else if (
					sub_type == EvolvingItems::SubTypes::ALL_EXP || sub_type == EvolvingItems::SubTypes::SOLO_EXP) {
					LogEvolveItemDetail("Sub_Type <green>[{}] Processing Item", sub_type);
					inst->SetEvolveAddToCurrentAmount(exp * RuleR(EvolvingItems, PercentOfSoloExperience) / 100);
				}

				inst->CalculateEvolveProgression();

				auto e = CharacterEvolvingItemsRepository::SetCurrentAmountAndProgression(
					database, inst->GetEvolveUniqueID(), inst->GetEvolveCurrentAmount(), inst->GetEvolveProgression());
				if (!e.id) {
					break;
				}

				SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, e);

				LogEvolveItem(
					"Processing Complete for item id <green>[{1}] Type 1 Amount of EXP - SubType <yellow>[{0}] - "
					"Assigned <yellow>[{2}] of exp to <green>[{1}]",
					sub_type,
					inst->GetID(),
					exp * 0.001
				);

				if (inst->GetEvolveProgression() >= 100) {
					queue.push_back(inst);
				}

				break;
			}
			case EvolvingItems::Types::SPECIFIC_MOB_RACE: {
				LogEvolveItemDetail("Type <green>[{}] Processing sub type", type);
				if (mob && mob->GetRace() == sub_type) {
					LogEvolveItemDetail("Sub_Type <green>[{}] Processing Item", sub_type);
					inst->SetEvolveAddToCurrentAmount(1);
					inst->CalculateEvolveProgression();

					auto e = CharacterEvolvingItemsRepository::SetCurrentAmountAndProgression(
						database,
						inst->GetEvolveUniqueID(),
						inst->GetEvolveCurrentAmount(),
						inst->GetEvolveProgression());
					if (!e.id) {
						break;
					}

					SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, e);

					LogEvolveItem(
						"Processing Complete for item id <green>[{1}] Type 3 Specific Mob Race - SubType <yellow>[{0}] "
						"- Increased count by 1 for <green>[{1}]",
						sub_type,
						inst->GetID()
					);
				}

				if (inst->GetEvolveProgression() >= 100) {
					queue.push_back(inst);
				}

				break;
			}
			case EvolvingItems::Types::SPECIFIC_ZONE_ID: {
				LogEvolveItemDetail("Type <green>[{}] Processing sub type", type);
				if (mob && mob->GetZoneID() == sub_type) {
					LogEvolveItemDetail("Sub_Type <green>[{}] Processing Item", sub_type);
					inst->SetEvolveAddToCurrentAmount(1);
					inst->CalculateEvolveProgression();

					auto e = CharacterEvolvingItemsRepository::SetCurrentAmountAndProgression(
						database,
						inst->GetEvolveUniqueID(),
						inst->GetEvolveCurrentAmount(),
						inst->GetEvolveProgression());
					if (!e.id) {
						break;
					}

					SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, e);

					LogEvolveItem(
						"Processing Complete for item id <green>[{1}] Type 4 Specific Zone ID - SubType <yellow>[{0}] "
						"- Increased count by 1 for <green>[{1}]",
						sub_type,
						inst->GetID()
					);
				}

				if (inst->GetEvolveProgression() >= 100) {
					queue.push_back(inst);
				}

				break;
			}
			default: {
			}
		}
	}

	if (!queue.empty()) {
		for (auto const &i: queue) {
			DoEvolveCheckProgression(*i);
		}
	}
}

void Client::DoEvolveItemDisplayFinalResult(const EQApplicationPacket *app)
{
	const auto in        = reinterpret_cast<EvolveItemToggle *>(app->pBuffer);

	const uint32 item_id = static_cast<uint32>(in->unique_id & 0xFFFFFFFF);
	if (item_id == 0) {
		LogEvolveItem("Error - Item ID of final evolve item is blank.");
		return;
	}

	std::unique_ptr<EQ::ItemInstance> const inst(database.CreateItem(item_id));

	LogEvolveItemDetail(
		"Character ID <green>[{}] requested to view final evolve item id <yellow>[{}] for evolve item id <yellow>[{}]",
		CharacterID(),
		item_id,
		evolving_items_manager.GetFirstItemInLoreGroupByItemID(item_id));

	inst->SetEvolveProgression(100);

	if (inst) {
		LogEvolveItemDetail(
			"Sending final result for item id <yellow>[{}] to Character ID <green>[{}]", item_id, CharacterID());
		SendItemPacket(0, inst.get(), ItemPacketViewLink);
	}
}

bool Client::DoEvolveCheckProgression(const EQ::ItemInstance &inst)
{
	if (inst.GetEvolveProgression() < 100 || inst.GetEvolveLvl() == inst.GetMaxEvolveLvl()) {
		return false;
	}

	const auto new_item_id = evolving_items_manager.GetNextEvolveItemID(inst);
	if (!new_item_id) {
		return false;
	}

	std::unique_ptr<EQ::ItemInstance> const new_inst(database.CreateItem(new_item_id));

	if (!new_inst) {
		return false;
	}

	CheckItemDiscoverability(new_inst->GetID());

	PlayerEvent::EvolveItem e{};

	RemoveItemBySerialNumber(inst.GetSerialNumber());
	evolving_items_manager.LoadPlayerEvent(inst, e);
	e.status = "Evolved Item due to obtaining progression - Old Evolve Item removed from inventory.";
	RecordPlayerEventLog(PlayerEvent::EVOLVE_ITEM, e);

	PushItemOnCursor(*new_inst, true);
	evolving_items_manager.LoadPlayerEvent(*new_inst, e);
	e.status = "Evolved Item due to obtaining progression - New Evolve Item placed in inventory.";
	RecordPlayerEventLog(PlayerEvent::EVOLVE_ITEM, e);

	MessageString(Chat::Yellow, EVOLVE_ITEM_EVOLVED, inst.GetItem()->Name);

	LogEvolveItem(
		"Evolved item id <red>[{}] into item id <green>[{}] for Character ID <green>[{}]",
		inst.GetID(),
		new_inst->GetID(),
		CharacterID());

	return true;
}

void Client::SendEvolveXPTransferWindow()
{
	auto       out  = std::make_unique<EQApplicationPacket>(OP_EvolveItem, sizeof(EvolveItemToggle));
	const auto data = reinterpret_cast<EvolveItemToggle *>(out->pBuffer);

	data->action    = 1;

	QueuePacket(out.get());
}

void Client::SendEvolveXPWindowDetails(const EQApplicationPacket *app)
{
	const auto in = reinterpret_cast<EvolveXPWindowReceive *>(app->pBuffer);

	const auto item_1_slot =
		GetInv().HasEvolvingItem(in->item1_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);
	const auto item_2_slot =
		GetInv().HasEvolvingItem(in->item2_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);

	if (item_1_slot == INVALID_INDEX || item_2_slot == INVALID_INDEX) {
		return;
	}

	const auto inst_from = GetInv().GetItem(item_1_slot);
	const auto inst_to   = GetInv().GetItem(item_2_slot);

	if (!inst_from || !inst_to) {
		return;
	}

	const auto results = evolving_items_manager.DetermineTransferResults(*inst_from, *inst_to);

	if (!results.item_from_id || !results.item_to_id) {
		SendEvolveTransferResults(*inst_from, *inst_to, *inst_from, *inst_to, 0, 0);
		return;
	}

	std::unique_ptr<EQ::ItemInstance> const inst_from_new(database.CreateItem(results.item_from_id));
	std::unique_ptr<EQ::ItemInstance> const inst_to_new(database.CreateItem(results.item_to_id));
	if (!inst_from_new || !inst_to_new) {
		SendEvolveTransferResults(*inst_from, *inst_to, *inst_from, *inst_to, 0, 0);
		return;
	}

	inst_from_new->SetEvolveCurrentAmount(results.item_from_current_amount);
	inst_from_new->CalculateEvolveProgression();
	inst_to_new->SetEvolveCurrentAmount(results.item_to_current_amount);
	inst_to_new->CalculateEvolveProgression();

	SendEvolveTransferResults(
		*inst_from, *inst_to, *inst_from_new, *inst_to_new, results.compatibility, results.max_transfer_level);
}

void Client::DoEvolveTransferXP(const EQApplicationPacket *app)
{
	const auto in = reinterpret_cast<EvolveXPWindowReceive *>(app->pBuffer);

	const auto item_1_slot =
		GetInv().HasEvolvingItem(in->item1_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);
	const auto item_2_slot =
		GetInv().HasEvolvingItem(in->item2_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);

	if (item_1_slot == INVALID_INDEX || item_2_slot == INVALID_INDEX) {
		return;
	}

	const auto inst_from = GetInv().GetItem(item_1_slot);
	const auto inst_to   = GetInv().GetItem(item_2_slot);

	if (!inst_from || !inst_to) {
		Message(Chat::Red, "Transfer Failed.  Incompatible Items.");
		LogEvolveItem("Transfer Failed for Character ID <green>[{}]", CharacterID());
		return;
	}

	const auto results = evolving_items_manager.DetermineTransferResults(*inst_from, *inst_to);

	if (!results.item_from_id || !results.item_to_id) {
		Message(Chat::Red, "Transfer Failed.  Incompatible Items.");
		LogEvolveItem("Transfer Failed for Character ID <green>[{}]", CharacterID());
		return;
	}

	std::unique_ptr<const EQ::ItemInstance> const inst_from_new(database.CreateItem(results.item_from_id));
	std::unique_ptr<const EQ::ItemInstance> const inst_to_new(database.CreateItem(results.item_to_id));

	if (!inst_from_new || !inst_to_new) {
		Message(Chat::Red, "Transfer Failed.  Incompatible Items.");
		LogEvolveItem("Transfer Failed for Character ID <green>[{}]", CharacterID());
		return;
	}

	inst_from_new->SetEvolveCurrentAmount(results.item_from_current_amount);
	inst_from_new->CalculateEvolveProgression();
	inst_to_new->SetEvolveCurrentAmount(results.item_to_current_amount);
	inst_to_new->CalculateEvolveProgression();

	PlayerEvent::EvolveItem e{};

	RemoveItemBySerialNumber(inst_from->GetSerialNumber());
	evolving_items_manager.LoadPlayerEvent(*inst_from, e);
	e.status = "Transfer XP - Original FROM Evolve Item removed from inventory.";
	RecordPlayerEventLog(PlayerEvent::EVOLVE_ITEM, e);

	PushItemOnCursor(*inst_from_new, true);
	evolving_items_manager.LoadPlayerEvent(*inst_from_new, e);
	e.status = "Transfer XP - Updated FROM item placed in inventory.";
	RecordPlayerEventLog(PlayerEvent::EVOLVE_ITEM, e);

	RemoveItemBySerialNumber(inst_to->GetSerialNumber());
	evolving_items_manager.LoadPlayerEvent(*inst_to, e);
	e.status = "Transfer XP - Original TO Evolve Item removed from inventory.";
	RecordPlayerEventLog(PlayerEvent::EVOLVE_ITEM, e);

	PushItemOnCursor(*inst_to_new, true);
	evolving_items_manager.LoadPlayerEvent(*inst_to_new, e);
	e.status = "Transfer XP - Updated TO Evolve item placed in inventory.";
	RecordPlayerEventLog(PlayerEvent::EVOLVE_ITEM, e);

	LogEvolveItem(
		"Evolve Transfer XP resulted in evolved item id <red>[{}] into item id <green>[{}] for Character ID "
		"<green>[{}]",
		inst_to->GetID(),
		inst_to_new->GetID(),
		CharacterID()
	);
}

void Client::SendEvolveTransferResults(
	const EQ::ItemInstance &inst_from,
	const EQ::ItemInstance &inst_to,
	const EQ::ItemInstance &inst_from_new,
	const EQ::ItemInstance &inst_to_new,
	const uint32            compatibility,
	const uint32            max_transfer_level)
{
	std::stringstream           ss;
	cereal::BinaryOutputArchive ar(ss);

	EvolveXPWindowSend e{};
	e.action             = EvolvingItems::Actions::TRANSFER_WINDOW_DETAILS;
	e.compatibility      = compatibility;
	e.item1_unique_id    = inst_from.GetEvolveUniqueID();
	e.item2_unique_id    = inst_to.GetEvolveUniqueID();
	e.max_transfer_level = max_transfer_level;
	e.item1_present      = 1;
	e.item2_present      = 1;
	e.serialize_item_1   = inst_from_new.Serialize(0);
	e.serialize_item_2   = inst_to_new.Serialize(0);

	{
		ar(e);
	}

	uint32 packet_size = sizeof(EvolveItemMessaging) + ss.str().length();

	std::unique_ptr<EQApplicationPacket> out(new EQApplicationPacket(OP_EvolveItem, packet_size));
	const auto                           data = reinterpret_cast<EvolveItemMessaging *>(out->pBuffer);

	data->action                              = EvolvingItems::Actions::TRANSFER_WINDOW_DETAILS;
	memcpy(data->serialized_data, ss.str().data(), ss.str().length());

	QueuePacket(out.get());

	ss.str("");
	ss.clear();
}
