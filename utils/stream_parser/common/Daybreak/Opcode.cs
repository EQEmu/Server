using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StreamParser.Common.Daybreak
{
    public class Opcode
    {
        public const byte Padding = 0;
        public const byte SessionRequest = 1;
        public const byte SessionResponse = 2;
        public const byte Combined = 3;
        public const byte SessionDisconnect = 5;
        public const byte KeepAlive = 6;
        public const byte SessionStatRequest = 7;
        public const byte SessionStatResponse = 8;
        public const byte Packet = 9;
        public const byte Packet2 = 10;
        public const byte Packet3 = 11;
        public const byte Packet4 = 12;
        public const byte Fragment = 13;
        public const byte Fragment2 = 14;
        public const byte Fragment3 = 15;
        public const byte Fragment4 = 16;
        public const byte OutOfOrderAck = 17;
        public const byte OutOfOrderAck2 = 18;
        public const byte OutOfOrderAck3 = 19;
        public const byte OutOfOrderAck4 = 20;
        public const byte Ack = 21;
        public const byte Ack2 = 22;
        public const byte Ack3 = 23;
        public const byte Ack4 = 22;
        public const byte AppCombined = 23;
        public const byte OutboundPing = 28;
        public const byte OutOfSession = 29;
    }
}
