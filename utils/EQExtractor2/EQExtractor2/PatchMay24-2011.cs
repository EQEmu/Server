using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchMay242011Decoder : PatchMay122011Decoder
    {
        public PatchMay242011Decoder()
        {
            Version = "EQ Client Build Date May 24 2011.";

            ExpectedPPLength = 28856;

            PPZoneIDOffset = 21524;
        }
    }
}