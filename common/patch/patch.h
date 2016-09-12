#pragma once

#include <opcodemgr.h>
#include <net/packet.h>
#include <net/daybreak_connection.h>
#include <functional>
#include <memory>
#include <map>

namespace EQ
{
	namespace Patches
	{
		enum IdentityMatchStatus
		{
			IdentityMatchFailure,
			IdentityMatchSuccess
		};

		struct Signature
		{
			int match_message_opcode;
			size_t match_message_size;
		};

		class BasePatch
		{
		public:
			typedef std::function<void(const EQ::Net::Packet*, EmuOpcode, EQ::Net::WritablePacket&)> DecodeStructFunction;
			typedef std::function<void(std::shared_ptr<EQ::Net::DaybreakConnection>, EmuOpcode, const EQ::Net::Packet*)> EncodeStructFunction;

			BasePatch() { }
			virtual ~BasePatch() { }
			virtual std::string GetName() const = 0;
	
			IdentityMatchStatus TryIdentityMatch(const EQ::Net::Packet &p) const;
	
			void Decode(const EQ::Net::Packet *in, EmuOpcode& opcode, EQ::Net::WritablePacket& out);
			void Encode(std::shared_ptr<EQ::Net::DaybreakConnection> connection, EmuOpcode opcode, const EQ::Net::Packet *in);
	
			void RegisterDecode(int protocol_number, DecodeStructFunction f);
			void RegisterEncode(EmuOpcode opcode, EncodeStructFunction f);
		protected:
			void SendPacket(std::shared_ptr<EQ::Net::DaybreakConnection> connection, EmuOpcode opcode, const EQ::Net::Packet *p);
	
			std::unique_ptr<OpcodeManager> m_opcode_manager;
			std::map<int, DecodeStructFunction> m_decode;
			std::map<EmuOpcode, EncodeStructFunction> m_encode;
			Signature m_signature;
			int m_message_size;
		};
	}
}
