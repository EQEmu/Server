#include "zone/dragonshoard.h"

#include "common/eqemu_logsys.h"
#include "zone/client.h"
#include "zone/string_ids.h"
#include "zone/zonedb.h"

#include <algorithm>

namespace DragonHoard {

namespace {

// The leading u32 of every OP_DragonHoard packet body. Values are defined by the client.
enum class Action : uint32 {
	WindowOpen   = 0,  // c->s: window opened
	SetSlotCount = 2,  // c->s: request, sent at zone-in; s->c: slot capacity
	Retrieve     = 3,  // c->s: request; s->c: remove the row and release the pending-op counter
	Deposit      = 4,  // c->s: deposit the cursor item
	Enable       = 8,  // s->c: enable the window
	Ack          = 10, // s->c: release the pending-op counter, nothing else
};

#pragma pack(push, 1)

struct SlotCount_Struct {
	Action action;
	uint32 slot_count;
};
static_assert(sizeof(SlotCount_Struct) == 8, "SlotCount_Struct size mismatch");

struct Enable_Struct {
	Action action;
	uint8  flag;
};
static_assert(sizeof(Enable_Struct) == 5, "Enable_Struct size mismatch");

struct Ack_Struct {
	Action action;
};
static_assert(sizeof(Ack_Struct) == 4, "Ack_Struct size mismatch");

struct DepositRequest_Struct {
	Action action;
	uint64 serial_number;
};
static_assert(sizeof(DepositRequest_Struct) == 12, "DepositRequest_Struct size mismatch");

struct Retrieve_Struct {
	Action action;
	uint64 serial_number;
	uint32 quantity; // always 1 from the client; the serial number identifies the row
};
static_assert(sizeof(Retrieve_Struct) == 16, "Retrieve_Struct size mismatch");

#pragma pack(pop)

template <typename T>
void Send(Client* client, const T& body)
{
	auto* outapp = new EQApplicationPacket(
		OP_DragonHoard, reinterpret_cast<const unsigned char*>(&body), sizeof(T));

	client->FastQueuePacket(&outapp);
}

// The client counts each deposit and retrieve request as a pending operation and locks its
// inventory until the count is released, so every request ends with this, as on live.
void SendAck(Client* client)
{
	Send(client, Ack_Struct{Action::Ack});
}

// Not VERIFY_PACKET_LENGTH: a rejected request still has to send the Ack.
template <typename T>
bool HasExactSize(Client* client, const EQApplicationPacket* app)
{
	if (app->size == sizeof(T)) {
		return true;
	}

	LogNetcode("OP_DragonHoard size mismatch, expected [{}] got [{}]", sizeof(T), app->size);
	SendAck(client);

	return false;
}

bool IsEnabled(Client* client)
{
	return client->ClientVersion() >= EQ::versions::ClientVersion::TOB &&
		client->GetHoardCache().SlotLimit() > 0;
}

void SendSlotCount(Client* client)
{
	Send(client, SlotCount_Struct{Action::SetSlotCount, client->GetHoardCache().SlotLimit()});
}

void SendItemRow(Client* client, const EQ::ItemInstance* inst)
{
	// The client keys hoard rows by item serial number. The slot only has to pass the inventory
	// checks in SendItemPacket, which hoard slot ids would not.
	client->SendItemPacket(EQ::invslot::slotCursor, inst, ItemPacketType::ItemPacketDragonHoard);
}

// Every row, then Enable. Live sends this on each window open and after each deposit; the client
// skips rows it already lists.
void SendItemList(Client* client)
{
	client->GetHoardCache().ForEach(database, client->CharacterID(), [client](uint32, const EQ::ItemInstance* inst) {
		SendItemRow(client, inst);
	});

	Send(client, Enable_Struct{Action::Enable, 0});
}

void HandleDeposit(Client* client, const EQApplicationPacket* app)
{
	const auto* request = reinterpret_cast<const DepositRequest_Struct*>(app->pBuffer);

	EQ::ItemInstance* cursor_item = client->GetInv().GetItem(EQ::invslot::slotCursor);
	if (!cursor_item || cursor_item->GetSerialNumber() != static_cast<int32>(request->serial_number)) {
		SendAck(client);
		return;
	}

	// The client refuses these before sending. They are enforced here as well: only the item row is
	// stored, which would lose augments and container contents, and a No Rent item must not outlive
	// its timer. Lore items are allowed, as on live; retrieve refuses a duplicate.
	const bool is_no_rent = cursor_item->GetItem()->NoRent == 0;
	if (cursor_item->IsAugmented() || cursor_item->IsNoneEmptyContainer() || is_no_rent) {
		client->Message(Chat::Red, "That item cannot be stored in your Dragon's Hoard.");
		SendAck(client);
		return;
	}

	HoardCache&  cache   = client->GetHoardCache();
	const uint32 slot_id = cache.FindFreeSlot(database, client->CharacterID());
	if (slot_id >= cache.SlotLimit()) {
		client->Message(Chat::Red, "Your Dragon's Hoard is full.");
		SendAck(client);
		return;
	}

	EQ::ItemInstance* hoard_item = cursor_item->Clone();

	database.TransactionBegin();

	if (!cache.Deposit(database, client->CharacterID(), slot_id, hoard_item)) {
		database.TransactionRollback();
		safe_delete(hoard_item);
		client->Message(Chat::Red, "Your Dragon's Hoard could not store that item.");
		SendAck(client);
		return;
	}

	client->DeleteItemInInventory(EQ::invslot::slotCursor, 0, true);
	database.TransactionCommit();

	// Same order as live: the new row, the Ack, then the full list.
	SendItemRow(client, hoard_item);
	SendAck(client);
	SendItemList(client);
}

void HandleRetrieve(Client* client, const EQApplicationPacket* app)
{
	const auto* request       = reinterpret_cast<const Retrieve_Struct*>(app->pBuffer);
	const auto  serial_number = static_cast<int32>(request->serial_number);

	HoardCache& cache   = client->GetHoardCache();
	const auto  slot_id = cache.FindSlot(database, client->CharacterID(), serial_number);
	if (!slot_id) {
		SendAck(client);
		return;
	}

	if (client->GetInv().GetItem(EQ::invslot::slotCursor)) {
		client->Message(Chat::Red, "You must have an empty cursor to retrieve an item.");
		SendAck(client);
		return;
	}

	if (client->CheckLoreConflict(cache.Get(*slot_id)->GetItem())) {
		client->MessageString(Chat::Red, DUP_LORE);
		SendAck(client);
		return;
	}

	database.TransactionBegin();

	EQ::ItemInstance* item = cache.Retrieve(database, client->CharacterID(), *slot_id);
	if (!item) {
		database.TransactionRollback();
		client->Message(Chat::Red, "Your Dragon's Hoard could not release that item.");
		SendAck(client);
		return;
	}

	// Deposit soft-deleted the evolving record along with the inventory row. Restore it so the
	// inventory load finds the item's progress again.
	item->TransferOwnership(database, client->CharacterID());

	if (!client->PushItemOnCursor(*item, true)) {
		LogError(
			"Dragon's Hoard item [{}] of character [{}] reached the cursor but was not saved",
			item->GetItem()->ID, client->CharacterID()
		);
	}

	database.TransactionCommit();

	// The client removes the row only when the quantity covers the count it lists.
	const auto quantity = static_cast<uint32>(std::max<int16>(item->GetCharges(), 1));

	Send(client, Retrieve_Struct{Action::Retrieve, request->serial_number, quantity});
	SendAck(client);
	safe_delete(item);
}

} // namespace

void HandlePacket(Client* client, const EQApplicationPacket* app)
{
	if (!IsEnabled(client) || app->size < sizeof(Action)) {
		return;
	}

	switch (static_cast<Action>(app->ReadUInt32(0))) {
		case Action::SetSlotCount:
			SendSlotCount(client);
			break;
		case Action::WindowOpen:
			SendItemList(client);
			break;
		case Action::Deposit:
			if (HasExactSize<DepositRequest_Struct>(client, app)) {
				HandleDeposit(client, app);
			}
			break;
		case Action::Retrieve:
			if (HasExactSize<Retrieve_Struct>(client, app)) {
				HandleRetrieve(client, app);
			}
			break;
		default:
			break;
	}
}

} // namespace DragonHoard
