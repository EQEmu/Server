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
#include "../common/repositories/character_parcels_repository.h"
#include "../common/repositories/character_parcels_containers_repository.h"
#include "worldserver.h"
#include "string_ids.h"
#include "client.h"
#include "../common/ruletypes.h"

extern WorldServer worldserver;

void Client::SendBulkParcels()
{
	SetEngagedWithParcelMerchant(true);
	LoadParcels();

	if (m_parcels.empty()) {
		return;
	}

	ParcelMessaging_Struct pms{};
	pms.packet_type = ItemPacketParcel;

	std::stringstream           ss;
	cereal::BinaryOutputArchive ar(ss);

	for (auto &p: m_parcels) {
		auto item = database.GetItem(p.second.item_id);
		if (item) {
			std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(
				item,
				p.second.quantity,
				p.second.aug_slot_1,
				p.second.aug_slot_2,
				p.second.aug_slot_3,
				p.second.aug_slot_4,
				p.second.aug_slot_5,
				p.second.aug_slot_6
			));
			if (inst) {
				inst->SetCharges(p.second.quantity > 0 ? p.second.quantity : 1);
				inst->SetMerchantCount(1);
				inst->SetMerchantSlot(p.second.slot_id);
				if (inst->IsStackable()) {
					inst->SetCharges(p.second.quantity);
				}

				if (item->ID == PARCEL_MONEY_ITEM_ID) {
					inst->SetPrice(p.second.quantity);
					inst->SetCharges(1);
				}

				pms.player_name     = p.second.from_name;
				pms.sent_time       = p.second.sent_date;
				pms.note            = p.second.note;
				pms.serialized_item = inst->Serialize(p.second.slot_id);
				pms.slot_id         = p.second.slot_id;
				ar(pms);

				uint32 packet_size = ss.str().length();
				std::unique_ptr<EQApplicationPacket> out(new EQApplicationPacket(OP_ItemPacket, packet_size));
				if (out->size != packet_size) {
					LogError(
						"Attempted to send a parcel packet of mismatched size {} with a buffer size of {}.",
						out->Size(),
						packet_size
					);
					return;
				}
				memcpy(out->pBuffer, ss.str().data(), out->size);
				QueuePacket(out.get());

				ss.str("");
				ss.clear();
			}
		}

	}
	if (m_parcels.size() >= RuleI(Parcel, ParcelMaxItems) + PARCEL_LIMIT) {
		LogError(
			"Found {} parcels for Character {}.  List truncated to the ParcelMaxItems rule [{}] + PARCEL_LIMIT.",
			m_parcels.size(),
			GetCleanName(),
			RuleI(Parcel, ParcelMaxItems)
		);
		SendParcelStatus();
		return;
	}
}

void Client::SendParcel(Parcel_Struct &parcel_in)
{
	auto results = CharacterParcelsRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = '{}' AND `slot_id` = '{}' LIMIT 1",
			CharacterID(),
			parcel_in.item_slot
		)
	);

	if (results.empty()) {
		return;
	}

	const auto& r = results.front();

	ParcelMessaging_Struct pms{};
	pms.packet_type = ItemPacketParcel;

	std::stringstream           ss;
	cereal::BinaryOutputArchive ar(ss);

	CharacterParcelsRepository::CharacterParcels p{};

	p.from_name  = r.from_name;
	p.id         = r.id;
	p.note       = r.note;
	p.quantity   = r.quantity;
	p.sent_date  = r.sent_date;
	p.item_id    = r.item_id;
	p.aug_slot_1 = r.aug_slot_1;
	p.aug_slot_2 = r.aug_slot_2;
	p.aug_slot_3 = r.aug_slot_3;
	p.aug_slot_4 = r.aug_slot_4;
	p.aug_slot_5 = r.aug_slot_5;
	p.aug_slot_6 = r.aug_slot_6;
	p.slot_id    = r.slot_id;
	p.char_id    = r.char_id;

	auto item = database.GetItem(p.item_id);
	if (item) {
		std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(
			item,
			p.quantity,
			p.aug_slot_1,
			p.aug_slot_2,
			p.aug_slot_3,
			p.aug_slot_4,
			p.aug_slot_5,
			p.aug_slot_6
		));
		if (inst) {
			inst->SetCharges(p.quantity > 0 ? p.quantity : 1);
			inst->SetMerchantCount(1);
			inst->SetMerchantSlot(p.slot_id);
			if (inst->IsStackable()) {
				inst->SetCharges(p.quantity);
			}

			if (item->ID == PARCEL_MONEY_ITEM_ID) {
				inst->SetPrice(p.quantity);
				inst->SetCharges(1);
			}

			pms.player_name     = p.from_name;
			pms.sent_time       = p.sent_date;
			pms.note            = p.note;
			pms.serialized_item = inst->Serialize(p.slot_id);
			pms.slot_id         = p.slot_id;
			ar(pms);

			uint32 packet_size = ss.str().length();
			std::unique_ptr<EQApplicationPacket> out(new EQApplicationPacket(OP_ItemPacket, packet_size));
			if (out->size != packet_size) {
				LogError(
					"Attempted to send a parcel packet of mismatched size {} with a buffer size of {}.",
					out->Size(),
					packet_size
				);
				return;
			}

			memcpy(out->pBuffer, ss.str().data(), out->size);
			QueuePacket(out.get());

			ss.str("");
			ss.clear();

			m_parcels.emplace(p.slot_id, p);
		}
	}
}

void Client::DoParcelCancel()
{
	if (
		m_parcel_platinum ||
		m_parcel_gold ||
		m_parcel_silver ||
		m_parcel_copper
	) {
		m_pp.platinum += m_parcel_platinum;
		m_pp.gold     += m_parcel_gold;
		m_pp.silver   += m_parcel_silver;
		m_pp.copper   += m_parcel_copper;
		m_parcel_platinum = 0;
		m_parcel_gold     = 0;
		m_parcel_silver   = 0;
		m_parcel_copper   = 0;
		SaveCurrency();
		SendMoneyUpdate();
	}
}

void Client::SendParcelStatus()
{
	LoadParcels();

	int32 num_of_parcels = GetParcelCount();
	if (num_of_parcels > 0) {
		int32 num_over_limit = (num_of_parcels - RuleI(Parcel, ParcelMaxItems)) < 0 ? 0 : (num_of_parcels - RuleI(Parcel, ParcelMaxItems));
		if (num_of_parcels == RuleI(Parcel, ParcelMaxItems)) {
			Message(
				Chat::Red,
				fmt::format(
					"You have reached the limit of {} parcels in your mailbox.  You will not be able to send parcels until you retrieve at least 1 parcel. ",
					RuleI(Parcel, ParcelMaxItems)
				).c_str()
			);
		}
		else if (num_over_limit == 1) {
			MessageString(
				Chat::Red,
				PARCEL_STATUS_1,
				std::to_string(num_of_parcels).c_str(),
				std::to_string(RuleI(Parcel, ParcelMaxItems)).c_str()
			);
		}
		else if (num_over_limit > 1) {
			MessageString(
				Chat::Red,
				PARCEL_STATUS_2,
				std::to_string(num_of_parcels).c_str(),
				std::to_string(num_over_limit).c_str(),
				std::to_string(RuleI(Parcel, ParcelMaxItems)).c_str()
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


void Client::DoParcelSend(const Parcel_Struct *parcel_in)
{
	auto send_to_client = CharacterParcelsRepository::GetParcelCountAndCharacterName(database, parcel_in->send_to);
	auto merchant       = entity_list.GetMob(parcel_in->npc_id);
	if (!merchant) {
		SendParcelAck();
		return;
	}

	auto num_of_parcels = GetParcelCount();
	if (num_of_parcels >= RuleI(Parcel, ParcelMaxItems)) {
		SendParcelIconStatus();
		Message(
			Chat::Yellow,
			fmt::format(
				"{} tells you, 'Unfortunately, I cannot send your parcel as you are at your parcel limit of {}. Please retrieve a parcel and try again.",
				merchant->GetCleanName(),
				RuleI(Parcel, ParcelMaxItems)
			).c_str()
		);
		DoParcelCancel();
		SendParcelAck();
		return;
	}

	if (send_to_client.at(0).parcel_count >= RuleI(Parcel, ParcelMaxItems)) {
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
		next_slot = FindNextFreeParcelSlot(send_to_client.at(0).char_id);
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
				LogError(
					"Handle_OP_ShopSendParcel Could not find item in inventory slot {} for character {}.",
					parcel_in->item_slot,
					GetCleanName()
				);
				SendParcelAck();
				DoParcelCancel();
				return;
			}

			if (send_to_client.at(0).character_name.empty()) {
				MessageString(
					Chat::Yellow,
					PARCEL_UNKNOWN_NAME,
					merchant->GetCleanName(),
					parcel_in->send_to,
					inst->GetItem()->Name
				);
				SendParcelAck();
				DoParcelCancel();
				return;
			}

			uint32 quantity{};
			if (inst->IsStackable()) {
				quantity = parcel_in->quantity;
			}
			else {
				quantity = inst->GetCharges() > 0 ? inst->GetCharges() : parcel_in->quantity;
			}

			CharacterParcelsRepository::CharacterParcels parcel_out{};
			parcel_out.from_name = GetName();
			parcel_out.note      = parcel_in->note;
			parcel_out.sent_date = time(nullptr);
			parcel_out.quantity  = quantity;
			parcel_out.item_id   = inst->GetID();
			parcel_out.char_id   = send_to_client.at(0).char_id;
			parcel_out.slot_id   = next_slot;
			parcel_out.id        = 0;

			if (inst->IsAugmented()) {
				auto augs			  = inst->GetAugmentIDs();
				parcel_out.aug_slot_1 = augs.at(0);
				parcel_out.aug_slot_2 = augs.at(1);
				parcel_out.aug_slot_3 = augs.at(2);
				parcel_out.aug_slot_4 = augs.at(3);
				parcel_out.aug_slot_5 = augs.at(4);
				parcel_out.aug_slot_6 = augs.at(5);
			}

			auto result = CharacterParcelsRepository::InsertOne(database, parcel_out);
			if (!result.id) {
				LogError(
					"Failed to add parcel to database.  From {} to {} item {} quantity {}",
					parcel_out.from_name,
					parcel_out.char_id,
					parcel_out.item_id,
					parcel_out.quantity
				);
				Message(Chat::Yellow, "Unable to save parcel to the database. Please see an administrator.");
				return;
			}

			std::vector<CharacterParcelsContainersRepository::CharacterParcelsContainers> all_entries{};
			if (inst->IsNoneEmptyContainer()) {
				CharacterParcelsContainersRepository::CharacterParcelsContainers cpc{};

				for (auto const &kv: *inst->GetContents()) {
					cpc.parcels_id = result.id;
					cpc.slot_id    = kv.first;
					cpc.item_id    = kv.second->GetID();
					if (kv.second->IsAugmented()) {
						auto augs      = kv.second->GetAugmentIDs();
						cpc.aug_slot_1 = augs.at(0);
						cpc.aug_slot_2 = augs.at(1);
						cpc.aug_slot_3 = augs.at(2);
						cpc.aug_slot_4 = augs.at(3);
						cpc.aug_slot_5 = augs.at(4);
						cpc.aug_slot_6 = augs.at(5);
					}
					cpc.quantity   = kv.second->GetCharges() > 0 ? kv.second->GetCharges() : 1;
					all_entries.push_back(cpc);
				}
				CharacterParcelsContainersRepository::InsertMany(database, all_entries);
			}

			RemoveItemBySerialNumber(inst->GetSerialNumber(), parcel_out.quantity);
			std::unique_ptr<EQApplicationPacket> outapp(new EQApplicationPacket(OP_ShopSendParcel));
			QueuePacket(outapp.get());

			if (inst->IsStackable() && (quantity - parcel_in->quantity > 0)) {
				inst->SetCharges(quantity - parcel_in->quantity);
				PutItemInInventory(parcel_in->item_slot, *inst, true);
			}

			MessageString(
				Chat::Yellow,
				PARCEL_DELIVERY,
				merchant->GetCleanName(),
				inst->GetItem()->Name,
				send_to_client.at(0).character_name.c_str()
			);

			if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{};
				e.from_player_name = parcel_out.from_name;
				e.to_player_name   = send_to_client.at(0).character_name;
				e.item_id          = parcel_out.item_id;
				e.aug_slot_1       = parcel_out.aug_slot_1;
				e.aug_slot_2       = parcel_out.aug_slot_2;
				e.aug_slot_3       = parcel_out.aug_slot_3;
				e.aug_slot_4       = parcel_out.aug_slot_4;
				e.aug_slot_5       = parcel_out.aug_slot_5;
				e.aug_slot_6       = parcel_out.aug_slot_6;
				e.quantity         = parcel_out.quantity;
				e.sent_date        = parcel_out.sent_date;

				RecordPlayerEventLog(PlayerEvent::PARCEL_SEND, e);

				if (!all_entries.empty()) {
					for (auto const &i: all_entries) {
						e.from_player_name = parcel_out.from_name;
						e.to_player_name   = send_to_client.at(0).character_name;
						e.item_id          = i.item_id;
						e.aug_slot_1       = i.aug_slot_1;
						e.aug_slot_2       = i.aug_slot_2;
						e.aug_slot_3       = i.aug_slot_3;
						e.aug_slot_4       = i.aug_slot_4;
						e.aug_slot_5       = i.aug_slot_5;
						e.aug_slot_6       = i.aug_slot_6;
						e.quantity         = i.quantity;
						e.sent_date        = parcel_out.sent_date;
						RecordPlayerEventLog(PlayerEvent::PARCEL_SEND, e);
					}
				}
			}

			Parcel_Struct ps{};
			ps.item_slot = parcel_out.slot_id;
			strn0cpy(ps.send_to, send_to_client.at(0).character_name.c_str(), sizeof(ps.send_to));

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

			std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(item, 1));
			if (!inst) {
				DoParcelCancel();
				SendParcelAck();
				return;
			}

			if (send_to_client.at(0).character_name.empty()) {
				MessageString(
					Chat::Yellow,
					PARCEL_UNKNOWN_NAME,
					merchant->GetCleanName(),
					parcel_in->send_to,
					"Money"
				);
				DoParcelCancel();
				SendParcelAck();
				return;
			}

			CharacterParcelsRepository::CharacterParcels parcel_out{};
			parcel_out.from_name = GetName();
			parcel_out.note      = parcel_in->note;
			parcel_out.sent_date = time(nullptr);
			parcel_out.quantity  = parcel_in->quantity;
			parcel_out.item_id   = PARCEL_MONEY_ITEM_ID;
			parcel_out.char_id   = send_to_client.at(0).char_id;
			parcel_out.slot_id   = next_slot;
			parcel_out.id        = 0;

			auto result = CharacterParcelsRepository::InsertOne(database, parcel_out);
			if (!result.id) {
				LogError(
					"Failed to add parcel to database.  From {} to {} item {} quantity {}",
					parcel_out.from_name,
					send_to_client.at(0).character_name,
					parcel_out.item_id,
					parcel_out.quantity
				);
				Message(
					Chat::Yellow,
					"Unable to save parcel to the database. Please see an administrator."
				);
				return;
			}

			MessageString(
				Chat::Yellow,
				PARCEL_DELIVERY,
				merchant->GetCleanName(),
				"Money",
				send_to_client.at(0).character_name.c_str()
			);

			if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{};
				e.from_player_name = parcel_out.from_name;
				e.to_player_name   = send_to_client.at(0).character_name;
				e.item_id          = parcel_out.item_id;
				e.quantity         = parcel_out.quantity;
				e.sent_date        = parcel_out.sent_date;

				RecordPlayerEventLog(PlayerEvent::PARCEL_SEND, e);
			}

			m_parcel_platinum = 0;
			m_parcel_gold     = 0;
			m_parcel_silver   = 0;
			m_parcel_copper   = 0;
			std::unique_ptr<EQApplicationPacket> outapp(new EQApplicationPacket(OP_FinishTrade));
			QueuePacket(outapp.get());

			Parcel_Struct ps{};
			ps.item_slot = parcel_out.slot_id;
			strn0cpy(ps.send_to, send_to_client.at(0).character_name.c_str(), sizeof(ps.send_to));

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
	std::unique_ptr<EQApplicationPacket> outapp(new EQApplicationPacket(OP_FinishTrade));
	QueuePacket(outapp.get());

	std::unique_ptr<EQApplicationPacket> outapp2(new EQApplicationPacket(OP_ShopSendParcel, sizeof(Parcel_Struct)));
	auto data = (Parcel_Struct *) outapp2->pBuffer;
	data->item_slot = 0xffffffff;
	data->quantity  = 0xffffffff;
	QueuePacket(outapp2.get());
}

void Client::SendParcelRetrieveAck()
{
	std::unique_ptr<EQApplicationPacket> outapp(new EQApplicationPacket(OP_ShopRetrieveParcel));
	QueuePacket(outapp.get());
}

void Client::SendParcelDeliveryToWorld(const Parcel_Struct &parcel)
{
	std::unique_ptr<ServerPacket> out(new ServerPacket(ServerOP_ParcelDelivery, sizeof(Parcel_Struct)));
	auto                          data = (Parcel_Struct *) out->pBuffer;

	data->item_slot = parcel.item_slot;
	strn0cpy(data->send_to, parcel.send_to, sizeof(data->send_to));

	worldserver.SendPacket(out.get());
}

void Client::DoParcelRetrieve(const ParcelRetrieve_Struct &parcel_in)
{
	auto merchant = entity_list.GetNPCByID(parcel_in.merchant_entity_id);
	if (!merchant) {
		SendParcelRetrieveAck();
		return;
	}

	auto p = m_parcels.find(parcel_in.parcel_slot_id);
	if (p != m_parcels.end()) {
		uint32 item_id       = parcel_in.parcel_item_id;
		uint32 item_quantity = p->second.quantity;
		if (!item_id || !item_quantity) {
			LogError(
				"Attempt to retrieve parcel with erroneous item id or quantity for client character id {}.",
				CharacterID()
			);
			SendParcelRetrieveAck();
			return;
		}

		std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(
			item_id,
			item_quantity,
			p->second.aug_slot_1,
			p->second.aug_slot_2,
			p->second.aug_slot_3,
			p->second.aug_slot_4,
			p->second.aug_slot_5,
			p->second.aug_slot_6
		)
		);
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
					"Money",
					p->second.from_name.c_str()
				);
				break;
			}
			default: {
				auto free_id = GetInv().FindFreeSlot(false, false);
				if (CheckLoreConflict(inst->GetItem())) {
					if (RuleB(Parcel, DeleteOnDuplicate)) {
						MessageString(Chat::Yellow, PARCEL_DUPLICATE_DELETE, inst->GetItem()->Name);
					}
					else {
						MessageString(Chat::Yellow, DUP_LORE);
						SendParcelRetrieveAck();
						return;
					}
				}
				else if (inst->IsStackable()) {
					inst->SetCharges(item_quantity);
					if (TryStacking(inst.get(), ItemPacketTrade, true, false)) {
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
						MessageString(Chat::Yellow, PARCEL_INV_FULL, merchant->GetCleanName());
						SendParcelRetrieveAck();
						return;
					}
				}
				else if (free_id != INVALID_INDEX) {
					std::vector<CharacterParcelsContainersRepository::CharacterParcelsContainers> results{};

					if (inst->IsClassBag() && inst->GetItem()->BagSlots > 0) {
						results = CharacterParcelsContainersRepository::GetWhere(database, fmt::format("`parcels_id` = {}", p->second.id));
						for (auto const &i : results) {
							std::unique_ptr<EQ::ItemInstance> item(
								database.CreateItem(
									i.item_id,
									i.quantity,
									i.aug_slot_1,
									i.aug_slot_2,
									i.aug_slot_3,
									i.aug_slot_4,
									i.aug_slot_5,
									i.aug_slot_6
								)
							);
							if (CheckLoreConflict(item->GetItem())) {
								Message(
									Chat::Yellow,
									fmt::format("Lore Item Found in Inventory: {}", item->GetItem()->Name).c_str());
								MessageString(Chat::Yellow, DUP_LORE);
								Message(Chat::Red, "Unable to retrieve parcel.");
								SendParcelRetrieveAck();
								return;
							}
						}
					}
					inst->SetCharges(item_quantity > 0 ? item_quantity : 1);
					if (PutItemInInventory(free_id, *inst.get(), true)) {
						if (inst->IsClassBag() && inst->GetItem()->BagSlots > 0) {
							for (auto const &i: results) {
								std::unique_ptr<EQ::ItemInstance> item(
									database.CreateItem(
										i.item_id,
										i.quantity,
										i.aug_slot_1,
										i.aug_slot_2,
										i.aug_slot_3,
										i.aug_slot_4,
										i.aug_slot_5,
										i.aug_slot_6
									)
								);
								auto bag_slot = EQ::InventoryProfile::CalcSlotId(free_id, i.slot_id);
								PutItemInInventory(bag_slot, *item.get(), true);
							}
						}
						MessageString(
							Chat::Yellow,
							PARCEL_DELIVERED,
							merchant->GetCleanName(),
							inst->GetItem()->Name,
							p->second.from_name.c_str()
						);
						if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_RETRIEVE)) {
							PlayerEvent::ParcelRetrieve e{};
							e.from_player_name = p->second.from_name;
							e.item_id          = p->second.item_id;
							e.aug_slot_1       = p->second.aug_slot_1;
							e.aug_slot_2       = p->second.aug_slot_2;
							e.aug_slot_3       = p->second.aug_slot_3;
							e.aug_slot_4       = p->second.aug_slot_4;
							e.aug_slot_5       = p->second.aug_slot_5;
							e.aug_slot_6       = p->second.aug_slot_6;
							e.quantity         = p->second.quantity;
							e.sent_date        = p->second.sent_date;
							RecordPlayerEventLog(PlayerEvent::PARCEL_RETRIEVE, e);

							for (auto const &i:results) {
								e.from_player_name = p->second.from_name;
								e.item_id          = i.item_id;
								e.aug_slot_1       = i.aug_slot_1;
								e.aug_slot_2       = i.aug_slot_2;
								e.aug_slot_3       = i.aug_slot_3;
								e.aug_slot_4       = i.aug_slot_4;
								e.aug_slot_5       = i.aug_slot_5;
								e.aug_slot_6       = i.aug_slot_6;
								e.quantity         = i.quantity;
								e.sent_date        = p->second.sent_date;
								RecordPlayerEventLog(PlayerEvent::PARCEL_RETRIEVE, e);
							}
						}
					}
					else {
						MessageString(Chat::Yellow, PARCEL_INV_FULL, merchant->GetCleanName());
						SendParcelRetrieveAck();
						return;
					}
				}
				else {
					MessageString(Chat::Yellow, PARCEL_INV_FULL, merchant->GetCleanName());
					SendParcelRetrieveAck();
					return;
				}
			}
		}

		DeleteParcel(p->second.id);
		SendParcelDelete(parcel_in);
		m_parcels.erase(p);
	}
	SendParcelRetrieveAck();
	SendParcelIconStatus();
}

bool Client::DeleteParcel(uint32 parcel_id)
{
	auto result = CharacterParcelsRepository::DeleteOne(database, parcel_id);
	if (!result) {
		LogError("Error deleting parcel id {} from the database.", parcel_id);
		return false;
	}

	auto it = std::find_if(m_parcels.cbegin(), m_parcels.cend(), [&](const auto &x) { return x.second.id == parcel_id; });
	SetParcelCount(GetParcelCount() - 1);

	return true;
}

void Client::LoadParcels()
{
	m_parcels.clear();
	auto results = CharacterParcelsRepository::GetWhere(database, fmt::format("char_id = '{}'", CharacterID()));

	for (auto const &p: results) {
		m_parcels.emplace(p.slot_id, p);
	}

	SetParcelCount(m_parcels.size());
}

void Client::SendParcelDelete(const ParcelRetrieve_Struct &parcel_in)
{
	std::unique_ptr<EQApplicationPacket> outapp(new EQApplicationPacket(OP_ShopDeleteParcel, sizeof(ParcelRetrieve_Struct)));
	auto data   = (ParcelRetrieve_Struct *) outapp->pBuffer;

	data->merchant_entity_id = parcel_in.merchant_entity_id;
	data->player_entity_id   = parcel_in.player_entity_id;
	data->parcel_slot_id     = parcel_in.parcel_slot_id;
	data->parcel_item_id     = parcel_in.parcel_item_id;

	QueuePacket(outapp.get());
}


int32 Client::FindNextFreeParcelSlot(uint32 char_id)
{
	auto results = CharacterParcelsRepository::GetWhere(
		database,
		fmt::format("char_id = '{}' ORDER BY slot_id ASC", char_id)
	);

	if (results.empty()) {
		return PARCEL_BEGIN_SLOT;
	}

	for (uint32 i = PARCEL_BEGIN_SLOT; i <= RuleI(Parcel, ParcelMaxItems); i++) {
		auto it = std::find_if(results.cbegin(), results.cend(), [&](const auto &x) { return x.slot_id == i; });
		if (it == results.end()) {
			return i;
		}
	}

	return INVALID_INDEX;
}

void Client::SendParcelIconStatus()
{
	std::unique_ptr<EQApplicationPacket> outapp(new EQApplicationPacket(OP_ShopParcelIcon, sizeof(ParcelIcon_Struct)));
	auto data   = (ParcelIcon_Struct *) outapp->pBuffer;

	auto const num_of_parcels = GetParcelCount();

	data->status = IconOn;
	if (num_of_parcels == 0) {
		data->status = IconOff;
	}
	else if (num_of_parcels > RuleI(Parcel, ParcelMaxItems)) {
		data->status = Overlimit;
	}

	QueuePacket(outapp.get());
}

void Client::AddParcel(CharacterParcelsRepository::CharacterParcels &parcel)
{
	auto result = CharacterParcelsRepository::InsertOne(database, parcel);
	if (!result.id) {
		LogError(
			"Failed to add parcel to database.  From {} to id {} item {} quantity {}",
			parcel.from_name,
			parcel.char_id,
			parcel.item_id,
			parcel.quantity
		);
		Message(
			Chat::Yellow,
			"Unable to send parcel at this time.  Please try again later."
		);
		SendParcelAck();
		return;
	}
}
