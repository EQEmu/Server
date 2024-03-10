/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include "../common/events/player_event_logs.h"
#include "../common/repositories/trader_repository.h"
#include "worldserver.h"
#include "string_ids.h"
#include "parcels.h"
#include "client.h"
#include "../common/ruletypes.h"

extern WorldServer worldserver;

void Client::SendBulkParcels()
{
	SetEngagedWithParcelMerchant(true);
	LoadParcels();

	if (parcels.empty()) {
		return;
	}

	ParcelMessaging_Struct pms{};
	pms.packet_type = ItemPacketParcel;

	std::stringstream           ss;
	cereal::BinaryOutputArchive ar(ss);

	for (auto &p: parcels) {
		auto item = database.GetItem(p.second.item_id);
		if (item) {
			auto inst = database.CreateItem(item, p.second.quantity);
			if (inst) {
				if (inst->IsStackable()) {
					inst->SetCharges(p.second.quantity);
				}

				if (item->ID == PARCEL_MONEY_ITEM_ID) {
					inst->SetPrice(p.second.quantity);
					inst->SetCharges(1);
				} else {
					inst->SetCharges(p.second.quantity > 0 ? p.second.quantity : 1);
				}

				inst->SetMerchantCount(1);
				inst->SetMerchantSlot(p.second.slot_id);

				pms.player_name     = p.second.from_name;
				pms.sent_time       = p.second.sent_date;
				pms.note            = p.second.note;
				pms.serialized_item = inst->Serialize(p.second.slot_id);
				pms.slot_id         = p.second.slot_id;
				ar(pms);

				uint32 packet_size = ss.str().length();
				auto   out         = new EQApplicationPacket(OP_ItemPacket, packet_size);
				if (out->size != packet_size) {
					LogError("Attempted to send a parcel packet of mismatched size {} with a buffer size of {}.",
							 out->Size(), packet_size);
					return;
				}
				memcpy(out->pBuffer, ss.str().data(), out->size);
				FastQueuePacket(&out);

				ss.str("");
				ss.clear();
			}
		}

	}
	if (parcels.size() >= PARCEL_LIMIT) {
		LogError("Found {} parcels for Character {}.  List truncated to the PARCEL_MAX_ITEMS [{}] + 5.",
				 parcels.size(),
				 GetCleanName(),
				 PARCEL_MAX_ITEMS
		);
		SendParcelStatus();
		return;
	}
}

void Client::SendParcel(Parcel_Struct parcel_in)
{
	auto results = ParcelsRepository::GetWhere(
		database,
		fmt::format("`to_name` = '{}' AND `slot_id` = '{}' LIMIT 1", parcel_in.send_to, parcel_in.item_slot));

	if (results.empty()) {
		return;
	}

	ParcelMessaging_Struct pms{};
	pms.packet_type = ItemPacketParcel;

	std::stringstream           ss;
	cereal::BinaryOutputArchive ar(ss);

	BaseParcelsRepository::Parcels parcel{};
	parcel.from_name = results[0].from_name;
	parcel.id        = results[0].id;
	parcel.note      = results[0].note;
	parcel.quantity  = results[0].quantity;
	parcel.sent_date = results[0].sent_date;
	parcel.item_id   = results[0].item_id;
	parcel.slot_id   = results[0].slot_id;
	parcel.to_name   = results[0].to_name;

	auto item = database.GetItem(parcel.item_id);
	if (item) {
		auto inst = database.CreateItem(item, parcel.quantity);
		if (inst) {
			if (inst->IsStackable()) {
				inst->SetCharges(parcel.quantity);
			}

			if (item->ID == PARCEL_MONEY_ITEM_ID) {
				inst->SetPrice(parcel.quantity);
				inst->SetCharges(1);
			} else {
				inst->SetCharges(parcel.quantity > 0 ? parcel.quantity : 1);
			}

			inst->SetMerchantCount(1);
			inst->SetMerchantSlot(parcel.slot_id);

			pms.player_name     = parcel.from_name;
			pms.sent_time       = parcel.sent_date;
			pms.note            = parcel.note;
			pms.serialized_item = inst->Serialize(parcel.slot_id);
			pms.slot_id         = parcel.slot_id;
			ar(pms);

			uint32 packet_size = ss.str().length();
			auto   out         = new EQApplicationPacket(OP_ItemPacket, packet_size);
			if (out->size != packet_size) {
				LogError("Attempted to send a parcel packet of mismatched size {} with a buffer size of {}.",
						 out->Size(), packet_size);
				return;
			}
			memcpy(out->pBuffer, ss.str().data(), out->size);
			FastQueuePacket(&out);

			ss.str("");
			ss.clear();

			parcels.emplace(parcel.slot_id, parcel);
		}
	}
}

void Client::DoParcelCancel()
{
	if (parcel_platinum || parcel_gold || parcel_silver || parcel_copper) {
		m_pp.platinum 	+= parcel_platinum;
		m_pp.gold 		+= parcel_gold;
		m_pp.silver 	+= parcel_silver;
		m_pp.copper 	+= parcel_copper;
		parcel_platinum = 0;
		parcel_gold     = 0;
		parcel_silver   = 0;
		parcel_copper   = 0;
		SaveCurrency();
		SendMoneyUpdate();
	}
}

void Client::SendParcelStatus()
{
	LoadParcels();

	int32 num_of_parcels = GetParcelCount();
	if (num_of_parcels > 0) {
		int32 num_over_limit = (num_of_parcels - PARCEL_MAX_ITEMS) < 0 ? 0 : (num_of_parcels - PARCEL_MAX_ITEMS);
		if (num_of_parcels == PARCEL_MAX_ITEMS) {
			Message(
				Chat::Red,
				fmt::format(
					"You have reached the limit of {} parcels in your mailbox.  You will not be able to send parcels until you retrieve at least 1 parcel. ",
					PARCEL_MAX_ITEMS
				).c_str()
			);
		}
		else if (num_over_limit == 1) {
			MessageString(
				Chat::Red,
				PARCEL_STATUS_1,
				std::to_string(num_of_parcels).c_str(),
				std::to_string(PARCEL_MAX_ITEMS).c_str()
			);
		}
		else if (num_over_limit > 1) {
			MessageString(
				Chat::Red,
				PARCEL_STATUS_2,
				std::to_string(num_of_parcels).c_str(),
				std::to_string(num_over_limit).c_str(),
				std::to_string(PARCEL_MAX_ITEMS).c_str()
			);
		}
		else {
			Message(
				Chat::Yellow,
				fmt::format(
					"You have {} parcels in your mailbox. Please visit a parcel merchant soon.",
					num_of_parcels
				).c_str()
			);
		}
	}
	SendParcelIconStatus();
}


void Client::DoParcelSend(Parcel_Struct *parcel_in)
{
	auto send_to_client = ParcelsRepository::GetParcelCountAndCharacterName(database, parcel_in->send_to);
	auto merchant       = entity_list.GetMob(parcel_in->npc_id);
	if (!merchant) {
		SendParcelAck();
		return;
	}

	auto num_of_parcels = GetParcelCount();
	if (num_of_parcels >= PARCEL_MAX_ITEMS) {
		SendParcelIconStatus();
		Message(
			Chat::Yellow,
			fmt::format(
				"{} tells you, 'Unfortunately, I cannot send your parcel as you are at your parcel limit of {}. Please retrieve a parcel and try again.",
				merchant->GetCleanName(),
				PARCEL_MAX_ITEMS
			).c_str()
		);
		DoParcelCancel();
		SendParcelAck();
		return;
	}

	if (send_to_client.at(0).parcel_count >= PARCEL_MAX_ITEMS) {
		Message(
			Chat::Yellow,
			fmt::format(
				"{} tells you, 'Unfortunately, {} cannot accept any more parcels at this time. Please try again later.",
				merchant->GetCleanName(),
				send_to_client.at(0).character_name == GetCleanName() ? "you" : send_to_client.at(0).character_name
			).c_str()
		);
		SendParcelAck();
		DoParcelCancel();
		return;
	}

	if (GetParcelTimer()->Check()) {
		SetParcelEnabled(true);
	}

	if (!GetParcelEnabled()) {
		MessageString(Chat::Yellow, PARCEL_DELAY, merchant->GetCleanName());
		DoParcelCancel();
		SendParcelAck();
		return;
	}
	auto next_slot = INVALID_INDEX;
	if (!send_to_client.at(0).character_name.empty()) {
		next_slot = FindNextFreeParcelSlot(send_to_client.at(0).character_name);
		if (next_slot == INVALID_INDEX) {
			Message(
				Chat::Yellow,
				fmt::format(
					"{} tells you, 'Unfortunately, {} cannot accept any more parcels at this time. Please try again later.",
					merchant->GetCleanName(),
					send_to_client.at(0).character_name
				).c_str()
			);
			SendParcelAck();
			DoParcelCancel();
			return;
		}
	}

	switch (parcel_in->money_flag) {
		case PARCEL_SEND_ITEMS: {
			auto inst = GetInv().GetItem(parcel_in->item_slot);
			if (!inst) {
				LogError("Handle_OP_ShopSendParcel Could not find item in inventory slot {} for character {}.",
						 parcel_in->item_slot, GetCleanName());
				SendParcelAck();
				DoParcelCancel();
				return;
			}

			if (send_to_client.at(0).character_name.empty()) {
				MessageString(
					Chat::Yellow, PARCEL_UNKNOWN_NAME, merchant->GetCleanName(), parcel_in->send_to,
					inst->GetItem()->Name
				);
				SendParcelAck();
				DoParcelCancel();
				return;
			}

			uint32 quantity {};
			if (inst->IsStackable()) {
				quantity = parcel_in->quantity;
			} else {
				quantity = inst->GetCharges() > 0 ? inst->GetCharges() : parcel_in->quantity;
			}

			ParcelsRepository::Parcels parcel_out;
			parcel_out.from_name = GetName();
			parcel_out.note      = parcel_in->note;
			parcel_out.sent_date = time(nullptr);
			parcel_out.quantity  = quantity;
			parcel_out.item_id   = inst->GetID();
			parcel_out.to_name   = parcel_in->send_to;
			parcel_out.slot_id   = next_slot;
			parcel_out.id        = 0;

			auto result = ParcelsRepository::InsertOne(database, parcel_out);
			if (!result.id) {
				LogError("Failed to add parcel to database.  From {} to {} item {} quantity {}", parcel_out.from_name,
						 parcel_out.to_name, parcel_out.item_id, parcel_out.quantity);
				Message(Chat::Yellow, "Unable to save parcel to the database. Please see an administrator.");
				return;
			}

			RemoveItem(parcel_out.item_id, parcel_out.quantity);
			//DeleteItemInInventory(parcel_in->item_slot, quantity, true, true);
			auto outapp = new EQApplicationPacket(OP_ShopSendParcel);
			FastQueuePacket(&outapp);

			if (inst->IsStackable() && (quantity - parcel_in->quantity > 0)) {
				inst->SetCharges(quantity - parcel_in->quantity);
				PutItemInInventory(parcel_in->item_slot, *inst, true);
			}

			MessageString(
				Chat::Yellow, PARCEL_DELIVERY, merchant->GetCleanName(), inst->GetItem()->Name,
				send_to_client.at(0).character_name.c_str());

			if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{};
				e.from_player_name = parcel_out.from_name;
				e.to_player_name   = parcel_out.to_name;
				e.item_id          = parcel_out.item_id;
				e.quantity         = parcel_out.quantity;
				e.sent_date        = parcel_out.sent_date;

				RecordPlayerEventLog(PlayerEvent::PARCEL_SEND, e);
			}

			Parcel_Struct ps{};
			ps.item_slot = parcel_out.slot_id;
			strn0cpy(ps.send_to, parcel_out.to_name.c_str(), sizeof(ps.send_to));

			SendParcelDeliveryToWorld(ps);

			break;
		}
		case PARCEL_SEND_MONEY: {
			auto item = database.GetItem(PARCEL_MONEY_ITEM_ID);
			if (!item) {
				DoParcelCancel();
				SendParcelAck();
				return;
			}

			auto inst = database.CreateItem(item, 1);
			if (!inst) {
				DoParcelCancel();
				SendParcelAck();
				return;
			}

			auto money = inst->DetermineMoneyStringForParcels(parcel_in->quantity);

			if (send_to_client.at(0).character_name.empty()) {
				MessageString(
					Chat::Yellow, PARCEL_UNKNOWN_NAME, merchant->GetCleanName(), parcel_in->send_to,
					money.c_str());
				DoParcelCancel();
				SendParcelAck();
				return;
			}

			ParcelsRepository::Parcels parcel_out;
			parcel_out.from_name = GetName();
			parcel_out.note      = parcel_in->note;
			parcel_out.sent_date = time(nullptr);
			parcel_out.quantity  = parcel_in->quantity;
			parcel_out.item_id   = PARCEL_MONEY_ITEM_ID;
			parcel_out.to_name   = parcel_in->send_to;
			parcel_out.slot_id   = next_slot;
			parcel_out.id        = 0;

			auto result = ParcelsRepository::InsertOne(database, parcel_out);
			if (!result.id) {
				LogError("Failed to add parcel to database.  From {} to {} item {} quantity {}", parcel_out.from_name,
						 parcel_out.to_name, parcel_out.item_id, parcel_out.quantity);
				Message(Chat::Yellow, "Unable to save parcel to the database. Please see an administrator.");
				return;
			}

			MessageString(
				Chat::Yellow, PARCEL_DELIVERY, merchant->GetCleanName(), money.c_str(),
				send_to_client.at(0).character_name.c_str());

			if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{};
				e.from_player_name = parcel_out.from_name;
				e.to_player_name   = parcel_out.to_name;
				e.item_id          = parcel_out.item_id;
				e.quantity         = parcel_out.quantity;
				e.sent_date        = parcel_out.sent_date;

				RecordPlayerEventLog(PlayerEvent::PARCEL_SEND, e);
			}

			parcel_platinum = 0;
			parcel_gold     = 0;
			parcel_silver   = 0;
			parcel_copper   = 0;
			auto outapp = new EQApplicationPacket(OP_FinishTrade);
			FastQueuePacket(&outapp);

			Parcel_Struct ps{};
			ps.item_slot = parcel_out.slot_id;
			strn0cpy(ps.send_to, parcel_out.to_name.c_str(), sizeof(ps.send_to));

			SendParcelDeliveryToWorld(ps);

			break;
		}
	}

	SendParcelAck();
	SendParcelIconStatus();
	SetParcelEnabled(false);
	GetParcelTimer()->Enable();
}

void Client::SendParcelAck()
{
	auto outapp = new EQApplicationPacket(OP_FinishTrade);
	FastQueuePacket(&outapp);

	outapp = new EQApplicationPacket(OP_ShopSendParcel, sizeof(Parcel_Struct));
	auto data = (Parcel_Struct *) outapp->pBuffer;
	data->item_slot = 0xffffffff;
	data->quantity  = 0xffffffff;
	FastQueuePacket(&outapp);
}

void Client::SendParcelRetrieveAck()
{
	auto outapp = new EQApplicationPacket(OP_ShopRetrieveParcel);
	FastQueuePacket(&outapp);
}

void Client::SendParcelDeliveryToWorld(Parcel_Struct parcel)
{
	auto out  = new ServerPacket(ServerOP_ParcelDelivery, sizeof(Parcel_Struct));
	auto data = (Parcel_Struct *) out->pBuffer;

	data->item_slot = parcel.item_slot;
	strn0cpy(data->send_to, parcel.send_to, sizeof(data->send_to));

	worldserver.SendPacket(out);
}

void Client::DoParcelRetrieve(ParcelRetrieve_Struct parcel_in)
{
	auto merchant = entity_list.GetNPCByID(parcel_in.merchant_entity_id);
	if (!merchant) {
		SendParcelRetrieveAck();
		return;
	}

	auto p = parcels.find(parcel_in.parcel_slot_id);
	if (p != parcels.end()) {
		uint32 item_id       = parcel_in.parcel_item_id;
		uint32 item_quantity = p->second.quantity;
		if (!item_id || !item_quantity) {
			LogError("Attempt to retrieve parcel with erroneous item id or quantity for client character id {}.",
					 CharacterID());
			SendParcelRetrieveAck();
			return;
		}

		auto inst = database.CreateItem(item_id, item_quantity);
		if (!inst) {
			SendParcelRetrieveAck();
			return;
		}

		switch (parcel_in.parcel_item_id) {
			case PARCEL_MONEY_ITEM_ID: {
				AddMoneyToPP(p->second.quantity, true);
				MessageString(
					Chat::Yellow,
					PARCEL_DELIVERED,
					merchant->GetCleanName(),
					inst->DetermineMoneyStringForParcels(p->second.quantity).c_str(),
					p->second.from_name.c_str()
				);
				break;
			}
			default: {
				auto free_id = GetInv().FindFreeSlot(false, false);
				if (CheckLoreConflict(inst->GetItem())) {
					if (RuleB(Parcel, DeleteOnDuplicate)) {
						MessageString(
							Chat::Yellow,
							PARCEL_DUPLICATE_DELETE,
							inst->GetItem()->Name
						);
					}
					else {
						MessageString(
							Chat::Yellow,
							DUP_LORE
						);
						SendParcelRetrieveAck();
						return;
					}
				}
				else if (inst->IsStackable()) {
					inst->SetCharges(item_quantity);
					if (TryStacking(inst, ItemPacketTrade, true, false)) {
						MessageString(
							Chat::Yellow,
							PARCEL_DELIVERED_2,
							merchant->GetCleanName(),
							std::to_string(item_quantity).c_str(),
							inst->GetItem()->Name,
							p->second.from_name.c_str()
						);
					}
					else if (free_id != INVALID_INDEX) {
						inst->SetCharges(item_quantity);
						if (PutItemInInventory(free_id, *inst, true)) {
							MessageString(
								Chat::Yellow,
								PARCEL_DELIVERED_2,
								merchant->GetCleanName(),
								std::to_string(item_quantity).c_str(),
								inst->GetItem()->Name,
								p->second.from_name.c_str()
							);
						}
					}
					else {
						MessageString(
							Chat::Yellow,
							PARCEL_INV_FULL,
							merchant->GetCleanName()
						);
						SendParcelRetrieveAck();
						return;
					}
				}
				else if (free_id != INVALID_INDEX) {
					inst->SetCharges(item_quantity > 0 ? item_quantity : 1);
					if (PutItemInInventory(free_id, *inst, true)) {
						MessageString(
							Chat::Yellow,
							PARCEL_DELIVERED,
							merchant->GetCleanName(),
							inst->GetItem()->Name,
							p->second.from_name.c_str()
						);
					}
					else {
						MessageString(
							Chat::Yellow,
							PARCEL_INV_FULL,
							merchant->GetCleanName()
						);
						SendParcelRetrieveAck();
						return;
					}
				}
				else {
					MessageString(
						Chat::Yellow,
						PARCEL_INV_FULL,
						merchant->GetCleanName()
					);
					SendParcelRetrieveAck();
					return;
				}
			}
		}

		if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_RETRIEVE)) {
			PlayerEvent::ParcelRetrieve e{};
			e.from_player_name = p->second.from_name;
			e.item_id          = p->second.item_id;
			e.quantity         = p->second.quantity;
			e.sent_date        = p->second.sent_date;

			RecordPlayerEventLog(PlayerEvent::PARCEL_RETRIEVE, e);
		}

		DeleteParcel(p->second.id);
		SendParcelDelete(parcel_in);
		parcels.erase(p);
	}
	SendParcelRetrieveAck();
	SendParcelIconStatus();
}

bool Client::DeleteParcel(uint32 parcel_id)
{
	auto result = BaseParcelsRepository::DeleteOne(database, parcel_id);
	if (!result) {
		LogError("Error deleting parcel id {} from the database.", parcel_id);
		return false;
	}

	auto it = std::find_if(parcels.cbegin(), parcels.cend(), [&](const auto &x) { return x.second.id == parcel_id; });
	SetParcelCount(GetParcelCount() - 1);

	return true;
}

void Client::LoadParcels()
{
	parcels.clear();
	auto results = ParcelsRepository::GetWhere(database, fmt::format("to_name = '{}'", GetCleanName()));

	for (auto const &p: results) {
		parcels.emplace(p.slot_id, p);
	}

	SetParcelCount(parcels.size());
}

void Client::SendParcelDelete(const ParcelRetrieve_Struct parcel_in)
{
	auto outapp = new EQApplicationPacket(OP_ShopDeleteParcel, sizeof(ParcelRetrieve_Struct));
	auto data   = (ParcelRetrieve_Struct *) outapp->pBuffer;

	data->merchant_entity_id = parcel_in.merchant_entity_id;
	data->player_entity_id   = parcel_in.player_entity_id;
	data->parcel_slot_id     = parcel_in.parcel_slot_id;
	data->parcel_item_id     = parcel_in.parcel_item_id;

	FastQueuePacket(&outapp);
}


int32 Client::FindNextFreeParcelSlot(std::string &character_name)
{
	auto results = ParcelsRepository::GetWhere(
		database, fmt::format("to_name = '{}' ORDER BY slot_id ASC", character_name.c_str()));

	if (results.empty()) {
		return PARCEL_BEGIN_SLOT;
	}
	else {
		for (uint32 i = PARCEL_BEGIN_SLOT; i <= PARCEL_END_SLOT; i++) {
			auto it = std::find_if(results.cbegin(), results.cend(), [&](const auto &x) { return x.slot_id == i; });
			if (it != results.end()) {
				continue;
			}
			else {
				return i;
			}
		}

		return INVALID_INDEX;
	}
}

void Client::SendParcelIconStatus()
{
	auto outapp = new EQApplicationPacket(OP_ShopParcelIcon, sizeof(ParcelIcon_Struct));
	auto data   = (ParcelIcon_Struct *) outapp->pBuffer;

	auto const num_of_parcels = GetParcelCount();

	if (num_of_parcels == 0) {
		data->status = IconOff;
	}
	else if (num_of_parcels > PARCEL_MAX_ITEMS) {
		data->status = Overlimit;
	}
	else {
		data->status = IconOn;
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::AddParcel(ParcelsRepository::Parcels parcel) {
	auto result = ParcelsRepository::InsertOne(database, parcel);
	if (!result.id) {
		LogError("Failed to add parcel to database.  From {} to {} item {} quantity {}", parcel.from_name,
				 parcel.to_name, parcel.item_id, parcel.quantity);
		Message(Chat::Yellow, "Unable to send parcel at this time.  Please try again later.");
		SendParcelAck();
		return;
	}

	//parcels.emplace(result.slot_id, result);
	//SetParcelCount(GetParcelCount() + 1);
}
