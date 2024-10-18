using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using StreamParser.Common.Daybreak;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Threading;
using System.Threading.Tasks;
using CommandLine;
using System.Globalization;
using Org.BouncyCastle.Crypto.Engines;
using Org.BouncyCastle.Crypto.Modes;
using Org.BouncyCastle.Crypto;
using Org.BouncyCastle.Crypto.Parameters;
using Ionic.Zlib;

namespace StreamParser
{
    public class ConsoleHostedService : IHostedService
    {
        private class ParsedPacket {
            public byte[] Data { get; set; }
            public Direction Direction { get; set; }
            public DateTime Time { get; set; }
        }

        private class ParsedConnection
        {
            public IPAddress ClientAddress { get; set; }
            public int ClientPort { get; set; }
            public IPAddress ServerAddress { get; set; }
            public int ServerPort { get; set; }
            public ConnectionType ConnectionType { get; set; }
            public List<ParsedPacket> Packets { get; set; }
            public DateTime ConnectedTime { get; set; }
            public DateTime? DisconnectedTime { get; set; }
        }

        private readonly ILogger<ConsoleHostedService> _logger;
        private readonly IHostApplicationLifetime _applicationLifetime;
        private readonly IParser _parser;
        private readonly Dictionary<Guid, ParsedConnection> _connections = new Dictionary<Guid, ParsedConnection>();

        public ConsoleHostedService(ILogger<ConsoleHostedService> logger,
            IHostApplicationLifetime applicationLifetime,
            IParser parser)
        {
            _logger = logger;
            _applicationLifetime = applicationLifetime;
            _parser = parser;
        }

        public Task StartAsync(CancellationToken cancellationToken)
        {
            _applicationLifetime.ApplicationStarted.Register(() =>
            {
                var args = Environment.GetCommandLineArgs();

                CommandLine.Parser.Default.ParseArguments<ConsoleHostedServiceOptions>(args)
                   .WithParsed<ConsoleHostedServiceOptions>(o =>
                   {
                       _parser.OnNewConnection += OnNewConnection;
                       _parser.OnLostConnection += OnLostConnection;

                       foreach(var f in o.Input)
                       {
                           _logger.LogInformation("Parsing {0}...", f);
                           _parser.Parse(f);
                       }

                       foreach(var c in _connections)
                       {
                           if(o.Text)
                           {
                               DumpConnectionToTextFile(c.Value, o.Output, o.Decrypt, o.DecompressOpcodes);
                           }
                       }

                       _applicationLifetime.StopApplication();
                   })
                   .WithNotParsed<ConsoleHostedServiceOptions>(e =>
                   {
                       bool stops_processing = false;
                       foreach(var err in e)
                       {
                           _logger.LogError("Error: {0}", err.Tag);
                           stops_processing = stops_processing || err.StopsProcessing;
                       }

                       if(stops_processing)
                       {
                           _applicationLifetime.StopApplication();
                       }
                   });
            });

            return Task.CompletedTask;
        }

        public Task StopAsync(CancellationToken cancellationToken)
        {
            return Task.CompletedTask;
        }

        private void OnNewConnection(IConnection connection, DateTime connectionTime)
        {
            _logger.LogTrace("New connection {0}:{1} <-> {2}:{3} of type {4}", 
                connection.ClientAddress, 
                connection.ClientPort, 
                connection.ServerAddress, 
                connection.ServerPort,
                connection.ConnectionType);
            connection.OnPacketRecv += OnPacketRecv;

            _connections.Add(connection.Id, new ParsedConnection
            {
                ClientAddress = connection.ClientAddress,
                ClientPort = connection.ClientPort,
                ServerAddress = connection.ServerAddress,
                ServerPort = connection.ServerPort,
                ConnectionType = connection.ConnectionType,
                Packets = new List<ParsedPacket>(),
                ConnectedTime = connectionTime,
            });
        }

        private void OnLostConnection(IConnection connection, DateTime connectionTime)
        {
            _logger.LogTrace("Lost connection {0}:{1} <-> {2}:{3}",
                connection.ClientAddress,
                connection.ClientPort,
                connection.ServerAddress,
                connection.ServerPort);
            connection.OnPacketRecv -= OnPacketRecv;

            var parsedConnection = _connections.GetValueOrDefault(connection.Id);
            parsedConnection.DisconnectedTime = connectionTime;
        }

        private void OnPacketRecv(IConnection connection, Direction direction, DateTime packetTime, ReadOnlySpan<byte> data)
        {
            var parsedConnection = _connections.GetValueOrDefault(connection.Id);
            parsedConnection.Packets.Add(new ParsedPacket
            {
                Data = data.ToArray(),
                Direction = direction,
                Time = packetTime
            });
        }

        private void DumpConnectionToTextFile(ParsedConnection c, string output, bool decrypt, IEnumerable<int> decompressOpcodes)
        {
            try
            {
                var path = output + string.Format("{0}-{1}.txt", c.ConnectionType.ToString().ToLower(), c.ConnectedTime.ToString("yyyyMMddHHmmssfff"));
                if (File.Exists(path))
                {
                    File.Delete(path);
                }

                File.AppendAllText(path, string.Format("### type: {0}\n", c.ConnectionType));
                File.AppendAllText(path, string.Format("### started: {0}\n", c.ConnectedTime.ToString("s")));
                File.AppendAllText(path, string.Format("### ended: {0}\n", c.DisconnectedTime.HasValue ? c.DisconnectedTime.Value.ToString("s") : "unknown"));
                File.AppendAllText(path, string.Format("### client: {0}:{1}\n", c.ClientAddress.ToString(), c.ClientPort));
                File.AppendAllText(path, string.Format("### server: {0}:{1}\n\n", c.ServerAddress.ToString(), c.ServerPort));

                foreach(var p in c.Packets)
                {
                    ReadOnlySpan<byte> data = p.Data;
                    string dir = p.Direction == Direction.ClientToServer ? "Client -> Server" : "Server -> Client";

                    switch (c.ConnectionType)
                    {
                        case ConnectionType.Login:
                            {
                                int opcode = BitConverter.ToUInt16(data.Slice(0, 2));
                                {
                                    File.AppendAllText(path,
                                    string.Format("{0} [Opcode: 0x{1}, Size: {2}] ({3})\n", dir, opcode.ToString("X4"), data.Length - 2, p.Time.ToString("s")));

                                    var gp = new GamePacket(data.Slice(2));
                                    File.AppendAllText(path, string.Format("{0}\n", gp.ToString()));

                                    if(decrypt && opcode == 2 || opcode == 24)
                                    {
                                        var encrypted_block = data.Slice(12, data.Length - 12);
                                        var dec = EQDecrypt(encrypted_block);

                                        if(dec != null)
                                        {
                                            File.AppendAllText(path, string.Format("[Decrypted Data, Offset: {0}, Size: {1}]\n", 10, dec.Length));
                                            gp = new GamePacket(dec);
                                            File.AppendAllText(path, string.Format("{0}\n", gp.ToString()));
                                        }
                                    }
                                }
                            }
                            break;
                        case ConnectionType.Chat:
                            {
                                int opcode = data[0];
                                File.AppendAllText(path, 
                                    string.Format("{0} [Opcode: 0x{1}, Size: {2}] ({3})\n", dir, opcode.ToString("X2"), data.Length - 1, p.Time.ToString("s")));

                                var gp = new GamePacket(data.Slice(1));
                                File.AppendAllText(path, string.Format("{0}\n", gp.ToString()));
                            }
                            break;
                        default:
                            {
                                bool reported_decompressed = false;
                                int opcode = BitConverter.ToUInt16(data.Slice(0, 2));
                                foreach (var decompressOpcode in decompressOpcodes)
                                {
                                    if (opcode == decompressOpcode && data.Length > 12)
                                    {
                                        if (data[10] == 0x78 && data[11] == 0xDA)
                                        {
                                            var totalLen = BitConverter.ToInt32(data.Slice(6, 4));
                                            if (totalLen > 0)
                                            {
                                                var decompressed = Inflate(data.Slice(10));
                                                if(decompressed != null)
                                                {
                                                    var decompressed_gp = new GamePacket(decompressed);
                                                    File.AppendAllText(path,
                                                        string.Format("{0} [Opcode: 0x{1}, Size (decompressed): {2}] ({3})\n", dir, opcode.ToString("X4"), totalLen, p.Time.ToString("s")));

                                                    File.AppendAllText(path, string.Format("{0}\n", decompressed_gp.ToString()));
                                                    reported_decompressed = true;
                                                    break;
                                                }
                                            }
                                        } 
                                        else if (data[6] == 0x78 && data[7] == 0xDA)
                                        {
                                            var totalLen = BitConverter.ToInt32(data.Slice(2, 4));
                                            if (totalLen > 0)
                                            {
                                                var decompressed = Inflate(data.Slice(6));
                                                if (decompressed != null)
                                                {
                                                    File.AppendAllText(path,
                                                        string.Format("{0} [Opcode: 0x{1}, Size (decompressed): {2}] ({3})\n", dir, opcode.ToString("X4"), totalLen, p.Time.ToString("s")));

                                                    var decompressed_gp = new GamePacket(decompressed);
                                                    File.AppendAllText(path, string.Format("{0}\n", decompressed_gp.ToString()));
                                                    reported_decompressed = true;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }

                                if (!reported_decompressed)
                                {
                                    File.AppendAllText(path,
                                        string.Format("{0} [Opcode: 0x{1}, Size: {2}] ({3})\n", dir, opcode.ToString("X4"), data.Length - 2, p.Time.ToString("s")));
                                    var gp = new GamePacket(data.Slice(2));
                                    File.AppendAllText(path, string.Format("{0}\n", gp.ToString()));
                                }
                            }
                            break;
                    }
                }
            }
            catch(Exception ex)
            {
                _logger.LogError(ex, "Error dumping connection {0} to txt file", c.ConnectedTime.ToString("s"));
            }
        }

        private byte[] Inflate(ReadOnlySpan<byte> data)
        {
            try
            {
                using (var out_stream = new MemoryStream())
                using (var in_stream = new MemoryStream(data.ToArray()))
                {
                    const int bufferLen = 4096;
                    var buffer = new byte[bufferLen];
                    using (var zs = new ZlibStream(in_stream, CompressionMode.Decompress))
                    {
                        int r = 0;
                        do
                        {
                            r = zs.Read(buffer, 0, bufferLen);
                            out_stream.Write(buffer, 0, r);
                        } while (r == bufferLen);
                    }

                    var ret = out_stream.ToArray();
                    return ret;
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error inflating data");
                return null;
            }
        }

        private byte[] EQDecrypt(ReadOnlySpan<byte> data)
        {
            try
            {
                var desEngine = new DesEngine();
                var cbcBlockCipher = new CbcBlockCipher(desEngine);
                var bufferedBlockCipher = new BufferedBlockCipher(cbcBlockCipher);
                bufferedBlockCipher.Init(false, new ParametersWithIV(new KeyParameter(new byte[16]), new byte[8]));
                var cipherData = new byte[bufferedBlockCipher.GetOutputSize(data.Length)];
                var outputLength = bufferedBlockCipher.ProcessBytes(data.ToArray(), 0, data.Length, cipherData, 0);
                bufferedBlockCipher.DoFinal(cipherData, outputLength);
                return cipherData;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error decrypting EQ Datablock");
                return null;
            }
        }
    }
}
