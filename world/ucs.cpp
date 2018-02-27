#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "ucs.h"
#include "world_config.h"
#include "zonelist.h"

#include "../common/misc_functions.h"
#include "../common/md5.h"
#include "../common/packet_dump.h"

extern ZSList zoneserver_list;

UCSConnection::UCSConnection()
{
	Stream = 0;
}

void UCSConnection::SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> inStream)
{
	if (Stream && Stream->Handle())
	{
		Log(Logs::Detail, Logs::UCS_Server, "Incoming UCS Connection while we were already connected to a UCS.");
		Stream->Handle()->Disconnect();
	}

	Stream = inStream;
	if (Stream) {
		Stream->OnMessage(std::bind(&UCSConnection::ProcessPacket, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void UCSConnection::ProcessPacket(uint16 opcode, EQ::Net::Packet &p)
{
	if (!Stream)
		return;

	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	switch (opcode)
	{
		case 0:
			break;

		case ServerOP_KeepAlive:
		{
			// ignore this
			break;
		}
		case ServerOP_ZAAuth:
		{
			Log(Logs::Detail, Logs::UCS_Server, "Got authentication from UCS when they are already authenticated.");
			break;
		}
		case ServerOP_UCSBroadcastServerReady:
		case ServerOP_UCSClientVersionRequest:
		{
			zoneserver_list.SendPacket(pack);
			break;
		}
		default:
		{
			Log(Logs::Detail, Logs::UCS_Server, "Unknown ServerOPcode from UCS 0x%04x, size %d", opcode, pack->size);
			DumpPacket(pack->pBuffer, pack->size);
			break;
		}
	}
}

void UCSConnection::SendPacket(ServerPacket* pack)
{
	if (!Stream)
		return;

	Stream->SendPacket(pack);
}

void UCSConnection::SendMessage(const char *From, const char *Message)
{
	auto pack = new ServerPacket(ServerOP_UCSMessage, strlen(From) + strlen(Message) + 2);

	char *Buffer = (char *)pack->pBuffer;

	VARSTRUCT_ENCODE_STRING(Buffer, From);
	VARSTRUCT_ENCODE_STRING(Buffer, Message);

	SendPacket(pack);
	safe_delete(pack);
}
