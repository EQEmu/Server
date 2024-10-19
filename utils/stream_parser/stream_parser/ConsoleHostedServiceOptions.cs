using System.Collections.Generic;
using CommandLine;

namespace StreamParser
{
    public class ConsoleHostedServiceOptions
    {
        [Option("input", Required = true, HelpText = "Input pcap files to be processed.")]
        public IEnumerable<string> Input { get; set; }

        [Option("output", Default = "output/", HelpText = "Directory to put output files")]
        public string Output { get; set; }

        [Option("text", Default = true, HelpText = "Dump connections to text files.")]
        public bool Text { get; set; }

        [Option("binary", Default = false, HelpText = "Dump connections to binary files.")]
        public bool Binary { get; set; }

        [Option("decrypt", Default = false, HelpText = "Decrypt the \"Encrypted\" packets.")]
        public bool Decrypt { get; set; }

        [Option("decompress", Default = null, HelpText = "Which opcodes to attempt to decompress")]
        public IEnumerable<int> DecompressOpcodes { get; set; }
    }
}
