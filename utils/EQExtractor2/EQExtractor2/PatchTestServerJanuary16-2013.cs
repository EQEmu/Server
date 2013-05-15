using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchTestServerJanuary162013Decoder : PatchJanuary162013Decoder
    {
        public PatchTestServerJanuary162013Decoder()
        {
            Version = "EQ Client Build Date Test Server January 16 2013.";

            PatchConfFileName = "patch_TestServer-Jan16-2013.conf";

            SupportsSQLGeneration = false;
        }
    }
}