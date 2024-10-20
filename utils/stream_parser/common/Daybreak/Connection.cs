using Ionic.Zlib;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace StreamParser.Common.Daybreak
{
    public class Connection : IConnection
    {
        private class EncodeType
        {
            public const int None = 0;
            public const int Compression = 1;
            public const int XOR = 4;
        }

        private enum SequenceOrder
        {
            Past,
            Current,
            Future
        }

        private readonly IParser _owner;
        private readonly IPAddress _srcAddr;
        private readonly int _srcPort;
        private readonly IPAddress _dstAddr;
        private readonly int _dstPort;
        private readonly Util.Crc32 _crc_generator = new Util.Crc32();
        private readonly Guid _id = Guid.NewGuid();
        private uint _connect_code = 0;
        private int _encode_key = 0;
        private int _crc_bytes = 0;
        private int[] _encode_pass = new int[2] { 0, 0 };
        private ConnectionStream[] _client_streams = new ConnectionStream[4] {
            new ConnectionStream(),
            new ConnectionStream(),
            new ConnectionStream(),
            new ConnectionStream()
        };
        private ConnectionStream[] _server_streams = new ConnectionStream[4] {
            new ConnectionStream(),
            new ConnectionStream(),
            new ConnectionStream(),
            new ConnectionStream()
        };

        public IConnection.OnPacketRecvHandler OnPacketRecv { get; set; }

        public IPAddress ClientAddress => _srcAddr;
        public int ClientPort => _srcPort;
        public IPAddress ServerAddress => _dstAddr;
        public int ServerPort => _dstPort;
        public Guid Id => _id;

        public ConnectionType ConnectionType {
            get
            {
                //World servers used to be coded to always be 9000 but live started using dynamic ports
                //I've seen from 9000 to 9008 on live
                if (_dstPort >= 9000 && _dstPort <= 9010)
                {
                    return ConnectionType.World;
                }
                else if (_encode_pass[0] == EncodeType.None && _encode_pass[1] == EncodeType.None)
                {
                    return ConnectionType.Login;
                }
                else if (_encode_pass[0] == EncodeType.XOR && _encode_pass[1] == EncodeType.None)
                {
                    return ConnectionType.Chat;
                }
                else if (_encode_pass[0] == EncodeType.Compression && _encode_pass[1] == EncodeType.None)
                {
                    return ConnectionType.Zone;
                }

                return ConnectionType.Unknown;
            }
        }

        public Connection(IParser owner, IPAddress srcAddr, int srcPort, IPAddress dstAddr, int dstPort)
        {
            _owner = owner;
            _srcAddr = srcAddr;
            _srcPort = srcPort;
            _dstAddr = dstAddr;
            _dstPort = dstPort;
        }

        public void ProcessPacket(IPAddress srcAddr, int srcPort, DateTime packetTime, ReadOnlySpan<byte> data)
        {
            if (data.Length < 1)
            {
                return;
            }

            var opcode = data[1];
            if (data[0] == 0 && (opcode == Opcode.KeepAlive || opcode == Opcode.OutboundPing))
            {
                return;
            }

            if (PacketCanBeDecoded(data))
            {
                if (!ValidateCRC(data))
                {
                    return;
                }

                if (_encode_pass[0] == EncodeType.None && _encode_pass[1] == EncodeType.None)
                {
                    ProcessDecodedPacket(srcAddr, srcPort, packetTime, data.Slice(0, data.Length - _crc_bytes));
                }
                else
                {
                    //unfortunately we can't avoid a copy here
                    var temp = data.Slice(0, data.Length - _crc_bytes).ToArray();
                    for (int i = 1; i >= 0; --i)
                    {
                        switch(_encode_pass[i])
                        {
                            case EncodeType.Compression:
                                if(temp[0] == 0)
                                {
                                    temp = Decompress(temp, 2, temp.Length - 2);
                                }
                                else
                                {
                                    temp = Decompress(temp, 1, temp.Length - 1);
                                }
                                break;
                            case EncodeType.XOR:
                                if (temp[0] == 0)
                                {
                                    temp = Decode(temp, 2, temp.Length - 2);
                                }
                                else
                                {
                                    temp = Decode(temp, 1, temp.Length - 1);
                                }
                                break;
                        }
                    }

                    ProcessDecodedPacket(srcAddr, srcPort, packetTime, temp);
                }
            }
            else
            {
                ProcessDecodedPacket(srcAddr, srcPort, packetTime, data);
            }
        }

        private void ProcessDecodedPacket(IPAddress srcAddr, int srcPort, DateTime packetTime, ReadOnlySpan<byte> data)
        {
            if (data.Length < 1)
            {
                return;
            }

            if (data[0] == 0)
            {
                if (data.Length < 2)
                {
                    return;
                }

                var opcode = data[1];
                switch (opcode)
                {
                    case Opcode.SessionResponse:
                        if (_connect_code == 0)
                        {
                            _connect_code = BitConverter.ToUInt32(data.Slice(2, 4));
                            _encode_key = IPAddress.NetworkToHostOrder(BitConverter.ToInt32(data.Slice(6, 4)));
                            _crc_bytes = data[10];
                            _encode_pass[0] = data[11];
                            _encode_pass[1] = data[12];
                            _owner.OnNewConnection?.Invoke(this, packetTime);
                        }
                        break;
                    case Opcode.SessionDisconnect:
                        if(_connect_code != 0)
                        {
                            _connect_code = 0;
                            _encode_key = 0;
                            _crc_bytes = 0;
                            _encode_pass[0] = 0;
                            _encode_pass[1] = 0;
                            _owner.OnLostConnection?.Invoke(this, packetTime);
                        }
                        break;
                    case Opcode.Combined:
                        {
                            int current = 2;
                            int end = data.Length;
                            while (current < end)
                            {
                                byte subpacket_length = data[current];
                                current += 1;

                                if (end < current + subpacket_length)
                                {
                                    return;
                                }

                                var subpacket = data.Slice(current, subpacket_length);
                                ProcessDecodedPacket(srcAddr, srcPort, packetTime, subpacket);
                                current += subpacket_length;
                            }
                        }
                        break;
                    case Opcode.AppCombined:
                        {
                            int current = 2;
                            int end = data.Length;
                            while (current < end)
                            {
                                int subpacket_length = 0;
                                if (data[current] == 0xff)
                                {
                                    if (end < current + 3)
                                    {
                                        return;
                                    }

                                    if (data[current + 1] == 0xff && data[current + 2] == 0xff)
                                    {
                                        if (end < current + 7)
                                        {
                                            return;
                                        }

                                        subpacket_length =
                                            ((data[current + 3]) << 24) |
                                            ((data[current + 4]) << 16) |
                                            ((data[current + 5]) << 8) |
                                            (data[current + 6]);

                                        current += 7;
                                    }
                                    else
                                    {
                                        subpacket_length =
                                            ((data[current + 1]) << 8) |
                                            (data[current + 2]);

                                        current += 3;
                                    }
                                }
                                else
                                {
                                    subpacket_length = data[current];
                                    current += 1;
                                }

                                var subpacket = data.Slice(current, subpacket_length);
                                ProcessDecodedPacket(srcAddr, srcPort, packetTime, subpacket);
                                current += subpacket_length;
                            }
                        }
                        break;
                    case Opcode.Packet:
                    case Opcode.Packet2:
                    case Opcode.Packet3:
                    case Opcode.Packet4:
                        {
                            var stream_id = opcode - Opcode.Packet;
                            var stream = FindStream(srcAddr, srcPort, stream_id);
                            var sequence = (ushort)IPAddress.NetworkToHostOrder(BitConverter.ToInt16(data.Slice(2, 2)));
                            var order = CompareSequence(stream.Sequence, sequence);
                            if (order == SequenceOrder.Future)
                            {
                                if (!stream.PacketQueue.ContainsKey(sequence))
                                {
                                    stream.PacketQueue.Add(sequence, new ConnectionStream.QueuedPacket
                                    {
                                        Data = data.ToArray(),
                                        PacketTime = packetTime
                                    });
                                }
                            }
                            else if (order == SequenceOrder.Current)
                            {
                                if (stream.PacketQueue.ContainsKey(sequence))
                                {
                                    stream.PacketQueue.Remove(sequence);
                                }

                                stream.Sequence++;
                                ProcessDecodedPacket(srcAddr, srcPort, packetTime, data.Slice(4));
                                ProcessQueue(srcAddr, srcPort, stream_id);
                            }
                        }
                        break;
                    case Opcode.Fragment:
                    case Opcode.Fragment2:
                    case Opcode.Fragment3:
                    case Opcode.Fragment4:
                        {
                            var stream_id = opcode - Opcode.Fragment;
                            var stream = FindStream(srcAddr, srcPort, stream_id);
                            var sequence = (ushort)IPAddress.NetworkToHostOrder(BitConverter.ToInt16(data.Slice(2, 2)));
                            var order = CompareSequence(stream.Sequence, sequence);
                            if (order == SequenceOrder.Future)
                            {
                                if (!stream.PacketQueue.ContainsKey(sequence))
                                {
                                    stream.PacketQueue.Add(sequence, new ConnectionStream.QueuedPacket
                                    {
                                        Data = data.ToArray(),
                                        PacketTime = packetTime
                                    });
                                }
                            }
                            else if (order == SequenceOrder.Current)
                            {
                                if (stream.PacketQueue.ContainsKey(sequence))
                                {
                                    stream.PacketQueue.Remove(sequence);
                                }

                                stream.Sequence++;

                                if (stream.TotalFragmentedBytes == 0)
                                {
                                    stream.TotalFragmentedBytes = (uint)IPAddress.NetworkToHostOrder(BitConverter.ToInt32(data.Slice(4, 4)));
                                    stream.CurrentFragmentedBytes = (uint)(data.Length - 8);

                                    if(stream.FragmentBuffer == null || stream.FragmentBuffer.Length < (stream.TotalFragmentedBytes + 512))
                                    {
                                        stream.FragmentBuffer = new byte[stream.TotalFragmentedBytes + 512];
                                    }

                                    var target = stream.FragmentBuffer.AsSpan();
                                    data.Slice(8).CopyTo(target);
                                } else
                                {
                                    var target = stream.FragmentBuffer.AsSpan((int)stream.CurrentFragmentedBytes);
                                    data.Slice(4).CopyTo(target);

                                    stream.CurrentFragmentedBytes += (uint)(data.Length - 4);

                                    if (stream.CurrentFragmentedBytes >= stream.TotalFragmentedBytes)
                                    {
                                        ProcessDecodedPacket(srcAddr, srcPort, packetTime,
                                            stream.FragmentBuffer.AsSpan(0, (int)stream.TotalFragmentedBytes));
                                        stream.CurrentFragmentedBytes = 0;
                                        stream.TotalFragmentedBytes = 0;
                                    }
                                }

                                ProcessQueue(srcAddr, srcPort, stream_id);
                            }

                        }
                        break;
                    case Opcode.Padding:
                        OnPacketRecv?.Invoke(this, GetDirection(srcAddr, srcPort), packetTime, data.Slice(1));
                        break;
                    default:
                        break;
                }
            } else
            {
                OnPacketRecv?.Invoke(this, GetDirection(srcAddr, srcPort), packetTime, data);
            }
        }

        private void ProcessQueue(IPAddress srcAddr, int srcPort, int stream_id)
        {
            var stream = FindStream(srcAddr, srcPort, stream_id);
            var sequence = stream.Sequence;

            //try to get the current sequence in the queue, if it exists then process it
            ConnectionStream.QueuedPacket value;
            if(stream.PacketQueue.TryGetValue(sequence, out value))
            {
                ProcessDecodedPacket(srcAddr, srcPort, value.PacketTime, value.Data);
            }
        }

        public bool Match(IPAddress srcAddr, int srcPort, IPAddress dstAddr, int dstPort)
        {
            var p1 = _srcAddr.Equals(srcAddr) && _srcPort == srcPort && _dstAddr.Equals(dstAddr) && _dstPort == dstPort;
            var p2 = _srcAddr.Equals(dstAddr) && _srcPort == dstPort && _dstAddr.Equals(srcAddr) && _dstPort == srcPort;
            return p1 || p2;
        }

        private SequenceOrder CompareSequence(ushort expected, ushort actual)
        {
            int diff = (int)actual - (int)expected;

            if (diff == 0)
            {
                return SequenceOrder.Current;
            }

            if (diff > 0)
            {
                if (diff > 10000)
                {
                    return SequenceOrder.Past;
                }

                return SequenceOrder.Future;
            }

            if (diff < -10000)
            {
                return SequenceOrder.Future;
            }

            return SequenceOrder.Past;
        }

        private bool PacketCanBeDecoded(ReadOnlySpan<byte> p)
        {
            if (p.Length < 2)
            {
                return false;
            }

            if (p[0] != 0)
            {
                return true;
            }

            var opcode = p[1];

            if (opcode == Opcode.SessionRequest || opcode == Opcode.SessionResponse || opcode == Opcode.OutOfSession)
            {
                return false;
            }

            return true;
        }

        private bool ValidateCRC(ReadOnlySpan<byte> p)
        {
            if (_crc_bytes == 0)
            {
                return true;
            }

            int actual = 0;
            int calculated = _crc_generator.Calculate(p.Slice(0, p.Length - _crc_bytes), _encode_key);
            switch (_crc_bytes)
            {
                case 2:
                    actual = IPAddress.NetworkToHostOrder(BitConverter.ToInt16(p.Slice(p.Length - 2, 2))) & 0xFFFF;
                    calculated = calculated & 0xFFFF;
                    break;
                case 4:
                    actual = IPAddress.NetworkToHostOrder(BitConverter.ToInt32(p.Slice(p.Length - 4, 4)));
                    break;
                default:
                    return false;
            }

            return actual == calculated;
        }

        private byte[] Decompress(byte[] p, int offset, int length)
        {
            if (length < 2)
            {
                return p;
            }

            Span<byte> header = p.AsSpan(0, offset);
            byte flag = p[offset];
            Span<byte> payload = p.AsSpan(offset + 1, length - 1);

            if (flag == 0x5a)
            {
                var pl = payload.ToArray();
                var inflated = Inflate(payload.ToArray());
                byte[] ret = new byte[offset + inflated.Length];
                Array.Copy(p, 0, ret, 0, offset);
                Array.Copy(inflated, 0, ret, offset, inflated.Length);
                return ret;
            }
            else if (flag == 0xa5)
            {
                byte[] ret = new byte[offset + length - 1];
                Array.Copy(p, 0, ret, 0, offset);
                Array.Copy(p, offset + 1, ret, offset, length - 1);
                return ret;
            }
            else
            {
                return p;
            }
        }

        private byte[] Inflate(byte[] p)
        {
            try
            {
                using (var out_stream = new MemoryStream())
                {
                    using (var in_stream = new MemoryStream(p))
                    {
                        var buffer = new byte[512];
                        using (var zs = new ZlibStream(in_stream, CompressionMode.Decompress))
                        {
                            int r = 0;
                            do
                            {
                                r = zs.Read(buffer, 0, 512);
                                out_stream.Write(buffer, 0, r);
                            } while (r == 512);
                        }
                    }

                    var ret = out_stream.ToArray();
                    return ret;
                }
            }
            catch (Exception)
            {
                return null;
            }
        }

        private byte[] Decode(byte[] p, int offset, int length)
        {
            int key = _encode_key;
            Span<byte> buffer = p.AsSpan(offset, length);
            int i = 0;
            for (i = 0; i + 4 <= length; i += 4)
            {
                int pt = BitConverter.ToInt32(buffer.Slice(i)) ^ key;
                key = BitConverter.ToInt32(buffer.Slice(i));

                if(BitConverter.TryWriteBytes(buffer.Slice(i), pt) == false)
                {
                    throw new Exception("Error writing bytes back in decode.");
                }
            }

            byte kc = (byte)(key & 0xFF);
            for (; i < length; i++)
            {
                buffer[i] = (byte)(buffer[i] ^ kc);
            }

            return p;
        }

        private Direction GetDirection(IPAddress srcAddr, int srcPort)
        {
            if(srcAddr.Equals(_srcAddr) && srcPort == _srcPort)
            {
                return Direction.ClientToServer;
            } else
            {
                return Direction.ServerToClient;
            }
        }

        private ConnectionStream FindStream(IPAddress srcAddr, int srcPort, int index)
        {
            if (index < 0 || index > 3)
            {
                return null;
            }

            var dir = GetDirection(srcAddr, srcPort);

            if(dir == Direction.ClientToServer)
            {
                return _client_streams[index];
            } else
            {
                return _server_streams[index];
            }
        }

        private class ConnectionStream
        {
            public class QueuedPacket
            {
                public byte[] Data { get; set; }
                public DateTime PacketTime { get; set; }
            }

            public ConnectionStream()
            {
                Sequence = 0;
                CurrentFragmentedBytes = 0;
                TotalFragmentedBytes = 0;
                FragmentBuffer = null;
                PacketQueue = new Dictionary<ushort, QueuedPacket>();
            }

            public ushort Sequence { get; set; }
            public uint CurrentFragmentedBytes { get; set; }
            public uint TotalFragmentedBytes { get; set; }
            public byte[] FragmentBuffer { get; set; }
            public Dictionary<ushort, QueuedPacket> PacketQueue { get; set; }
        }
    }
}
