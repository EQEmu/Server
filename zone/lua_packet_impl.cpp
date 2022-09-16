#ifdef LUA_EQEMU
#include "masterentity.h"
#include "lua_packet.h"

Lua_Packet::Lua_Packet(int opcode, int size) {
	SetLuaPtrData(new EQApplicationPacket(static_cast<EmuOpcode>(opcode), size));
	owned_ = true;
}

Lua_Packet::Lua_Packet(int opcode, int size, bool raw) {
	if(raw) {
		SetLuaPtrData(new EQApplicationPacket(OP_Unknown, size));
		owned_ = true;

		EQApplicationPacket *self = reinterpret_cast<EQApplicationPacket*>(d_);
		self->SetOpcodeBypass(opcode);
	} else {
		SetLuaPtrData(new EQApplicationPacket(static_cast<EmuOpcode>(opcode), size));
		owned_ = true;
	}
}

Lua_Packet& Lua_Packet::operator=(const Lua_Packet& o) {
	if(o.owned_) {
		owned_ = true;
		EQApplicationPacket *app = reinterpret_cast<EQApplicationPacket*>(o.d_);
		if(app) {
			d_ = new EQApplicationPacket(app->GetOpcode(), app->pBuffer, app->size);

			EQApplicationPacket *self = reinterpret_cast<EQApplicationPacket*>(d_);
			self->SetOpcodeBypass(app->GetOpcodeBypass());
		} else {
			d_ = nullptr;
		}
	} else {
		owned_ = false;
		d_ = o.d_;
	}
	return *this;
}

Lua_Packet::Lua_Packet(const Lua_Packet& o) {
	if(o.owned_) {
		owned_ = true;
		EQApplicationPacket *app = reinterpret_cast<EQApplicationPacket*>(o.d_);
		if(app) {
			d_ = new EQApplicationPacket(app->GetOpcode(), app->pBuffer, app->size);

			EQApplicationPacket *self = reinterpret_cast<EQApplicationPacket*>(d_);
			self->SetOpcodeBypass(app->GetOpcodeBypass());
		} else {
			d_ = nullptr;
		}
	} else {
		owned_ = false;
		d_ = o.d_;
	}
}

int Lua_Packet::GetSize() {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->size);
}

int Lua_Packet::GetOpcode() {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->GetOpcode());
}

void Lua_Packet::SetOpcode(int op) {
	Lua_Safe_Call_Void();
	self->SetOpcodeBypass(static_cast<uint16>(op));
}

int Lua_Packet::GetRawOpcode() {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->GetOpcodeBypass());
}

void Lua_Packet::SetRawOpcode(int op) {
	Lua_Safe_Call_Void();
	self->SetOpcode(static_cast<EmuOpcode>(op));
}

int Lua_Packet::GetWritePosition() {
	Lua_Safe_Call_Int();
	return self->GetWritePosition();
}

void Lua_Packet::SetWritePosition(int offset) {
	Lua_Safe_Call_Void();
	self->SetWritePosition(offset);
}

void Lua_Packet::WriteInt8(int value) {
	Lua_Safe_Call_Void();
	self->WriteUInt8(static_cast<uint8_t>(value));
}

void Lua_Packet::WriteInt8(int offset, int value) {
	Lua_Safe_Call_Void();

	if(offset + sizeof(int8) <= self->size) {
		*reinterpret_cast<int8*>(self->pBuffer + offset) = value;
	}
}

void Lua_Packet::WriteInt16(int value) {
	Lua_Safe_Call_Void();
	self->WriteUInt16(static_cast<uint16_t>(value));
}

void Lua_Packet::WriteInt16(int offset, int value) {
	Lua_Safe_Call_Void();

	if(offset + sizeof(int16) <= self->size) {
		*reinterpret_cast<int16*>(self->pBuffer + offset) = value;
	}
}

void Lua_Packet::WriteInt32(int value) {
	Lua_Safe_Call_Void();
	self->WriteUInt32(static_cast<uint32_t>(value));
}

void Lua_Packet::WriteInt32(int offset, int value) {
	Lua_Safe_Call_Void();

	if(offset + sizeof(int32) <= self->size) {
		*reinterpret_cast<int32*>(self->pBuffer + offset) = value;
	}
}

void Lua_Packet::WriteInt64(int64 value) {
	Lua_Safe_Call_Void();
	self->WriteUInt64(static_cast<uint64_t>(value));
}

void Lua_Packet::WriteInt64(int offset, int64 value) {
	Lua_Safe_Call_Void();

	if(offset + sizeof(int64) <= self->size) {
		*reinterpret_cast<int64*>(self->pBuffer + offset) = value;
	}
}

void Lua_Packet::WriteFloat(float value) {
	Lua_Safe_Call_Void();
	self->WriteFloat(value);
}

void Lua_Packet::WriteFloat(int offset, float value) {
	Lua_Safe_Call_Void();

	if(offset + sizeof(float) <= self->size) {
		*reinterpret_cast<float*>(self->pBuffer + offset) = value;
	}
}

void Lua_Packet::WriteDouble(double value) {
	Lua_Safe_Call_Void();
	self->WriteDouble(value);
}

void Lua_Packet::WriteDouble(int offset, double value) {
	Lua_Safe_Call_Void();

	if(offset + sizeof(double) <= self->size) {
		*reinterpret_cast<double*>(self->pBuffer + offset) = value;
	}
}

void Lua_Packet::WriteString(std::string value) {
	Lua_Safe_Call_Void();
	self->WriteString(value.c_str());
}

void Lua_Packet::WriteString(int offset, std::string value) {
	Lua_Safe_Call_Void();

	if(offset + value.length() + 1 <= self->size) {
		memcpy(self->pBuffer + offset, value.c_str(), value.length());
		*reinterpret_cast<int8*>(self->pBuffer + offset + value.length()) = 0;
	}
}

void Lua_Packet::WriteFixedLengthString(std::string value) {
	Lua_Safe_Call_Void();
	self->WriteLengthString(static_cast<uint32_t>(value.size()), value.c_str());
}

void Lua_Packet::WriteFixedLengthString(int offset, std::string value, int string_length) {
	Lua_Safe_Call_Void();

	if(offset + string_length <= static_cast<int>(self->size)) {
		memset(self->pBuffer + offset, 0, string_length);
		memcpy(self->pBuffer + offset, value.c_str(), value.length());
	}
}

int Lua_Packet::ReadInt8(int offset) {
	Lua_Safe_Call_Int();
	if(offset + sizeof(int8) <= self->size) {
		int8 v = *reinterpret_cast<int8*>(self->pBuffer + offset);
		return v;
	}

	return 0;
}

int Lua_Packet::ReadInt16(int offset) {
	Lua_Safe_Call_Int();

	if(offset + sizeof(int16) <= self->size) {
		int16 v = *reinterpret_cast<int16*>(self->pBuffer + offset);
		return v;
	}

	return 0;
}

int Lua_Packet::ReadInt32(int offset) {
	Lua_Safe_Call_Int();

	if(offset + sizeof(int32) <= self->size) {
		int32 v = *reinterpret_cast<int32*>(self->pBuffer + offset);
		return v;
	}

	return 0;
}

int64 Lua_Packet::ReadInt64(int offset) {
	Lua_Safe_Call_Int();

	if(offset + sizeof(int64) <= self->size) {
		int64 v = *reinterpret_cast<int64*>(self->pBuffer + offset);
		return v;
	}

	return 0;
}

float Lua_Packet::ReadFloat(int offset) {
	Lua_Safe_Call_Real();

	if(offset + sizeof(float) <= self->size) {
		float v = *reinterpret_cast<float*>(self->pBuffer + offset);
		return v;
	}

	return 0;
}

double Lua_Packet::ReadDouble(int offset) {
	Lua_Safe_Call_Real();

	if(offset + sizeof(double) <= self->size) {
		double v = *reinterpret_cast<double*>(self->pBuffer + offset);
		return v;
	}

	return 0;
}

std::string Lua_Packet::ReadString(int offset) {
	Lua_Safe_Call_String();

	if(offset < static_cast<int>(self->size)) {
		std::string ret;

		int i = offset;
		for(;;) {
			if(i >= static_cast<int>(self->size)) {
				break;
			}

			char c = *reinterpret_cast<char*>(self->pBuffer + i);

			if(c == '\0') {
				break;
			}

			ret.push_back(c);
			++i;
		}

		return ret;
	}

	return "";
}

std::string Lua_Packet::ReadFixedLengthString(int offset, int string_length) {
	Lua_Safe_Call_String();

	if(offset + string_length <= static_cast<int>(self->size)) {
		std::string ret;

		int i = offset;
		for(;;) {
			if(i >= offset + string_length) {
				break;
			}

			char c = *reinterpret_cast<char*>(self->pBuffer + i);
			ret.push_back(c);
			++i;
		}

		return ret;
	}

	return "";
}
#endif
