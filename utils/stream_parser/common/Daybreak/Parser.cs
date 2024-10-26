using Microsoft.Extensions.Logging;
using SharpPcap;
using SharpPcap.LibPcap;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace StreamParser.Common.Daybreak
{
    public class Parser : IParser
    {
        /**
         * Dependencies
         */ 
        private readonly ILogger<Parser> _logger;
        private readonly List<IConnection> _connections = new List<IConnection>();

        public IParser.ConnectionHandler OnNewConnection { get; set; }
        public IParser.ConnectionHandler OnLostConnection { get; set; }

        public Parser(ILogger<Parser> logger)
        {
            _logger = logger;
        }

        public void Parse(string filename)
        {
            ICaptureDevice device = null;
            try
            {
                device = new CaptureFileReaderDevice(filename);
                device.Open();

                device.OnPacketArrival += new PacketArrivalEventHandler(OnPacketCapture);

                device.Capture();
                device.Close();
            } catch(Exception ex)
            {
                _logger.LogError(ex, "Error reading device capture.");
            }
        }

        private void OnPacketCapture(object sender, PacketCapture capture)
        {
            var raw = capture.GetPacket();
            if (raw.LinkLayerType == PacketDotNet.LinkLayers.Ethernet)
            {
                var packet = PacketDotNet.Packet.ParsePacket(raw.LinkLayerType, raw.Data);
                var ipPacket = packet.Extract<PacketDotNet.IPv4Packet>();
                var udpPacket = packet.Extract<PacketDotNet.UdpPacket>();

                if (ipPacket != null && udpPacket != null)
                {
                    try
                    {
                        ProcessPacket(ipPacket.SourceAddress,
                            udpPacket.SourcePort,
                            ipPacket.DestinationAddress,
                            udpPacket.DestinationPort,
                            raw.Timeval.Date,
                            udpPacket.PayloadData);
                    } catch(Exception ex)
                    {
                        _logger.LogError(ex, "Error processing packet");
                    }
                }
            }
        }

        private void ProcessPacket(IPAddress srcAddr, int srcPort, IPAddress dstAddr, int dstPort, DateTime packetTime, ReadOnlySpan<byte> data)
        {
            if(data.Length < 2)
            {
                _logger.LogTrace("Tossing packet, {0} was less than minimum packet size", data.Length);
                return;
            }

            var c = FindConnection(srcAddr, srcPort, dstAddr, dstPort);

            if(c != null)
            {
                c.ProcessPacket(srcAddr, srcPort, packetTime, data);
            }
            else if (data[0] == 0 && data[1] == Opcode.SessionRequest)
            {
                if(data.Length != 24)
                {
                    _logger.LogTrace("Tossing packet, {0} was not the right size for a SessionRequest", data.Length);
                    return;
                }

                c = new Connection(this, srcAddr, srcPort, dstAddr, dstPort);
                _connections.Add(c);
                c.ProcessPacket(srcAddr, srcPort, packetTime, data);
            }
        }

        private IConnection FindConnection(IPAddress srcAddr, int srcPort, IPAddress dstAddr, int dstPort)
        {
            foreach (var c in _connections)
            {
                if (c.Match(srcAddr, srcPort, dstAddr, dstPort))
                {
                    return c;
                }
            }

            return null;
        }
    }
}
