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
    class PatchTestServerFebruary52013Decoder : PatchJanuary162013Decoder
    {
        public PatchTestServerFebruary52013Decoder()
        {
            Version = "EQ Client Build Date Test Server February 5 2013.";

            PatchConfFileName = "patch_TestServer-Feb5-2013.conf";

            SupportsSQLGeneration = false;
        }
    }
}