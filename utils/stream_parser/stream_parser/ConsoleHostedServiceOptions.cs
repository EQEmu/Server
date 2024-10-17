using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
        public bool Dump { get; set; }

        [Option("csv", Default = false, HelpText = "Dump connections to csv files appropriate for building ML models.")]
        public bool Csv { get; set; }

        [Option("decrypt", Default = false, HelpText = "Decrypt the \"Encrypted\" packets.")]
        public bool Decrypt { get; set; }
    }
}
