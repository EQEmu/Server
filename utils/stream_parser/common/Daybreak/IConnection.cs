using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace StreamParser.Common.Daybreak
{
    public enum Direction
    {
        ClientToServer,
        ServerToClient
    }

    public enum ConnectionType
    {
        Unknown,
        Login,
        World,
        Chat,
        Zone
    }

    public interface IConnection
    {
        delegate void OnPacketRecvHandler(Connection connection, Direction direction, DateTime packetTime, ReadOnlySpan<byte> data);
        OnPacketRecvHandler OnPacketRecv { get; set; }

        void ProcessPacket(IPAddress srcAddr, int srcPort, DateTime packetTime, ReadOnlySpan<byte> data);
        bool Match(IPAddress srcAddr, int srcPort, IPAddress dstAddr, int dstPort);

        ConnectionType ConnectionType { get; }
        IPAddress ClientAddress { get; }
        int ClientPort { get; }
        IPAddress ServerAddress { get; }
        int ServerPort { get; }
        Guid Id { get; }
    }
}
