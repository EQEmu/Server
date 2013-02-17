using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchOct202010Decoder : PatchTestSep222010Decoder
    {
        public PatchOct202010Decoder()
        {
            Version = "EQ Client Build Date October 20 2010.";

            ExpectedPPLength = 27816;

            PPZoneIDOffset = 20484;
        }
    }
}